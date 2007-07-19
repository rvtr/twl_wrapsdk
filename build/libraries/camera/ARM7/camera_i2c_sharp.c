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
//#include "5030_15fpsFIX_x8_CC_FilterKIM_MatrixOn3_PLL.autogen.c"
#include "5030_15fps_5fps_adjust2_x8_CC_FilterKIM_MatrixOn3_PLL_1676MHz-1676MHz.autogen.c"


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
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    // should not send init command same time
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = CAMERAi_S_Initialize(CAMERA_SELECT_IN)
           && CAMERAi_S_WriteRegister(CAMERA_SELECT_IN, 0x18, 0x02)     // force to order YUYV
           && CAMERAi_S_I2CStandby(CAMERA_SELECT_IN, TRUE);
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = CAMERAi_S_Initialize(CAMERA_SELECT_OUT)
           && CAMERAi_S_WriteRegister(CAMERA_SELECT_OUT, 0x18, 0x02)    // force to order YUYV
            && CAMERAi_S_I2CStandby(CAMERA_SELECT_OUT, TRUE);
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_S_I2CStandby

  Description:  standby or resume CAMERA

  Arguments:    camera  : one of CameraSelect
                standby : TRUE if goto standby mode

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_S_I2CStandby(CameraSelect camera, BOOL standby)
{
    if (standby)
    {
        return CAMERAi_S_WriteRegister(camera, BANK_ADDR, BANK_GROUP_B)
            && CAMERAi_S_ClearFlags(camera, 0x04, 0x80);
    }
    else
    {
        return CAMERAi_S_WriteRegister(camera, BANK_ADDR, BANK_GROUP_B)
            && CAMERAi_S_SetFlags(camera, 0x04, 0x80);
    }
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
    u8 data[2] = { 0, 0 };
    switch (width)
    {
    case 640:   data[0] = 32;   break;
    case 320:   data[0] = 64;   break;
    case 160:   data[0] = 128;  break;
    }
    switch (height)
    {
    case 480:   data[1] = 32;   break;
    case 240:   data[1] = 64;   break;
    case 120:   data[1] = 128;  break;
    }
    if (data[0] && data[1])
    {
        return CAMERAi_S_WriteRegister(camera, BANK_ADDR, BANK_GROUP_B)
            && CAMERAi_S_WriteRegisters(camera, 0x3D, data, 2);
    }
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
    (void)rate;
    (void)camera;
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
    (void)effect;
    (void)camera;
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
    (void)flip;
    (void)camera;
    return FALSE;
}
