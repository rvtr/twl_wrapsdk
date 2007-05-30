/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APISTAT.C - Contains user api level source code.

    The following routines are included:

    pc_fstat        -  Obtain statistics on an open file
    pc_stat         -  Obtain statistics on a path.

*/

#include <rtfs.h>

void pc_finode_stat(FINODE *pi, ERTFS_STAT *pstat);                /*__fn__*/

/****************************************************************************
    PS_FSTAT  -  Obtain statistics on an open file
    
 Description
    Fills in the stat buffer for information about an open file.

    See pc_stat for a description of the stat buffer.
    

 Returns
    Returns 0 if all went well otherwise it returns -1

    errno is set to one of the following
    0               - No error
    PEBADF          - Invalid file descriptor
****************************************************************************/

int pc_fstat(PCFD fd, ERTFS_STAT *pstat)                              /*__apifn__*/
{
PC_FILE *pfile;
CHECK_MEM(int, -1)  /* Make sure memory is initted */

    rtfs_set_errno(0);  /* pc_fstat: clear error status */

    /* Get the file structure and semaphore lock the drive */
    pfile = pc_fd2file(fd, 0);
    if (!pfile)
    { /* fd2file set errno */
        return(-1);
    }
    else
    {
        /* cal pc_finode_stat() to update the stat structure   */
        pc_finode_stat(pfile->pobj->finode, pstat);
        release_drive_mount(pfile->pobj->pdrive->driveno);/* Release lock, unmount if aborted */
        return(0);
    }
}

/****************************************************************************
    PC_STAT  -  Obtain statistics on a path.
    
 Description
    This routine searches for the file or directory provided in the first 
    argument. If found it fills in the stat structure as described here.
    
    st_dev  -   The entry s drive number
    st_mode;        
        S_IFMT  type of file mask 
        S_IFCHR character special (unused) 
        S_IFDIR directory 
        S_IFBLK block special   (unused) 
        S_IFREG regular         (a file)  
        S_IWRITE    Write permitted
        S_IREAD Read permitted.
    st_rdev -   The entry s drive number
    st_size -   file size
    st_atime    -   creation date in DATESTR format
    st_mtime    -   creation date in DATESTR format
    st_ctime    -   creation date in DATESTR format
    t_blksize   -   optimal blocksize for I/O (cluster size)
    t_blocks    -   blocks allocated for file 
    fattributes -   The DOS attributes. This is non-standard but supplied 
                    if you want to look at them


 Returns
    Returns 0 if all went well otherwise it returns -1.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PENOENT         - File or directory not found
    An ERTFS system error 
****************************************************************************/

int pc_stat(byte *name, ERTFS_STAT *pstat)                            /*__apifn__*/
{
    DROBJ *pobj;
    int driveno;
    int ret_val;
    CHECK_MEM(int, -1)  /* Make sure memory is initted */

    rtfs_set_errno(0);  /* pc_stat: clear error status */
    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(name);
    if (driveno < 0)
    {   /* errno was set by check_drive */
        return(-1);
    }

    /* pc_fndnode will set errno */
    pobj = pc_fndnode(name);
    if (pobj)
    {       
        if (pobj->isroot)
        {
            pstat->st_rdev  = 
            pstat->st_dev   = pobj->finode->my_drive->driveno;
            pstat->st_ino   = 0;
            pstat->fattribute = ADIRENT; 
            pstat->st_mode = S_IFDIR;
            pstat->st_nlink  = 1;            /* (always 1) */
            pstat->st_size  = 0;     /* file size, in bytes */
            pstat->st_atime.date  =  pstat->st_atime.time  = 0;
            pstat->st_mtime  = pstat->st_atime;         /* last modification */
            pstat->st_ctime  = pstat->st_atime;         /* last status change */
            pstat->st_blksize = (dword) pobj->finode->my_drive->bytespcluster;
            pstat->st_blocks  =  (dword) ((pobj->finode->fsize + 511)>>9);
        }
        else
        {
            /* cal pc_finode_stat() to update the stat structure   */
            pc_finode_stat(pobj->finode, pstat);
        }
        ret_val = 0;
    }
    else
        ret_val = -1;
    if (pobj)
        pc_freeobj(pobj);
    release_drive_mount(driveno);/* Release lock, unmount if aborted */

    return(ret_val);
}

/****************************************************************************
    PC_FINODE_STAT - Convert finode information to stat info for stat and fstat

 Description
    Given a pointer to a FINODE and a ERTFS_STAT structure
    load ERTFS_STAT with filesize, date of modification et al. Interpret
    the fattributes field of the finode to fill in the st_mode field of the
    the stat structure.

 Returns
    Nothing


****************************************************************************/

void pc_finode_stat(FINODE *pi, ERTFS_STAT *pstat)                /*__fn__*/
{
    pstat->st_dev   = pi->my_drive->driveno;    /* (drive number, rtfs) */
    pstat->st_ino   = 0;                        /* inode number (0) */
    pstat->st_mode  = 0;                        /* (see S_xxxx below) */

    /* Store away the DOS file attributes in case someone needs them   */
    pstat->fattribute = pi->fattribute;
    pstat->st_mode |= S_IREAD;
    if(!(pstat->fattribute & ARDONLY))
        pstat->st_mode |= S_IWRITE;
    if (pstat->fattribute & ADIRENT)
        pstat->st_mode |= S_IFDIR;
    if (!(pstat->fattribute & (AVOLUME|ADIRENT)))
        pstat->st_mode |= S_IFREG;

    pstat->st_nlink  = 1;                       /* (always 1) */
    pstat->st_rdev  = pstat->st_dev;            /* (drive number, rtfs) */
    pstat->st_size  = pi->fsize;                /* file size, in bytes */

    pstat->st_atime.date  = pi->fdate;          /* last access  */
    pstat->st_atime.time  = pi->ftime;
    pstat->st_mtime  = pstat->st_atime;         /* last modification */
    pstat->st_ctime  = pstat->st_atime;         /* last status change */
    /* optimal buffering size. is a cluster   */
    pstat->st_blksize = (dword) pi->my_drive->bytespcluster;
    /* blocks is file size / 512. with round up   */
    pstat->st_blocks  =  (dword) ((pi->fsize + 511)>>9);
}



