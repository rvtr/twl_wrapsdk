/*---------------------------------------------------------------------------*
  Project:  TwlSDK - mcu - include
  File:     i2c.h

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
#ifndef TWL_MCU_I2C_H_
#define TWL_MCU_I2C_H_

#ifdef SDK_ARM7

#include <twl/i2c/ARM7/i2c.h>

#ifdef _cplusplus
extern "C" {
#endif

//================================================================================
//        I2C ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MCUi_WriteRegister

  Description:  set value to decive register through I2C.

  Arguments:    reg     : decive register
                data    : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL MCUi_WriteRegister( u8 reg, u8 data )
{
    return I2Ci_WriteRegister( I2C_SLAVE_MICRO_CONTROLLER, reg, data );
}
static inline BOOL MCU_WriteRegister( u8 reg, u8 data )
{
    return I2C_WriteRegister( I2C_SLAVE_MICRO_CONTROLLER, reg, data );
}

/*---------------------------------------------------------------------------*
  Name:         MCUi_ReadRegister

  Description:  get value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline u8 MCUi_ReadRegister( u8 reg )
{
    return I2Ci_ReadRegister( I2C_SLAVE_MICRO_CONTROLLER, reg );
}
static inline u8 MCU_ReadRegister( u8 reg )
{
    return I2C_ReadRegister( I2C_SLAVE_MICRO_CONTROLLER, reg );
}

//================================================================================
//        I2C BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MCUi_SetParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL MCUi_SetParams( u8 reg, u8 setBits, u8 maskBits )
{
    u8  tmp;
    tmp = MCU_ReadRegister( reg );
    tmp &= ~maskBits;
    setBits &= maskBits;
    tmp |= setBits;
    return MCU_WriteRegister( reg, tmp );
}
static inline BOOL MCU_SetParams( u8 reg, u8 setBits, u8 maskBits )
{
    BOOL result;
    (void)I2C_Lock();
    result = MCUi_SetParams( reg, setBits, maskBits );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MCUi_SetFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL MCUi_SetFlags( u8 reg, u8 setBits )
{
    return MCUi_SetParams( reg, setBits, setBits );
}
static inline BOOL MCU_SetFlags( u8 reg, u8 setBits )
{
    return MCU_SetParams( reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         MCUi_ClearFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                clrBits  : bits to clear

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL MCUi_ClearFlags( u8 reg, u8 clrBits )
{
    return MCUi_SetParams( reg, 0, clrBits );
}
static inline BOOL MCU_ClearFlags( u8 reg, u8 clrBits )
{
    return MCU_SetParams( reg, 0, clrBits );
}

#ifdef _cplusplus
} /* extern "C" */
#endif

#endif // SDK_ARM7

/* TWL_MCU_I2C_H_ */
#endif
