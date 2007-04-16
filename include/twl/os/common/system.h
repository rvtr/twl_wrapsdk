/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     system.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_OS_SYSTEM_H_
#define TWL_OS_SYSTEM_H_

#include  <nitro/os/common/system.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

typedef enum
{
    OS_CHIPTYPE_TWL = 0,
    OS_CHIPTYPE_DEBUGGER = 1,
    OS_CHIPTYPE_EVALUATE = 3
}
OSChipType;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_SYSTEM_H_ */
#endif
