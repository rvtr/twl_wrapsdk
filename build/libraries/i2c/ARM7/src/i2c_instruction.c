/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - I2C_
  File:     I2C__instruction.c

  Copyright 2006-2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: I2C_.c,v $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/i2c/ARM7/i2c.h>

#define RETRY_COUNT     8

static u8 I2C_DeviceAddrTable[I2C_SLAVE_NUM] = { I2C_ADDR_CODEC,
                                                I2C_ADDR_CAMERA,
                                              };

static OSMutex mutex;
static BOOL isInitialized = FALSE;

static inline void I2Ci_Start( void )
{
    reg_EXI_I2CCNT = (u8)((1 << REG_EXI_I2CCNT_E_SHIFT) |
                          (1 << REG_EXI_I2CCNT_I_SHIFT) |   // 割り込み禁止は IE にて行うことで仕様統一
                          (I2C_WRITE << REG_EXI_I2CCNT_RW_SHIFT) |
                          (0 << REG_EXI_I2CCNT_ACK_SHIFT) |
                          (1 << REG_EXI_I2CCNT_START_SHIFT));
}

static inline void I2Ci_Continue( I2CReadWrite rw )
{
    reg_EXI_I2CCNT = (u8)((1 << REG_EXI_I2CCNT_E_SHIFT) |
                          (1 << REG_EXI_I2CCNT_I_SHIFT) |
                          (rw << REG_EXI_I2CCNT_RW_SHIFT) |
                          (rw << REG_EXI_I2CCNT_ACK_SHIFT));
}

static inline void I2Ci_Stop( I2CReadWrite rw )
{
    reg_EXI_I2CCNT = (u8)((1 << REG_EXI_I2CCNT_E_SHIFT) |
                          (1 << REG_EXI_I2CCNT_I_SHIFT) |
                          (rw << REG_EXI_I2CCNT_RW_SHIFT) |
                          (0 << REG_EXI_I2CCNT_ACK_SHIFT) |
                          (1 << REG_EXI_I2CCNT_STOP_SHIFT));
}

static inline void I2Ci_StopPhase1( I2CReadWrite rw )
{
    reg_EXI_I2CCNT = (u8)((1 << REG_EXI_I2CCNT_E_SHIFT) |
                          (1 << REG_EXI_I2CCNT_I_SHIFT) |
                          (rw << REG_EXI_I2CCNT_RW_SHIFT) |
                          (0 << REG_EXI_I2CCNT_ACK_SHIFT));
}
static inline void I2Ci_StopPhase2( void )
{
    reg_EXI_I2CCNT = (u8)((1 << REG_EXI_I2CCNT_E_SHIFT) |
                          (1 << REG_EXI_I2CCNT_I_SHIFT) |
                          (1 << REG_EXI_I2CCNT_STOP_SHIFT) |
                          (1 << REG_EXI_I2CCNT_NT_SHIFT));
}


static inline void I2Ci_SetData( u8 data )
{
    reg_EXI_I2CD = data;
}


static inline u8 I2Ci_GetData( void )
{
    return reg_EXI_I2CD;
}

static inline BOOL I2Ci_GetResult( void )
{
    I2Ci_Wait();
    return (BOOL)((reg_EXI_I2CCNT & REG_EXI_I2CCNT_ACK_MASK) >> REG_EXI_I2CCNT_ACK_SHIFT);
}

static inline BOOL I2Ci_SendStart( I2CSlave id )
{
    I2Ci_Wait();
    I2Ci_SetData( (u8)(I2C_DeviceAddrTable[id] | (u8)I2C_WRITE) );
    I2Ci_Start();
    return I2Ci_GetResult();
}

static inline BOOL I2Ci_SendMiddle( u8 data )
{
    I2Ci_Wait();
    I2Ci_SetData( data );
    I2Ci_Continue( I2C_WRITE );
    return I2Ci_GetResult();
}

static inline BOOL I2Ci_SendLast( u8 data )
{
    I2Ci_Wait();
    I2Ci_SetData( data );
    I2Ci_Stop( I2C_WRITE );
    return I2Ci_GetResult();
}

static inline BOOL I2Ci_ReceiveStart( I2CSlave id )
{
    I2Ci_Wait();
    I2Ci_SetData( (u8)(I2C_DeviceAddrTable[id] | I2C_READ) );
    I2Ci_Start();
    return I2Ci_GetResult();
}

static inline void I2Ci_ReceiveMiddle( void )
{
    I2Ci_Wait();
    I2Ci_Continue( I2C_READ );
}

static inline void I2Ci_ReceiveLast( void )
{
    I2Ci_Wait();
    I2Ci_Stop( I2C_READ );
}

static inline u8 I2Ci_WaitReceiveMiddle( void )
{
    I2Ci_ReceiveMiddle();
    I2Ci_Wait();
    return I2Ci_GetData();
}

static inline u8 I2Ci_WaitReceiveLast( void )
{
    I2Ci_ReceiveLast();
    I2Ci_Wait();
    return I2Ci_GetData();
}

/*---------------------------------------------------------------------------*
  Name:         I2C_Init

  Description:  initialize I2C

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL I2C_Init( void )
{
    if (isInitialized == FALSE)
    {
        OS_InitMutex(&mutex);
        I2Ci_Init();
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_Lock

  Description:  Lock I2C device

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL I2C_Lock( void )              // 外部スレッドから呼ばれ、I2Cデバイスの操作権利を取得する
{
    if( isInitialized == FALSE ) {
        if( FALSE == I2C_Init() ) {
            return FALSE;
        }
    }
    OS_LockMutex( &mutex );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_Unlock

  Description:  Unlock I2C device

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL I2C_Unlock( void )            // 外部スレッドから呼ばれ、I2Cデバイスの操作権利を解放する
{
    if( isInitialized == FALSE ) {
        if( FALSE == I2C_Init() ) {
            return FALSE;
        }
    }
    OS_UnlockMutex( &mutex );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_Init

  Description:  initialize I2C

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void I2Ci_Init( void )
{
#if 0 // TODO
    I2C_DeviceAddrTable[I2C_SLAVE_CODEC_TP] = ?; // from NorFlash
    I2C_DeviceAddrTable[I2C_SLAVE_LCDDAC] = ?;   // from NorFlash
    I2C_DeviceAddrTable[I2C_SLAVE_CAMERA] = ?;   // from NorFlash
#endif
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
BOOL I2Ci_SetParams( I2CSlave id, u8 reg, u8 setBits, u8 maskBits )
{
    u8      tmp;
    tmp = I2Ci_ReadRegister( id, reg );
    tmp &= ~maskBits;
    setBits &= maskBits;
    tmp |= setBits;
    return I2Ci_WriteRegister( id, reg, tmp );
}
BOOL I2C_SetParams( I2CSlave id, u8 reg, u8 setBits, u8 maskBits )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_SetParams( id, reg, setBits, maskBits );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_SetFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_SetFlags( I2CSlave id, u8 reg, u8 setBits )
{
    return I2Ci_SetParams( id, reg, setBits, setBits );
}
BOOL I2C_SetFlags( I2CSlave id, u8 reg, u8 setBits )
{
    return I2C_SetParams( id, reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         I2C_ClearFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                clrBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ClearFlags( I2CSlave id, u8 reg, u8 clrBits )
{
    return I2Ci_SetParams( id, reg, 0, clrBits );
}
BOOL I2C_ClearFlags( I2CSlave id, u8 reg, u8 clrBits )
{
    return I2C_SetParams( id, reg, 0, clrBits );
}

//================================================================================
//        DEVICE ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         I2Ci_WriteRegister

  Description:  set value to decive register through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_WriteRegister( I2CSlave id, u8 reg, u8 data )
{
    int r;
    int error;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle( reg ) == FALSE)                error++;
        if (I2Ci_SendLast( data ) == FALSE)                 error++;
        if (error == 0) break;
    }
    return error ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_ReadRegister

  Description:  get value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u8 I2Ci_ReadRegister( I2CSlave id, u8 reg )
{
    int r;
    u8 data;
    int error;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle( reg ) == FALSE)                error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        data = I2Ci_WaitReceiveLast();
        if (error == 0) break;
    }
    return error ? (u8)0xee : data;
}
/*---------------------------------------------------------------------------*
  Name:         I2Ci_ReadRegisterSC

  Description:  get value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u8 I2Ci_ReadRegisterSC( I2CSlave id, u8 reg )
{
    int r;
    u8 data;
    int error;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendLast( reg ) == FALSE)                  error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        data = I2Ci_WaitReceiveLast();
        if (error == 0) break;
    }
    return error ? (u8)0xee : data;
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_VerifyRegister

  Description:  get and verify value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegister( I2CSlave id, u8 reg, u8 data )
{
    int r;
    int error;
    BOOL result;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        result = TRUE;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle( reg ) == FALSE)                error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        if (data != I2Ci_WaitReceiveLast())
        {
            result = FALSE;
        }
        if (error == 0) break;
    }
    return error ? FALSE : (result ? TRUE : FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_VerifyRegisterSC

  Description:  get and verify value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegisterSC( I2CSlave id, u8 reg, u8 data )
{
    int r;
    int error;
    BOOL result;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        result = TRUE;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendLast( reg ) == FALSE)                  error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        if (data != I2Ci_WaitReceiveLast())
        {
            result = FALSE;
        }
        if (error == 0) break;
    }
    return error ? FALSE : (result ? TRUE : FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_WriteRegisters

  Description:  set value to decive registers through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_WriteRegisters( I2CSlave id, u8 reg, const u8 *bufp, size_t size )
{
    int i;
    int r;
    int error;
    const u8 *ptr;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        ptr = bufp;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle( reg ) == FALSE)                error++;
        for ( i=0; error==0 && i<(size-1); i++ )
        {
            if (I2Ci_SendMiddle( *ptr++ ) == FALSE)         error++;
        }
        if (I2Ci_SendLast( *ptr++ ) == FALSE)               error++;
        if (error == 0) break;
    }
    return error ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_ReadRegisters

  Description:  get value from decive registers through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ReadRegisters( I2CSlave id, u8 reg, u8 *bufp, size_t size )
{
    int i;
    int r;
    int error;
    u8  *ptr;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        ptr = bufp;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle( reg ) == FALSE)                error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        for ( i=0; error==0 && i<(size-1); i++ )
        {
            *ptr++ = I2Ci_WaitReceiveMiddle();
        }
        if (error == 0)
        {
            *ptr++ = I2Ci_WaitReceiveLast();
            break;
        }
        else
        {
            (void)I2Ci_WaitReceiveLast();
        }
    }
    return error ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_ReadRegistersSC

  Description:  get value from decive registers through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ReadRegistersSC( I2CSlave id, u8 reg, u8 *bufp, size_t size )
{
    int i;
    int r;
    int error;
    u8  *ptr;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        ptr = bufp;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendLast( reg ) == FALSE)                  error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        for ( i=0; error==0 && i<(size-1); i++ )
        {
            *ptr++ = I2Ci_WaitReceiveMiddle();
        }
        if (error == 0)
        {
            *ptr++ = I2Ci_WaitReceiveLast();
            break;
        }
        else
        {
            (void)I2Ci_WaitReceiveLast();
        }
    }
    return error ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_VerifyRegisters

  Description:  get and verify value from decive registers through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegisters( I2CSlave id, u8 reg, const u8 *bufp, size_t size )
{
    int i;
    int r;
    int error;
    const u8 *ptr;
    BOOL result;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        ptr = bufp;
        result = TRUE;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle( reg ) == FALSE)                error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        for ( i=0; error==0 && result!=FALSE && i<(size-1); i++ )
        {
            if (*ptr++ != I2Ci_WaitReceiveMiddle()) {
                result = FALSE;
            }
        }
        if (*ptr++ != I2Ci_WaitReceiveLast())
        {
            result = FALSE;
        }
        if (error == 0) break;
    }
    return error ? FALSE : (result ? TRUE : FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_VerifyRegistersSC

  Description:  get and verify value from decive registers through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegistersSC( I2CSlave id, u8 reg, const u8 *bufp, size_t size )
{
    int i;
    int r;
    int error;
    const u8 *ptr;
    BOOL result;
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        ptr = bufp;
        result = TRUE;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendLast( reg ) == FALSE)                  error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        for ( i=0; error==0 && result!=FALSE && i<(size-1); i++ )
        {
            if (*ptr++ != I2Ci_WaitReceiveMiddle()) {
                result = FALSE;
            }
        }
        if (*ptr++ != I2Ci_WaitReceiveLast())
        {
            result = FALSE;
        }
        if (error == 0) break;
    }
    return error ? FALSE : (result ? TRUE : FALSE);
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
void I2Ci_EnableInterrupt( void )
{
    OS_EnableIrqMask( OS_IE_I2C );
}

/*---------------------------------------------------------------------------*
  Name:         I2Ci_DisableInterrupt

  Description:  disable I2C interrupt for each device.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void I2Ci_DisableInterrupt( void )
{
    OS_DisableIrqMask( OS_IE_I2C );
}
#endif

