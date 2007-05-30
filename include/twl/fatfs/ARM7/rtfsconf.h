/*****************************************************************************
*Filename: RTFSCONF.H - RTFS tuning constants
*
*
* EBS - RTFS (Real Time File Manager)
*
* Copyright Peter Van Oudenaren, 1993
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
* Description:
*   This file contains tuning constants for configuring RTFS.
*   It is included by rtfs.h
*
****************************************************************************/

#ifndef __RTFSCONF__
#define __RTFSCONF__ 1

/* Include CPU and peripheral configuration */
#include <portconf.h>

/* Character set support */
#define INCLUDE_CS_JIS       0  /* Set to 1 to support JIS (kanji)  */
#define INCLUDE_CS_ASCII     1  /* Set to 1 to support ASCII only  */
#define INCLUDE_CS_UNICODE   0  /* Set to 1 to support unicode characters requires VFAT */

/* Note: After we implemented VFAT we learned that Microsoft patented
   the Win95 VFS implementation. US PATENT # 5,758,352.
   Leaving VFAT set to zero will exclude potential patent infringment
   problems.
   3-19-99
*/

/* Set to 1 to support long filenames */
#define VFAT              1
/* Set to 1 to support 32 bit FATs */
#define FAT32             1
/* Set to 0 to disable file share modes saves ~0.5 K */
#define RTFS_SHARE        0
/* Set to 0 to disable subdirs. Feature not implemented must be 1*/
#define RTFS_SUBDIRS      1
/* Set to 0 to disable write support. Feature not implemented must be 1*/
#define RTFS_WRITE        1
/* Set to 1 to include failsafe support */
#define INCLUDE_FAILSAFE_CODE 1

/* Set to 1 to include support for extended DOS partitions */
/* ERTFS contains code to interpret extended DOS partitions but since this
   feature is rarely used it is provided as a compile time option */
#define SUPPORT_EXTENDED_PARTITIONS 0

/* STORE_DEVICE_NAMES_IN_DRIVE_STRUCT - If this value is set to one then
   we save device names for future viewing by diagnostics */
#define STORE_DEVICE_NAMES_IN_DRIVE_STRUCT 1

/* Set to the maximum file size ERTFS may create. If po_chsize or po_extend_file()
   are called with a size request larger than this they fail and set errno
   to PETOOLARGE. When po_write() is asked to expend the file beyond this maximum
   the behavior is determined by the value of RTFS_TRUNCATE_WRITE_TO_MAX */
#define RTFS_MAX_FILE_SIZE      0xffffffff
/* #define RTFS_MAX_FILE_SIZE      0x80000000 */
/* Set to 1 to force RTFS to truncate po_write() requests to fit within
   RTFS_MAX_FILE_SIZE. If RTFS_TRUNCATE_WRITE_TO_MAX is set to 0, po_write
   requests that attempt to extend the file beyond RTFS_TRUNCATE_WRITE_TO_MAX
   Fail and set errno to PETOOLARGE. If RTFS_TRUNCATE_WRITE_TO_MAX is set to
   1, po_write requests that attempt to extend the file beyond
   RTFS_MAX_FILE_SIZE are truncated to fill the file until its
   size reaches RTFS_MAX_FILE_SIZE bytes. */
#define RTFS_TRUNCATE_WRITE_TO_MAX    1



#if (VFAT)
#define FILENAMESIZE_CHARS    255
#else
#if (INCLUDE_CS_UNICODE)
#error - Unicode requires VFAT
#endif
#define FILENAMESIZE_CHARS      8
#endif


#if (VFAT)
#define EMAXPATH_CHARS        260  /* Maximum path length. Change if you like */
#else
#define EMAXPATH_CHARS        148  /* Maximum path length. Change if you like */
#endif

/* Declare buffer sizes, leave room for terminating NULLs, allign to
   four bytes for good form. */
#if (VFAT)
#if (INCLUDE_CS_UNICODE || INCLUDE_CS_JIS)
#define EMAXPATH_BYTES 524
#define FILENAMESIZE_BYTES 512
#else
#define EMAXPATH_BYTES 264
#define FILENAMESIZE_BYTES 256
#endif
#else /* Not VFAT */
#if (INCLUDE_CS_UNICODE || INCLUDE_CS_JIS)
#define EMAXPATH_BYTES 300
#define FILENAMESIZE_BYTES 20
#else
#define EMAXPATH_BYTES 152
#define FILENAMESIZE_BYTES 12
#endif
#endif

/* When scanning a directory cluster chain fail if more than this many
   clusters are in the chain. (Indicates endless loop)
*/
#define MAX_CLUSTERS_PER_DIR 4096


/* Make sure a character set is enabled */
#if (INCLUDE_CS_JIS)
#if (INCLUDE_CS_UNICODE||INCLUDE_CS_ASCII)
#error Only one character set may be selected
#endif
#elif (INCLUDE_CS_UNICODE)
#if (INCLUDE_CS_JIS||INCLUDE_CS_ASCII)
#error Only one character set may be selected
#endif
#elif (INCLUDE_CS_ASCII)
#if (INCLUDE_CS_UNICODE||INCLUDE_CS_JIS)
#error Only one character set may be selected
#endif
#else
#error At least one character set must be selected
#endif


/*--- ctr modified ---*/
#define	RTFS_DEBUG_PRINT_ON		(0)
/*--- ctr modified end ---*/


/********************************************************************
 TYPES
********************************************************************/

#define TRUE  1                 /* Don't change */
#define FALSE 0                 /* Don't change */

typedef unsigned char byte;     /* Don't change */
typedef unsigned short word;    /* Don't change */
typedef unsigned long dword;    /* Don't change */
/* typedef int  BOOLEAN;            Don't change */
#define BOOLEAN int

#endif /* __RTFSCONF__ */
