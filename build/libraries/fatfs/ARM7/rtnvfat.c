/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* RTNVFAT.C - Contains routines specific to the non-vfat implementation */

#include <rtfs.h>

#if (!VFAT)

/***************************************************************************
    PC_FINDIN -  Find a filename in the same directory as the argument.

 Description
    Look for the next match of filename or pattern filename:ext in the
    subdirectory containing pobj. If found update pobj to contain the
    new information  (essentially getnext.) Called by pc_get_inode().

    Note: Filename and ext must be right filled with spaces to 8 and 3 bytes
            respectively. Null termination does not matter.

    Note the use of the action variable. This is used so we do not have to
    use match patterns in the core code like *.* and ..
    GET_INODE_MATCH   Must match the pattern exactly
    GET_INODE_WILD    Pattern may contain wild cards
    GET_INODE_STAR    Like he passed *.* (pattern will be null)
    GET_INODE_DOTDOT  Like he past .. (pattern will be null

 Returns
    Returns TRUE if found or FALSE.

****************************************************************************/
/* Find filename in the directory containing pobj. If found, load the inode
section of pobj. If the inode is already in the inode buffers we free the current inode
and stitch the existing one in, bumping its open count */
BOOLEAN pc_findin( DROBJ *pobj, byte *filename, byte *fileext, int action)          /*__fn__*/
{
    BLKBUFF *rbuf;
    DIRBLK *pd;
    DOSINODE *pi;
    FINODE *pfi;
    BOOLEAN matchfound;
    BOOLEAN dowildcard;

    if (action == GET_INODE_WILD)
        dowildcard = TRUE;
    else
        dowildcard = FALSE;

    rtfs_set_errno(0);  /* Clear it here just in case */
    /* For convenience. We want to get at block info here   */
    pd = &pobj->blkinfo;

    /* Read the data   */
    pobj->pblkbuff = rbuf = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);

    while (rbuf)
    {
        pi = (DOSINODE *) &rbuf->data[0];

        /* Look at the current inode   */
        pi += pd->my_index;

        /* And look for a match   */
        while ( pd->my_index < INOPBLOCK )
        {
            matchfound = FALSE;
            /* End of dir if name is 0   */
            if (!pi->fname[0])
            {
                rtfs_set_errno(PENOENT); /* pc_findin: file or dir not found */
                pc_release_buf(rbuf);
                return(FALSE);
            }
            if (pi->fattribute != CHICAGO_EXT  && pi->fname[0] != PCDELETE)
            {
                if (action == GET_INODE_STAR)
                    matchfound = TRUE;
                else if (action == GET_INODE_DOTDOT)
                {
                    if (pi->fname[0] == CS_OP_ASCII('.') && pi->fname[1] == CS_OP_ASCII('.'))
                        matchfound = TRUE; /* 8.3, not lfn */
                }
                else
                {
                    matchfound =
                        ( pc_patcmp_8(pi->fname, (byte*) filename, dowildcard) &&
                            pc_patcmp_3(pi->fext,  (byte*) fileext, dowildcard ) );
                }
	        if (matchfound && pi->fattribute & AVOLUME &&
	            action != GET_INODE_STAR && action != GET_INODE_WILD)
	        {
	            /* Don't match volume labels if we are finding a specific match. */
	            matchfound = FALSE;
	        }
            }
            if (matchfound)
            {
                /* We found it   */
                /* See if it already exists in the inode list.
                If so.. we use the copy from the inode list */
                pfi = pc_scani(pobj->pdrive, rbuf->blockno, pd->my_index);

                if (pfi)
                {
                    pc_freei(pobj->finode);
                    pobj->finode = pfi;
                }
                else    /* No inode in the inode list. Copy the data over
                        and mark where it came from */
                {
                    pfi = pc_alloci();
                    if (pfi)
                    {
                        pc_freei(pobj->finode); /* Release the current */
                        pobj->finode = pfi;
                        pc_dos2inode(pobj->finode , pi );
                        pc_marki(pobj->finode , pobj->pdrive , pd->my_block,
                                pd->my_index );
                    }
                    else
                    {
                        pc_release_buf(rbuf);
                        return (FALSE);
                    }
                }
                /* Free, no error   */
                pc_release_buf(rbuf);
                return (TRUE);
            }                   /* if (match) */
            pd->my_index++;
            pi++;
        }
        /* Not in that block. Try again   */
        pc_release_buf(rbuf);
        /* Update the objects block pointer   */
        if (!pc_next_block(pobj))
            break;
        pd->my_index = 0;
        pobj->pblkbuff = rbuf = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    }
    if (!get_errno())
        rtfs_set_errno(PENOENT); /* pc_findin: file or dir not found */
    return (FALSE);
}

/***************************************************************************
    PC_INSERT_INODE - Insert a new inode into an existing directory inode.

Description
    Take mom , a fully defined DROBJ, and pobj, a DROBJ with a finode
    containing name, ext, etc, but not yet stitched into the inode buffer
    pool, and fill in pobj and its inode, write it to disk and make the inode
    visible in the inode buffer pool. (see also pc_mknode() )


Returns
    Returns TRUE if all went well, FALSE on a write error, disk full error or
    root directory full.

**************************************************************************/
/* Note: the parent directory is locked before this routine is called   */

BOOLEAN pc_insert_inode(DROBJ *pobj , DROBJ *pmom, byte attr, CLUSTERTYPE initcluster, byte *filename, byte *fileext)
{
    BLKBUFF *pbuff;
    DIRBLK *pd;
    DOSINODE *pi;
    int i;
    CLUSTERTYPE cluster;
    DDRIVE *pdrive;
    DATESTR crdate;

    pc_init_inode( pobj->finode, filename, fileext,
                    attr, initcluster, /*size*/ 0L ,pc_getsysdate(&crdate) );

    /* Set up pobj      */
    pdrive = pobj->pdrive = pmom->pdrive;
    pobj->isroot = FALSE;
    pd = &pobj->blkinfo;

    /* Now get the start of the dir   */
    pd->my_block = pd->my_frstblock = pc_firstblock(pmom);

    if (!pd->my_block)
    {
        rtfs_set_errno(PEINVALIDBLOCKNUMBER);   /* pc_insert_inode: Internal error, invalid block id */
        return (FALSE);
    }
    else
        pd->my_index = 0;

    /* Read the data   */
    pobj->pblkbuff = pbuff = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    if (!pbuff)
        return(FALSE);
    while (pbuff)
    {
        i = pd->my_index = 0;
        pi = (DOSINODE *) &pbuff->data[0];
        /* look for a slot   */
        while ( i < INOPBLOCK )
        {
            /* End of dir if name is 0   */
            if ( (pi->fname[0] == CS_OP_ASCII('\0')) || (pi->fname[0] == PCDELETE) )
            {
                pd->my_index = (word)i;
                /* Update the DOS disk   */
                pc_ino2dos( pi, pobj->finode );
                /* Write the data   */
                if (pc_write_blk(pbuff))
                {
                    /* Mark the inode in the inode buffer   */
                    pc_marki(pobj->finode , pobj->pdrive , pd->my_block,
                            pd->my_index );
                    pc_release_buf(pbuff);
                    return(TRUE);
                }
                else
                {
                    pc_discard_buf(pbuff);
                    return(FALSE);
                }
            }
            i++;
            pi++;
        }
        /* Not in that block. Try again   */
        pc_release_buf(pbuff);
        /* Update the objects block pointer   */
        rtfs_set_errno(0);  /* Clear errno to be safe */
        if (!pc_next_block(pobj))
        {
            if (get_errno())
                return(FALSE);
            else
                break;
        }
        pobj->pblkbuff = pbuff = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    }

    cluster = pc_grow_dir(pdrive, pmom);
    if (!cluster)
        return (FALSE);
    /* Do not forget where the new item is   */
    pd->my_block = pc_cl2sector(pobj->pdrive , cluster);
    pd->my_index = 0;

    /* Zero out the cluster    */
    if (!pc_clzero( pobj->pdrive , cluster ) )
        goto clean_and_fail;
    /* Copy the item into the first block   */
    pbuff = pc_init_blk( pobj->pdrive , pd->my_block);
    if (!pbuff)
        goto clean_and_fail;
    pc_ino2dos (  (DOSINODE *) &pbuff->data[0] ,  pobj->finode ) ;
    /* Write it out   */
    if ( !pc_write_blk ( pbuff ) )
    {
        pc_discard_buf(pbuff);
        goto clean_and_fail;
    }

    /* We made a new slot. Mark the inode as belonging there   */
    pc_marki(pobj->finode , pobj->pdrive , pd->my_block, pd->my_index );
    pc_release_buf(pbuff);
    return (TRUE);
clean_and_fail:
    pc_truncate_dir(pdrive, pmom, cluster);
    return (FALSE);
}

/****************************************************************************
PC_NIBBLEPARSE -  Nibble off the left most part of a pathspec

  Description
  Take a pathspec (no leading D:). and parse the left most element into
  filename and files ext. (SPACE right filled.).

    Returns
    Returns a pointer to the rest of the path specifier beyond file.ext
    ****************************************************************************/
    /* Parse a path. Return NULL if problems or a pointer to the next */
byte *pc_nibbleparse(byte *filename, byte *fileext, byte *path)     /* __fn__*/
{
    byte *p;
    BLKBUFF *scratch;
    byte *tbuf;
    byte *t;

    p = path;

    if (!p)  /* Path must exist */
        return (0);
    scratch = pc_scratch_blk();
    if (!scratch)
        return(0);
    tbuf = scratch->data;
    t = tbuf;

    while (CS_OP_IS_NOT_EOS(p))
    {
        if (CS_OP_CMP_ASCII(p,'\\'))
        {
            CS_OP_INC_PTR(p);
            break;
        }
        else
        {
            CS_OP_CP_CHR(t, p);
            CS_OP_INC_PTR(t);
            CS_OP_INC_PTR(p);
        }
    }
    CS_OP_TERM_STRING(t);

    if (pc_ascii_fileparse(filename, fileext, tbuf))
    {
        pc_free_scratch_blk(scratch);
        return (p);
    }
    else
    {
        pc_free_scratch_blk(scratch);
        return (0);
    }
}

BOOLEAN pc_parsepath(byte *topath, byte *filename, byte *fileext, byte *path) /*__fn__*/
{
    byte *pfile, *pto, *pfr, *p, *pslash, *pcolon;
    int i;

    /* Check the path length - compare the string length in bytes 
       against the max path in chars. This is correct since in
       ASCII charlen==bytelen, in JIS charlen does not equal bytelen
       but we don't want the string to be > the char len */
    if (rtfs_cs_strlen_bytes(path) > EMAXPATH_CHARS)
        return(FALSE);

    pslash = pcolon = 0;
    pfr = path;
    pto = topath;
    /* Copy input path to output keep note colon and backslash positions */

    for (i = 0; CS_OP_IS_NOT_EOS(pfr); i++,CS_OP_INC_PTR(pfr),CS_OP_INC_PTR(pto))
    {
        CS_OP_CP_CHR(pto, pfr);
        if (CS_OP_CMP_ASCII(pto,'\\'))
            pslash = pto;
        else if (CS_OP_CMP_ASCII(pto,':'))
        {
            if (i != 1)     /* A: B: C: .. x: y: z: only */
                return (FALSE);
            pcolon = pto;
            CS_OP_INC_PTR(pcolon); /* Look one past */
        }
    }
    CS_OP_TERM_STRING(pto);

    if (pslash)
    {
        pfile = pslash;
        CS_OP_INC_PTR(pfile);
    }
    else if (pcolon)
        pfile = pcolon;
    else
        pfile = topath;

    if (!pc_ascii_fileparse(filename, fileext, pfile))
        return (FALSE);
        /* Terminate path:
        If X:\ or \ leave slash on. Else zero it
    */
    p = topath; /* Default */
    if (!pslash)
    {
        if (pcolon) p = pcolon;
    }
    else /* if slash. and at 0 or right after colon leave else zero it */
    {
        p = pslash;
        /*    \         or  A:\ */
        if (p == topath || p == pcolon)
        {
            CS_OP_INC_PTR(p); /* (leave it in path) */
        }
    }
    CS_OP_TERM_STRING(p);
    return(TRUE);
}
/* Byte oriented */
BOOLEAN _illegal_lfn_char(byte ch)
{
    RTFS_ARGSUSED_INT((int) ch);
    return(FALSE);
}

static BOOLEAN pc_allspace(byte *p, int i)                                                             /* __fn__*/
{while (i--) if (*p++ != ' ') return (FALSE);   return (TRUE); }
BOOLEAN pc_isdot(byte *fname, byte *fext)                                               /* __fn__*/
{
    return (BOOLEAN)((*fname == '.') &&
        pc_allspace(fname+1,7) && pc_allspace(fext,3) );
}
BOOLEAN pc_isdotdot(byte *fname, byte *fext)                                            /* __fn__*/
{
    return (BOOLEAN)( (*fname == '.') && (*(fname+1) == '.') &&
        pc_allspace(fname+2,6) && pc_allspace(fext,3) );
}
BOOLEAN pc_delete_lfn_info(DROBJ *pobj)
{
    RTFS_ARGSUSED_PVOID((void *) pobj);
     return(TRUE);
}
void pc_zero_lfn_info(FINODE *pdir)
{
    RTFS_ARGSUSED_PVOID((void *) pdir);
}
BOOLEAN pc_get_lfn_filename(DROBJ *pobj, byte *path)
{
    RTFS_ARGSUSED_PVOID((void *) pobj);
    RTFS_ARGSUSED_PVOID((void *) path);
    return(FALSE);
}
dword scan_for_bad_lfns(DROBJ *pmom, int delete_bad_lfn)          /*__fn__*/
{
    RTFS_ARGSUSED_PVOID((void *) pmom);
    RTFS_ARGSUSED_INT(delete_bad_lfn);
    return(0);
}

#endif /* #if (!VFAT) */



