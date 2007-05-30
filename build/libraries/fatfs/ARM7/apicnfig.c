/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS inc, 1996
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/

/******************************************************************************
PC_ERTFS_CONFIG() - Set up the ERTFS configuration block and allocate
or assign memory blocks for ERTFS usage. 

  The user must modify this code if he wishes to reconfigure ERTFS.
  He must also initialize the RTFS configuration structure with the 
  addresses of memory blocks to be used by ERTFS.
  
    Tutorial:
    pc_ertfs_config(void) initializes the ertfs configuration block and
    provides ERTFS with the addresses of memory that it needs. It is 
    called from the ertfs initialization routine pc_ertfs_init().
    
      This routine is designed to be modified by the user if he wishes to 
      change the default configuration. To simplify the user's task we 
      define some configuration constants in this file that may be modified
      to change the configuration. These constants are only used locally to
      this file. If another method of configuring ERTFS is more appropriate 
      for your environment then devize an alternate method to initialize the 
      configuration block.
      
        The default configuration is:
        
          ALLOC_FROM_HEAP     1   Use malloc() to allocate, set to 0 to use declared
          memory arrays.
          NDRIVES             10   L: Max number of drives from 0 to 25 A: - Z:
          Note: A FAT buffer pool must be allocated for each drives. If NDRIVES
          is changed you must add code to allocate or assign more FAT buffer pools.
          The default configuration allocates or assigns 10 fat buffer pools, 
          If you increase NDRIVES then add code that duplicates the cuurent code 
          that initialises prtfs_cfg->fat_buffers[9], to dircves 10, 11. 12 ...
          If you decrease NDRIVES then removes the cuurent code that 
          initialises prtfs_cfg->fat_buffers[9], 8, 7, 6 etc
          If you are not using ALLOC_FROM_HEAP you shaoul also add or remove the
          memory array declarations.
          NUM_USERS           1  Maximum number of USER contexts set to 1 for POLLED 
          mode, otherwise set to the number of tasks that 
          will simultaneously use ERTFS 
          NBLKBUFFS           10  Number of blocks in the buffer pool. Uses 532
          bytes per block. Impacts performance during
          directory traversals. Must be at least 4
          
            
              BLKHASHSIZE         16  Size of the block buffer hash table.
              This value must be a power of two.
              
                NUSERFILES          10  The maximum number of open Files at a time 
                
                  LARGE_FAT_SIZE  32  The number of 520 byte blocks committed for 
                  buffering fat blocks on drive C:
                  
                    LARGE_FAT_HASHSIZE  32  The number of 12 byte hash table entries 
                    committed for use on drive C:
                    This value must be a power of two.
                    
                      SMALL_FAT_SIZE  2  The number of 520 byte blocks committed for 
                      buffering fat blocks on drives other than C:
                      
                        SMALL_FAT_HASHSIZE  2 The number of 12 byte hash table entries 
                        committed for use on drives other than C:
                        This value must be a power of two.
                        
                          Note: Each drive may have a different fat buffer and cache sizes. For 
                          convenience here we only use two possible different sizes. You may tune 
                          each drive individually by setting the configuration value:
                          prtfs_cfg->cfg_FAT_BUFFER_SIZE[DRIVENUMBER]; and
                          prtfs_cfg->cfg_FAT_CACHE_SIZE[DRIVENUMBER]; 
                          prtfs_cfg->fat_buffers[DRIVENUMBER];
                          prtfs_cfg->fat_hash_table[DRIVENUMBER];
                          
*/
#include <rtfs.h>


#define ALLOC_FROM_HEAP  0   /* Set this to 1 to use malloc() to allocate 
ERTFS memory at startup, set to 0 to use
declare memory arrays and provide ertfs 
with the addresses of those arrays */
#define NDRIVES           8     /* Number of drives */
#define NUM_USERS         1     /* Must be one for POLLOS */
#define NBLKBUFFS         20    /* Number of blocks in the buffer pool. Uses 532
bytes per block. Impacts performance during
directory traversals must be at least 4 */
#define BLKHASHSIZE      16     /* This value must be a power of two.
Size of the block buffer hash table.  */

#define LARGE_FAT_SIZE  16      /* Number of 520 byte blocks committed for 
buffering fat blocks on drive C: */ //ctr modified 32->16

#define LARGE_FAT_HASHSIZE  16  /* This value must be a power of two.
Number of 12 byte hash table entries //ctr modified 32->16
committed for use on drive C: */
#define SMALL_FAT_SIZE   2       /* Number of 520 byte blocks committed for 
buffering fat blocks all other drives */

#define SMALL_FAT_HASHSIZE  2    /* This value must be a power of two.
Number of 12 byte hash table entries 
committed for use on all other drivees */



#define NUSERFILES       10    /* Maximum Number of open Files at a time */

/* Directory Object Needs. Conservative guess is One CWD per user per drive +
One per file + one per User for directory traversal */
/* This is calculated... do not change */
#define NFINODES   (32 + NUM_USERS*NDRIVES + NUM_USERS + NUSERFILES)
#define NDROBJS    (32 + NUM_USERS*NDRIVES + NUM_USERS + NUSERFILES)

RTFS_CFG *prtfs_cfg;   /* The user must initialize this value to point 
to a valid configuration block */
static RTFS_CFG rtfs_cfg_core;   /* The user must initialize this value to point */
#if (ALLOC_FROM_HEAP)
                                 /* Using malloc to allocate memory at startup. If malloc is not avaiable
                                 but some other heap manager is, then change the calls to malloc() to 
call your heap manager instead */
#include <malloc.h>
#else
/* Not using malloc to allocate memory so declare arrays that we can send 
assign to the configuration block at startup. */
DDRIVE          __mem_drives_structures[NDRIVES];   
BLKBUFF         __mem_block_pool[NBLKBUFFS];    
BLKBUFF *       __mem_block_hash_table[BLKHASHSIZE];
PC_FILE         __mem_file_pool[NUSERFILES];
DROBJ           __mem_drobj_pool[NDROBJS];
FINODE          __mem_finode_pool[NFINODES];
RTFS_SYSTEM_USER __rtfs_user_table[NUM_USERS];

FATBUFF KS_FAR __fat_buffer_0[SMALL_FAT_SIZE];
FATBUFF KS_FAR __fat_buffer_1[SMALL_FAT_SIZE];
FATBUFF KS_FAR __fat_buffer_2[LARGE_FAT_SIZE];
FATBUFF KS_FAR __fat_buffer_3[SMALL_FAT_SIZE];
FATBUFF KS_FAR __fat_buffer_4[SMALL_FAT_SIZE];
FATBUFF KS_FAR __fat_buffer_5[SMALL_FAT_SIZE];
FATBUFF KS_FAR __fat_buffer_6[SMALL_FAT_SIZE];
FATBUFF KS_FAR __fat_buffer_7[SMALL_FAT_SIZE];
FATBUFF KS_FAR __fat_buffer_8[SMALL_FAT_SIZE];
FATBUFF KS_FAR __fat_buffer_9[SMALL_FAT_SIZE];

FATBUFF * KS_FAR __fat_hash_table_0[SMALL_FAT_HASHSIZE];
FATBUFF * KS_FAR __fat_hash_table_1[SMALL_FAT_HASHSIZE];
FATBUFF * KS_FAR __fat_hash_table_2[LARGE_FAT_HASHSIZE];
FATBUFF * KS_FAR __fat_hash_table_3[SMALL_FAT_HASHSIZE];
FATBUFF * KS_FAR __fat_hash_table_4[SMALL_FAT_HASHSIZE];
FATBUFF * KS_FAR __fat_hash_table_5[SMALL_FAT_HASHSIZE];
FATBUFF * KS_FAR __fat_hash_table_6[SMALL_FAT_HASHSIZE];
FATBUFF * KS_FAR __fat_hash_table_7[SMALL_FAT_HASHSIZE];
FATBUFF * KS_FAR __fat_hash_table_8[SMALL_FAT_HASHSIZE];
FATBUFF * KS_FAR __fat_hash_table_9[SMALL_FAT_HASHSIZE];

byte * KS_FAR __fat_primary_cache_0[SMALL_FAT_HASHSIZE];
byte * KS_FAR __fat_primary_cache_1[SMALL_FAT_HASHSIZE];
byte * KS_FAR __fat_primary_cache_2[LARGE_FAT_HASHSIZE];
byte * KS_FAR __fat_primary_cache_3[SMALL_FAT_HASHSIZE];
byte * KS_FAR __fat_primary_cache_4[SMALL_FAT_HASHSIZE];
byte * KS_FAR __fat_primary_cache_5[SMALL_FAT_HASHSIZE];
byte * KS_FAR __fat_primary_cache_6[SMALL_FAT_HASHSIZE];
byte * KS_FAR __fat_primary_cache_7[SMALL_FAT_HASHSIZE];
byte * KS_FAR __fat_primary_cache_8[SMALL_FAT_HASHSIZE];
byte * KS_FAR __fat_primary_cache_9[SMALL_FAT_HASHSIZE];

dword  KS_FAR __fat_primary_index_0[SMALL_FAT_HASHSIZE];
dword  KS_FAR __fat_primary_index_1[SMALL_FAT_HASHSIZE];
dword  KS_FAR __fat_primary_index_2[LARGE_FAT_HASHSIZE];
dword  KS_FAR __fat_primary_index_3[SMALL_FAT_HASHSIZE];
dword  KS_FAR __fat_primary_index_4[SMALL_FAT_HASHSIZE];
dword  KS_FAR __fat_primary_index_5[SMALL_FAT_HASHSIZE];
dword  KS_FAR __fat_primary_index_6[SMALL_FAT_HASHSIZE];
dword  KS_FAR __fat_primary_index_7[SMALL_FAT_HASHSIZE];
dword  KS_FAR __fat_primary_index_8[SMALL_FAT_HASHSIZE];
dword  KS_FAR __fat_primary_index_9[SMALL_FAT_HASHSIZE];

#endif /* (ALLOC_FROM_HEAP) */

BOOLEAN pc_ertfs_config(void) /* __apifn__*/
{
    /* Important: prtfs_cfg must point to a configuration block */
    prtfs_cfg = &rtfs_cfg_core;
    
    /* Important: the configuration block must be zeroed */
    rtfs_memset(prtfs_cfg, 0, sizeof(rtfs_cfg_core));
    
    /* Set Configuration values */
    prtfs_cfg->cfg_NDRIVES              = NDRIVES;
    prtfs_cfg->cfg_NBLKBUFFS            = NBLKBUFFS;
    prtfs_cfg->cfg_BLK_HASHTBLE_SIZE    = BLKHASHSIZE;
    prtfs_cfg->cfg_NUSERFILES           = NUSERFILES;
    prtfs_cfg->cfg_NDROBJS              = NDROBJS;
    prtfs_cfg->cfg_NFINODES             = NFINODES;
    prtfs_cfg->cfg_NUM_USERS            = NUM_USERS;
    
    /* Set FAT size configuration values for each drive */
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[0]   = SMALL_FAT_SIZE;
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[1]   = SMALL_FAT_SIZE;
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[2]   = LARGE_FAT_SIZE;
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[3]   = SMALL_FAT_SIZE;
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[4]   = SMALL_FAT_SIZE;
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[5]   = SMALL_FAT_SIZE;
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[6]   = SMALL_FAT_SIZE;
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[7]   = SMALL_FAT_SIZE;
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[8]   = SMALL_FAT_SIZE;
    prtfs_cfg->cfg_FAT_BUFFER_SIZE[9]   = SMALL_FAT_SIZE;
    
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[0]  = SMALL_FAT_HASHSIZE;
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[1]  = SMALL_FAT_HASHSIZE;
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[2]  = LARGE_FAT_HASHSIZE;
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[3]  = SMALL_FAT_HASHSIZE;
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[4]  = SMALL_FAT_HASHSIZE;
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[5]  = SMALL_FAT_HASHSIZE;
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[6]  = SMALL_FAT_HASHSIZE;
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[7]  = SMALL_FAT_HASHSIZE;
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[8]  = SMALL_FAT_HASHSIZE;
    prtfs_cfg->cfg_FAT_HASHTBL_SIZE[9]  = SMALL_FAT_HASHSIZE;
    
    /* Core that must be provided by the user */
#if (!ALLOC_FROM_HEAP)
    
    /* Not using malloc() so assign memory arrays to the configuration block */
    prtfs_cfg->mem_drives_structures  = (DDRIVE *)              &__mem_drives_structures[0];
    prtfs_cfg->mem_block_pool         = (BLKBUFF *)             &__mem_block_pool[0];
    prtfs_cfg->mem_block_hash_table   = (BLKBUFF **)            &__mem_block_hash_table[0];
    prtfs_cfg->mem_file_pool          = (PC_FILE *)             &__mem_file_pool[0];
    prtfs_cfg->mem_drobj_pool         = (DROBJ *)               &__mem_drobj_pool[0];
    prtfs_cfg->mem_finode_pool        = (FINODE *)              &__mem_finode_pool[0];
    prtfs_cfg->rtfs_user_table        = (RTFS_SYSTEM_USER *)    &__rtfs_user_table[0];
    
    prtfs_cfg->fat_buffers[0] = (FATBUFF *)     &__fat_buffer_0[0];
    prtfs_cfg->fat_buffers[1] = (FATBUFF *)     &__fat_buffer_1[0];
    prtfs_cfg->fat_buffers[2] = (FATBUFF *)     &__fat_buffer_2[0];
    prtfs_cfg->fat_buffers[3] = (FATBUFF *)     &__fat_buffer_3[0];
    prtfs_cfg->fat_buffers[4] = (FATBUFF *)     &__fat_buffer_4[0];
    prtfs_cfg->fat_buffers[5] = (FATBUFF *)     &__fat_buffer_5[0];
    prtfs_cfg->fat_buffers[6] = (FATBUFF *)     &__fat_buffer_6[0];
    prtfs_cfg->fat_buffers[7] = (FATBUFF *)     &__fat_buffer_7[0];
    prtfs_cfg->fat_buffers[8] = (FATBUFF *)     &__fat_buffer_8[0];
    prtfs_cfg->fat_buffers[9] = (FATBUFF *)     &__fat_buffer_9[0];
    
    prtfs_cfg->fat_hash_table[0] = (FATBUFF **) &__fat_hash_table_0[0];
    prtfs_cfg->fat_hash_table[1] = (FATBUFF **) &__fat_hash_table_1[0];
    prtfs_cfg->fat_hash_table[2] = (FATBUFF **) &__fat_hash_table_2[0];
    prtfs_cfg->fat_hash_table[3] = (FATBUFF **) &__fat_hash_table_3[0];
    prtfs_cfg->fat_hash_table[4] = (FATBUFF **) &__fat_hash_table_4[0];
    prtfs_cfg->fat_hash_table[5] = (FATBUFF **) &__fat_hash_table_5[0];
    prtfs_cfg->fat_hash_table[6] = (FATBUFF **) &__fat_hash_table_6[0];
    prtfs_cfg->fat_hash_table[7] = (FATBUFF **) &__fat_hash_table_7[0];
    prtfs_cfg->fat_hash_table[8] = (FATBUFF **) &__fat_hash_table_8[0];
    prtfs_cfg->fat_hash_table[9] = (FATBUFF **) &__fat_hash_table_9[0];
    
    prtfs_cfg->fat_primary_cache[0] = (byte **) &__fat_primary_cache_0[0];
    prtfs_cfg->fat_primary_cache[1] = (byte **) &__fat_primary_cache_1[0];
    prtfs_cfg->fat_primary_cache[2] = (byte **) &__fat_primary_cache_2[0];
    prtfs_cfg->fat_primary_cache[3] = (byte **) &__fat_primary_cache_3[0];
    prtfs_cfg->fat_primary_cache[4] = (byte **) &__fat_primary_cache_4[0];
    prtfs_cfg->fat_primary_cache[5] = (byte **) &__fat_primary_cache_5[0];
    prtfs_cfg->fat_primary_cache[6] = (byte **) &__fat_primary_cache_6[0];
    prtfs_cfg->fat_primary_cache[7] = (byte **) &__fat_primary_cache_7[0];
    prtfs_cfg->fat_primary_cache[8] = (byte **) &__fat_primary_cache_8[0];
    prtfs_cfg->fat_primary_cache[9] = (byte **) &__fat_primary_cache_9[0];
    
    prtfs_cfg->fat_primary_index[0] = (dword *) &__fat_primary_index_0[0];
    prtfs_cfg->fat_primary_index[1] = (dword *) &__fat_primary_index_1[0];
    prtfs_cfg->fat_primary_index[2] = (dword *) &__fat_primary_index_2[0];
    prtfs_cfg->fat_primary_index[3] = (dword *) &__fat_primary_index_3[0];
    prtfs_cfg->fat_primary_index[4] = (dword *) &__fat_primary_index_4[0];
    prtfs_cfg->fat_primary_index[5] = (dword *) &__fat_primary_index_5[0];
    prtfs_cfg->fat_primary_index[6] = (dword *) &__fat_primary_index_6[0];
    prtfs_cfg->fat_primary_index[7] = (dword *) &__fat_primary_index_7[0];
    prtfs_cfg->fat_primary_index[8] = (dword *) &__fat_primary_index_8[0];
    prtfs_cfg->fat_primary_index[9] = (dword *) &__fat_primary_index_9[0];
    return(TRUE);
    
#else
    /* Use Malloc do allocated ERTFS data */
    prtfs_cfg->mem_drives_structures    = (DDRIVE *)    malloc(prtfs_cfg->cfg_NDRIVES*sizeof(DDRIVE));  
    prtfs_cfg->mem_block_pool           = (BLKBUFF*)    malloc(prtfs_cfg->cfg_NBLKBUFFS*sizeof(BLKBUFF));           
    prtfs_cfg->mem_block_hash_table     = (BLKBUFF **)  malloc(prtfs_cfg->cfg_BLK_HASHTBLE_SIZE*sizeof(BLKBUFF *));
    prtfs_cfg->mem_file_pool            = (PC_FILE *)   malloc(prtfs_cfg->cfg_NUSERFILES* sizeof(PC_FILE));         
    prtfs_cfg->mem_drobj_pool           = (DROBJ *)     malloc(prtfs_cfg->cfg_NDROBJS* sizeof(DROBJ));          
    prtfs_cfg->mem_finode_pool          = (FINODE *)    malloc(prtfs_cfg->cfg_NFINODES* sizeof(FINODE));        
    prtfs_cfg->rtfs_user_table          = (RTFS_SYSTEM_USER *) malloc(prtfs_cfg->cfg_NUM_USERS * sizeof(RTFS_SYSTEM_USER));     
    
    prtfs_cfg->fat_buffers[0] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[0]*sizeof(FATBUFF));
    prtfs_cfg->fat_buffers[1] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[1]*sizeof(FATBUFF));
    prtfs_cfg->fat_buffers[2] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[2]*sizeof(FATBUFF));
    prtfs_cfg->fat_buffers[3] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[3]*sizeof(FATBUFF));
    prtfs_cfg->fat_buffers[4] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[4]*sizeof(FATBUFF));
    prtfs_cfg->fat_buffers[5] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[5]*sizeof(FATBUFF));
    prtfs_cfg->fat_buffers[6] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[6]*sizeof(FATBUFF));
    prtfs_cfg->fat_buffers[7] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[7]*sizeof(FATBUFF));
    prtfs_cfg->fat_buffers[8] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[8]*sizeof(FATBUFF));
    prtfs_cfg->fat_buffers[9] = (FATBUFF *)     malloc(prtfs_cfg->cfg_FAT_BUFFER_SIZE[9]*sizeof(FATBUFF));
    
    prtfs_cfg->fat_hash_table[0] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[0]*sizeof(FATBUFF *));
    prtfs_cfg->fat_hash_table[1] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[1]*sizeof(FATBUFF *));
    prtfs_cfg->fat_hash_table[2] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[2]*sizeof(FATBUFF *));
    prtfs_cfg->fat_hash_table[3] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[3]*sizeof(FATBUFF *));
    prtfs_cfg->fat_hash_table[4] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[4]*sizeof(FATBUFF *));
    prtfs_cfg->fat_hash_table[5] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[5]*sizeof(FATBUFF *));
    prtfs_cfg->fat_hash_table[6] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[6]*sizeof(FATBUFF *));
    prtfs_cfg->fat_hash_table[7] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[7]*sizeof(FATBUFF *));
    prtfs_cfg->fat_hash_table[8] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[8]*sizeof(FATBUFF *));
    prtfs_cfg->fat_hash_table[9] = (FATBUFF **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[9]*sizeof(FATBUFF *));
    
    prtfs_cfg->fat_primary_cache[0] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[0]*sizeof(byte *));
    prtfs_cfg->fat_primary_cache[1] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[1]*sizeof(byte *));
    prtfs_cfg->fat_primary_cache[2] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[2]*sizeof(byte *));
    prtfs_cfg->fat_primary_cache[3] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[3]*sizeof(byte *));
    prtfs_cfg->fat_primary_cache[4] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[4]*sizeof(byte *));
    prtfs_cfg->fat_primary_cache[5] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[5]*sizeof(byte *));
    prtfs_cfg->fat_primary_cache[6] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[6]*sizeof(byte *));
    prtfs_cfg->fat_primary_cache[7] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[7]*sizeof(byte *));
    prtfs_cfg->fat_primary_cache[8] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[8]*sizeof(byte *));
    prtfs_cfg->fat_primary_cache[9] = (byte **) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[9]*sizeof(byte *));
    
    prtfs_cfg->fat_primary_index[0] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[0]*sizeof(dword));
    prtfs_cfg->fat_primary_index[1] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[1]*sizeof(dword));
    prtfs_cfg->fat_primary_index[2] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[2]*sizeof(dword));
    prtfs_cfg->fat_primary_index[3] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[3]*sizeof(dword));
    prtfs_cfg->fat_primary_index[4] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[4]*sizeof(dword));
    prtfs_cfg->fat_primary_index[5] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[5]*sizeof(dword));
    prtfs_cfg->fat_primary_index[6] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[6]*sizeof(dword));
    prtfs_cfg->fat_primary_index[7] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[7]*sizeof(dword));
    prtfs_cfg->fat_primary_index[8] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[8]*sizeof(dword));
    prtfs_cfg->fat_primary_index[9] = (dword *) malloc(prtfs_cfg->cfg_FAT_HASHTBL_SIZE[9]*sizeof(dword));
    
    /* Do some sanity checks */
    if (!prtfs_cfg->mem_drives_structures   ||
        !prtfs_cfg->mem_block_pool          ||
        !prtfs_cfg->mem_block_hash_table    ||
        !prtfs_cfg->mem_file_pool           ||
        !prtfs_cfg->mem_drobj_pool          ||
        !prtfs_cfg->mem_finode_pool         ||
        !prtfs_cfg->rtfs_user_table)
        goto malloc_failed;
    
    if (!prtfs_cfg->fat_buffers[0]         ||
        !prtfs_cfg->fat_buffers[1]          ||
        !prtfs_cfg->fat_buffers[2]          ||
        !prtfs_cfg->fat_buffers[3]          ||
        !prtfs_cfg->fat_buffers[4]          ||
        !prtfs_cfg->fat_buffers[5]          ||
        !prtfs_cfg->fat_buffers[6]          ||
        !prtfs_cfg->fat_buffers[7]          ||
        !prtfs_cfg->fat_buffers[8]          ||
        !prtfs_cfg->fat_buffers[9])
        goto malloc_failed;
    
    if (!prtfs_cfg->fat_hash_table[0]      ||
        !prtfs_cfg->fat_hash_table[1]       ||
        !prtfs_cfg->fat_hash_table[2]       ||
        !prtfs_cfg->fat_hash_table[3]       ||
        !prtfs_cfg->fat_hash_table[4]       ||
        !prtfs_cfg->fat_hash_table[5]       ||
        !prtfs_cfg->fat_hash_table[6]       ||
        !prtfs_cfg->fat_hash_table[7]       ||
        !prtfs_cfg->fat_hash_table[8]       ||
        !prtfs_cfg->fat_hash_table[9])
        goto malloc_failed;
    
    if (!prtfs_cfg->fat_primary_cache[0]   ||
        !prtfs_cfg->fat_primary_cache[1]    ||
        !prtfs_cfg->fat_primary_cache[2]    ||
        !prtfs_cfg->fat_primary_cache[3]    ||
        !prtfs_cfg->fat_primary_cache[4]    ||
        !prtfs_cfg->fat_primary_cache[5]    ||
        !prtfs_cfg->fat_primary_cache[6]    ||
        !prtfs_cfg->fat_primary_cache[7]    ||
        !prtfs_cfg->fat_primary_cache[8]    ||
        !prtfs_cfg->fat_primary_cache[9])
        goto malloc_failed;
    
    if (!prtfs_cfg->fat_primary_index[0]   ||
        !prtfs_cfg->fat_primary_index[1]    ||
        !prtfs_cfg->fat_primary_index[2]    ||
        !prtfs_cfg->fat_primary_index[3]    ||
        !prtfs_cfg->fat_primary_index[4]    ||
        !prtfs_cfg->fat_primary_index[5]    ||
        !prtfs_cfg->fat_primary_index[6]    ||
        !prtfs_cfg->fat_primary_index[7]    ||
        !prtfs_cfg->fat_primary_index[8]    ||
        !prtfs_cfg->fat_primary_index[9])
        goto malloc_failed;
    
    
    return(TRUE);
    
malloc_failed:
    if (prtfs_cfg->mem_drives_structures)   free(prtfs_cfg->mem_drives_structures );
    if (prtfs_cfg->mem_block_pool)          free(prtfs_cfg->mem_block_pool);
    if (prtfs_cfg->mem_block_hash_table)    free(prtfs_cfg->mem_block_hash_table);
    if (prtfs_cfg->mem_file_pool)           free(prtfs_cfg->mem_file_pool);
    if (prtfs_cfg->mem_drobj_pool)          free(prtfs_cfg->mem_drobj_pool);
    if (prtfs_cfg->mem_finode_pool)         free(prtfs_cfg->mem_finode_pool);
    if (prtfs_cfg->rtfs_user_table)         free(prtfs_cfg->rtfs_user_table);
    if (prtfs_cfg->fat_buffers[0])          free(prtfs_cfg->fat_buffers[0]);
    if (prtfs_cfg->fat_buffers[1])          free(prtfs_cfg->fat_buffers[1]);
    if (prtfs_cfg->fat_buffers[2])          free(prtfs_cfg->fat_buffers[2]);
    if (prtfs_cfg->fat_buffers[3])          free(prtfs_cfg->fat_buffers[3]);
    if (prtfs_cfg->fat_buffers[4])          free(prtfs_cfg->fat_buffers[4]);
    if (prtfs_cfg->fat_buffers[5])          free(prtfs_cfg->fat_buffers[5]);
    if (prtfs_cfg->fat_buffers[6])          free(prtfs_cfg->fat_buffers[6]);
    if (prtfs_cfg->fat_buffers[7])          free(prtfs_cfg->fat_buffers[7]);
    if (prtfs_cfg->fat_buffers[8])          free(prtfs_cfg->fat_buffers[8]);
    if (prtfs_cfg->fat_buffers[9])          free(prtfs_cfg->fat_buffers[9]);
    if (prtfs_cfg->fat_hash_table[0])       free(prtfs_cfg->fat_hash_table[0]);
    if (prtfs_cfg->fat_hash_table[1])       free(prtfs_cfg->fat_hash_table[1]);
    if (prtfs_cfg->fat_hash_table[2])       free(prtfs_cfg->fat_hash_table[2]);
    if (prtfs_cfg->fat_hash_table[3])       free(prtfs_cfg->fat_hash_table[3]);
    if (prtfs_cfg->fat_hash_table[4])       free(prtfs_cfg->fat_hash_table[4]);
    if (prtfs_cfg->fat_hash_table[5])       free(prtfs_cfg->fat_hash_table[5]);
    if (prtfs_cfg->fat_hash_table[6])       free(prtfs_cfg->fat_hash_table[6]);
    if (prtfs_cfg->fat_hash_table[7])       free(prtfs_cfg->fat_hash_table[7]);
    if (prtfs_cfg->fat_hash_table[8])       free(prtfs_cfg->fat_hash_table[8]);
    if (prtfs_cfg->fat_hash_table[9])       free(prtfs_cfg->fat_hash_table[9]);
    if (prtfs_cfg->fat_primary_cache[0])    free(prtfs_cfg->fat_primary_cache[0]);
    if (prtfs_cfg->fat_primary_cache[1])    free(prtfs_cfg->fat_primary_cache[1]);
    if (prtfs_cfg->fat_primary_cache[2])    free(prtfs_cfg->fat_primary_cache[2]);
    if (prtfs_cfg->fat_primary_cache[3])    free(prtfs_cfg->fat_primary_cache[3]);
    if (prtfs_cfg->fat_primary_cache[4])    free(prtfs_cfg->fat_primary_cache[4]);
    if (prtfs_cfg->fat_primary_cache[5])    free(prtfs_cfg->fat_primary_cache[5]);
    if (prtfs_cfg->fat_primary_cache[6])    free(prtfs_cfg->fat_primary_cache[6]);
    if (prtfs_cfg->fat_primary_cache[7])    free(prtfs_cfg->fat_primary_cache[7]);
    if (prtfs_cfg->fat_primary_cache[8])    free(prtfs_cfg->fat_primary_cache[8]);
    if (prtfs_cfg->fat_primary_cache[9])    free(prtfs_cfg->fat_primary_cache[9]);
    if (prtfs_cfg->fat_primary_index[0])    free(prtfs_cfg->fat_primary_index[0]);
    if (prtfs_cfg->fat_primary_index[1])    free(prtfs_cfg->fat_primary_index[1]);
    if (prtfs_cfg->fat_primary_index[2])    free(prtfs_cfg->fat_primary_index[2]);
    if (prtfs_cfg->fat_primary_index[3])    free(prtfs_cfg->fat_primary_index[3]);
    if (prtfs_cfg->fat_primary_index[4])    free(prtfs_cfg->fat_primary_index[4]);
    if (prtfs_cfg->fat_primary_index[5])    free(prtfs_cfg->fat_primary_index[5]);
    if (prtfs_cfg->fat_primary_index[6])    free(prtfs_cfg->fat_primary_index[6]);
    if (prtfs_cfg->fat_primary_index[7])    free(prtfs_cfg->fat_primary_index[7]);
    if (prtfs_cfg->fat_primary_index[8])    free(prtfs_cfg->fat_primary_index[8]);
    if (prtfs_cfg->fat_primary_index[9])    free(prtfs_cfg->fat_primary_index[9]);
    
    return(FALSE);
#endif
    
    
}

