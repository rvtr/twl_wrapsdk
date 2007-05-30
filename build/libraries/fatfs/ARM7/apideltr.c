/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APIDELTR.C - Contains user api level source code.

  The following routines are included:
  
    pc_deltree      - Delete an entire directory tree.
*/

#include <rtfs.h>

/****************************************************************************
PC_DELTREE - Delete a directory tree.

  Description
  
    Delete the directory specified in name, all subdirectories of that 
    directory, and all files contained therein. Fail if name is not a 
    directory, or is read only.
    
      Returns
      Returns TRUE if the directory was successfully removed.
      
        errno will be set to one of these values
        
          0               - No error
          PEINVALIDDRIVEID- Drive component of path is invalid
          PEINVALIDPATH   - Path specified by name is badly formed.
          PENOENT         - Can't find path specified by name.
          PEACCES         - Directory or one of its subdirectories is read only or 
          in use.
          An ERTFS system error 
          
*****************************************************************************/

/* Remove a directory   */
BOOLEAN pc_deltree(byte  *name)  /*__apifn__*/
{
    DROBJ *parent_obj;
    DROBJ *pobj;
    DROBJ *pdotdot;
    DROBJ *pchild;
    BOOLEAN ret_val;
    DDRIVE *pdrive;
    byte  *path;
    byte  *filename;
    byte  fileext[4];
    int driveno;
    int p_errno;
    int dir_depth;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */
        
    parent_obj = 0;
    pchild = 0;
    pobj = 0;
    pdotdot = 0;
    ret_val = FALSE;
    dir_depth = 1;
    
    p_errno = 0;
    rtfs_set_errno(0);  /* pc_deltree: clear error status */
    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(name);
    if (driveno < 0)
    { /* pc_deltree: errno was by check_drive */
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
    parent_obj = pc_fndnode(path);
    if (!parent_obj)    
        goto errex;     /* pc_fndnode set errno */
    
    if (!pc_isadir(parent_obj) || pc_isavol(parent_obj))
    {   
        p_errno = PENOENT;
        goto errex;
    }
    /* Find the file and init the structure   */
    pobj = pc_get_inode(0, parent_obj, filename, (byte*)fileext, GET_INODE_MATCH);
    if (!pobj)
        goto errex; /* pc_get_inode set errno */
    
    if ( !pc_isadir(pobj) || (pobj->finode->opencount > 1) ||
            (pobj->finode->fattribute & ARDONLY ))
    {
        p_errno = PEACCES;
        goto errex;
    }
    
    /* Search through the directory. look at all files   */
    /* Call pc_get_inode with 0 to give us an obj     */
    while (dir_depth > 0) 
    {
        if (pchild)
            pc_freeobj(pchild);
        pchild = pc_get_inode(0, pobj, 0 , 0, GET_INODE_STAR);
        if (pdotdot) {
            pc_freeobj(pdotdot);
            pdotdot = 0;
        }

        if (pchild) do
        {
        /* delete all nodes which are not subdirs; 
            step into all subdirs and destroy their contents. */
            if (!(pc_isdot(pchild->finode->fname, pchild->finode->fext) ) )
            {
                if (!(pc_isdotdot(pchild->finode->fname, pchild->finode->fext) ) )
                {
                    if (pc_isadir(pchild))
                    {
                        if ( (pchild->finode->opencount > 1) ||
                             (pchild->finode->fattribute&ARDONLY) )
                        {
                            p_errno = PEACCES;
                            ret_val = FALSE;
                            goto errex;
                        }
                        dir_depth++;
                        pc_freeobj(pobj);
                        pobj = pchild; /* enter first subdir */
                        pchild = 0;
                        goto start_over;
                    } 
                    else 
                    {
                    /* Be sure it is not the root. Since the root is an abstraction 
                        we can not delete it plus Check access permissions */
                        if ( pc_isroot(pchild) || (pchild->finode->opencount > 1) ||
                            (pchild->finode->fattribute&(ARDONLY|AVOLUME|ADIRENT)))
                        {
                            p_errno = PEACCES;
                            ret_val = FALSE;
                            goto errex;
                        }
                        else
                        {
                        /* Remove the file */
                        /* calculate max number of clusters to release. 
                            Add bytespcluster-1 in case we are not on a cluster boundary */
                            ret_val = pc_rmnode(pchild);
                            if (!ret_val)
                                goto errex; /* pc_rmnode sets errno */
                            goto start_over;
                        }
                    }
                }
                else 
                {
                    if (pdotdot)
                        pc_freeobj(pdotdot);
                    pdotdot = pc_get_mom(pchild);
                }
            }
        } 
        while (pc_get_inode(pchild, pobj, 0 , 0, GET_INODE_STAR));

        if (get_errno() != PENOENT)
           goto errex; /* pc_get_inode set errno */
         
        /* dir empty; step out and delete */
        if (pobj) {
            pc_freeobj(pobj);
            pobj = 0;
        }
        if (pchild) {
            pc_freeobj(pchild);
            pchild = 0;
        }       
        dir_depth--;
        if (dir_depth > 0)
        {
            if (!pdotdot)
            {
                p_errno = PEINVALIDCLUSTER;
                goto errex;
            }
            pchild = pc_get_inode(0, pdotdot, 0 , 0, GET_INODE_STAR);
            if (pchild)
            {
                do
                {           
                    if (!(pc_isdot(pchild->finode->fname, pchild->finode->fext) ) )
                    {
                        if (!(pc_isdotdot(pchild->finode->fname, pchild->finode->fext) ) )
                        {
                            ret_val = pc_rmnode(pchild); /* remove the directory */
                            if (!ret_val)
                                goto errex; /* pc_mnode set errno */
                            break;
                        }
                    }
                } while (pc_get_inode(pchild, pdotdot, 0, 0, GET_INODE_STAR));
            }
            else
            {
                if (get_errno() != PENOENT)
                    goto errex; /* pc_get_inode set errno */
            }
             pobj = pdotdot;
            pdotdot = 0;
        }
        else
        {
            pobj = pc_get_inode(0, parent_obj, filename, (byte*)fileext, GET_INODE_MATCH);
            if (!pobj)
                goto errex; /* pc_mnode set errno */
            ret_val = pc_rmnode(pobj); /* Remove the directory */
            if (!ret_val)
                goto errex; /* pc_mnode set errno */
        }
start_over:
            ;
    }
errex:
    if (pdotdot)
        pc_freeobj(pdotdot);
    if (pchild)
        pc_freeobj(pchild);
    if (pobj)
        pc_freeobj(pobj);
    if (parent_obj)
        pc_freeobj(parent_obj);

    if (p_errno)        
        rtfs_set_errno(p_errno);
    if (!release_drive_mount_write(driveno))/* Release lock, unmount if aborted */
        ret_val = FALSE;
    return(ret_val);
}

