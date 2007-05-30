/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APISETCWD.C - Contains user api level source code.

    The following routines are included:

    pc_set_cwd      - Set the current working directory.

*/

#include <rtfs.h>

/***************************************************************************
    PC_SET_CWD -  Set the current working directory for a drive.

 Description
    Find path. If it is a subdirectory make it the current working 
    directory for the drive.

 Returns
    Returns TRUE if the current working directory was changed.

    errno is set to one of the following
    0               - No error
    PEINVALIDPATH   - Path specified badly formed.
    PENOENT         - Path not found
    PEINVALIDDIR    - Not a directory
    An ERTFS system error 
****************************************************************************/

BOOLEAN pc_set_cwd(byte *name) /* __apifn__ */
{
    DROBJ *pobj;
    int driveno;
    DDRIVE *pdrive;
    DROBJ *parent_obj;
    byte  fileext[4];
    byte  *path, *pfilename,*pfilename_plus_1, *pfileext;
    BOOLEAN  ret_val;
    int p_errno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    rtfs_set_errno(0);  /* pc_set_cwd: clear error status */


    ret_val = FALSE;
    p_errno = 0;
    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(name);
    if (driveno < 0)
    {   /* errno was set by check_drive */
        return(FALSE);
    }

    pdrive = pc_drno2dr(driveno);

    /* Use the buffers in the DRIVE structure. Access is locked via semaphore */
    pfilename_plus_1 =  pfilename = (byte *)&(pdrive->filename_buffer[0]);
    pfileext = &fileext[0];
    CS_OP_INC_PTR(pfilename_plus_1);
    path = (byte *)&(pdrive->pathname_buffer[0]);

     /* Get out the filename and d:parent   */
    if (!pc_parsepath(path, pfilename,pfileext,name))
    {
        p_errno = PEINVALIDPATH;
        goto errex;
    }
    /* Find the parent and make sure it is a directory    */
    parent_obj = pc_fndnode(path);
    if (!parent_obj)
        goto errex; /* pc_fndnode set errno */

    if (!pc_isadir(parent_obj))
    {
        p_errno = PEACCES;      /* Path is not a directory */
        goto errex;
    }
    /* Get the directory   */
    if (CS_OP_CMP_ASCII(pfilename,'\0') || CS_OP_CMP_ASCII(pfilename,' '))
    {
        pobj = parent_obj;
    }
    else if (CS_OP_CMP_ASCII(pfilename,'.') && CS_OP_CMP_ASCII(pfilename_plus_1,'.'))
    {
        if (pc_isroot(parent_obj))
            pobj = parent_obj;
        else
        {
            pobj = pc_get_inode(0, parent_obj, 0, 0, GET_INODE_DOTDOT);
            /* If the request is cd .. then we just found the .. directory entry
                we have to call get_mom to access the parent. */
            pc_freeobj(parent_obj);
            if (!pobj)  /* pc_get_inode() has set errno to PENOENT or to an internal or IO error status */
                goto errex;
            parent_obj = pobj;
            /* Find parent_objs parent. By looking back from ..   */
            pobj = pc_get_mom(parent_obj);
            pc_freeobj(parent_obj);
            if (!pobj)
            {   /* if pc_get_mom() set errno, use it otherwise set PENOENT */
                if (!get_errno())
                    p_errno = PENOENT;      /* Not found */
                goto errex;
            }
        }
    }
    else if (CS_OP_CMP_ASCII(pfilename,'.'))
    {
        pobj = parent_obj;
    }
    else
    {
        pobj = pc_get_inode(0, parent_obj, pfilename, pfileext, GET_INODE_MATCH);
        pc_freeobj(parent_obj);
    }
    if (!pobj)
    {
        /* pc_get_inode set errno */
        goto errex;
    }
    else if (!pc_isadir(pobj))
    {
        pc_freeobj(pobj);
        p_errno = PEINVALIDDIR;      /* Path is not a directory */
        goto errex;
    }

    driveno = pobj->pdrive->driveno;
    if (rtfs_get_system_user()->lcwd[driveno] != 0)
        pc_freeobj((DROBJ *)(rtfs_get_system_user()->lcwd[driveno]));
    rtfs_get_system_user()->lcwd[driveno] = (void *) pobj;
    ret_val = TRUE;
errex:
    release_drive_mount(driveno);/* Release lock, unmount if aborted */
    if (p_errno)
        rtfs_set_errno(p_errno);
    return(ret_val);
}

