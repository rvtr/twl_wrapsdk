/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* PRBLOCK.C - ERTFS-PRO Directory block buffering routines */

#include <rtfs.h>


static void pc_add_blk(BLKBUFFCNTXT *pbuffcntxt, BLKBUFF *pinblk);
static void pc_release_blk(BLKBUFFCNTXT *pbuffcntxt, BLKBUFF *pinblk);
static BLKBUFF *pc_allocate_blk(DDRIVE *pdrive, BLKBUFFCNTXT *pbuffcntxt);



/* Debugging tools to be removed in he final product */
#define DEBUG_BLOCK_CODE    0
#define DEBUG_FAT_CODE      0
void debug_check_blocks(BLKBUFFCNTXT *pbuffcntxt, int numblocks,  char *where, dword line);
void debug_check_fat(FATBUFFCNTXT *pfatbuffcntxt, char *where);
void debug_break(char *where, dword line, char *message);
#if (DEBUG_BLOCK_CODE)
#define DEBUG_CHECK_BLOCKS(X,Y,Z) debug_check_blocks(X,Y,Z,0);
#else
#define DEBUG_CHECK_BLOCKS(X,Y,Z)
#endif
#if (DEBUG_FAT_CODE)
#define DEBUG_CHECK_FAT(X,Y) debug_check_fat(X,Y);
#else
#define DEBUG_CHECK_FAT(X,Y)
#endif

#if (!INCLUDE_FAILSAFE_CODE)

BOOLEAN block_devio_read(DDRIVE *pdrive, dword blockno, byte * buf)
{
    return(devio_read(pdrive->driveno,  blockno, buf, (int) 1, FALSE));
}

BOOLEAN block_devio_write(BLKBUFF *pblk)
{
/*    dword blockno;
    blockno = pblk->blockno;*/
    return(devio_write(pblk->pdrive->driveno,pblk->blockno, pblk->data, (int) 1, FALSE));
}

BOOLEAN fat_devio_write(DDRIVE *pdrive, FATBUFF *pblk, int fatnumber)
{
dword blockno;
        if (fatnumber)
            blockno = pblk->fat_blockno+pdrive->secpfat;
        else
            blockno = pblk->fat_blockno;
        return(devio_write(pdrive->driveno,blockno, pblk->fat_data, (int) 1, FALSE));
}

BOOLEAN fat_devio_read(DDRIVE *pdrive, dword blockno, byte *fat_data)
{
    return(devio_read(pdrive->driveno,  blockno, fat_data, (int) 1, FALSE));
}
#else
/* Replacement routines provided by failsafe package */
BOOLEAN block_devio_read(DDRIVE *pdrive, dword blockno, byte * buf);
BOOLEAN block_devio_write(BLKBUFF *pblk);
BOOLEAN fat_devio_write(DDRIVE *pdrive, FATBUFF *pblk, int fatnumber);
BOOLEAN fat_devio_read(DDRIVE *pdrive, dword blockno, byte *fat_data);
#endif
/*****************************************************************************
    pc_release_buf - Unlock a block buffer.

Description
    Give back a buffer to the system buffer pool so that it may
    be re-used. If was_err is TRUE this means that the data in the
    buffer is invalid so discard the buffer from the buffer pool.

 Returns
    Nothing

***************************************************************************/

void pc_release_buf(BLKBUFF *pblk)
{
    if (!pblk)
        return;
    DEBUG_CHECK_BLOCKS(pblk->pdrive->pbuffcntxt, pblk->pdrive->pbuffcntxt->num_blocks, "Release")
#if (DEBUG_BLOCK_CODE)
    if (!pblk->pdrive->mount_valid)
        debug_break("release buf", __LINE__, "Mount not valid");
    if (pblk->block_state != DIRBLOCK_COMMITTED && pblk->block_state != DIRBLOCK_UNCOMMITTED)
        debug_break("release buf", __LINE__,"releasing buffer not in use list");
#endif

    if (pblk->block_state != DIRBLOCK_COMMITTED && pblk->block_state != DIRBLOCK_UNCOMMITTED)
        return;
    OS_CLAIM_FSCRITICAL()
    if (pblk->use_count)
    {
        pblk->use_count -= 1;
      /* 03-07-07 Changed. No longer increment num_free if usecount goes to zero */
    }
    OS_RELEASE_FSCRITICAL()
}

/*****************************************************************************
    pc_discard_buf - Put a buffer back on the free list.

Description
    Check if a buffer is in the buffer pool, unlink it from the
    buffer pool if it is.
    Put the buffer on the free list.

 Returns
    Nothing

***************************************************************************/

void pc_discard_buf(BLKBUFF *pblk)
{
BLKBUFFCNTXT *pbuffcntxt;

    if (!pblk)
        return;
    /*note:pblk->pdrive must not be 0. if so, use pc_free_scratch_buffer() */
#if (DEBUG_BLOCK_CODE)
    if (pblk->block_state == DIRBLOCK_FREE)
    {
        printf("Warning: discarding a free buffer \n");
    }
    if (pblk->block_state == DIRBLOCK_COMMITTED)
    {   /* Could come from a disk free call */
        printf("Warning discarding a committed buffer \n");
    }
#endif
    if (pblk->block_state == DIRBLOCK_FREE)
        return;

    OS_CLAIM_FSCRITICAL()
    pbuffcntxt = pblk->pdrive->pbuffcntxt;
    DEBUG_CHECK_BLOCKS(pbuffcntxt, pbuffcntxt->num_blocks, "Discard 1")
    if (pblk->block_state == DIRBLOCK_COMMITTED || pblk->block_state == DIRBLOCK_UNCOMMITTED)
    {
        /* Release it from the buffer pool */
        pc_release_blk(pbuffcntxt, pblk);
#if (DEBUG_BLOCK_CODE)
        if (pblk->pnext && pblk->pnext->pprev != pblk)
            debug_break("discard buf", __LINE__,"Buffer and populated pool inconsistent");
        if (pblk->pprev && pblk->pprev->pnext != pblk)
            debug_break("discard buf", __LINE__,"Buffer and populated pool inconsistent");
#endif
        /* Unlink it from the populated pool double check link integrity */
        if (pblk->pnext && pblk->pnext->pprev == pblk)
            pblk->pnext->pprev = pblk->pprev;
        if (pblk->pprev && pblk->pprev->pnext == pblk)
            pblk->pprev->pnext = pblk->pnext;
        if (pbuffcntxt->ppopulated_blocks == pblk)
            pbuffcntxt->ppopulated_blocks = pblk->pnext;
    }
    pblk->block_state = DIRBLOCK_FREE;
    pblk->pnext = pbuffcntxt->pfree_blocks;
    pbuffcntxt->num_free += 1;
    pbuffcntxt->pfree_blocks = pblk;
    OS_RELEASE_FSCRITICAL()
    DEBUG_CHECK_BLOCKS(pbuffcntxt, pbuffcntxt->num_blocks, "Discard 2")
}

/****************************************************************************
    PC_READ_BLK - Allocate and read a BLKBUFF, or get it from the buffer pool.

Description
    Use pdrive and blockno to determine what block to read. Read the block
    or get it from the buffer pool and return the buffer.

    Note: After reading, you own the buffer. You must release it by
    calling pc_release_buff() or pc_discard_buff() before it may be
    used for other blocks.

 Returns
    Returns a valid pointer or NULL if block not found and not readable.

*****************************************************************************/



BLKBUFF *pc_read_blk(DDRIVE *pdrive, dword blockno)                /*__fn__*/
{
BLKBUFF *pblk;
BLKBUFFCNTXT *pbuffcntxt;

    if ( !pdrive || (blockno >= pdrive->numsecs) )
        return(0);
    OS_CLAIM_FSCRITICAL()
    DEBUG_CHECK_BLOCKS(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks, "Read 1")
    /* TRy to find it in the buffer pool */
    pblk = pc_find_blk(pdrive, blockno);
    if (pblk)
    {
        pdrive->pbuffcntxt->stat_cache_hits += 1;
        DEBUG_CHECK_BLOCKS(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks, "Read 2")
        pblk->use_count += 1;
        OS_RELEASE_FSCRITICAL()
    }
    else
    {
        pdrive->pbuffcntxt->stat_cache_misses += 1;
        /* Allocate, read, stitch into the populated list, and the buffer pool */
        pblk = pc_allocate_blk(pdrive, pdrive->pbuffcntxt);
        OS_RELEASE_FSCRITICAL()
        if (pblk)
        {
            if (block_devio_read(pdrive, blockno, pblk->data))
            {
                pbuffcntxt = pdrive->pbuffcntxt;
                pblk->use_count = 1;
                pblk->block_state = DIRBLOCK_UNCOMMITTED;
                pblk->blockno = blockno;
                pblk->pdrive = pdrive;
                /* Put in front of populated list (it's new) */
                OS_CLAIM_FSCRITICAL()
                pblk->pprev = 0;
                pblk->pnext = pbuffcntxt->ppopulated_blocks;
                if (pbuffcntxt->ppopulated_blocks)
                    pbuffcntxt->ppopulated_blocks->pprev = pblk;
                pbuffcntxt->ppopulated_blocks = pblk;
                pc_add_blk(pbuffcntxt, pblk); /* Add it to the buffer pool */
                DEBUG_CHECK_BLOCKS(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks, "Read 3")
                OS_RELEASE_FSCRITICAL()
            }
            else
            {
                /* set errno to IO error unless devio set PEDEVICE */
                if (!get_errno())
                    rtfs_set_errno(PEIOERRORREADBLOCK); /* pc_read_blk device read error */
                pc_discard_buf(pblk);
                DEBUG_CHECK_BLOCKS(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks, "Read 4")
                pblk = 0;
            }
        }
    }
    DEBUG_CHECK_BLOCKS(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks, "Read 5")
    return(pblk);
}
/***************************************************************************
    PC_SCRATCH_BLK - Return a block for scratch purposes.
Description
    Use the block buffer pool as a heap of 512 byte memory locations
    When done call void pc_free_scratch_buf(pblk) to clean up

 Returns
    Returns a blkbuf if one is available or NULL
****************************************************************************/
/* Allocate a scratch buffer from the shared (amongst drives) buffer pool */
BLKBUFF *pc_scratch_blk(void)                                   /*__fn__*/
{
BLKBUFF *pblk;
    OS_CLAIM_FSCRITICAL()
    pblk = pc_allocate_blk(0, &prtfs_cfg->buffcntxt);
    if (pblk)
        prtfs_cfg->buffcntxt.scratch_alloc_count += 1;
    OS_RELEASE_FSCRITICAL()
    return (pblk);
}

/* Free a scratch buffer to the shared (amongst drives) buffer pool */
void pc_free_scratch_blk(BLKBUFF *pblk)
{
    BLKBUFFCNTXT *pbuffcntxt;
    OS_CLAIM_FSCRITICAL()
    pbuffcntxt = &prtfs_cfg->buffcntxt;
    pblk->pnext = pbuffcntxt->pfree_blocks;
    pbuffcntxt->pfree_blocks = pblk;
    pblk->block_state = DIRBLOCK_FREE;
    pbuffcntxt->num_free += 1;
    pbuffcntxt->scratch_alloc_count -= 1;
    OS_RELEASE_FSCRITICAL()
}


/***************************************************************************
    PC_INIT_BLK - Zero a BLKBUFF and add it to the buffer pool
Description
    Allocate and zero a BLKBUFF and add it to the to the buffer pool.

    Note: After initializing you own the buffer. You must release it by
    calling pc_release_buff() or pc_discard_buf() before it may be used
    for other blocks.

 Returns
    Returns a valid pointer or NULL if no core.

****************************************************************************/

BLKBUFF *pc_init_blk(DDRIVE *pdrive, dword blockno)                /*__fn__*/
{
BLKBUFF *pblk;
BLKBUFFCNTXT *pbuffcntxt;

    if ( !pdrive || (blockno >= pdrive->numsecs) )
        return(0);
    /* Find it in the buffer pool */
    OS_CLAIM_FSCRITICAL()
    DEBUG_CHECK_BLOCKS(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks, "Init 1")
    pblk = pc_find_blk(pdrive, blockno);
    if (pblk)
    {
        pblk->use_count += 1;
        OS_RELEASE_FSCRITICAL()
    }
    else
    {
        /* Allocate, read, stitch into the populated list, and the buffer pool */
        DEBUG_CHECK_BLOCKS(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks, "Init 2")
        pblk = pc_allocate_blk(pdrive, pdrive->pbuffcntxt);

        OS_RELEASE_FSCRITICAL()
        if (pblk)
        {
            DEBUG_CHECK_BLOCKS(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks-1, "Init 2_a")
            pbuffcntxt = pdrive->pbuffcntxt;
            pblk->use_count = 1;
            pblk->block_state = DIRBLOCK_UNCOMMITTED;
            pblk->blockno = blockno;
            pblk->pdrive = pdrive;
            /* Put in front of populated list (it's new) */
            OS_CLAIM_FSCRITICAL()
            pblk->pprev = 0;
            pblk->pnext = pbuffcntxt->ppopulated_blocks;
            if (pbuffcntxt->ppopulated_blocks)
                pbuffcntxt->ppopulated_blocks->pprev = pblk;
            pbuffcntxt->ppopulated_blocks = pblk;
            pc_add_blk(pbuffcntxt, pblk); /* Add it to the buffer pool */
            OS_RELEASE_FSCRITICAL()
            DEBUG_CHECK_BLOCKS(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks, "Init 3")
        }
    }
    if (pblk)
        rtfs_memset(pblk->data, (byte) 0, 512);
    return(pblk);
}

/****************************************************************************
    PC_FREE_ALL_BLK - Release all buffers associated with a drive
Description
    Use pdrive to find all buffers in the buffer pool associated with the
    drive. Mark them as unused, called by dsk_close.
    If any are locked, print a debug message in debug mode to warn the
    programmer.

 Returns
    Nothing
****************************************************************************/
void pc_free_all_blk(DDRIVE *pdrive)                                /*__fn__*/
{
BLKBUFFCNTXT *pbuffcntxt;
BLKBUFF *pblk;
BOOLEAN deleting;

    if (!pdrive)
        return;
    pbuffcntxt = pdrive->pbuffcntxt;
    DEBUG_CHECK_BLOCKS(pbuffcntxt, pbuffcntxt->num_blocks, "Free all 1")
    do
    {
        deleting = FALSE;
        OS_CLAIM_FSCRITICAL()
        pblk = pbuffcntxt->ppopulated_blocks;
        while (pblk)
        {
            if (pblk->pdrive == pdrive)
            {
                OS_RELEASE_FSCRITICAL()
                pc_discard_buf(pblk);
                OS_CLAIM_FSCRITICAL()
                deleting = TRUE;
                break;
            }
            else
                pblk = pblk->pnext;
        }
        OS_RELEASE_FSCRITICAL()
    } while (deleting);
    DEBUG_CHECK_BLOCKS(pbuffcntxt, pbuffcntxt->num_blocks, "Free all 2")
}

/***************************************************************************
    PC_WRITE_BLK - Flush a BLKBUFF to disk.
Description
    Use pdrive and blockno information in pblk to flush its data buffer
    to disk.

 Returns
    Returns TRUE if the write succeeded.
****************************************************************************/

/* Write */
BOOLEAN pc_write_blk(BLKBUFF *pblk)                                 /*__fn__*/
{
        if (!block_devio_write(pblk))
        {
            /* set errno to IO error unless devio set PEDEVICE */
            if (!get_errno())
                rtfs_set_errno(PEIOERRORWRITEBLOCK); /* pc_write_blk device write error */
            return(FALSE);
        }
        else
            return(TRUE);
}

/* Add a block to the block buffer pool */
static void pc_add_blk(BLKBUFFCNTXT *pbuffcntxt, BLKBUFF *pinblk)
{
int hash_index;
    hash_index = (int) (pinblk->blockno&pbuffcntxt->hash_mask);
    pinblk->pnext2 = *(pbuffcntxt->blk_hash_tbl+hash_index);
    *(pbuffcntxt->blk_hash_tbl+hash_index) = pinblk;
}
/* Remove a block from the block buffer pool */
static void pc_release_blk(BLKBUFFCNTXT *pbuffcntxt, BLKBUFF *pinblk)
{
int hash_index;
BLKBUFF *pblk;

    hash_index = (int) (pinblk->blockno&pbuffcntxt->hash_mask);
    pblk = *(pbuffcntxt->blk_hash_tbl+hash_index);
    if (pblk == pinblk)
        *(pbuffcntxt->blk_hash_tbl+hash_index) = pinblk->pnext2;
    else
    {
        while (pblk)
        {
            if (pblk->pnext2==pinblk)
            {
                pblk->pnext2 = pinblk->pnext2;
                break;
            }
            pblk = pblk->pnext2;
        }
    }
}
/* Find a block in the block buffer pool */
BLKBUFF *pc_find_blk(DDRIVE *pdrive, dword blockno)
{
BLKBUFFCNTXT *pbuffcntxt;
int hash_index;
BLKBUFF *pblk;

    pbuffcntxt = pdrive->pbuffcntxt;
    hash_index = (int) (blockno&pbuffcntxt->hash_mask);
    pblk = *(pbuffcntxt->blk_hash_tbl+hash_index);
    while (pblk)
    {
        if (pblk->blockno == blockno && pblk->pdrive == pdrive)
        {
                if (pblk->block_state == DIRBLOCK_UNCOMMITTED)
                { /* Put in front of populated list (it's the last touched) */
                /* Unlink it from the populated pool */
                    if (pbuffcntxt->ppopulated_blocks != pblk)
                    {   /* Since we aren't the root we know pprev is good */
                        pblk->pprev->pnext = pblk->pnext;   /* Unlink. */
                        if (pblk->pnext)
                            pblk->pnext->pprev = pblk->pprev;
                        pblk->pprev = 0;                    /* link in front*/
                        pblk->pnext = pbuffcntxt->ppopulated_blocks;
                        if (pbuffcntxt->ppopulated_blocks)
                            pbuffcntxt->ppopulated_blocks->pprev = pblk;
                        pbuffcntxt->ppopulated_blocks = pblk;
                    }
                }
                return(pblk);
        }
        pblk=pblk->pnext2;
    }
    return(0);
}
/* Allocate a block or re-use an un-committed one */
static BLKBUFF *pc_allocate_blk(DDRIVE *pdrive, BLKBUFFCNTXT *pbuffcntxt)
{
BLKBUFF *pfreeblk,*puncommitedblk, *pfoundblk, *pblkscan;
int populated_but_uncommited;

    /* Note: pdrive may be NULL, do not dereference the pointer */
    pfreeblk = pfoundblk = puncommitedblk = 0;
    populated_but_uncommited = 0;

    /* Use blocks that are on the freelist first */
    if (pbuffcntxt->pfree_blocks)
    {
        pfreeblk = pbuffcntxt->pfree_blocks;
        pbuffcntxt->pfree_blocks = pfreeblk->pnext;
        pbuffcntxt->num_free -= 1;
    }

    /* Scan the populated list. Count the number of uncommited blocks to set low water marks
       and, if we haven't already allocated a block from the free list, select a replacement block. */
    if (pbuffcntxt->ppopulated_blocks)
    {
        int loop_guard = 0;
        /* Count UNCOMMITED blocks and find the oldest UNCOMMITED block in the list */
        pblkscan = pbuffcntxt->ppopulated_blocks;
        while (pblkscan)
        {
           if (pblkscan->block_state == DIRBLOCK_UNCOMMITTED && !pblkscan->use_count)
           {
                puncommitedblk = pblkscan;
                populated_but_uncommited += 1;
           }
           pblkscan = pblkscan->pnext;
           /* Guard against endless loop */
           if (loop_guard++ > pbuffcntxt->num_blocks)
           {
                rtfs_set_errno(PEINTERNAL); /* pc_allocate_blk: Internal error*/
                return(0);
           }
        }
        /* If we don't already have a free block we'll reuse the oldest uncommitted block so release it */
        if (!pfreeblk && puncommitedblk)
        {
            pc_release_blk(pbuffcntxt, puncommitedblk); /* Remove it from buffer pool */
            /* Unlink it from the populated pool */
            if (puncommitedblk->pnext)
                puncommitedblk->pnext->pprev = puncommitedblk->pprev;
            if (puncommitedblk->pprev)
                puncommitedblk->pprev->pnext = puncommitedblk->pnext;
            if (pbuffcntxt->ppopulated_blocks == puncommitedblk)
                pbuffcntxt->ppopulated_blocks = puncommitedblk->pnext;
        }
    }
    if (pfreeblk)
        pfoundblk = pfreeblk;
    else
        pfoundblk = puncommitedblk;

    if (pfoundblk)
    {   /* Put in a known state */
        /* 03-07-2007 using a different method to calculate low water mark. Previous method
           undercounted the worst case buffer allocation requirements */
        if (pbuffcntxt->num_free + populated_but_uncommited < pbuffcntxt->low_water)
            pbuffcntxt->low_water = pbuffcntxt->num_free + populated_but_uncommited;
        pfoundblk->use_count = 0;
        pfoundblk->block_state = DIRBLOCK_ALLOCATED;
        pfoundblk->pdrive = pdrive;
    }
    else
    {
        pbuffcntxt->num_alloc_failures += 1;
        rtfs_set_errno(PERESOURCEBLOCK); /* pc_allocate_blk out of resources */
    }
    return(pfoundblk);
}
/* Tomo */
/* Traverse a cluster chain and make sure that all blocks in the cluster
 chain are flushed from the buffer pool. This is required when deleting
 a directory since it is possible, although unlikely, that blocks used in
 the directory may be used in a file. This may cause the buffered
 block to be different from on-disk block.
 Called by pc_rmnode
*/
void pc_flush_chain_blk(DDRIVE *pdrive, CLUSTERTYPE cluster)
{
int i;
dword blockno;
BLKBUFF *pblk;

    if ( cluster < 2 || cluster > pdrive->maxfindex)
        return;
    while (cluster != 0xffffffff) /* End of chain */
    {
        blockno = pc_cl2sector(pdrive, cluster);
        if (blockno)
        {
            for (i = 0; i < pdrive->secpalloc; i++, blockno++)
            {
            /* Find it in the buffer pool */
                OS_CLAIM_FSCRITICAL()
                pblk = pc_find_blk(pdrive, blockno);
                if (pblk)
                    pblk->use_count += 1;
                OS_RELEASE_FSCRITICAL()
                if (pblk)
                {
                    pc_discard_buf(pblk);
                }
            }
        }
         /* Consult the fat for the next cluster   */
        cluster = FATOP(pdrive)->fatop_clnext(pdrive, cluster);
        if (cluster == 0) /* clnext detected error */
            break;
     }
}

/* Initialize and populate a block buffer context structure */
BOOLEAN pc_initialize_block_pool(BLKBUFFCNTXT *pbuffcntxt, int nblkbuffs,
        BLKBUFF *pmem_block_pool, int blk_hashtble_size, BLKBUFF **pblock_hash_table)
{
int i;
dword t;
BLKBUFF *pblk;
    rtfs_memset(pbuffcntxt,(byte) 0, sizeof(BLKBUFFCNTXT));
    rtfs_memset(pblock_hash_table,(byte) 0, sizeof(BLKBUFF *)*blk_hashtble_size);
    pblk = pmem_block_pool;
    for (i = 0; i < (nblkbuffs-1); i++, pblk++)
    {
        rtfs_memset(pblk,(byte) 0, sizeof(BLKBUFF));
        pblk->pnext = pblk+1;
    }
    rtfs_memset(pblk,(byte) 0, sizeof(BLKBUFF));
    /*  pblk->pnext = 0; accomplished by memset */
    pbuffcntxt->pfree_blocks = pmem_block_pool;
    pbuffcntxt->hash_size = blk_hashtble_size;
    /* Take size and subtract 1 to get the mask */
    t = (dword) blk_hashtble_size;
    pbuffcntxt->hash_mask = t-1;
    pbuffcntxt->blk_hash_tbl = pblock_hash_table;
    pbuffcntxt->num_blocks = nblkbuffs;
    pbuffcntxt->num_free = nblkbuffs;
    pbuffcntxt->low_water = nblkbuffs;
    pbuffcntxt->num_alloc_failures = 0;
    return(TRUE);
}

FATBUFF *pc_find_fat_blk(FATBUFFCNTXT *pfatbuffcntxt, dword blockno);
static void pc_commit_fat_blk(FATBUFFCNTXT *pfatbuffcntxt, FATBUFF *pblk);
void pc_commit_fat_table(FATBUFFCNTXT *pfatbuffcntxt);
void pc_sort_committed_blocks(FATBUFFCNTXT *pfatbuffcntxt);


BOOLEAN pc_flush_fat_blocks(DDRIVE *pdrive)
{
FATBUFFCNTXT *pfatbuffcntxt;
FATBUFF *pblk, *plast;
int hash_index;
dword b;

    pfatbuffcntxt = &pdrive->fatcontext;
    DEBUG_CHECK_FAT(pfatbuffcntxt, "Flush 1")
    /* Make sure the primary cache is is sync with secondary */
    pc_commit_fat_table(pfatbuffcntxt);
    DEBUG_CHECK_FAT(pfatbuffcntxt, "Flush 2")
    if (!pfatbuffcntxt->pcommitted_blocks)
        return(TRUE);
    /* Sort the blocks in ascending order */
    DEBUG_CHECK_FAT(pfatbuffcntxt, "Flush 3")
    pc_sort_committed_blocks(pfatbuffcntxt);
    DEBUG_CHECK_FAT(pfatbuffcntxt, "Flush 4")
    pblk = pfatbuffcntxt->pcommitted_blocks;
    while (pblk)
    {
        if (!fat_devio_write(pdrive,pblk,0))
        {
            /* set errno to PEDEVICE unless devio set errno */
            if (!get_errno())
                rtfs_set_errno(PEIOERRORWRITEFAT); /* flush_fat device write error */
io_error:
            pc_report_error(PCERR_FAT_FLUSH);
            return(FALSE);
        }
        else
            pblk = pblk->pnext;
    }
    if (pdrive->numfats >= 2)
    {   /* Write the second copy */
        pblk = pfatbuffcntxt->pcommitted_blocks;
        while (pblk)
        {
            if (!fat_devio_write(pdrive,pblk,1))
            {
                /* set errno to PEDEVICE unless devio set errno */
                if (!get_errno())
                    rtfs_set_errno(PEIOERRORWRITEFAT); /* flush_fat device write error */
                goto io_error;
            }
            else
                pblk = pblk->pnext;
        }
    }
    /* Set all entries uncommitted and put them in front of uncommitted list */
    DEBUG_CHECK_FAT(pfatbuffcntxt, "Flush 5")
    plast = 0;
    pblk = pfatbuffcntxt->pcommitted_blocks;
    while (pblk)
    {
        pblk->fat_block_state = FATBLOCK_UNCOMMITTED;

        /* Clear from the primary cache if needed */
        hash_index = (int) (pblk->fat_blockno&pfatbuffcntxt->hash_mask);
        b = *(pfatbuffcntxt->mapped_blocks+hash_index);
        if ( (b & 0x0ffffffful) == pblk->fat_blockno)
        {
            b &= 0x3ffffffful;  /* Clear 0x40000000ul and 0x80000000ul*/
            *(pfatbuffcntxt->mapped_blocks+hash_index) =  b;
        }
        plast = pblk;
        pblk = pblk->pnext;
    }
    if (pfatbuffcntxt->puncommitted_blocks)
        pfatbuffcntxt->puncommitted_blocks->pprev = plast;
    plast->pnext = pfatbuffcntxt->puncommitted_blocks;
    /* pfatbuffcntxt->pcommitted_blocks->pprev = 0; - this is already true*/
    pfatbuffcntxt->puncommitted_blocks = pfatbuffcntxt->pcommitted_blocks;
    pfatbuffcntxt->pcommitted_blocks = 0;
    DEBUG_CHECK_FAT(pfatbuffcntxt, "Flush 6")
    return(TRUE);
}

byte *pc_map_fat_block(DDRIVE *pdrive, dword blockno, dword usage_flags)
{
int hash_index, temp_hash_index;
FATBUFFCNTXT *pfatbuffcntxt;
FATBUFF *pblk, *pblkscan;
dword b;
    pfatbuffcntxt = &pdrive->fatcontext;
    DEBUG_CHECK_FAT(pfatbuffcntxt, "Map 1")
    hash_index = (int) (blockno&pfatbuffcntxt->hash_mask);
    b = *(pfatbuffcntxt->mapped_blocks+hash_index);
    if ( (b & 0x0ffffffful) == blockno)
    {
        pfatbuffcntxt->stat_primary_cache_hits += 1;
#if (INCLUDE_FAILSAFE_CODE)
        /* If in the primary cache and the new flag is write but the old
           flag was not write then we will need to journal the block */
        if ((usage_flags & 0x80000000ul) && (!(b & 0x80000000ul)))
            if (pro_failsafe_journal_full(pdrive))
                return(0); /* Failsafe sets errno */
#endif
        if (usage_flags) /* bit 31 0x80000000ul is write bit 29 0x20000000ul is lock */
            *(pfatbuffcntxt->mapped_blocks+hash_index) |= usage_flags;
        return (*(pfatbuffcntxt->mapped_data+hash_index));
    }
    else
    {   /* Swap out of the primary cache */
        if ( (b & 0xc0000000ul) == 0x80000000ul)
        {  /* Move to commit list if state changed from not_for_write to for_write */
            pblk = pc_find_fat_blk(pfatbuffcntxt, (b & 0x0ffffffful));
            if (pblk) /* This should not fail */
                pc_commit_fat_blk(pfatbuffcntxt, pblk);
            DEBUG_CHECK_FAT(pfatbuffcntxt, "Map 2")
            *(pfatbuffcntxt->mapped_blocks+hash_index) = 0;
        }
        /* See if blockno is in the secondary cache */
        pblk = pc_find_fat_blk(pfatbuffcntxt, blockno);
        if (pblk)
        {
            pfatbuffcntxt->stat_secondary_cache_hits += 1;
            DEBUG_CHECK_FAT(pfatbuffcntxt, "Map 3")
            b = blockno;
            if (pblk->fat_block_state == FATBLOCK_COMMITTED)
            {
                b |= 0x40000000ul;
            }
#if (INCLUDE_FAILSAFE_CODE)
            else
            {
                /* If in the primary cache but uncommitted, if it will
                   switch to committed now then we will have to journal the block */
                if (usage_flags & 0x80000000ul)
                {
                    if (pro_failsafe_journal_full(pdrive))
                        return(0); /* Failsafe sets errno */
                }
            }
#endif
            if (usage_flags) /* bit 31 0x80000000ul is write bit 29 0x20000000ul is lock */
                b |= usage_flags;
            *(pfatbuffcntxt->mapped_blocks+hash_index) = b;
            *(pfatbuffcntxt->mapped_data+hash_index) = pblk->fat_data;
            return(pblk->fat_data);
        }
        /* Not in secondary. Are there any free ? */
        if (!pfatbuffcntxt->pfree_blocks)
        {
            /* Make sure commit state changes in the fat table are up to date */
            /* Since hhere may committed blocks in the primary, but not secondary */
            pc_commit_fat_table(pfatbuffcntxt);
            DEBUG_CHECK_FAT(pfatbuffcntxt, "Map 4")
            if (!pfatbuffcntxt->puncommitted_blocks)
            {   /* No uncommitted blocks left */
                /* Write the oldest committed block to disk */
                /* So we can re-use the buffer space for another block */
                pblk = pfatbuffcntxt->pcommitted_blocks;
                while (pblk && pblk->pnext) /* Go to end of list */
                {
                    pblk = pblk->pnext;
                }
                if (pblk)
                {   /* Found one */
                    pfatbuffcntxt->stat_secondary_cache_swaps += 1;
                    if (!fat_devio_write(pdrive,pblk,0))
                    {
swap_write_error:
                        if (!get_errno())
                            rtfs_set_errno(PEIOERRORWRITEFAT); /* pc_map_fat_block device write error */
                        return(0);
                    }
                    if (pdrive->numfats >= 2)
                    {   /* Write the second copy */
                         if (!fat_devio_write(pdrive,pblk,1))
                             goto swap_write_error;
                    }
                     /* Remove it from committed list */
                    if (pblk->pnext)
                        pblk->pnext->pprev = pblk->pprev;
                    if (pblk->pprev)
                        pblk->pprev->pnext = pblk->pnext;
                    if (pblk == pfatbuffcntxt->pcommitted_blocks)
                        pfatbuffcntxt->pcommitted_blocks = pblk->pnext;
                    pfatbuffcntxt->stat_secondary_cache_swaps += 1;
                }
            }
            else
            {
                /* Find the oldest UNCOMMITED block (deepest into the list) */
                pblk = pfatbuffcntxt->puncommitted_blocks;
                while (pblk && pblk->pnext) /* Go to end of list */
                {
                    pblk = pblk->pnext;
                }
                if (pblk)
                {   /* Found one */
                    /* Remove it from uncommitted list */
                    if (pblk->pnext)
                        pblk->pnext->pprev = pblk->pprev;
                    if (pblk->pprev)
                        pblk->pprev->pnext = pblk->pnext;
                    if (pblk == pfatbuffcntxt->puncommitted_blocks)
                        pfatbuffcntxt->puncommitted_blocks = pblk->pnext;
                }
            }
            if (pblk) /* Came from either the committed or uncommited list */
            {
                /* Clear from the primary cache if needed */
                temp_hash_index = (int) (pblk->fat_blockno&pfatbuffcntxt->hash_mask);
                b = *(pfatbuffcntxt->mapped_blocks+temp_hash_index);
                if ( (b & 0x0ffffffful) == pblk->fat_blockno)
                    *(pfatbuffcntxt->mapped_blocks+temp_hash_index) =  0;
                /* Now get it out of the hash table */
                if (pblk == *(pfatbuffcntxt->fat_blk_hash_tbl+temp_hash_index))
                    *(pfatbuffcntxt->fat_blk_hash_tbl+temp_hash_index) = pblk->pnext2;
                else
                {
                    pblkscan = *(pfatbuffcntxt->fat_blk_hash_tbl+temp_hash_index);
                    while (pblkscan)
                    {
                        if (pblkscan->pnext2==pblk)
                        {
                            pblkscan->pnext2 = pblk->pnext2;
                            break;
                        }
                        pblkscan = pblkscan->pnext2;
                    }
                }
                /* Put it on the free list */
                pblk->pnext = pfatbuffcntxt->pfree_blocks;
                pfatbuffcntxt->pfree_blocks = pblk;
                DEBUG_CHECK_FAT(pfatbuffcntxt, "Map 5")
            }
        }
        else
        { /* the free list already had a block available for us */
            pfatbuffcntxt->stat_secondary_cache_loads += 1;
        }
        pblk = pfatbuffcntxt->pfree_blocks;
        if (!pblk)
        {
            /* No blocks available. We tried to flush the secondary and
               that didn't come up with any so we're completely out */
            rtfs_set_errno(PERESOURCEFATBLOCK);
            return(0);
        }
        else if (fat_devio_read(pdrive,  blockno, pblk->fat_data))
        {   /* The read worked. Unhook from the free list */
            pfatbuffcntxt->stat_secondary_cache_loads += 1;
            DEBUG_CHECK_FAT(pfatbuffcntxt, "Map 6")
            pfatbuffcntxt->pfree_blocks = pblk->pnext;
            pblk->fat_block_state = FATBLOCK_UNCOMMITTED;
            pblk->fat_blockno = blockno;
            /* Move to the front of the list so we don't swap too soon */
            if (pfatbuffcntxt->puncommitted_blocks)
                pfatbuffcntxt->puncommitted_blocks->pprev = pblk;
            pblk->pprev = 0;
            pblk->pnext = pfatbuffcntxt->puncommitted_blocks;
            pfatbuffcntxt->puncommitted_blocks = pblk;
            /* Insert in the hash table */
            pblk->pnext2 = *(pfatbuffcntxt->fat_blk_hash_tbl+hash_index);
            *(pfatbuffcntxt->fat_blk_hash_tbl+hash_index) = pblk;
            if (usage_flags) /* bit 31 0x80000000ul is write bit 29 0x20000000ul is lock */
                blockno |= usage_flags;
            *(pfatbuffcntxt->mapped_blocks+hash_index) = blockno;
            *(pfatbuffcntxt->mapped_data+hash_index) = pblk->fat_data;
            DEBUG_CHECK_FAT(pfatbuffcntxt, "Map 7")
#if (INCLUDE_FAILSAFE_CODE)
            /* We are coming from the FREE state, but write is
                set so journal the buffer */
            if (usage_flags & 0x80000000ul)
            {
                    if (pro_failsafe_journal_full(pdrive))
                        return(0); /* Failsafe sets errno */
            }
#endif
            return(pblk->fat_data);
        }
        else
        {
            if (!get_errno())
                rtfs_set_errno(PEIOERRORREADFAT); /* pc_map_fat_block device write error */
            return(0);
        }
    }
}

BOOLEAN pc_initialize_fat_block_pool(FATBUFFCNTXT *pfatbuffcntxt,
            int fat_buffer_size, FATBUFF *pfat_buffers,
            int fat_hashtbl_size,   FATBUFF **pfat_hash_table,
            byte **pfat_primary_cache, dword *pfat_primary_index)
{
dword t;

    pfatbuffcntxt->pfat_buffers = pfat_buffers; /* save the address of buffer pool */
    pfatbuffcntxt->mapped_data = pfat_primary_cache;
    pfatbuffcntxt->mapped_blocks = pfat_primary_index;
    pfatbuffcntxt->fat_blk_hash_tbl = pfat_hash_table;
    pfatbuffcntxt->hash_size = fat_hashtbl_size;
    /* Take size and subtract 1 to get the mask */
    t = (dword) fat_hashtbl_size;
    pfatbuffcntxt->hash_mask = t-1;
    pfatbuffcntxt->num_blocks = fat_buffer_size;
    /* Now clear it to the NO cached state */
    pc_free_all_fat_blocks(pfatbuffcntxt);
    return(TRUE);
}

/* Reset it fat buffer cache */
void    pc_free_all_fat_blocks(FATBUFFCNTXT *pfatbuffcntxt)
{
int i;
FATBUFF *pblk;

    /* Clear stats */
    pfatbuffcntxt->stat_primary_cache_hits =
    pfatbuffcntxt->stat_secondary_cache_hits =
    pfatbuffcntxt->stat_secondary_cache_loads =
    pfatbuffcntxt->stat_secondary_cache_swaps = 0;

    /* Clear hash tables */
    rtfs_memset(pfatbuffcntxt->fat_blk_hash_tbl,0, sizeof(FATBUFF *)*pfatbuffcntxt->hash_size);
    rtfs_memset(pfatbuffcntxt->mapped_data,(byte) 0, sizeof(byte *)*pfatbuffcntxt->hash_size);
    rtfs_memset(pfatbuffcntxt->mapped_blocks,(byte) 0, sizeof(dword)*pfatbuffcntxt->hash_size);
    /* Reset block lists */
    pfatbuffcntxt->puncommitted_blocks = 0;
    pfatbuffcntxt->pcommitted_blocks = 0;
    /* Regenerate the free list */
    pfatbuffcntxt->pfree_blocks = pfatbuffcntxt->pfat_buffers;
    pblk =  pfatbuffcntxt->pfat_buffers;
    for (i = 0; i < (pfatbuffcntxt->num_blocks-1); i++, pblk++)
    {
        rtfs_memset(pblk,(byte) 0, sizeof(FATBUFF));
        pblk->pnext = pblk+1;
    }
    rtfs_memset(pblk,(byte) 0, sizeof(FATBUFF));
    pblk->pnext = 0;
}

FATBUFF *pc_find_fat_blk(FATBUFFCNTXT *pfatbuffcntxt, dword blockno)
{
int hash_index;
FATBUFF *pblk;

    hash_index = (int) (blockno&pfatbuffcntxt->hash_mask);
    pblk = *(pfatbuffcntxt->fat_blk_hash_tbl+hash_index);
    while (pblk)
    {
        if (pblk->fat_blockno == blockno)
            return(pblk);
        else
            pblk=pblk->pnext2;
    }
    return(0);
}

static void pc_commit_fat_blk(FATBUFFCNTXT *pfatbuffcntxt, FATBUFF *pblk)
{
#if (DEBUG_FAT_CODE)
    if (pblk->fat_block_state != FATBLOCK_UNCOMMITTED)
        debug_break("commit fat block", __LINE__,"Not un-committed");
#endif
    /* Remove from the uncommitted list */
    if (pblk->pnext)
        pblk->pnext->pprev = pblk->pprev;
    if (pblk->pprev)
        pblk->pprev->pnext = pblk->pnext;
    if (pfatbuffcntxt->puncommitted_blocks == pblk)
        pfatbuffcntxt->puncommitted_blocks = pblk->pnext;

    /* Put on the committed list */
    pblk->fat_block_state = FATBLOCK_COMMITTED;
    pblk->pprev = 0;
    if (pfatbuffcntxt->pcommitted_blocks)
        pfatbuffcntxt->pcommitted_blocks->pprev = pblk;
    pblk->pnext = pfatbuffcntxt->pcommitted_blocks;
    pfatbuffcntxt->pcommitted_blocks = pblk;
}

void pc_commit_fat_table(FATBUFFCNTXT *pfatbuffcntxt)
{
dword b;
int i;
FATBUFF *pblk;
    for (i = 0; i < pfatbuffcntxt->hash_size; i++)
    {
        b = *(pfatbuffcntxt->mapped_blocks+i);
        if ( b && ((b & 0xc0000000ul) == 0x80000000ul) )
        { /* It changed to committed */
            /* Move from uncommitted list to committed list */
            pblk = pc_find_fat_blk(pfatbuffcntxt, (b & 0x0ffffffful));
            if (pblk) /* This should not fail */
                pc_commit_fat_blk(pfatbuffcntxt, pblk);
            b |= 0x40000000ul;  /* set commited flag in primary */
            *(pfatbuffcntxt->mapped_blocks+i) = b;
        }
    }
}
void pc_sort_committed_blocks(FATBUFFCNTXT *pfatbuffcntxt)
{
FATBUFF *pblk, *pprev, *psorted_list, *psort, *pblk_source_scan;

    /* The first element is the root of the sorted list, we begin from the
       next element scanning foreward and inserting in sorted order */
    psorted_list = pfatbuffcntxt->pcommitted_blocks; /* pfatbuffcntxt->pcommitted_blocks is guaranteed not null */

    pblk_source_scan = pfatbuffcntxt->pcommitted_blocks->pnext;
    if (pblk_source_scan)
    {
        psorted_list->pnext = 0;
        psorted_list->pprev = 0;
        pblk_source_scan->pprev = 0;
    }
    while (pblk_source_scan)
    {
        pblk = pblk_source_scan;
        pblk_source_scan = pblk_source_scan->pnext;

        pblk->pnext = 0;

        pprev = 0;
        psort = psorted_list;
        /* Find the first block in the sorted list with blockno > the new entry */
        while (psort && psort->fat_blockno < pblk->fat_blockno)
        {
            pprev = psort;
            psort = psort->pnext;
        }
        if (pprev)
        {   /* Insert the new one after pprev */
            if (pprev->pnext)
                pprev->pnext->pprev = pblk;
            pblk->pprev = pprev;
            pblk->pnext = pprev->pnext;
            pprev->pnext = pblk;
        }
        else
        {   /* We are the smallest */
            if (psorted_list)
                psorted_list->pprev = pblk;
            pblk->pprev = 0;
            pblk->pnext = psorted_list;
            psorted_list = pblk;
        }
    }
    pfatbuffcntxt->pcommitted_blocks = psorted_list;
}

#if (DEBUG_BLOCK_CODE || DEBUG_FAT_CODE)
void debug_break(char *where, dword line, char *message)
{
    if (line)
        printf("%s (%d): %s\n", where, line, message);
    else
        printf("%s: %s\n", where, message);
}
#endif

#if (DEBUG_FAT_CODE)
void debug_check_fat(FATBUFFCNTXT *pfatbuffcntxt, char *where)
{
FATBUFF *pblk;
FATBUFF *pblk_prev;
int nb = 0;
int numblocks;

    numblocks = pfatbuffcntxt->num_blocks;
    pblk = pfatbuffcntxt->pfree_blocks;
    while (pblk)
    {
        nb += 1;
        if (nb > numblocks)
            debug_break(where, "Bad Fat freelist");
        pblk = pblk->pnext;
    }
    pblk = pfatbuffcntxt->pcommitted_blocks;
    if (pblk && pblk->pprev)
        debug_break(where, "Bad fat committed root");
    while (pblk)
    {
        nb += 1;
        if (nb > numblocks)
            debug_break(where, "Bad fat committed list");
        if (pblk->fat_block_state != FATBLOCK_COMMITTED)
            debug_break(where, "Uncommitted blokc on committed list");
        pblk = pblk->pnext;
    }
    if (pfatbuffcntxt->pcommitted_blocks)
    {
        pblk_prev = pblk = pfatbuffcntxt->pcommitted_blocks;
        pblk = pblk->pnext;
        while (pblk)
        {
            if (pblk->pprev != pblk_prev)
                debug_break(where, "Bad link in committed list");
            pblk_prev = pblk;
            pblk = pblk->pnext;
        }
    }
    pblk = pfatbuffcntxt->puncommitted_blocks;
    if (pblk && pblk->pprev)
        debug_break(where, "Bad fat uncommitted root");
    while (pblk)
    {
        nb += 1;
        if (nb > numblocks)
            debug_break(where, "Bad fat uncommitted list");
        if (pblk->fat_block_state != FATBLOCK_UNCOMMITTED)
            debug_break(where, "Uncommitted blokc on committed list");
        pblk = pblk->pnext;
    }
    if (pfatbuffcntxt->puncommitted_blocks)
    {
        pblk_prev = pblk = pfatbuffcntxt->puncommitted_blocks;
        pblk = pblk->pnext;
        while (pblk)
        {
            if (pblk->pprev != pblk_prev)
                debug_break(where, "Bad link in uncommitted list");
            pblk_prev = pblk;
            pblk = pblk->pnext;
        }
    }

    if (nb != numblocks)
        debug_break(where, "Fat Leak");

}
#endif /* DEBUG_FAT_CODE */

#if (DEBUG_BLOCK_CODE)
void debug_check_blocks(BLKBUFFCNTXT *pbuffcntxt, int numblocks,  char *where, dword line)
{
BLKBUFF *pblk;
BLKBUFF *pblk_prev;
int nb = 0;
int nfreelist = 0;
int npopulatedlist = 0;

int i;
    pblk = pbuffcntxt->pfree_blocks;
    while (pblk)
    {
        nb += 1;
        if (nb > numblocks)
            debug_break(where,line, "Bad freelist");
        pblk = pblk->pnext;
    }
    nfreelist = nb;
    pblk = pbuffcntxt->ppopulated_blocks;
    if (pblk && pblk->pprev)
        debug_break(where,line, "Bad populated root");
    while (pblk)
    {
        npopulatedlist += 1;
        nb += 1;
        if (nb > numblocks)
            debug_break(where, line, "Bad populated list");
        pblk = pblk->pnext;
    }

    /* Add in outstanding scratch allocates */
    nb += pbuffcntxt->scratch_alloc_count;

    if (nb != numblocks)
        debug_break(where, line, "Leak");

    if (pbuffcntxt->ppopulated_blocks)
    {
        pblk_prev = pblk = pbuffcntxt->ppopulated_blocks;
        pblk = pblk->pnext;
        while (pblk)
        {
            if (pblk->pprev != pblk_prev)
                debug_break(where, line, "Bad link in populated list");
            pblk_prev = pblk;
            pblk = pblk->pnext;
        }
    }
    for (i = 0; i <  pbuffcntxt->hash_size; i++)
    {
        nb = 0;
        pblk = *(pbuffcntxt->blk_hash_tbl+i);
        while (pblk)
        {
            if (i != (int) (pblk->blockno&pbuffcntxt->hash_mask))
                debug_break(where, line, "Block in wrong hash slot");

            nb += 1;
            if (nb > numblocks)
                debug_break(where, line, "Loop in hash table");
            pblk = pblk->pnext2;
        }
    }
}

/* Diagnostic to display list list contents for FINODE and DROBJ pools.

    display_free_lists(char *in_where)

    Prints:
        FINODES on FREE list, FINODES on in use list.
        Drobj structures on  freelist,
        drob structure count marked free by scanning the drobj pool sequentially
        BLKBUFF buffer free count, and low water count
        BLKBUFF buffers counted on populated list
        BLKBUFF buffers counted on free list


        If populated count and free list don't add up the remainder will be scratch
        buffers.

        To Do: Add counters for scratch buffer allocation and frees.

    Useful for validating that no leaks are occuring.

    Requires printf


*/

void display_free_lists(char *in_where)
{
    FINODE *pfi;
    DROBJ *pobj;
    struct blkbuff *pblk;
    int j, i, objcount, finodecount,populated_block_count,free_list_count;
    objcount = finodecount = i = populated_block_count = free_list_count = 0;

    pfi = prtfs_cfg->mem_finode_freelist;
    while (pfi)
    {
        i++;
        pfi = pfi->pnext;
    }
    finodecount = 0;
    pfi = prtfs_cfg->inoroot;
    while (pfi)
    {
        finodecount++;
        pfi = pfi->pnext;
    }
    printf("%-10.10s:INODES  free:%4.4d in-use:%4.4d total:%4.4d \n", in_where, i,finodecount,prtfs_cfg->cfg_NFINODES);
    i = 0;
    pobj = prtfs_cfg->mem_drobj_freelist;
    while (pobj)
    {
        i++;
        pobj = (DROBJ *) pobj->pdrive;
    }
    pobj =  prtfs_cfg->mem_drobj_pool;
    objcount = 0;
    for (j = 0; j < prtfs_cfg->cfg_NDROBJS; j++, pobj++)
    {
        if (!pobj->is_free)
            objcount += 1;
    }
    printf("%-10.10s:DROBJS  free:%4.4d in-use:%4.4d total:%4.4d \n", in_where, i,objcount, prtfs_cfg->cfg_NDROBJS);

    pblk = prtfs_cfg->buffcntxt.ppopulated_blocks; /* uses pnext/pprev */
    populated_block_count = 0;
    while (pblk)
    {
        populated_block_count += 1;
        pblk = pblk->pnext;
    }
    printf("%-10.10s:BLKBUFS free:%4.4d in-use:%4.4d low w:%4.4d scratch:%4.4d total:%4.4d \n",in_where,
        prtfs_cfg->buffcntxt.num_free,
        populated_block_count,
        prtfs_cfg->buffcntxt.low_water,
        prtfs_cfg->buffcntxt.scratch_alloc_count,
        prtfs_cfg->buffcntxt.num_blocks);

    pblk = prtfs_cfg->buffcntxt.pfree_blocks;
    free_list_count = 0;
    while (pblk)
    {
        free_list_count += 1;
        pblk = pblk->pnext;
    }

    if (free_list_count != prtfs_cfg->buffcntxt.num_free)
    {
        printf("%-10.10s:Error num_freelist == %d but %d elements on the freelist\n",in_where, prtfs_cfg->buffcntxt.num_free, free_list_count);
    }
}

/* May be called to detect buffer pool leaks */
void check_blocks(DDRIVE *pdrive, char *prompt, dword line)
{
    debug_check_blocks(pdrive->pbuffcntxt, pdrive->pbuffcntxt->num_blocks, prompt, line);
}

#endif /* (DEBUG_BLOCK_CODE) */
