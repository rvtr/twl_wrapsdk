/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - include
  File:     aes.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_AES_H_
#define TWL_AES_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <twl/aes/common/fifo.h>
#include    <twl/aes/common/assert.h>
#include    <twl/aes/common/swap.h>

#ifdef  SDK_ARM7

#include    <twl/aes/ARM7/control.h>
#include    <twl/aes/ARM7/instruction.h>
#include    <twl/aes/ARM7/transfer.h>

#else  // SDK_ARM9

#include    <twl/aes/ARM9/aes.h>

#endif

/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_AES_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
