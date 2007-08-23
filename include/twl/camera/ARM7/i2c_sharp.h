/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera
  File:     i2c_sharp.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CAMERA_I2C_SHARP_H_
#define TWL_CAMERA_I2C_SHARP_H_

#include <twl/i2c/ARM7/i2c.h>

#if 0
    Write時にNONEを指定するとTRUEで返り、Read時にNONEを指定するとFALSEで返る。
    Write時にBOTHを指定すると両方に書き込み、Read時にBOTHを指定するとFALSEで返る。
    SetParams等はWriteと同じ仕様。
#endif

#ifdef _cplusplus
extern "C" {
#endif

//================================================================================
//        I2C ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_WriteRegister

  Description:  set value to decive register through I2C.

  Arguments:    camera  : one of CameraSelect
                reg     : decive register
                data    : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_S_WriteRegister( CameraSelect camera, u8 reg, u8 data )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2Ci_WriteRegister( I2C_SLAVE_CAMERA_SHARP_IN, reg, data );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2Ci_WriteRegister( I2C_SLAVE_CAMERA_SHARP_OUT, reg, data );
    }
    return (rIn && rOut);
}
static inline BOOL CAMERA_S_WriteRegister( CameraSelect camera, u8 reg, u8 data )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2C_WriteRegister( I2C_SLAVE_CAMERA_SHARP_IN, reg, data );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2C_WriteRegister( I2C_SLAVE_CAMERA_SHARP_OUT, reg, data );
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_ReadRegister

  Description:  get value from decive register through I2C.

  Arguments:    camera  : one of CameraSelect w/o BOTH
                reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline u8 CAMERAi_S_ReadRegister( CameraSelect camera, u8 reg )
{
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2Ci_ReadRegister( I2C_SLAVE_CAMERA_SHARP_IN, reg );
    case CAMERA_SELECT_OUT:
        return I2Ci_ReadRegister( I2C_SLAVE_CAMERA_SHARP_OUT, reg );
    }
    return FALSE;
}
static inline u8 CAMERA_S_ReadRegister( CameraSelect camera, u8 reg )
{
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2C_ReadRegister( I2C_SLAVE_CAMERA_SHARP_IN, reg );
    case CAMERA_SELECT_OUT:
        return I2C_ReadRegister( I2C_SLAVE_CAMERA_SHARP_OUT, reg );
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_WriteRegisters

  Description:  set value to decive registers through I2C.

  Arguments:    camera  : one of CameraSelect
                reg      : decive register
                bufp     : data array to be written
                size     : data size

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_S_WriteRegisters( CameraSelect camera, u8 reg, const u8 *bufp, size_t size )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2Ci_WriteRegisters( I2C_SLAVE_CAMERA_SHARP_IN, reg, bufp, size );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2Ci_WriteRegisters( I2C_SLAVE_CAMERA_SHARP_OUT, reg, bufp, size );
    }
    return (rIn && rOut);
}
static inline BOOL CAMERA_S_WriteRegisters( CameraSelect camera, u8 reg, const u8 *bufp, size_t size )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rOut = I2C_WriteRegisters( I2C_SLAVE_CAMERA_SHARP_IN, reg, bufp, size );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2C_WriteRegisters( I2C_SLAVE_CAMERA_SHARP_OUT, reg, bufp, size );
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_ReadRegisters

  Description:  get value from decive registers through I2C.

  Arguments:    camera  : one of CameraSelect w/o BOTH
                reg      : decive register
                bufp     : data array to be read
                size     : data size

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_S_ReadRegisters( CameraSelect camera, u8 reg, u8 *bufp, size_t size )
{
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2Ci_ReadRegisters( I2C_SLAVE_CAMERA_SHARP_IN, reg, bufp, size );
    case CAMERA_SELECT_OUT:
        return I2Ci_ReadRegisters( I2C_SLAVE_CAMERA_SHARP_OUT, reg, bufp, size );
    }
    return FALSE;
}
static inline BOOL CAMERA_S_ReadRegisters( CameraSelect camera, u8 reg, u8 *bufp, size_t size )
{
    switch (camera)
    {
    case CAMERA_SELECT_IN:
        return I2C_ReadRegisters( I2C_SLAVE_CAMERA_SHARP_IN, reg, bufp, size );
    case CAMERA_SELECT_OUT:
        return I2C_ReadRegisters( I2C_SLAVE_CAMERA_SHARP_OUT, reg, bufp, size );
    }
    return FALSE;
}

//================================================================================
//        I2C BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_SetParams

  Description:  set control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_S_SetParams( CameraSelect camera, u8 reg, u8 setBits, u8 maskBits )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2Ci_SetParams( I2C_SLAVE_CAMERA_SHARP_IN, reg, setBits, maskBits );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2Ci_SetParams( I2C_SLAVE_CAMERA_SHARP_OUT, reg, setBits, maskBits );
    }
    return (rIn && rOut);
}
static inline BOOL CAMERA_S_SetParams( CameraSelect camera, u8 reg, u8 setBits, u8 maskBits )
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = I2C_SetParams( I2C_SLAVE_CAMERA_SHARP_IN, reg, setBits, maskBits );
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = I2C_SetParams( I2C_SLAVE_CAMERA_SHARP_OUT, reg, setBits, maskBits );
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_SetFlags

  Description:  set control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_S_SetFlags( CameraSelect camera, u8 reg, u8 setBits )
{
    return CAMERAi_S_SetParams( camera, reg, setBits, setBits );
}
static inline BOOL CAMERA_S_SetFlags( CameraSelect camera, u8 reg, u8 setBits )
{
    return CAMERA_S_SetParams( camera, reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_ClearFlags

  Description:  clear control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                clrBits  : bits to clear

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_S_ClearFlags( CameraSelect camera, u8 reg, u8 clrBits )
{
    return CAMERAi_S_SetParams( camera, reg, 0, clrBits );
}
static inline BOOL CAMERA_S_ClearFlags( CameraSelect camera, u8 reg, u8 clrBits )
{
    return CAMERA_S_SetParams( camera, reg, 0, clrBits );
}


//================================================================================
//        I2C API
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CInit(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CStandby

  Description:  goto standby

  Arguments:    camera  : one of CameraSelect (IN/OUT/BOTH) to goto standby

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CStandby(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CResume

  Description:  resume from standby

  Arguments:    camera  : one of CameraSelect (IN/OUT) to resume

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CResume(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CResumeBoth

  Description:  resume both CAMERAs, but only one will output

  Arguments:    camera  : one of CameraSelect (IN/OUT) to output

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CResumeBoth(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CResize

  Description:  resize CAMERA

  Arguments:    camera  : one of CameraSelect
                width   : width of output image
                height  : height of output image

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CResize(CameraSelect camera, u16 width, u16 height);


/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CFrameRate

  Description:  set CAMERA frame rate

  Arguments:    camera  : one of CameraSelect
                rate    : fps (0: auto)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CFrameRate(CameraSelect camera, int rate);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CEffect

  Description:  set CAMERA effect

  Arguments:    camera  : one of CameraSelect
                effect  : one of CameraEffect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CEffect(CameraSelect camera, CameraEffect effect);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CFlip

  Description:  set CAMERA flip/mirror

  Arguments:    camera  : one of CameraSelect
                flip    : one of CameraFlip

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CFlip(CameraSelect camera, CameraFlip flip);

#ifdef _cplusplus
} /* extern "C" */
#endif

/* TWL_CAMERA_I2C_SHARP_H_ */
#endif
