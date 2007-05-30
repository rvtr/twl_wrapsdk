/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* File APIENUM.C */
/* pc_enumerate - List select all directory entries that match rules and
*                  allow a callback  routine to process each on.
*
*  Description - This routine traverses a subdirectory tree. It tests each
*  directory entry to see if it matches user supplied selection criteria.
*  if it does match the criteria a user supplied callback function is 
*  called with the full path name of the directory entry and a pointer
*  to a DSTAT structure that contains detailed information about the
*  directory entry. (see the manual page for a detailed description of the
*  DSTAT structure.
*  
*  Selection criteria - Two arguments are used to determine the selection 
*  criteria. On is a flags word that specifies attributes the other is
*  a pattern that specifies a wild card pattern.
*  The flags argument contains a bitwise or of one or more of the following:
*    MATCH_DIR      - Select directory entries
*    MATCH_VOL      - Select volume labels
*    MATCH_FILES    - Select files
*    MATCH_DOT      - Select the '.'  entry MATH_DIR must be true also
*    MATCH_DOTDOT   - Select the '..' entry MATCH_DIR must be true also
*
*  The selection pattern is a standard wildcard pattern such as '*.*' or
*  *.txt. 
*  Note: Patterns don't work the same for VFAT and DOS 8.3. If VFAT is
*  enable the pattern *.* will return any file name that has a '.' in it
*  in 8.3 systems it returns all files. 
*
*  Note: pc_enumerate() requires a fair amount of buffer space to function.
*  Instead of allocating the space internally we require that the application
*  pass two buffers of size EMAXPATH_BYTES in to the function. See below.
*
*  Se
*  Summary:
*
* int pc_enumerate(
*   byte * from_pattern_buffer - pointer to a scratch buffer of size EMAXPATH_BYTES
*   byte * spath_buffer        - pointer to a scratch buffer of size EMAXPATH_BYTES
*   byte * dpath_buffer        - pointer to a scratch buffer of size EMAXPATH_BYTES
*   byte * root_search         - Root of the search IE C:\ or C:\USR etc.
*   word match_flags           - Selection flags (see above)
*   byte match_pattern         - Match pattern (see above)
*   int     maxdepth           - Maximum depth of the traversal. 
*                                Note: to scan only one level set this to
*                                1. For all levels set it to 99
*   PENUMCALLBACK pcallback    - User callback function. (see below)
*
*  Return value
*   pc_enumerate() returns 0 unless the callback function returns a non-
*   zero value at any point. If the callback returns a non-zero value the
*   scan terminates imediately and returns the returned value to the
*   application.
*
*   errno is set to one of the following
*   pc_enumerate() does not set errno
*
*
*  About the callback.
*
*  The callback function is a function that returns an integer and is passed
*  the fully qualified path to the current directory entry and a DSTAT 
*  structure. The callback fuction must return 0 if it wishes the scan to
*  continue or any other integer value to stop the scan and return the 
*  callback's return value to the application layer. 
*
* Examples
*
* The next two function implement a multilevel directory scan.
* int rdir_callback(byte *path, DSTAT *d) {printf("%s\n", path);return(0);}
* 
* rdir(byte *path, byte *pattern)
* {
*   pc_enumerate(from_path,from_pattern,spath,dpath,path, 
*   (MATCH_DIR|MATCH_VOL|MATCH_FILES), pattern, 99, rdir_callback);
* }
* 
* Poor mans deltree package 
* int delfile_callback(byte *path, DSTAT *d) {
*     pc_unlink(path);  return(0);
* }
* int deldir_callback(byte *path, DSTAT *d) {
*     pc_rmdir(path); return(0);
* }
* 
*
* deltree(byte *path)
* {
* int i;
*  ==> First delete all of the files
*   pc_enumerate(from_path,from_pattern,spath,dpath,path, 
*   (MATCH_FILES), "*",99, delfile_callback);
*   i = 0;
*   ==> Now delete all of the dirs.. deleting path won't  work until the 
*   ==> tree is empty 
*   while(!pc_rmdir(path) && i++ < 50)
*       pc_enumerate(from_path,from_pattern,spath,dpath,path, 
*       (MATCH_DIR), "*", 99, deldir_callback);
* }     
*
*/

#include <rtfs.h>

#if (!INCLUDE_CS_UNICODE) /* BUGBUG - Not doing ENUM yet */
#define MATCH_DIR       0x01
#define MATCH_VOL       0x02
#define MATCH_FILES     0x04
#define MATCH_DOT       0x08
#define MATCH_DOTDOT    0x10
typedef int (*PENUMCALLBACK)(byte *path, DSTAT *pstat);

int pc_enumerate(
                 byte    * from_path_buffer,
                 byte    * from_pattern_buffer,
                 byte    * spath_buffer,
                 byte    * dpath_buffer,
                 byte    * root_search,
                 word    match_flags,
                 byte    * match_pattern,
                 int     maxdepth,
                 PENUMCALLBACK pcallback);


#if (VFAT)
/* UNICODE Fixed */
/* #define ALL_FILES "*" */
#define ALL_FILES "*"
#else
/* UNICODE Fixed */
#define ALL_FILES "*.*"
/* #define ALL_FILES string_star_dot_star */
#endif

int get_parent_path(byte *parent, byte *path);
int dirscan_isdot(DSTAT *statobj);
int dirscan_isdotdot(DSTAT *statobj);


int pc_enumerate( /* __apifn__ */
                 byte    * from_path_buffer,
                 byte    * from_pattern_buffer,
                 byte    * spath_buffer,
                 byte    * dpath_buffer,
                 byte    * root_search,
                 word    match_flags,
                 byte    * match_pattern,
                 int     maxdepth,
                 PENUMCALLBACK pcallback)
{
    int     dir_index[30];
    dword   dir_block[30];
    int depth;
    DSTAT statobj;
    int process_it;
    int dodone;
    int call_back;
    int ret_val;
#if (!VFAT)
    int i;
    /* IN 8.3 systems we parse the match pattern into file and ext parts */
    byte filepat[9];
    byte extpat[9];
    pc_ascii_fileparse(filepat, extpat, (byte *)match_pattern);
    for(i = 0; i < 8; i++) if (filepat[i]==' ') filepat[i]=0;
    for(i = 0; i < 3; i++) if (extpat[i]==' ') extpat[i]=0;
#endif
    
    ret_val = 0;
    pc_str2upper((byte *)from_path_buffer, (byte *) root_search);
    
    depth = 0;
    
    pc_mpath(from_pattern_buffer, from_path_buffer, (byte *)ALL_FILES);
    
    dir_index[0] = 0;
    dir_block[0] = 0;
    
    do
    {
step_into_dir:
    dodone = 0;
    
    if (pc_gfirst(&statobj, (byte *)from_pattern_buffer)) 
    { 
        dodone = 1;
        process_it = 0;
        do
        {
            if (!dir_block[depth] || ((dir_block[depth] ==
                ((DROBJ *) (statobj.pobj))->blkinfo.my_block ) &&
                (dir_index[depth] ==
                ((DROBJ *) (statobj.pobj))->blkinfo.my_index )))
            {
                process_it = 1;
            }
            if (process_it)
            {
                call_back = 1;
                
                /* Don't report directories if not requested */ 
                if ((statobj.fattribute & ADIRENT) &&
                    !(match_flags & MATCH_DIR) )
                    call_back = 0;
                
                /* Don't report volumes if not requested */ 
                if (call_back && (statobj.fattribute & AVOLUME) &&
                    !(match_flags & MATCH_VOL) )
                    call_back = 0;
                
                /* Don't report plain files if not requested */ 
                if (call_back && !(statobj.fattribute & (AVOLUME|ADIRENT)) &&
                    !(match_flags & MATCH_FILES) )
                    call_back = 0;
                
                /* Don't report DOT if not requested */ 
                if (call_back && dirscan_isdot(&statobj) && !(match_flags & MATCH_DOT))
                    call_back = 0;
                
                /* Don't report DOTDOT if not requested */ 
                if (call_back && dirscan_isdotdot(&statobj) && !(match_flags & MATCH_DOTDOT))
                    call_back = 0;
                
                if (call_back)
                {
                    /* Take it if the pattern match work */
#if (VFAT)
                    /* Under VFAT do a pattern match on the long file name */
                    if (statobj.lfname[0])
                        call_back = pc_patcmp_vfat(match_pattern, statobj.lfname, TRUE);
                    else
                        call_back = pc_patcmp_vfat(match_pattern, (byte *)(statobj.filename), TRUE);
#else
                    /* Non VFAT uses 8.3 matching conventions */
                    pc_ascii_fileparse(filepat, extpat, (byte *)match_pattern);
                    call_back = 
                        pc_patcmp_8((byte *)&statobj.fname[0], (byte *)&filepat[0] , TRUE);
                    call_back = call_back &&
                        pc_patcmp_3((byte *)&statobj.fext[0],  (byte *)&extpat[0]  , TRUE);
#endif
                }
                /* Construct the full path */
                pc_mpath(dpath_buffer, from_path_buffer, (byte *)statobj.filename);
                if (call_back && pcallback)
                {
                    /* If the callback returns non zero he says quit */
                    ret_val = pcallback(dpath_buffer, &statobj);
                    if (ret_val)
                    {
                        pc_gdone(&statobj);
                        goto ex_it;
                    }
                }
            }
            /* If it is a subdirectory and we are still in our depth range
            then process the subdirectory */
            if (process_it && (depth < maxdepth) &&
                !dirscan_isdot(&statobj) &&
                !dirscan_isdotdot(&statobj))
            {
                /* Construct the full path */
                pc_mpath(spath_buffer, from_path_buffer, (byte *)statobj.filename);
                
                if (statobj.fattribute & (AVOLUME | ADIRENT))
                {
                    if (pc_gnext(&statobj))
                    {
                        /* If path is a dir, mark our place and step in */
                        dir_block[depth] = ((DROBJ *) (statobj.pobj))->blkinfo.my_block;
                        dir_index[depth] = ((DROBJ *) (statobj.pobj))->blkinfo.my_index;
                        dodone = 0;
                        pc_gdone(&statobj);
                        
                    }
                    else
                    {
                        dodone = 0;
                        pc_gdone(&statobj);
                        dir_index[depth] = -1;
                    }
                    
                    depth += 1;
                    dir_block[depth] = 0;
                    dir_index[depth] = 0;
                    
                    rtfs_cs_strcpy((byte *)from_path_buffer, (byte *)spath_buffer);
                    pc_mpath(from_pattern_buffer, from_path_buffer, (byte *)ALL_FILES);
                    
                    goto step_into_dir;
                }
            }
            } while (pc_gnext(&statobj));   /* Get the next file in dir */
            
            if (dodone)
                pc_gdone(&statobj);
        }
        
        if (!get_parent_path(from_path_buffer, from_path_buffer))
            break;
        pc_mpath(from_pattern_buffer, from_path_buffer, (byte *)ALL_FILES);
        depth--;
        while (depth >= 0 && dir_index[depth] == -1)
        {
            if (!get_parent_path(from_path_buffer, from_path_buffer))
                break;
            pc_mpath(from_pattern_buffer, from_path_buffer, (byte *)ALL_FILES);
            depth--;
        }
    } while (depth >= 0);
ex_it:
    return (ret_val);
}

int get_parent_path(byte *parent, byte *path)                                   /*__fn__*/
{
    byte *last_backslash;
    int  size;
    
    last_backslash = 0;
    
    size = 0;
    while (*path != '\0')
    {
        size++;
        if (*path == '\\')
        {
            last_backslash = parent;
        }
        *(parent++) = *(path++);
    }
    /* This is to prevent catastrophie if caller ignores failure */
    *parent = '\0'; 
    
    if (size < 3)
        return (0);
    
    if (last_backslash)
        *last_backslash = '\0';
    return (1);
}


/************************************************************************
*                                                                      *
* File system abstraction layer                                        *
*                                                                      *
************************************************************************/


int dirscan_isdot(DSTAT *statobj)
{
    return(rtfs_strcmp((byte *)".", statobj->filename)==0);
}

int dirscan_isdotdot(DSTAT *statobj)
{
    return(rtfs_strcmp((byte *)"..", statobj->filename)==0);
}

#ifdef NOTDEF
/* The section from here to the bottom of the file contains code that 
shows via example how to use the enumerate function.
*/


#include <conio.h>
#include <ctype.h>
#include <direct.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <stdlib.h>
#include <stdio.h>


void usage(byte *progname)
{
    RTFS_PRINTF("Usage:  %s [options] <source_path> <dest_path_1> [<dest_path_2> ...]\n",progname);
}


rdir(byte *path, byte *pattern);
deltree(byte *path, byte *pattern);




#define FILTER ALL_FILES        /* Default filter */

int main(int argc,byte **argv)
{
    byte src_path[EMAXPATH_BYTES];
    byte filter[20];
    byte op;
    rtfs_kernel_init();
    
    argc--;
    argv++;
    
    if (argc < 2)
    {
        goto usage;
    }
    op = *argv[0];
    rtfs_cs_strcpy((byte *)filter,FILTER);
    
    argc--; argv++;
    /* Process user options */
    rtfs_cs_strcpy(src_path,*argv);
    
    argc--; argv++;
    if (argc)
    {
        rtfs_cs_strcpy(filter,*argv);
    }
    else
    {
        rtfs_cs_strcpy((byte *)filter,FILTER);
    }
    
    if (op=='R')
    {
        rdir(src_path, filter);
    }
    else if (op == 'D')
    {
        deltree(src_path, filter);
    }
    else
    {
usage:
    RTFS_PRINTF("(R)dir or (D)eltree PATH [pattern]\n");
    }
    
    
    
    /*  treeprintnew (src_path, filter, be_verbose); */
}

/* Examples */
byte    from_path[EMAXPATH_BYTES];
byte    from_pattern[EMAXPATH_BYTES];
byte    spath[EMAXPATH_BYTES];
byte    dpath[EMAXPATH_BYTES];
/* Recursive DIR function */
int rdir_callback(byte *path, DSTAT *d)
{
    RTFS_PRINTF("%s\n", path);
    return(0);
}

rdir(byte *path, byte *pattern)
{
    pc_enumerate(from_path,from_pattern,spath,dpath,path, 
        (MATCH_DIR|MATCH_VOL|MATCH_FILES), pattern, 99, rdir_callback);
}

/* Poor mans deltree package */
int delfile_callback(byte *path, DSTAT *d)
{
    RTFS_PRINTF("Deleting file %s\n", path);
    pc_unlink(path);
    return(0);
}
int deldir_callback(byte *path, DSTAT *d)
{
    RTFS_PRINTF("Deleting directory %s\n", path);
    pc_rmdir(path);
    return(0);
}


deltree(byte *path)
{
    int i;
    /* First delete all of the files */
    pc_enumerate(from_path,from_pattern,spath,dpath,path, 
        (MATCH_FILES), "*",99, delfile_callback);
    i = 0;
    /* Now delete all of the dirs.. deleting the root of the path won't
    work until the tree is empty */
    while(!pc_rmdir(path) && i++ < 50)
    {
        pc_enumerate(from_path,from_pattern,spath,dpath,path, 
            (MATCH_DIR), "*", 99, deldir_callback);
        
    }
}

#endif
#endif /* (!INCLUDE_CS_UNICODE)  BUGBUG - Not doing ENUM yet */

