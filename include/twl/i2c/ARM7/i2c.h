/*---------------------------------------------------------------------------*
  Project:  TwlSDK - I2C
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

#ifndef TWL_I2C_I2C_H_
#define TWL_I2C_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/misc.h>
#include <twl/ioreg.h>
#include <nitro/hw/ARM7/ioreg_EXI.h>

//----------------------------------------------------------------
//        enums
//
//---- I2C device ID
typedef enum
{
    I2C_SLAVE_CODEC_TP = 0,
    I2C_SLAVE_CAMERA_MICRON_IN,
    I2C_SLAVE_CAMERA_MICRON_OUT,
    I2C_SLAVE_CAMERA_SHARP_IN,
    I2C_SLAVE_CAMERA_SHARP_OUT,
    I2C_SLAVE_MICRO_CONTROLLER,
    I2C_SLAVE_DEBUG_LED,
    I2C_SLAVE_NUM
}
I2CSlave;


//---- I2C read/write
typedef enum
{
    I2C_WRITE = 0,
    I2C_READ  = 1
}
I2CReadWrite;


#define I2C_ADDR_CODEC      (0x18 << 1)
#define I2C_ADDR_CAMERA_MICRON_IN   0x7A    // MICRON
#define I2C_ADDR_CAMERA_MICRON_OUT  0x78    // MICRON
#define I2C_ADDR_CAMERA_SHARP_IN    0xE0    // SHARP
#define I2C_ADDR_CAMERA_SHARP_OUT   0xA0    // SHARP
#define I2C_ADDR_MICRO_CONTROLLER   0x4A
#define I2C_ADDR_DEBUG_LED  (0x20 << 1)

//----------------------------------------------------------------
//        subroutine definition
//
BOOL    I2C_Init( void );
BOOL    I2C_Lock( void );              // 外部スレッドから呼ばれ、I2Cデバイスの操作権利を取得する
BOOL    I2C_Unlock( void );            // 外部スレッドから呼ばれ、I2Cデバイスの操作権利を解放する

//----------------------------------------------------------------
//---- check I2C is busy
static inline BOOL I2C_IsBusy(void)
{
    return (BOOL)((reg_EXI_I2CCNT & REG_EXI_I2CCNT_E_MASK) >> REG_EXI_I2CCNT_E_SHIFT);
}
/*---------------------------------------------------------------------------*
  Name:         I2Ci_Wait

  Description:  I2Cを用いたデータ転送を実行中の場合、1バイト転送が完了するまで
                待機する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void I2Ci_Wait(void)
{
    while (reg_EXI_I2CCNT & REG_EXI_I2CCNT_E_MASK)
    {
    }
}


//================================================================================
//        DEVICE BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         I2C_SetParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_SetParams( I2CSlave id, u8 reg, u8 setBits, u8 maskBits );
BOOL I2C_SetParams( I2CSlave id, u8 reg, u8 setBits, u8 maskBits );

/*---------------------------------------------------------------------------*
  Name:         I2C_SetFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_SetFlags( I2CSlave id, u8 reg, u8 setBits );
BOOL I2C_SetFlags( I2CSlave id, u8 reg, u8 setBits );

/*---------------------------------------------------------------------------*
  Name:         I2C_ClearFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ClearFlags( I2CSlave id, u8 reg, u8 clrBits );
BOOL I2C_ClearFlags( I2CSlave id, u8 reg, u8 clrBits );

/*---------------------------------------------------------------------------*
  Name:         I2C_SetParams16

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_SetParams16( I2CSlave id, u16 reg, u16 setBits, u16 maskBits );
BOOL I2C_SetParams16( I2CSlave id, u16 reg, u16 setBits, u16 maskBits );

/*---------------------------------------------------------------------------*
  Name:         I2C_SetFlags16

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_SetFlags16( I2CSlave id, u16 reg, u16 setBits );
BOOL I2C_SetFlags16( I2CSlave id, u16 reg, u16 setBits );

/*---------------------------------------------------------------------------*
  Name:         I2C_ClearFlags16

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ClearFlags16( I2CSlave id, u16 reg, u16 clrBits );
BOOL I2C_ClearFlags16( I2CSlave id, u16 reg, u16 clrBits );

//================================================================================
//        DEVICE ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         I2C_WriteRegister

  Description:  set value to decive register through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_WriteRegister( I2CSlave id, u8 reg, u8 data );
static inline BOOL I2C_WriteRegister( I2CSlave id, u8 reg, u8 data )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_WriteRegister( id, reg, data );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_WriteRegister16

  Description:  set value to decive register through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_WriteRegister16( I2CSlave id, u16 reg, u16 data );
static inline BOOL I2C_WriteRegister16( I2CSlave id, u16 reg, u16 data )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_WriteRegister16( id, reg, data );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_ReadRegister

  Description:  get value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u8 I2Ci_ReadRegister( I2CSlave id, u8 reg );
static inline u8 I2C_ReadRegister( I2CSlave id, u8 reg )
{
    u8 result;
    (void)I2C_Lock();
    result = I2Ci_ReadRegister( id, reg );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_ReadRegisterSC

  Description:  get value from decive register through I2C.
                this version sends second byte with stop condition

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u8 I2Ci_ReadRegisterSC( I2CSlave id, u8 reg );
static inline u8 I2C_ReadRegisterSC( I2CSlave id, u8 reg )
{
    u8 result;
    (void)I2C_Lock();
    result = I2Ci_ReadRegisterSC( id, reg );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_ReadRegister16

  Description:  get value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u16 I2Ci_ReadRegister16( I2CSlave id, u16 reg );
static inline u16 I2C_ReadRegister16( I2CSlave id, u16 reg )
{
    u16 result;
    (void)I2C_Lock();
    result = I2Ci_ReadRegister16( id, reg );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_VerifyRegister

  Description:  get and verify value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegister( I2CSlave id, u8 reg, u8 data );
static inline BOOL I2C_VerifyRegister( I2CSlave id, u8 reg, u8 data )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_VerifyRegister( id, reg, data );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_VerifyRegisterSC

  Description:  get and verify value from decive register through I2C.
                this version sends second byte with stop condition

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegisterSC( I2CSlave id, u8 reg, u8 data );
static inline BOOL I2C_VerifyRegisterSC( I2CSlave id, u8 reg, u8 data )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_VerifyRegisterSC( id, reg, data );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_VerifyRegister16

  Description:  get and verify value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegister16( I2CSlave id, u16 reg, u16 data );
static inline BOOL I2C_VerifyRegister16( I2CSlave id, u16 reg, u16 data )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_VerifyRegister16( id, reg, data );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_WriteRegisters

  Description:  set value to decive register through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_WriteRegisters( I2CSlave id, u8 reg, const u8 *bufp, size_t size );
static inline BOOL I2C_WriteRegisters( I2CSlave id, u8 reg, const u8 *bufp, size_t size )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_WriteRegisters( id, reg, bufp, size );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_WriteRegisters16

  Description:  set value to decive register through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_WriteRegisters16( I2CSlave id, u16 reg, const u16 *bufp, size_t size );
static inline BOOL I2C_WriteRegisters16( I2CSlave id, u16 reg, const u16 *bufp, size_t size )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_WriteRegisters16( id, reg, bufp, size );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_ReadRegisters

  Description:  get value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ReadRegisters( I2CSlave id, u8 reg, u8 *bufp, size_t size );
static inline BOOL I2C_ReadRegisters( I2CSlave id, u8 reg, u8 *bufp, size_t size )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_ReadRegisters( id, reg, bufp, size );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_ReadRegistersSC

  Description:  get value from decive register through I2C.
                this version sends second byte with stop condition

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ReadRegistersSC( I2CSlave id, u8 reg, u8 *bufp, size_t size );
static inline BOOL I2C_ReadRegistersSC( I2CSlave id, u8 reg, u8 *bufp, size_t size )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_ReadRegistersSC( id, reg, bufp, size );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_ReadRegisters16

  Description:  get value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ReadRegisters16( I2CSlave id, u16 reg, u16 *bufp, size_t size );
static inline BOOL I2C_ReadRegisters16( I2CSlave id, u16 reg, u16 *bufp, size_t size )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_ReadRegisters16( id, reg, bufp, size );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_VerifyRegisters

  Description:  get and verify value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegisters( I2CSlave id, u8 reg, const u8 *bufp, size_t size );
static inline BOOL I2C_VerifyRegisters( I2CSlave id, u8 reg, const u8 *bufp, size_t size )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_VerifyRegisters( id, reg, bufp, size );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_VerifyRegistersSC

  Description:  get and verify value from decive register through I2C.
                this version sends second byte with stop condition

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegistersSC( I2CSlave id, u8 reg, const u8 *bufp, size_t size );
static inline BOOL I2C_VerifyRegistersSC( I2CSlave id, u8 reg, const u8 *bufp, size_t size )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_VerifyRegistersSC( id, reg, bufp, size );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_VerifyRegisters16

  Description:  get and verify value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegisters16( I2CSlave id, u16 reg, const u16 *bufp, size_t size );
static inline BOOL I2C_VerifyRegisters16( I2CSlave id, u16 reg, const u16 *bufp, size_t size )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_VerifyRegisters16( id, reg, bufp, size );
    (void)I2C_Unlock();
    return result;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_I2C_I2C_H_ */
#endif
