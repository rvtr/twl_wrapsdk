/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - camera
  File:     camera_i2c.c

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
#include <twl/camera/ARM7/i2c_micron.h>
#include <twl/camera/ARM7/i2c_sharp.h>

typedef enum
{
    CAMERA_TYPE_MICRON,
    CAMERA_TYPE_SHARP,

    CAMERA_TYPE_UNKNOWN
}
CAMERAType;

static CAMERAType cameraType = CAMERA_TYPE_SHARP;

#if 0
    CAMERA_I2CInit()だけがカメラの種類を入れ替える機能を有する
#endif

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CInit(CameraSelect camera)
{
    BOOL result = FALSE;
    (void)I2C_Lock();
    if (cameraType == CAMERA_TYPE_MICRON)
    {
        result = CAMERAi_M_I2CInit(camera);
        if (result == FALSE)
        {
            cameraType = CAMERA_TYPE_SHARP;
        }
    }
    if (cameraType == CAMERA_TYPE_SHARP)
    {
        result = CAMERAi_S_I2CInit(camera);
        if (result == FALSE)
        {
            cameraType = CAMERA_TYPE_MICRON; // rotate for next try
            //cameraType = CAMERA_TYPE_UNKNOWN; // annihilate camera I2C
        }
    }
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CStandby

  Description:  standby or resume CAMERA

  Arguments:    camera  : one of CameraSelect
                standby : TRUE if goto standby mode

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CStandby(CameraSelect camera, BOOL standby)
{
    BOOL result = FALSE;
    (void)I2C_Lock();
    switch (cameraType)
    {
    case CAMERA_TYPE_MICRON:
        result = CAMERAi_M_I2CStandby(camera, standby);
        break;
    case CAMERA_TYPE_SHARP:
        result = CAMERAi_S_I2CStandby(camera, standby);
        break;
    }
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResize

  Description:  resize CAMERA

  Arguments:    camera  : one of CameraSelect
                width   : width of output image
                height  : height of output image

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CResize(CameraSelect camera, u16 width, u16 height)
{
    BOOL result = FALSE;
    (void)I2C_Lock();
    switch (cameraType)
    {
    case CAMERA_TYPE_MICRON:
        result = CAMERAi_M_I2CResize(camera, width, height);
        break;
    case CAMERA_TYPE_SHARP:
        result = CAMERAi_S_I2CResize(camera, width, height);
        break;
    }
    (void)I2C_Unlock();
    return result;
}


/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRate

  Description:  set CAMERA frame rate

  Arguments:    camera  : one of CameraSelect
                rate    : fps (0: auto)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CFrameRate(CameraSelect camera, int rate)
{
    BOOL result = FALSE;
    (void)I2C_Lock();
    switch (cameraType)
    {
    case CAMERA_TYPE_MICRON:
        result = CAMERAi_M_I2CFrameRate(camera, rate);
        break;
    case CAMERA_TYPE_SHARP:
        result = CAMERAi_S_I2CFrameRate(camera, rate);
        break;
    }
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffect

  Description:  set CAMERA effect

  Arguments:    camera  : one of CameraSelect
                effect  : one of CameraEffect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CEffect(CameraSelect camera, CameraEffect effect)
{
    BOOL result = FALSE;
    (void)I2C_Lock();
    switch (cameraType)
    {
    case CAMERA_TYPE_MICRON:
        result = CAMERAi_M_I2CEffect(camera, effect);
        break;
    case CAMERA_TYPE_SHARP:
        result = CAMERAi_S_I2CEffect(camera, effect);
        break;
    }
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlip

  Description:  set CAMERA flip/mirror

  Arguments:    camera  : one of CameraSelect
                flip    : one of CameraFlip

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CFlip(CameraSelect camera, CameraFlip flip)
{
    BOOL result = FALSE;
    (void)I2C_Lock();
    switch (cameraType)
    {
    case CAMERA_TYPE_MICRON:
        result = CAMERAi_M_I2CFlip(camera, flip);
        break;
    case CAMERA_TYPE_SHARP:
        result = CAMERAi_S_I2CFlip(camera, flip);
        break;
    }
    (void)I2C_Unlock();
    return result;
}

