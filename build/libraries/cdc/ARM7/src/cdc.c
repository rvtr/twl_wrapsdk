/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - CDC_
  File:     CDC_.c

  Copyright 2006-2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/i2c/ARM7/i2c.h>
#include <twl/cdc/ARM7/cdc_reg.h>
#include <twl/cdc/ARM7/cdc.h>

#include "spi_sp.h"

void CDCi_PreInitAudio( void );
void CDCi_PostInitAudio( void );
void CDCi_PreInitMic( void );
void CDCi_PostInitMic( void );
void CDCi_InitTouchPanel( void );
void CDCi_InitCoefTable( void );


SPIBaudRate cdcSPIBaudRate = CDC_SPI_BAUDRATE_DEFAULT;
BOOL        cdcIsTwlMode   = TRUE;
int         cdcCurrentPage = 0;

int         cdcRevisionID  = 0;

#define     CDC_SPI_MODE_SETTING_REVISION_A     ((u16)((1 << REG_SPI_SPICNT_E_SHIFT) |  \
                           (0 << REG_SPI_SPICNT_I_SHIFT) |                              \
                           (SPI_SLAVE_RESERVED << REG_SPI_SPICNT_SEL_SHIFT) |           \
                           (CDC_SPI_BAUDRATE_DEFAULT << REG_SPI_SPICNT_BAUDRATE_SHIFT)))
#define     CDC_SPI_MODE_SETTING_REVISION_B     ((u16)((1 << REG_SPI_SPICNT_E_SHIFT) |  \
                           (0 << REG_SPI_SPICNT_I_SHIFT) |                              \
                           (SPI_COMMPARTNER_TP << REG_SPI_SPICNT_SEL_SHIFT) |           \
                           (CDC_SPI_BAUDRATE_DEFAULT << REG_SPI_SPICNT_BAUDRATE_SHIFT)))
#define     CDC_SPI_MODE_SETTING_REVISION_C     CDC_SPI_MODE_SETTING_REVISION_B

u16         cdcSpiMode     = CDC_SPI_MODE_SETTING_REVISION_B;

//================================================================================
static inline void CDCi_ChangeSpiMode( SPITransMode continuous )
{
    reg_SPI_SPICNT = (u16)((continuous << REG_SPI_SPICNT_MODE_SHIFT) | cdcSpiMode );
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
void CDCi_SetSpiParams( u8 reg, u8 setBits, u8 maskBits )
{
    u8      tmp;
    tmp = CDCi_ReadSpiRegister( reg );
    tmp &= ~maskBits;
    setBits &= maskBits;
    tmp |= setBits;
    CDCi_WriteSpiRegister( reg, tmp );
}
void CDC_SetSpiParams( u8 reg, u8 setBits, u8 maskBits )
{
    (void)SPI_Lock(123);
    CDCi_SetSpiParams( reg, setBits, maskBits );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_SetSpiFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_SetSpiFlags( u8 reg, u8 setBits )
{
    CDCi_SetSpiParams( reg, setBits, setBits );
}
void CDC_SetSpiFlags( u8 reg, u8 setBits )
{
    CDC_SetSpiParams( reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_ClearSpiFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                clrBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_ClearSpiFlags( u8 reg, u8 clrBits )
{
    CDCi_SetSpiParams( reg, 0, clrBits );
}
void CDC_ClearSpiFlags( u8 reg, u8 clrBits )
{
    CDC_SetSpiParams( reg, 0, clrBits );
}

//================================================================================
//        SPI ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDCi_WriteSpiRegister

  Description:  set value to PMIC register

  Arguments:    reg      : PMIC register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_WriteSpiRegister( u8 reg, u8 data )
{
    SPI_Wait();

    CDCi_ChangeSpiMode( SPI_TRANSMODE_CONTINUOUS );
    SPI_SendWait( (u8)(reg << 1) );

    CDCi_ChangeSpiMode( SPI_TRANSMODE_1BYTE );
    SPI_Send( data );
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_ReadSpiRegister

  Description:  get value from PMIC register

  Arguments:    reg      : PMIC register

  Returns:      value which is read from specified PMIC register
 *---------------------------------------------------------------------------*/
u8 CDCi_ReadSpiRegister( u8 reg )
{
    u8      data;

    SPI_Wait();

    CDCi_ChangeSpiMode( SPI_TRANSMODE_CONTINUOUS );
    SPI_SendWait( (u8)(reg << 1) );

    CDCi_ChangeSpiMode( SPI_TRANSMODE_1BYTE );
    data = (u8)SPI_DummyWaitReceive();
    return data;
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_WriteSpiRegisters

  Description:  set value to decive registers through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_WriteSpiRegisters( u8 reg, const u8 *bufp, size_t size )
{
    int i;

    SPI_Wait();

    CDCi_ChangeSpiMode( SPI_TRANSMODE_CONTINUOUS );
    SPI_SendWait( (u8)(reg << 1) );

    for ( i=0; i<(size-1); i++ )
    {
        SPI_Wait();
        SPI_Send( *bufp++ );
    }
    SPI_Wait();
    CDCi_ChangeSpiMode( SPI_TRANSMODE_1BYTE );
    SPI_Send( *bufp++ );
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_ReadSpiRegisters

  Description:  get value from decive registers through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
void CDCi_ReadSpiRegisters( u8 reg, u8 *bufp, size_t size )
{
    int i;

    SPI_Wait();

    CDCi_ChangeSpiMode( SPI_TRANSMODE_CONTINUOUS );
    SPI_SendWait( (u8)(reg << 1) );

    for ( i=0; i<(size-1); i++ )
    {
        SPI_Wait();
        *bufp++ = (u8)SPI_DummyWaitReceive();
    }
    CDCi_ChangeSpiMode( SPI_TRANSMODE_1BYTE );
    *bufp++ = (u8)SPI_DummyWaitReceive();
}

//================================================================================
//        Utility Functions
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDCi_ChangePage

  Description:  change register page

  Arguments:    page_no  : next page number

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_ChangePage( int page_no )
{
    SDK_ASSERT( (page_no == 0) || (page_no == 1) || (page_no == 3)   || (page_no == 4) ||
                (page_no == 8) || (page_no == 9) || (page_no == 252) || (page_no == 255) );

    // Šù‚É‚»‚Ìƒy[ƒW‚É‚¢‚é
    if (cdcCurrentPage == page_no)
        return;

    // 255 ‚¾‚¯•Êˆµ‚¢
    if (cdcCurrentPage == 255)
    {
        CDCi_WriteI2cRegister( REG_CDC255_PAGE_CTL_ADDR, (u8)page_no );
    }
    else
    {
        CDCi_WriteI2cRegister( REG_CDC_PAGE_CTL_ADDR, (u8)page_no );
    }

    cdcCurrentPage = page_no;
}
// maybe change i2c to spi
void CDC_ChangePage( int page_no )
{
    (void)I2C_Lock();
    CDCi_ChangePage( page_no );
    (void)I2C_Unlock();
}


