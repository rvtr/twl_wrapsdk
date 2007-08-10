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

//================================================================
// CODEC status variables
//================================================================
extern SPIBaudRate cdcSPIBaudRate;
extern BOOL        cdcIsTwlMode;
extern int         cdcCurrentPage;

#define CDC_REVISION_A      0
#define CDC_REVISION_B      1
#define CDC_REVISION_C      2
extern int         cdcRevisionID;

extern u16         cdcSpiMode;

//================================================================
//    BAUDRATE parameter
//================================================================

// CODECÇÃêßå¿Ç…ÇÊÇË4MHZÇç≈ëÂÇ∆Ç∑ÇÈ
#define CDC_SPI_BAUDRATE_DEFAULT  SPI_BAUDRATE_4MHZ

/*---------------------------------------------------------------------------*
  Name:         CDCi_SetSPIBaudRate

  Description:  set SPI baud rate.

  Arguments:    baud rate.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void CDCi_SetSPIBaudRate( SPIBaudRate rate )
{
    cdcSPIBaudRate = rate;
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_GetSPIBaudRate

  Description:  get SPI baud rate.

  Arguments:    None.

  Returns:      baud rate.
 *---------------------------------------------------------------------------*/
static inline SPIBaudRate CDCi_GetSPIBaudRate( void )
{
    return cdcSPIBaudRate;
}

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

/*---------------------------------------------------------------------------*
  Name:         CDC_SetSpiFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_SetSpiFlags( u8 reg, u8 setBits );
void CDC_SetSpiFlags( u8 reg, u8 setBits );

/*---------------------------------------------------------------------------*
  Name:         CDC_ClearSpiFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_ClearSpiFlags( u8 reg, u8 clrBits );
void CDC_ClearSpiFlags( u8 reg, u8 clrBits );

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
static inline void CDC_WriteSpiRegister( u8 reg, u8 data )
{
    (void)SPI_Lock(123);
    CDCi_WriteSpiRegister( reg, data );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadSpiRegister

  Description:  get value from decive register through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u8 CDCi_ReadSpiRegister( u8 reg );
static inline u8 CDC_ReadSpiRegister( u8 reg )
{
	u8 value;
    (void)SPI_Lock(123);
    value = CDCi_ReadSpiRegister( reg );
    (void)SPI_Unlock(123);
    return value;
}

/*---------------------------------------------------------------------------*
  Name:         CDC_WriteSpiRegisters

  Description:  set value to decive registers through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_WriteSpiRegisters( u8 reg, const u8 *bufp, size_t size );
static inline void CDC_WriteSpiRegisters( u8 reg, const u8 *bufp, size_t size )
{
    (void)SPI_Lock(123);
    CDCi_WriteSpiRegisters( reg, bufp, size );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadSpiRegisters

  Description:  get value from decive registers through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
void CDCi_ReadSpiRegisters( u8 reg, u8 *bufp, size_t size );
static inline void CDC_ReadSpiRegisters( u8 reg, u8 *bufp, size_t size )
{
    (void)SPI_Lock(123);
    CDCi_ReadSpiRegisters( reg, bufp, size );
    (void)SPI_Unlock(123);
}

//================================================================================
//        Utility Functions
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_ChangePage

  Description:  change register page

  Arguments:    page_no  : next page number

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_ChangePage( int page_no );
void CDC_ChangePage( int page_no );

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_CDC_CDC_H_ */
#endif
