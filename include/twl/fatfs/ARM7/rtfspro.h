/*****************************************************************************
*Filename: RTFSPRO.H - Defines & structures for RTFSPRO Enhancements
*
*
* EBS - RTFS (Real Time File Manager)
*
* Copyright Peter Van Oudenaren , 2004
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
*
*
* Description: 
*   
* This file is automatically included in rtfs.h if INCLUDE_RTFS_PRO is enabled
* this file is note intended for inclusion by user code.
*
*
****************************************************************************/

#ifndef __RTFSPRO__
#define __RTFSPRO__ 1

struct pro_buffer_stats {
    /* Informational fields. These are filled in by a call to ?? */
int     failsafe_mode;
dword   failsafe_blocks_used;
dword   failsafe_blocks_free;
dword   total_block_buffers; 
dword   block_buffers_pending;
dword   block_buffers_available;
dword   block_buffers_fail;
dword   block_buffers_low;
dword   block_buffers_cache_hits;
dword   block_buffers_cache_misses;
dword   fat_buffers_pending;
dword   fat_buffers_free;
dword   fat_buffers_available;
dword   fat_buffer_primary_cache_hits;
dword   fat_buffer_secondary_cache_hits;
dword   fat_buffer_cache_loads;
dword   fat_buffer_cache_swaps;
dword   total_fat_buffers;
};

BOOLEAN pro_buffer_status(byte *drive_name, struct pro_buffer_stats *fsstat);

BOOLEAN pro_assign_buffers(byte *drivename, BLKBUFFCNTXT *block_buffer_context,
        int block_hashtable_size, BLKBUFF **block_hash_table,
        int block_buffer_pool_size, BLKBUFF *block_buffer_pool_data);
#endif

