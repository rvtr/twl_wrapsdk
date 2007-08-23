/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera
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
#ifndef TWL_CAMERA_I2C_COMMON_H_
#define TWL_CAMERA_I2C_COMMON_H_

#include <twl/types.h>
#include <twl/camera/common/types.h>

#ifdef _cplusplus
extern "C" {
#endif

//================================================================================
//        I2C API
//================================================================================

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CInit(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CStandby

  Description:  goto standby

  Arguments:    camera  : one of CameraSelect (IN/OUT/BOTH) to goto standby

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CStandby(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResume

  Description:  resume from standby

  Arguments:    camera  : one of CameraSelect (IN/OUT) to resume

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CResume(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResumeBoth

  Description:  resume both CAMERAs, but only one will output

  Arguments:    camera  : one of CameraSelect (IN/OUT) to output

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CResumeBoth(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResize

  Description:  resize CAMERA

  Arguments:    camera  : one of CameraSelect
                width   : width of output image
                height  : height of output image

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CResize(CameraSelect camera, u16 width, u16 height);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRate

  Description:  set CAMERA frame rate

  Arguments:    camera  : one of CameraSelect
                rate    : fps (0: auto)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CFrameRate(CameraSelect camera, int rate);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffect

  Description:  set CAMERA effect

  Arguments:    camera  : one of CameraSelect
                effect  : one of CameraEffect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CEffect(CameraSelect camera, CameraEffect effect);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlip

  Description:  set CAMERA flip/mirror

  Arguments:    camera  : one of CameraSelect
                flip    : one of CameraFlip

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CFlip(CameraSelect camera, CameraFlip flip);

#ifdef _cplusplus
} /* extern "C" */
#endif

/* TWL_CAMERA_I2C_COMMON_H_ */
#endif
