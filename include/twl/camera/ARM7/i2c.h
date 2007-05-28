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

#ifdef _cplusplus
extern "C" {
#endif

//================================================================================
//        I2C_ ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERA_WriteRegister

  Description:  set value to decive register through I2C_.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_WriteRegister( u8 reg, u8 data )
{
    return I2Ci_WriteRegister( I2C_SLAVE_CAMERA, reg, data );
}
static inline BOOL CAMERA_WriteRegister( u8 reg, u8 data )
{
    return I2C_WriteRegister( I2C_SLAVE_CAMERA, reg, data );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ReadRegister

  Description:  get value from decive register through I2C_.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline u8 CAMERAi_ReadRegister( u8 reg )
{
    return I2Ci_ReadRegisterSC( I2C_SLAVE_CAMERA, reg );
}
static inline u8 CAMERA_ReadRegister( u8 reg )
{
    return I2C_ReadRegisterSC( I2C_SLAVE_CAMERA, reg );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_WriteRegisters

  Description:  set value to decive registers through I2C_.

  Arguments:    reg      : decive register
                data     : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_WriteRegisters( u8 reg, const u8 *bufp, size_t size )
{
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
    while ( size > 0 )
    {
        if ( FALSE == I2Ci_WriteRegisters( I2C_SLAVE_CAMERA, reg++, bufp++, 1 ) )
        {
            break;
        }
        size--;
    }
    return (size == 0 ? TRUE : FALSE);
#else
    return I2Ci_WriteRegisters( I2C_SLAVE_CAMERA, reg, bufp, size );
#endif
}
static inline BOOL CAMERA_WriteRegisters( u8 reg, const u8 *bufp, size_t size )
{
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
    (void)I2C_Lock();
    while ( size > 0 )
    {
        if ( FALSE == I2Ci_WriteRegisters( I2C_SLAVE_CAMERA, reg++, bufp++, 1 ) )
        {
            break;
        }
        size--;
    }
    (void)I2C_Unlock();
    return (size == 0 ? TRUE : FALSE);
#else
    return I2C_WriteRegisters( I2C_SLAVE_CAMERA, reg, bufp, size );
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ReadRegisters

  Description:  get value from decive registers through I2C_.

  Arguments:    reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_ReadRegisters( u8 reg, u8 *bufp, size_t size )
{
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
    while ( size > 0 )
    {
        if ( FALSE == I2Ci_ReadRegisters( I2C_SLAVE_CAMERA, reg++, bufp++, 1 ) )
        {
            break;
        }
        size--;
    }
    return (size == 0 ? TRUE : FALSE);
#else
    return I2Ci_ReadRegistersSC( I2C_SLAVE_CAMERA, reg, bufp, size );
#endif
}
static inline BOOL CAMERA_ReadRegisters( u8 reg, u8 *bufp, size_t size )
{
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
    (void)I2C_Lock();
    while ( size > 0 )
    {
        if ( FALSE == I2Ci_ReadRegisters( I2C_SLAVE_CAMERA, reg++, bufp++, 1 ) )
        {
            break;
        }
        size--;
    }
    (void)I2C_Unlock();
    return (size == 0 ? TRUE : FALSE);
#else
    return I2C_ReadRegistersSC( I2C_SLAVE_CAMERA, reg, bufp, size );
#endif
}

//================================================================================
//        I2C_ BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_SetParams( u8 reg, u8 setBits, u8 maskBits )
{
    return I2Ci_SetParams( I2C_SLAVE_CAMERA, reg, setBits, maskBits );
}
static inline BOOL CAMERA_SetParams( u8 reg, u8 setBits, u8 maskBits )
{
    return I2C_SetParams( I2C_SLAVE_CAMERA, reg, setBits, maskBits );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_SetFlags( u8 reg, u8 setBits )
{
    return CAMERAi_SetParams( reg, setBits, setBits );
}
static inline BOOL CAMERA_SetFlags( u8 reg, u8 setBits )
{
    return CAMERA_SetParams( reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ClearFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                clrBits  : bits to clear

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_ClearFlags( u8 reg, u8 clrBits )
{
    return CAMERAi_SetParams( reg, 0, clrBits );
}
static inline BOOL CAMERA_ClearFlags( u8 reg, u8 clrBits )
{
    return CAMERA_SetParams( reg, 0, clrBits );
}

//================================================================================
//        I2C_ API
//================================================================================
#define CAMERA_I2CSetCropping(x, y, w, h)   CAMERA_I2CSetCroppingParams(w, h)

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSetFlipMode

  Description:  set CAMERA's flip mode

  Arguments:    mode            one of CameraFlipMode to apply

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CSetFlipMode(CameraFlipMode mode);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSetSpecialMode

  Description:  set CAMERA's special mode

  Arguments:    mode            one of CameraSpecialMode to apply

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CSetSpecialMode(CameraSpecialMode mode);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CSetCroppingParams

  Description:  set CAMERA_ cropping parameters.

  Arguments:    width           width of image (up to 640)
                height          height of image (up to 480)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CSetCroppingParams(u16 width, u16 height);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CGetCroppingParams

  Description:  get current CAMERA_ cropping parameters.

  Arguments:    pWidth          address to store the width
                pHeight         address to store the height

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CGetCroppingParams(u16 *pWidth, u16 *pHeight);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPause

  Description:  pause to send frame

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CPause(void)
{
    BOOL result;
    (void)I2C_Lock();
    result = CAMERAi_WriteRegister( 0xef, 0x00 ) &&
            CAMERAi_ClearFlags( 0xde, 0x04 );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResume

  Description:  resume from pause state

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CResume(void)
{
    BOOL result;
    (void)I2C_Lock();
    result = CAMERAi_WriteRegister( 0xef, 0x00 ) &&
            CAMERAi_SetFlags( 0xde, 0x04 );
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPreSleep

  Description:  pre-sleep

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CPreSleep(void)
{
    BOOL result;
    (void)I2C_Lock();
    // not impremented yet
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPostSleep

  Description:  post-sleep

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CPostSleep(void)
{
    BOOL result;
    (void)I2C_Lock();
    // not impremented yet
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPreset

  Description:  preset CAMERA registers

  Arguments:    preset          one of CameraPreset

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CPreset(CameraPreset preset);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize CAMERA

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERA_I2CInit(void)
{
    if (CAMERA_I2CPreset(CAMERA_PRESET_DEFAULT) == FALSE) {
        return FALSE;
    }
    if (CAMERA_I2CSetFlipMode(CAMERA_FLIPMODE_DEFAULT) == FALSE) {
        return FALSE;
    }
    return TRUE;
}


#ifdef _cplusplus
} /* extern "C" */
#endif

/* TWL_CAMERA_CAMERA_I2CH_ */
#endif
