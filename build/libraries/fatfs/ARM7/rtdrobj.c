/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* RTDROBJ.C - Directory object manipulation routines
    pc_get_cwd          -  Get the current working directory DROBJ for a drive,
    pc_fndnode          -  Find a file or directory on disk and return a DROBJ.
    pc_get_inode        -  Find a filename within a subdirectory
    static pc_findin    -  Find a filename in the same directory as argument.

    pc_get_mom          -  Find the parent inode of a subdirectory.

    pc_mkchild          -  Allocate a DROBJ and fill in based on parent object.
    pc_mknode           -  Create an empty subdirectory or file.
    pc_insert_inode - Called only by pc_mknode

    pc_rmnode           -  Delete an inode unconditionally.
    pc_update_inode -  Flush an inode to disk

    pc_read_obj     -  Assign an initialized BLKBUFF to a DROBJ
    pc_write_obj        -  Write a DROBJ s BLKBUFF to disk

    pc_get_root     -  Create the special ROOT object for a drive.
    pc_firstblock   -  Return the absolute block number of a directory
    pc_next_block   -  Calculate the next block owned by an object.
    pc_l_next_block -  Calculate the next block in a chain.

        We keep a shared image of directory entries around. These routines
        keep track of them for us.
    pc_marki        -  Set dr:sec:index, + stitch FINODE into the inode list
    pc_scani        -  Search for an inode in the internal inode list.
        Heap management routines

    pc_allocobj     -
    pc_alloci           -
    pc_free_all_i       -
    pc_freei            -
    pc_freeobj          -

        Simple helper routines
    pc_dos2inode        -
    pc_ino2dos          -
    pc_init_inode       -
    pc_isadir           -

*/

#include <rtfs.h>


/***************************************************************************
    PC_GET_CWD -  Get the current working directory for a drive,

 Description
    Return the current directory inode for the drive represented by ddrive.

***************************************************************************/

/*  Get the current working directory and copy it into pobj   */
DROBJ *pc_get_cwd(DDRIVE *pdrive)                                   /*__fn__*/
{
    DROBJ *pcwd;
    DROBJ *pobj;

    pcwd = (DROBJ *)(rtfs_get_system_user()->lcwd[pdrive->driveno]);

    /* If no current working dir set it to the root   */
    if (!pcwd)
    {
        pcwd = pc_get_root(pdrive);
        rtfs_get_system_user()->lcwd[pdrive->driveno] = pcwd;
    }

    if (pcwd)
    {
        pobj = pc_allocobj();
        if (!pobj)
        {
            return (0);
        }
        /* Free the inode that comes with allocobj   */
        pc_freei(pobj->finode);
        OS_CLAIM_FSCRITICAL()
        copybuff(pobj, pcwd, sizeof(DROBJ));
        pobj->finode->opencount += 1;
        OS_RELEASE_FSCRITICAL()
        return (pobj);
    }
    else    /* If no cwd is set error */
    {
        rtfs_set_errno(PEINTERNAL);
        return(0);
    }
}


 /**************************************************************************
    PC_FNDNODE -  Find a file or directory on disk and return a DROBJ.

 Description
    Take a full path name and traverse the path until we get to the file
    or subdir at the end of the path spec. When found allocate and init-
    ialize (OPEN) a DROBJ.

 Returns
    Returns a pointer to a DROBJ if the file was found, otherwise 0.

***************************************************************************/


/* Find path and create a DROBJ structure if found   */
DROBJ *pc_fndnode(byte *path)                                   /*__fn__*/
{
    DROBJ *pobj;
    DROBJ *pmom;
    DROBJ *pchild;
    int  driveno;
    DDRIVE *pdrive;
    byte *filename;
    byte fileext[4];
    byte *pf0,*pf1;
    BLKBUFF *scratch;


    /* Get past D: plust get drive number if there   */
    path = pc_parsedrive( &driveno, path );
    if (!path)
    {
        rtfs_set_errno(PEINVALIDDRIVEID); /* pc_fndnode: parsedrive failed */
        return (0);
    }
    /* Find the drive   */
    pdrive = pc_drno2dr(driveno);
    if (!pdrive)
    {
        rtfs_set_errno(PEINVALIDDRIVEID); /* pc_fndnode: pc_drno2dr failed */
        return (0);
    }
    /* Get the top of the current path   */
    if (CS_OP_CMP_ASCII(path, '\\'))
    {
        CS_OP_INC_PTR(path);
        pobj = pc_get_root(pdrive);
    }
    else
    {
        pobj = pc_get_cwd(pdrive);
    }

    if (!pobj)
        return (0);
    scratch = pc_scratch_blk();
    if (!scratch)
        return(0);
    filename = (byte *)scratch->data;

    /* Search through the path til exausted   */
    while (CS_OP_IS_NOT_EOS(path))
    {
        path = pc_nibbleparse(filename,fileext, path );
        if (!path)
        {
            pc_free_scratch_blk(scratch);
            rtfs_set_errno(PENOENT); /* pc_fndnode: no match */
            pc_freeobj(pobj);
            return (0);
        }
#if (RTFS_SUBDIRS)
        pf1 = pf0 = filename;
        CS_OP_INC_PTR(pf1);
#if (VFAT)
        if (CS_OP_CMP_ASCII(pf0,'.') && CS_OP_IS_EOS(pf1)) /* DOT */
            ;
#else
        /* Tomo */
        if (CS_OP_CMP_ASCII(pf0,'.') && CS_OP_CMP_ASCII(pf1,' ')) /* DOT-in NON-VFAT it is space filled  */
            ;
#endif
#endif
        else
        {
            /* Find Filename in pobj. and initialize lpobj with result   */
            pchild = pc_get_inode(0, pobj, filename, fileext, GET_INODE_MATCH);
            if (!pchild)
            { /* get_inode set errno */
                pc_free_scratch_blk(scratch);
                pc_freeobj(pobj);
                return (0);
            }
#if (RTFS_SUBDIRS)
            /* We found it. We have one special case. if DOTDOT we need
                to shift up a level so we are not the child of mom
                but of grand mom. */
            pf1 = pf0 = filename;
            CS_OP_INC_PTR(pf1);
            if (CS_OP_CMP_ASCII(pf0,'.') && CS_OP_CMP_ASCII(pf1,'.'))
            {
                /* Find pobj s parent. By looking back from DOTDOT   */
                pmom = pc_get_mom(pchild);
                /* We are done with pobj for now   */
                pc_freeobj(pobj);

                if (!pmom)
                { /* Get mom set errno */
                    pc_free_scratch_blk(scratch);
                    pc_freeobj(pchild);
                    return (0);
                }
                else
                {
                    /* We found the parent now free the child   */
                    pobj = pmom;
                    pc_freeobj(pchild);
                }
            }
            else
#endif      /* SUBDIRS */
            {
                /* We are done with pobj for now   */
                pc_freeobj(pobj);
                /* Make sure pobj points at the next inode   */
                pobj = pchild;
#if (RTFS_SUBDIRS)
#else
/* No subdirectory support. Return the one we found   */
                pc_free_scratch_blk(scratch);
                return (pobj);
#endif
            }
        }
    }
    pc_free_scratch_blk(scratch);
    return (pobj);
}

/***************************************************************************
    PC_GET_INODE -  Find a filename within a subdirectory

 Description
    Search the directory pmom for the pattern or name in filename:ext and
    return the an initialized object. If pobj is NULL start the search at
    the top of pmom (getfirst) and allocate pobj before returning it.
    Otherwise start the search at pobj (getnext). (see also pc_gfirst,
    pc_gnext)


    Note: Filename and ext must be right filled with spaces to 8 and 3 bytes
            respectively. Null termination does not matter.

    Note the use of the action variable. This is used so we do not have to
    use match patterns in the core code like *.* and ..
    GET_INODE_MATCH   Must match the pattern exactly
    GET_INODE_WILD    Pattern may contain wild cards
    GET_INODE_STAR    Like he passed *.* (pattern will be null)
    GET_INODE_DOTDOT  Like he past .. (pattern will be null


 Returns
    Returns a drobj pointer or NULL if file not found.
***************************************************************************/

/* Give a directory mom. And a file name and extension.
    Find find the file or dir and initialize pobj.
    If pobj is NULL. We allocate and initialize the object otherwise we get the
    next item in the chain of dirents.
*/
DROBJ *pc_get_inode( DROBJ *pobj, DROBJ *pmom, byte *filename, byte *fileext, int action) /*__fn__*/
{
    BOOLEAN  starting = FALSE;
    /* Create the child if just starting   */
    if (!pobj)
    {
        starting = TRUE;
        pobj = pc_mkchild(pmom);    /* If failure sets errno */
        if (!pobj)
            return(0);
    }
    else    /* If doing a gnext do not get stuck in and endless loop */
    {
        if ( ++(pobj->blkinfo.my_index) >= INOPBLOCK )
        {
            rtfs_set_errno(0);  /* Clear errno to be safe */
            if (!pc_next_block(pobj)) /* Will set errno illegal block id encountered */
            {
                if (!get_errno())
                    rtfs_set_errno(PENOENT); /* pc_get_inode: end of dir */
                if (starting)
                    pc_freeobj(pobj);
                return(0);
            }
            else
                pobj->blkinfo.my_index = 0;
        }
    }
    if (pc_findin(pobj, filename, fileext,action))
    {
        return (pobj);
    }
    else
    {
        /* pc_findin set errno */
        if (starting)
            pc_freeobj(pobj);
        return (0);
    }
}

/**************************************************************************
    PC_GET_MOM -  Find the parent inode of a subdirectory.

 Description
    Given a DROBJ initialized with the contents of a subdirectory s DOTDOT
    entry, initialize a DROBJ which is the parent of the current directory.

 Returns
    Returns a DROBJ pointer or NULL if could something went wrong.


****************************************************************************/
#if (RTFS_SUBDIRS)

/*
* Get mom:
*   if (!dotodot->cluster)  Mom is root.
*       getroot()
*   else                    cluster points to mom.
*       find .. in mom
*       then search through the directory pointed to by moms .. until
*       you find mom. This will be current block startblock etc for mom.
*/

DROBJ *pc_get_mom(DROBJ *pdotdot)                                   /*__fn__*/
{
    DROBJ *pmom;
    DDRIVE *pdrive = pdotdot->pdrive;
    BLOCKT sectorno;
    BLKBUFF *rbuf;
    DIRBLK *pd;
    DOSINODE *pi;
    FINODE *pfi;
    dword clno;
    /* We have to be a subdir   */
    if (!pc_isadir(pdotdot))
    {
        rtfs_set_errno(PEINVALIDDIR);
        return(0);
    }

    /* If ..->cluster is zero then parent is root   */
    if (!pc_finode_cluster(pdrive,pdotdot->finode))
        return(pc_get_root(pdrive));

    /* Otherwise : cluster points to the beginning of our parent.
                    we also need the position of our parent in its parent  */
    pmom = pc_allocobj();
    if (!pmom)
        return (0);

    pmom->pdrive = pdrive;
    /* Find .. in our parent s directory   */
    clno = pc_finode_cluster(pdrive,pdotdot->finode);
    if ((clno < 2) || (clno > pdrive->maxfindex) )
    {
        pc_freeobj(pmom);
        rtfs_set_errno(PEINVALIDCLUSTER);
        return (0);
    }
    sectorno = pc_cl2sector(pdrive,clno);
    /* We found .. in our parents dir.   */
    pmom->pdrive = pdrive;
    pmom->blkinfo.my_frstblock =  sectorno;
    pmom->blkinfo.my_block  =  sectorno;
    pmom->blkinfo.my_index  =  0;
    pmom->isroot = FALSE;
    pd = &pmom->blkinfo;

    pmom->pblkbuff = rbuf = pc_read_blk(pdrive, pmom->blkinfo.my_block);
    if (rbuf)
    {
        pi = (DOSINODE *) &rbuf->data[0];
        OS_CLAIM_FSCRITICAL()
        pc_dos2inode(pmom->finode , pi );
        OS_RELEASE_FSCRITICAL()

        pc_release_buf(rbuf);

        /* See if the inode is in the buffers   */
        pfi = pc_scani(pdrive, sectorno, 0);
        if (pfi)
        {
            pc_freei(pmom->finode);
            pmom->finode = pfi;
        }
        else
        {
            pc_marki(pmom->finode , pmom->pdrive , pd->my_block,
                    pd->my_index);
        }
        return (pmom);

    }
    else    /* Error, something did not work */
    {
        pc_freeobj(pmom);
        return (0);
    }
}
#endif

/**************************************************************************
    PC_MKCHILD -  Allocate a DROBJ and fill in based on parent object.

 Description
    Allocate an object and fill in as much of the the block pointer section
    as possible based on the parent.

 Returns
    Returns a partially initialized DROBJ if enough core available and
    pmom was a valid subdirectory.

****************************************************************************/

DROBJ *pc_mkchild( DROBJ *pmom)                                     /*__fn__*/
{
    DROBJ *pobj;
    DIRBLK *pd;

    /* Mom must be a directory   */
    if (!pc_isadir(pmom))
    {
        rtfs_set_errno(PEINVALIDDIR); /* pc_mkchild: Internal error, parent dir provided */
        return(0);
    }
    /* init the object -   */
    pobj = pc_allocobj();
    if (!pobj)
        return (0);

    pd = &pobj->blkinfo;

    pobj->isroot = FALSE;               /* Child can not be root */
    pobj->pdrive =  pmom->pdrive;   /* Child inherets moms drive */

    /* Now initialize the fields storing where the child inode lives   */
    pd->my_index = 0;
    pd->my_block = pd->my_frstblock = pc_firstblock(pmom);
    if (!pd->my_block)
    {
        pc_freeobj(pobj);
        return (0);
    }

    return (pobj);
}

/**************************************************************************
    PC_MKNODE -  Create an empty subdirectory or file.

 Description
    Creates a file or subdirectory inode depending on the flag values in
    attributes. A pointer to an inode is returned for further processing. See
    po_open(),po_close(), pc_mkdir() et al for examples.

    Note: After processing, the DROBJ must be released by calling pc_freeobj.

 Returns
    Returns a pointer to a DROBJ structure for further use, or NULL if the
    inode name already exists or path not found.
**************************************************************************/

/* Make a node from path and attribs create and fill in pobj             */
/* Note: the parent directory is locked before this routine is called    */
DROBJ *pc_mknode(DROBJ *pmom ,byte *filename, byte *fileext, byte attributes, CLUSTERTYPE incluster) /*__fn__*/
{
    DROBJ *pobj;
    BOOLEAN ret_val;
    DOSINODE *pdinodes;
    FINODE lfinode;
    CLUSTERTYPE cluster;
    CLUSTERTYPE cltemp;
    DATESTR crdate;
    BLKBUFF *pbuff;
    DDRIVE *pdrive;
    byte attr;
#if (RTFS_SUBDIRS)
    byte dot_str[4]; /* Make DOT and DOTDOT strings */
    byte null_str[4];
#endif

    ret_val = TRUE;

    if (!pmom || !pmom->pdrive)
    {
        rtfs_set_errno(PEINTERNAL); /* pc_mknode: Internal error*/
        return (0);
    }
    /* Make sure the file/directory name is legal */
    if (!validate_filename(filename, fileext))
    {
        rtfs_set_errno(PEINVALIDPATH);
        return (0);
    }


    pdrive = pmom->pdrive;
    cluster = incluster;
#if (RTFS_SUBDIRS)
    if (attributes & ADIRENT)
    {
        /*Unless renaming a directory we grab a cluster for a new dir and */
        /* clear it   */
        if (!incluster)
        {
            /* pc_alloc_dir will set errno */
            cluster = pc_alloc_dir(pdrive,pmom);
            if (!cluster)
                ret_val = FALSE;
            else if (!pc_clzero( pdrive , cluster ) )
            {
                /* clzero will set errno */
                FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
                ret_val = FALSE;
            }
        }
    }
#endif

    if (!ret_val)
    {
        return (0);
    }

    /* For a subdirectory. First make it a simple file. We will change the
        attribute after all is clean */
    attr = attributes;
#if (RTFS_SUBDIRS)
    if (attr & ADIRENT)
        attr = ANORMAL;
#endif
    /* Allocate an empty DROBJ and FINODE to hold the new file   */

    pobj = pc_allocobj();
    if (!pobj)
    {
        return (0);
    }
    /* Set up the drive link */
    pobj->pdrive = pmom->pdrive;

    /* Load the inode copy name,ext,attr,cluster, size,datetime  */
    /* Convert pobj to native and stitch it in to mom   */
    if (!pc_insert_inode(pobj , pmom, attr, cluster, filename, fileext))
    {
        if (cluster && !incluster)
            FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
        pc_freeobj(pobj);
        return (0);
    }

#if (RTFS_SUBDIRS)
    /* Now if we are creating subdirectory we have to make the DOT and DOT DOT
        inodes and then change pobj s attribute to ADIRENT
        The DOT and DOTDOT are not buffered inodes. We are simply putting
        the to disk  */
    if ( attributes & ADIRENT)
    {
        if (incluster)
        {
            pbuff = pc_read_blk( pdrive , pc_cl2sector(pdrive , incluster));
            if (!pbuff)
            {
                pc_freeobj(pobj);
                return (0);
            }
        }
        else
        {
            /* Set up a buffer to do surgery   */
            pbuff = pc_init_blk( pdrive , pc_cl2sector(pdrive , cluster));
            if (!pbuff)
            {
                pc_freeobj(pobj);
                FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
                return (0);
            }
        }
        pdinodes = (DOSINODE *) &pbuff->data[0];
        /* Load DOT and DOTDOT in native form                     */
        /* DOT first. It points to the begining of this sector    */
        dot_str[0] = CS_OP_ASCII('.');dot_str[1]=CS_OP_ASCII('\0');
        null_str[0] = CS_OP_ASCII('\0');

        /* Load the time and date stamp to be used for "." and ".."
           from the subdirectory that we are creating. In this way
           the three directory entries will all have the same timestamp
           value */
        crdate.time = pobj->finode->ftime;
        crdate.date = pobj->finode->fdate;
        pc_init_inode( &lfinode, dot_str,null_str, ADIRENT|ARCHIVE,
                            cluster, /*size*/ 0L , &crdate);
        /* And to the buffer in intel form   */
        pc_ino2dos (pdinodes, &lfinode);
        /* Now DOTDOT points to mom s cluster   */
        cltemp = pc_get_parent_cluster(pdrive, pobj);
        dot_str[0] = CS_OP_ASCII('.');dot_str[1] = CS_OP_ASCII('.');
        dot_str[2] = CS_OP_ASCII('\0');
        null_str[0] = CS_OP_ASCII('\0');
        pc_init_inode( &lfinode, dot_str, null_str, ADIRENT|ARCHIVE, cltemp,
                    /*size*/ 0L , &crdate);
        /* And to the buffer in intel form   */
        pc_ino2dos (++pdinodes, &lfinode );
        /* Write the cluster out   */
        if ( !pc_write_blk ( pbuff ) )
        {
            pc_discard_buf(pbuff);
            pc_freeobj(pobj);
            if (!incluster)
                FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
            return (0);
        }
        else
            pc_release_buf(pbuff);

        /* And write the node out with the original attributes   */
        pobj->finode->fattribute = (byte)(attributes|ARCHIVE);

        /* Convert to native. Overwrite the existing inode.Set archive/date  */
        if (!pc_update_inode(pobj, TRUE, TRUE))
        {
            pc_freeobj(pobj);
            if (!incluster)
                FATOP(pdrive)->fatop_clrelease_dir(pdrive , cluster);
            return (0);
        }
    }
#endif
    ret_val = FATOP(pdrive)->fatop_flushfat(pdrive->driveno);

    if (ret_val)
    {
        return (pobj);
    }
    else
    {
        pc_freeobj(pobj);
        return (0);
    }
}

/***************************************************************************
    PC_RMNODE - Delete an inode unconditionally.

 Description
    Delete the inode at pobj and flush the file allocation table. Does not
    check file permissions or if the file is already open. (see also pc_unlink
    and pc_rmdir). The inode is marked deleted on the disk and the cluster
    chain associated with the inode is freed. (Un-delete wo not work)

 Returns
    Returns TRUE if it successfully deleted the inode an flushed the fat.

*****************************************************************************/


/* Delete a file / dir or volume. Do not check for write access et al     */
/* Note: the parent directory is locked before this routine is called    */
BOOLEAN pc_rmnode( DROBJ *pobj)                                         /*__fn__*/
{
    CLUSTERTYPE cluster;
    DDRIVE *pdrive;

    /* Do not delete anything that has multiple links   */
    if (pobj->finode->opencount > 1)
    {
        rtfs_set_errno(PEACCES); /* pc_rmnode() - directory entry is in use */
err_ex:
        pc_report_error(PCERR_REMINODE);
        return (FALSE);
    }
    pdrive = pobj->pdrive;
    /* Mark it deleted and unlink the cluster chain   */
    pobj->finode->fname[0] = PCDELETEESCAPE;

    cluster = pc_finode_cluster(pdrive,pobj->finode);

    if (!pc_delete_lfn_info(pobj)) /* Delete lonf file name info associated with DROBJ */
        goto err_ex;

    /* We free up store right away. Do not leave cluster pointer
    hanging around to cause problems. */
    pc_pfinode_cluster(pdrive,pobj->finode,0);
    /* Convert to native. Overwrite the existing inode.Set archive/date  */
    if (pc_update_inode(pobj, TRUE, TRUE))
    {
        /* If there is no cluster chain to delete don't call freechain */
        if (!cluster)
            return(TRUE);
        /* And clear up the space   */
        /* Tomo */
        /* Make sure the blocks contained within the cluster chain
           are flushed from the block buffer pool */
        pc_flush_chain_blk(pdrive, cluster);
        /* Set min to 0 and max to 0xffffffff to eliminate range checking on the
           cluster chain and force removal of all clusters */
           if (!FATOP(pdrive)->fatop_freechain(pdrive, cluster, 0, 0xffffffff))
           {
                /* If freechain failed still flush the fat in INVALID CLUSTER
                   was the failure condition */
                if (get_errno() == PEINVALIDCLUSTER)
                   FATOP(pdrive)->fatop_flushfat(pobj->pdrive->driveno);
                return(FALSE);

           }
           else if ( FATOP(pdrive)->fatop_flushfat(pobj->pdrive->driveno) )
            return (TRUE);
    }
    /* If it gets here we had a problem   */
    return(FALSE);
}

/**************************************************************************
    PC_UPDATE_INODE - Flush an inode to disk

Summary

Description
    Read the disk inode information stored in pobj and write it to
    the block and offset on the disk where it belongs. The disk is
    first read to get the block and then the inode info is merged in
    and the block is written. (see also pc_mknode() )

Returns
    Returns TRUE if all went well, no on a write error.

*****************************************************************************
*/

/* Take a DROBJ that contains correct my_index & my_block. And an inode.
    Load the block. Copy the inode in and write it back out */
BOOLEAN pc_update_inode(DROBJ *pobj, BOOLEAN set_archive, BOOLEAN set_date) /*__fn__*/
{
    BLKBUFF *pbuff;
    DOSINODE *pi;
    int i;
    DIRBLK *pd;
    DATESTR crdate;

    pd = &pobj->blkinfo;
    i   = pd->my_index;
    if ( i >= INOPBLOCK || i < 0 )  /* Index into block */
    {
        rtfs_set_errno(PEINTERNAL); /* pc_update_inode: Internal error, illegal inode index */
        return (FALSE);
    }
    OS_CLAIM_FSCRITICAL()
    /* Set the archive bit and the date   */
    if (set_archive)
        pobj->finode->fattribute |= ARCHIVE;

    if (set_date)
    {
        pc_getsysdate(&crdate);
        pobj->finode->ftime = crdate.time;
        pobj->finode->fdate = crdate.date;
    }
    OS_RELEASE_FSCRITICAL()
    /* Read the data   */
    pobj->pblkbuff = pbuff = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    if (pbuff)
    {
        pi = (DOSINODE *) &pbuff->data[0];
        /* Copy it off and write it   */
        pc_ino2dos( (pi+i), pobj->finode );
        if (!pc_write_blk(pbuff))
        {
            pc_discard_buf(pbuff);
            return (FALSE);
        }
        else
        {
            pc_release_buf(pbuff);
            return (TRUE);
        }
    }
    return (FALSE);
}

#if (RTFS_WRITE)
    /* Load an in memory inode up with user supplied values  */
void pc_init_inode(FINODE *pdir, KS_CONSTANT byte *filename,        /*__fn__*/
            KS_CONSTANT byte *fileext, byte attr,
            CLUSTERTYPE cluster, dword size, DATESTR *crdate)       /*__fn__*/
{
    /* Copy the file names and pad with spaces   */
    pc_cppad(pdir->fname,(byte*)filename,8);
    pc_cppad(pdir->fext,(byte*)fileext,3);
    pdir->fattribute = attr;
    rtfs_memset(&pdir->resarea[0],(byte) 0,8);

    pdir->ftime = crdate->time;
    pdir->fdate = crdate->date;
    /* Note: fclusterhi is of resarea in fat 16 system and cluster>>16 is 0*/
    pdir->fclusterhi = (word)(cluster >> 16);
    pdir->fcluster = (word) cluster;
    pdir->fsize = size;
    pc_zero_lfn_info(pdir);

}

/***************************************************************************
    PC_INO2DOS - Convert  an in memory inode to a dos disk entry.

 Description
    Take in memory native format inode information and copy it to a
    buffer. Translate the inode to INTEL byte ordering during the transfer.

 Returns
    Nothing

***************************************************************************/
/* Un-Make a disk directory entry         */
/* Convert an inmem inode to dos  form.   */
void pc_ino2dos (DOSINODE *pbuff, FINODE *pdir)                 /*__fn__*/
{
    pc_ascii_strn2upper((byte *)&pbuff->fname[0],(byte *)&pdir->fname[0],8);      /*X*/
    pc_ascii_strn2upper((byte *)&pbuff->fext[0],(byte *)&pdir->fext[0],3);            /*X*/
    pbuff->fattribute = pdir->fattribute;               /*X*/
    /* If the first character is 0xE5 (valid kanji) convert it to 0x5 */
    if (pdir->fname[0] == PCDELETE) /* 0XE5 */
        pbuff->fname[0] = 0x5;

    /* If rmnode wants us to delete the file set it to 0xE5 */
    if (pdir->fname[0] == PCDELETEESCAPE)
        pbuff->fname[0] = PCDELETE;


    copybuff(&pbuff->resarea[0],&pdir->resarea[0], 8);  /*X*/
#if (KS_LITTLE_ENDIAN)
    pbuff->ftime = pdir->ftime;
    pbuff->fdate = pdir->fdate;
    pbuff->fcluster = pdir->fcluster;
    /* Note: fclusterhi is of resarea in fat 16 system */
    pbuff->fclusterhi = pdir->fclusterhi;
    pbuff->fsize = pdir->fsize;
#else
    fr_WORD((byte *) &pbuff->ftime,pdir->ftime);        /*X*/
    fr_WORD((byte *) &pbuff->fdate,pdir->fdate);        /*X*/
    fr_WORD((byte *) &pbuff->fcluster,pdir->fcluster);  /*X*/
    /* Note: fclusterhi is of resarea in fat 16 system */
    fr_WORD((byte *) &pbuff->fclusterhi,pdir->fclusterhi);  /*X*/
    fr_DWORD((byte *) &pbuff->fsize,pdir->fsize);       /*X*/
#endif
}

#endif


/*************************************************************************
    PC_GET_ROOT -  Create the special ROOT object for a drive.
 Description
    Use the information in pdrive to create a special object for accessing
    files in the root directory.

 Returns
    Returns a pointer to a DROBJ, or NULL if no core available.
****************************************************************************/

/* Initialize the special root object
    Note: We do not read any thing in here we just set up
    the block pointers. */

DROBJ *pc_get_root( DDRIVE *pdrive)                                 /*__fn__*/
{
    DIRBLK *pd;
    DROBJ *pobj;
    FINODE *pfi;

    pobj = pc_allocobj();
    if (!pobj)
        return (0);
    pobj->pdrive = pdrive;

    pfi = pc_scani(pdrive, 0, 0);

    if (pfi)
    {
        pc_freei(pobj->finode);
        pobj->finode = pfi;
    }
    else    /* No inode in the inode list. Copy the data over
                and mark where it came from */
    {
        pc_marki(pobj->finode , pdrive , 0, 0);
    }


    /* Add a TEST FOR DRIVE INIT Here later   */
    pobj->pdrive = pdrive;
    /* Set up the tree stuf so we know it is the root   */
    pd = &pobj->blkinfo;
    pd->my_frstblock = pdrive->rootblock;
    pd->my_block = pdrive->rootblock;
    pd->my_index = 0;
    pobj->isroot = TRUE;
    return (pobj);
}


/****************************************************************************
    PC_FIRSTBLOCK -  Return the absolute block number of a directory s
                    contents.
 Description
    Returns the block number of the first inode in the subdirectory. If
    pobj is the root directory the first block of the root will be returned.

 Returns
    Returns 0 if the obj does not point to a directory, otherwise the
    first block in the directory is returned.

*****************************************************************************/

/* Get  the first block of a root or subdir   */
BLOCKT pc_firstblock( DROBJ *pobj)                                  /*__fn__*/
{
    dword clno;
    if (!pc_isadir(pobj))
    {
        rtfs_set_errno(PEINTERNAL); /* pc_firstblock: Internal error */
        return (BLOCKEQ0);
    }
    /* Root dir ?   */
#if (RTFS_SUBDIRS)
    if (!pobj->isroot)
    {
        clno = pc_finode_cluster(pobj->pdrive, pobj->finode);
        if ((clno < 2) || (clno > pobj->pdrive->maxfindex) )
        {
            rtfs_set_errno(PEINVALIDCLUSTER);
            return (0);
        }
        return (pc_cl2sector(pobj->pdrive , clno));
    }
    else
#endif
        return (pobj->blkinfo.my_frstblock);
}

/***************************************************************************
    PC_NEXT_BLOCK - Calculate the next block owned by an object.

 Description
    Find the next block owned by an object in either the root or a cluster
    chain and update the blockinfo section of the object.

 Returns
    Returns TRUE or FALSE on end of chain.

*****************************************************************************/


/* Calculate the next block in an object   */
BOOLEAN pc_next_block( DROBJ *pobj)                                 /*__fn__*/
{
    BLOCKT nxt;

    nxt = pc_l_next_block(pobj->pdrive, pobj->blkinfo.my_block);

    if (nxt)
    {
        pobj->blkinfo.my_block = nxt;
        return (TRUE);
    }
    else
        return (FALSE);
}

/**************************************************************************
    PC_L_NEXT_BLOCK - Calculate the next block in a chain.

 Description
    Find the next block in either the root or a cluster chain.

 Returns
    Returns 0 on end of root dir or chain.

****************************************************************************/

    /* Return the next block in a chain   */
BLOCKT pc_l_next_block(DDRIVE *pdrive, BLOCKT curblock)             /*__fn__*/
{
    CLUSTERTYPE cluster;
    /* If the block is in the root area   */
    if (curblock < pdrive->firstclblock)
    {
        if (curblock < pdrive->rootblock)
        {
            rtfs_set_errno(PEINVALIDBLOCKNUMBER); /* pc_l_next_block: Internal error */
            return (BLOCKEQ0);
        }
        else if (++curblock < pdrive->firstclblock)
            return (curblock);
        else
            return (BLOCKEQ0);
    }
    else  /* In cluster space   */
    {
        if (curblock >= pdrive->numsecs)
        {
            rtfs_set_errno(PEINVALIDBLOCKNUMBER); /* pc_l_next_block: Internal error */
            return (BLOCKEQ0);
        }
        /* Get the next block   */
        curblock += 1;

        /* If the next block is not on a cluster edge then it must be
            in the same cluster as the current. - otherwise we have to
            get the firt block from the next cluster in the chain */
        if (pc_sec2index(pdrive, curblock))
            return (curblock);
        else
        {
            curblock -= 1;
            /* Get the old cluster number */
            cluster = pc_sec2cluster(pdrive,curblock);
            if ((cluster < 2) || (cluster > pdrive->maxfindex) )
            {
                rtfs_set_errno(PEINVALIDBLOCKNUMBER); /* pc_l_next_block: Internal error */
                return (BLOCKEQ0);
            }
            /* Consult the fat for the next cluster   */
            cluster = FATOP(pdrive)->fatop_clnext(pdrive, cluster); /* Directory  */
            if (cluster == 0xffffffff)
                return (BLOCKEQ0); /* End of chain */
            else if (cluster == 0) /* clnext detected error */
            {
                return (BLOCKEQ0);
            }
            else
                return (pc_cl2sector(pdrive, cluster));
        }
    }
}

/**************************************************************************
    PC_MARKI -  Set dr:sec:index info and stitch a FINODE into the inode list


Description
    Each inode is uniquely determined by DRIVE, BLOCK and Index into that
    block. This routine takes an inode structure assumed to contain the
    equivalent of a DOS directory entry. And stitches it into the current
    active inode list. Drive block and index are stored for later calls
    to pc_scani and the inode s opencount is set to one.

Returns
    Nothing

***************************************************************************/



/* Take an unlinked inode and link it in to the inode chain. Initialize
    the open count and sector locater info. */
void pc_marki( FINODE *pfi, DDRIVE *pdrive, BLOCKT sectorno, int  index)/*__fn__*/
{
    OS_CLAIM_FSCRITICAL()

    pfi->my_drive = pdrive;
    pfi->my_block = sectorno;
    pfi->my_index = index;
    pfi->opencount = 1;

    /* Stitch the inode at the front of the list   */
    if (prtfs_cfg->inoroot)
        prtfs_cfg->inoroot->pprev = pfi;

    pfi->pprev = 0;
    pfi->pnext = prtfs_cfg->inoroot;
    prtfs_cfg->inoroot = pfi;

    OS_RELEASE_FSCRITICAL()
}


/**************************************************************************
    PC_SCANI -  Search for an inode in the internal inode list.

Description
    Each inode is uniquely determined by DRIVE, BLOCK and Index into that
    block. This routine searches the current active inode list to see
    if the inode is in use. If so the opencount is changed and a pointer is
    returned. This guarantees that two processes will work on the same
    information when manipulating the same file or directory.

Returns
    A pointer to the FINODE for pdrive:sector:index or NULL if not found

****************************************************************************/

/* See if the inode for drive,sector , index is in the list. If so..
    bump its open count and return it. Else return NULL */

FINODE *pc_scani( DDRIVE *pdrive, BLOCKT sectorno, int index)       /*__fn__*/
{
    FINODE *pfi;
    OS_CLAIM_FSCRITICAL()
    pfi = prtfs_cfg->inoroot;
    while (pfi)
    {
        if ( (pfi->my_drive == pdrive) &&
            (pfi->my_block == sectorno) &&
            (pfi->my_index == index) )
        {
            pfi->opencount += 1;
            OS_RELEASE_FSCRITICAL()
            return (pfi);
        }
        pfi = pfi->pnext;
    }
    OS_RELEASE_FSCRITICAL()
    return (0);
}


/**************************************************************************
    PC_ALLOCOBJ -  Allocate a DROBJ structure
 Description
    Allocates and zeroes the space needed to store a DROBJ structure. Also
    allocates and zeroes a FINODE structure and links the two via the
    finode field in the DROBJ structure.

 Returns
    Returns a valid pointer or NULL if no more core.

*****************************************************************************/

DROBJ *pc_allocobj(void)                                    /*__fn__*/
{
    DROBJ *pobj;

    /* Alloc a DROBJ   */
    pobj = pc_memory_drobj(0);
    if (pobj)
    {
        pobj->finode = pc_alloci();
        if (!pobj->finode)
        {
            /* Free the DROBJ   */
            pc_memory_drobj(pobj);
            pobj = 0;
        }
    }
    return (pobj);
}

/**************************************************************************
    PC_ALLOCI -  Allocate a FINODE structure

 Description
    Allocates and zeroes a FINODE structure.

 Returns
    Returns a valid pointer or NULL if no more core.

****************************************************************************/

FINODE *pc_alloci(void)                                         /*__fn__*/
{
FINODE *p;
    p = pc_memory_finode(0);
    return(p);
}

/**************************************************************************
    PC_FREE_ALL_DROBJ -  Release all drobj buffers associated with a drive.
Description
    For each internally buffered drobj structure associated with pdrive
Returns
    Nothing
****************************************************************************/

void pc_free_all_drobj( DDRIVE *pdrive)                             /*__fn__*/
{
    int i;
    DROBJ *pobj;
    pobj = prtfs_cfg->mem_drobj_pool;
    for (i = 0; i < prtfs_cfg->cfg_NDROBJS; i++,pobj++)
    {
        if (pobj->pdrive == pdrive)
            pc_memory_drobj(pobj);
    }
}

/**************************************************************************
    PC_FREE_ALL_I -  Release all inode buffers associated with a drive.
Description
    For each internally buffered finode (dirent) check if it exists on
    pdrive. If so delete it. In debug mode print a message since all
    finodes should be freed before pc_dskclose is called.
Returns
    Nothing
****************************************************************************/

void pc_free_all_i( DDRIVE *pdrive)                             /*__fn__*/
{
    FINODE *pfi;

    OS_CLAIM_FSCRITICAL()
    pfi = prtfs_cfg->inoroot;
    OS_RELEASE_FSCRITICAL()
    while (pfi)
    {
        if (pfi->my_drive == pdrive)
        {
            /* Set the opencount to 1 so freei releases the inode   */
            pfi->opencount = 1;
            pc_freei(pfi);
            /* Since we changed the list go back to the top   */
            OS_CLAIM_FSCRITICAL()
            pfi = prtfs_cfg->inoroot;
            OS_RELEASE_FSCRITICAL()
        }
        else
        {
            OS_CLAIM_FSCRITICAL()
            pfi = pfi->pnext;
            OS_RELEASE_FSCRITICAL()
        }
    }
}


/*****************************************************************************
    PC_FREEI -  Release an inode from service

Description
    If the FINODE structure is only being used by one file or DROBJ, unlink it
    from the internal active inode list and return it to the heap; otherwise
    reduce its open count.

 Returns
    Nothing

****************************************************************************/

void pc_freei( FINODE *pfi)                                     /*__fn__*/
{
    if (!pfi)
    {
        return;
    }
    OS_CLAIM_FSCRITICAL()
    if (pfi->opencount)
    {
        if (--pfi->opencount) /* Decrement opencount and return if non zero */
        {
            OS_RELEASE_FSCRITICAL()
            return;
        }
        else
        {
            if (pfi->pprev) /* Pont the guy behind us at the guy in front*/
            {
                pfi->pprev->pnext = pfi->pnext;
            }
            else
            {

                prtfs_cfg->inoroot = pfi->pnext; /* No prev, we were at the front so
                                        make the next guy the front */
            }

            if (pfi->pnext)         /* Make the next guy point behind */
            {
                pfi->pnext->pprev = pfi->pprev;
            }
        }
    }
    OS_RELEASE_FSCRITICAL()
    /* release the core   */
    pc_memory_finode(pfi);
}

/***************************************************************************
    PC_FREEOBJ -  Free a DROBJ structure

 Description
    Return a drobj structure to the heap. Calls pc_freei to reduce the
    open count of the finode structure it points to and return it to the
    heap if appropriate.

 Returns
    Nothing


****************************************************************************/

void pc_freeobj( DROBJ *pobj)                                   /*__fn__*/
{
    if (pobj)
    {
        pc_freei(pobj->finode);
        /* Release the core   */
        pc_memory_drobj(pobj);
    }
}

/***************************************************************************
    PC_DOS2INODE - Convert a dos disk entry to an in memory inode.

 Description
    Take the data from pbuff which is a raw disk directory entry and copy
    it to the inode at pdir. The data goes from INTEL byte ordering to
    native during the transfer.

 Returns
    Nothing
****************************************************************************/

/* Convert a dos inode to in mem form.  */
void pc_dos2inode (FINODE *pdir, DOSINODE *pbuff)                   /*__fn__*/
{
    copybuff(&pdir->fname[0],&pbuff->fname[0],8);           /*X*/
    /* If the on disk representation is 0x5, change it to 0xE5, a valid
       kanji character */
    if (pdir->fname[0] == 0x5)
        pdir->fname[0] = PCDELETE;

    copybuff(&pdir->fext[0],&pbuff->fext[0],3);             /*X*/
    pdir->fattribute = pbuff->fattribute;                   /*X*/
    copybuff(&pdir->resarea[0],&pbuff->resarea[0], 8);      /*X*/
#if (!KS_LITTLE_ENDIAN)
    pdir->ftime = to_WORD((byte *) &pbuff->ftime);          /*X*/
    pdir->fdate = to_WORD((byte *) &pbuff->fdate);          /*X*/
    /* Note: fclusterhi is of resarea in fat 16 system */
    pdir->fclusterhi = to_WORD((byte *) &pbuff->fclusterhi);
    pdir->fcluster = to_WORD((byte *) &pbuff->fcluster);    /*X*/
    pdir->fsize = to_DWORD((byte *) &pbuff->fsize);     /*X*/
#else
    pdir->ftime = pbuff->ftime;         /*X*/
    pdir->fdate = pbuff->fdate;         /*X*/
    /* Note: fclusterhi is of resarea in fat 16 system */
    pdir->fclusterhi = pbuff->fclusterhi;   /*X*/
    pdir->fcluster = pbuff->fcluster;   /*X*/
    pdir->fsize = pbuff->fsize;     /*X*/
#endif
}

/**************************************************************************
    PC_INIT_INODE -  Load an in memory inode up with user supplied values.

 Description
    Take an uninitialized inode (pdir) and fill in some fields. No other
    processing is done. This routine simply copies the arguments into the
    FINODE structure.

    Note: filename & fileext do not need null termination.

 Returns
    Nothing
****************************************************************************/
/**************************************************************************
    PC_ISAVOL -  Test a DROBJ to see if it is a volume

 Description
    Looks at the appropriate elements in pobj and determines if it is a root
    or subdirectory.

 Returns
    Returns FALSE if the obj does not point to a directory.

****************************************************************************/

BOOLEAN pc_isavol( DROBJ *pobj)                                     /*__fn__*/
{
    if (pobj->finode->fattribute & AVOLUME)
        return(TRUE);
    else
        return(FALSE);
}


/**************************************************************************
    PC_ISADIR -  Test a DROBJ to see if it is a root or subdirectory

 Description
    Looks at the appropriate elements in pobj and determines if it is a root
    or subdirectory.

 Returns
    Returns FALSE if the obj does not point to a directory.

****************************************************************************/

BOOLEAN pc_isadir( DROBJ *pobj)                                     /*__fn__*/
{
    if ( (pobj->isroot) || (pobj->finode->fattribute & ADIRENT)  )
        return(TRUE);
    else
        return(FALSE);
}


/**************************************************************************
    PC_ISROOT -  Test a DROBJ to see if it is the root directory

 Description
    Looks at the appropriate elements in pobj and determines if it is a root
    directory.

 Returns
    Returns NO if the obj does not point to the root directory.

****************************************************************************/

/* Get  the first block of a root or subdir   */
BOOLEAN pc_isroot( DROBJ *pobj)                                        /*__fn__*/
{
    return(pobj->isroot);
}
