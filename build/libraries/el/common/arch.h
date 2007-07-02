/*---------------------------------------------------------------------------*
  Project:  CTR - ELF Loader
  File:     arch.h

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef ARCH_H_
#define ARCH_H_



/*---------------------------------------------------------
 Archive Header
 --------------------------------------------------------*/
#define  ARMAG   "!<arch>\n"    /* magic string */
#define  SARMAG  8              /* length of magic string */

#define  ARFMAG       "\`\n"    /* header trailer string */
#define  AR_NAME_LEN  16        /* ar_name size, includes `/' */


typedef struct    /* archive file member header - printable ascii */
{
    char    ar_name[16];    /* file member name - `/' terminated */
    char    ar_date[12];    /* file member date - decimal */
    char    ar_uid[6];      /* file member user id - decimal */
    char    ar_gid[6];      /* file member group id - decimal */
    char    ar_mode[8];     /* file member mode - octal */
    char    ar_size[10];    /* file member size - decimal */
    char    ar_fmag[2];     /* ARFMAG - string to end header */
}ArchHdr;                   /* 計60(0x3C)バイト */






/*---------------------------------------------------------
 エントリヘッダからエントリのサイズを求める
 --------------------------------------------------------*/
u32 AR_GetEntrySize( ArchHdr* ArHdr);


#endif /*ARCH_H_*/
