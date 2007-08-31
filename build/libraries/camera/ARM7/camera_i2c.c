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

static CameraSelect cameraSharp = CAMERA_SELECT_NONE;

#define GET_MICRON(camera)  (CameraSelect)(camera & ~cameraSharp)
#define GET_SHARP(camera)   (CameraSelect)(camera & cameraSharp)

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CInit(CameraSelect camera)
{
    BOOL ri = TRUE;
    BOOL ro = TRUE;
    (void)I2C_Lock();
    if (camera & CAMERA_SELECT_IN)
    {
        if (FALSE == (ri = CAMERAi_M_I2CInit(CAMERA_SELECT_IN)))
        {
            cameraSharp |= CAMERA_SELECT_IN;
            if (FALSE == (ri = CAMERAi_S_I2CInit(CAMERA_SELECT_IN)))
            {
                cameraSharp &= ~CAMERA_SELECT_IN;
            }
        }
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        if (FALSE == (ro = CAMERAi_M_I2CInit(CAMERA_SELECT_OUT)))
        {
            cameraSharp |= CAMERA_SELECT_OUT;
            if (FALSE == (ro = CAMERAi_S_I2CInit(CAMERA_SELECT_OUT)))
            {
                cameraSharp &= ~CAMERA_SELECT_OUT;
            }
        }
    }
    (void)I2C_Unlock();
    return ri & ro;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CStandby

  Description:  goto standby

  Arguments:    camera  : one of CameraSelect (IN/OUT/BOTH) to goto standby

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CStandby(CameraSelect camera)
{
    BOOL rm, rs;
    (void)I2C_Lock();
    rm = GET_MICRON(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_M_I2CStandby(GET_MICRON(camera)) : TRUE;
    rs = GET_SHARP(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_S_I2CStandby(GET_SHARP(camera)) : TRUE;
    (void)I2C_Unlock();
    return rm & rs;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResume

  Description:  resume from standby

  Arguments:    camera  : one of CameraSelect (IN/OUT) to resume

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CResume(CameraSelect camera)
{
    BOOL rm, rs;
    (void)I2C_Lock();
    rm = GET_MICRON(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_M_I2CResume(GET_MICRON(camera)) : TRUE;
    rs = GET_SHARP(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_S_I2CResume(GET_SHARP(camera)) : TRUE;
    (void)I2C_Unlock();
    return rm & rs;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResumeBoth

  Description:  resume both CAMERAs, but only one will output

  Arguments:    camera  : one of CameraSelect (IN/OUT) to output

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CResumeBoth(CameraSelect camera)
{
    BOOL rm, rs;
    (void)I2C_Lock();
    rm = GET_MICRON(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_M_I2CResumeBoth(GET_MICRON(camera)) : TRUE;
    rs = GET_SHARP(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_S_I2CResumeBoth(GET_SHARP(camera)) : TRUE;
    (void)I2C_Unlock();
    return rm & rs;
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
    BOOL rm, rs;
    (void)I2C_Lock();
    rm = GET_MICRON(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_M_I2CResize(GET_MICRON(camera), width, height) : TRUE;
    rs = GET_SHARP(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_S_I2CResize(GET_SHARP(camera), width, height) : TRUE;
    (void)I2C_Unlock();
    return rm & rs;
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
    BOOL rm, rs;
    (void)I2C_Lock();
    rm = GET_MICRON(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_M_I2CFrameRate(GET_MICRON(camera), rate) : TRUE;
    rs = GET_SHARP(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_S_I2CFrameRate(GET_SHARP(camera), rate) : TRUE;
    (void)I2C_Unlock();
    return rm & rs;
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
    BOOL rm, rs;
    (void)I2C_Lock();
    rm = GET_MICRON(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_M_I2CEffect(GET_MICRON(camera), effect) : TRUE;
    rs = GET_SHARP(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_S_I2CEffect(GET_SHARP(camera), effect) : TRUE;
    (void)I2C_Unlock();
    return rm & rs;
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
    BOOL rm, rs;
    (void)I2C_Lock();
    rm = GET_MICRON(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_M_I2CFlip(GET_MICRON(camera), flip) : TRUE;
    rs = GET_SHARP(camera) != CAMERA_SELECT_NONE ?
            CAMERAi_S_I2CFlip(GET_SHARP(camera), flip) : TRUE;
    (void)I2C_Unlock();
    return rm & rs;
}

#if 0
    その他のAPI候補
    ホワイトバランス、露光
    フォーマット(YUYVの順番、RGB444もあり?)
#endif
