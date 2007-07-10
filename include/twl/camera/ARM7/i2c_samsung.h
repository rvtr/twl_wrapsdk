/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera
  File:     camera_i2c.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CAMERA_CAMERA_I2CH_
#define TWL_CAMERA_CAMERA_I2CH_

#include <twl/types.h>
#include <twl/i2c/ARM7/i2c.h>
#include <twl/camera/common/types.h>

#define CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO

#if 0
    Write����NONE���w�肷���TRUE�ŕԂ�ARead����NONE���w�肷���FALSE�ŕԂ�B
    Write����BOTH���w�肷��Ɨ����ɏ������݁ARead����BOTH���w�肷���FALSE�ŕԂ�B
    SetParams����Write�Ɠ����d�l�B
#endif

#ifdef _cplusplus
extern "C" {
#endif

//================================================================================
//        I2C_ ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERA_WriteRegister

  Description:  set value to decive register through I2C_.

  Arguments:    camera  : one of CameraSelect
                reg     : decive register
                data    : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_WriteRegister( CameraSelect camera, u8 reg, u8 data )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2Ci_WriteRegister( I2C_SLAVE_CAMERA_IN, reg, data );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2Ci_WriteRegister( I2C_SLAVE_CAMERA_OUT, reg, data );
    }
    return (rIn && rOut);
}
static inline BOOL CAMERA_WriteRegister( CameraSelect camera, u8 reg, u8 data )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2C_WriteRegister( I2C_SLAVE_CAMERA_IN, reg, data );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2C_WriteRegister( I2C_SLAVE_CAMERA_OUT, reg, data );
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ReadRegister

  Description:  get value from decive register through I2C_.

  Arguments:    camera  : one of CameraSelect w/o BOTH
                reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline u8 CAMERAi_ReadRegister( CameraSelect camera, u8 reg )
{
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2Ci_ReadRegisterSC( I2C_SLAVE_CAMERA_IN, reg );
    case CAMERA_SELECT_OUT:
        return I2Ci_ReadRegisterSC( I2C_SLAVE_CAMERA_OUT, reg );
    }
    return FALSE;
}
static inline u8 CAMERA_ReadRegister( CameraSelect camera, u8 reg )
{
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2C_ReadRegisterSC( I2C_SLAVE_CAMERA_IN, reg );
    case CAMERA_SELECT_OUT:
        return I2C_ReadRegisterSC( I2C_SLAVE_CAMERA_OUT, reg );
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_WriteRegisters

  Description:  set value to decive registers through I2C_.

  Arguments:    camera  : one of CameraSelect
                reg      : decive register
                bufp     : data array to be written
                size     : data size

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_WriteRegisters( CameraSelect camera, u8 reg, const u8 *bufp, size_t size )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_WriteRegisters( I2C_SLAVE_CAMERA_IN, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        rIn = (size == 0 ? TRUE : FALSE);
#else
        rIn = I2Ci_WriteRegisters( I2C_SLAVE_CAMERA_IN, reg, bufp, size );
#endif
    }
    if (camera & CAMERA_SELECT_OUT)
    {
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_WriteRegisters( I2C_SLAVE_CAMERA_OUT, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        rOut = (size == 0 ? TRUE : FALSE);
#else
        rOut = I2Ci_WriteRegisters( I2C_SLAVE_CAMERA_OUT, reg, bufp, size );
#endif
    }
    return (rIn && rOut);
}
static inline BOOL CAMERA_WriteRegisters( CameraSelect camera, u8 reg, const u8 *bufp, size_t size )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
        (void)I2C_Lock();
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_WriteRegisters( I2C_SLAVE_CAMERA_IN, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        (void)I2C_Unlock();
        rIn = (size == 0 ? TRUE : FALSE);
#else
        rOut = I2C_WriteRegisters( I2C_SLAVE_CAMERA_IN, reg, bufp, size );
#endif
    }
    if (camera & CAMERA_SELECT_OUT)
    {
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
        (void)I2C_Lock();
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_WriteRegisters( I2C_SLAVE_CAMERA_OUT, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        (void)I2C_Unlock();
        rOut = (size == 0 ? TRUE : FALSE);
#else
        rOut = I2C_WriteRegisters( I2C_SLAVE_CAMERA_OUT, reg, bufp, size );
#endif
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ReadRegisters

  Description:  get value from decive registers through I2C_.

  Arguments:    camera  : one of CameraSelect w/o BOTH
                reg      : decive register
                bufp     : data array to be read
                size     : data size

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_ReadRegisters( CameraSelect camera, u8 reg, u8 *bufp, size_t size )
{
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_ReadRegistersSC( I2C_SLAVE_CAMERA_IN, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        break;
    case CAMERA_SELECT_OUT:
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_ReadRegistersSC( I2C_SLAVE_CAMERA_OUT, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        break;
    }
    return (size == 0 ? TRUE : FALSE);
#else
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2Ci_ReadRegistersSC( I2C_SLAVE_CAMERA_IN, reg, bufp, size );
    case CAMERA_SELECT_OUT:
        return I2Ci_ReadRegistersSC( I2C_SLAVE_CAMERA_OUT, reg, bufp, size );
    }
    return FALSE;
#endif
}
static inline BOOL CAMERA_ReadRegisters( CameraSelect camera, u8 reg, u8 *bufp, size_t size )
{
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
    (void)I2C_Lock();
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_ReadRegistersSC( I2C_SLAVE_CAMERA_IN, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        break;
    case CAMERA_SELECT_OUT:
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_ReadRegistersSC( I2C_SLAVE_CAMERA_OUT, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        break;
    }
    (void)I2C_Unlock();
    return (size == 0 ? TRUE : FALSE);
#else
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2C_ReadRegistersSC( I2C_SLAVE_CAMERA_IN, reg, bufp, size );
    case CAMERA_SELECT_OUT:
        return I2C_ReadRegistersSC( I2C_SLAVE_CAMERA_OUT, reg, bufp, size );
    }
    return FALSE;
#endif
}

//================================================================================
//        I2C_ BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetParams

  Description:  set control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_SetParams( CameraSelect camera, u8 reg, u8 setBits, u8 maskBits )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2Ci_SetParams( I2C_SLAVE_CAMERA_IN, reg, setBits, maskBits );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2Ci_SetParams( I2C_SLAVE_CAMERA_OUT, reg, setBits, maskBits );
    }
    return (rIn && rOut);
}
static inline BOOL CAMERA_SetParams( CameraSelect camera, u8 reg, u8 setBits, u8 maskBits )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2C_SetParams( I2C_SLAVE_CAMERA_IN, reg, setBits, maskBits );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2C_SetParams( I2C_SLAVE_CAMERA_OUT, reg, setBits, maskBits );
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetFlags

  Description:  set control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_SetFlags( CameraSelect camera, u8 reg, u8 setBits )
{
    return CAMERAi_SetParams( camera, reg, setBits, setBits );
}
static inline BOOL CAMERA_SetFlags( CameraSelect camera, u8 reg, u8 setBits )
{
    return CAMERA_SetParams( camera, reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ClearFlags

  Description:  clear control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                clrBits  : bits to clear

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_ClearFlags( CameraSelect camera, u8 reg, u8 clrBits )
{
    return CAMERAi_SetParams( camera, reg, 0, clrBits );
}
static inline BOOL CAMERA_ClearFlags( CameraSelect camera, u8 reg, u8 clrBits )
{
    return CAMERA_SetParams( camera, reg, 0, clrBits );
}

//================================================================================
//        I2C_ API
//================================================================================
#define CAMERA_I2CSetCropping(c, x, y, w, h)   CAMERA_I2CSetCroppingParams(c, w, h)

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSetFlipMode

  Description:  set CAMERA's flip mode

  Arguments:    camera  : one of CameraSelect
                mode    : one of CameraFlipMode to apply

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CSetFlipMode(CameraSelect camera, CameraFlipMode mode);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSetSpecialMode

  Description:  set CAMERA's special mode

  Arguments:    camera  : one of CameraSelect
                mode    : one of CameraSpecialMode to apply

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CSetSpecialMode(CameraSelect camera, CameraSpecialMode mode);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSetCroppingParams

  Description:  set CAMERA_ cropping parameters.

  Arguments:    camera  : one of CameraSelect
                width   : width of image (up to 640)
                height  : height of image (up to 480)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CSetCroppingParams(CameraSelect camera, u16 width, u16 height);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CGetCroppingParams

  Description:  get current CAMERA_ cropping parameters.

  Arguments:    camera  : one of CameraSelect
                pWidth  : address to store the width
                pHeight : address to store the height

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CGetCroppingParams(CameraSelect camera, u16 *pWidth, u16 *pHeight);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPause

  Description:  pause to send frame

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CPause(CameraSelect camera)
{
    BOOL result;
    (void)I2C_Lock();
    result = CAMERAi_WriteRegister( camera, 0xef, 0x00 ) &&
            CAMERAi_ClearFlags( camera, 0xde, 0x04 );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResume

  Description:  resume from pause state

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CResume(CameraSelect camera)
{
    BOOL result;
    (void)I2C_Lock();
    result = CAMERAi_WriteRegister( camera, 0xef, 0x00 ) &&
            CAMERAi_SetFlags( camera, 0xde, 0x04 );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPreSleep

  Description:  pre-sleep

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CPreSleep(CameraSelect camera)
{
    BOOL result;
    (void)I2C_Lock();
    (void)camera;   // not impremented yet
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPostSleep

  Description:  post-sleep

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CPostSleep(CameraSelect camera)
{
    BOOL result;
    (void)I2C_Lock();
    (void)camera;   // not impremented yet
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPreset

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect
                preset  : one of CameraPreset

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CPreset(CameraSelect camera, CameraPreset preset);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CInit(CameraSelect camera)
{
    if (CAMERA_I2CPreset(camera, CAMERA_PRESET_DEFAULT) == FALSE) {
        return FALSE;
    }
    if (CAMERA_I2CSetFlipMode(camera, CAMERA_FLIPMODE_DEFAULT) == FALSE) {
        return FALSE;
    }
    return TRUE;
}


#ifdef _cplusplus
} /* extern "C" */
#endif

/* TWL_CAMERA_CAMERA_I2CH_ */
#endif
