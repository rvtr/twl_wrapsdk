/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APIPRO.C - Contains user api level source code for ERTFS-Pro functions.

    The following routines are included:

    pro_buffer_status       - Return disk, failsafe and buffer status.
    pro_assign_buffer_pool  - Assign a private buffer pool to a drive.
*/

#include <rtfs.h>

/* pro_buffer_status - Return disk and failsafe status.
*
*   Summary:
*     BOOLEAN pro_buffer_status(byte *drive_name, struct pro_buffer_stats *pstat)
* 
* Description:
* 
* This routine returns status information about failsafe, the block buffer 
* pools and the fat buffer pools. 
* The information is returned to the user in the structure of type
* pro_buffer_stats that must be passed to the routine.
*
*  If the routine succeeds it will return 0 and the following fields in 
*  pstat will be poulated:
* 
* int       failsafe_mode - Failsafe operating mode.
*   0 - Failsafe disabled
*   1 - Failsafe enabled 
* dword   failsafe_blocks_used
*   If failsafe is enabled this the number of blocks currently 
*   journalled in the FailSafe file. If FailSafe is disabled this 
*   value is zero.
* dword   failsafe_blocks_free
*   If failsafe is enabled this the number of blocks still available 
*   in the FailSafe file for use. If FailSafe is disabled this 
*   value is zero.
* dword   total_block_buffers;           
*   Total number of directory buffers available.
* dword   block_buffers_pending
*   Total number of directory blocks scheduled to write but not yet written 
*   If directory writeback mode is not enabled this value will be zero.
* dword   block_buffers_available;
*   Number of directory blocks still available to hold pending writes.
*   If directory writeback mode is not enabled this value will be zero.
* dword   block_buffers_cache_hits;
*   Number of block reads so far that were in the cache when the request
*   was made.
* dword   block_buffers_cache_misses;
*   Number of block reads so far that were not in the cache when the request
*   was made.
* dword   block_buffers_low
*   The low water mark or lowest number of block buffers that have been 
*   available for allocation so far. 
*   If directory writeback mode is not enabled this value will be zero.
* dword   block_buffers_fail
*   The number of block allocation failures that have occured due to 
*   insufficient buffer pool space
*   If directory writeback mode is not enabled this value will be zero.
* dword   block_buffers_cache_hits;
*   Number of block reads so far that were in the cache when the request
*   was made.
* dword   block_buffers_cache_misses;
*   Number of block reads so far that were not in the cache when the request
*   was made.
* dword   fat_buffer_primary_cache_hits
*   Number of fat block accesses so far that were in the fat primary cache 
*   when the request was made.
* dword   fat_buffer_secondary_cache_hits
*   Number of fat block accesses so far that were in the fat secondary cache 
*   when the request was made.
* dword   fat_buffer_cache_misses;
*   Number of fat block accesses so far that were in the fat secondary cache 
*   when the request was made.
* dword   total_fat_buffers
*   Total number of fat directory buffers for this drive. This value is 
*   determined by the value assigned to prtfs_cfg->cfg_FAT_BUFFER_SIZE[driveno] in apiconfig.c 
* dword   fat_buffers_pending
*   Total number of fat blocks scheduled to write but not yet written 
*   If FAT writeback mode is not enabled this value will be zero.
* dword   fat_buffers_available
*   Number of fat blocks still available to store pending writes.
*   If FAT writeback mode is not enabled this value will be zero.
* dword   fat_buffers_free
*   Number of fat buffer blocks that have never been used. If this value
*   is non-zero it means no fat swapping has occured. 
* dword fat_buffer_cache_loads
*   Number of fat block reads so far that were not in the cache when 
*   the request was made.
* dword   fat_buffer_cache_swaps;
*   Number of fat block swaps so far. Blocks that were written because they 
*   contained changed data but the buffer was required in order to load 
*   another block.
*
* Returns:
*   TRUE            - Success
*   FALSE           - Failure
* If FALSE is return errno will be set to one of the following.
*
*   PEINVALIDDRIVEID        - Drive argument invalid
*/

BOOLEAN pro_buffer_status(byte *drive_name, struct pro_buffer_stats *pstat)
{
#if (INCLUDE_FAILSAFE_CODE)
FAILSAFECONTEXT *pfscntxt;
#endif
int driveno;
DDRIVE *pdr;
FATBUFF  *pfblk;
    driveno = pc_parse_raw_drive(drive_name);
    if (driveno < 0)
    {
inval:
        rtfs_set_errno(PEINVALIDDRIVEID); 
        return(FALSE);
    }
    pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr || !(pdr->drive_flags&DRIVE_FLAGS_VALID) )
        goto inval;

    OS_CLAIM_LOGDRIVE(driveno)  /* pro_buffer_status Register drive in use */
    rtfs_set_errno(0);
#if (INCLUDE_FAILSAFE_CODE)
    pfscntxt = (FAILSAFECONTEXT *) pdr->pfscntxt;
    if (!pfscntxt)
    {
        pstat->failsafe_mode = 0;
        pstat->failsafe_blocks_used = 0;
        pstat->failsafe_blocks_free = 0;
    }
    else if (pfscntxt->configuration_flags & FS_MODE_JOURNALING)
    {
        pstat->failsafe_mode = 1;
        pstat->failsafe_blocks_used = pfscntxt->total_blocks_mapped;
        pstat->failsafe_blocks_free = pfscntxt->num_remap_blocks - 
                                        pfscntxt->total_blocks_mapped;
    }
    else
    {
        pstat->failsafe_mode = 2;
        pstat->failsafe_blocks_used = pfscntxt->total_blocks_mapped;
        pstat->failsafe_blocks_free = pfscntxt->num_remap_blocks - 
                                        pfscntxt->total_blocks_mapped;
    }
#else
    pstat->failsafe_mode = 0;
    pstat->failsafe_blocks_used = 0;
    pstat->failsafe_blocks_free = 0;
#endif

    OS_CLAIM_FSCRITICAL()
    /* Critical since block buffers may be shared among drives */
    pstat->total_block_buffers      = pdr->pbuffcntxt->num_blocks;           
    pstat->block_buffers_pending        = 0;
/*    for (pblk = pdr->pbuffcntxt->ppopulated_blocks; pblk; pblk = pblk->pnext) */
/*    { */
/*        if (pblk->block_state == DIRBLOCK_COMMITTED) */
/*            pstat->block_buffers_pending += 1; */
/*    } */
    pstat->block_buffers_available  = pstat->total_block_buffers - 
                                            pstat->block_buffers_pending;
    pstat->block_buffers_cache_hits     = pdr->pbuffcntxt->stat_cache_hits;
    pstat->block_buffers_cache_misses   = pdr->pbuffcntxt->stat_cache_misses;
    pstat->block_buffers_low            = pdr->pbuffcntxt->low_water;
    pstat->block_buffers_fail           = pdr->pbuffcntxt->num_alloc_failures;
    OS_RELEASE_FSCRITICAL()

    pstat->total_fat_buffers 
            = pdr->fatcontext.num_blocks;
    pstat->fat_buffer_primary_cache_hits
            = pdr->fatcontext.stat_primary_cache_hits;
    pstat->fat_buffer_secondary_cache_hits
            = pdr->fatcontext.stat_secondary_cache_hits;
    pstat->fat_buffer_cache_loads
            = pdr->fatcontext.stat_secondary_cache_loads;
    pstat->fat_buffer_cache_swaps
            = pdr->fatcontext.stat_secondary_cache_swaps;

    pstat->fat_buffers_free = 0;
    for (pfblk = pdr->fatcontext.pfree_blocks;pfblk;pfblk=pfblk->pnext)
        pstat->fat_buffers_free += 1;
    pstat->fat_buffers_pending = 0;
    for (pfblk = pdr->fatcontext.pcommitted_blocks;pfblk;pfblk=pfblk->pnext)
        pstat->fat_buffers_pending += 1;
    pstat->fat_buffers_available    
            = pstat->total_fat_buffers - pstat->fat_buffers_pending;
    OS_RELEASE_LOGDRIVE(driveno) /* pro_buffer_status release */
    return(TRUE);
}

/* pro_assign_buffer_pool - Assign a private buffer pool to a drive.
*
*   Summary:
*       BOOLEAN pro_assign_buffer_pool(byte *drive_name, 
*                             int     block_hashtable_size,
*                             BLKBUFF **block_hash_table,
*                             int     block_buffer_pool_size,
*                             BLKBUFF *block_buffer_pool_data) 
* 
* Description:
*
* This routine provides a way to assign a private block 
* buffer pool to a named drive. Normally block buffer space 
* is shared among all drives in the system, but this function
* allows the caller to assign block buffers that are private to
* the drive.
*  
* This function may improve performance by reducing the amount 
* of buffer swapping that occurs when multiple drives compete for 
* buffers in the common pool.
* 
* It is advisable to assign a private buffer pool to a drive 
* if writeback block buffering mode is being used. Failure to do so 
* may result in block allocation failures occuring when using other  
* drives because blocks are being consumed by pending writes.
* 
*
* Inputs:
* drive_name - Null terminated drive designator, for example "A:"
* block_buffer_context - User supplied memory for the block buffer context
*                        block that will be assigned to the drive to be used
*                        as a private buffer pool management structure.
*                        This must be a pointer to a structure of type
*                        BLKBUFFCNTXT that must remain valid for the whole
*                        session and may not be deallocated.
* block_hashtable_size - You must set this to the size of the block hash 
*                        table. Each entry in the table takes up 4 bytes.
*                        This value must be a power of two.
* block_hash_table     - User supplied memory for the block hash table. 
*                        This must be a pointer to an array of data of type
*                        BLKBUF *, containing block_hashtable_size elements.
*                        It must remain valid for the whole session and must
*                        not be deallocated.
* block_buffer_pool_size-You must set this to the size of the block buffer
*                        pool. Each entry in buffer pool requires 
*                        approximately 540 bytes.
* block_buffer_pool_data - User supplied memory for the block buffer pool.
*                        This must be a pointer to an array of data of type
*                        BLKBUF, containing block_buffer_pool_size elements.
*                        It must remain valid for the whole session and must
*                        not be deallocated.
* 
* Returns:
* TRUE             - Success
* FALSE            - Error 
*
* If it returns FALSE errno will be set to one of the following:
*
* PEINVALIDPARMS   - Invalid parameters either no context block was passed
*                    or the fields were not initialized correctly.
* PEINVALIDDRIVEID - Invalid drive
* An ERTFS system error 
* 
*/

BOOLEAN pro_assign_buffers(byte *drivename, 
                       BLKBUFFCNTXT *block_buffer_context,
                       int          block_hashtable_size,
                       BLKBUFF      **block_hash_table,
                       int          block_buffer_pool_size,
                       BLKBUFF      *block_buffer_pool_data) 
{
DDRIVE *pdrive;
int  driveno;

    if (!pc_parsedrive( &driveno, drivename))
        return (-1);
    OS_CLAIM_LOGDRIVE(driveno)  /* pro_assign_buffers register drive in use */
    if ( (block_hashtable_size & 0x3) ||
        (block_buffer_pool_size < 2) ||
        (!block_hash_table) ||
        (!block_buffer_pool_data) ||
        (!block_buffer_context) )
    {
        goto bad_args;
    }
    /* Initialize the block buffer pool */
    if (!pc_initialize_block_pool(
        block_buffer_context,
        block_buffer_pool_size,
        block_buffer_pool_data,
        block_hashtable_size,
        block_hash_table))
    {
bad_args:
        OS_RELEASE_LOGDRIVE(driveno) /* pro_assign_buffers release */
        rtfs_set_errno(PEINVALIDPARMS);
        return (-1);
    }

    /* Assign a private block buffer pool to the drive */
    pdrive = pc_drno_to_drive_struct(driveno);
    /* Make sure no blocks are buffered for this drive */ 
    pc_free_all_blk(pdrive);
    pdrive->pbuffcntxt = block_buffer_context;

    OS_RELEASE_LOGDRIVE(driveno)
    return(0);
}


