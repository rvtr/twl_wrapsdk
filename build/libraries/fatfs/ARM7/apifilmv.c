/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APIFILEMV.C - Contains user api level source code.

    The following routines are included:

    pc_mv           - Rename a file.
    pc_unlink       - Delete a file.
*/

#include <rtfs.h>

/***************************************************************************
    PC_MV -  Rename a file.

 Description
    Renames the file in path (name) to newname. Fails if name is invalid,
    newname already exists or path not found.

    01-07-99 - Rewrote to support moving files between subdirectories
               no longer supports renaming subdirectories or volumes
 Returns
    Returns TRUE if the file was renamed. Or no if the name not found.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid or they are not the same
    PEINVALIDPATH   - Path specified by old_name or new_name is badly formed.
    PEACCESS        - File or directory in use, or old_name is read only
    PEEXIST         - new_name already exists
    An ERTFS system error
***************************************************************************/

/* Rename a file */
BOOLEAN pc_mv(byte *old_name, byte *new_name)  /*__apifn__*/
{
    int old_driveno;
    DROBJ *old_obj;
    DROBJ *old_parent_obj;
    byte  *path;
    byte  *filename;
    byte fileext[4];
    int new_driveno;
    DROBJ *new_obj;
    DROBJ *new_parent_obj;
    BOOLEAN ret_val;
    CLUSTERTYPE  cluster;
    DDRIVE *pdrive;

    int  p_errno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */
    p_errno = 0;

    /* Drives must be the same */
    if (    !pc_parsedrive( &old_driveno, old_name ) ||
            !pc_parsedrive( &new_driveno, new_name ) ||
            old_driveno != new_driveno)
    {
        rtfs_set_errno(PEINVALIDDRIVEID);
        return(FALSE);
    }

    /* Get the drive and make sure it is mounted   */
    old_driveno = check_drive_name_mount(old_name);
    if (old_driveno < 0)
    {
        /* errno was set by check_drive */
        return(FALSE);
    }
    rtfs_set_errno(0);  /* pc_mv: clear error status */

    old_obj         = 0;
    old_parent_obj  = 0;
    new_obj         = 0;
    new_parent_obj  = 0;
    ret_val = FALSE;

    pdrive = pc_drno2dr(old_driveno);

    /* Use the buffers in the DRIVE structure. Access is locked via semaphore */
    path = (byte *)&(pdrive->pathname_buffer[0]);
    filename = (byte *)&(pdrive->filename_buffer[0]);

    /* Get out the filename and d:parent */
    if (!pc_parsepath(path, filename,fileext,old_name))
    {
        p_errno = PEINVALIDPATH;
        goto errex;
    }
    /* Find the parent and make sure it is a directory  */
    old_parent_obj = pc_fndnode(path);
    if (!old_parent_obj)
        goto errex; /* pc_fndinode - set errno */

    if (!pc_isadir(old_parent_obj))
    {
        p_errno = PENOENT;
        goto errex;
    }
    /* Find the file */
    old_obj = pc_get_inode(0, old_parent_obj,
        filename, (byte*)fileext, GET_INODE_MATCH);
    if (!old_obj)
        goto errex; /* pc_get_inode - set errno */

     /* Be sure it exists and is a normal directory or file and is not open */
    if (pc_isroot(old_obj) || (old_obj->finode->opencount > 1) ||
       (old_obj->finode->fattribute&(ARDONLY|AVOLUME)))
    {
        p_errno = PEACCES;
        goto errex;
    }

    /* At this point old_obj contains the file we are renaming */

    /* See if the new directory entry already exists */
    new_obj = pc_fndnode(new_name);
    if (new_obj)
    {
        p_errno = PEEXIST;
        goto errex;
    }
    rtfs_set_errno(0);  /* pc_mv - clear errno condition after failed pc_fndnode */

    /* Get out the filename and d:parent */
    if (!pc_parsepath(path,filename,fileext,new_name))
    {
        p_errno = PEINVALIDPATH;
        goto errex;
    }
    /* Find the parent and make sure it is a directory  */
    new_parent_obj = pc_fndnode(path);
    if (!new_parent_obj || !pc_isadir(new_parent_obj) ||  pc_isavol(new_parent_obj))
        goto errex;


    /* The cluster value old */
    cluster = pc_finode_cluster(old_obj->pdrive,old_obj->finode);

    /* Create the new entry and assign cluster to it. If it is a directory
       .. will be linked correctly */
    new_obj = pc_mknode( new_parent_obj, filename, fileext, old_obj->finode->fattribute, cluster);
    if (!new_obj)
        goto errex;

    /* Copy the old directory entry stuf over */
    new_obj->finode->fattribute = old_obj->finode->fattribute;
    new_obj->finode->ftime = old_obj->finode->ftime;
    new_obj->finode->fdate = old_obj->finode->fdate;
    new_obj->finode->fsize = old_obj->finode->fsize;

    /* Update the new inode. Do not set archive bit or change date */
     if (!pc_update_inode(new_obj, FALSE, FALSE))
        goto errex;

    /* Set the old cluster value to zero */
    pc_pfinode_cluster(old_obj->pdrive,old_obj->finode,0);
    /* Delete the old but won't delete any clusters */
    if (!pc_rmnode(old_obj))
        goto errex;

    p_errno = 0;
    ret_val = TRUE;

    /* Good conditions fall through here, error exits jump to here */
errex:
    if (old_parent_obj)
        pc_freeobj(old_parent_obj);
    if (old_obj)
        pc_freeobj(old_obj);
    if (new_parent_obj)
        pc_freeobj(new_parent_obj);
    if (new_obj)
        pc_freeobj(new_obj);
    /* Set errno if we have one and not set by lower level already */
    if ((p_errno) && !get_errno())
        rtfs_set_errno(p_errno);
    if (!release_drive_mount_write(old_driveno))/* Release lock, unmount if aborted */
        ret_val = FALSE;
    return(ret_val);
}


/****************************************************************************
    PC_UNLINK - Delete a file.

 Description
    Delete the file in name. Fail if not a simple file,if it is open,
    does not exist or is read only.

 Returns
    Returns TRUE if it successfully deleted the file.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PEINVALIDPATH   - Path specified badly formed.
    PENOENT         - Can't find file to delete
    PEACCESS        - File in use, is read only or is not a simple file.
    An ERTFS system error
***************************************************************************/

/* Delete a file   */
BOOLEAN pc_unlink(byte *name)       /*__apifn__*/
{
    DROBJ *pobj;
    DROBJ *parent_obj;
    BOOLEAN ret_val;
    DDRIVE *pdrive;
    byte  *path;
    byte  *filename;
    byte  fileext[4];
    int driveno;
    int p_errno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    ret_val = FALSE;
    parent_obj  = 0;
    pobj        = 0;
    p_errno = 0;
    rtfs_set_errno(0);  /* pc_unlink: clear error status */

    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(name);
    if (driveno < 0)
    {
        /* errno was set by check_drive */
        return(FALSE);
    }

    pdrive = pc_drno2dr(driveno);
    /* Use the buffers in the DRIVE structure. Access is locked via semaphore */
    path = (byte *)&(pdrive->pathname_buffer[0]);
    filename = (byte *)&(pdrive->filename_buffer[0]);

    /* Get out the filename and d:parent   */
    if (!pc_parsepath(path, filename,fileext,name))
    {
        p_errno = PEINVALIDPATH;
        goto errex;
    }

    /* Find the parent and make sure it is a directory    */
    parent_obj = pc_fndnode(path);
    if (!parent_obj)
        goto errex;     /* pc_fndnode set errno */
    if (!pc_isadir(parent_obj) ||  pc_isavol(parent_obj))
    {
        p_errno = PEACCES;
        goto errex;
    }

    /* Find the file   */
    pobj = pc_get_inode(0, parent_obj, filename, (byte*)fileext, GET_INODE_MATCH);
    /* if pc_get_inode() fails it sets errno to PENOENT or to an internal or IO error status */
    if (pobj)
    {
        /* Be sure it is not the root. Since the root is an abstraction
            we can not delete it plus Check access permissions */
        if (   pc_isroot(pobj) || (pobj->finode->opencount > 1) ||
            (pobj->finode->fattribute&(ARDONLY|AVOLUME|ADIRENT)))
        {
            p_errno = PEACCES;
            ret_val = FALSE;
            goto errex;
        }
        else
        {   /* pc_rmnode sets errno */
            ret_val = pc_rmnode(pobj);
        }
    }

errex:
    if (pobj)
        pc_freeobj(pobj);
    if (parent_obj)
    {
        pc_freeobj(parent_obj);
    }
    /* Set errno if we have one and not set by lower level already */
    if ((p_errno) && !get_errno())
        rtfs_set_errno(p_errno);
    if (!release_drive_mount_write(driveno))/* Release lock, unmount if aborted */
        ret_val = FALSE;
    return(ret_val);
}
