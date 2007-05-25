/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CDC - include - cdc
  File:     cdc_CDC_Dsmode_access.h

  Copyright 2006-2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CDC_CDC_DSMODE_ACCESS_H_
#define TWL_CDC_CDC_DSMODE_ACCESS_H_

#include <twl/misc.h>
#include <twl/types.h>
#include <twl/cdc.h>
#include <nitro/spi.h>
//#include <nitro/tp/tp_reg.h>


#ifdef __cplusplus
extern "C" {
#endif


//================================================================
// CODEC status variables
//================================================================
extern SPIBaudRate cdcDsmodeSPIBaudRate;


//================================================================
//    BAUDRATE parameter
//================================================================

#define DSMODE_SPI_BAUDRATE_DEFAULT     SPI_BAUDRATE_1MHZ


/*---------------------------------------------------------------------------*
  Name:         i_tpSetSPIBaudRate

  Description:  set SPI baud rate.

  Arguments:    baud rate.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void CDCi_DsmodeSetSPIBaudRate( SPIBaudRate rate )
{
    cdcDsmodeSPIBaudRate = rate;
}

/*---------------------------------------------------------------------------*
  Name:         i_tpGetSPIBaudRate

  Description:  get SPI baud rate.

  Arguments:    None.

  Returns:      baud rate.
 *---------------------------------------------------------------------------*/
static inline SPIBaudRate CDCi_DsmodeGetSPIBaudRate( void )
{
    return cdcDsmodeSPIBaudRate;
}

/*---------------------------------------------------------------------------*
  Name:         i_tpChangeSpiMode

  Description:  change SPI mode..

  Arguments:    continuous : SPI_TRANSMODE_CONTINUOUS or SPI_TRANSMODE_1BYTE

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void CDCi_DsmodeChangeSpiMode( SPITransMode continuous )
{
    reg_SPI_SPICNT = (u16)((1 << REG_SPI_SPICNT_E_SHIFT) |
                           (0 << REG_SPI_SPICNT_I_SHIFT) |
                           (SPI_COMMPARTNER_TP << REG_SPI_SPICNT_SEL_SHIFT) |
                           (continuous << REG_SPI_SPICNT_MODE_SHIFT) |
                           (CDCi_DsmodeGetSPIBaudRate() << REG_SPI_SPICNT_BAUDRATE_SHIFT));
}

//================================================================================
//        SPI BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDCi_DsmodeSetSpiParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeSetSpiParams( u8 reg, u8 setBits, u8 maskBits );
void CDC_DsmodeSetSpiParams( u8 reg, u8 setBits, u8 maskBits );

/*---------------------------------------------------------------------------*
  Name:         CDC_DsmodeSetSpiFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeSetSpiFlags( u8 reg, u8 setBits );
void CDC_DsmodeSetSpiFlags( u8 reg, u8 setBits );

/*---------------------------------------------------------------------------*
  Name:         CDC_DsmodeClearSpiFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                clrBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeClearSpiFlags( u8 reg, u8 clrBits );
void CDC_DsmodeClearSpiFlags( u8 reg, u8 clrBits );

//================================================================================
//        SPI ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDCi_DsmodeWriteSpiRegister

  Description:  set value to Touch-Panel register

  Arguments:    reg      : TP register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeWriteSpiRegister( u8 reg, u8 data );
static inline void CDC_DsmodeWriteSpiRegister( u8 reg, u8 data )
{
    (void)SPI_Lock(123);
    CDCi_DsmodeWriteSpiRegister( reg, data );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DsmodeReadSpiRegister

  Description:  get value from Touch-Panel register

  Arguments:    reg      : TP register

  Returns:      value which is read from specified TP register
 *---------------------------------------------------------------------------*/
u8 CDCi_DsmodeReadSpiRegister( u8 reg );
static inline void CDC_DsmodeReadSpiRegister( u8 reg )
{
    (void)SPI_Lock(123);
    CDCi_DsmodeReadSpiRegister( reg );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DsmodeWriteSpiRegisters

  Description:  set value to decive registers through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeWriteSpiRegisters( u8 reg, const u8 *bufp, size_t size );
static inline void CDC_DsmodeWriteSpiRegisters( u8 reg, const u8 *bufp, size_t size )
{
    (void)SPI_Lock(123);
    CDCi_DsmodeWriteSpiRegisters( reg, bufp, size );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DsmodeReadSpiRegisters

  Description:  get value from decive registers through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeReadSpiRegisters( u8 reg, u8 *bufp, size_t size );
static inline void CDC_DsmodeReadSpiRegisters( u8 reg, u8 *bufp, size_t size )
{
    (void)SPI_Lock(123);
    CDC_DsmodeReadSpiRegisters( reg, bufp, size );
    (void)SPI_Unlock(123);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_CDC_CDC_DSMODE_ACCESS_H_ */
#endif
