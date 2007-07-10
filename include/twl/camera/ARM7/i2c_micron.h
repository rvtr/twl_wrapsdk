/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera
  File:     i2c_micron.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CAMERA_I2C_MICRON_H_
#define TWL_CAMERA_I2C_MICRON_H_

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
  Name:         CAMERA_M_WriteRegister

  Description:  set value to decive register through I2C_.

  Arguments:    camera  : one of CameraSelect
                reg     : decive register
                data    : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_M_WriteRegister( CameraSelect camera, u16 reg, u16 data )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2Ci_WriteRegister16( I2C_SLAVE_CAMERA_MICRON_IN, reg, data );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2Ci_WriteRegister16( I2C_SLAVE_CAMERA_MICRON_OUT, reg, data );
    }
    return (rIn && rOut);
}
static inline BOOL CAMERA_M_WriteRegister( CameraSelect camera, u16 reg, u16 data )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2C_WriteRegister16( I2C_SLAVE_CAMERA_MICRON_IN, reg, data );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2C_WriteRegister16( I2C_SLAVE_CAMERA_MICRON_OUT, reg, data );
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_ReadRegister

  Description:  get value from decive register through I2C_.

  Arguments:    camera  : one of CameraSelect w/o BOTH
                reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline u16 CAMERAi_M_ReadRegister( CameraSelect camera, u16 reg )
{
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2Ci_ReadRegister16( I2C_SLAVE_CAMERA_MICRON_IN, reg );
    case CAMERA_SELECT_OUT:
        return I2Ci_ReadRegister16( I2C_SLAVE_CAMERA_MICRON_OUT, reg );
    }
    return FALSE;
}
static inline u16 CAMERA_M_ReadRegister( CameraSelect camera, u16 reg )
{
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2C_ReadRegister16( I2C_SLAVE_CAMERA_MICRON_IN, reg );
    case CAMERA_SELECT_OUT:
        return I2C_ReadRegister16( I2C_SLAVE_CAMERA_MICRON_OUT, reg );
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_WriteRegisters

  Description:  set value to decive registers through I2C_.

  Arguments:    camera  : one of CameraSelect
                reg      : decive register
                bufp     : data array to be written
                size     : data size

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_M_WriteRegisters( CameraSelect camera, u16 reg, const u16 *bufp, size_t size )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_WriteRegisters16( I2C_SLAVE_CAMERA_MICRON_IN, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        rIn = (size == 0 ? TRUE : FALSE);
#else
        rIn = I2Ci_WriteRegisters16( I2C_SLAVE_CAMERA_MICRON_IN, reg, bufp, size );
#endif
    }
    if (camera & CAMERA_SELECT_OUT)
    {
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_WriteRegisters16( I2C_SLAVE_CAMERA_MICRON_OUT, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        rOut = (size == 0 ? TRUE : FALSE);
#else
        rOut = I2Ci_WriteRegisters16( I2C_SLAVE_CAMERA_MICRON_OUT, reg, bufp, size );
#endif
    }
    return (rIn && rOut);
}
static inline BOOL CAMERA_M_WriteRegisters( CameraSelect camera, u16 reg, const u16 *bufp, size_t size )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
        (void)I2C_Lock();
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_WriteRegisters16( I2C_SLAVE_CAMERA_MICRON_IN, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        (void)I2C_Unlock();
        rIn = (size == 0 ? TRUE : FALSE);
#else
        rOut = I2C_WriteRegisters16( I2C_SLAVE_CAMERA_MICRON_IN, reg, bufp, size );
#endif
    }
    if (camera & CAMERA_SELECT_OUT)
    {
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
        (void)I2C_Lock();
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_WriteRegisters16( I2C_SLAVE_CAMERA_MICRON_OUT, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        (void)I2C_Unlock();
        rOut = (size == 0 ? TRUE : FALSE);
#else
        rOut = I2C_WriteRegisters16( I2C_SLAVE_CAMERA_MICRON_OUT, reg, bufp, size );
#endif
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_ReadRegisters

  Description:  get value from decive registers through I2C_.

  Arguments:    camera  : one of CameraSelect w/o BOTH
                reg      : decive register
                bufp     : data array to be read
                size     : data size

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_M_ReadRegisters( CameraSelect camera, u16 reg, u16 *bufp, size_t size )
{
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_ReadRegisters16( I2C_SLAVE_CAMERA_MICRON_IN, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        break;
    case CAMERA_SELECT_OUT:
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_ReadRegisters16( I2C_SLAVE_CAMERA_MICRON_OUT, reg++, bufp++, 1 ) )
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
        return I2Ci_ReadRegisters16( I2C_SLAVE_CAMERA_MICRON_IN, reg, bufp, size );
    case CAMERA_SELECT_OUT:
        return I2Ci_ReadRegisters16( I2C_SLAVE_CAMERA_MICRON_OUT, reg, bufp, size );
    }
    return FALSE;
#endif
}
static inline BOOL CAMERA_M_ReadRegisters( CameraSelect camera, u16 reg, u16 *bufp, size_t size )
{
#ifdef CAMERA_DOES_NOT_SUPPORT_MULTIPLE_IO
    (void)I2C_Lock();
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_ReadRegisters16( I2C_SLAVE_CAMERA_MICRON_IN, reg++, bufp++, 1 ) )
            {
                break;
            }
            size--;
        }
        break;
    case CAMERA_SELECT_OUT:
        while ( size > 0 )
        {
            if ( FALSE == I2Ci_ReadRegisters16( I2C_SLAVE_CAMERA_MICRON_OUT, reg++, bufp++, 1 ) )
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
        return I2C_ReadRegisters16( I2C_SLAVE_CAMERA_MICRON_IN, reg, bufp, size );
    case CAMERA_SELECT_OUT:
        return I2C_ReadRegisters16( I2C_SLAVE_CAMERA_MICRON_OUT, reg, bufp, size );
    }
    return FALSE;
#endif
}

//================================================================================
//        I2C_ BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_SetParams

  Description:  set control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_M_SetParams( CameraSelect camera, u16 reg, u16 setBits, u16 maskBits )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2Ci_SetParams16( I2C_SLAVE_CAMERA_MICRON_IN, reg, setBits, maskBits );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2Ci_SetParams16( I2C_SLAVE_CAMERA_MICRON_OUT, reg, setBits, maskBits );
    }
    return (rIn && rOut);
}
static inline BOOL CAMERA_M_SetParams( CameraSelect camera, u16 reg, u16 setBits, u16 maskBits )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2C_SetParams16( I2C_SLAVE_CAMERA_MICRON_IN, reg, setBits, maskBits );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2C_SetParams16( I2C_SLAVE_CAMERA_MICRON_OUT, reg, setBits, maskBits );
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_SetFlags

  Description:  set control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_M_SetFlags( CameraSelect camera, u16 reg, u16 setBits )
{
    return CAMERAi_M_SetParams( camera, reg, setBits, setBits );
}
static inline BOOL CAMERA_M_SetFlags( CameraSelect camera, u16 reg, u16 setBits )
{
    return CAMERA_M_SetParams( camera, reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_ClearFlags

  Description:  clear control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                clrBits  : bits to clear

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_M_ClearFlags( CameraSelect camera, u16 reg, u16 clrBits )
{
    return CAMERAi_M_SetParams( camera, reg, 0, clrBits );
}
static inline BOOL CAMERA_M_ClearFlags( CameraSelect camera, u16 reg, u16 clrBits )
{
    return CAMERA_M_SetParams( camera, reg, 0, clrBits );
}


//================================================================================
//        I2C_ API
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CInit(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CStandby

  Description:  standby or resume CAMERA

  Arguments:    camera  : one of CameraSelect
                standby : TRUE if goto standby mode

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CStandby(CameraSelect camera, BOOL standby);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CResize

  Description:  resize CAMERA

  Arguments:    camera  : one of CameraSelect
                width   : width of output image
                height  : height of output image

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CResize(CameraSelect camera, u16 width, u16 height);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CPreSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CPreSleep(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CPostSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CPostSleep(CameraSelect camera);


#ifdef _cplusplus
} /* extern "C" */
#endif

/* TWL_CAMERA_I2C_MICRON_H_ */
#endif
