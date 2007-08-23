/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - camera
  File:     camera_i2c_sharp.c

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
#include <twl/camera/ARM7/i2c_sharp.h>

// insert auto-generated code
//#include "VGA_15fps_5fps_x8_CC_FilterKIM_MatrixOn3_PLL_QVGA_23Jul07_1676MHz.autogen.c"
#include "VGA_15fps_5fps_x8_CC_FilterKIM_MatrixOn3_PLL_QVGA_23Jul07_1676MHz_Improve.autogen.c"
//#include "VGA_15fps_5fps_x8_CC_FilterKIM_MatrixOn3_PLL_VGA_23Jul07_1676MHz_Improve.autogen.c"

#define BANK_ADDR   0x03
typedef enum
{
    BANK_GROUP_A    = 0x00,
    BANK_GROUP_B    = 0x01,
    BANK_GROUP_C    = 0x02
}
BankGroup;

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CInit(CameraSelect camera)
{
#if 0
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    // should not send init command same time (TODO:èoóÕÇ»ÇµÇ≈èâä˙âªÇ≈Ç´ÇÈÇ»ÇÁìØéûÇ…èàóùÇ∑ÇÈÇÊÇ§Ç…ïœÇ¶ÇÈ)
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = CAMERAi_S_Initialize(CAMERA_SELECT_IN)
           && CAMERAi_S_WriteRegister(CAMERA_SELECT_IN, BANK_ADDR, BANK_GROUP_B)
           && CAMERAi_S_SetFlags(CAMERA_SELECT_IN, 0x1A, 0x08)          // reverse RCLK polarity
           && CAMERAi_S_WriteRegister(CAMERA_SELECT_IN, 0x18, 0x02)     // force to order YUYV
           && CAMERAi_S_I2CStandby(CAMERA_SELECT_IN);
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = CAMERAi_S_Initialize(CAMERA_SELECT_OUT)
            && CAMERAi_S_WriteRegister(CAMERA_SELECT_OUT, BANK_ADDR, BANK_GROUP_B)
            && CAMERAi_S_SetFlags(CAMERA_SELECT_OUT, 0x1A, 0x08)        // reverse RCLK polarity
            && CAMERAi_S_WriteRegister(CAMERA_SELECT_OUT, 0x18, 0x02)   // force to order YUYV
           && CAMERAi_S_I2CStandby(CAMERA_SELECT_OUT);
    }
    return (rIn && rOut);
#else
    return CAMERAi_S_Initialize(camera)
        && CAMERAi_S_WriteRegister(camera, BANK_ADDR, BANK_GROUP_B)
        && CAMERAi_S_SetFlags(camera, 0x1A, 0x08)       // reverse RCLK polarity
        && CAMERAi_S_WriteRegister(camera, 0x18, 0x02)  // force to order YUYV
        && CAMERAi_S_ClearFlags(camera, 0x04, 0x88);    // goto standby (maybe already into)
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CStandby

  Description:  goto standby

  Arguments:    camera  : one of CameraSelect (IN/OUT/BOTH) to goto standby

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CStandby(CameraSelect camera)
{
    return CAMERAi_S_WriteRegister(camera, BANK_ADDR, BANK_GROUP_B)
        && CAMERAi_S_ClearFlags(camera, 0x04, 0x88);    // goto standby and disable to output
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CResume

  Description:  resume from standby

  Arguments:    camera  : one of CameraSelect (IN/OUT) to resume

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CResume(CameraSelect camera)
{
    if (camera == CAMERA_SELECT_BOTH)
    {
        return FALSE;
    }
    return CAMERAi_S_WriteRegister(camera, BANK_ADDR, BANK_GROUP_B)
        && CAMERAi_S_SetFlags(camera, 0x04, 0x88);  // resume from standby and enable to output
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CResumeBoth

  Description:  resume both CAMERAs, but only one will output

  Arguments:    camera  : one of CameraSelect (IN/OUT) to output

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CResumeBoth(CameraSelect camera)
{
    CameraSelect alternative = (camera == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN);
    if (camera == CAMERA_SELECT_BOTH)
    {
        return FALSE;
    }
    return CAMERAi_S_WriteRegister(CAMERA_SELECT_BOTH, BANK_ADDR, BANK_GROUP_B)
        && CAMERAi_S_SetParams(alternative, 0x04, 0x80, 0x88)   // resume but no output
        && CAMERAi_S_SetFlags(camera, 0x04, 0x88);              // resume and output
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CResize

  Description:  resize CAMERA

  Arguments:    camera  : one of CameraSelect
                width   : width of output image
                height  : height of output image

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CResize(CameraSelect camera, u16 width, u16 height)
{
    u8 data[2];
    if ( width <= 80 || width > 640 || height <= 60 || height > 480
      || (640*32) % width != 0 || (480*32) % height != 0 )
    {
        return FALSE;   // cannnot match for scale-down parameters
    }
    data[0] = (u8)((640*32) / width);
    data[1] = (u8)((480*32) / height);
    return CAMERAi_S_WriteRegister(camera, BANK_ADDR, BANK_GROUP_B)
            && CAMERAi_S_WriteRegisters(camera, 0x3D, data, 2);
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CFrameRate

  Description:  set CAMERA frame rate

  Arguments:    camera  : one of CameraSelect
                rate    : fps (0: auto)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CFrameRate(CameraSelect camera, int rate)
{
    (void)camera;
    if (rate == 0)
    {
    }
    else if (rate > 0 && rate <= 30)
    {
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CEffect

  Description:  set CAMERA effect

  Arguments:    camera  : one of CameraSelect
                effect  : one of CameraEffect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CEffect(CameraSelect camera, CameraEffect effect)
{
    (void)camera;
    switch (effect)
    {
    case CAMERA_EFFECT_NONE:
    case CAMERA_EFFECT_MONO:
    case CAMERA_EFFECT_SEPIA:
    case CAMERA_EFFECT_NEGATIVE:
        return FALSE;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CFlip

  Description:  set CAMERA flip/mirror

  Arguments:    camera  : one of CameraSelect
                flip    : one of CameraFlip

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CFlip(CameraSelect camera, CameraFlip flip)
{
    u8 data = 0;
    switch (flip)
    {
    //case CAMERA_FLIP_NONE:      data = 0x00;    break;
    case CAMERA_FLIP_VERTICAL:  data = 0x02;    break;
    case CAMERA_FLIP_HORIZONTAL:data = 0x01;    break;
    case CAMERA_FLIP_REVERSE:   data = 0x03;    break;
    }
    return CAMERAi_S_WriteRegister(camera, BANK_ADDR, BANK_GROUP_B)
        && CAMERAi_S_SetParams(camera, 0x0F, data, 0x03);
}
