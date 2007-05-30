/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APIMKDIR.C - Contains user api level source code.

    The following routines are included:

    pc_mkdir        - Create a directory.
    pc_rmdir        - Delete a directory.

*/

#include <rtfs.h>

/***************************************************************************
    PC_MKDIR    -  Create a directory.

Description
    Create a sudirectory in the path specified by name. Fails if a
    file or directory of the same name already exists or if the path
    is not found.


Returns
    Returns TRUE if it was able to create the directory, otherwise
    it returns FALSE.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PEINVALIDPATH   - Path specified badly formed.
    PENOENT         - Path to new directory not found
    PEEXIST         - File or directory of this name already exists
    An ERTFS system error
****************************************************************************/

BOOLEAN pc_mkdir(byte  *name)    /*__apifn__*/
{
    DROBJ *pobj;
    DROBJ *parent_obj;
    byte  *path;
    byte  *filename;
    byte  fileext[4];
    BOOLEAN  ret_val;
    int driveno;
    DDRIVE *pdrive;
    int p_errno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    parent_obj = 0;
    pobj = 0;
    p_errno = 0;
    ret_val = FALSE;
    rtfs_set_errno(0);  /* pc_mkdir: clear error status */

    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(name);
    if (driveno < 0)
    {
        /*  errno was set by check_drive */
        return(FALSE);
    }

    pdrive = pc_drno2dr(driveno);

    /* Use the buffers in the DRIVE structure. Access is locked via semaphore */
    path = (byte *)&(pdrive->pathname_buffer[0]);
    filename = (byte *)&(pdrive->filename_buffer[0]);

    /* Get out the filename and d:parent   */
    if (!pc_parsepath(path,filename,fileext,name))
    {
        p_errno = PEINVALIDPATH;
        goto errex;
    }
    /* Find the parent and make sure it is a directory \   */
    /* pc_fndnode set errno */
    parent_obj = pc_fndnode(path);
    if (!parent_obj)
        goto errex;

    /* Lock the parent      */
    if (!pc_isadir(parent_obj) ||  pc_isavol(parent_obj))
    {
        p_errno = PENOENT;      /* Path is not a directory */
        goto errex;
    }
    /* Fail if the directory exists   */
    pobj = pc_get_inode(0, parent_obj, filename,(byte*) fileext, GET_INODE_MATCH);
    if (pobj)
    {
        p_errno = PEEXIST;      /* Exclusive fail */
        goto errex;
    }
    else
    {
        if (get_errno() != PENOENT) /* If pobj is NULL we abort on abnormal errors */
            goto errex;
        rtfs_set_errno(0);  /* pc_mkdir: clear PENOENT error status */
        pobj = pc_mknode( parent_obj,filename, fileext, ADIRENT, 0);
        if (pobj)
        {
            p_errno = 0;
            ret_val = TRUE;
        }
        else
        {
            /* pc_mknode has set errno */
            goto errex;
        }
    }

errex:
    if (pobj)
        pc_freeobj(pobj);
    if (parent_obj)
    {
        pc_freeobj(parent_obj);
    }
    if (p_errno)
        rtfs_set_errno(p_errno);
    
    /*---------- ctr modified ----------*/
    if( pdrive) {
        if( pdrive->dev_table_perform_device_ioctl) {
            pdrive->dev_table_perform_device_ioctl( driveno,
                                                    DEVCTL_FLUSH,
                                                    NULL);
        }
    }
    /*----------------------------------*/
    
    if (!release_drive_mount_write(driveno))/* Release lock, unmount if aborted */
        ret_val = FALSE;
    return(ret_val);
}


/****************************************************************************
    PC_RMDIR - Delete a directory.

Description

    Delete the directory specified in name. Fail if name is not a directory,
    is read only or contains more than the entries . and ..

 Returns
    Returns TRUE if the directory was successfully removed.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PEINVALIDPATH   - Path specified badly formed.
    PENOENT         - Directory not found
    PEACCESS        - Directory is in use or is read only
    An ERTFS system error
*****************************************************************************/

/* Remove a directory   */
BOOLEAN pc_rmdir(byte  *name) /*__apifn__*/
{
    DROBJ *parent_obj;
    DROBJ *pobj;
    DROBJ *pchild;
    BOOLEAN ret_val;
    byte  *path;
    byte  *filename;
    byte  fileext[4];
    int driveno;
    DDRIVE *pdrive;
    int p_errno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    parent_obj = 0;
    pchild = 0;
    pobj = 0;

    rtfs_set_errno(0);  /* pc_rmdir: clear error status */
    ret_val = FALSE;
    p_errno = 0;

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
    if (!pc_parsepath(path,filename,fileext,name))
    {
        p_errno = PEINVALIDPATH;
        goto errex;
    }
    /* Don't allow removal of . or .. PEACCES (by definition we are busy */
    if (pc_isdot(filename,fileext) || pc_isdotdot(filename,fileext))
    {
        p_errno = PEACCES;
        goto errex;
    }
    /* Find the parent and make sure it is a directory \   */
    parent_obj = pc_fndnode(path);
    if (!parent_obj)
        goto errex; /* pc_fndnode set errno */

    if (!pc_isadir(parent_obj))
    {
        p_errno = PEACCES;
        goto errex;
    }

    /* Find the file and init the structure   */
    pobj = pc_get_inode(0, parent_obj, filename, (byte*)fileext,GET_INODE_MATCH);
    if (!pobj)
    {
        /* pc_get_inode() has set errno to PENOENT or to an internal or IO error status */
        goto errex;
    }

    if ( !pc_isadir(pobj) || (pobj->finode->opencount > 1) ||
            (pobj->finode->fattribute & ARDONLY ))
    {
        p_errno = PEACCES;
        goto errex;
    }

    /* Search through the directory. look at all files   */
    /* Any file that is not . or .. is a problem     */
    /* Call pc_get_inode with 0 to give us an obj     */
    pchild = pc_get_inode(0, pobj, 0, 0, GET_INODE_STAR);
    if (pchild)
    {
        do
        {
            if (!(pc_isdot(pchild->finode->fname, pchild->finode->fext) ) )
                if (!(pc_isdotdot(pchild->finode->fname, pchild->finode->fext) ) )
                {
                    p_errno = PEACCES;
                    ret_val = FALSE;
                    goto errex;
                }
        }
        while (pc_get_inode(pchild, pobj, 0, 0, GET_INODE_STAR));
    }
    /* If either of the above calls to pc_get_inode() failed due to
       an error other than PENOENT then we have an error condition
       Continue if the error was PEINVALIDCLUSTER that means that
       the chain is corrupted. In this case proceed to delete the
       directory too

    */
    p_errno = get_errno();
    if (!p_errno || (p_errno == PENOENT)) /* normal scan termination */
    {
        p_errno = 0;
        rtfs_set_errno(0);  /* Clear errno so others can use it */
        ret_val = pc_rmnode(pobj);
    }
    else if (p_errno == PEINVALIDCLUSTER)  /* termination because of bad
                                              cluster  */
    {
        /* Clear p_errno, errno is already PEINVALIDCLUSTER, if rmnode
           overwrites errno that will be the value, otherwise rmnode and
           children will set errno to another value */
        p_errno = 0;
        pc_rmnode(pobj);
        ret_val = FALSE;

    }
    else    /* Scan terminated due to IO error or internal error */
        ret_val = FALSE;

errex:
    if (pchild)
        pc_freeobj(pchild);
    if (pobj)
        pc_freeobj(pobj);
    if (parent_obj)
        pc_freeobj(parent_obj);
    if (p_errno)
        rtfs_set_errno(p_errno);
    
    /*---------- ctr modified ----------*/
    if( pdrive) {
        if( pdrive->dev_table_perform_device_ioctl) {
            pdrive->dev_table_perform_device_ioctl( driveno,
                                                    DEVCTL_FLUSH,
                                                    NULL);
        }
    }
    /*----------------------------------*/
    
    if (!release_drive_mount_write(driveno))/* Release lock, unmount if aborted */
        ret_val = FALSE;
    return(ret_val);
}
