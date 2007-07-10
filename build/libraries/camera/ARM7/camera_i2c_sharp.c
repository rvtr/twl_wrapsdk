/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - camera
  File:     camera_i2c_sharp.c

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

//#define USE_MULTIPLE_IO   // use [Read|Write]Registers();

// for sharp

/*---------------------------------------------------------------------------*
  Name:         CAMERA_S_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_S_I2CInit(CameraSelect camera)
{
    (void)camera;
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_S_I2CStandby

  Description:  standby or resume CAMERA

  Arguments:    camera  : one of CameraSelect
                standby : TRUE if goto standby mode

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_S_I2CStandby(CameraSelect camera, BOOL standby)
{
    if (standby)
    {
    }
    else
    {
    }
    (void)camera;
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_S_I2CResize

  Description:  resize CAMERA

  Arguments:    camera  : one of CameraSelect
                width   : width of output image
                height  : height of output image

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_S_I2CResize(CameraSelect camera, u16 width, u16 height)
{
    (void)height;
    (void)width;
    (void)camera;
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_S_I2CPreSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_S_I2CPreSleep(CameraSelect camera)
{
    (void)camera;
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_S_I2CPostSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_S_I2CPostSleep(CameraSelect camera)
{
    (void)camera;
    return FALSE;
}
