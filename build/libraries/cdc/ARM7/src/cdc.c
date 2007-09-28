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

/*

	このソースで定義されているSPIアクセス関数はCODECがTWLモードの時に
　　CODECにアクセスするために使用します。CODECがDSモードの時は 
　　cdc_dsmode_access.c で定義されている関数を使用してください。

    [CTRモード時]

 	ＳＰＩ最大速度　：　4MHz (CODEC側の制限）
	チップセレクト　：　TCSN
	Read/Write指定　：　LSB

*/

#include <twl.h>
#include <twl/cdc/ARM7/cdc_reg.h>
#include <twl/cdc/ARM7/cdc.h>
#include <spi_sp.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define  CDC_SPI_MODE_SETTING   ((u16)((1 << REG_SPI_SPICNT_E_SHIFT)             |  \
                                (0 << REG_SPI_SPICNT_I_SHIFT)                    |  \
                                (SPI_COMMPARTNER_TP << REG_SPI_SPICNT_SEL_SHIFT) |  \
                                (CDC_SPI_BAUDRATE_DEFAULT << REG_SPI_SPICNT_BAUDRATE_SHIFT)))

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static OSMutex cdcMutex;

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

static inline void CDCi_ChangeSpiMode( SPITransMode continuous )
{
    reg_SPI_SPICNT = (u16)((continuous << REG_SPI_SPICNT_MODE_SHIFT) | CDC_SPI_MODE_SETTING );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_InitMutex

  Description:  Init CODEC Mutex

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_InitMutex(void)
{
    OS_InitMutex(&cdcMutex);
}

/*---------------------------------------------------------------------------*
  Name:         CDC_Lock

  Description:  Lock CODEC device

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
void CDC_Lock( void )              // 外部スレッドから呼ばれ、CODECデバイスの操作権利を取得する
{
    OS_LockMutex( &cdcMutex );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_Unlock

  Description:  Unlock CODEC device

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
void CDC_Unlock( void )            // 外部スレッドから呼ばれ、CODECデバイスの操作権利を解放する
{
    OS_UnlockMutex( &cdcMutex );
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

/*---------------------------------------------------------------------------*
  Name:         CDC_SetSpiParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetSpiParams( u8 reg, u8 setBits, u8 maskBits )
{
    (void)SPI_Lock(123);
    CDCi_SetSpiParams( reg, setBits, maskBits );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_SetSpiParamsEx

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_SetSpiParamsEx( u8 page, u8 reg, u8 setBits, u8 maskBits )
{
    CDC_ChangePage( page );
    CDC_SetSpiParams( reg, setBits, maskBits );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_SetSpiParamsEx

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetSpiParamsEx( u8 page, u8 reg, u8 setBits, u8 maskBits )
{
	CDC_Lock();
    CDCi_SetSpiParamsEx( page, reg, setBits, maskBits );
	CDC_Unlock();
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
  Name:         CDC_WriteSpiRegister

  Description:  set value to decive register through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_WriteSpiRegister( u8 reg, u8 data )
{
    (void)SPI_Lock(123);
    CDCi_WriteSpiRegister( reg, data );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_WriteSpiRegisterEx

  Description:  set value to decive register through SPI.

  Arguments:    page
  				reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_WriteSpiRegisterEx( u8 page, u8 reg, u8 data )
{
    CDC_ChangePage( page );
    CDC_WriteSpiRegister( reg, data );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_WriteSpiRegisterEx

  Description:  set value to decive register through SPI.

  Arguments:    page
  				reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_WriteSpiRegisterEx( u8 page, u8 reg, u8 data )
{
	CDC_Lock();
	CDCi_WriteSpiRegisterEx( page, reg, data );
	CDC_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_ReadSpiRegister

  Description:  get value from register

  Arguments:    reg      : register

  Returns:      value which is read from specified PMIC register
 *---------------------------------------------------------------------------*/
u8 CDCi_ReadSpiRegister( u8 reg )
{
    u8      data;

    SPI_Wait();

    CDCi_ChangeSpiMode( SPI_TRANSMODE_CONTINUOUS );
    SPI_SendWait( (u8)((reg << 1) | 1));

    CDCi_ChangeSpiMode( SPI_TRANSMODE_1BYTE );
    data = (u8)SPI_DummyWaitReceive();
    return data;
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadSpiRegister

  Description:  get value from decive register through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u8 CDC_ReadSpiRegister( u8 reg )
{
	u8 value;
    (void)SPI_Lock(123);
    value = CDCi_ReadSpiRegister( reg );
    (void)SPI_Unlock(123);
    return value;
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_ReadSpiRegisterEx

  Description:  get value from decive register through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u8 CDCi_ReadSpiRegisterEx( u8 page, u8 reg )
{
	u8 value;
    CDC_ChangePage( page );
    value = CDC_ReadSpiRegister( reg );
    return value;
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadSpiRegisterEx

  Description:  get value from decive register through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u8 CDC_ReadSpiRegisterEx( u8 page, u8 reg )
{
	u8 value;

	CDC_Lock();
    value = CDCi_ReadSpiRegisterEx( page, reg );
	CDC_Unlock();

    return value;
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
  Name:         CDC_WriteSpiRegisters

  Description:  set value to decive registers through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_WriteSpiRegisters( u8 reg, const u8 *bufp, size_t size )
{
    (void)SPI_Lock(123);
    CDCi_WriteSpiRegisters( reg, bufp, size );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_WriteSpiRegistersEx

  Description:  set value to decive registers through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_WriteSpiRegistersEx( u8 page, u8 reg, const u8 *bufp, size_t size )
{
    CDC_ChangePage( page );
    CDC_WriteSpiRegisters( reg, bufp, size );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_WriteSpiRegistersEx

  Description:  set value to decive registers through SPI.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_WriteSpiRegistersEx( u8 page, u8 reg, const u8 *bufp, size_t size )
{
	CDC_Lock();
    CDCi_WriteSpiRegistersEx( page, reg, bufp, size );
	CDC_Unlock();
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
    SPI_SendWait( (u8)((reg << 1) | 1));

    for ( i=0; i<(size-1); i++ )
    {
        SPI_Wait();
        *bufp++ = (u8)SPI_DummyWaitReceive();
    }
    CDCi_ChangeSpiMode( SPI_TRANSMODE_1BYTE );
    *bufp++ = (u8)SPI_DummyWaitReceive();
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadSpiRegisters

  Description:  get value from decive registers through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
void CDC_ReadSpiRegisters( u8 reg, u8 *bufp, size_t size )
{
    (void)SPI_Lock(123);
    CDCi_ReadSpiRegisters( reg, bufp, size );
    (void)SPI_Unlock(123);
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_ReadSpiRegistersEx

  Description:  get value from decive registers through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
void CDCi_ReadSpiRegistersEx( u8 page, u8 reg, u8 *bufp, size_t size )
{
    CDC_ChangePage( page );
    CDC_ReadSpiRegisters( reg, bufp, size );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_ReadSpiRegistersEx

  Description:  get value from decive registers through SPI.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
void CDC_ReadSpiRegistersEx( u8 page, u8 reg, u8 *bufp, size_t size )
{
	CDC_Lock();
    CDCi_ReadSpiRegistersEx( page, reg, bufp, size );
	CDC_Unlock();
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
void CDCi_ChangePage( u8 page_no )
{
	static u8 cdcCurrentPage = 0;

    SDK_ASSERT( (page_no == 0) || (page_no == 1) || (page_no == 3)   || (page_no == 4) ||
                (page_no == 8) || (page_no == 9) || (page_no == 252) || (page_no == 255) );

    // 既にそのページにいる
    if (cdcCurrentPage == page_no)
        return;

    // 255 だけ別扱い
    if (cdcCurrentPage == 255)
    {
        CDCi_WriteSpiRegister( REG_CDC255_PAGE_CTL_ADDR, (u8)page_no );
    }
    else
    {
        CDCi_WriteSpiRegister( REG_CDC_PAGE_CTL_ADDR, (u8)page_no );
    }

    cdcCurrentPage = page_no;
}

void CDC_ChangePage( u8 page_no )
{
    (void)SPI_Lock(123);
    CDCi_ChangePage( page_no );
    (void)SPI_Unlock(123);
}


