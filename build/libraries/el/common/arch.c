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
 �G���g���w�b�_����G���g���̃T�C�Y�����߂�
 --------------------------------------------------------*/
u32 AR_GetEntrySize( ArchHdr* ArHdr)
{
    u16 i;
    u32 digit = 1;
    u32 size = 0;

    /*----- �������邩���ׂ� -----*/
    for( i=0; i<10; i++) {
        if( ArHdr->ar_size[i] == 0x20) {
            break;
        }else{
            digit *= 10;
        }
    }
    digit /= 10;
    /*----------------------------*/

    /*----- �T�C�Y���Z�o���� -----*/
    for( i=0; i<10; i++) {
        size += (*(((u8*)(ArHdr->ar_size))+i) - 0x30) * digit;	//char��u8�ɕϊ�
        if( digit == 1) {
            break;
        }else{
            digit /= 10;
        }
    }
    /*----------------------------*/
    
    return size;
}


