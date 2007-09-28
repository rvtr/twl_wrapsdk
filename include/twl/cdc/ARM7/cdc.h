/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CDC - include
  File:     CDC_.h

  Copyright 2006-2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CDC_CDC_H_
#define TWL_CDC_CDC_H_

#include <twl/misc.h>
#include <twl/types.h>
#include <twl/cdc/ARM7/cdc_reg.h>
#include <nitro/spi/ARM7/spi.h>
#include <twl/i2c/ARM7/i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void SPI_Lock(u32 id);
extern void SPI_Unlock(u32 id);

/*---------------------------------------------------------------------------*
  Name:         CDC_InitMutex

  Description:  Init CODEC Mutex

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_InitMutex(void);

/*---------------------------------------------------------------------------*
  Name:         CDC_Lock

  Description:  Lock CODEC device

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
void CDC_Lock( void );              // 外部スレッドから呼ばれ、CODECデバイスの操作権利を取得する

/*---------------------------------------------------------------------------*
  Name:         CDC_Unlock

  Description:  Unlock CODEC device

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
void CDC_Unlock( void );            // 外部スレッドから呼ばれ、CODECデバイスの操作権利を解放する

//================================================================
//    BAUDRATE parameter
//================================================================

// CODECの制限により4MHZを最大とする
#define CDC_SPI_BAUDRATE_DEFAULT  SPI_BAUDRATE_4MHZ

//================================================================================
//        I2C BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_SetI2cParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CDCi_SetI2cParams( u8 reg, u8 setBits, u8 maskBits )
{
    I2Ci_SetParams( I2C_SLAVE_CODEC_TP, reg, setBits, maskBits );
}
static inline void CDC_SetI2cParams( u8 reg, u8 setBits, u8 maskBits )
{
    I2C_SetParams( I2C_SLAVE_CODEC_TP, reg, setBits, maskBits );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_SetI2cFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CDCi_SetI2cFlags( u8 reg, u8 setBits )
{
    I2Ci_SetFlags( I2C_SLAVE_CODEC_TP, reg, setBits );
}
static inline void CDC_SetI2cFlags( u8 reg, u8 setBits )
{
    I2C_SetFlags( I2C_SLAVE_CODEC_TP, reg, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ClearI2cFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CDCi_ClearI2cFlags( u8 reg, u8 clrBits )
{
    I2Ci_ClearFlags( I2C_SLAVE_CODEC_TP, reg, clrBits );
}
static inline void CDC_ClearI2cFlags( u8 reg, u8 clrBits )
{
    I2C_ClearFlags( I2C_SLAVE_CODEC_TP, reg, clrBits );
}

//================================================================================
//        I2C ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_WriteI2cRegister

  Description:  set value to decive register through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CDCi_WriteI2cRegister( u8 reg, u8 data )
{
    I2Ci_WriteRegister( I2C_SLAVE_CODEC_TP, reg, data );
}
static inline void CDC_WriteI2cRegister( u8 reg, u8 data )
{
    I2C_WriteRegister( I2C_SLAVE_CODEC_TP, reg, data );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadI2cRegister

  Description:  get value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline u8 CDCi_ReadI2cRegister( u8 reg )
{
    return I2Ci_ReadRegister( I2C_SLAVE_CODEC_TP, reg );
}
static inline u8 CDC_ReadI2cRegister( u8 reg )
{
    return I2C_ReadRegister( I2C_SLAVE_CODEC_TP, reg );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_WriteI2cRegisters

  Description:  set value to decive registers through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CDCi_WriteI2cRegisters( u8 reg, const u8 *bufp, size_t size )
{
    I2Ci_WriteRegisters( I2C_SLAVE_CODEC_TP, reg, bufp, size );
}
static inline void CDC_WriteI2cRegisters( u8 reg, const u8 *bufp, size_t size )
{
    I2C_WriteRegisters( I2C_SLAVE_CODEC_TP, reg, bufp, size );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadI2cRegisters

  Description:  get value from decive registers through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline void CDCi_ReadI2cRegisters( u8 reg, u8 *bufp, size_t size )
{
    I2Ci_ReadRegisters( I2C_SLAVE_CODEC_TP, reg, bufp, size );
}
static inline void CDC_ReadI2cRegisters( u8 reg, u8 *bufp, size_t size )
{
    I2C_ReadRegisters( I2C_SLAVE_CODEC_TP, reg, bufp, size );
}

//================================================================================
//        SPI BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_SetSpiParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_SetSpiParams( u8 reg, u8 setBits, u8 maskBits );
void CDC_SetSpiParams( u8 reg, u8 setBits, u8 maskBits );
void CDCi_SetSpiParamsEx( u8 page, u8 reg, u8 setBits, u8 maskBits );
void CDC_SetSpiParamsEx( u8 page, u8 reg, u8 setBits, u8 maskBits );

/*---------------------------------------------------------------------------*
  Name:         CDC_SetSpiFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_SetSpiFlags( u8 reg, u8 setBits );
static inline void CDC_SetSpiFlags( u8 reg, u8 setBits )
{
    CDC_SetSpiParams( reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ClearSpiFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_ClearSpiFlags( u8 reg, u8 clrBits );
static inline void CDC_ClearSpiFlags( u8 reg, u8 clrBits )
{
    CDC_SetSpiParams( reg, 0, clrBits );
}

//================================================================================
//        SPI ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_WriteSpiRegister

  Description:  set value to decive register through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_WriteSpiRegister( u8 reg, u8 data );
void CDC_WriteSpiRegister( u8 reg, u8 data );
void CDCi_WriteSpiRegisterEx( u8 page, u8 reg, u8 data );
void CDC_WriteSpiRegisterEx( u8 page, u8 reg, u8 data );

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadSpiRegister

  Description:  get value from decive register through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u8 CDCi_ReadSpiRegister( u8 reg );
u8 CDC_ReadSpiRegister( u8 reg );
u8 CDCi_ReadSpiRegisterEx( u8 page, u8 reg );
u8 CDC_ReadSpiRegisterEx( u8 page, u8 reg );

/*---------------------------------------------------------------------------*
  Name:         CDC_WriteSpiRegisters

  Description:  set value to decive registers through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_WriteSpiRegisters( u8 reg, const u8 *bufp, size_t size );
void CDC_WriteSpiRegisters( u8 reg, const u8 *bufp, size_t size );
void CDCi_WriteSpiRegistersEx( u8 page, u8 reg, const u8 *bufp, size_t size );
void CDC_WriteSpiRegistersEx( u8 page, u8 reg, const u8 *bufp, size_t size );

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadSpiRegisters

  Description:  get value from decive registers through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
void CDCi_ReadSpiRegisters( u8 reg, u8 *bufp, size_t size );
void CDC_ReadSpiRegisters( u8 reg, u8 *bufp, size_t size );
void CDCi_ReadSpiRegistersEx( u8 page, u8 reg, u8 *bufp, size_t size );
void CDC_ReadSpiRegistersEx( u8 page, u8 reg, u8 *bufp, size_t size );

//================================================================================
//        Utility Functions
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_ChangePage

  Description:  change register page

  Arguments:    page_no  : next page number

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_ChangePage( u8 page_no );
void CDC_ChangePage( u8 page_no );

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_CDC_CDC_H_ */
#endif
