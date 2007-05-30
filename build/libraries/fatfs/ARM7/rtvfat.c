/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* RTVFAT.C - Contains routines specific to the vfat implementation */

#include <rtfs.h>

#if (VFAT)

#define FIRST_NAMESEG 0x40
#define NAMESEG_ORDER 0x3F

/* LFNINODE - This is an image of lfn extended names in a subdirectory */
/* Note: lfn file names are null terminated unicode 00, the lfninode is
   padded mod 13 with ffff */
typedef struct lfninode {
                              /* The first LNF has 0x40 + N left */
        byte    lfnorder;     /* 0x45, 0x04, 0x03, 0x02, 0x01 they are stored in
                               reverse order */
        byte    lfname1[10];
        byte    lfnattribute; /* always 0x0F */
        byte    lfnres;       /* reserved */
        byte    lfncksum;   /* All lfninode in one dirent have the same chksum */
        byte    lfname2[12];
        word    lfncluster; /* always 0x0000 */
        byte    lfname3[4];
        } LFNINODE;

void pcdel2lfi(LFNINODE *lfi, int nsegs);
BOOLEAN pc_deleteseglist(DDRIVE *pdrive, SEGDESC *s);
byte *text2lfi(byte *lfn, LFNINODE *lfi, int nsegs, byte ncksum, byte order);
BOOLEAN pc_seglist2disk(DDRIVE * pdrive, SEGDESC *s, byte *lfn);
byte *lfi2text(byte *lfn, int *current_lfn_length, LFNINODE *lfi, int nsegs);
byte *pc_seglist2text(DDRIVE * pdrive, SEGDESC *s, byte *lfn);
void pc_zeroseglist(SEGDESC *s);
void pc_zeroseglist(SEGDESC *s);
void pc_addtoseglist(SEGDESC *s, BLOCKT my_block, int my_index);
void pc_reduceseglist(SEGDESC *s);
BOOLEAN pc_patcmp_vfat_8_3(byte *pat, byte *name, BOOLEAN dowildcard);

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
    BLKBUFF *scratch;
    byte *lfn;
    byte sfn[26]; /* Leave room in case unicode */
    LFNINODE *lfn_node;
    SEGDESC s;
    byte lastsegorder;
    byte *p;

    RTFS_ARGSUSED_PVOID((void *) fileext);
    rtfs_set_errno(0);  /* Clear it here just in case */
    scratch = pc_scratch_blk();
    if (!scratch)
        return(FALSE);
    lfn = (byte *)scratch->data;

    pc_zeroseglist(&s);

    if (action == GET_INODE_WILD)
        dowildcard = TRUE;
    else
        dowildcard = FALSE;

    /* For convenience. We want to get at block info here   */
    pd = &pobj->blkinfo;

    /* Read the data   */
    pobj->pblkbuff = rbuf = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);

    lastsegorder = 0;
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
                pc_free_scratch_blk(scratch);
                pc_release_buf(rbuf);
                return(FALSE);
            }

            if (pi->fname[0] == PCDELETE)
            {
                pc_zeroseglist(&s);
            }
            else
            {
                /* Support long file names   */
                if (pi->fattribute == CHICAGO_EXT)
                {
                    lfn_node = (LFNINODE *) pi;
                    if (lfn_node->lfnorder & FIRST_NAMESEG)
                    {
                        pc_addtoseglist(&s, pd->my_block, pd->my_index);
                        /*  .. we could build up the name here too    */
                        lastsegorder = lfn_node->lfnorder;
                        s.ncksum = lfn_node->lfncksum;
                    }
                    else
                    {
                        if (s.nsegs)/* if a chain already exists */
                        {
                            if ( ((lfn_node->lfnorder & NAMESEG_ORDER) == (lastsegorder & NAMESEG_ORDER) - 1) &&
                                  (lfn_node->lfncksum == s.ncksum) )
                            {
                                /* Add new segment to lfn chain   */
                                lastsegorder = lfn_node->lfnorder;
                                pc_addtoseglist(&s, pd->my_block, pd->my_index);
                            }
                            else
                            {
                               /* disconnect chain... segments do not match   */
                                lastsegorder = 0;
                                pc_zeroseglist(&s);
                            }
                        }
                    }
                }
                else
                {
               /* Note: Patcmp wo not match on deleted   */
                    if (s.nsegs)
                    {
                        if (action == GET_INODE_STAR)
                            matchfound = TRUE;
                        else if (action == GET_INODE_DOTDOT)
                            matchfound = FALSE; /* 8.3, not lfn */
                        else
                        {
                            p = (byte*)pc_seglist2text(pobj->pdrive, &s,lfn);
                            if (!p)
                                matchfound = FALSE; /* 8.3, not lfn */
                            else
                                matchfound = pc_patcmp_vfat(filename, p ,dowildcard);
                        }
                    }
                    else
                        matchfound = FALSE; /* 8.3, not lfn */

                    if (matchfound)
                        matchfound = (BOOLEAN)(pc_cksum( (byte*) pi ) == s.ncksum);
                    else
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
                            pc_cs_mfile(sfn,pi->fname,pi->fext);
                            matchfound = pc_patcmp_vfat_8_3(filename, sfn, dowildcard);
                        }
                    }
                    if (matchfound && pi->fattribute & AVOLUME &&
                        action != GET_INODE_STAR && action != GET_INODE_WILD)
                    {
                        /* Don't match volume labels if we are finding a specific match. */
                        matchfound = FALSE;
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
                                if (s.nsegs)
                                {
                                    if (pc_cksum( (byte*) pi ) != s.ncksum)
                                    {
                                        pc_zeroseglist(&s);
                                        lastsegorder = 0;
                                    }
                                    pobj->finode->s = s;
                                }
                                else
                                    pc_zeroseglist(&pobj->finode->s);
                            }
                            else
                            {
                                pc_free_scratch_blk(scratch);
                                pc_release_buf(rbuf);
                                return (FALSE);
                            }
                        }
                        /* Free, no error   */
                        pc_free_scratch_blk(scratch);
                        pc_release_buf(rbuf);
                        return (TRUE);
                    }                   /* if (match) */
                    else /* disconnect chain... segments do not match */
                    {
                        pc_zeroseglist(&s);
                    }
                }               /* else (CHICAGO_EXT) */
            }                   /* if (!PCDELETE) */
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
    pc_free_scratch_blk(scratch);
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
    byte vffilename[9],vffileext[4];
    byte cksum;
    BOOLEAN end_of_dir;
    int n_segs;

    cluster = 0;
    /* How many segments do we need   */
    n_segs = (rtfs_cs_strlen(filename) + 12 )/13;

    RTFS_ARGSUSED_PVOID((void *) fileext);
    if (!pc_malias(vffilename,vffileext, (byte *)filename,pmom))
        return (FALSE); /* pc_malias set errno */

    pc_init_inode( pobj->finode, vffilename, vffileext,
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
    /* If the alias contains Kanji E5 this goes on disk as 05 so we have to
       checksum it with 05 */
    if (pobj->finode->fname[0] == 0xe5)
    {
        pobj->finode->fname[0] = 0x05;
        cksum = pc_cksum((byte*)pobj->finode);
        pobj->finode->fname[0] = 0xe5;
    }
    else
         cksum = pc_cksum((byte*)pobj->finode);
    end_of_dir = FALSE;
    pc_zeroseglist(&pobj->finode->s);
    pobj->finode->s.ncksum = cksum;
    while (pbuff)
    {
        i = pd->my_index = 0;
        pi = (DOSINODE *) &pbuff->data[0];
        /* look for a slot   */
        while ( i < INOPBLOCK )
        {
            if (pi->fname[0] == CS_OP_ASCII('\0'))
                end_of_dir = TRUE;
            if (pi->fname[0] == PCDELETE || end_of_dir)
            {
                /* Note that we fake an allocation of n_segs + 1 segments.
                   this scheme makes sure that there is room for the DOSINODE
                   immediately after the segment list. We reduce the segment
                   count before we write the segments */
                if (pobj->finode->s.nsegs != (n_segs +1))
                {
                    pc_addtoseglist(&pobj->finode->s, pd->my_block, i);
                }
                /* Write the lfn and dos inode if we have the space   */
                if (pobj->finode->s.nsegs == (n_segs +1))
                {
                    /* Write the long file name entries to their respective
                        locations */
                    /* Drop the segment count by one before we write.
                       with this scheme we fall out of the loop and
                       put the DOSINODE in the correct location */
                    pc_reduceseglist(&pobj->finode->s);
                    if (!pc_seglist2disk(pobj->pdrive, &pobj->finode->s, filename))
                    {
                        pc_discard_buf(pbuff);
                        goto clean_and_fail;
                    }
                    pd->my_index = i;
                    /* Update the DOS disk   */
                    pc_ino2dos( pi, pobj->finode );
                    /* Write the data   */
                    /* Mark the inode in the inode buffer   */
                    if (pc_write_blk(pbuff))
                    {
                        pc_marki(pobj->finode , pobj->pdrive , pd->my_block,
                               pd->my_index );
                        pc_release_buf(pbuff);
                        return(TRUE);
                    }
                    else
                    {
                        pc_release_buf(pbuff);
                        return(FALSE);
                    }
                }
            }
            else
            {
                pc_zeroseglist(&pobj->finode->s);
            }
            i++;
            pi++;
        }
        /* Not in that block. Try again   */
        pc_release_buf(pbuff);
        /* Update the objects block pointer   */
        rtfs_set_errno(0);  /* Should already be 0 but don't chance it */
        if (!pc_next_block(pobj))
        {
            if (get_errno())    /* The zero was an error return */
                return (FALSE);
            /* Ok:There are no slots in mom. We have to make one. And copy our stuff in  */
            cluster = pc_grow_dir(pobj->pdrive, pmom);
            if (!cluster)
                return (FALSE);

            /* Do not forget where the new item is   */
            pd->my_block = pc_cl2sector(pobj->pdrive , cluster);
            pd->my_index = 0;

            /* Zero out the cluster    */
            if (!pc_clzero( pobj->pdrive , cluster ) )
                goto clean_and_fail;

            /* Copy the item into the first block   */
            pobj->pblkbuff = pbuff = pc_init_blk( pobj->pdrive , pd->my_block);
            if (!pbuff)
                goto clean_and_fail;
        }
        else    /* 02-06-2003 - Added Else, had been lost */
            pobj->pblkbuff = pbuff = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    }

clean_and_fail:
    if (cluster)
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
    byte *t = filename;

    RTFS_ARGSUSED_PVOID((void *)fileext);

    p = path;

    if (!p)  /* Path must exist */
        return (0);
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

    return (p);
}



/*****************************************************************************
PC_CKSUM - Compute checksum on string

  Description
  Computes a single UTINY checksum based on the first 11 characters in
  the input string (test).  This value is used to link LFNINODE's on the
  disk directory table.
  Returns
  The value of the checksum
*****************************************************************************/

byte pc_cksum(byte *test)   /*__fn__*/
{
    byte  sum,i;

    for (sum = i = 0; i < 11; i++)
    {
        sum = (byte)((((sum & 0x01)<<7)|((sum & 0xFE)>>1)) + test[i]);
    }

    return(sum);
}

void pcdel2lfi(LFNINODE *lfi, int nsegs)    /* __fn__ */
{
    for (;nsegs;nsegs--, lfi--)
        lfi->lfnorder = PCDELETE;
}


BOOLEAN pc_deleteseglist(DDRIVE *pdrive, SEGDESC *s)  /* __fn__ */
{
BLKBUFF *rbuf;
LFNINODE *lfi;
int ntodo_0,ntodo_1,ntodo_2;

    if (!s->nsegs)
        return(TRUE);
    /* Read segblock[0] and copy text   */
    rbuf = pc_read_blk(pdrive, s->segblock[0]);
    if (!rbuf)
        return(FALSE);
    lfi = (LFNINODE *) &rbuf->data[0];
    lfi += s->segindex;
    /* If the lfn segments span two or more blocks then segblock[0] contains the
       last block, segblock[1] contains next to last, segblock[3] contains
        the last if there are three. We delete all of the segments up to
        and including segindex in last block value stored in segblock[0]. */
    if (s->nsegs > s->segindex+1)
        ntodo_0 = s->segindex+1;
    else
        ntodo_0 = s->nsegs;

    if (ntodo_0 > 0) /* Test just in case should never be <= 0 */
        pcdel2lfi(lfi, ntodo_0);
    if ( !pc_write_blk(rbuf) )
    {
        pc_discard_buf(rbuf);
        return (FALSE);
    }
    else
    {
        pc_release_buf(rbuf);
    }
       if (s->segblock[1])
    {
           rbuf = pc_read_blk(pdrive, s->segblock[1]);
           if (!rbuf)
               return (FALSE);
        lfi = (LFNINODE *) &rbuf->data[0];
        lfi += 15;  /* The last index */

        /* Delete the next N segments. Clip it at 16 since there are only
           16 segments per block */
        ntodo_1 = s->nsegs - ntodo_0;
        if (ntodo_1 > 16)
            ntodo_1 = 16;

        if (ntodo_1 > 0) /* Test just in case should never be <= 0 */
            pcdel2lfi(lfi, ntodo_1);
        if ( !pc_write_blk ( rbuf ) )
        {
            pc_discard_buf(rbuf);
            return (FALSE);
        }
        else
            pc_release_buf(rbuf);
           if (s->segblock[2])
        {
               rbuf = pc_read_blk(pdrive, s->segblock[2]);
               if (!rbuf)
                   return (FALSE);
            lfi = (LFNINODE *) &rbuf->data[0];
            lfi += 15;  /* The last index */

            ntodo_2 = s->nsegs - (ntodo_1 + ntodo_0);
            /* Delete the next N segments. Clip it at 16 since there are only
            16 segments per block - this should not happen but just to be safe*/
            if (ntodo_2 > 16)
                ntodo_2 = 16;

            if (ntodo_2 > 0) /* Test just in case should never be <= 0 */
                pcdel2lfi(lfi, ntodo_2);
            if ( !pc_write_blk ( rbuf ) )
            {
                pc_discard_buf(rbuf);
                return (FALSE);
            }
            else
                pc_release_buf(rbuf);
        }
    }
    return(TRUE);
}

byte *text2lfi(byte *lfn, LFNINODE *lfi, int nsegs, byte ncksum, byte order) /* __fn__ */
{
    int n;
    BOOLEAN end_of_lfn = FALSE;
    byte *pfi;

    for (;nsegs && !end_of_lfn;nsegs--, lfi--, order++)
    {
        pfi = lfi->lfname1;
        for(n=0; n<10; n += 2, pfi += 2)
        {
            if (end_of_lfn)
                *pfi = *(pfi+1) = 0xff;
            else
            {
                CS_OP_TO_LFN(pfi, lfn);
                CS_OP_INC_PTR(lfn);
                if ((*pfi== 0) && (*(pfi+1) == 0))
                    end_of_lfn = TRUE;
            }
        }
        pfi = lfi->lfname2;
        for(n=0; n<12; n += 2, pfi += 2)
        {
            if (end_of_lfn)
                *pfi = *(pfi+1) = 0xff;
            else
            {
                CS_OP_TO_LFN(pfi, lfn);
                CS_OP_INC_PTR(lfn);
                if ((*pfi== 0) && (*(pfi+1) == 0))
                    end_of_lfn = TRUE;
            }
        }
        pfi = lfi->lfname3;
        for(n=0; n<4; n += 2, pfi += 2)
        {
            if (end_of_lfn)
                *pfi = *(pfi+1) = 0xff;
            else
            {
                CS_OP_TO_LFN(pfi, lfn);
                CS_OP_INC_PTR(lfn);
                if ((*pfi== 0) && (*(pfi+1) == 0))
                    end_of_lfn = TRUE;
            }
        }
        if (CS_OP_IS_EOS(lfn))
        {
            end_of_lfn = TRUE;
        }
        if (end_of_lfn)
            order |= FIRST_NAMESEG;
        lfi->lfnorder = order;
        lfi->lfnattribute = 0x0F;
        lfi->lfnres =  0;
        lfi->lfncksum = ncksum;
        lfi->lfncluster = 0x0000;
    }
    return(lfn);
}

BOOLEAN pc_seglist2disk(DDRIVE * pdrive, SEGDESC *s, byte *lfn) /* __fn__*/
{
BLKBUFF *rbuf;
LFNINODE *lfi;
int ntodo_0, ntodo_1, ntodo_2;
byte *psegtext;
    if (!s->nsegs)
        return(FALSE);
    /* Read segblock[0] and copy text   */
    rbuf = pc_read_blk(pdrive, s->segblock[0]);
    if (!rbuf)
        return(FALSE);
    lfi = (LFNINODE *) &rbuf->data[0];
    lfi += s->segindex;
    /* If the lfn segments span two or more blocks then segblock[0]
       contains the last block and segblock[1] contains the next to last
       if there are 3 blocks segblock[2] contains the first */
    if (s->nsegs > s->segindex+1)
        ntodo_0 = s->segindex+1;
    else
        ntodo_0 = s->nsegs;
    psegtext = lfn; /* Initialize the variable so compile doesn't complain */
    if (ntodo_0 > 0) /* should always be true */
        psegtext = text2lfi(lfn, lfi, ntodo_0, s->ncksum, 1);
    if ( !pc_write_blk ( rbuf ) )
    {
        pc_discard_buf(rbuf);
        return (FALSE);
    }
    else
        pc_release_buf(rbuf);

    if (s->segblock[1])
    {
        rbuf = pc_read_blk(pdrive, s->segblock[1]);
        if (!rbuf)
            return(FALSE);
        lfi = (LFNINODE *) &rbuf->data[0];
        lfi += 15;  /* The last index */
        /* Do 16 more segments (means more blocks to follow) or whatever is
           left */
        ntodo_1 = s->nsegs - ntodo_0;
        if (ntodo_1 > 16)
            ntodo_1 = 16;
        if (ntodo_1 > 0) /* Should always be true */
            psegtext = text2lfi(psegtext, lfi, ntodo_1, s->ncksum, (byte) (ntodo_0+1));
        if ( !pc_write_blk ( rbuf ) )
        {
            pc_discard_buf(rbuf);
            return (FALSE);
        }
        else
            pc_release_buf(rbuf);

        if (s->segblock[2])
        {
            rbuf = pc_read_blk(pdrive, s->segblock[2]);
            if (!rbuf)
                return(FALSE);
            lfi = (LFNINODE *) &rbuf->data[0];
            lfi += 15;  /* The last index */
            /* Do 16 more segments (means more blocks to follow) or whatever is
            left */
            ntodo_2 = s->nsegs - (ntodo_1 + ntodo_0);
            if (ntodo_2 > 16) /* Should always be <= 16 */
                ntodo_2 = 16;
            if (ntodo_2 > 0) /* Should always be true */
                psegtext = text2lfi(psegtext, lfi, ntodo_2, s->ncksum, (byte) (ntodo_1+ntodo_0+1));
            if ( !pc_write_blk ( rbuf ) )
            {
                pc_discard_buf(rbuf);
                return (FALSE);
            }
            else
                pc_release_buf(rbuf);
        }
    }
    return(TRUE);
}



byte *lfi2text(byte *lfn, int *current_lfn_length, LFNINODE *lfi, int nsegs) /* __fn__ */
{
    int n;
    byte *pfi;

    for (;nsegs;nsegs--, lfi--)
    {
        pfi = (byte *) lfi->lfname1;
        for(n=0; n<10; n += 2,pfi+=2)
        {
            if((*pfi==0x00)&&(*(pfi+1)==0x00))
                goto  lfi2text_eos;
            if (*current_lfn_length == FILENAMESIZE_CHARS)
                 return(0);
            CS_OP_LFI_TO_TXT(lfn, pfi);
            CS_OP_INC_PTR(lfn);
            *current_lfn_length += 1;
        }
        pfi = (byte *) lfi->lfname2;
        for(n=0; n<12; n += 2,pfi+=2)
        {
            if((*pfi==0x00)&&(*(pfi+1)==0x00))
                goto  lfi2text_eos;
            if (*current_lfn_length == FILENAMESIZE_CHARS)
                 return(0);
            CS_OP_LFI_TO_TXT(lfn, pfi);
            CS_OP_INC_PTR(lfn);
            *current_lfn_length += 1;
        }
        pfi = (byte *) lfi->lfname3;
        for(n=0; n<4; n += 2,pfi+=2)
        {
            if((*pfi==0x00)&&(*(pfi+1)==0x00))
                goto  lfi2text_eos;
            if (*current_lfn_length == FILENAMESIZE_CHARS)
                 return(0);
            CS_OP_LFI_TO_TXT(lfn, pfi);
            CS_OP_INC_PTR(lfn);
            *current_lfn_length += 1;
        }
        CS_OP_TERM_STRING(lfn);
    }
    return(lfn);
lfi2text_eos:
    CS_OP_TERM_STRING(lfn);
    return(lfn);
}

byte *pc_seglist2text(DDRIVE * pdrive, SEGDESC *s, byte *lfn) /* __fn__ */
{
BLKBUFF *rbuf;
LFNINODE *lfi;
byte *p;
int ntodo_0,ntodo_1,ntodo_2;
int current_lfn_length;

    CS_OP_TERM_STRING(lfn);
    p = lfn;
    if (!s->nsegs)
        goto sl2_done;
    /* Read segblock[0] and copy text   */
    rbuf = pc_read_blk(pdrive, s->segblock[0]);
    if (!rbuf)
        goto sl2_done;
    lfi = (LFNINODE *) &rbuf->data[0];
    lfi += s->segindex;
    /* If the lfn segments span two or blocks then segblock[0] contains the
       last block and segblock[1] contains next to last and if their are
       three segblock[2] contains the first. */
    if (s->nsegs > s->segindex+1)
        ntodo_0 = s->segindex+1;
    else
        ntodo_0 = s->nsegs;
    current_lfn_length = 0;
    p = lfi2text(p, &current_lfn_length, lfi, ntodo_0);
    pc_release_buf(rbuf);
    if (!p)
        return(0);
    if (s->segblock[1])
    {
        rbuf = pc_read_blk(pdrive, s->segblock[1]);
        if (!rbuf)
            goto sl2_done;
        lfi = (LFNINODE *) &rbuf->data[0];
        lfi += 15;  /* The last index */
        /* Read the next N segments. Clip it at 16 since there are only
           16 segments per block */
        ntodo_1 = s->nsegs - ntodo_0;
        if (ntodo_1 > 16)
            ntodo_1 = 16;
        if (ntodo_1)
            p = lfi2text(p, &current_lfn_length, lfi, ntodo_1);
        pc_release_buf(rbuf);
        if (!p)
            return(0);

        if (s->segblock[2])
        {
            rbuf = pc_read_blk(pdrive, s->segblock[2]);
            if (!rbuf)
                goto sl2_done;
            lfi = (LFNINODE *) &rbuf->data[0];
            lfi += 15;  /* The last index */
        /* Read the next N segments. Clip it at 16 since there are only
           16 segments per block */
            ntodo_2 = s->nsegs - (ntodo_1 + ntodo_0);
            if (ntodo_2 > 16)
                ntodo_2 = 16;
            if (ntodo_2)
                p = lfi2text(p, &current_lfn_length, lfi, ntodo_2);
            pc_release_buf(rbuf);
            if (!p)
                return(0);
        }
    }
sl2_done:
    return(lfn);
}

void pc_zeroseglist(SEGDESC *s)  /* __fn__ */
{
/* Note: we do not zero the checksum field here   */
    s->nsegs = 0;
    s->segblock[0] =
    s->segblock[1] =
    s->segblock[2] =
    s->segindex = 0;
}
void pc_addtoseglist(SEGDESC *s, BLOCKT my_block, int my_index) /*__fn__*/
{
    s->nsegs += 1;
    /* The block list is a LIFO stack so if it's empty start it
       otherwise ripple copy in */
    if (!s->segblock[0])
    {
        s->segblock[0] = my_block;
    }
    else if ( s->segblock[0] != my_block &&
              s->segblock[1] != my_block &&
              s->segblock[2] != my_block)
    {
        s->segblock[2] = s->segblock[1];
        s->segblock[1] = s->segblock[0];
        s->segblock[0] = my_block;
    }
    s->segindex = my_index;
}

/* This function is used by pc_insert_inode(). That function builds a seglist
   that is 1 segment longer then the lfn. The extra segment is where the
   dosinode will be placed. Before we write the the lfn to disk we call this
   function to reduce the segment list by one. */

void pc_reduceseglist(SEGDESC *s) /*__fn__ */
{
    if (s->nsegs)                   /* This should always be true */
    {
        s->nsegs -= 1;
        if (s->segblock[2] && s->segindex == 0)
        {
            s->segblock[0] = s->segblock[1];
            s->segblock[1] = s->segblock[2];
            s->segblock[2] = 0;
            s->segindex = INOPBLOCK-1;
        }
        else if (s->segblock[1] && s->segindex == 0)
        {
            s->segblock[0] = s->segblock[1];
            s->segblock[1] = s->segblock[2] = 0;
            s->segindex = INOPBLOCK-1;
        }
        else
        {
            if (s->segindex)        /* This should always be true */
                s->segindex -= 1;
        }
    }
}

/****************************************************************************
PC_PARSEPATH -  Parse a path specifier into path,file,ext

  Description
  Take a path specifier in path and break it into three null terminated
  strings topath,filename and file ext.
  The result pointers must contain enough storage to hold the results.
  Filename and fileext are BLANK filled to [8,3] spaces.

    Rules:

      SPEC                                        PATH            FILE                EXT
      B:JOE                                       B:              'JOE        '   '   '
      B:\JOE                                  B:\             'JOE        '   '   '
      B:\DIR\JOE                          B:\DIR      'JOE        '   '   '
      B:DIR\JOE                               B:DIR           'JOE        '   '   '
      Returns
      Returns TRUE.


 ****************************************************************************/
BOOLEAN pc_parsepath(byte *topath, byte *filename, byte *fileext, byte *path) /*__fn__*/
{
    int i,keep_slash;
    byte /**lastchar,*/*pfile,*pslash,*pcolon,*p,*pto,*pfilespace;
    RTFS_ARGSUSED_PVOID((void *)fileext);

    /* Check the path length, compare it EMAXPATH_CHARS (255) the
       maximum filename length for VFAT */
    if (rtfs_cs_strlen(path) > EMAXPATH_CHARS)
        return(FALSE);

    pslash = pfile = 0;
    p = path;
    pcolon = 0;
    keep_slash = 0;
    /* if A:\ or \ only keep slash */
    i = 0;
    while (CS_OP_IS_NOT_EOS(p))
    {
        if (CS_OP_CMP_ASCII(p,'\\'))
            pslash = p;
        else if (CS_OP_CMP_ASCII(p,':'))
        {
            if (i != 1)     /* A: B: C: .. x: y: z: only */
                return (FALSE);
            pcolon = p;
        }
        CS_OP_INC_PTR(p);
        i++;
    }
    if (pslash == path)
        keep_slash = 1;
    else if (pcolon && pslash)
    {
        CS_OP_INC_PTR(pcolon);
        if (pslash == pcolon)
            keep_slash = 1;
    }


    /*lastchar = 0;*/
    p = path;
    /* Find the file section, after the colon or last backslash */
    while (CS_OP_IS_NOT_EOS(p))
    {
        if (CS_OP_CMP_ASCII(p,'\\') ||  CS_OP_CMP_ASCII(p,':') )
        {
            CS_OP_INC_PTR(p);
            pfile = p;
        }
        else
        {
            CS_OP_INC_PTR(p);
        }
    }

    /* Now copy the path. Up to the file or NULL if no file */
    pto = topath;
    if (pfile)
    {
        p = path;
        while (p < pfile)
        {
            /* Don't put slash on the end if more than one */
            if (p == pslash && !keep_slash)
                break;
            CS_OP_CP_CHR(pto, p);
            CS_OP_INC_PTR(pto);
            CS_OP_INC_PTR(p);
        }
    }
    CS_OP_TERM_STRING(pto);
    /* Now copy the file portion or the whole path to file if no path portion */
    pto = filename;
    if (pfile)
        p = pfile;
    else
        p = path;

    /* Skip leading spaces */
    while (CS_OP_CMP_ASCII(p,' '))
        CS_OP_INC_PTR(p);
    /* Check the file length */
    if (rtfs_cs_strlen(p) > FILENAMESIZE_CHARS)
        return(FALSE);

    pfilespace = 0;
    while (CS_OP_IS_NOT_EOS(p))
    {
        CS_OP_CP_CHR(pto, p);
        CS_OP_INC_PTR(p);
        if (CS_OP_CMP_ASCII(pto,' '))
        {
            if (!pfilespace)
                pfilespace = pto;
        }
        else
            pfilespace = 0;
        CS_OP_INC_PTR(pto);
    }
    /* If the trailing character is a space NULL terminate */
    if (pfilespace)
        {CS_OP_TERM_STRING(pfilespace);}
    else
        CS_OP_TERM_STRING(pto);
    return(TRUE);
}
/******************************************************************************
PC_PATCMP  - Compare a pattern with a string

  Description
  Compare size bytes of p against pattern. Applying the following rules.
  If size == 8.
  (To handle the way dos handles deleted files)
  if p[0] = DELETED, never match
  if pattern[0] == DELETED, match with 0x5

    '?' in pattern always matches the current char in p.
    '*' in pattern always matches the rest of p.
    Returns
    Returns TRUE if they match

****************************************************************************/
BOOLEAN pc_patcmp_vfat(byte *in_pat, byte *name, BOOLEAN dowildcard)    /*__fn__*/
{
    byte *pat, *p, *pp, *pn, *pp2, *pn2;
    byte star[4];
    BOOLEAN res = FALSE;

    /* Convert *.* to just * */
    p = pat = in_pat;
    if (dowildcard && CS_OP_CMP_ASCII(p,'*'))
    {
        CS_OP_INC_PTR(p);
        if (CS_OP_CMP_ASCII(p,'.'))
        {
            CS_OP_INC_PTR(p);
            if (CS_OP_CMP_ASCII(p,'*'))
            {
                CS_OP_INC_PTR(p);
                if (CS_OP_IS_EOS(p))
                {
                    /* Change *.* to * but since the argument may have been
                       const we do it in a private buffer */
                    p = pat = star;
                    CS_OP_ASSIGN_ASCII(p,'*');
                    CS_OP_INC_PTR(p);
                    CS_OP_TERM_STRING(p);
                }
            }
        }
    }
    /* * matches everything */
    p = pat;
    if (dowildcard && CS_OP_CMP_ASCII(p,'*'))
    {
        CS_OP_INC_PTR(p);
        if (CS_OP_IS_EOS(p))
            return(TRUE);
    }

    for (pp=pat,pn=name;CS_OP_IS_NOT_EOS(pp); CS_OP_INC_PTR(pn),CS_OP_INC_PTR(pp))
    {
        if(CS_OP_CMP_ASCII(pp,'*') && dowildcard)
        {
            pp2 = pp;
            CS_OP_INC_PTR(pp2);
            if (CS_OP_IS_EOS(pp2))
                return(TRUE); /* '*' at end */
            pn2 = pn;
            /* We hit star. Now go past it and see if there is another
            exact match. IE: a*YYY matches abcdefgYYY but not abcdefgXXX */
            for (;!res && CS_OP_IS_NOT_EOS(pn2); CS_OP_INC_PTR(pn2))
            {
                res = res+pc_patcmp_vfat(pp2,pn2,TRUE);
            }
            return(res);
        }

        else if (CS_OP_CMP_CHAR(pp, pn))
            ;
        else if (CS_OP_CMP_ASCII(pp,'?') && dowildcard)
            ;
        else if (CS_OP_CMP_CHAR_NC(pp, pn))
            ;
        else
            return(FALSE);
    }
    if(CS_OP_IS_EOS(pn))
        return(TRUE);
    else
        return(FALSE);
}

BOOLEAN pc_patcmp_vfat_8_3(byte *pat, byte *name, BOOLEAN dowildcard)    /*__fn__*/
{
    byte save_char;
    BOOLEAN ret_val;
    if (CS_OP_CMP_ASCII(name,PCDELETE))
        return (FALSE);
    save_char = *name;
    if (*name == 0x05)
        *name = 0xe5;
    ret_val = pc_patcmp_vfat(pat, name, dowildcard);
    *name = save_char;
    return(ret_val);
}

/*****************************************************************************
PC_MALIAS - Create a unique alias for input_file

  Description
  Fills fname and fext with a valid short file name alias that is unique
  in the destination directory (dest).  Not to be confused with pc_galias,
  which finds the currently used short file name alias for an existing
  file.
  Returns
  TRUE if a unique alias could be found, FALSE otherwise
*****************************************************************************/

BOOLEAN pc_malias(byte *fname, byte *fext, byte *input_file, DROBJ *dest) /*__fn__*/
{
    int try;
    DROBJ *pobj;
    BOOLEAN aliasunique;
    byte alias[26];
    byte ascii_alias[26];

    /* See if already a valid alias. If so we use */
    /* Note: This always fails for unicode */
    if (pc_cs_malias(alias, input_file, -1)) /*__fn__*/
    {
        pc_ascii_str2upper(alias,alias);
        pobj = pc_get_inode(0,dest,alias,0,GET_INODE_MATCH);
        if(pobj)
        {
            pc_freeobj(pobj);
        }
        else
        {
            if (get_errno() != PENOENT)
                return(FALSE);
            rtfs_set_errno(0);  /* Clear PENOENT */
            pc_ascii_fileparse(fname,fext,(byte *)input_file);
            pc_ascii_str2upper(fname,fname);
            pc_ascii_str2upper(fext,fext);
            return(TRUE);
        }
    }


    /* Loop building up alias names and testing if unique */
    try=0; /* i=0; i is the alias index */
    do
    {
        try++;
        if (!pc_cs_malias(alias, input_file, try))
        {
            /* Ran out of valid alias names. Use PENOSPC as errno */
            rtfs_set_errno(PENOSPC);
            return(FALSE);
        }

        pobj = pc_get_inode(0,dest,alias,0,GET_INODE_MATCH);
        if(pobj)
        {
            aliasunique = FALSE;
            pc_freeobj(pobj);
        }
        else
        {
            if (get_errno() != PENOENT)
                return(FALSE);
            aliasunique = TRUE;
        }
    }
    while(!aliasunique);

    rtfs_set_errno(0); /* Clear PENOENT */
    /* Parse the alias into 8.3 */
    CS_OP_CS_TO_ASCII_STR(ascii_alias,alias);
    pc_ascii_fileparse(fname,fext,ascii_alias);

    return(TRUE);
}
/* Byte oriented */
static BOOLEAN pc_allspace(byte *p, int i)                                                             /* __fn__*/
{while (i--) if (*p++ != ' ') return (FALSE);   return (TRUE); }

BOOLEAN _illegal_lfn_char(byte ch)
{
    if (pc_strchr(rtfs_strtab_user_string(USTRING_SYS_BADLFN), ch))
        return(TRUE);
    else
        return(FALSE);
}

BOOLEAN pc_isdot(byte *fname, byte *fext)                                               /* __fn__*/
{
    RTFS_ARGSUSED_PVOID((void *)fext);
    return (BOOLEAN)((*fname == '.') &&
        ((*(fname+1) == '\0') || (pc_allspace((fname+1),10))) );
}
BOOLEAN pc_isdotdot(byte *fname, byte *fext)                                            /* __fn__*/
{
    RTFS_ARGSUSED_PVOID((void *)fext);
    return (BOOLEAN)( (*fname == '.') && (*(fname+1) == '.') &&
        ((*(fname+2) == '\0') || (pc_allspace((fname+2),9)) ) );
}

BOOLEAN pc_delete_lfn_info(DROBJ *pobj)
{
     return(pc_deleteseglist(pobj->pdrive, &pobj->finode->s));
}
void pc_zero_lfn_info(FINODE *pdir)
{
    pc_zeroseglist(&pdir->s);
}
BOOLEAN pc_get_lfn_filename(DROBJ *pobj, byte *path)
{
   if (pobj->finode->s.nsegs)
   {
      if (pc_seglist2text(pobj->pdrive, &pobj->finode->s, path))
          return(TRUE);
      else
          return(FALSE);
    }
    else
      return(FALSE);
}

/* scan_for_bad_lfns(DROBJ *pmom)
 *
 *  Scan through a directory and count all Win95 long file name errors.
 *
 *  Errors detected:
 *      Bad lfn checksums
 *      Bad lfn sequence numbers
 *      Stray lfn chains
 *      Incomplete lfn chains
 *
 *  If gl.delete_bad_lfn is set, free up the directory space used up by
 *  invalid long file name information by deleting invalid chains
 *
 *  Returns: number of invalid lfn chains found
 *
 *  This fn is called by chkdsk. It is based on pc_findin
 */

dword scan_for_bad_lfns(DROBJ *pmom, int delete_bad_lfn)          /*__fn__*/
{
DROBJ    *pobj;
BLKBUFF  *rbuf;
DIRBLK   *pd;
DOSINODE *pi;
LFNINODE *lfn_node;
SEGDESC  s;
byte     lastsegorder;
dword    bad_lfn_count;

    pc_zeroseglist(&s);
    lastsegorder = 0;

    /* pobj will be used to scan through the directory */
    pobj = pc_mkchild(pmom);
    if (!pobj)
        return(0);

    bad_lfn_count = 0;

    /* For convenience. We want to get at block info here   */
    pd = &pobj->blkinfo;

    /* Read the data   */
    pobj->pblkbuff = rbuf = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);

    while (rbuf)
    {
        pi = (DOSINODE *) &rbuf->data[0];

        /* Look at the current inode   */
        pi += pd->my_index;

        while ( pd->my_index < INOPBLOCK )
        {
            /* End of dir if name is 0   */
            if (!pi->fname[0])
            {
                pc_release_buf(rbuf);
                pc_freeobj(pobj);
                return(bad_lfn_count);
            }

            if (pi->fname[0] == PCDELETE)
            {
                if (s.nsegs)
                {
                    /* lfn chain interrupted by empty dir entry */
                    bad_lfn_count++;
                    if (delete_bad_lfn)
                        pc_deleteseglist(pobj->pdrive, &s);
                    pc_zeroseglist(&s);
                    lastsegorder = 0;
                }
            }
            else
            {
                if (pi->fattribute == CHICAGO_EXT)
                {
                    /* Found a piece of an lfn */
                    lfn_node = (LFNINODE *) pi;

                    if (lfn_node->lfnorder & FIRST_NAMESEG)
                    {
                        if (s.nsegs) /* if a chain already exists */
                        {
                            /* lfn chain begins in the middle of another one;
                               Delete the one that was interrupted, keep the
                               new one. */
                            bad_lfn_count++;
                            if (delete_bad_lfn)
                                pc_deleteseglist(pobj->pdrive, &s);
                            pc_zeroseglist(&s);
                        }
                        lastsegorder = lfn_node->lfnorder;
                        pc_addtoseglist(&s, pd->my_block, pd->my_index);
                        s.ncksum = lfn_node->lfncksum;
                    }
                    else
                    {
                        /* optimization - the current segment should first be
                           linked onto the chain in each of the branches
                           below */
                        pc_addtoseglist(&s, pd->my_block, pd->my_index);
                        if ((s.nsegs - 1) &&
                            (lfn_node->lfnorder & NAMESEG_ORDER) == (lastsegorder & NAMESEG_ORDER) - 1 &&
                            lfn_node->lfncksum == s.ncksum )
                        {
                            /* Sequence number and checksum match; Add new
                               segment to lfn chain   */
                            lastsegorder = lfn_node->lfnorder;
                        }
                        else
                        {
                            /* New segment has a checksum or sequence number
                               that doesn't match the current chain; Delete
                               the whole chain plus new segment */
                            bad_lfn_count++;
                            if (delete_bad_lfn)
                                pc_deleteseglist(pobj->pdrive, &s);
                            pc_zeroseglist(&s);
                            lastsegorder = 0;
                        }
                    }
                }
                else
             /* if (pi->fattribute != CHICAGO_EXT) */
                {
                    /* Found a file entry - make sure our lfn chain matches
                       (if we have one) */
                    if (s.nsegs) /* if a chain has been built up */
                    {
                        if (pc_cksum((byte*)pi) != s.ncksum ||
                            (lastsegorder & NAMESEG_ORDER) != 1)
                        {
                            /* chain's checksum doesn't match the DOSINODE,
                               or the last sequence number isn't 1; Delete
                               the lfn chain */
                            bad_lfn_count++;
                            if (delete_bad_lfn)
                                pc_deleteseglist(pobj->pdrive, &s);
                        }
                        /* We want to release this chain, whether good or bad */
                        pc_zeroseglist(&s);
                        lastsegorder = 0;
                    }
                }       /* if (!CHICAGO_EXT) */
            }           /* if (!PCDELETE) */
            pd->my_index++;
            pi++;
        }
        /* Current block is clean; go to next one */
        pc_release_buf(rbuf);
        /* Update the objects block pointer */
        if (!pc_next_block(pobj))
            break;
        pd->my_index = 0;
        pobj->pblkbuff = rbuf = pc_read_blk(pobj->pdrive, pobj->blkinfo.my_block);
    }

    pc_freeobj(pobj);
    return (bad_lfn_count);
}

#endif


