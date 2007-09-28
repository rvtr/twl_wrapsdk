/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - tp
  File:     cdc_Dsmode_access.c

  Copyright 2006-2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*

	このソースで定義されているSPIアクセス関数はCODECがDSモードの時に
　　CODECにアクセスするために使用します。CODECがTWLモードの時は 
    cdc.c で定義されている関数を使用してください。

    [DSモード時]

 	ＳＰＩ最大速度　：　2MHz (CODEC側の制限）
	チップセレクト　：　PCSN
	Read/Write指定　：　MSB

*/

#include <twl/cdc.h>
#include <spi_sp.h>

SPIBaudRate cdcDsmodeSPIBaudRate = DSMODE_SPI_BAUDRATE_DEFAULT;



//================================================================================
//        SPI BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_DsmodeSetSpiParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeSetSpiParams( u8 reg, u8 setBits, u8 maskBits )
{
    u8      tmp;
    tmp = CDCi_DsmodeReadSpiRegister( reg );
    tmp &= ~maskBits;
    setBits &= maskBits;
    tmp |= setBits;
    CDCi_DsmodeWriteSpiRegister( reg, tmp );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DsmodeSetSpiFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeSetSpiFlags( u8 reg, u8 setBits )
{
    CDCi_DsmodeSetSpiParams( reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DsmodeClearSpiFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                clrBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeClearSpiFlags( u8 reg, u8 clrBits )
{
    CDCi_DsmodeSetSpiParams( reg, 0, clrBits );
}

//================================================================================
//        SPI ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDCi_DsmodeWriteSpiRegister

  Description:  set value to Touch-Panel register

  Arguments:    reg      : DS-mode PMIC register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeWriteSpiRegister( u8 reg, u8 data )
{
    SPI_Wait();

    CDCi_DsmodeChangeSpiMode( SPI_TRANSMODE_CONTINUOUS );
    SPI_SendWait( reg );

    CDCi_DsmodeChangeSpiMode( SPI_TRANSMODE_1BYTE );
    SPI_Send( data );
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_DsmodeReadSpiRegister

  Description:  get value from Touch-Panel register

  Arguments:    reg      : DS-mode PMIC register

  Returns:      value which is read from specified TP register
 *---------------------------------------------------------------------------*/
u8 CDCi_DsmodeReadSpiRegister( u8 reg )
{
    u8      data;

    SPI_Wait();

    CDCi_DsmodeChangeSpiMode( SPI_TRANSMODE_CONTINUOUS );
    SPI_SendWait( (u8)(0x80 | reg) );

    CDCi_DsmodeChangeSpiMode( SPI_TRANSMODE_1BYTE );
    data = (u8)SPI_DummyWaitReceive();
    return data;
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_DsmodeWriteSpiRegisters

  Description:  set value to decive registers through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeWriteSpiRegisters( u8 reg, const u8 *bufp, size_t size )
{
    int i;

    SPI_Wait();

    CDCi_DsmodeChangeSpiMode( SPI_TRANSMODE_CONTINUOUS );
    SPI_SendWait( reg );

    for ( i=0; i<(size-1); i++ )
    {
        SPI_Wait();
        SPI_Send( *bufp++ );
    }
    SPI_Wait();
    CDCi_DsmodeChangeSpiMode( SPI_TRANSMODE_1BYTE );
    SPI_Send( *bufp++ );
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_DsmodeReadSpiRegisters

  Description:  get value from decive registers through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
void CDCi_DsmodeReadSpiRegisters( u8 reg, u8 *bufp, size_t size )
{
    int i;

    SPI_Wait();

    CDCi_DsmodeChangeSpiMode( SPI_TRANSMODE_CONTINUOUS );
    SPI_SendWait( (u8)(0x80 | reg) );

    for ( i=0; i<(size-1); i++ )
    {
        SPI_Wait();
        *bufp++ = (u8)SPI_DummyWaitReceive();
    }
    CDCi_DsmodeChangeSpiMode( SPI_TRANSMODE_1BYTE );
    *bufp++ = (u8)SPI_DummyWaitReceive();
}


