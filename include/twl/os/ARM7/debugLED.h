/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     debugLED.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef TWL_OS_DEBUG_LED_H_
#define TWL_OS_DEBUG_LED_H_

//#if PLATFORM == TS

#include <twl/i2c/ARM7/i2c.h>

#define OS_InitDebugLED()       I2C_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x03, 0x00)
#define OS_SetDebugLED(pattern) I2C_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, (pattern))
#define OS_GetDebugLED()        I2C_ReadRegister(I2C_SLAVE_DEBUG_LED, 0x01)

//#endif // PLATFORM == TS

/* TWL_OS_DEBUG_LED_H_ */
#endif
