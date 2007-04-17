/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - HW
  File:     memorymap.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MEMORYMAP_H_
#define TWL_MEMORYMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	SDK_ARM9
#include	<nitro/hw/ARM9/mmap_global.h>
#include	<nitro/hw/ARM9/mmap_main.h>
#include	<nitro/hw/ARM9/mmap_tcm.h>
#include	<nitro/hw/ARM9/mmap_vram.h>
#include	<nitro/hw/common/mmap_shared.h>

#else  //SDK_ARM7
#include	<nitro/hw/ARM7/mmap_global.h>
#include	<nitro/hw/ARM7/mmap_main.h>
#include	<nitro/hw/ARM7/mmap_wram.h>
#include	<nitro/hw/common/mmap_shared.h>
#endif

#include	<twl/ioreg.h>

#ifdef __cplusplus
} /* extern "C" */
#endif
/* TWL_MEMORYMAP_H_ */
#endif
