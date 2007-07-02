/*---------------------------------------------------------------------------*
  Project:  CTR - ELF Loader
  File:     arch.c

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include "el_config.h"

#if (TARGET_OS_NITRO == 1)
 #include <twl.h>
#else
 #include <ctr.h>
#endif

#include "arch.h"


/*---------------------------------------------------------
 エントリヘッダからエントリのサイズを求める
 --------------------------------------------------------*/
u32 AR_GetEntrySize( ArchHdr* ArHdr)
{
    u16 i;
    u32 digit = 1;
    u32 size = 0;

    /*----- 何桁あるか調べる -----*/
    for( i=0; i<10; i++) {
        if( ArHdr->ar_size[i] == 0x20) {
            break;
        }else{
            digit *= 10;
        }
    }
    digit /= 10;
    /*----------------------------*/

    /*----- サイズを算出する -----*/
    for( i=0; i<10; i++) {
        size += (*(((u8*)(ArHdr->ar_size))+i) - 0x30) * digit;	//charをu8に変換
        if( digit == 1) {
            break;
        }else{
            digit /= 10;
        }
    }
    /*----------------------------*/
    
    return size;
}


