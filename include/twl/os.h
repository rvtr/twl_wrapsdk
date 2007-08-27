/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - OS
  File:     os.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_H_
#define TWL_OS_H_

#include <nitro/os.h>

#include <twl/os/common/system.h>
#include <twl/os/common/emulator.h>
#include <twl/os/common/systemCall.h>

#ifdef SDK_ARM9
#include <twl/os/ARM9/cache_tag.h>
#endif // SDK_ARM9

#ifdef SDK_DEBUGGER_KMC
#include <twl/vlink.h>
#endif // SDK_DEBUGGER_KMC

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_H_ */
#endif
