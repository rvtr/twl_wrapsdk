/*---------------------------------------------------------------------------*
  Project:  TwlSDK - - types definition
  File:     types.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_TYPES_H_
#define TWL_TYPES_H_

#include <nitro/types.h>

#ifdef  SDK_ASM
#else  //SDK_ASM

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	u32 e[4];
}
u128;

typedef volatile u128 vu128;

/*
    io_register_list_XX.hで使用するマクロと型
 */

typedef u128 REGType128;
typedef vu128 REGType128v;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //SDK_ASM

/* TWL_TYPES_H_ */
#endif
