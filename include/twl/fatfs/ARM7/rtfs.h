/*****************************************************************************
*Filename: RTFS.H - Defines & structures for RTFS ms-dos utilities
*
*
* EBS - RTFS (Real Time File Manager)
*
* Copyright Peter Van Oudenaren , 1993
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
*
*
* Description:
*
*
*
*
****************************************************************************/

#ifndef __RTFS__
#define __RTFS__ 1

/* Set this next line to 1 if not building RTFS */
#define CDFS_ONLY 0

typedef unsigned long int BLOCKT;   /* 32 BIT unsigned */

#include "rtfs_naming_convention.h"
#include "rtfsconf.h"
#include "csstrtab.h"

/* Begin API Definition section - the following lines export the public
   API and user required defines RTFS internal definitions begin
   at the line #define BLOCKEQ0 0L
*/


typedef int PCFD;                  /* file desc */

#define ARDONLY 0x1  /* MS-DOS File attributes */
#define AHIDDEN 0x2
#define ASYSTEM 0x4
#define AVOLUME 0x8
#define ADIRENT 0x10
#define ARCHIVE 0x20
#define ANORMAL 0x00
#define CHICAGO_EXT 0x0f    /* Chicago extended filename attribute */


/* Date stamping buffer */
typedef struct datestr {
        word date;
        word time;
        } DATESTR;
/* Structure for use by pc_gfirst, pc_gnext */
typedef struct dstat {
        byte    fname[10];           /* Null terminated file and extension only 9 bytes used */
        byte    fext[4];
#if (VFAT)
        byte    lfname[FILENAMESIZE_BYTES];         /* Long file name for vfat. */
#else
        byte    lfname[14];                   /* Long file name non-vfat. */
#endif
        byte    filename[14];       /* Null terminated file.ext only 13 bytes used */
        byte    fattribute;         /* File attributes */
        word    ftime;              /* time & date lastmodified. See date */
        word    fdate;              /* and time handlers for getting info */
        dword   fsize;              /* File size */
        /* INTERNAL */
        int     driveno;
        int drive_opencounter;      /* Value of drive structures opencounter */
/*        byte    pname[10];            Pattern. */
        byte    pname[FILENAMESIZE_BYTES];
        byte    pext[4];
        byte    path[EMAXPATH_BYTES];
        void   *pobj;                 /* Info for getting at the inode */
        void   *pmom;                 /* Info for getting at parent inode */
        }
DSTAT;

/* Structure for use by pc_stat and pc_fstat */
/* Portions of this structure and the macros were lifted from BSD sources.
   See the RTFS ANSI library for BSD terms & conditions */
typedef struct ertfs_stat
{
    int st_dev;      /* (drive number, rtfs) */
    int st_ino;      /* inode number (0) */
    dword   st_mode;        /* (see S_xxxx below) */
    int st_nlink;      /* (always 1) */
    int st_rdev;        /* (drive number, rtfs) */
    dword   st_size;        /* file size, in bytes */
    DATESTR  st_atime;     /* last access (all times are the same) */
    DATESTR  st_mtime;     /* last modification */
    DATESTR  st_ctime;     /* last file status change */
    long    st_blksize;   /* optimal blocksize for I/O (cluster size) */
    long    st_blocks;     /* blocks allocated for file */
    byte   fattribute;    /* File attributes - DOS attributes
                                (non standard but useful) */
} ERTFS_STAT;

/* File seg info structure. An array of these structures is passed
    to pc_get_file_extents(). The extents of the file are returned
    in this array */
typedef struct fileseginfo {
        long    block;          /* Block number of the current extent */
        long    nblocks;        /* Number of blocks in the extent */
        } FILESEGINFO;

/* Free list info structure. An array of these structures is passed
    to pc_get_free_list(). The list of free clusters is returned in
    this array */
typedef struct freelistinfo {
    dword       cluster;        /* Cluster where the free region starts */
    long        nclusters;      /* Number of free clusters the free segment */
    } FREELISTINFO;

typedef struct chkdisk_stats {
    dword  n_user_files;                    /* Total #user files found */
    dword  n_hidden_files;                  /* Total #hidden files found */
    dword  n_user_directories;              /* Total #directories found */
    dword  n_free_clusters;                 /* # free available clusters */
    dword  n_bad_clusters;                  /* # clusters marked bad */
    dword  n_file_clusters;                 /* Clusters in non hidden files */
    dword  n_hidden_clusters;               /* Clusters in hidden files */
    dword  n_dir_clusters;                  /* Clusters in directories */
    dword  n_crossed_points;                /* Number of crossed chains. */
    dword  n_lost_chains;                   /* # lost chains */
    dword  n_lost_clusters;                 /* # lost clusters */
    dword  n_bad_lfns;                      /* # corrupt/disjoint win95 lfn chains */
} CHKDISK_STATS;


/* Values for the st_mode field */
#define S_IFMT   0170000        /* type of file mask */
#define S_IFCHR  0020000       /* character special (unused) */
#define S_IFDIR  0040000       /* directory */
#define S_IFBLK  0060000       /* block special  (unused) */
#define S_IFREG  0100000       /* regular */
#define S_IWRITE 0000400    /* Write permitted  */
#define S_IREAD  0000200    /* Read permitted. (Always true anyway)*/

#define DEFFILEMODE (S_IREAD|S_IWRITE)
#define S_ISDIR(m)  ((m & 0170000) == 0040000)  /* directory */
#define S_ISCHR(m)  ((m & 0170000) == 0020000)  /* char special */
#define S_ISBLK(m)  ((m & 0170000) == 0060000)  /* block special */
#define S_ISREG(m)  ((m & 0170000) == 0100000)  /* regular file */
#define S_ISFIFO(m) ((m & 0170000) == 0010000)  /* fifo */

/* Error codes */

#define PCERR_FAT_FLUSH 0 /*Cant flush FAT */
#define PCERR_INITMEDI  1 /*Not a DOS disk:pc_dskinit */
#define PCERR_INITDRNO  2 /*Invalid driveno to pc_dskinit */
#define PCERR_INITCORE  3 /*Out of core:pc_dskinit */
#define PCERR_INITDEV   4 /*Can't initialize device:pc_dskinit */
#define PCERR_INITREAD  5 /*Can't read block 0:pc_dskinit */
#define PCERR_BLOCKCLAIM 6 /*PANIC: Buffer Claim */
#define PCERR_BLOCKLOCK  7 /*Warning: freeing a locked buffer */
#define PCERR_REMINODE   8 /*Trying to remove inode with open > 1 */
#define PCERR_FATREAD    9 /* "IO Error While Failed Reading FAT" */
#define PCERR_DROBJALLOC 10 /* "Memory Failure: Out of DROBJ Structures" */
#define PCERR_FINODEALLOC 11  /* "Memory Failure: Out of FINODE Structures" */


/* File creation permissions for open */
/* Note: OCTAL */
#define PS_IWRITE 0000400   /* Write permitted  */
#define PS_IREAD  0000200   /* Read permitted. (Always true anyway)*/


/* File access flags */
#define PO_RDONLY 0x0000        /* Open for read only*/
#define PO_WRONLY 0x0001        /* Open for write only*/
#define PO_RDWR   0x0002        /* Read/write access allowed.*/
#define PO_APPEND 0x0008        /* Seek to eof on each write*/
#define PO_BUFFERED 0x0010      /* Non-Block alligned File IO is buffered */
#define PO_AFLUSH   0x0020      /* Auto-Flush. File is flushed automatically
                                   each time po_write changes the size */
#define PO_CREAT  0x0100        /* Create the file if it does not exist.*/
#define PO_TRUNC  0x0200        /* Truncate the file if it already exists*/
#define PO_EXCL   0x0400        /* Fail if creating and already exists*/
#define PO_TEXT   0x4000        /* Ignored*/
#define PO_BINARY 0x8000        /* Ignored. All file access is binary*/

#define PO_NOSHAREANY   0x0004   /* Wants this open to fail if already open.
                                      Other opens will fail while this open
                                      is active */
#define PO_NOSHAREWRITE 0x0800   /* Wants this opens to fail if already open
                                      for write. Other open for write calls
                                      will fail while this open is active. */

/* Errno values */
#define PEACCES                  1 /* deleting an in-use object or witing to read only object */
#define PEBADF                   2 /* Invalid file descriptor*/
#define PEEXIST                  3 /* Creating an object that already exists */
#define PEINVAL                  4 /* Invalid api argument */
#define PEMFILE                  5 /* Out of file descriptors */
#define PENOENT                  6 /* File or directory not found */
#define PENOSPC                  7 /* Out of space to perform the operation */
#define PESHARE                  8 /* Sharing violation in po_open */
#define PEINVALIDPARMS           9 /* Missing or invalid parameters */
#define PEINVALIDPATH            10/* Invalid path name used as an argument */
#define PEINVALIDDRIVEID         11/* Invalid drive specified in an argument */
#define PEIOERRORREAD            12/* Read error performing the API's function */
#define PEIOERRORWRITE           13/* Write error performing the API's function */
#define PECLOSED                 14/* Invalid file descriptor because a
                                      removal or media failure asynchronously
                                      closed the volume. po_close must be
                                      called for this file descriptor to
                                      clear this error. */
#define PETOOLARGE                15 /* Trying to extend a file beyond RTFS_MAX_FILE_SIZE */


#define PEDEVICECHANGED          50/* Device was removed and replaced before flush */
#define PEDEVICEFAILURE          51/* Driver reports that the device is not working */
#define PEDEVICEINIT             52/* Trying to access a drive that wasn't initialized */
#define PEDEVICENOMEDIA          53/* Driver reports that the device is empty */
#define PEDEVICEUNKNOWNMEDIA     54/* Driver reports that the device is not recognized */

#define PEINVALIDBPB             60/* No signature found in BPB (please format) */
#define PEINVALIDMBR             61/* No signature found in MBR (please write partition table) */
#define PEINVALIDMBROFFSET       62/* Partition requested but none at that offset */
#define PEIOERRORREADMBR         63/* IO error reading MBR (note: MBR is first to be read on a new insert) */
#define PEIOERRORREADBPB         64/* IO error reading BPB (block 0) */
#define PEIOERRORREADINFO32      65/* IO error reading FAT32 INFO struc (BPB extension) */

#define PEIOERRORREADBLOCK       70/* Error reading a directory block through the buffer pool */
#define PEIOERRORREADFAT         71/* Error reading a fat block through the fat buffer pool */
#define PEIOERRORWRITEBLOCK      72/* Error writing a directory block through the buffer pool */
#define PEIOERRORWRITEFAT        73/* Error writing a fat block through the fat buffer pool */
#define PEIOERRORWRITEINFO32     74/* Error writing info block during fat flush */

#define PEINVALIDBLOCKNUMBER     100/* Unexpected block number encountered. Run check disk */
#define PEINVALIDCLUSTER         101/* Unexpected cluster encountered. Run check disk */
#define PEINVALIDDIR             102/* A specified path that must be a directory was not. */
#define PEINTERNAL               103/* Unexpected condition, reset, run check disk */

#define PERESOURCE               110/* Out of directory object structures */
#define PERESOURCEBLOCK          111/* Out of directory and scratch blocks */
#define PERESOURCEFATBLOCK       112/* Out of fat blocks. (normal only in failsafe mode) */

#define PENOINIT                 120/* Subsystem not initialized */

#define PEFSCREATE               130/*Failsafe -opening journal failed */
#define PEJOURNALFULL            131/*Failsafe - no room for journal file */
#define PEIOERRORWRITEJOURNAL    132/*Failsafe - failure writing journal file */
#define PEFSRESTOREERROR         133/*Failsafe - failure restoring from journal file */
#define PEFSRESTORENEEDED        134/*Failsafe - restore required but AUTORESTORE
                                      feature is disabled */
#define PEIOERRORREADJOURNAL     135/*Failsafe - failure reading journal file */
#define PEFSREINIT               136/*Failsafe - failsafe init called but
                                     failsafe is already initialized for this
                                     device */
#define PEILLEGALERRNO           200 /* Illegal errno used by regression test */
/* Arguments to SEEK */
#define PSEEK_SET   0   /* offset from begining of file*/
#define PSEEK_CUR   1   /* offset from current file pointer*/
#define PSEEK_END   2   /* offset from end of file*/
#define PSEEK_CUR_NEG   3  /* negative offset from end of file*/

/* Arguments to po_extend_file */
#define PC_FIRST_FIT    1
#define PC_BEST_FIT     2
#define PC_WORST_FIT    3
#define PC_FIXED_FIT    4

/* Arguments to critical_error_handler() */
#define CRERR_BAD_FORMAT        1
#define CRERR_NO_CARD           2
#define CRERR_BAD_CARD          3
#define CRERR_CHANGED_CARD      4
#define CRERR_CARD_FAILURE      5

/* Return code from critical_error_handler() */
#define CRITICAL_ERROR_ABORT    1
#define CRITICAL_ERROR_RETRY    2

/* File API.C: */
BOOLEAN pc_diskflush(byte *path);
BOOLEAN pc_mkdir(byte  *name);
PCFD po_open(byte *name, word flag, word mode);
int po_read(PCFD fd,  byte *buf, int count);
int po_write(PCFD fd, byte *buf, int count);
long po_lseek(PCFD fd, long offset, int origin);
BOOLEAN po_ulseek(PCFD fd, dword offset, dword *pnew_offset, int origin);
BOOLEAN po_truncate(PCFD fd, dword offset);
BOOLEAN po_flush(PCFD fd);
int po_close(PCFD fd);
BOOLEAN pc_mv(byte *name, byte *newname);
BOOLEAN pc_unlink(byte *name);
BOOLEAN pc_rmdir(byte  *name);
BOOLEAN pc_deltree(byte  *name);
long pc_free(byte *path, dword *blocks_total, dword *blocks_free);
BOOLEAN pc_gfirst(DSTAT *statobj, byte *name);
BOOLEAN pc_gnext(DSTAT *statobj);
void pc_gdone(DSTAT *statobj);
BOOLEAN pc_set_default_drive(byte *drive);
int pc_getdfltdrvno(void);
BOOLEAN pc_set_cwd(byte *name);
BOOLEAN pc_isdir(byte *path);
BOOLEAN pc_isvol(byte *path);
BOOLEAN pc_pwd(byte *drive, byte *path);
int pc_fstat(PCFD fd, ERTFS_STAT *pstat);
int pc_stat(byte *name, ERTFS_STAT *pstat);
BOOLEAN pc_get_attributes(byte *path, byte *p_return);
BOOLEAN pc_set_attributes(byte *path, byte attributes);


/* APIRT.C */
int pc_cluster_size(byte *drive);
BOOLEAN po_extend_file(PCFD fd, dword n_bytes, dword *new_bytes, dword start_cluster, int method);
int pc_get_file_extents(PCFD fd, int infolistsize, FILESEGINFO *plist, BOOLEAN raw);
int pc_raw_read(int driveno,  byte *buf, long blockno, int nblocks, BOOLEAN raw_io);
int pc_raw_write(int driveno,  byte *buf, long blockno, int nblocks, BOOLEAN raw_io);
int pc_get_free_list(int driveno, int listsize, FREELISTINFO *plist, long threshhold);
int po_chsize(PCFD fd, dword offset);

/* File REGRESS.C: */
BOOLEAN pc_regression_test(byte *driveid, BOOLEAN do_clean);

/* APICKDSK.C */
BOOLEAN pc_check_disk(byte *drive_id, CHKDISK_STATS *pstat, int verbose, int fix_problems, int write_chains);

/* End API Definition section */

#define BLOCKEQ0 0L

typedef unsigned long  CLUSTERTYPE;

/* 10-24-2000 added LBA formatting. Submit to main tree */

/* Structure used to reserve arrays of blocks in the BSS */
typedef struct block_alloc {
        byte    core[512];
        } BLOCK_ALLOC;

#define PCDELETE (byte) 0xE5       /* MS-DOS file deleted char */
#define PCDELETEESCAPE (byte) 0xFF /* Escape character rmode
                                      passes to pc_ino2dos to really
                                      delete a directory entry */



typedef struct fatbuff {
        struct fatbuff *pnext;
        struct fatbuff *pprev;
        struct fatbuff *pnext2;     /* All hash table entries start one of these */
#define FATBLOCK_FREE               0
#define FATBLOCK_ALLOCATED          1
#define FATBLOCK_COMMITTED          2
#define FATBLOCK_UNCOMMITTED        3
        int  fat_block_state;
        dword fat_blockno;
        byte  fat_data[512];
        } FATBUFF;


/* Fat buffer context */
typedef struct fatbuffcntxt {
    dword   stat_primary_cache_hits;
    dword   stat_secondary_cache_hits;
    dword   stat_secondary_cache_loads;
    dword   stat_secondary_cache_swaps;
    struct fatbuff *puncommitted_blocks; /* uses pnext/pprev */
    struct fatbuff *pcommitted_blocks;   /* uses pnext/pprev */
    struct fatbuff *pfree_blocks;        /* uses pnext */
    struct fatbuff *pfat_buffers;        /* address of buffer pool */
    int     num_blocks;
    int     num_free;
    int     low_water;
    int     hash_size;
    dword   hash_mask;
    dword *mapped_blocks;
    byte  **mapped_data;
    FATBUFF **fat_blk_hash_tbl;         /* uses pnext2 */
} FATBUFFCNTXT;

    /* Structure to contain block 0 image from the disk */
typedef struct ddrive {
        BOOLEAN  mount_valid;       /* True if valid volume and
                                       BLOCK 0, fat buffer, block pool
                                       etc are valid */
        BOOLEAN  mount_abort;       /* True if error handler requests abort */
        int drive_opencounter;      /* Value of global opencounter when we mounted */

        dword volume_serialno;      /* Volume serial number block 0 */
        byte  volume_label[14];     /* Volume entry from block 0 */
        int   bytespcluster;        /*  */
        dword byte_into_cl_mask;    /* And this with file pointer to get the
                                       byte offset into the cluster */
        int    fasize;             /* Nibbles per fat entry. (2 or 4) */
        BLOCKT  rootblock;          /* First block of root dir */
        BLOCKT  firstclblock;       /* First block of cluster area */
        int    driveno;            /* Driveno. Set when open succeeds */
        CLUSTERTYPE maxfindex;      /* Last element in the fat - FAT32*/
        BLOCKT  fatblock;           /* First block in fat */
        int     secproot;           /* blocks in root dir */
        BOOLEAN fat_is_dirty;
        dword   bootaddr;
        byte    oemname[9];
        word    bytspsector;        /* Must be 512 for this implementation */
        byte    secpalloc;          /* Sectors per cluster */
        int     log2_secpalloc;     /* Log of sectors per cluster */
        word    secreserved;        /* Reserved sectors before the FAT */
        byte    numfats;            /* Number of FATS on the disk */
        word    numroot;            /* Maximum # of root dir entries */
        BLOCKT  numsecs;            /* Total # sectors on the disk */
        byte    mediadesc;          /* Media descriptor byte */
        CLUSTERTYPE secpfat;        /* Size of each fat */
        word    secptrk;            /* sectors per track */
        word    numhead;            /* number of heads */
        BLOCKT  numhide;            /* # hidden sectors */
        CLUSTERTYPE  free_contig_base;   /* Guess of where file data would most */
        CLUSTERTYPE  free_contig_pointer;/* Efficiently stored */
        long    known_free_clusters;/* If not -1 pc_free may use this value - FAT32 */
        word    infosec;            /* Only used for FAT32 */

/* These arent new but they are moved to this section because they play a larger */
/* role. These values are loaded by the routine pc_read_partition_table(). And */
/* used by formatting and read/write routines. */
        BLOCKT partition_base;      /* Start of the partition */
        dword  partition_size;      /* Size of the partition  */
        int    partition_type;      /* Partition type */
        /* These buffers are used for parsing arguments to API calls.
           they are large, so rather then use the stack we place them
           in the DDRIVE structure. The drive is always locked when they
           are being used */
        byte   pathname_buffer[EMAXPATH_BYTES];
        byte   filename_buffer[FILENAMESIZE_BYTES];
        int    begin_user_area;     /* Beyond this is initialized at
                                       run time and must not be written
                                       by ertfs */
/* user init is required for the following elements required */
        dword register_file_address;
        int       interrupt_number;      /* note -1 is polled for IDE */

/* Flags - These must be set by the pc_ertfs_init */
#define DRIVE_FLAGS_PARTITIONED     0x0002  /* Partitioned device  */
#define DRIVE_FLAGS_PCMCIA          0x0004  /* Pcmcia device */
#define DRIVE_FLAGS_PCMCIA_ATA      0x0008
#define DRIVE_FLAGS_FAILSAFE        0x0800  /* Automatically initialize
                                               failsafe operations for
                                               the device. */
/* Flags - These must be set by the warmstrt IOCTL call to the driver */
/* VALID is set by the device driver as a result of a successful call to
   the device ioctl call DEVCTL_WARMSTART. If the driver does not set this
   flag then it i assumed that the driver probe or init sequence failed */
#define DRIVE_FLAGS_VALID           0x0001  /* Flags have been set */
#define DRIVE_FLAGS_REMOVABLE       0x0040  /* Device is removable */
#define DRIVE_FLAGS_INSERTED        0x0080  /* Device drivers use to
                                               remember states */
#define DRIVE_FLAGS_FORMAT          0x0100  /* If set by the driver then
                                               rtfs_init must format
                                               the device before it
                                               is usable. */
#define DRIVE_FLAGS_CDFS            0x0200
#define DRIVE_FLAGS_RDONLY          0x0400  /* Device is read only */

#define DRIVE_FLAGS_FILEIO          0x0020  /* Set by RTFS when the current
                                               block transfer is file io */
                                            /* only used by the driver */
        dword drive_flags;                  /* Note: the upper byte is reserved
                                               for private use by device drivers */
        int partition_number;
        int pcmcia_slot_number;
        int pcmcia_controller_number;
        byte pcmcia_cfg_opt_value;

        int controller_number;
        int logical_unit_number;
        /* These two routines are attached to device driver specific routines */
        BOOLEAN (*dev_table_drive_io)(int driveno, dword sector, void  *buffer, word count, BOOLEAN readin);
        int (*dev_table_perform_device_ioctl)(int driveno, int opcode, void * arg);
        /* Access semaphore for the drive. This is initialized in pc_ertfs_init
           from a value in value prtfs_cfg->drive_semaphores[i] */
        dword access_semaphore;
#if (STORE_DEVICE_NAMES_IN_DRIVE_STRUCT)
        byte device_name[80];
#endif
        FATBUFFCNTXT fatcontext;    /* Controls fat cache */
         /* Controls the buffer pool by default this points to
            the shared buffcntxt value in rtfs_cfg. If a drive specific
            private buffer pool is assigned through the API this will
            point to it */
        struct blkbuffcntxt *pbuffcntxt;
        void *pfscntxt; /* Failsafe context. O if not in failsafe mode */
        void *fad;      /* Pointer to FAT driver structure caste to FAT_DRIVER * at run time*/
/* end user init required */
    } DDRIVE;

/* Fat driver functions bound to the drive structure once the type of fat is determined */
typedef CLUSTERTYPE (*fatfn_alloc_chain)(DDRIVE *pdr, CLUSTERTYPE *pstart_cluster, CLUSTERTYPE n_clusters, BOOLEAN dolink);
typedef CLUSTERTYPE (*fatfn_clnext)(DDRIVE *pdr, CLUSTERTYPE  clno);
typedef BOOLEAN (*fatfn_clrelease_dir)(DDRIVE *pdr, CLUSTERTYPE  clno);
typedef BOOLEAN (*fatfn_faxx)(DDRIVE *pdr, CLUSTERTYPE clno, CLUSTERTYPE *pvalue);
typedef BOOLEAN (*fatfn_flushfat)(int driveno);
typedef BOOLEAN (*fatfn_freechain)(DDRIVE *pdr, CLUSTERTYPE cluster, dword min_to_free, dword max_to_free);
typedef CLUSTERTYPE  (*fatfn_cl_truncate_dir)(DDRIVE *pdr, CLUSTERTYPE cluster, CLUSTERTYPE l_cluster);
typedef CLUSTERTYPE (*fatfn_get_chain)(DDRIVE *pdr, CLUSTERTYPE start_cluster, CLUSTERTYPE *pnext_cluster, CLUSTERTYPE n_clusters, int *end_of_chain);
typedef BOOLEAN (*fatfn_pfaxxterm)(DDRIVE   *pdr, CLUSTERTYPE  clno);
typedef BOOLEAN (*fatfn_pfaxx)(DDRIVE *pdr, CLUSTERTYPE  clno, CLUSTERTYPE  value);

typedef struct fat_driver {
    fatfn_alloc_chain   fatop_alloc_chain;
    fatfn_clnext        fatop_clnext;
    fatfn_clrelease_dir     fatop_clrelease_dir;
    fatfn_faxx          fatop_faxx;
    fatfn_flushfat      fatop_flushfat;
    fatfn_freechain     fatop_freechain;
    fatfn_cl_truncate_dir   fatop_cl_truncate_dir;
    fatfn_get_chain     fatop_get_chain;
    fatfn_pfaxxterm     fatop_pfaxxterm;
    fatfn_pfaxx         fatop_pfaxx;
} FAT_DRIVER;

#define FATOP(PDR) ((FAT_DRIVER *) PDR->fad)


    /* Dos Directory Entry Memory Image of Disk Entry */
#define INOPBLOCK 16                /* 16 of these fit in a block */
typedef struct dosinode {
        byte    fname[8];
        byte    fext[3];
        byte    fattribute;      /* File attributes */
        byte    resarea[8];
        word    fclusterhi;     /* This is where FAT32 stores file location */
        word  ftime;            /* time & date lastmodified */
        word  fdate;
        word  fcluster;         /* Cluster for data file */
        dword   fsize;            /* File size */
        } DOSINODE;


#if (VFAT)

/* Trag lfn segments. segblock[0] is the block that contains beginning of the
   file name. segindex is the segment in that block where the beginning is
   stored. If the lfn spans > 1 block the next block # is in segblock[2] */
typedef struct segdesc {
    int nsegs;      /* # segs in the lfn */
    int segindex;
    BLOCKT segblock[3];
    byte ncksum;    /* checksum of the associated DOSINODE */
    byte fill[3];   /* nice */
    } SEGDESC;

#define LFNRECORD SEGDESC
#endif

    /* Internal representation of DOS entry */
    /* The first 8 fields !MUST! be identical to the DOSINODE structure.
       The code will be changed later so finode contains a DOSINODE (bug) */
typedef struct finode {
        byte   fname[8];
        byte   fext[3];
        byte   fattribute;       /* File attributes */
        byte    resarea[8];
        word    fclusterhi; /* This is where FAT32 stores file location */
        word  ftime;              /* time & date lastmodified */
        word  fdate;
        word  fcluster;        /* Cluster for data file */
        dword   fsize;            /* File size */

        dword   alloced_size;      /* Size rounded up to the hearest cluster
                                       (only maintained for files */
        int   opencount;
/* If the finode is an open file the following flags control the sharing.
   they are maintained by po__open                                    */
#ifdef OF_WRITE
/* The watcom Windows include files define OF_WRITE too */
#undef OF_WRITE
#endif
#define OF_WRITE            0x01    /* File is open for write by someone */
#define OF_WRITEEXCLUSIVE   0x02    /* File is open for write by someone
                                       they wish exclusive write access */
#define OF_EXCLUSIVE        0x04    /* File is open with exclusive access not
                                       sharing write or read */
#define OF_BUFFERED         0x10    /* Non block alligned data is buffered */
        int   openflags;          /* For Files. Track how files have it open */
        struct blkbuff *pfile_buffer; /* If the file is opened in buffered
                                         mode this is the last buffered
                                         item */
        int     file_buffer_dirty;  /* If 1 file buffer needs flush */
        DDRIVE  *my_drive;
        BLOCKT  my_block;
        int   my_index;
        struct finode *pnext;
        struct finode *pprev;
        BOOLEAN is_free;        /* True if on the free list */
#if (VFAT)
        LFNRECORD s;    /* Defined as SEGDESC for VFAT */
#endif
        } FINODE;


/* contain location information for a directory */
    typedef struct dirblk {
        BLOCKT  my_frstblock;      /* First block in this directory */
        BLOCKT  my_block;          /* Current block number */
        int     my_index;          /* dirent number in my block   */
    } DIRBLK;

/* Block buffer */
typedef struct blkbuff {
        struct blkbuff *pnext;  /* Used to navigate free and populated lists */
        struct blkbuff *pprev;  /* the populated list is double linked. */
                                /* Free list is not */
        struct blkbuff *pnext2; /* Each hash table entry starts a chain of these */
#define DIRBLOCK_FREE           0
#define DIRBLOCK_ALLOCATED      1
#define DIRBLOCK_COMMITTED      2
#define DIRBLOCK_UNCOMMITTED        3
        int  block_state;
        int  use_count;
        DDRIVE *pdrive;
        dword blockno;
        byte  data[512];
        } BLKBUFF;

/* Block buffer context */
typedef struct blkbuffcntxt {
        dword   stat_cache_hits;
        dword   stat_cache_misses;
        struct blkbuff *ppopulated_blocks; /* uses pnext/pprev */
        struct blkbuff *pfree_blocks;      /* uses pnext */
        int     num_blocks;
        int     num_free;
        int     low_water;
        int     num_alloc_failures;
        int     hash_size;
        dword   hash_mask;
        struct blkbuff **blk_hash_tbl;  /* uses pnext2 */
        } BLKBUFFCNTXT;

/* Object used to find a dirent on a disk and its parent's */
typedef struct drobj {
        DDRIVE  *pdrive;
        FINODE  *finode;
        DIRBLK  blkinfo;
        BOOLEAN isroot;      /* True if this is the root */
        BOOLEAN is_free;     /* True if on the free list */
        BLKBUFF *pblkbuff;
        } DROBJ;

/* Internal file representation */
typedef struct pc_file {
    DROBJ *     pobj;           /* Info for getting at the inode */
    word        flag;           /* Acces flags from po_open(). */
    dword       fptr;           /* Current file pointer */
    CLUSTERTYPE fptr_cluster;   /* Current cluster boundary for fptr */
    dword       fptr_block;     /* Block address at boundary of fprt_cluster */
    BOOLEAN     needs_flush;    /* If TRUE this FILE must be flushed */
    BOOLEAN     is_free;        /* If TRUE this FILE may be used (see pc_memry.c) */
    BOOLEAN     at_eof;         /* True if fptr was > alloced size last time we set
                                   it. If so synch_file pointers will fix it if the
                                   file has expanded. */
    } PC_FILE;

/* INTERNAL !! */
/* Structure to contain block 0 image from the disk */
struct pcblk0 {
        byte  jump;               /* Should be E9 or EB on formatted disk */
        byte  oemname[9];
        word  bytspsector;        /* Must be 512 for this implementation */
        byte  secpalloc;          /* Sectors per cluster */
        word  secreserved;        /* Reserved sectors before the FAT */
        byte  numfats;            /* Number of FATS on the disk */
        word  numroot;            /* Maximum # of root dir entries */
        word  numsecs;            /* Total # sectors on the disk */
        byte  mediadesc;          /* Media descriptor byte */
        word  secpfat;            /* Size of each fat */
        word  secptrk;            /* sectors per track */
        word  numhead;            /* number of heads */
        word  numhide;            /* # hidden sectors High word if DOS4 */
        word  numhide2;           /* # hidden sectors Low word if DOS 4 */
        dword numsecs2;           /* # secs if numhid+numsec > 32M (4.0) */
        dword secpfat2;           /* Size of FAT in sectors (FAT32 only) */
        byte  physdrv;            /* Physical Drive No. (4.0) */
        byte  xtbootsig;          /* Extended signt 29H if 4.0 stuf valid */
        dword volid;              /* Unique number per volume (4.0) */
        byte  vollabel[11];       /* Volume label (4.0) */
        word  flags;              /* Defined below (FAT32 only) */
#define NOFATMIRROR 0x0080
#define ACTIVEFAT   0x000F
        word    fs_version;         /* Version of FAT32 used (FAT32 only) */
        dword   rootbegin;          /* Location of 1st cluster in root dir(FAT32 only) */
        word    infosec;            /* Location of information sector (FAT32 only) */
        word    backup;             /* Location of backup boot sector (FAT32 only) */
        dword   free_alloc;         /* Free clusters on drive (-1 if unknown) (FAT32 only) */
        dword   next_alloc;         /* Most recently allocated cluster (FAT32 only) */
        };

/* Partition table descriptions. */
/* One disk partition table */
typedef struct ptable_entry {
    byte  boot;
    byte  s_head;
    word  s_cyl;
    byte  p_typ;
    byte  e_head;
    word e_cyl;
    dword  r_sec;   /* Relative sector of start of part */
    dword  p_size;  /* Size of partition */
    } PTABLE_ENTRY;

typedef struct ptable {
    PTABLE_ENTRY ents[4];
    word signature; /* should be 0xaa55 */
    } PTABLE;


/* Parameter block for formatting: Used by format.c */
typedef struct fmtparms {
        byte    oemname[9];      /* Only first 8 bytes are used */
        byte   secpalloc;         /* Sectors per cluster */
        word  secreserved;      /* Reserved sectors before the FAT */
        byte   numfats;         /* Number of FATS on the disk */
        dword  secpfat;            /* Sectors per fat */
        dword   numhide;                /* Hidden sectors */
        word  numroot;          /* Maximum # of root dir entries */
        byte   mediadesc;         /* Media descriptor byte */
        word  secptrk;          /* sectors per track */
        word  numhead;          /* number of heads */
        word  numcyl;            /* number of cylinders */
        byte physical_drive_no;
        dword binary_volume_label;
        byte  text_volume_label[12];
        } FMTPARMS;



#define CS_OP_ASCII(C) C /* 8 bit unity operator, all char sets */

#if (INCLUDE_CS_JIS)
#define CS_OP_CP_CHR(TO,FR) jis_char_copy((TO),(FR))
#define CS_OP_INC_PTR(P) (P )= jis_increment((P))
#define CS_OP_CMP_CHAR(P1, P2) jis_compare((P1), (P2))
#define CS_OP_CMP_CHAR_NC(P1, P2) jis_compare_nc((P1), (P2))
#define CS_OP_ASCII_INDEX(P,C) jis_ascii_index(P,C)
#define CS_OP_DRNO_TO_LETTER(P,D) *P = ((byte) ('A' + D))
#define CS_OP_TO_LFN(TO, FROM) jis_to_unicode(TO, FROM)
#define CS_OP_LFI_TO_TXT(TO, FROM) unicode_to_jis(TO, FROM)
#define CS_OP_IS_EOS(P) (*P == 0)
#define CS_OP_IS_NOT_EOS(P) (*P)
#define CS_OP_TERM_STRING(P) *P=0
#define CS_OP_CMP_ASCII(P,C) (*(P)==C)
#define CS_OP_ASSIGN_ASCII(P,C) (*(P)=C)
#define CS_OP_ASCII_TO_CS_STR(PCSSTR,PASCSTR) rtfs_strcpy(PCSSTR,PASCSTR)
#define CS_OP_CS_TO_ASCII_STR(PASCSTR,PCSSTR) rtfs_strcpy(PASCSTR,PCSSTR)
#define CS_OP_GOTO_EOS(P) jis_goto_eos(P)
#define CS_OP_FORMAT_OUTPUT(P) P
#elif (INCLUDE_CS_ASCII)
#define CS_OP_CP_CHR(TO,FR) *(TO) = *(FR)
#define CS_OP_INC_PTR(P) (P)++
#define CS_OP_CMP_CHAR(P1, P2) (*(P1)==*(P2))
#define CS_OP_CMP_CHAR_NC(P1, P2) ascii_compare_nc((P1), (P2))
#define CS_OP_ASCII_INDEX(P,C) ascii_ascii_index(P,C)
#define CS_OP_DRNO_TO_LETTER(P,D) *P = ((byte) ('A' + D))
#define CS_OP_TO_LFN(TO, FROM) {*TO = *FROM; *(TO+1) = 0;}
#define CS_OP_LFI_TO_TXT(TO, FROM) *TO = *FROM
#define CS_OP_IS_EOS(P) (*P == 0)
#define CS_OP_IS_NOT_EOS(P) (*P)
#define CS_OP_TERM_STRING(P) *P=0
#define CS_OP_CMP_ASCII(P,C) (*(P)==C)
#define CS_OP_ASSIGN_ASCII(P,C) (*(P)=C)
#define CS_OP_ASCII_TO_CS_STR(PCSSTR,PASCSTR) rtfs_strcpy(PCSSTR,PASCSTR)
#define CS_OP_CS_TO_ASCII_STR(PASCSTR,PCSSTR) rtfs_strcpy(PASCSTR,PCSSTR)
#define CS_OP_GOTO_EOS(P) ascii_goto_eos(P)
#define CS_OP_FORMAT_OUTPUT(P) P
#elif (INCLUDE_CS_UNICODE)
#define CS_OP_CMP_CHAR(P1, P2) unicode_compare((P1), (P2))
#define CS_OP_CMP_CHAR_NC(P1, P2) unicode_compare_nc((P1), (P2))
#define CS_OP_CP_CHR(TO,FR) {*(TO)=*(FR);*((TO)+1)=*((FR)+1);}
#define CS_OP_INC_PTR(P) (P)=(P)+2
#define CS_OP_TO_LFN(TO, FROM) unicode_chr_to_lfn((TO), FROM)
#define CS_OP_LFI_TO_TXT(TO, FROM) lfn_chr_to_unicode(TO, FROM)
#define CS_OP_IS_EOS(P) (*P == 0 && *(P +1) == 0)
#define CS_OP_IS_NOT_EOS(P) (*P || *(P +1))
#define CS_OP_TERM_STRING(P) {*P=0;*(P +1)=0;}
#define CS_OP_ASCII_INDEX(P,C) unicode_ascii_index(P,C)
#define CS_OP_DRNO_TO_LETTER(P,D) unicode_drno_to_letter(P,D)
#define CS_OP_CMP_ASCII(P,C) unicode_cmp_to_ascii_char((P),C)
#define CS_OP_ASSIGN_ASCII(P,C) unicode_assign_ascii_char((P),C)
#define CS_OP_ASCII_TO_CS_STR(PCSSTR,PASCSTR) map_ascii_to_unicode(PCSSTR,PASCSTR)
#define CS_OP_CS_TO_ASCII_STR(PASCSTR,PCSSTR) map_unicode_to_ascii(PASCSTR,PCSSTR)
#define CS_OP_GOTO_EOS(P) unicode_goto_eos(P)
#define CS_OP_FORMAT_OUTPUT(P) unicode_make_printable(P)
#endif



/* Make sure memory is initted prolog for api functions */
#define CHECK_MEM(TYPE, RET)  if (!prtfs_cfg) {return((TYPE) RET);}
#define VOID_CHECK_MEM()  if (!prtfs_cfg) {return;}
#define IS_AVOLORDIR(X) ((X->isroot) || (X->finode->fattribute & AVOLUME|ADIRENT))


/* File RTFSINIT.C: */
BOOLEAN pc_ertfs_init(void);

/* File APIPRINI.C: */
BOOLEAN pro_auto_drive_init(DDRIVE *pdrive);

/* File RTFSCFG.C */
BOOLEAN pc_ertfs_config(void);

/* File API.C: */
BOOLEAN _po_ulseek(PC_FILE *pfile, dword offset, dword *new_offset, int origin);
long _po_lseek(PC_FILE *pfile, long offset, int origin);
BOOLEAN _po_flush(PC_FILE *pfile);
BOOLEAN pc_is(int op, byte *path);
dword pc_find_contig_clusters(DDRIVE *pdr, CLUSTERTYPE startpt, CLUSTERTYPE  *pchain, CLUSTERTYPE min_clusters, int method);

/* File APIUTIL.C: */
BOOLEAN pc_i_dskopen(int driveno);
int check_drive(byte *name);
PC_FILE *pc_fd2file(PCFD fd,int flags);
PCFD pc_allocfile(void);
int pc_enum_file(DDRIVE *pdrive, int chore);
void pc_free_all_fil(DDRIVE *pdrive);
BOOLEAN pc_flush_all_fil(DDRIVE *pdrive);
int pc_test_all_fil(DDRIVE *pdrive);
BOOLEAN pc_flush_file_buffer(PC_FILE *pfile);
BOOLEAN pc_load_file_buffer(PC_FILE *pfile, dword new_blockno);
BOOLEAN pc_sync_file_buffer(PC_FILE *pfile, dword start_block, dword nblocks, BOOLEAN doflush);
int pc_log_base_2(word n);
BOOLEAN pc_dskinit(int driveno);
BOOLEAN get_disk_volume(int driveno, byte *pvollabel, dword *pserialno);
BOOLEAN pc_idskclose(int driveno);
DROBJ *pc_get_cwd(DDRIVE *pdrive);
void pc_upstat(DSTAT *statobj);
BOOLEAN  _synch_file_ptrs(PC_FILE *pfile);
int pc_read_partition_table(int driveno, DDRIVE *pdr);

/* File PRBLOCK.C: */
void pc_free_all_blk(DDRIVE *pdrive);
void pc_discard_buf(BLKBUFF *pblk);
void pc_release_buf(BLKBUFF *pblk);
BOOLEAN pc_initialize_block_pool(BLKBUFFCNTXT *pbuffcntxt, int nblkbuffs,
        BLKBUFF *pmem_block_pool, int blk_hashtble_size, BLKBUFF **pblock_hash_table);
BLKBUFF *pc_find_blk(DDRIVE *pdrive, dword blockno);
BLKBUFF *pc_init_blk(DDRIVE *pdrive, BLOCKT blockno);
BLKBUFF *pc_read_blk(DDRIVE *pdrive, BLOCKT blockno);
BOOLEAN pc_write_blk(BLKBUFF *pblk);
BLKBUFF *pc_scratch_blk(void);
FATBUFF *pc_find_fat_blk(FATBUFFCNTXT *pfatbuffcntxt, dword blockno);
void pc_flush_chain_blk(DDRIVE *pdrive, CLUSTERTYPE cluster);
void pc_free_scratch_blk(BLKBUFF *pblk);

BLKBUFF *pc_alloc_file_buffer(DDRIVE *pdrive);
void pc_free_file_buffer(BLKBUFF *pblk);
BOOLEAN pc_initialize_fat_block_pool(FATBUFFCNTXT *pfatbuffcntxt,
            int fat_buffer_size, FATBUFF *pfat_buffers,
            int fat_hashtbl_size,   FATBUFF **pfat_hash_table,
            byte **pfat_primary_cache, dword *pfat_primary_index);
void    pc_free_all_fat_blocks(FATBUFFCNTXT *pfatbuffcntxt);
BOOLEAN pc_flush_fat_blocks(DDRIVE *pdrive);
byte *pc_map_fat_block(DDRIVE *pdrive, dword blockno, dword usage_flags);



/* File DEVIO.C: */
int check_drive_name_mount(byte *name);
BOOLEAN check_drive_number_mount(int driveno);
void release_drive_mount(int driveno);
BOOLEAN release_drive_mount_write(int driveno);
BOOLEAN check_drive_number_present(int driveno);
BOOLEAN devio_read(int driveno, dword blockno, byte * buf, word n_to_read, BOOLEAN raw);
BOOLEAN devio_write(int driveno, dword blockno, byte * buf, word n_to_write, BOOLEAN raw);
BOOLEAN devio_write_format(int driveno, dword blockno, byte * buf, word n_to_write, BOOLEAN raw);

/* File DROBJ.C: */
DROBJ *pc_fndnode(byte *path);
DROBJ *pc_get_inode( DROBJ *pobj, DROBJ *pmom, byte *filename, byte *fileext, int action);
BOOLEAN pc_findin( DROBJ *pobj, byte *filename, byte *fileext, int action);
DROBJ *pc_get_mom(DROBJ *pdotdot);
DROBJ *pc_mkchild( DROBJ *pmom);
DROBJ *pc_mknode(DROBJ *pmom ,byte *filename, byte *fileext, byte attributes, CLUSTERTYPE incluster);
BOOLEAN pc_insert_inode(DROBJ *pobj , DROBJ *pmom, byte attr, CLUSTERTYPE cluster, byte *filename, byte *fileext);
BOOLEAN pc_rmnode( DROBJ *pobj);
BOOLEAN pc_update_inode(DROBJ *pobj, BOOLEAN set_archive, BOOLEAN set_date);
DROBJ *pc_get_root( DDRIVE *pdrive);
BLOCKT pc_firstblock( DROBJ *pobj);
BOOLEAN pc_next_block( DROBJ *pobj);
BLOCKT pc_l_next_block(DDRIVE *pdrive, BLOCKT curblock);
void pc_marki( FINODE *pfi, DDRIVE *pdrive, BLOCKT sectorno, int  index);
FINODE *pc_scani( DDRIVE *pdrive, BLOCKT sectorno, int index);
DROBJ *pc_allocobj(void);
FINODE *pc_alloci(void);
void pc_free_all_drobj( DDRIVE *pdrive);
void pc_free_all_i( DDRIVE *pdrive);
void pc_freei( FINODE *pfi);
void pc_freeobj( DROBJ *pobj);
void pc_dos2inode (FINODE *pdir, DOSINODE *pbuff);
void pc_init_inode(FINODE *pdir, KS_CONSTANT byte *filename,
            KS_CONSTANT byte *fileext, byte attr,
            CLUSTERTYPE cluster, dword size, DATESTR *crdate);
void pc_ino2dos (DOSINODE *pbuff, FINODE *pdir);
BOOLEAN pc_isavol( DROBJ *pobj);
BOOLEAN pc_isadir( DROBJ *pobj);
BOOLEAN pc_isroot( DROBJ *pobj);


/* File FORMAT.C: */
/* See end of file for prototypes */

/* FATXX.C  */
BOOLEAN init_fat32(DDRIVE *pdr);
BOOLEAN init_fat16(DDRIVE *pdr);
BOOLEAN init_fat12(DDRIVE *pdr);

/* FAT32.C  */
BOOLEAN pc_init_drv_fat_info(DDRIVE *pdr, struct pcblk0 *pbl0);
/* These are defined twice. once for FAT32 systems once for non FAT32 */
CLUSTERTYPE pc_get_parent_cluster(DDRIVE *pdrive, DROBJ *pobj);
CLUSTERTYPE pc_grow_dir(DDRIVE *pdrive, DROBJ *pobj);
CLUSTERTYPE pc_alloc_dir(DDRIVE *pdrive, DROBJ *pobj);
void pc_truncate_dir(DDRIVE *pdrive, DROBJ *pobj, CLUSTERTYPE cluster);
BOOLEAN pc_mkfs32(int driveno, FMTPARMS *pfmt, BOOLEAN use_raw);
CLUSTERTYPE pc_finode_cluster(DDRIVE *pdr, FINODE *finode);
void pc_pfinode_cluster(DDRIVE *pdr, FINODE *finode, CLUSTERTYPE value);
BOOLEAN pc_gblk0_32(word driveno, struct pcblk0 *pbl0, byte *b);
BOOLEAN pc_validate_partition_type(byte p_type);
BOOLEAN fat_flushinfo(DDRIVE *pdr);


/* FAT16.C  */
BOOLEAN pc_init_drv_fat_info16(DDRIVE *pdr, struct pcblk0 *pbl0);
BOOLEAN pc_mkfs16(int driveno, FMTPARMS *pfmt, BOOLEAN use_raw);

/* File LOWL.C: */
BOOLEAN pc_gblk0(word driveno, struct pcblk0 *pbl0);
BOOLEAN pc_clzero(DDRIVE *pdrive, CLUSTERTYPE cluster);
DDRIVE  *pc_drno2dr(int driveno);
DDRIVE  *pc_drno_to_drive_struct(int driveno);
BOOLEAN pc_dskfree(int driveno);
CLUSTERTYPE pc_sec2cluster(DDRIVE *pdrive, BLOCKT blockno);
word pc_sec2index(DDRIVE *pdrive, BLOCKT blockno);
BLOCKT pc_cl2sector(DDRIVE *pdrive, CLUSTERTYPE cluster);
CLUSTERTYPE pc_finode_cluster(DDRIVE *pdr, FINODE *finode);
void pc_pfinode_cluster(DDRIVE *pdr, FINODE *finode, CLUSTERTYPE value);
dword pc_chain_length(DDRIVE *pdrive, dword byte_length);

/* File PC_MEMRY.C: */
int pc_num_drives(void);
int pc_num_users(void);
int pc_nuserfiles(void);
BOOLEAN pc_validate_driveno(int driveno);
BOOLEAN pc_memory_init(void);
void pc_memory_close(void);
DROBJ *pc_memory_drobj(DROBJ *pobj);
FINODE *pc_memory_finode(FINODE *pinode);

/* File UTILJIS.C: */
byte *jis_goto_eos(byte *p);
int jis_char_length(byte *p);
int jis_char_copy(byte *to, byte *from);
byte *jis_increment(byte *p);
int rtfs_jis_strlen(byte * string);
int jis_compare(byte *p1, byte *p2);
int jis_compare_nc(byte *p1, byte *p2);
int jis_ascii_index(byte *p, byte c);
BOOLEAN valid_jis_char(byte *p);
byte *map_jis_input(byte *pin);
byte *map_jis_output(byte *pin);
BOOLEAN _illegal_jis_alias_char(byte *p);
int rtfs_jis_strlen(byte * string);
int jis_char_copy(byte *to, byte *from);
BOOLEAN _illegal_jis_lfn_char(byte *p);

/* File UTILUNI.C: */
byte *unicode_goto_eos(byte *p);
void unicode_drno_to_letter(byte *p,int driveno);
int unicode_compare(byte *p1, byte *p2);
int unicode_compare_nc(byte *p1, byte *p2);

BOOLEAN pc_unicode_patcmp(byte *bpat, byte *bname, BOOLEAN dowildcard);
BOOLEAN pc_valid_lfn(byte *afilename);
void lfn_chr_to_unicode(byte *to, byte *fr);
void unicode_chr_to_lfn(byte *to, byte *fr);
int rtfs_unicode_strcpy(byte * targ, byte * src);
int rtfs_unicode_strcmp(byte * s1, byte * s2);
int rtfs_unicode_strlen(byte * string);
int unicode_ascii_index(byte *p, byte c);
void map_ascii_to_unicode(byte *unicode_to, byte *ascii_from);
void map_unicode_to_ascii(byte *to, byte *from);
BOOLEAN pc_parsepath(byte *atopath, byte *afilename, byte *afileext, byte *apath);
void pc_ascii_strn2upper(byte *to, byte *from, int n);
void pc_byte2upper(byte *to, byte *from);
void pc_ascii_str2upper(byte *to, byte *from);
BOOLEAN pc_cs_malias(byte *alias, byte *input_file, int try);
int unicode_cmp_to_ascii_char(byte *p, byte c);
void unicode_assign_ascii_char(byte *p, byte c);
byte *unicode_make_printable(byte *p);
byte *unicode_convert_strtable(byte *p);

/* File UTILASCI.C */
byte *ascii_goto_eos(byte *p);
void pc_ascii_strn2upper(byte *to, byte *from, int n);
void pc_byte2upper(byte *to, byte *from);
void pc_ascii_str2upper(byte *to, byte *from);
void pc_str2upper(byte *to, byte *from);
BOOLEAN validate_filename(byte * name, byte * ext);
int ascii_compare_nc(byte *p1, byte *p2);
int ascii_ascii_index(byte *p,byte c);
static BOOLEAN valid_alias_char(byte c);
BOOLEAN validate_8_3_name(byte * name,int len);
BOOLEAN validate_filename(byte * name, byte * ext);
BOOLEAN pc_valid_sfn(byte *filename);
BOOLEAN pc_valid_lfn(byte *filename);
BOOLEAN pc_cs_malias(byte *alias, byte *input_file, int try);
BOOLEAN pc_ascii_fileparse(byte *fname, byte *fext, byte *alias);


/* These routines are duplicated in UTILJIS.C UTILASCI.C UTILUNI.C */
BOOLEAN pc_patcmp_8(byte *pat, byte *name, BOOLEAN dowildcard);
BOOLEAN pc_patcmp_3(byte *pat, byte *name, BOOLEAN dowildcard);
void pc_ascii_strn2upper(byte *to, byte *from, int n);
void pc_byte2upper(byte *to, byte *from);
void pc_ascii_str2upper(byte *to, byte *from);
void pc_str2upper(byte *to, byte *from);
BOOLEAN validate_filename(byte * name, byte * fext);
BOOLEAN validate_8_3_name(byte * name,int len);
BOOLEAN pc_valid_sfn(byte *filename);
BOOLEAN pc_cs_malias(byte *alias, byte *input_file, int try);
byte *pc_cs_mfile(byte *to, byte *filename, byte *ext);
byte *pc_ascii_mfile(byte *to, byte *filename, byte *ext);
int rtfs_cs_strcmp(byte * s1, byte * s2);
int rtfs_cs_strcpy(byte * targ, byte * src);
int rtfs_cs_strlen(byte * string);
int rtfs_cs_strlen_bytes(byte * string);
byte *rtfs_cs_strcat(byte *to, byte *from);

/* File JISTAB.C */
int unicode_to_jis(byte *pjis, byte *punicode);
void jis_to_unicode(byte *to, byte *from);

/* File UTILBYTE.C: */
byte *pc_strchr(byte *string, byte ch);
BOOLEAN _illegal_alias_char(byte ch);
BOOLEAN _illegal_lfn_char(byte ch);
BOOLEAN pc_isdot(byte *fname, byte *fext);
BOOLEAN pc_isdotdot(byte *fname, byte *fext);
BOOLEAN name_is_reserved(byte *filename);
byte pc_cksum(byte *test);
int rtfs_strcpy(byte * targ, byte * src);
int rtfs_strcmp(byte * s1, byte * s2);
int rtfs_strlen(byte * string);
void copybuff(void *vto, void *vfrom, int size);
void pc_cppad(byte *to, byte *from, int size);
void rtfs_memset(void *pv, byte b, int n);
byte * rtfs_strcat(byte * targ, byte * src);

/* File UTIL.C: */
byte *pc_mpath(byte *to, byte *path, byte *filename);
int pc_path_to_driveno(byte  *path);
int pc_parse_raw_drive(byte  *path);
byte *pc_parsedrive(int *driveno, byte  *path);
byte *pc_nibbleparse(byte *filename, byte *fileext, byte *path);
BOOLEAN pc_parsepath(byte *topath, byte *filename, byte *fileext, byte *path);
BOOLEAN pc_patcmp_vfat(byte *pat, byte *name, BOOLEAN dowildcard);
BOOLEAN pc_malias(byte *fname, byte *fext, byte *input_file, DROBJ *dest);
void RTFS_ARGSUSED_PVOID(void * p);
void RTFS_ARGSUSED_INT(int i);
dword to_DWORD ( byte *from);
word to_WORD ( byte *from);
void fr_WORD ( byte *to,  word from);
void fr_DWORD ( byte *to,  dword from);


/* Extra arguments to pc_get_inode() and pc_findin() */
#define GET_INODE_MATCH  0 /* Must match the pattern exactly */
#define GET_INODE_WILD   1 /* Pattern may contain wild cards */
#define GET_INODE_STAR   2 /* Like he passed *.* (pattern will be null) */
#define GET_INODE_DOTDOT 3 /* Like he past .. (pattern will be null */


/* User structure management */
typedef struct rtfs_system_user
{
    dword         task_handle;     /* Task this is for */
    int           rtfs_errno;       /* current errno value for the task */
    dword         rtfs_driver_errno;/* device driver errno value for the task */
    int           dfltdrv;          /* Default drive to use if no drive specified  */
    int           dfltdrv_set;      /* If 1 Default drive was set,
                                       otherwise use the default in rtfs_cfg */
    void *        lcwd[26];         /* current working enough for 26 drives  */

} RTFS_SYSTEM_USER;

typedef struct rtfs_system_user  *PRTFS_SYSTEM_USER;

/* RTFSKERN.C */
void pc_report_error(int error_number);
PRTFS_SYSTEM_USER rtfs_get_system_user(void);
BOOLEAN rtfs_resource_init(void);
void  pc_free_user(void);
void  pc_free_all_users(int driveno);
int rtfs_set_errno(int error);
int get_errno(void);
void rtfs_set_driver_errno(dword error);
dword rtfs_get_driver_errno(void);
int  critical_error_handler(int driveno, int media_status, dword sector);

/* These opcodes must be supported by the device driver's
   perform_device_ioctl() routine.
*/

#define DEVCTL_CHECKSTATUS 0

/*      DEVCTL_WARMSTART - Called when RTIP initializes.
            When ERTFS initializes it calls xxx_device_ioctl() with this
            argument in ascending order for each possible logical device.
            The device driver may use this call to perform spin up of the
            device or to clear driver specific information. In most cases
            this routine can do nothing and simply return zero.
            The device driver must return zero. There is no error condition
*/

#define DEVCTL_WARMSTART        1

/* This is called by RTFS when it notices that power has been restored. IE
   if current power count is not the same as it was the last time that the
   device was accessed. */

#define DEVCTL_POWER_RESTORE    2

/* Called when RTFS when power is going down.
!!! RTFS does not make this call. It is provided as a mechanism
    that that system power management system may use to communicate a power
    down event to the device driver */
#define DEVCTL_POWER_LOSS       3


/*  Perform a low level format on the media. (If required). Return 0 on
    success or return a driver specific format error code. ERTFS does not
    interpret the error code but it will abort a file system format operation
    if anything but zero is returned. */

#define DEVCTL_FORMAT           4


/*
        DEVCTL_GET_GEOMETRY - Report back to ertfs the geometry of the
            device in the structure pointed to by the arg pointer.
            The structure pointed to at parg is of type PDEV_GEOMETRY:

        Note: The geometry must be in DOS HCN format. If the device is a
        logical block oriented device (where blocks are a contiguous
        array of blocks) then the device must convert lba units to HCN units.

        The device should return zero if it can report the geometry.
        Any other return code will cause ERTFS to abort a file
        system format operation.

*/

#define DEVCTL_GET_GEOMETRY     5


typedef struct dev_geometry {
    int dev_geometry_heads;      /*- Must be < 256 */
    dword dev_geometry_cylinders;  /*- Must be < 1024 */
    int dev_geometry_secptrack;  /*- Must be < 64 */
    dword dev_geometry_lbas;     /*- For oversize media */
    BOOLEAN fmt_parms_valid;     /*If the device io control call sets this */
                                 /*TRUE then it it telling the applications */
                                 /*layer that these format parameters should */
                                 /*be used. This is a way to format floppy */
                                 /*disks exactly as they are fromatted by dos. */
    FMTPARMS fmt;
} DEV_GEOMETRY;

#define DEVCTL_REPORT_REMOVE    6 /* this can be called by an external  */
                                  /* interrupt to tell the driver that */
                                  /* the device has been removed. For  */
                                  /* pcmcia management interrupt calls  */
                                  /* this and the pcmcia aware drivers */
                                  /* become aware tht they have to re-mount */


/*---------- ctr modified ----------*/
/*_po_flushから呼ばれる。RTFSがflush時にdeviceのIO関数を呼んだ後に
  これが呼ばれる。メディアにフラッシュせよとの指示。
  _po_flush は apiwrite.c を参照のこと。*/
#define	DEVCTL_FLUSH			7	
/*----------------------------------*/


typedef struct dev_geometry  *PDEV_GEOMETRY;


/* These codes are to be returned from the device driver's
   perform_device_ioctl() function when presented with
   DEVCTL_CHECKSTATUS as an opcode.
*/

#define DEVTEST_NOCHANGE        0 /* Status is 'UP' */
#define DEVTEST_NOMEDIA         1 /*  The device is empty */
#define DEVTEST_UNKMEDIA        2 /*  Contains unknown media */
#define DEVTEST_CHANGED         3 /*  Controller recognized and cleared a */
                                  /*  change condition */
#define BIN_VOL_LABEL 0x12345678L

#define READ_PARTION_OK   0 /* Partition read succesfully  */
#define READ_PARTION_ERR  -1 /* Internal error (couldn't allocate buffers ?) */
#define READ_PARTION_NO_TABLE   -2 /* No partition table found */
#define READ_PARTION_NO_ENTRY   -3 /* Request entry not found */
#define READ_PARTION_IOERROR    -4 /* Device IO error  */

/* File FORMAT.C: */
BOOLEAN pc_get_media_parms(byte *path, PDEV_GEOMETRY pgeometry);
BOOLEAN pc_format_media(byte *path, PDEV_GEOMETRY pgeometry);
BOOLEAN pc_partition_media(byte *path, PDEV_GEOMETRY pgeometry, dword * partition_list);
BOOLEAN pc_format_volume(byte *path, PDEV_GEOMETRY pgeometry);

/* File VFAT.C */
BOOLEAN pc_delete_lfn_info(DROBJ *pobj);
void pc_zero_lfn_info(FINODE *pdir);
BOOLEAN pc_get_lfn_filename(DROBJ *pobj, byte *path);

/*        Terminal input/output macros.
*           RTFS_PRINT_LONG_1
*           RTFS_PRINT_STRING_1
*           RTFS_PRINT_STRING_2
*
*  These macros by default are defined as calls to functions in rtfsterm.c as seen here
*
*       #define RTFS_PRINT_LONG_1(L1,FLAGS) rtfs_print_long_1(L1,FLAGS)
*       #define RTFS_PRINT_STRING_1(STR1ID,FLAGS) rtfs_print_string_1(STR1ID,FLAGS)
*       #define RTFS_PRINT_STRING_2(STR1ID,STR2,FLAGS) rtfs_print_string_2(STR1ID,STR2,FLAGS)
*
*  If no console output available thy may be dfined as seen here
*
*       #define RTFS_PRINT_LONG_1(L1,FLAGS)
*       #define RTFS_PRINT_STRING_1(STR1ID,FLAGS)
*       #define RTFS_PRINT_STRING_2(STR1ID,STR2,FLAGS)
*
*/
/* String printing control characters used throughout the library */
#define PRFLG_NL        0x0001  /* Newline Carriage return at end */
#define PRFLG_CR        0x0002  /* Carriage Return only at end    */

/* Prototype of console input function that is implemented in rtfsterm.c */
void rtfs_print_prompt_user(int prompt_id, byte *buf);

/* Prototypes of print functions that are implemented in rtfsterm.c
   these routines are never called directly but are called through macros */
void rtfs_print_long_1(dword l1,int flags);
void rtfs_print_string_1(int str1_id,int flags);
void rtfs_print_string_2(int str1_id,byte *pstr2, int flags);

/* Macros that are used to access the print routines */
#define RTFS_PRINT_LONG_1(L1,FLAGS) rtfs_print_long_1(L1,FLAGS)
#define RTFS_PRINT_STRING_1(STR1ID,FLAGS) rtfs_print_string_1(STR1ID,FLAGS)
#define RTFS_PRINT_STRING_2(STR1ID,STR2,FLAGS) rtfs_print_string_2(STR1ID,STR2,FLAGS)

BOOLEAN pc_failsafe_hold(DDRIVE *pdrive);

/* These STUBBED versions may be used instead if output is not available */

/*
#define RTFS_PRINT_LONG_1(L1,FLAGS)
#define RTFS_PRINT_STRING_1(STR1ID,FLAGS)
#define RTFS_PRINT_STRING_2(STR1ID,STR2,FLAGS)
*/

/* PORTRTFS.C */
dword rtfs_port_alloc_mutex(void);
void rtfs_port_claim_mutex(dword handle);
void rtfs_port_release_mutex(dword handle);
dword rtfs_port_alloc_signal(void);
void rtfs_port_clear_signal(dword handle);
int rtfs_port_test_signal(dword handle, int timeout);
void rtfs_port_set_signal(dword handle) ;
void rtfs_port_sleep(int sleeptime) ;
dword rtfs_port_elapsed_zero(void) ;
int rtfs_port_elapsed_check(dword zero_val, int timeout);
dword rtfs_port_get_taskid(void) ;
void rtfs_port_tm_gets(byte *buffer);
void rtfs_port_puts(byte *buffer);
void rtfs_port_disable(void);
void rtfs_port_enable(void);
void rtfs_port_exit(void);
DATESTR *pc_getsysdate(DATESTR * pd);
void hook_82365_pcmcia_interrupt(int irq);
void phys82365_to_virtual(byte ** virt, dword phys);
void write_82365_index_register(byte value) ;
void write_82365_data_register(byte value);
byte read_82365_data_register(void);
void hook_ide_interrupt(int irq, int controller_number);
byte ide_rd_status(dword register_file_address);
word ide_rd_data(dword register_file_address);
byte ide_rd_sector_count(dword register_file_address);
byte ide_rd_alt_status(dword register_file_address,int contiguous_io_mode);
byte ide_rd_error(dword register_file_address);
byte ide_rd_sector_number(dword register_file_address);
byte ide_rd_cyl_low(dword register_file_address);
byte ide_rd_cyl_high(dword register_file_address);
byte ide_rd_drive_head(dword register_file_address);
byte ide_rd_drive_address(dword register_file_address, int contiguous_io_mode);
void ide_wr_dig_out(dword register_file_address, int contiguous_io_mode, byte value);
void ide_wr_data(dword register_file_address, word value);
void ide_wr_sector_count(dword register_file_address, byte value);
void ide_wr_sector_number(dword register_file_address, byte value);
void ide_wr_cyl_low(dword register_file_address, byte value);
void ide_wr_cyl_high(dword register_file_address, byte value);
void ide_wr_drive_head(dword register_file_address, byte value);
void ide_wr_command(dword register_file_address, byte value);
void ide_wr_feature(dword register_file_address, byte value);
byte ide_rd_udma_status(dword bus_master_address);
void ide_wr_udma_status(dword bus_master_address, byte value);
byte ide_rd_udma_command(dword bus_master_address);
void ide_wr_udma_command(dword bus_master_address, byte value);
void ide_wr_udma_address(dword bus_master_address, dword bus_address);
void ide_insw(dword register_file_address, unsigned short *p, int nwords);
void ide_outsw(dword register_file_address, unsigned short *p, int nwords);
dword rtfs_port_ide_bus_master_address(int controller_number);
unsigned long rtfs_port_bus_address(void * p);

BOOLEAN pcmctrl_init(void);
BOOLEAN pcmcia_card_is_ata(int socket, dword register_file_address,int interrupt_number, byte pcmcia_cfg_opt_value);
BOOLEAN pcmctrl_card_installed(int socket);
/* Drive access semaphores are store in the drive structure. These macros
   map logical drive number to drive structure and then access the semaphores */
#define OS_CLAIM_LOGDRIVE(X)    rtfs_port_claim_mutex((prtfs_cfg->drno_to_dr_map[X])->access_semaphore);
#define OS_RELEASE_LOGDRIVE(X)  rtfs_port_release_mutex((prtfs_cfg->drno_to_dr_map[X])->access_semaphore);
/* System wide critical region semaphore */
#define OS_CLAIM_FSCRITICAL()   rtfs_port_claim_mutex(prtfs_cfg->critical_semaphore);
#define OS_RELEASE_FSCRITICAL() rtfs_port_release_mutex(prtfs_cfg->critical_semaphore);


/* Configuration structure. Must be filled in by the user.
   see rtfscfg.c */
typedef struct rtfs_cfg {
    /* Configuration values */
    int cfg_NDRIVES;                    /* The number of drives to support */
    int cfg_NBLKBUFFS;                  /* The number of block buffers */
    int cfg_BLK_HASHTBLE_SIZE;          /* The number entries in the hash table */
    int cfg_NUSERFILES;                 /* The number of user files */
    int cfg_NDROBJS;                    /* The number of directory objects */
    int cfg_NFINODES;                   /* The number of directory inodes */
    int cfg_NUM_USERS;                  /* The number of users to support */
    int cfg_FAT_BUFFER_SIZE[26];        /* Fat buffer size in blocks,  per logical drive */
    int cfg_FAT_HASHTBL_SIZE[26];       /* Fat hash table entries, per logical drive */

    /* Core that must be provided by the user */
    DDRIVE   *mem_drives_structures;    /* Point at cfg_NDRIVES * sizeof(DDRIVE) bytes*/
    BLKBUFF  *mem_block_pool;           /* Point at cfg_NBLKBUFFS * sizeof(BLKBUFF) bytes*/
    BLKBUFF  **mem_block_hash_table;    /* Point  cfg_BLK_HASHTBL_SIZE dwords */
    PC_FILE  *mem_file_pool;            /* Point at cfg_USERFILES * sizeof(PC_FILE) bytes*/
    DROBJ    *mem_drobj_pool;           /* Point at cfg_DROBJS * sizeof(DROBJ) bytes*/
    FINODE   *mem_finode_pool;      /* Point at cfg_NFINODE * sizeof(FINODE) bytes*/
    FATBUFF  *fat_buffers[26];      /* Point fat_buffers[N] at cfg_FAT_BUFFER_SIZE[N] * sizeof(FATBUFF) bytes*/
    FATBUFF  **fat_hash_table[26];  /* Point fat_hash_table[N] at cfg_FAT_HASHTBL_SIZE[N] * sizeof(FATBUFF *) */
    byte     **fat_primary_cache[26];   /* Point fat_primary_cache[[N] at cfg_FAT_HASHTBL_SIZE[N] * sizeof(byte *) */
    dword     *fat_primary_index[26];   /* Point fat_primary_index[[N] at cfg_FAT_HASHTBL_SIZE[N] * sizeof(dword) */
    RTFS_SYSTEM_USER *rtfs_user_table;      /* Point at cfg_NUM_USERS * sizeof(RTFS_SYSTEM_USER) bytes*/
    /* These pointers are internal no user setup is needed */
    BLKBUFFCNTXT buffcntxt;             /* Systemwide shared buffer pool */
    FINODE   *inoroot;          /* Begining of inode pool */
    FINODE   *mem_finode_freelist;
    DROBJ    *mem_drobj_freelist;
    DDRIVE   *drno_to_dr_map[26];

    dword  ide_semaphore;       /* Allocated and used by the IDE driver */
    dword  floppy_semaphore;    /* Allocated and used by the FLOPPY driver */
    dword  floppy_signal;       /* Allocated and used by the FLOPPY driver */
    dword  critical_semaphore;  /* Used by ERTFS for critical sections */
/* Note: cfg_NDRIVES semaphores are allocated and assigned to the individual
   drive structure within routine pc_ertfs_init() */
    /* This value is set in pc_rtfs_init(). It is the drive number of the
       lowest (0-25) == A: - Z:. valid drive identifier in the system.
       If the user does not set a default drive, this value will be used. */
    int    default_drive_id;
    /* BSS area used by the block buffer pool system */
    dword  useindex;
    DDRIVE *scratch_pdrive;
    /* Counter used to uniquely identify a drive mount. Each time an open
       succeeds this value is incremented and stored in the drive structure.
       it is used by gfirst gnext et al to ensure that the drive was not
       closed and remounted between calls */
    int drive_opencounter;
} RTFS_CFG;

extern RTFS_CFG *prtfs_cfg;

#if (INCLUDE_FAILSAFE_CODE)
#include "prfs.h"
#endif      /* INCLUDE_FAILSAFE_CODE */
/* Include RTFS Pro features */
#include "rtfspro.h" //twl modified


#include "attach.h"			//ctr modified

BOOLEAN rtfs_init( void);	//ctr modified



#endif      /* __RTFS__ */
