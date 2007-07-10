/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - camera
  File:     camera_i2c_common.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/camera.h>

typedef enum
{
    CAMERA_TYPE_MICRON,
    CAMERA_TYPE_SHARP,

    CAMERA_TYPE_UNKNOWN
}
CAMERAType;

static CAMERAType cameraType = CAMERA_TYPE_MICRON;

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
        result = CAMERA_M_I2CInit(camera);
        if (result == FALSE)
        {
            cameraType = CAMERA_TYPE_SHARP;
        }
    }

    if (cameraType == CAMERA_TYPE_SHARP)
    {
        result = CAMERA_S_I2CInit(camera);
        if (result == FALSE)
        {
            cameraType = CAMERA_TYPE_MICRON; //rotation CAMERA_TYPE_UNKNOWN;
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
    switch (camera)
    {
    case CAMERA_TYPE_MICRON:
        CAMERA_M_I2CStandby(camera, standby);
        break;
    case CAMERA_TYPE_SHARP:
        CAMERA_S_I2CStandby(camera, standby);
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
    switch (camera)
    {
    case CAMERA_TYPE_MICRON:
        CAMERA_M_I2CResize(camera, width, height);
        break;
    case CAMERA_TYPE_SHARP:
        CAMERA_S_I2CResize(camera, width, height);
        break;
    }
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPreSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CPreSleep(CameraSelect camera)
{
    BOOL result = FALSE;
    (void)I2C_Lock();
    switch (camera)
    {
    case CAMERA_TYPE_MICRON:
        CAMERA_M_I2CPreSleep(camera);
        break;
    case CAMERA_TYPE_SHARP:
        CAMERA_M_I2CPreSleep(camera);
        break;
    }
    (void)I2C_Unlock();
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPostSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CPostSleep(CameraSelect camera)
{
    BOOL result = FALSE;
    (void)I2C_Lock();
    switch (camera)
    {
    case CAMERA_TYPE_MICRON:
        CAMERA_M_I2CPostSleep(camera);
        break;
    case CAMERA_TYPE_SHARP:
        CAMERA_M_I2CPostSleep(camera);
        break;
    }
    (void)I2C_Unlock();
    return result;
}



