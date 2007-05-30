/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright Peter Van Oudenaren , 1993
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/****************************************************************************
CHKDSK.C   - Check Files System Integrity

  Summary

    CHKDSK

      Description
      This program scans the disk searching for crosslinked chains and
      and lost clusters. If the -f argument is present it converts lost
      chains to FILE???.CHK in the root, and file sizes are adjusted if
      they are not correct.

        If -v is specified every file on the disk is listed. If crossed cluster
        chains are found the files/directories containing them are printed.

          NOTE: This program is portable except for calls to exit.

            Bugs:
            This routine does not recognize bad directories as well as it could.
            While travesring directories it calls pc_fndnode(). This in itself is
            good but we have to add validation code to fndnode and makenode (soon).

              Returns:

                Example:


****************************************************************************/
/* Tips for usage:

  A. First run chkdsk without the save lost clusters option.
  B. If there are any crossed files make copies of them. Chances are all
  copies will end up fine since copy will copy only file_size bytes.
  Delete the original files.
  C. If there are crossed chains in a directory first recreate the
  directory in another subdirectory. Then delete all files
  and subdirectories inside the bad subdirectory. If the dir is
  crossed with a file treat the file as in part B.
  D. You will probably not be able to delete the bad directory
  because it is non-empty. There is no API call similar to the
  icheck -p call in unix. A very slightly modified version of
  pc_unlink() could provide this service (do not check for
  ADIRENT).
  E. If any files are reported to have the wrong size make a copy of them.
  F. run chkdsk with save lost clusters on to fix up the file system.

*/

#include <rtfs.h>

/* 10-24-2000 added LBA formatting. Submit to main tree */

typedef CLUSTERTYPE CLTYPE;

#define PATHSIZE 256


/************************************************************************
*                                                                      *
* Function prototypes                                                  *
*                                                                      *
************************************************************************/

void app_entry(void);
void print_crossed_files(void);
BOOLEAN allocate_chkdsk_core(void);
void free_chkdsk_core(void);
BOOLEAN write_lost_chains(void);
BOOLEAN build_chk_file(long bad_chain_no, long current_file_no, long *ret_file_no);
BOOLEAN scan_all_files(byte *dir_name);
BOOLEAN process_used_map(DROBJ *pobj, byte *filename);
BOOLEAN scan_crossed_files(byte *dir_name);
BOOLEAN process_crossed_file(DROBJ *pobj, byte *filename);
BOOLEAN add_cluster_to_lost_list(DDRIVE  *pdr , CLTYPE cluster);
BOOLEAN check_lost_clusters(DDRIVE  *pdr);
dword count_lost_clusters(DDRIVE *pdr);
BOOLEAN add_cluster_to_crossed(CLTYPE cluster);
CLTYPE chain_size(CLTYPE cluster);
void clr_bit(byte *bitmap, dword index);
byte get_bit(byte *bitmap, dword index);
void set_bit(byte *bitmap, dword index);

/* Provided in the vfat soecific code section */
dword scan_for_bad_lfns(DROBJ *pmom, int delete_bad_lfn);

/************************************************************************
*                                                                      *
* Programmer modifiable constants                                      *
*                                                                      *
************************************************************************/

#define NCROSSED_ALLOWED 50
/* For controlling how much to alloc. You may change it . */

#define NLOST_ALLOWED 50
/* For controlling how much to alloc. You may change it . */

#define MAX_RECURSION_DEPTH 16 /* 8 */
/* For controlling how deep you will allow the directory traversal to
go. This guards against stack overflow in systems with deeply nested
subdirectories. Each recursion chews up around 160 bytes if EMAXPATH_BYTES
is 145. Less if EMAXPATH_BYTES is smaller. */

#define CL_WINDOW_SIZE 0x10000L
/* This is the size of the largest number of clusters that can be
scanned for crossed and lost chains on a single pass:
(# of passes) = (# of clusters)/CL_WINDOW_SIZE */

#define CL_BITMAP_SIZE (int) ((CL_WINDOW_SIZE+7)/8)
/* Size of the bitmap needed to process CL_WINDOW_SIZE clusters on a
single pass */

/************************************************************************
*                                                                      *
* CHKDSK data structures                                               *
*                                                                      *
************************************************************************/

typedef struct crossed_file {
    byte  file_name[EMAXPATH_BYTES];
    struct crossed_file *pnext;
} CROSSED_FILE;

typedef struct crossing_point {
    CLTYPE cluster;
    struct crossed_file *plist;
} CROSSING_POINT;

/* The program uses a lot of global data. To keep it manageable we put
it all in one big structure. */

typedef struct chk_global {
    int be_verbose;                       /* BE_VERBOSE */
    int fix_problems;
    int write_chains;                     /* if 1 create chk files otherwise delete */
    DDRIVE *drive_structure;                /* The drive we are working on */
    dword  n_user_files;                    /* Total #user files found */
    dword  n_hidden_files;                  /* Total #hidden files found */
    dword  n_user_directories;              /* Total #directories found */
    CLTYPE n_free_clusters;                 /* # free available clusters */
    CLTYPE n_bad_clusters;                  /* # clusters marked bad */
    CLTYPE n_file_clusters;                 /* Clusters in non hidden files */
    CLTYPE n_hidden_clusters;               /* Clusters in hidden files */
    CLTYPE n_dir_clusters;                  /* Clusters in directories */
    CROSSED_FILE *crossed_file_freelist;    /* Freelist of crossed file structures */
    CROSSING_POINT crossed_points[NCROSSED_ALLOWED]; /* Array: containing cluster number and a list of
                                            files crossed at that cluster. */
    dword  n_crossed_points;                /* Number of crossed chains. */
    CLTYPE lost_chain_list[NLOST_ALLOWED];  /* Array listheads of lost chains */
    dword  n_lost_chains;                   /* # lost chains */
    dword  n_lost_clusters;                 /* # lost clusters */
    byte  bm_used[CL_BITMAP_SIZE];          /* Bitmap of all clusters used
                                            by directories/files */
            /* Use a global buffer so we do not load the stack up during recursion */
    byte gl_file_name[26];
    byte gl_file_path[EMAXPATH_BYTES];
    int recursion_depth;                  /* how deep in the stack we are */
    dword n_bad_lfns;               /* # corrupt/disjoint win95 lfn chains */

    /* These fields bound the cluster map processing so we can process
    disks with very large FATs by making multiple passes */
    CLTYPE cl_start;
    CLTYPE cl_end;
    int on_first_pass;
} CHK_GLOBAL;


/************************************************************************
*                                                                      *
* CHKDSK parameters - set parameters here if no console input is       *
*  available                                                           *
*                                                                      *
************************************************************************/

#define VERBOSE TRUE
/* Set to TRUE for verbose mode; FALSE for silent mode */



void print_chkdsk_statistics(CHK_GLOBAL *pgl);
void print_chkdsk_crossed_files(CHK_GLOBAL *pgl);

/************************************************************************
*                                                                      *
* GLOBAL DATA                                                          *
*                                                                      *
************************************************************************/

/* This is where all the globals are kept. */
CHK_GLOBAL KS_FAR gl;
CROSSED_FILE KS_FAR crossed_file_core[NCROSSED_ALLOWED]; /* Base of the list */


/************************************************************************
*                                                                       *
*                            MAIN PROGRAM                               *
*                                                                       *
************************************************************************/


BOOLEAN pc_check_disk(byte *drive_id, CHKDISK_STATS *pstat, int verbose, int fix_problems, int write_chains) /* __apifn__*/
{
    int drive_number,i;
    byte str_slash[8];
    BOOLEAN ret_val;
    byte *p;

    ret_val = FALSE;

    rtfs_memset((byte *)&gl, 0, sizeof(gl));
    gl.fix_problems = fix_problems;
    gl.write_chains = write_chains;
    gl.be_verbose   = verbose;

    /* Initialize filesystem memory */
    CHECK_MEM(BOOLEAN, 0)    /* Make sure memory is initted */

    /* Now make \\ in native char set */
    p = &str_slash[0];
    CS_OP_ASSIGN_ASCII(p,'\\');
    CS_OP_INC_PTR(p);
    CS_OP_TERM_STRING(p);

    /* Mount the disk */
    drive_number = (int) check_drive_name_mount(drive_id);
    if (drive_number < 0)
    {
        goto ex_it;
    }
    /* Release the lock. chkdsk is not atomic */
    release_drive_mount(drive_number); /* Release lock, unmount if aborted */

    gl.drive_structure = pc_drno2dr(drive_number);
    if (!pc_set_default_drive(drive_id))
    {
        goto ex_it;
    }

    /* Allocate the bit maps and data structures we will need we pass in
    the size of the fat so we know how many bits we will need to allocate
    in our used bitmap. */
    if (!allocate_chkdsk_core())
    {
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_1(USTRING_CHKDSK_02,PRFLG_NL);} /* "Failed Allocating Core To Run" */
        goto ex_it;
    }

    gl.on_first_pass = 1;
    gl.cl_start = 2;
    gl.cl_end = CL_WINDOW_SIZE + 2;
    if (gl.cl_end > gl.drive_structure->maxfindex+1)
        gl.cl_end = (CLTYPE) (gl.drive_structure->maxfindex+1);

    while (gl.cl_start < gl.drive_structure->maxfindex)
    {
        gl.n_user_files = 0;
        gl.n_hidden_files = 0;
        gl.n_user_directories = 0;

        /* Build a used map plus get statistics on cluster usage */
        if (!scan_all_files(str_slash))
        {
            if (gl.be_verbose)
            {RTFS_PRINT_STRING_1(USTRING_CHKDSK_03,PRFLG_NL);} /* "Failed Scanning Disk Files" */
            goto ex_it;
        }

        /* Now check if any allocated clusters are unaccounted for */
        if (!check_lost_clusters(gl.drive_structure))
        {
            if (gl.be_verbose)
            {RTFS_PRINT_STRING_1(USTRING_CHKDSK_04,PRFLG_NL);} /* "Failed Scanning Fat" */
            goto ex_it;
        }

        /* Advance the active cluster window */
        gl.cl_start = gl.cl_end;
        gl.cl_end += CL_WINDOW_SIZE;
        if (gl.cl_end > gl.drive_structure->maxfindex+1)
            gl.cl_end = (CLTYPE) (gl.drive_structure->maxfindex+1);
        gl.on_first_pass = 0;

        /* Clear the used cluster table */
        rtfs_memset((byte *)gl.bm_used,0,CL_BITMAP_SIZE);
    }


    /* If there are lost chains count the clusters inside. */
    if (gl.n_lost_chains)
        count_lost_clusters(gl.drive_structure);

    /* Now recover lost chains into .CHK files */
    if (gl.fix_problems && gl.n_lost_chains)
    {
        if (gl.write_chains)
        {
            if (gl.be_verbose)
            {RTFS_PRINT_STRING_1(USTRING_CHKDSK_05,PRFLG_NL);} /* "     Creating .CHK Files" */
            if (!write_lost_chains())
            {
                if (gl.be_verbose)
                {RTFS_PRINT_STRING_1(USTRING_CHKDSK_06,PRFLG_NL);} /* "     Failed Creating .CHK Files" */
                goto ex_it;
            }
        }
        else
        {
            for (i = 0; i < (long) gl.n_lost_chains; i++)
            {
                if (!FATOP(gl.drive_structure)->fatop_freechain(gl.drive_structure, gl.lost_chain_list[i], 0, 0xffffffff))
                    goto ex_it;
            }
        }
    }

    /* If there are crossed chains we have to rescan the whole file
    system looking for the files that contain he crossed chains. */
    if (gl.n_crossed_points)
    {
        if (!scan_crossed_files(str_slash))
        {
            if (gl.be_verbose)
            {RTFS_PRINT_STRING_1(USTRING_CHKDSK_08,PRFLG_NL);} /* "Failed Scanning Crossed Files" */
            goto ex_it;
        }
    }

    /* Now print the statistics */
    if (gl.be_verbose)
        print_chkdsk_statistics(&gl);

    /* print the names of files with crossed chains. */
    if (gl.n_crossed_points)
    {
        if (gl.be_verbose)
        {
            RTFS_PRINT_STRING_1(USTRING_CHKDSK_09,PRFLG_NL); /* "      Crossed Chains Were Found" */
            print_chkdsk_crossed_files(&gl);
        }
    }
    ret_val = TRUE;
    pstat->n_user_files = gl.n_user_files;
    pstat->n_hidden_files = gl.n_hidden_files;
    pstat->n_user_directories = gl.n_user_directories;
    pstat->n_free_clusters = gl.n_free_clusters;
    pstat->n_bad_clusters = gl.n_bad_clusters;
    pstat->n_file_clusters = gl.n_file_clusters;
    pstat->n_hidden_clusters = gl.n_hidden_clusters;
    pstat->n_dir_clusters = gl.n_dir_clusters;
    pstat->n_crossed_points = gl.n_crossed_points;
    pstat->n_lost_chains = gl.n_lost_chains;
    pstat->n_lost_clusters = gl.n_lost_clusters;
    pstat->n_bad_lfns = gl.n_bad_lfns;
ex_it:
    return (ret_val);
}

/************************************************************************
*                                                                      *
* User I/O functions                                                   *
*                                                                      *
************************************************************************/
void print_chkdsk_statistics(CHK_GLOBAL *pgl)
{
    dword ltemp;
    if (!gl.be_verbose)
        return;

    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
    RTFS_PRINT_LONG_1  ((dword)pgl->n_user_files, 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_11,0); /* "   user files in " */
    RTFS_PRINT_LONG_1  ((dword)pgl->n_user_directories, PRFLG_NL);

    ltemp = pgl->drive_structure->numsecs;
    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
    RTFS_PRINT_LONG_1  ((dword)(ltemp/2), 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_13, PRFLG_NL); /* "     KBytes total disk space" */

    ltemp =  (dword) pgl->n_hidden_clusters;
    ltemp *= (dword) pgl->drive_structure->secpalloc;
    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
    RTFS_PRINT_LONG_1  ((dword)(ltemp/2), 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_15, 0); /* " KBytes in " */
    RTFS_PRINT_LONG_1  ((dword)pgl->n_hidden_files, 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_16, PRFLG_NL); /* " hidden files" */



    ltemp =  (dword) pgl->n_dir_clusters;
    ltemp *= (dword) pgl->drive_structure->secpalloc;
    ltemp += (dword) pgl->drive_structure->secproot;
    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
    RTFS_PRINT_LONG_1  ((dword)(ltemp/2), 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_18, 0); /* " KBytes in " */
    RTFS_PRINT_LONG_1  ((dword)pgl->n_user_directories, 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_19, PRFLG_NL); /* " directories" */

    ltemp =  (dword) pgl->n_file_clusters;
    ltemp *= (dword) pgl->drive_structure->secpalloc;
    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
    RTFS_PRINT_LONG_1  ((dword)(ltemp/2), 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_21, 0); /* " KBytes in " */
    RTFS_PRINT_LONG_1  ((dword)pgl->n_user_files, 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_22, PRFLG_NL); /* " user files" */

    ltemp =  (dword) pgl->n_bad_clusters;
    ltemp *= (dword) pgl->drive_structure->secpalloc;
    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
    RTFS_PRINT_LONG_1  ((dword)(ltemp/2), 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_24, PRFLG_NL); /* " KBytes in bad sectors" */

    ltemp =  (dword) pgl->n_free_clusters;
    ltemp *= (dword) pgl->drive_structure->secpalloc;
    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
    RTFS_PRINT_LONG_1  ((dword)(ltemp), 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_26, PRFLG_NL); /* " Free sectors available on disk" */

    ltemp = (dword) pgl->drive_structure->secpalloc * 512;
    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, PRFLG_NL); /* "       " */
    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
    RTFS_PRINT_LONG_1  ((dword)ltemp, 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_29, PRFLG_NL); /* " Bytes Per Allocation Unit" */

    ltemp = (dword)(pgl->drive_structure->maxfindex - 1);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_30, PRFLG_NL); /* "" */
    RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
    RTFS_PRINT_LONG_1  ((dword)ltemp, 0);
    RTFS_PRINT_STRING_1(USTRING_CHKDSK_32, PRFLG_NL); /* " Total Allocation Units On Disk" */

    RTFS_PRINT_STRING_1(USTRING_SYS_NULL, PRFLG_NL); /* "" */
    if (pgl->n_lost_chains)
    {
        RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
        RTFS_PRINT_LONG_1((dword)pgl->n_lost_clusters, 0);
        RTFS_PRINT_STRING_1(USTRING_CHKDSK_35, 0); /* " lost clusters found in " */
        RTFS_PRINT_LONG_1((dword)pgl->n_lost_chains, 0);
        RTFS_PRINT_STRING_1(USTRING_CHKDSK_36, PRFLG_NL); /* " lost chains" */
    }

    if (pgl->n_bad_lfns)
    {
        RTFS_PRINT_STRING_1(USTRING_SYS_TAB, 0); /* "       " */
        RTFS_PRINT_LONG_1((dword)pgl->n_bad_lfns, 0);
        RTFS_PRINT_STRING_1(USTRING_CHKDSK_38, 0); /* "  bad long file name chains found" */
        if (pgl->fix_problems)
            RTFS_PRINT_STRING_1(USTRING_CHKDSK_39, PRFLG_NL); /* " and deleted" */
        else
            RTFS_PRINT_STRING_1(USTRING_CHKDSK_40, PRFLG_NL); /* " that were not deleted" */
    }
}

/* Print the names of all files that we were able to determine were crossed */
void print_chkdsk_crossed_files(CHK_GLOBAL *pgl)                                          /*__fn__*/
{
    int i;
    CROSSED_FILE *pcross;
    int n_printed;

    for (i = 0; i < (int) pgl->n_crossed_points; i++)
    {
        if (gl.be_verbose)
        {
        RTFS_PRINT_STRING_1(USTRING_CHKDSK_41, 0); /* "      Chains Crossed at Cluster" */
        RTFS_PRINT_LONG_1((dword) pgl->crossed_points[i].cluster, PRFLG_NL);
        }
        pcross = pgl->crossed_points[i].plist;
        n_printed = 0;
        while (pcross)
        {
            if (gl.be_verbose)
            {RTFS_PRINT_STRING_2(USTRING_CHKDSK_42, pcross->file_name, PRFLG_NL);} /* "             " */
            pcross = pcross->pnext;
            n_printed += 1;
        }
        if (gl.be_verbose)
        {
        if (!n_printed)
        {RTFS_PRINT_STRING_1(USTRING_CHKDSK_43, PRFLG_NL);} /* "       Lost Chains" */
        else if (n_printed == 1)
        {RTFS_PRINT_STRING_1(USTRING_CHKDSK_44, PRFLG_NL);} /* "       A Lost Chain" */
        }
    }
}


/************************************************************************
*                                                                      *
* Init/exit functions                                                  *
*                                                                      *
************************************************************************/

/* Allocate core for bitmaps, lost chains and crossed files. */
BOOLEAN allocate_chkdsk_core(void)                        /*__fn__*/
{
    long i;

    gl.recursion_depth = 0;

    rtfs_memset((byte *)gl.bm_used, 0, CL_BITMAP_SIZE);
    rtfs_memset((byte *)gl.crossed_points, 0, NCROSSED_ALLOWED*sizeof(CROSSING_POINT));
    rtfs_memset((byte *)gl.lost_chain_list, 0, NLOST_ALLOWED * sizeof(CLTYPE));
    /* Build a freelist we can use for crossed files */
    gl.crossed_file_freelist = crossed_file_core;
    for (i = 0; i < NCROSSED_ALLOWED-1; i++)
    {
        gl.crossed_file_freelist->pnext = gl.crossed_file_freelist+1;
        gl.crossed_file_freelist++;
    }
    gl.crossed_file_freelist->pnext = 0;
    gl.crossed_file_freelist = crossed_file_core;
    return(TRUE);
}

void free_chkdsk_core()                                             /*__fn__*/
{
}


/************************************************************************
*                                                                      *
* Functions for saving lost chains to CHK files                        *
*                                                                      *
************************************************************************/

/* Create a FILEXXXX.CHK file for each found lost chain */
BOOLEAN write_lost_chains()                                           /*__fn__*/
{
    long current_chk_file;
    long i;
    current_chk_file = 0;
    for (i = 0; i < (long) gl.n_lost_chains; i++)
    {
        if (!build_chk_file(i, current_chk_file, &current_chk_file))
            return(FALSE);
    }
    return(TRUE);
}

/* Given a lost chain number and a likely starting point for the
FILE???.CHK file create the file from the lost chain. */
BOOLEAN build_chk_file(long bad_chain_no, long current_file_no, long *ret_file_no)    /*__fn__*/
{
    long remainder;
    long temp;
    int fd;
    PC_FILE *pfile;
    DDRIVE  *pdrive;
    byte filename[13];
    byte cs_filename[26];

    for ( ; current_file_no < 999;  current_file_no++)
    {
        /* Create a file name */
        rtfs_strcpy((byte *) &filename[0], (byte *) (CS_OP_ASCII("\\FILE000.CHK")));
        temp = (long) (current_file_no/100);
        filename[5] = (byte) (CS_OP_ASCII('0') + temp);
        remainder = (long) (current_file_no - (temp * 100));
        temp = (long) (remainder/10);
        filename[6] = (byte) (CS_OP_ASCII('0') + (byte) temp);
        remainder = (long) (remainder - (temp * 10));
        filename[7] = (byte) (CS_OP_ASCII('0') + remainder);
        /* Map to the native character set */
        CS_OP_ASCII_TO_CS_STR(cs_filename, filename);
        /* Try to open it exclusive. This will fail if the file exists */
        fd = (int)po_open(cs_filename, (word)PO_CREAT|PO_EXCL|PO_WRONLY, (word)PS_IREAD|PS_IWRITE);
        if (fd >= 0)
        {
        /* Get underneath the file and set the first cluster to the
            beginning of the lost_chain. */
            /* Get the file structure and semaphore lock the drive */
            pfile = pc_fd2file(fd, FALSE);
            if (pfile)
            {
                pdrive = pfile->pobj->pdrive;
                pc_pfinode_cluster(pfile->pobj->pdrive,pfile->pobj->finode,
                    gl.lost_chain_list[bad_chain_no]);
                /* Update the size. */
                pfile->pobj->finode->fsize = chain_size(pc_finode_cluster(pfile->pobj->pdrive,pfile->pobj->finode));
                pfile->needs_flush = TRUE;
                release_drive_mount(pdrive->driveno); /* Release lock, unmount if aborted */
            }
            /* Close the file. This will write everything out. */
            po_close(fd);
            break;
        }
    }

    /* If we get here it did not work */
    if (current_file_no == 999)
        return(FALSE);

    /* Return the next ??? for FILE???.CHK that will probably work */
    *ret_file_no =  (int) (current_file_no + 1);
    return(TRUE);
}



/************************************************************************
*                                                                      *
* File/Directory Scanning                                              *
*                                                                      *
************************************************************************/

/* Scan all files/directories on the drive -
*  Mark all used clusterd in the used bit map.
*  Note any crossed cluster chains
*  Adjust any incorrect file sizes
*/


/* int scan_all_files(byte *dir_name)
*
*  This routine scans all subdirectories and does the following:
*   . calculates gl.n_user_directories
*   . calculates gl.n_hidden_files
*   . calculates gl.n_user_files
*       Then it calls process_used_map for each file in the directory and
*       for the directory itself .
*     process_used_map does the following:
*   .       calculates gl.n_dir_clusters
*   .       calculates gl.n_hidden_clusters
*   .       calculates gl.n_file_clusters
*   .       notes and if writing adjusts incorrect filesizes
*   .       finds crossed chains
*   . calls scan_for_bad_lfns
*   .
*   . scan_all_files calls itself recursively for each subdirectory it
*   . encounters.
*   .
*   .
*/

BOOLEAN scan_all_files(byte *dir_name)                                /*__fn__*/
{
    DROBJ *directory;
    DROBJ *entry;
    byte ldir_name[EMAXPATH_BYTES];

    if (gl.recursion_depth > MAX_RECURSION_DEPTH)
    {
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_2(USTRING_CHKDSK_45, dir_name,PRFLG_NL);} /* "Path too deep , directory == " */
        return(FALSE);
    }
    gl.recursion_depth += 1;


    /* Only do this on the first pass */
    if (gl.on_first_pass)
    {
        /* Find the directory again for scanning the dir for lfn errors */
        directory = pc_fndnode(dir_name);
        if (!directory)
        {
            if (gl.be_verbose)
            {RTFS_PRINT_STRING_2(USTRING_CHKDSK_46, dir_name,PRFLG_NL);} /* "Failed Scanning This Directory on LFN Pass  -" */
            return(FALSE);
        }

        /* Scan through the directory looking for bad lfn data */
        gl.n_bad_lfns += scan_for_bad_lfns(directory, gl.fix_problems);
        pc_freeobj(directory);
    }

    /* Find the directory for scanning the dir for files */
    directory = pc_fndnode(dir_name);
    if (!directory)
    {
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_2(USTRING_CHKDSK_47, dir_name,PRFLG_NL);} /* "Failed Scanning This Directory -" */
        return(FALSE);
    }
    if (gl.be_verbose && gl.on_first_pass)
    {
        RTFS_PRINT_STRING_2(USTRING_SYS_NULL,dir_name,PRFLG_NL);
    }
    if (!pc_isroot(directory))
        gl.n_user_directories += 1;

    /* Mark all of this Dirs clusters IN use, and look for crossed chains */
    if (!process_used_map(directory, dir_name))
    {
        pc_freeobj(directory);
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_2(USTRING_CHKDSK_48, dir_name,PRFLG_NL);} /* "Failed Scanning This Directory -" */
        return(FALSE);
    }

    /* Scan through the directory looking for all files */
    entry = pc_get_inode(0,directory, 0, 0, GET_INODE_STAR);
    if (entry)
    {
        do
        {
            if (!(entry->finode->fattribute & (AVOLUME | ADIRENT) ))
            {
                pc_cs_mfile((byte*)gl.gl_file_name, (byte*)entry->finode->fname,(byte *) entry->finode->fext);
                pc_mpath((byte *)gl.gl_file_path, (byte *)dir_name, (byte *)gl.gl_file_name);
                if (gl.be_verbose && gl.on_first_pass)
                {
                    RTFS_PRINT_STRING_2(USTRING_CHKDSK_49, gl.gl_file_path,PRFLG_NL); /* "    " */
                }
                if (entry->finode->fattribute & AHIDDEN)
                    gl.n_hidden_files += 1;
                else
                    gl.n_user_files += 1;
                    /* Mark all of this File s clusters IN use, check the size
                and look for crossed chains */
                if (!process_used_map(entry, gl.gl_file_path))
                {
                    pc_freeobj(entry);
                    pc_freeobj(directory);
                    if (gl.be_verbose)
                    {RTFS_PRINT_STRING_2(USTRING_CHKDSK_50, gl.gl_file_path,PRFLG_NL);} /* "Failed Scanning This File " */
                    return(FALSE);
                }
            }
        }     while (pc_get_inode(entry , directory, 0, 0, GET_INODE_STAR));
        pc_freeobj(entry);
    }
    pc_freeobj(directory);

    /* Now we call scan_all_files() for each subdirectory */
    /* Find the directory for scanning the dir for files */
    directory = pc_fndnode(dir_name);
    if (!directory)
    {
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_2(USTRING_CHKDSK_51, dir_name,PRFLG_NL);} /* "Failed Scanning This Directory -" */
        return(FALSE);
    }

    /* Scan through the directory looking for all files */
    entry = pc_get_inode(0,directory, 0, 0, GET_INODE_STAR);
    if (entry)
    {
        do
        {
            /* Scan it if it is a directory and not . or .. */
            if (entry->finode->fattribute & ADIRENT)
            {
                if ( !pc_isdot(entry->finode->fname, entry->finode->fext) &&
                    !pc_isdotdot(entry->finode->fname, entry->finode->fext) )
                {
                    pc_cs_mfile((byte *)gl.gl_file_name, (byte *)entry->finode->fname, (byte *)entry->finode->fext);
                    pc_mpath((byte *)ldir_name, (byte *)dir_name, (byte *)gl.gl_file_name);
                    if (!scan_all_files(ldir_name))
                    {
                        pc_freeobj(directory);
                        pc_freeobj(entry);
                        return(FALSE);
                    }
                }
            }
        } while (pc_get_inode(entry , directory, 0, 0, GET_INODE_STAR));
        pc_freeobj(entry);
    }
    pc_freeobj(directory);
    gl.recursion_depth -= 1;

    return (TRUE);
}


/* process_used_map(DROBJ *pobj, byte *filename)
*
*  This routine is called for each subdirectory and file in the system.
*  It traverses the chain owned by drobj and does the following with each
*  cluster in the chain:
*      . updates gl.n_dir_clusters        or ..
*      . updates gl.n_hidden_clusters     or ..
*       . updates gl.n_file_clusters
*  It then checks the bit map of already accounted for clusters, if the
*  cluster is accounted for it adds the cluster to the list of lost
*  chains by calling add_cluster_to_crossed(), if another cluster in the
*  chain was already found to be crossed we do not call add_cluster_to_crossed()
*  since it works with the whole chain.
*
*  After the chain has been traversed its size is compared with the size
*  recorded in the directory. If they differ the file is printed and if -f
*  was specified the size is adjusted.
*/

BOOLEAN process_used_map(DROBJ *pobj, byte *filename)                 /*__fn__*/
{
    CLTYPE cluster;
    CLTYPE n_clusters;
    CLTYPE proper_size;
    CLTYPE true_size;
    dword t1,t2,t3;
    BOOLEAN is_dir;
    BOOLEAN is_hidden;
    BOOLEAN found_an_intersection;

    is_hidden = FALSE;
    if (pc_isroot(pobj))
        /* FAT32 stores the root directory as a cluster chain */
    {
        if (pobj->pdrive->fasize == 8) /* FAT32 volume */
        {
            cluster = pc_sec2cluster(pobj->pdrive,pobj->pdrive->rootblock);
            is_dir = TRUE;
        }
        else
            return(TRUE);
    }
    else
    {
    /* For tracking crossed chains. We only want to note the intersection
        points of lost chains not every cluster in the chain. */

        if (pobj->finode->fattribute & ADIRENT)
            is_dir = TRUE;
        else
        {
            if (pobj->finode->fattribute & AHIDDEN)
                is_hidden = TRUE;
            else
                is_hidden = FALSE;
            is_dir = FALSE;
        }
        cluster = pc_finode_cluster(pobj->pdrive, pobj->finode);
    }
    found_an_intersection = FALSE;
    n_clusters = 0;
    /* If the incoming value is bad do not traverse */
    if ((cluster < 2) || (cluster > pobj->pdrive->maxfindex) )
        cluster = 0;
    /* 0xffffffff is gnext's end marker, 0 is error */
    while (cluster && cluster != 0xffffffff)
    {
        n_clusters += 1;
        if ( (gl.cl_start <= cluster) && (cluster < gl.cl_end) )
        {
        /* If the cluster is already in use we have a problem its
            a crossed chain */
            if ( get_bit(gl.bm_used, cluster - gl.cl_start) )
            {
                if (!found_an_intersection)
                {
                    if (!add_cluster_to_crossed(cluster))
                        return(FALSE);
                    found_an_intersection = TRUE;
                }
            }
            set_bit(gl.bm_used, cluster - gl.cl_start);
        }

        cluster = FATOP(pobj->pdrive)->fatop_clnext(pobj->pdrive, cluster);  /* Fat */
    }

    if (gl.on_first_pass)
    {
        if (is_dir)
        {
            /* To do in version 5. if (n_clusters == 0) convert directory
               to a file */
            gl.n_dir_clusters = (CLTYPE) (gl.n_dir_clusters + n_clusters);
        }
        else
        {
            if (is_hidden)
                gl.n_hidden_clusters = (CLTYPE) (gl.n_hidden_clusters + n_clusters);
            else
                gl.n_file_clusters = (CLTYPE) (gl.n_file_clusters + n_clusters);

            /* Check the file size here */
            /* Re-use the true size variable to save a little stack (we are
            inside a recursive algorithm here */
#define cl_size_minus_1 true_size
            cl_size_minus_1 = pobj->pdrive->secpalloc;
            cl_size_minus_1 <<= 9; /* *= 512 */
            cl_size_minus_1 -=1;
            /* proper_size =
            (pobj->finode->fsize + cl_size_minus_1) & ~cl_size_minus_1;
            */
            t1 = (dword) cl_size_minus_1;
            t2 = (dword) (pobj->finode->fsize + t1);
            t3 = (dword) (t2 & ~t1);
            /* This is how many bytes the file should occupy in the FAT. */
            proper_size = (CLTYPE) t3;
#undef cl_size_minus_1
            true_size = n_clusters;
            true_size <<= pobj->pdrive->log2_secpalloc;
            true_size <<= 9; /* *= 512 */
            if (proper_size != true_size)
            {
            /* Size in the directory entry does not match the size of the
                chain. If -f was specified update the file size in the directory  */
                if (gl.fix_problems)
                {
                    pobj->finode->fsize = true_size;
                    if (pobj->finode->fsize == 0)
                    { /* File is zero sized so zero the cluster pointer */
                        pc_pfinode_cluster(pobj->pdrive, pobj->finode, 0);
                    }
                    if (!pc_update_inode(pobj, TRUE, TRUE))
                    {
                        if (gl.be_verbose)
                        {RTFS_PRINT_STRING_2(USTRING_CHKDSK_52, filename,PRFLG_NL);} /* "Failed Writing This Adusted File:    " */
                        return(FALSE);
                    }
                    if (gl.be_verbose)
                    {RTFS_PRINT_STRING_2(USTRING_CHKDSK_53, filename, PRFLG_NL);} /* "Size Adusted, This File:    " */
                }
                else
                {
                    if (gl.be_verbose)
                    {RTFS_PRINT_STRING_2(USTRING_CHKDSK_54, filename,PRFLG_NL);} /* "Size Needs Adusting This File:    " */
                }
            }
        }
    } /* if (on_first_pass) */
    return(TRUE);
}


/************************************************************************
*                                                                      *
* Lost cluster fns                                                     *
*                                                                      *
************************************************************************/

/* check_lost_clusters(DDRIVE *pdr)
*
* This routine is called by (app_entry()) after scan_all_files() was called
* to produce the bm_used bitmap of clusters cliamed by files and
* sub-direcories.
* It scans the FILE allocation table. A cluster is lost if it
* is allocated in the FAT but not in the bm_used bitmap we built up while
* scanning the file system (scan_all_files()). We maintain an array of
* chain heads of lost clusters.
*/

BOOLEAN check_lost_clusters(DDRIVE  *pdr)                             /*__fn__*/
{
    CLTYPE cluster;
    CLTYPE nxt;

    for (cluster = gl.cl_start ; cluster < gl.cl_end; cluster++)
    {

        if (!FATOP(pdr)->fatop_faxx(pdr, cluster, &nxt))    /* Fat */
        {
            return(FALSE);
        }
        if (nxt != 0)
        {
            /* If we did not see the cluster during the directory scan */
            if ( !get_bit(gl.bm_used, cluster - gl.cl_start) )
            {
            /* ff(f)7 marks a bad cluster if it is not already in
                a chain. */
                if ( ((pdr->fasize == 3) && (nxt ==  0xff7)) ||
                    ((pdr->fasize == 4) && (nxt == 0xfff7))
                    || ((pdr->fasize == 8) && (nxt == 0xfffffff7ul))
                    )
                    gl.n_bad_clusters += 1;

                if ( ((pdr->fasize == 3) && ((nxt < 0xff0) || (nxt > 0xff8))) ||
                    ((pdr->fasize == 4) && ((nxt < 0xfff0) || (nxt > 0xfff8)))
                    || ((pdr->fasize == 8) && ((nxt < 0xfffffff0ul) || (nxt > 0xfffffff8ul)))
                    )
                {
                    /* And if it is not a bad or reserved cluster */
                    /* Add it to the lost list */
                    if (!add_cluster_to_lost_list(pdr, cluster))
                        return(FALSE);
                }
            }
        }
        else
            gl.n_free_clusters += 1;

    }

    return (TRUE);
}

/* int add_cluster_to_lost_list(pdrive, cluster)
*
*  This routine is called by check_lost_clusters. It takes a cluster known
*  to be lost and adds the chain which it heads to the lost chain list. If a
*  chain head in the lost chain list is a member of the chain headed by
*  cluster, it is replaced. With cluster.
*/

BOOLEAN add_cluster_to_lost_list(DDRIVE  *pdr , CLTYPE cluster)   /*__fn__*/
{
    CLTYPE first_cluster_in_chain;
    int i;
    BOOLEAN found;

    /* Save the top of the chain */
    first_cluster_in_chain = cluster;

    /* No need to check the incoming cluster argument, known good */

    found = FALSE;
    /* 0xffffffff is gnext's end marker, 0 is error */
    while (cluster && cluster != 0xffffffff)
    {
    /* See if this cluster is the head of another lost chain
    If so replace the head of that chain with the head of
        the chain we are traversing */
        if (!found)
        {
            for (i = 0; i < (int)gl.n_lost_chains; i++)
            {
                if (gl.lost_chain_list[i] == cluster)
                {
                    gl.lost_chain_list[i] = first_cluster_in_chain;
                    found = TRUE;
                    break;
                }
            }
        }
        /* Now mark the cluster used. - we do this so we do not re-process
        any of the clusters in this chain  */
        if ((gl.cl_start <= cluster) && (cluster < gl.cl_end))
        {
            set_bit(gl.bm_used, cluster - gl.cl_start);
        }

        cluster = FATOP(pdr)->fatop_clnext(pdr, cluster);   /* Fat */
    }
    /* If we did not attach the chain to a list we already had we will
    add it to the end */
    if (!found)
        gl.lost_chain_list[gl.n_lost_chains++] = first_cluster_in_chain;

    if (gl.n_lost_chains >= NLOST_ALLOWED)
    {
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_1(USTRING_CHKDSK_55,PRFLG_NL);} /* "Chkdsk Giving up. Too Many Lost Chains" */
        return(FALSE);
    }
    return(TRUE);
}

/* count_lost_clusters(DDRIVE *pdr)
*
*  This routine scans the lost chain list and tallies the total number of
*  clusters that were found.
*/

dword count_lost_clusters(DDRIVE *pdr)                              /*__fn__*/
{
    int i;
    CLTYPE cluster;

    gl.n_lost_clusters = 0;
    for (i = 0; i < (int)gl.n_lost_chains; i++)
    {
        cluster = gl.lost_chain_list[i];
        /* clnext will return zero but check range for initial value */
        if ((cluster < 2) || (cluster > pdr->maxfindex) )
            cluster = 0;
        /* 0xffffffff is gnext's end marker, 0 is error */
        while (cluster && cluster != 0xffffffff)
        {
            gl.n_lost_clusters++;
            cluster = FATOP(pdr)->fatop_clnext(pdr, cluster); /* Fat */
        }
    }
    return(gl.n_lost_clusters);
}


/************************************************************************
*                                                                      *
* Crossed chain fns                                                    *
*                                                                      *
************************************************************************/

/* scan_crossed_files()
*
*  This routine scans all files and subdirectories in the whole filesystem.
*  For each cluster it looks to see if it is in the crossed file list that
*  we generated earlier. If the cluster is found the file or subdirectory
*  is added to the list of crossed files at that cluster.
*/

BOOLEAN scan_crossed_files(byte *dir_name)                            /*__fn__*/
{
    DROBJ *directory;
    DROBJ *entry;
    byte ldir_name[EMAXPATH_BYTES];

    /* Find the directory for scanning the dir for files */
    directory = pc_fndnode(dir_name);
    if (!directory)
    {
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_2(USTRING_CHKDSK_56, dir_name, PRFLG_NL);} /* "Failed Scanning This Directory " */
        return(FALSE);
    }
    if (gl.be_verbose)
    {RTFS_PRINT_STRING_2(USTRING_SYS_NULL,dir_name, PRFLG_NL);}
    if (!pc_isroot(directory))
        gl.n_user_directories += 1;

    /* See if this directory is crossed with another */
    if (!process_crossed_file(directory, dir_name))
    {
        pc_freeobj(directory);
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_2(USTRING_CHKDSK_57, dir_name, PRFLG_NL);} /* "Failed Scanning This Directory " */
        return(FALSE);
    }

    /* Scan through the directory looking for all files */
    entry = pc_get_inode(0,directory,  0, 0, GET_INODE_STAR);
    if (entry)
    {
        do
        {
            if (!(entry->finode->fattribute & (AVOLUME | ADIRENT) ))
            {
                pc_cs_mfile((byte *)gl.gl_file_name, (byte *)entry->finode->fname, (byte *)entry->finode->fext);
                pc_mpath((byte *)gl.gl_file_path, (byte *)dir_name, (byte *)gl.gl_file_name);
                if (gl.be_verbose)
                {RTFS_PRINT_STRING_2(USTRING_CHKDSK_58, gl.gl_file_path, PRFLG_NL);} /* "    " */
                /* See if this file is crossed with another */
                if (!process_crossed_file(entry, gl.gl_file_path))
                {
                    pc_freeobj(entry);
                    pc_freeobj(directory);
                    if (gl.be_verbose)
                    {RTFS_PRINT_STRING_2(USTRING_CHKDSK_59, gl.gl_file_path,PRFLG_NL);} /* "Failed Scanning This File " */
                    return(FALSE);
                }

            }
        }     while (pc_get_inode(entry , directory, 0, 0, GET_INODE_STAR));

        pc_freeobj(entry);
    }
    pc_freeobj(directory);


    /* Now we call scan_crossed_files() for each subdirectory */
    /* Find the directory for scanning the dir for files */
    directory = pc_fndnode(dir_name);
    if (!directory)
    {
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_2(USTRING_CHKDSK_60, dir_name, PRFLG_NL);} /* "Failed Scanning This Directory " */
        return(FALSE);
    }

    /* Scan through the directory looking for all files */
    entry = pc_get_inode(0,directory, 0, 0, GET_INODE_STAR);
    if (entry)
    {
        do
        {
            /* Scan it if it is a directory and not "." or ".." */
            if (entry->finode->fattribute & ADIRENT)
            {
                if ( !pc_isdot(entry->finode->fname, entry->finode->fext) &&
                    !pc_isdotdot(entry->finode->fname, entry->finode->fext) )
                {
                    pc_cs_mfile((byte *)gl.gl_file_name, (byte *)entry->finode->fname, (byte *)entry->finode->fext);
                    pc_mpath((byte *)ldir_name, (byte *)dir_name, (byte *)gl.gl_file_name);
                    if (!scan_crossed_files(ldir_name))
                    {
                        pc_freeobj(directory);
                        pc_freeobj(entry);
                        return(FALSE);
                    }
                }
            }
        } while (pc_get_inode(entry , directory, 0, 0, GET_INODE_STAR));
        pc_freeobj(entry);
    }
    pc_freeobj(directory);
    return (TRUE);
}


/* Given the file or directory at pobj see if any of its clusters are
crossed with another file or directory. */
BOOLEAN process_crossed_file(DROBJ *pobj, byte *filename)                         /*__fn__*/
{
    CLTYPE cluster;
    int i;
    CROSSED_FILE *pcross;

    if (pc_isroot(pobj))
        return(TRUE);

    cluster = pc_finode_cluster(pobj->pdrive, pobj->finode);
    /* If the incoming value is bad do not traverse */
    if ((cluster < 2) || (cluster > pobj->pdrive->maxfindex) )
        cluster = 0;
    /* 0xffffffff is gnext's end marker, 0 is error */
    while (cluster && cluster != 0xffffffff)
    {
        if ((cluster < 2) || (cluster > pobj->pdrive->maxfindex) )
            break;
        for (i = 0; i < (int)gl.n_crossed_points; i++)
        {
            /* If this cluster intersects with another chain. */
            if (gl.crossed_points[i].cluster == cluster)
            {
                pcross = gl.crossed_points[i].plist;
                while (pcross)
                {
                /* If already in the list for this cluster continue
                    processing the next cluster. */
                    if (rtfs_cs_strcmp(pcross->file_name, filename) == 0)
                        break;
                    pcross = pcross->pnext;
                }
                if (!pcross)
                {
                /* Add this file/dir name to the list of files crossed
                    at this point */
                    if (!gl.crossed_file_freelist)
                    {
                        if (gl.be_verbose)
                        {RTFS_PRINT_STRING_1(USTRING_CHKDSK_61,PRFLG_NL);} /* "Chkdsk gives up, too many crossed Files" */
                        return(FALSE);
                    }
                    else
                    {
                    /* Add the file to the beginning of the list of files
                        crossed at this cluster */
                        pcross = gl.crossed_file_freelist;
                        gl.crossed_file_freelist = pcross->pnext;
                        pcross->pnext = gl.crossed_points[i].plist;
                        gl.crossed_points[i].plist = pcross;
                        rtfs_cs_strcpy(pcross->file_name, filename);
                    }
                }
            }
        }
        cluster = FATOP(pobj->pdrive)->fatop_clnext(pobj->pdrive, cluster);  /* Fat */
    }
    return(TRUE);
}

/* add_cluster_to_crossed(cluster)
*
*  This routine is called by process_used_map if it detects a crossed
*  chain (by finding a cluster that is already marked in the used map)
*  It scans the list of crossed clusters. If cluster is not already in
*  the list it is added. If we exceed the predefined value NCROSSED_ALLOWED
*  it returns FALSE.
*/

BOOLEAN add_cluster_to_crossed(CLTYPE cluster)                    /*__fn__*/
{
    int i;

    if (gl.n_crossed_points >=  NCROSSED_ALLOWED)
    {
        if (gl.be_verbose)
        {RTFS_PRINT_STRING_1(USTRING_CHKDSK_62,PRFLG_NL);} /* "Chkdsk gives up, Too many crossed chains" */
        return(FALSE);
    }

    for (i = 0; i < (int) gl.n_crossed_points; i++)
    {
        if (gl.crossed_points[i].cluster == cluster)
            return(TRUE);
    }
    gl.crossed_points[gl.n_crossed_points].cluster = cluster;
    gl.n_crossed_points += 1;
    return(TRUE);
}


/************************************************************************
*                                                                      *
* Utility Functions                                                    *
*                                                                      *
************************************************************************/

/* chain_size (CLTYPE cluster)
*
*  Calculate (return) a chain s size in bytes
*
*  Called by: build_chk_file
*/

CLTYPE chain_size(CLTYPE cluster)                           /*__fn__*/
{
    CLTYPE n_clusters;

    n_clusters = 0;
    /* If the incoming value is bad do not traverse */
    if ((cluster < 2) || (cluster > gl.drive_structure->maxfindex))
        cluster = 0;
    /* 0xffffffff is gnext's end marker, 0 is error */
    while (cluster && cluster != 0xffffffff)
    {
        n_clusters = (CLTYPE) (n_clusters + 1);
        cluster = FATOP(gl.drive_structure)->fatop_clnext(gl.drive_structure, cluster);  /* Fat */
    }
    n_clusters = (CLTYPE) (n_clusters * gl.drive_structure->secpalloc);
    n_clusters *= (CLTYPE) 512;
    return(n_clusters);
}

/* clr_bit (byte *bitmap, dword index)
*
*  Clear the bit at bitmap[index]
*/

void clr_bit(byte *bitmap, dword index)
{
    bitmap[(dword) (index >> 3)] &= ~(1 << (index & 0x7));
}

/* get_bit (byte *bitmap, dword index)
*
*  Return the bit at bitmap[index] (assuming bitmap is a bit array)
*/

byte get_bit(byte *bitmap, dword index)
{
    return ((byte)(bitmap[(dword) (index >> 3)] & (1 << (index & 0x7))));
}

/* set_bit (byte *bitmap, dword index)
*
*  Set the bit at bitmap[index] to 1
*/

void set_bit(byte *bitmap, dword index)
{
    bitmap[(dword) (index >> 3)] |= (1 << (index & 0x7));
}
