/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     emulator.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_OS_EMULATOR_H_
#define TWL_OS_EMULATOR_H_

#include <twl/misc.h>
#include <twl/types.h>

#include <nitro/os/common/emulator.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         OSi_IsRunOnDebuggerTWL

  Description:  Detect Debugger
                (subroutine of OS_GetConsoleType)

  Arguments:    None

  Returns:      TRUE  : debugger
                FALSE : not debugger
 *---------------------------------------------------------------------------*/
BOOL OSi_IsRunOnDebuggerTWL(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_EMULATOR_H_ */
#endif
