/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - mcu
  File:     i2c.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MCU_I2C_COMMON_H_
#define TWL_MCU_I2C_COMMON_H_

#include <twl/types.h>
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

/*---------------------------------------------------------------------------*
  Name:         MCUi_WriteRegisters

  Description:  set value to decive registers through I2C.

  Arguments:    reg      : decive register
                bufp     : data array to be written
                size     : data size

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL MCUi_WriteRegisters( u8 reg, const u8 *bufp, size_t size )
{
    return I2Ci_WriteRegisters( I2C_SLAVE_MICRO_CONTROLLER, reg, bufp, size );
}
static inline BOOL MCU_WriteRegisters( u8 reg, const u8 *bufp, size_t size )
{
    return I2C_WriteRegisters( I2C_SLAVE_MICRO_CONTROLLER, reg, bufp, size );
}

/*---------------------------------------------------------------------------*
  Name:         MCUi_ReadRegisters

  Description:  get value from decive registers through I2C.

  Arguments:    reg      : decive register
                bufp     : data array to be read
                size     : data size

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline BOOL MCUi_ReadRegisters( u8 reg, u8 *bufp, size_t size )
{
    return I2Ci_ReadRegisters( I2C_SLAVE_MICRO_CONTROLLER, reg, bufp, size );
}
static inline BOOL MCU_ReadRegisters( u8 reg, u8 *bufp, size_t size )
{
    return I2C_ReadRegisters( I2C_SLAVE_MICRO_CONTROLLER, reg, bufp, size );
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
    return I2Ci_SetParams( I2C_SLAVE_MICRO_CONTROLLER, reg, setBits, maskBits );
}
static inline BOOL MCU_SetParams( u8 reg, u8 setBits, u8 maskBits )
{
    return I2C_SetParams( I2C_SLAVE_MICRO_CONTROLLER, reg, setBits, maskBits );
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

/* TWL_MCU_I2C_COMMON_H_ */
#endif
