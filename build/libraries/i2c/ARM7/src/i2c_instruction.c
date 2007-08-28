/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libralies - i2c
  File:     i2c_instruction.c

  Copyright 2007 Nintendo.  All rights reserved.

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

//#define PRINT_DEBUG
//#define PRINT_DEBUG_MINI  // rough version

#ifdef PRINT_DEBUG
#include <nitro/os/common/printf.h>
#define DBG_PRINTF OS_TPrintf
#undef PRINT_DEBUG_MINI     // because of the alternative option
#else
#define DBG_PRINTF( ... )  ((void)0)
#endif
#ifdef PRINT_DEBUG_MINI
#include <nitro/os/common/printf.h>
#define DBG_PRINT_FUNC()    OS_TPrintf("%s(0x%02X, 0x%02X, ...);\n", __func__, deviceAddrTable[id], reg)
#define DBG_PRINT_ERR()     OS_TPrintf("  Failed(%d) @ %d\n", error, r)
#else
#define DBG_PRINT_FUNC()    ((void)0)
#define DBG_PRINT_ERR()     ((void)0)
#endif

#define RETRY_COUNT     8

static const u8 deviceAddrTable[I2C_SLAVE_NUM] = {
                                                I2C_ADDR_CODEC,
                                                I2C_ADDR_CAMERA_MICRON_IN,
                                                I2C_ADDR_CAMERA_MICRON_OUT,
                                                I2C_ADDR_CAMERA_SHARP_IN,
                                                I2C_ADDR_CAMERA_SHARP_OUT,
                                                I2C_ADDR_MICRO_CONTROLLER,
                                                I2C_ADDR_DEBUG_LED,
                                            };

/*static const*/ s32 I2CSlowRateTable[I2C_SLAVE_NUM] = {
                                                0,      // CODEC
                                                0,      // CAMERA_MICRON_IN
                                                0,      // CAMERA_MICRON_OUT
                                                0,      // CAMERA_SHARP_IN
                                                0,      // CAMERA_SHARP_OUT
                                                0x90,   // MICRO_CONTROLLER
                                                0,      // DEBUG_LED
                                            };

static OSMutex mutex;
static BOOL isInitialized = FALSE;

static BOOL slowRate = 0;

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

static inline void I2Ci_WaitEx( void )  // support slowRate
{
    I2Ci_Wait();
    SVC_WaitByLoop(slowRate);
}

static inline void I2Ci_StopEx( I2CReadWrite rw )   // support slowRate
{
    if (slowRate)
    {
        I2Ci_StopPhase1(rw);
        I2Ci_Wait();
        SVC_WaitByLoop(slowRate);
        I2Ci_StopPhase2();
    }
    else
    {
        I2Ci_Stop(rw);
    }
}

static inline void I2Ci_SetData( u8 data )
{
    DBG_PRINTF("%02X", data);
    reg_EXI_I2CD = data;
}


static inline u8 I2Ci_GetData( void )
{
    DBG_PRINTF("(%02X)", reg_EXI_I2CD);
    return reg_EXI_I2CD;
}

static inline BOOL I2Ci_GetResult( void )
{
    I2Ci_WaitEx();
    DBG_PRINTF("%c", (reg_EXI_I2CCNT & REG_EXI_I2CCNT_ACK_MASK) ? '.' : '*');
    return (BOOL)((reg_EXI_I2CCNT & REG_EXI_I2CCNT_ACK_MASK) >> REG_EXI_I2CCNT_ACK_SHIFT);
}
static inline BOOL I2Ci_SendStart( I2CSlave id )
{
    DBG_PRINTF("\n");
    slowRate = I2CSlowRateTable[id];
    I2Ci_Wait();
    I2Ci_SetData( (u8)(deviceAddrTable[id] | I2C_WRITE) );
    I2Ci_Start();
    return I2Ci_GetResult();
}

static inline BOOL I2Ci_SendMiddle( u8 data )
{
    I2Ci_WaitEx();
    I2Ci_SetData( data );
    I2Ci_Continue( I2C_WRITE );
    return I2Ci_GetResult();
}

static inline BOOL I2Ci_SendLast( u8 data )
{
    I2Ci_WaitEx();
    I2Ci_SetData( data );
    I2Ci_StopEx( I2C_WRITE );
    return I2Ci_GetResult();
}

static inline BOOL I2Ci_ReceiveStart( I2CSlave id )
{
    I2Ci_WaitEx();
    I2Ci_SetData( (u8)(deviceAddrTable[id] | I2C_READ) );
    I2Ci_Start();
    return I2Ci_GetResult();
}

static inline void I2Ci_ReceiveMiddle( void )
{
    I2Ci_WaitEx();
    I2Ci_Continue( I2C_READ );
}

static inline void I2Ci_ReceiveLast( void )
{
    I2Ci_WaitEx();
    I2Ci_StopEx( I2C_READ );
}

static inline u8 I2Ci_WaitReceiveMiddle( void )
{
    I2Ci_ReceiveMiddle();
    I2Ci_WaitEx();
    return I2Ci_GetData();
}

static inline u8 I2Ci_WaitReceiveLast( void )
{
    I2Ci_ReceiveLast();
    I2Ci_WaitEx();
    return I2Ci_GetData();
}

// 16 bit sequence

static inline BOOL I2Ci_SendMiddle16( u16 data )
{
    BOOL rHi = I2Ci_SendMiddle( (u8)(data >> 8) );
    BOOL rLo = I2Ci_SendMiddle( (u8)(data & 0xFF) );
    return (rHi && rLo);
}

static inline BOOL I2Ci_SendLast16( u16 data )
{
    BOOL rHi = I2Ci_SendMiddle( (u8)(data >> 8) );
    BOOL rLo = I2Ci_SendLast( (u8)(data & 0xFF) );
    return (rHi && rLo);
}

static inline u16 I2Ci_WaitReceiveMiddle16( void )
{
    return (u16)((I2Ci_WaitReceiveMiddle() << 8) | I2Ci_WaitReceiveMiddle());
}

static inline u16 I2Ci_WaitReceiveLast16( void )
{
    return (u16)((I2Ci_WaitReceiveMiddle() << 8) | I2Ci_WaitReceiveLast());
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
        isInitialized = TRUE;
        OS_InitMutex(&mutex);
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

/*---------------------------------------------------------------------------*
  Name:         I2C_SetParams16

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_SetParams16( I2CSlave id, u16 reg, u16 setBits, u16 maskBits )
{
    u16      tmp;
    tmp = I2Ci_ReadRegister16( id, reg );
    tmp &= ~maskBits;
    setBits &= maskBits;
    tmp |= setBits;
    return I2Ci_WriteRegister16( id, reg, tmp );
}
BOOL I2C_SetParams16( I2CSlave id, u16 reg, u16 setBits, u16 maskBits )
{
    BOOL result;
    (void)I2C_Lock();
    result = I2Ci_SetParams16( id, reg, setBits, maskBits );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         I2C_SetFlags16

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_SetFlags16( I2CSlave id, u16 reg, u16 setBits )
{
    return I2Ci_SetParams16( id, reg, setBits, setBits );
}
BOOL I2C_SetFlags16( I2CSlave id, u16 reg, u16 setBits )
{
    return I2C_SetParams16( id, reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         I2C_ClearFlags16

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                clrBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ClearFlags16( I2CSlave id, u16 reg, u16 clrBits )
{
    return I2Ci_SetParams16( id, reg, 0, clrBits );
}
BOOL I2C_ClearFlags16( I2CSlave id, u16 reg, u16 clrBits )
{
    return I2C_SetParams16( id, reg, 0, clrBits );
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
    DBG_PRINT_FUNC();
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle( reg ) == FALSE)                error++;
        if (I2Ci_SendLast( data ) == FALSE)                 error++;
        if (error == 0) break;
        DBG_PRINT_ERR();
    }
    return error ? FALSE : TRUE;
}
/*---------------------------------------------------------------------------*
  Name:         I2Ci_WriteRegister16

  Description:  set value to decive register through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_WriteRegister16( I2CSlave id, u16 reg, u16 data )
{
    int r;
    int error;
    DBG_PRINT_FUNC();
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle16( reg ) == FALSE)              error++;
        if (I2Ci_SendLast16( data ) == FALSE)               error++;
        if (error == 0) break;
        DBG_PRINT_ERR();
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
    DBG_PRINT_FUNC();
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle( reg ) == FALSE)                error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        data = I2Ci_WaitReceiveLast();
        if (error == 0) break;
        DBG_PRINT_ERR();
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
    DBG_PRINT_FUNC();
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendLast( reg ) == FALSE)                  error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        data = I2Ci_WaitReceiveLast();
        if (error == 0) break;
        DBG_PRINT_ERR();
    }
    return error ? (u8)0xee : data;
}
/*---------------------------------------------------------------------------*
  Name:         I2Ci_ReadRegister16

  Description:  get value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
u16 I2Ci_ReadRegister16( I2CSlave id, u16 reg )
{
    int r;
    u16 data;
    int error;
    DBG_PRINT_FUNC();
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle16( reg ) == FALSE)              error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        data = I2Ci_WaitReceiveLast16();
        if (error == 0) break;
        DBG_PRINT_ERR();
    }
    return error ? (u16)0xeeee : data;
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
    DBG_PRINT_FUNC();
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
        DBG_PRINT_ERR();
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
    DBG_PRINT_FUNC();
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
        DBG_PRINT_ERR();
    }
    return error ? FALSE : (result ? TRUE : FALSE);
}
/*---------------------------------------------------------------------------*
  Name:         I2Ci_VerifyRegister16

  Description:  get and verify value from decive register through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegister16( I2CSlave id, u16 reg, u16 data )
{
    int r;
    int error;
    BOOL result;
    DBG_PRINT_FUNC();
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        result = TRUE;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle16( reg ) == FALSE)              error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        if (data != I2Ci_WaitReceiveLast16())
        {
            result = FALSE;
        }
        if (error == 0) break;
        DBG_PRINT_ERR();
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
    DBG_PRINT_FUNC();
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
        DBG_PRINT_ERR();
    }
    return error ? FALSE : TRUE;
}
/*---------------------------------------------------------------------------*
  Name:         I2Ci_WriteRegisters16

  Description:  set value to decive registers through I2C.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL I2Ci_WriteRegisters16( I2CSlave id, u16 reg, const u16 *bufp, size_t size )
{
    int i;
    int r;
    int error;
    const u16 *ptr;
    DBG_PRINT_FUNC();
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        ptr = bufp;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle16( reg ) == FALSE)              error++;
        for ( i=0; error==0 && i<(size-1); i++ )
        {
            if (I2Ci_SendMiddle16( *ptr++ ) == FALSE)       error++;
        }
        if (I2Ci_SendLast16( *ptr++ ) == FALSE)             error++;
        if (error == 0) break;
        DBG_PRINT_ERR();
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
    DBG_PRINT_FUNC();
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
        DBG_PRINT_ERR();
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
    DBG_PRINT_FUNC();
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
        DBG_PRINT_ERR();
    }
    return error ? FALSE : TRUE;
}
/*---------------------------------------------------------------------------*
  Name:         I2Ci_ReadRegisters16

  Description:  get value from decive registers through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_ReadRegisters16( I2CSlave id, u16 reg, u16 *bufp, size_t size )
{
    int i;
    int r;
    int error;
    u16  *ptr;
    DBG_PRINT_FUNC();
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        ptr = bufp;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle16( reg ) == FALSE)              error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        for ( i=0; error==0 && i<(size-1); i++ )
        {
            *ptr++ = I2Ci_WaitReceiveMiddle16();
        }
        if (error == 0)
        {
            *ptr++ = I2Ci_WaitReceiveLast16();
            break;
        }
        else
        {
            (void)I2Ci_WaitReceiveLast16();
        }
        DBG_PRINT_ERR();
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
    DBG_PRINT_FUNC();
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
        DBG_PRINT_ERR();
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
    DBG_PRINT_FUNC();
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
        DBG_PRINT_ERR();
    }
    return error ? FALSE : (result ? TRUE : FALSE);
}
/*---------------------------------------------------------------------------*
  Name:         I2Ci_VerifyRegisters16

  Description:  get and verify value from decive registers through I2C.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
BOOL I2Ci_VerifyRegisters16( I2CSlave id, u16 reg, const u16 *bufp, size_t size )
{
    int i;
    int r;
    int error;
    const u16 *ptr;
    BOOL result;
    DBG_PRINT_FUNC();
    for (r = 0; r < RETRY_COUNT; r++)
    {
        error = 0;
        ptr = bufp;
        result = TRUE;
        if (I2Ci_SendStart( id ) == FALSE)                  error++;
        if (I2Ci_SendMiddle16( reg ) == FALSE)              error++;
        if (I2Ci_ReceiveStart( id ) == FALSE)               error++;
        for ( i=0; error==0 && result!=FALSE && i<(size-1); i++ )
        {
            if (*ptr++ != I2Ci_WaitReceiveMiddle16()) {
                result = FALSE;
            }
        }
        if (*ptr++ != I2Ci_WaitReceiveLast16())
        {
            result = FALSE;
        }
        if (error == 0) break;
        DBG_PRINT_ERR();
    }
    return error ? FALSE : (result ? TRUE : FALSE);
}
