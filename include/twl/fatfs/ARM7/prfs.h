/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
* This file is automatically included in rtfs.h if INCLUDE_FAILSAFE_CODE is enabled
* this file is note intended for inclusion by user code.
*/
/* PRFS.H - ERTFS-PRO Directory FailSafe routines */

/*
  CFG_VALIDATE_JOURNAL - Set this to one to force a a low level read of the
  whole journal file before it is accessed. If any of the reads call Failsafe
  tries to recover by re-writing the whole file with zeros, for some media
  types this will correct a read error. When the overwrite is completed
  the file is read again, if that succeeds we continue, otherwise the journal
  file open fails. A disk mount that encounters this error will fail and
  errno will be set to PEIOERRORREADJOURNAL */

#define CFG_VALIDATE_JOURNAL  0

/*
    CFG_VALIDATE_BUFFER_SIZE - This is the size, in blocks of the special
    buffer dedicated to performing the tasks described in CFG_VALIDATE_JOURNAL.
    The largest usable value for CFG_VALIDATE_BUFFER_SIZE is 128. Larger
    values will not be utilized but memory will be wasted.
    The whole journal file is scanned during startup in disk reads of up to
    CFG_VALIDATE_BUFFER_SIZE blocks. Larger values will require fewer reads
    and improve perfromance over smaller values.
    The buffer is declared as:
        static byte validate_buffer[CFG_VALIDATE_BUFFER_SIZE*512];
    in prfsnvio.c.
    If CFG_VALIDATE_JOURNAL is zero the value of CFG_VALIDATE_BUFFER_SIZE
    is irrelevant because the buffer is not declared.
*/

#define CFG_VALIDATE_BUFFER_SIZE 64

/*
  CFG_NUM_JOURNAL_BLOCKS - This is the default number of 512 byte blocks
  to assign to the failsafe for journalling directory blocks. If
  the user does not provide a non-zero value in the journal_size argument
  to pro_failsafe_init() then the journal file will sized to be large
  enough to hold the whole file allocation table plus CFG_NUM_JOURNAL_BLOCKS.
  The default value is 128. This represents a very small percentage of all
  but the smallest disks but it should be adequate for almost any imaginable
  usage. It may be reduced to as little as 1 and FailSafe will still work
  fine under most conditions.
*/

#define CFG_NUM_JOURNAL_BLOCKS  128

/*
  CFG_NUM_INDEX_BUFFERS - This compile time constant determines number of
  512 byte buffers to reserve for buffering FailSafe index pages. It
  effects the size of the failsafe context structure. It must be at
  least one and unless ram resources are very precious it should be
  set to at least two if possible. The default value is 4 which should
  be large enough for almost any application.
*/

#define CFG_NUM_INDEX_BUFFERS  4

/* Fail configuration and context structure */
typedef struct failsafecontext {
/* Configuration values to be set before calling pro_failsafe_init
   and not modified thereafter */
#define FS_MODE_AUTORESTORE            0x01
#define FS_MODE_AUTORECOVER            0x02
#define FS_MODE_AUTOCOMMIT             0x04
/* Internal flags set by failsafe. Not user assignable */
#define FS_MODE_JOURNALING             0x08
    int  configuration_flags;
/* configuration_flags values
  FS_MODE_AUTORESTORE- If removable media is reinserted and the
    journal file indicates that restore is needed, automatically restore
    and continue. If this flag is not set when this condition occurs
    the mount of the volume fails and the errno is set to PEFSRESTORENEEDED,
    the application must then handle this errno setting and call the
    pro_failsafe_restore routine
  FS_MODE_AUTORECOVER- If autorestore is required but the restore process
    failed due to a bad journal file or IO error abort the restore but don't
    report an error. Try to    reinitialize the journal file and continue.
    If a restore error did occur and FS_MODE_AUTORECOVER is not set the
    mount will fail and errno will be set to PEFSRESTOREERROR
  FS_MODE_AUTOCOMMIT- If AUTOCOMMIT is enabled the FailSafe commit operation
    will be performed automatically be ERTFS at the completion of each API
    call. With AUTOCOMMIT enabled the FailSafe operation is transparent to the
    user and it is not necessary to call pro_failsafe_commit().
*/
    dword            user_journal_size;        /* In blocks */

/* Internal elements, not intended to be accessed from user code */
    DDRIVE  *pdrive;
    dword   journal_file_size;        /* In blocks */
    dword   num_index_blocks; /* Blocks reserved remap index */
    dword   num_remap_blocks; /* Blocks reserved for block remaps */
    dword   total_blocks_mapped; /* number of valid index entries */
    dword   journal_checksum;    /* Checksum of all index entries */
    dword   known_free_clusters; /* number of valid index entries */
    dword   nv_buffer_handle;
    int    blockmap_size;
    struct fsblockmap *blockmap_freelist;
    struct fsblockmap *sorted_blockmap;
#define FAILSAFE_HASHTABLE_SIZE 16
#define FAILSAFE_HASHMASK 0xf
    struct fsblockmap *blockmap_hash_tbl[FAILSAFE_HASHTABLE_SIZE];
     int next_index_buffer;
     int index_offset[CFG_NUM_INDEX_BUFFERS];
     BOOLEAN index_dirty[CFG_NUM_INDEX_BUFFERS];
     dword index_buffer[CFG_NUM_INDEX_BUFFERS][128];
     byte scratch_block[512];
     /* Internal variable used to differentiate between IO
        errors and other errors during journal file re-opens */
     int open_status;
} FAILSAFECONTEXT;

typedef struct fsblockmap {
        struct fsblockmap *pnext;
        struct fsblockmap *pnext_by_block;
        dword blockno;
        dword replacement_block;
        } FSBLOCKMAP;

/* Return codes for pro_failsafe_restore() */
#define FS_STATUS_OK               0
#define FS_STATUS_NO_JOURNAL       1
#define FS_STATUS_BAD_JOURNAL      2
#define FS_STATUS_IO_ERROR         3
#define FS_STATUS_BAD_CHECKSUM     4
#define FS_STATUS_RESTORED         5
#define FS_STATUS_MUST_RESTORE     6
#define FS_STATUS_OUT_OF_DATE      7
#define FS_STATUS_NO_INIT          8

/* Failsafe api prototypes see prfsapi.c */
BOOLEAN pro_failsafe_init(byte *drivename, FAILSAFECONTEXT *pfscntxt);
BOOLEAN pro_failsafe_commit(byte *path);
int pro_failsafe_restore(byte *drive_name, FAILSAFECONTEXT *fscntxt, BOOLEAN dorestore,BOOLEAN doclear);
BOOLEAN pro_failsafe_auto_init(DDRIVE *pdrive);
BOOLEAN pro_failsafe_shutdown(byte *drive_name, BOOLEAN abort);


/* Routines called internally by ertfs. see prfscore.c */
BOOLEAN pro_failsafe_dskopen(DDRIVE *pdrive);
BOOLEAN pro_failsafe_autorestore(DDRIVE *pdrive);
void fs_rewind_failsafe(FAILSAFECONTEXT *pfscntxt);

/* The rest of this header file contains information that is internal to
   failsafe */
#define FS_JOURNAL_SIGNATURE_1          0x4641494C /* 'F''A''I''L' */
#define FS_JOURNAL_SIGNATURE_2          0x53414645 /* 'S''A''F''E' */
/* April-2004 version 2. - substantial rework  */
#define FS_JOURNAL_VERSION              0x00000002
#define FS_STATUS_PROCESSING            0x01010101
#define FS_STATUS_COMPLETE              0x11111111

/* Internal field offsets in failsafe header */
#define INDEX_OFFSET_SIGNATURE_1            0
#define INDEX_OFFSET_SIGNATURE_2            1
#define INDEX_OFFSET_VERSION                2
#define INDEX_OFFSET_INDEX_STATUS           3
#define INDEX_OFFSET_TOTAL_BLOCKS_MAPPED    4
#define INDEX_OFFSET_INDEX_CHECKSUM         5
#define INDEX_OFFSET_FREE_CLUSTERS          6
#define INDEX_OFFSET_NUMB_INDEX_BLOCKS      7
#define INDEX_OFFSET_NUMB_REMAP_BLOCKS      8
#define JOURNAL_HEADER_SIZE                 9 /* First 9 dwords are status */
#define JOURNAL_ENTRIES_P_BLOCK 128
#define JOURNAL_ENTRIES_ZERO (JOURNAL_ENTRIES_P_BLOCK-JOURNAL_HEADER_SIZE)

/* Prototypes of internal function implemented in prfscore.c */
BOOLEAN pro_failsafe_init_internal(DDRIVE *pdrive, FAILSAFECONTEXT *pfscntxt);
int pro_failsafe_commit_internal(FAILSAFECONTEXT *pfscntxt);
int failsafe_restore_internal(FAILSAFECONTEXT *pfscntxt, BOOLEAN dorestore, BOOLEAN doclear);
BOOLEAN pro_failsafe_dskopen(DDRIVE *pdrive);
BOOLEAN pro_failsafe_autorestore(DDRIVE *pdrive);
BOOLEAN pro_failsafe_autocommit(DDRIVE *pdrive);
BOOLEAN pro_failsafe_journal_full(DDRIVE *pdrive);

/* Prototypes of internal function implemented in prfsnvio.c */
BOOLEAN failsafe_reopen_nv_buffer(FAILSAFECONTEXT *pfscntxt);
BOOLEAN failsafe_create_nv_buffer(FAILSAFECONTEXT *pfscntxt);
BOOLEAN failsafe_write_nv_buffer(FAILSAFECONTEXT *pfscntxt, dword block_no, byte *pblock);
BOOLEAN failsafe_read_nv_buffer(FAILSAFECONTEXT *pfscntxt, dword block_no, byte *pblock);


