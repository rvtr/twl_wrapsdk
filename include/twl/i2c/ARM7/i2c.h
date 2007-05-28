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
    I2C_SLAVE_CAMERA,
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


#define I2C_ADDR_CODEC     (0x18 << 1)
//#define I2C_ADDR_CAMERA     0x5a       // SAMSUNG 1/8
#define I2C_ADDR_CAMERA    (0x62 << 1) // SAMSUNG 1/10

//----------------------------------------------------------------
//        subroutine definition
//
BOOL    I2C_Init( void );
BOOL    I2C_Lock( void );              // �O���X���b�h����Ă΂�AI2C�f�o�C�X�̑��쌠�����擾����
BOOL    I2C_Unlock( void );            // �O���X���b�h����Ă΂�AI2C�f�o�C�X�̑��쌠�����������

//----------------------------------------------------------------
//---- check I2C is busy
static inline BOOL I2C_IsBusy(void)
{
    return (BOOL)((reg_EXI_I2CCNT & REG_EXI_I2CCNT_E_MASK) >> REG_EXI_I2CCNT_E_SHIFT);
}
/*---------------------------------------------------------------------------*
  Name:         I2Ci_Wait

  Description:  I2C��p�����f�[�^�]�������s���̏ꍇ�A1�o�C�g�]������������܂�
                �ҋ@����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void I2Ci_Wait(void)
{
    while (reg_EXI_I2CCNT & REG_EXI_I2CCNT_E_MASK)
    {
    }
}


/*---------------------------------------------------------------------------*
  Name:         I2Ci_Init

  Description:  initialize I2C

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void I2Ci_Init( void );


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
#if 0
//================================================================================
//        INTERRUPT
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         I2Ci_EnableInterrupt

  Description:  enable I2C interrupt for each device.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void I2Ci_EnableInterrupt( void );

/*---------------------------------------------------------------------------*
  Name:         I2Ci_DisableInterrupt

  Description:  disable I2C interrupt for each device.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void I2Ci_DisableInterrupt( void );
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_I2C_I2C_H_ */
#endif