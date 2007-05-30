/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APIINFO.C - Contains user api level source code.

    The following routines are included:

    pc_set_default_drive - Set the default drive number.
    pc_free         - Calculate and return the free space on a disk.
    pc_isdir        -   Determine if a path is a directory.
    pc_isvol        -   Determine if a path is a volume
    pc_get_attributes - Get File Attributes  
    pc_getdfltdrvno - Get the default drive number.
*/

#include <rtfs.h>

/***************************************************************************
    PC_SET_DEFAULT_DRIVE - Set the current default drive.

 Description
    Use this function to set the current default drive that will be used 
    when a path specifier does not contain a drive specifier.
    Note: The default default is zero (drive A:)


 Returns
    Return FALSE if the drive is out of range.

    errno is set to one of the following
     0                - No error
     PEINVALIDDRIVEID - Driveno is incorrect
****************************************************************************/

/* Set the currently stored default drive       */
BOOLEAN pc_set_default_drive(byte *drive)        /*__apifn__*/
{
int drive_no;

    rtfs_set_errno(0);  /* pc_set_default_drive: clear error status */
    /* get drive no   */
    drive_no = pc_parse_raw_drive(drive);
    if ( ( drive_no < 0) || !pc_validate_driveno(drive_no))
    {
        rtfs_set_errno(PEINVALIDDRIVEID);/* pc_set_default_drive: invalid argument */
        return(FALSE);
    }
    else
    {
        rtfs_get_system_user()->dfltdrv_set = 1;
        rtfs_get_system_user()->dfltdrv = drive_no;
        return(TRUE);
    }
}

/****************************************************************************
    PC_FREE - Count the number of free bytes remaining on a disk

 Description
    Given a path containing a valid drive specifier count the number 
    of free bytes on the drive. The function also takes two additional 
    argument that point to location that will be loaded with the 
    total number of blocks on the drive and the total number of 
    free clusters


 Returns
    The number of free bytes or zero if the drive is full, -1 if not open,
    or out of range.

    dword *blocks_total - Contains the total block count
    dword *blocks_free  - Contain the total count of free blocks.

    errno is set to one of the following
    0                - No error
    PEINVALIDDRIVEID - Driveno is incorrect
    An ERTFS system error 
*****************************************************************************/

/* Return # free bytes on a drive   */
long pc_free(byte *path, dword *blocks_total, dword *blocks_free)  /*__apifn__*/
{
    int driveno;
    DDRIVE  *pdr;
    long bytes_free;
    CHECK_MEM(long, 0)  /* Make sure memory is initted */


    rtfs_set_errno(0);  /* po_free: clear error status */

    /* assume failure to start   */
    bytes_free = -1;
    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(path);
    /* if error check_drive errno was set by check_drive */
    if (driveno >= 0)
    {
        pdr = pc_drno2dr(driveno);
        if (!pdr)
        {
            rtfs_set_errno(PEINVALIDDRIVEID); /* pc_free: no valid drive present */
            bytes_free = -1;
        }
        else
        {
            *blocks_free = pdr->known_free_clusters;
            *blocks_free *= pdr->secpalloc;          /* Size of each fat */
            *blocks_total = pdr->maxfindex - 1;      /* Number of entries in the fat */
            *blocks_total *= pdr->secpalloc ;        /* Size of each fat */
            bytes_free = *blocks_free * 512;
        }
        release_drive_mount(driveno);/* Release lock, unmount if aborted */
    }
    return(bytes_free);
}

/*****************************************************************************
    PC_ISDIR - Test if a path name is a directory 
                        
 Description
    Test to see if a path specification ends at a subdirectory or a
    file.
    Note: \ is a directory.

 Returns
    Returns TRUE if it is a directory.
****************************************************************************/


#define ISDIR 1
#define ISVOL 2

BOOLEAN pc_is(int op, byte *path)                                   /*__fn__*/
{
    DROBJ  *pobj;
    BOOLEAN ret_val = FALSE;
    int driveno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */
    
    rtfs_set_errno(0); /* pc_isdir/pc_isvol: clear errno */

    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(path);
    /* if check_drive failed errno was set by check_drive */
    if (driveno >= 0)
    {
        pobj = pc_fndnode(path);
        /* pc_isdir/pc_isvol: if pc_fndnode fails it will set errno */
        if (pobj)
        {
            if (op == ISDIR)
                ret_val = pc_isadir(pobj);
            else if (op == ISVOL)
                ret_val = pc_isavol(pobj);
            pc_freeobj(pobj);
        }
        release_drive_mount(driveno);/* Release lock, unmount if aborted */
    }
    return (ret_val);
}
/*****************************************************************************
    PC_ISDIR - Test if a path name is a directory 
                        
 Description
    Test to see if a path specification ends at a subdirectory.

 Returns
    Returns TRUE if it is a directory.

    errno is set to one of the following
    0                - No error
    PENOENT          - Path not found
    An ERTFS system error 
****************************************************************************/


BOOLEAN pc_isdir(byte *path) /*__apifn__*/
{
    return(pc_is(ISDIR, path));
}

/*****************************************************************************
    PC_ISVOL - Test if a path name is a volume entry
                        
 Description
    Test to see if a path specification ends at a volume label

 Returns
    Returns TRUE if it is a volume

    errno is set to one of the following
    0                - No error
    PENOENT          - Path not found
    An ERTFS system error 
****************************************************************************/

BOOLEAN pc_isvol(byte *path)  /*__apifn__*/
{
    return(pc_is(ISVOL, path));
}

/*****************************************************************************
    pc_get_attributes - Get File Attributes  
                        
 Description
    Given a file or directory name return the directory entry attributes 
    associated with the entry.
    
    The following values are returned:

    BIT Nemonic
    0       ARDONLY
    1       AHIDDEN
    2       ASYSTEM
    3       AVOLUME 
    4       ADIRENT
    5       ARCHIVE
    6-7 Reserved

 Returns
    Returns TRUE if successful otherwise it returns FALSE
    
    errno is set to one of the following
    0                - No error
    PENOENT          - Path not found
    An ERTFS system error 
****************************************************************************/

BOOLEAN pc_get_attributes(byte *path, byte *p_return)           /*__apifn__*/
{
    DROBJ  *pobj;
    BOOLEAN ret_val = FALSE;
       int driveno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    rtfs_set_errno(0); /* pc_get_attributes: clear errno */

    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(path);
    if (driveno < 0)
    {
        /* pc_get_attributes: if check_drive failed errno was set by check_drive */
        return (FALSE);
    }   
    pobj = pc_fndnode(path);
    /* if pc_fndnode fails it will set errno */
    if (pobj)
    {
        *p_return = pobj->finode->fattribute;
        pc_freeobj(pobj);
        ret_val = TRUE;
    }
    release_drive_mount(driveno);/* Release lock, unmount if aborted */
    return (ret_val);
}

/***************************************************************************
    PC_GETDFLTDRVNO - Return the current default drive.

 Description
    Use this function to get the current default drive when a path specifier
    does not contain a drive specifier.

    see also pc_setdfltdrvno()

 Returns
    Return the current default drive.

    pc_getdfltdrvno() does not set errno
*****************************************************************************/

/* Return the currently stored default drive    */
int pc_getdfltdrvno(void)                                       /*__apifn__*/
{
    CHECK_MEM(int, 0)   /* Make sure memory is initted */
    if (!rtfs_get_system_user()->dfltdrv_set)
        return(prtfs_cfg->default_drive_id);
    else
        return(rtfs_get_system_user()->dfltdrv);
}

