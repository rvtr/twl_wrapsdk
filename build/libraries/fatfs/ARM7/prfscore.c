/*               EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* PRFSCORE.C - ERTFS-PRO FailSafe internal routines */

#include <rtfs.h>
#if (INCLUDE_FAILSAFE_CODE)

BOOLEAN fs_copy_fat_block(FAILSAFECONTEXT *pfscntxt, dword replacement_block, dword blockno, dword fat_offset);
BOOLEAN fs_copy_dir_block(FAILSAFECONTEXT *pfscntxt, dword replacement_block, dword blockno);
BOOLEAN fs_copy_journal_block_pages(FAILSAFECONTEXT *pfscntxt);
BOOLEAN fs_copy_journal_fat_pages(FAILSAFECONTEXT *pfscntxt);
BOOLEAN fs_copy_journal_fsinfo_page(FAILSAFECONTEXT *pfscntxt);
BOOLEAN pro_failsafe_checksum_index(FAILSAFECONTEXT *pfscntxt, int *error);
BOOLEAN pro_failsafe_restore_buffers(FAILSAFECONTEXT *pfscntxt, int *error);
dword fs_check_mapped(FAILSAFECONTEXT *pfscntxt, dword blockno, int *error);
dword fs_map_block(FAILSAFECONTEXT *pfscntxt, dword blockno);
dword fs_block_map_scan(FAILSAFECONTEXT *pfscntxt, dword blockno, int *error);
dword fs_block_map_find_replacement(FAILSAFECONTEXT *pfscntxt, dword replacement_block, int *error);
dword fs_block_map_replace(FAILSAFECONTEXT *pfscntxt, dword blockno);
dword * fs_map_index_page(FAILSAFECONTEXT *pfscntxt, int index_page, BOOLEAN writing);
BOOLEAN fs_flush_index_pages(FAILSAFECONTEXT *pfscntxt);
int pro_failsafe_flush_header(FAILSAFECONTEXT *pfscntxt, dword status);


/* pro_failsafe_init_internal - Initiate a failsafe session
*
*   Summary:
*   BOOLEAN pro_failsafe_init_internal(DDRIVE *pdrive,
*               FAILSAFECONTEXT *pfscntxt)
*
* Description:
*
* This routine checks the user supplied parameters, initializes the
* failsafe context structure and places the drive in failsafe mode.
* Once the device is in failsafe mode directory block and FAT block
* writes will be held in a journal file pro_failsafe_commit() is called.
*
* This routine is called by both pro_failsafe_init() and
* pro_failsafe_auto_init(). Please see the manual page for
* pro_failsafe_init() for a discussion of arguments and
* configuration values.
*
* Inputs:
* pdrive   - drive structure
* pfscntxt - The address of a block of data that will be used as a
* context block for failsafe. This block must remain valid for the whole
* session and may not be deallocated.
*
*
* Returns:
* TRUE          - Success
* FALSE         - Error
*
* If it returns FALSE errno will be set to one of the following:
*
* PEFSREINIT       - Failsafe already initialized
* PEINVALIDPARMS   - Invalid parameters either no context block was passed
*                    the journal size parameter is too small
* PEINVALIDDRIVEID - Invalid drive
* PEFSCREATE       - Error creating the journal file
* An ERTFS system error
*
*/

BOOLEAN pro_failsafe_init_internal(DDRIVE *pdrive, FAILSAFECONTEXT *pfscntxt)
{
dword ltemp;
int  i;
FSBLOCKMAP *pbm;

    /* Make sure only valid bits are used */
    ltemp = FS_MODE_AUTOCOMMIT|FS_MODE_AUTORESTORE|FS_MODE_AUTORECOVER;
    ltemp = ~ltemp;
    if (pfscntxt->configuration_flags & ltemp)
    {
bad_parms:
            rtfs_set_errno(PEINVALIDPARMS);
            return (FALSE);
    }
    if (pfscntxt->user_journal_size)
    {
        /* Index in dwords */
        ltemp = pfscntxt->user_journal_size + JOURNAL_HEADER_SIZE;
        /* Index in blocks */
        pfscntxt->num_index_blocks =
           (ltemp+(JOURNAL_ENTRIES_P_BLOCK-1))/JOURNAL_ENTRIES_P_BLOCK;
        if (pfscntxt->num_index_blocks >= pfscntxt->user_journal_size)
            goto bad_parms;
        pfscntxt->num_remap_blocks = pfscntxt->user_journal_size - pfscntxt->num_index_blocks;
    }

    if (pfscntxt->blockmap_size)
    {
        pbm = pfscntxt->blockmap_freelist;
        if (!pbm)
            goto bad_parms;
        for( i = 1; i < pfscntxt->blockmap_size; i++)
        {
            pbm->pnext = (pbm+1);
            pbm += 1;
        }
        pbm->pnext = 0;
    }

    /* Point the drive at the failsafe context block to put the drive in
       failsafe mode */
    pfscntxt->pdrive = pdrive;
    pdrive->pfscntxt = (void *) pfscntxt;
    return(TRUE);
}

/* pro_failsafe_dskopen - Open failsafe operations on a disk volume
*
*
*   Summary:
*       BOOLEAN pro_failsafe_dskopen(DDRIVE *pdrive)
*
* Description:
*
* This routine is called by check_media when it automounts a volume.
* This routine creates or resets the journal file.
*
*
* Returns:
*   TRUE  - Success
*   FALSE - Error
*
* If FALSE is returned errno is set to one of these values
*
*   PEFSCREATE       - Error creating the journal file
*   PEIOERRORWRITEJOURNAL - Error initializing the journal file
*
*/

BOOLEAN pro_failsafe_dskopen(DDRIVE *pdrive)
{
FAILSAFECONTEXT *pfscntxt;
dword ltemp,*pdw;

    pfscntxt = (FAILSAFECONTEXT *) pdrive->pfscntxt;
    if (!pfscntxt)
        return(TRUE);
    /* Make sure journalling is off for the moment */
    pfscntxt->configuration_flags &= ~FS_MODE_JOURNALING;
    /* Calculate num_index_blocks and num_remap_blocks */
    /* if pfscntxt->user_journal_size is non zero num_remap_blocks
       and num_index_blocks were already calculated */
    if (!pfscntxt->user_journal_size)
    {
        pfscntxt->num_remap_blocks = pdrive->secpfat + CFG_NUM_JOURNAL_BLOCKS;
         /* Index in dwords */
         ltemp = pfscntxt->num_remap_blocks + JOURNAL_HEADER_SIZE;
        /* Index in blocks */
         pfscntxt->num_index_blocks =
           (ltemp+(JOURNAL_ENTRIES_P_BLOCK-1))/JOURNAL_ENTRIES_P_BLOCK;
    }

    /* Create the failsafe file */
    if (!failsafe_create_nv_buffer(pfscntxt))
    {
        rtfs_set_errno(PEFSCREATE);
        return(FALSE);
    }

    pdw = fs_map_index_page(pfscntxt, 0, TRUE);
    if (!pdw)
    {
         pfscntxt->journal_file_size = 0;
         return(FALSE);
     }
    /* Format the first block. Thats enough to clear the file */
    if (pro_failsafe_flush_header(pfscntxt, FS_STATUS_COMPLETE))
    {
         pfscntxt->journal_file_size = 0;
         return(FALSE);
    }
    /* Restore journal flag */
    pfscntxt->configuration_flags |= FS_MODE_JOURNALING;
    pfscntxt->known_free_clusters = pfscntxt->pdrive->known_free_clusters;
    return(TRUE);
}

/* pro_failsafe_journal_full - Check if a disk journal is full.
*
*   Summary:
*       BOOLEAN pro_failsafe_journal_full(DDRIVE *pdrive)
*
* Description:
*
* This routine is called by fat_map_block before it places a in the
* dirty state. IF the journal file is full this routine returns TRUE
* Forcing the fat map to fail. This catches Journal full errors
* earlier in the process
*/

BOOLEAN pro_failsafe_journal_full(DDRIVE *pdrive)
{
    FAILSAFECONTEXT *pfscntxt;
    pfscntxt = (FAILSAFECONTEXT *) pdrive->pfscntxt;
    if (pfscntxt && (pfscntxt->total_blocks_mapped >= pfscntxt->num_remap_blocks))
    {
        rtfs_set_errno(PEJOURNALFULL);
        return(TRUE);
    }
    else
        return(FALSE);
}



/* pro_failsafe_autorestore - Restore a disk volume at mount time
*
*   Summary:
*       BOOLEAN pro_failsafe_autorestore(DDRIVE *pdrive)
*
* Description:
*
* This routine is called by check_media when it automounts a volume.
* This routine restores the disk from the journal file if necessary (see
* below).
*
* If failsafe is configured for autorestore
* (pfscntxt->configuration_flags & FS_MODE_AUTORESTORE is true),
* and the journal file indicates that restore is needed, it attempts to
* restore the volume.
* If the restore fails and failsafe is not configured for autorecover, it
* returns an error that forces the disk mount to fail.
* If the restore fails and failsafe is configured for autorecover
* (pfscntxt->configuration_flags & FS_MODE_AUTORECOVER is true). Then the
* restore error is ignored.
*
* If failsafe is not configured for autorestore
* (pfscntxt->configuration_flags & FS_MODE_AUTORESTORE is not true),
* and the journal file indicates that restore is needed, it sets errno
* to PEFSRESTORENEEDED and returns FALSE, forcing the mount to fail.
**
* Returns:
*   TRUE - Success
*   FALSE - Error
*
* If FALSE is returned errno is set to one of these values
*
*   PEFSRESTOREERROR - Restore failed but not in autorecover mode
*   PEFSRESTORENEEDED- Restore needed but not in autorestore mode
*
*/

BOOLEAN pro_failsafe_autorestore(DDRIVE *pdrive)
{
FAILSAFECONTEXT *pfscntxt;
int restore_state;

    pfscntxt = (FAILSAFECONTEXT *) pdrive->pfscntxt;
    if (!pfscntxt)
        return(TRUE);

     /* This is excuted every time there is a mount so make sure
        the failsafe buffer is cleared */
    fs_rewind_failsafe(pfscntxt);
    /* Check the status but don't restore */
    restore_state = failsafe_restore_internal(pfscntxt, FALSE,FALSE);
    switch (restore_state)
    {
        case FS_STATUS_OK:
        case FS_STATUS_NO_JOURNAL:
            return(TRUE);
        case FS_STATUS_MUST_RESTORE:
            if (pfscntxt->configuration_flags & FS_MODE_AUTORESTORE)
            {
                /* Restore the journal file if autorestore enabled */
                /* Clear the directory cache */
                 pc_free_all_blk(pdrive);
                 /* Clear the fat cache */
                 pc_free_all_fat_blocks(&pdrive->fatcontext);
                 restore_state = failsafe_restore_internal(pfscntxt, TRUE, FALSE);
                 if (restore_state != FS_STATUS_RESTORED)
                 {
                     if (!(pfscntxt->configuration_flags & FS_MODE_AUTORECOVER))
                     {
                          pfscntxt->configuration_flags &= ~FS_MODE_JOURNALING;
                          rtfs_set_errno(PEFSRESTOREERROR);
                          return(FALSE);
                     }
                     /* Autorecover is enabled, so fall through and ignore
                        the error */
                 }
                /* We changed the FATS from the journal so close and reopen */
                pc_dskfree(pdrive->driveno);
                if (!pc_i_dskopen(pdrive->driveno))
                    return(FALSE);
            }
            else
            {
                rtfs_set_errno(PEFSRESTORENEEDED);
                return(FALSE);
            }
            break;
        case FS_STATUS_IO_ERROR:
            /* IO error during read */
            /* Make sure journalling is off for the moment */
            pfscntxt->configuration_flags &= ~FS_MODE_JOURNALING;
            rtfs_set_errno(PEIOERRORREADJOURNAL);
            return(FALSE);
        case FS_STATUS_BAD_JOURNAL:
        case FS_STATUS_BAD_CHECKSUM:
        case FS_STATUS_OUT_OF_DATE:
        default:
            if ( (pfscntxt->configuration_flags & FS_MODE_AUTORESTORE) &&
                 (pfscntxt->configuration_flags & FS_MODE_AUTORECOVER)) {
                 return(TRUE);
            }else{
                /* Make sure journalling is off for the moment */
                pfscntxt->configuration_flags &= ~FS_MODE_JOURNALING;
                rtfs_set_errno(PEFSRESTOREERROR);
                return(FALSE);
            }
            //break;
    } /* End switch */
    return(TRUE);
}

int pro_failsafe_flush_header(FAILSAFECONTEXT *pfscntxt, dword status)
{
dword *pdw;
    /* update index information to disk */
    pdw = fs_map_index_page(pfscntxt, 0, TRUE);
    if (!pdw)
        return(-1);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_SIGNATURE_1) ,FS_JOURNAL_SIGNATURE_1);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_SIGNATURE_2) ,FS_JOURNAL_SIGNATURE_2);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_VERSION    ) ,FS_JOURNAL_VERSION);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_TOTAL_BLOCKS_MAPPED),pfscntxt->total_blocks_mapped);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_INDEX_CHECKSUM),pfscntxt->journal_checksum);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_INDEX_STATUS),status);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_FREE_CLUSTERS),pfscntxt->known_free_clusters);
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_NUMB_INDEX_BLOCKS),pfscntxt->num_index_blocks );
    fr_DWORD((byte *) (pdw + INDEX_OFFSET_NUMB_REMAP_BLOCKS),pfscntxt->num_remap_blocks);
    if (!fs_flush_index_pages(pfscntxt))
        return(-1);
    return(0);
}


int pro_failsafe_commit_internal(FAILSAFECONTEXT *pfscntxt)
{
int   ret_val;
dword saved_free_clusters;

    if (!pc_flush_all_fil(pfscntxt->pdrive))
        return(-1);
    /* Flush any FAT buffers */
    if (!FATOP(pfscntxt->pdrive)->fatop_flushfat(pfscntxt->pdrive->driveno))
        return(-1);

    if (!pfscntxt->total_blocks_mapped)
        return(0);
    /* update index information to disk */
    if (pro_failsafe_flush_header(pfscntxt, FS_STATUS_PROCESSING))
    {
       ret_val = -1;
       goto ex_it;
    }
    if (!fs_copy_journal_block_pages(pfscntxt))
    {
       ret_val = -1;
       goto ex_it;
    }
    /* We are going to flush the fats now so invalidate
       the check for known_free_clusters changing. since
       it will give false positives */
    saved_free_clusters = pfscntxt->known_free_clusters;
    pfscntxt->known_free_clusters = 0xffffffff;
    if (pro_failsafe_flush_header(pfscntxt, FS_STATUS_PROCESSING))
    {
       pfscntxt->known_free_clusters = saved_free_clusters;
       ret_val = -1;
       goto ex_it;
    }
    if (!fs_copy_journal_fat_pages(pfscntxt))
    {
       pfscntxt->known_free_clusters = saved_free_clusters;
       ret_val = -1;
       goto ex_it;
    }
    if (pro_failsafe_flush_header(pfscntxt, FS_STATUS_PROCESSING))
    {
       pfscntxt->known_free_clusters = saved_free_clusters;
       ret_val = -1;
       goto ex_it;
    }
    if (!fs_copy_journal_fsinfo_page(pfscntxt))
    {
       pfscntxt->known_free_clusters = saved_free_clusters;
       ret_val = -1;
       goto ex_it;
    }
    if (pro_failsafe_flush_header(pfscntxt, FS_STATUS_COMPLETE))
    {
       pfscntxt->known_free_clusters = saved_free_clusters;
       ret_val = -1;
       goto ex_it;
    }
    fs_rewind_failsafe(pfscntxt);  /* Rewind the failsafe buffer */
    pfscntxt->known_free_clusters = pfscntxt->pdrive->known_free_clusters;
    ret_val = 0;
ex_it:
    return (ret_val);
}

BOOLEAN pro_failsafe_autocommit(DDRIVE *pdrive)
{
FAILSAFECONTEXT *pfscntxt;
    pfscntxt = (FAILSAFECONTEXT *) pdrive->pfscntxt;
    if (pfscntxt && (pfscntxt->configuration_flags & FS_MODE_AUTOCOMMIT) )
    {
        if (pro_failsafe_commit_internal(pfscntxt) != 0)
            return(FALSE);
    }
    return(TRUE);
}

int failsafe_restore_internal(FAILSAFECONTEXT *pfscntxt, BOOLEAN dorestore, BOOLEAN doclear)
{
dword *pdw;
int error;

    if (!pfscntxt)
    {
        return(FS_STATUS_NO_INIT);
    }

    /* If we fail, default to FS_STATUS_NO_JOURNAL. Under extraordinary
       conditions this will be changed to FS_STATUS_IO_ERROR */
    pfscntxt->open_status = FS_STATUS_NO_JOURNAL;
    if (!failsafe_reopen_nv_buffer(pfscntxt))
    {
        /* return either FS_STATUS_IO_ERROR or FS_STATUS_NO_JOURNAL */
        return(pfscntxt->open_status);
    }
    if (doclear)
    {
        if (!pro_failsafe_flush_header(pfscntxt, FS_STATUS_COMPLETE))
            return(FS_STATUS_OK);
        else
            return(FS_STATUS_IO_ERROR);
    }
    pdw = fs_map_index_page(pfscntxt, 0, FALSE);
    if (!pdw)
        return(FS_STATUS_IO_ERROR);
    if (
        (to_DWORD((byte *)(pdw+INDEX_OFFSET_SIGNATURE_1)) != FS_JOURNAL_SIGNATURE_1) ||
        (to_DWORD((byte *)(pdw+INDEX_OFFSET_SIGNATURE_2)) != FS_JOURNAL_SIGNATURE_2) ||
        (to_DWORD((byte *)(pdw+INDEX_OFFSET_VERSION)) != FS_JOURNAL_VERSION)
        )
    {
bad_journal:
        return(FS_STATUS_BAD_JOURNAL); /* Unknown status */
    }

    /* It's a valid file. Did it complete ? */
    if (to_DWORD((byte *)(pdw+INDEX_OFFSET_INDEX_STATUS)) == FS_STATUS_COMPLETE)
    {
        return(FS_STATUS_OK);
    }
    else if (to_DWORD((byte *)(pdw+INDEX_OFFSET_INDEX_STATUS)) != FS_STATUS_PROCESSING)
        goto bad_journal;

    /* It's a valid file but the disk flush did not complete */
    if (to_DWORD((byte *)(pdw+INDEX_OFFSET_FREE_CLUSTERS)) != (dword) pfscntxt->pdrive->known_free_clusters)
    {
        /* If free clusters value is all 1's, let it pass.
           we set free clusters field to all 1's before we write the
           FATS to the volume. To indicate that we have changed
           FATs and the free clusters test will give false
           positives */
        if (to_DWORD((byte *)(pdw+INDEX_OFFSET_FREE_CLUSTERS)) != 0xffffffff)
            return(FS_STATUS_OUT_OF_DATE);
    }

    /* It is a valid journal file, flush was begun but not completed so
       we must restore the fat and block buffer region */
    if (!pro_failsafe_checksum_index(pfscntxt, &error))
        return(error);

    if (!dorestore)
        return(FS_STATUS_MUST_RESTORE);
    else
    {
        if (!pro_failsafe_restore_buffers(pfscntxt,&error))
            return (error);
        /* We succeeded so write FS_STATUS_COMPLETE to the index */
        pdw = fs_map_index_page(pfscntxt, 0, TRUE);
        if (!pdw)
            return(FS_STATUS_IO_ERROR);
        fr_DWORD((byte *) (pdw + INDEX_OFFSET_INDEX_STATUS),FS_STATUS_COMPLETE);
        if (!fs_flush_index_pages(pfscntxt))
            return (FS_STATUS_IO_ERROR);
        return (FS_STATUS_RESTORED);
    }
}

BOOLEAN fs_copy_fat_block(FAILSAFECONTEXT *pfscntxt, dword replacement_block, dword blockno, dword fat_offset)
{
FATBUFF *pfatblk;
byte *pdata;

    pfatblk = pc_find_fat_blk(&(pfscntxt->pdrive->fatcontext), blockno);
    if (pfatblk)
        pdata = pfatblk->fat_data;
    else
    {
        pdata = pfscntxt->scratch_block;
        if (!failsafe_read_nv_buffer(pfscntxt, replacement_block, pdata))
            return(FALSE);
    }
    if (!(devio_write(pfscntxt->pdrive->driveno,
        blockno + fat_offset, pdata, (int) 1, FALSE) ))
        return(FALSE);
    return(TRUE);
}

BOOLEAN fs_copy_dir_block(FAILSAFECONTEXT *pfscntxt, dword replacement_block, dword blockno)
{
BLKBUFF *pblk;
byte *pdata;
BOOLEAN ret_val;

    OS_CLAIM_FSCRITICAL()
    pblk = pc_find_blk(pfscntxt->pdrive, blockno);
    if (pblk)
        pblk->use_count += 1;
    OS_RELEASE_FSCRITICAL()

    if (!pblk)
    {
        pdata = pfscntxt->scratch_block;
        if (!failsafe_read_nv_buffer(pfscntxt, replacement_block, pdata))
            return(FALSE);
    }
    else
        pdata = pblk->data;

    ret_val = devio_write(pfscntxt->pdrive->driveno,
            blockno, pdata, (int) 1, FALSE);
    if (pblk)
    {
    OS_CLAIM_FSCRITICAL()
    pblk->use_count -= 1;
    OS_RELEASE_FSCRITICAL()
    }
    return(ret_val);
}



BOOLEAN fs_copy_journal_block_pages(FAILSAFECONTEXT *pfscntxt)
{
struct fsblockmap *pbm;
dword fat_start,fat_end, replacement_block, blockno, info_block;
int error;

    fat_start =  pfscntxt->pdrive->fatblock;
    fat_end =    pfscntxt->pdrive->fatblock+pfscntxt->pdrive->secpfat;
    info_block = pfscntxt->pdrive->infosec;

    if (pfscntxt->blockmap_freelist)
    { /* If the freelist is non empty that means that all remap blocks are
         cached and in sorted order */
        pbm = pfscntxt->sorted_blockmap;
        while (pbm)
        {
            replacement_block = pbm->replacement_block;
            blockno = pbm->blockno;
            if (blockno != info_block && (blockno < fat_start || blockno >= fat_end))
            {
                if (!fs_copy_dir_block(pfscntxt, replacement_block, blockno))
                    return(FALSE);
            }
            pbm = pbm->pnext_by_block;
        }
    }
    else
    {
        dword ltemp;
        for (replacement_block = pfscntxt->num_index_blocks, ltemp = 0;
            ltemp < pfscntxt->total_blocks_mapped;
            ltemp++, replacement_block++)
        {
            error = 0;
            blockno = fs_block_map_find_replacement(pfscntxt, replacement_block, &error);
            if (error)
                return(FALSE);
            if (blockno != info_block && (blockno < fat_start || blockno >= fat_end))
            {
                if (!fs_copy_dir_block(pfscntxt, replacement_block, blockno))
                    return(FALSE);
            }
        }
    }
    return(TRUE);
}

BOOLEAN fs_copy_journal_fat_pages(FAILSAFECONTEXT *pfscntxt)
{
struct fsblockmap *pbm;
dword fat_start, fat_offset,fat_end, replacement_block, blockno;
int pass_number,error;

    fat_start = pfscntxt->pdrive->fatblock;
    fat_end =   pfscntxt->pdrive->fatblock+pfscntxt->pdrive->secpfat;
    fat_offset =  0;

    if (pfscntxt->blockmap_freelist)
    { /* If the freelist is non empty that means that all remap blocks are
         cached and in sorted order */
        for (pass_number = 0; pass_number < pfscntxt->pdrive->numfats; pass_number++)
        {
            pbm = pfscntxt->sorted_blockmap;
            while (pbm)
            {
                replacement_block = pbm->replacement_block;
                blockno = pbm->blockno;
                if (fat_start <= blockno && fat_end > blockno)
                { /* write fats on all passes */
                    if (!fs_copy_fat_block(pfscntxt, replacement_block, blockno, fat_offset))
                        return(FALSE);
                }
                pbm = pbm->pnext_by_block;
            }
            fat_offset += pfscntxt->pdrive->secpfat;
        }
    }
    else
    {
        for (pass_number = 0; pass_number < pfscntxt->pdrive->numfats; pass_number++)
        {
            dword ltemp;
            for (replacement_block = pfscntxt->num_index_blocks, ltemp = 0;
                ltemp < pfscntxt->total_blocks_mapped;
                ltemp++, replacement_block++)
            {
                error = 0;
                blockno = fs_block_map_find_replacement(pfscntxt, replacement_block, &error);
                if (error)
                    return(FALSE);
                if (fat_start <= blockno && fat_end > blockno)
                { /* write fats on all passes */
                    if (!fs_copy_fat_block(pfscntxt, replacement_block, blockno, fat_offset))
                        return(FALSE);
                }
            }
            fat_offset += pfscntxt->pdrive->secpfat;
        }
    }
    return(TRUE);
}

BOOLEAN fs_copy_journal_fsinfo_page(FAILSAFECONTEXT *pfscntxt)
{
struct fsblockmap *pbm;
dword info_block, replacement_block, blockno;
int error;

    info_block = pfscntxt->pdrive->infosec;

    /* Only FAT32 has an info block */
    if (!info_block)
        return(TRUE);

    if (pfscntxt->blockmap_freelist)
    { /* If the freelist is non empty that means that all remap blocks are
         cached and in sorted order */
        pbm = pfscntxt->sorted_blockmap;
        while (pbm)
        {
            replacement_block = pbm->replacement_block;
            blockno = pbm->blockno;
            if (blockno == info_block)
            {
                if (!fs_copy_dir_block(pfscntxt, replacement_block, blockno))
                    return(FALSE);
                break;
            }
            pbm = pbm->pnext_by_block;
        }
    }
    else
    {
        dword ltemp;
        for (replacement_block = pfscntxt->num_index_blocks, ltemp = 0;
            ltemp < pfscntxt->total_blocks_mapped;
            ltemp++, replacement_block++)
        {
            error = 0;
            blockno = fs_block_map_find_replacement(pfscntxt, replacement_block, &error);
            if (error)
                return(FALSE);
            if (blockno == info_block)
            {
                if (!fs_copy_dir_block(pfscntxt, replacement_block, blockno))
                    return(FALSE);
                break;
            }
        }
    }
    return(TRUE);
}


BOOLEAN pro_failsafe_checksum_index(FAILSAFECONTEXT *pfscntxt, int *error)
{
dword *pdw;
dword stored_check_sum,running_check_sum;
dword entries_this_record,entries_completed;
dword total_blocks_mapped;//,num_index_blocks;
int i,index_offset,index_page;

    *error = 0;
    pdw = fs_map_index_page(pfscntxt, 0, FALSE);
    if (!pdw)
    { /* fs_map_index_page set errno */
io_error:
        *error = FS_STATUS_IO_ERROR;
        return(FALSE);
    }
    total_blocks_mapped = to_DWORD((byte *)(pdw+INDEX_OFFSET_TOTAL_BLOCKS_MAPPED));
    //num_index_blocks = to_DWORD((byte *)(pdw+INDEX_OFFSET_NUMB_INDEX_BLOCKS));
    stored_check_sum = to_DWORD((byte *)(pdw+INDEX_OFFSET_INDEX_CHECKSUM));
    running_check_sum = 0;

    entries_completed = 0;
    entries_this_record = JOURNAL_ENTRIES_ZERO;
    index_offset = JOURNAL_HEADER_SIZE;
    index_page = 0;
    while (entries_completed < total_blocks_mapped)
    {
        if ( (entries_completed + entries_this_record) > total_blocks_mapped)
            entries_this_record = total_blocks_mapped-entries_completed;
        pdw = fs_map_index_page(pfscntxt, index_page, FALSE);
        if (!pdw)
            goto io_error;
        pdw += index_offset;
        for (i = 0; i < (int)entries_this_record; i++,pdw++)
            running_check_sum += to_DWORD((byte *)pdw);
        entries_completed += entries_this_record;
        index_offset = 0;
        entries_this_record = JOURNAL_ENTRIES_P_BLOCK;
        index_page += 1;
    }
    if (stored_check_sum != running_check_sum)
    {
        *error = FS_STATUS_BAD_CHECKSUM;
        return(FALSE);
    }
    return(TRUE);
}

BOOLEAN pro_failsafe_restore_buffers(FAILSAFECONTEXT *pfscntxt, int *error)
{
dword *pdw;
dword blockno,entries_this_record,entries_completed;
dword total_blocks_mapped,replacement_block;
dword fat_start, fat_end;
int i,index_offset,index_page;

    *error = 0;
    pdw = fs_map_index_page(pfscntxt, 0, FALSE);
    if (!pdw)
    { /* fs_map_index_page set errno */
io_error:
        *error = FS_STATUS_IO_ERROR;
        return(FALSE);
    }
    total_blocks_mapped = to_DWORD((byte *)(pdw+INDEX_OFFSET_TOTAL_BLOCKS_MAPPED));
    /* first replacement block is just past index blocks */
    replacement_block = to_DWORD((byte *)(pdw+INDEX_OFFSET_NUMB_INDEX_BLOCKS));

    fat_start = pfscntxt->pdrive->fatblock;
    fat_end =   pfscntxt->pdrive->fatblock+pfscntxt->pdrive->secpfat;

    entries_completed = 0;
    entries_this_record = JOURNAL_ENTRIES_ZERO;
    index_offset = JOURNAL_HEADER_SIZE;
    index_page = 0;
    while (entries_completed < total_blocks_mapped)
    {
        if ( (entries_completed + entries_this_record) > total_blocks_mapped)
            entries_this_record = total_blocks_mapped-entries_completed;
        pdw = fs_map_index_page(pfscntxt, index_page, FALSE);
        if (!pdw)
            goto io_error;
        pdw += index_offset;
        for (i = 0; i < (int)entries_this_record; i++,pdw++,replacement_block++)
        {
            blockno = to_DWORD((byte *)pdw);
            if (!failsafe_read_nv_buffer(pfscntxt, replacement_block,(byte *)pfscntxt->scratch_block))
                goto io_error;
            if (!(devio_write(pfscntxt->pdrive->driveno,
                blockno, (byte *)pfscntxt->scratch_block, (int) 1, FALSE) ))
                goto io_error;
            if (fat_start <= blockno && fat_end > blockno)
                if (!(devio_write(pfscntxt->pdrive->driveno,
                    blockno + pfscntxt->pdrive->secpfat, (byte *)pfscntxt->scratch_block, (int) 1, FALSE) ))
                    goto io_error;
        }
        entries_completed += entries_this_record;
        index_offset = 0;
        entries_this_record = JOURNAL_ENTRIES_P_BLOCK;
        index_page += 1;
    }
    return(TRUE);
}

void fs_rewind_failsafe(FAILSAFECONTEXT *pfscntxt)
{
int i,hash_index;
struct fsblockmap *pbm;
struct fsblockmap *pbm_temp;
    for (hash_index = 0; hash_index < FAILSAFE_HASHTABLE_SIZE; hash_index++)
    {
        pbm = pfscntxt->blockmap_hash_tbl[hash_index];
        while (pbm)
        {
            pbm_temp = pbm;
            pbm = pbm->pnext;
            pbm_temp->pnext_by_block = 0;
            pbm_temp->pnext = pfscntxt->blockmap_freelist;
            pfscntxt->blockmap_freelist = pbm_temp;
        }
        pfscntxt->blockmap_hash_tbl[hash_index] = 0;
    }
    /* Rewind the index page buffers */
    pfscntxt->next_index_buffer = 0;
    for (i = 0; i < CFG_NUM_INDEX_BUFFERS; i++)
    {
        pfscntxt->index_offset[i] = 0;
        pfscntxt->index_dirty[i] = 0;

    }

    /* Rewind the failsafe buffer */
    pfscntxt->sorted_blockmap = 0;
    pfscntxt->total_blocks_mapped = 0;
    pfscntxt->journal_checksum = 0;

}

dword fs_check_mapped(FAILSAFECONTEXT *pfscntxt, dword blockno, int *error)
{
int hash_index;
struct fsblockmap *pbm;
    *error = 0;
    hash_index = (int) blockno&FAILSAFE_HASHMASK;
    pbm = pfscntxt->blockmap_hash_tbl[hash_index];
    while (pbm)
    {
        if (pbm->blockno > blockno)
            break;  /* Sorted list and we are past it */
        if (pbm->blockno == blockno)
            return(pbm->replacement_block);
        pbm = pbm->pnext;
    }
    if (!pfscntxt->blockmap_freelist)
    { /* The free list is full double check in manual mode */
        return(fs_block_map_scan(pfscntxt, blockno, error));
    }
    return(0);
}

dword fs_block_map_replace(FAILSAFECONTEXT *pfscntxt, dword blockno);

dword fs_map_block(FAILSAFECONTEXT *pfscntxt, dword blockno)
{
int hash_index,error;
dword replacement_block;
struct fsblockmap *pbm;
struct fsblockmap *prevpbm;
struct fsblockmap *newpbm;
    hash_index = (int) blockno&FAILSAFE_HASHMASK;
    pbm = pfscntxt->blockmap_hash_tbl[hash_index];
    /* Search the hash table for a replacement */
    prevpbm = 0;
    while (pbm)
    {
        if (pbm->blockno > blockno)
            break;
        if (pbm->blockno == blockno)
            return(pbm->replacement_block);
        prevpbm = pbm;
        pbm = pbm->pnext;
    }
    /* We didn't find one. If we are out of block map structures search
       the actual map */
    if (!pfscntxt->blockmap_freelist)
    {
        error = 0;

        replacement_block = fs_block_map_scan(pfscntxt,blockno,&error);
        if (error) /* fs_block_map_scan set errno */
            return(0);
        if (replacement_block)
            return (replacement_block);
    }
    /* remap it now */
    replacement_block = fs_block_map_replace(pfscntxt,blockno);
    if (!replacement_block)
      return(0); /* fs_block_map_replace set errno */
    if (!pfscntxt->blockmap_freelist) /* out of block map structure. go manual */
        return (replacement_block);
    /* Allocate a block map structure and put in the hash table */
    newpbm = pfscntxt->blockmap_freelist;
    pfscntxt->blockmap_freelist = newpbm->pnext;
    newpbm->replacement_block = replacement_block;
    newpbm->blockno = blockno;

    if (prevpbm)
    {
        newpbm->pnext = prevpbm->pnext;
        prevpbm->pnext = newpbm;
    }
    else
    {
        newpbm->pnext = pbm;
        pfscntxt->blockmap_hash_tbl[hash_index] = newpbm;
    }
    /* Now link it in a list sorted by block number */
    newpbm->pnext_by_block = 0;
    if (!pfscntxt->sorted_blockmap)
        pfscntxt->sorted_blockmap = newpbm;
    else
    {

        prevpbm = 0;
        pbm = pfscntxt->sorted_blockmap;
        while (pbm)
        {
            if (blockno < pbm->blockno)
            {
                newpbm->pnext_by_block = pbm;
                if (prevpbm)
                    prevpbm->pnext_by_block = newpbm;
                else
                    pfscntxt->sorted_blockmap = newpbm;
                break;
            }
            else
            {
                prevpbm = pbm;
                pbm = pbm->pnext_by_block;
                if (!pbm)
                    prevpbm->pnext_by_block = newpbm;
            }
        }
    }
    return(replacement_block);
}

dword fs_block_map_scan(FAILSAFECONTEXT *pfscntxt, dword blockno, int *error)
{
dword *pdw;
dword blockno_little_endian;
dword replacement_block;
dword entries_this_record,entries_completed;
int i,index_offset,index_page;

    blockno_little_endian = to_DWORD((byte *)&blockno);
    replacement_block = pfscntxt->num_index_blocks; /* Starting block */

    entries_completed = 0;
    entries_this_record = JOURNAL_ENTRIES_ZERO;
    index_offset = JOURNAL_HEADER_SIZE;
    index_page = 0;
    while (entries_completed < pfscntxt->total_blocks_mapped)
    {
        if ( (entries_completed + entries_this_record)  >
              pfscntxt->total_blocks_mapped )
            entries_this_record = pfscntxt->total_blocks_mapped-entries_completed;
        pdw = fs_map_index_page(pfscntxt, index_page,FALSE);
        if (!pdw)
        { /* fs_map_index_page set errno */
            *error = 1;
            return(0);
        }
        pdw += index_offset;

        for (i = 0; i < (int)entries_this_record; i++,replacement_block++,pdw++)
            if (*pdw == blockno_little_endian)
                return(replacement_block);
        entries_completed += entries_this_record;
        index_offset = 0;
        entries_this_record = JOURNAL_ENTRIES_P_BLOCK;
        index_page += 1;
    }
    return(0); /* Did not find one */
}

dword fs_block_map_find_replacement(FAILSAFECONTEXT *pfscntxt, dword replacement_block, int *error)
{
dword ltemp, *pdw;
int index_page,index_offset;

    if (replacement_block < pfscntxt->num_index_blocks)
    { /* fs_map_index_page set errno */
        *error = 1;
        return(0);
    }

    ltemp = replacement_block-pfscntxt->num_index_blocks;
    if (ltemp < JOURNAL_ENTRIES_ZERO)
    {
        index_offset = (int) (ltemp+JOURNAL_HEADER_SIZE);
        index_page = 0;
    }
    else
    {
        ltemp = ltemp-JOURNAL_ENTRIES_ZERO;
        index_page = 1 + (int)ltemp/JOURNAL_ENTRIES_P_BLOCK;
        index_offset = (int) ltemp%JOURNAL_ENTRIES_P_BLOCK;
    }

    pdw = fs_map_index_page(pfscntxt, index_page, FALSE);
    if (!pdw)
    { /* fs_map_index_page set errno */
        *error = 1;
        return(0);
    }
    pdw += index_offset;
    return(to_DWORD((byte *)pdw));
}


dword fs_block_map_replace(FAILSAFECONTEXT *pfscntxt, dword blockno)
{
dword *pdw;
dword blockno_little_endian;
dword replacement_block;
int   index_offset,index_page;

    if (pfscntxt->total_blocks_mapped >= pfscntxt->num_remap_blocks)
    {
        rtfs_set_errno(PEJOURNALFULL);
        return(0);
    }

    blockno_little_endian = to_DWORD((byte *)&blockno);
    replacement_block = pfscntxt->total_blocks_mapped;

    if (replacement_block < JOURNAL_ENTRIES_ZERO)
    {
        index_page = 0;
        index_offset = (int) replacement_block + JOURNAL_HEADER_SIZE;
    }
    else
    {
    dword ltemp;
        ltemp = replacement_block - JOURNAL_ENTRIES_ZERO;
        index_page = 1 + (int)ltemp/JOURNAL_ENTRIES_P_BLOCK;
        index_offset = (int) ltemp%JOURNAL_ENTRIES_P_BLOCK;
    }
    pdw = fs_map_index_page(pfscntxt, index_page, TRUE);
    if (!pdw)
        return(0); /* fs_map_index_page set errno */
    pdw += index_offset;
    *pdw = blockno_little_endian;
    replacement_block = replacement_block + pfscntxt->num_index_blocks;
    pfscntxt->journal_checksum += blockno;
    pfscntxt->total_blocks_mapped += 1;
    return(replacement_block);
}

dword * fs_map_index_page(FAILSAFECONTEXT *pfscntxt, int index_page, BOOLEAN writing)
{
    int i,j,record_handle;

    record_handle = index_page + 1; /* add one since record_handle(0) == free */
    /* Search allocated buffer pool for a match */
    for (i = 0; i < CFG_NUM_INDEX_BUFFERS; i++)
    {
        if (pfscntxt->index_offset[i] == record_handle)
        {
            if (writing)
                pfscntxt->index_dirty[i] = TRUE;
            return (&pfscntxt->index_buffer[i][0]);
        }
    }
    /* swap out a buffer */
    j = pfscntxt->next_index_buffer;
    for (i = 0; i < CFG_NUM_INDEX_BUFFERS; i++)
    {
        /* don't swap first buffer record_handle == 1 because that
           contains journal_index_buffer and is accessed often */
        if (pfscntxt->index_offset[j] != 1)
        {
            if (pfscntxt->index_dirty[j])
            {
                /* write the buffer if it is dirty */
                if (!failsafe_write_nv_buffer(pfscntxt, pfscntxt->index_offset[j]-1, (byte *)pfscntxt->index_buffer[j]))
                {
                    rtfs_set_errno(PEIOERRORWRITEJOURNAL);
                    return(0);
                }
            }
            if (!failsafe_read_nv_buffer(pfscntxt, index_page, (byte *)pfscntxt->index_buffer[j]))
            {
                rtfs_set_errno(PEIOERRORREADJOURNAL);
                return(0);
            }

            pfscntxt->index_dirty[j] = writing;
            pfscntxt->index_offset[j] = record_handle;
            i = j+1;
            if (i == CFG_NUM_INDEX_BUFFERS)
                i = 0;
            pfscntxt->next_index_buffer = i;
            return (&pfscntxt->index_buffer[j][0]);
        }
        j = j+1;
        if (j == CFG_NUM_INDEX_BUFFERS)
            j = 0;
        pfscntxt->next_index_buffer = j;
    }
    return (0); /* Should never get here */
}

BOOLEAN fs_flush_index_pages(FAILSAFECONTEXT *pfscntxt)
{
    int i;
    for (i = 0; i < CFG_NUM_INDEX_BUFFERS; i++)
    {
        if (pfscntxt->index_dirty[i])
        {
            /* write the buffer if it is dirty */
            /* index_offset[i]-1 is because records are tagged 1-N not 0-N*/
            if (!failsafe_write_nv_buffer(pfscntxt, pfscntxt->index_offset[i]-1, (byte *)pfscntxt->index_buffer[i]))
            {
                rtfs_set_errno(PEIOERRORWRITEJOURNAL);
                return (FALSE); /* Should never get here */
            }
            pfscntxt->index_dirty[i] = FALSE;
         }
    }
    return (TRUE); /* Should never get here */
}


BOOLEAN block_devio_read(DDRIVE *pdrive, dword blockno, byte * buf)
{
int error = 0;
dword mapped_block;
FAILSAFECONTEXT *pfscntxt;

    /* Read from the journal file if enabled and the block is mapped */
    pfscntxt = (FAILSAFECONTEXT *) pdrive->pfscntxt;
    if (pfscntxt && (pfscntxt->configuration_flags & FS_MODE_JOURNALING))
    {
        mapped_block = fs_check_mapped(pfscntxt, blockno, &error);
        if (error)
            return(FALSE);
        if (mapped_block)
        {
            if (failsafe_read_nv_buffer(pfscntxt, mapped_block, buf))
                return(TRUE);
            else
            {
                rtfs_set_errno(PEIOERRORREADJOURNAL);
                return(FALSE);
            }
        }
    } /* not journalling or not mapped so read from the volume */
    return(devio_read(pdrive->driveno,  blockno, buf, (int) 1, FALSE));
}

BOOLEAN block_devio_write(BLKBUFF *pblk)
{
dword mapped_block;
FAILSAFECONTEXT *pfscntxt;
    /* Write to the journal file if enabled */
    pfscntxt = (FAILSAFECONTEXT *) pblk->pdrive->pfscntxt;
    if (pfscntxt && pfscntxt->configuration_flags & FS_MODE_JOURNALING)
    {
        mapped_block = fs_map_block(pfscntxt, pblk->blockno);
        if (!mapped_block)  /* fs_map_block() set errno */
            return(FALSE);
        if (failsafe_write_nv_buffer(pfscntxt, mapped_block,pblk->data))
            return (TRUE);
        else
        {
            rtfs_set_errno(PEIOERRORWRITEJOURNAL);
            return(FALSE);
        }
    }
    else /* not journalling or not mapped so read from the volume */
        return(devio_write(pblk->pdrive->driveno,pblk->blockno, pblk->data, (int) 1, FALSE));
}

BOOLEAN fat_devio_write(DDRIVE *pdrive, FATBUFF *pblk, int fatnumber)
{
dword mapped_block;
FAILSAFECONTEXT *pfscntxt;

    /* Write to the journal file if enabled */
    pfscntxt = (FAILSAFECONTEXT *) pdrive->pfscntxt;
    if (pfscntxt && pfscntxt->configuration_flags & FS_MODE_JOURNALING)
    {
        if (fatnumber)      /* Don't journal second FAT copy */
            return (TRUE);
        mapped_block = fs_map_block(pfscntxt, pblk->fat_blockno);
        if (!mapped_block)  /* fs_map_block() set errno */
            return(FALSE);
        if (failsafe_write_nv_buffer(pfscntxt, mapped_block,pblk->fat_data))
            return (TRUE);
        else
        {
            rtfs_set_errno(PEIOERRORWRITEJOURNAL);
            return(FALSE);
        }
    }
    else /* not journalling or not mapped so read from the volume */
    {
    dword blockno;
        if (fatnumber)
            blockno = pblk->fat_blockno+pdrive->secpfat;
        else
            blockno = pblk->fat_blockno;
        return(devio_write(pdrive->driveno,blockno, pblk->fat_data, (int) 1, FALSE));
    }
}

BOOLEAN fat_devio_read(DDRIVE *pdrive, dword blockno, byte *fat_data)
{
int error = 0;
dword mapped_block;
FAILSAFECONTEXT *pfscntxt;

    /* Read from the journal file if enabled and the block is mapped */
    pfscntxt = (FAILSAFECONTEXT *) pdrive->pfscntxt;
    if (pfscntxt && pfscntxt->configuration_flags & FS_MODE_JOURNALING)
    {
        mapped_block = fs_check_mapped(pfscntxt, blockno, &error);
        if (error)
            return(FALSE);
        if (mapped_block)
        {
            if (failsafe_read_nv_buffer(pfscntxt, mapped_block, fat_data))
                return(TRUE);
            else
            {
                rtfs_set_errno(PEIOERRORREADJOURNAL);
                return(FALSE);
            }
        }
    } /* not journalling or not mapped so read from the volume */
    return(devio_read(pdrive->driveno,  blockno, fat_data, (int) 1, FALSE));
}


#endif /* (INCLUDE_FAILSAFE_CODE) */


