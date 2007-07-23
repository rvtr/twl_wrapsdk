/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - camera
  File:     camera_i2c_micron.c

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

// insert auto-generated code
#include "MT9V113-MTM10-3.autogen.c"
//#include "MT9V113-MTM11.autogen.c"

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CInit(CameraSelect camera)
{
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    // should not send init command same time
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = CAMERAi_M_Default_Registers(CAMERA_SELECT_IN)
           && CAMERAi_M_WriteMCU(CAMERA_SELECT_IN, 0x2755, 0x0002)  // YUYV format (required to refresh)
           && CAMERAi_M_I2CResize(CAMERA_SELECT_IN, 320, 240)
           && CAMERAi_M_I2CStandby(CAMERA_SELECT_IN, TRUE);
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = CAMERAi_M_Default_Registers(CAMERA_SELECT_OUT)
            && CAMERAi_M_WriteMCU(CAMERA_SELECT_OUT, 0x2755, 0x0002)    // YUYV format (required to refresh)
            && CAMERAi_M_I2CResize(CAMERA_SELECT_OUT, 320, 240)
            && CAMERAi_M_I2CStandby(CAMERA_SELECT_OUT, TRUE);
    }
    return (rIn && rOut);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CStandby

  Description:  standby or resume CAMERA

  Arguments:    camera  : one of CameraSelect
                standby : TRUE if goto standby mode

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CStandby(CameraSelect camera, BOOL standby)
{
    if (standby)
    {
        return CAMERAi_M_ClearFlags(camera, 0x001A, 0x0200) // stop to output
//      return CAMERAi_M_Stop(camera)
            && CAMERAi_M_SetFlags(camera, 0x0018, 0x0001);  // go to standby
    }
    else
    {
        return CAMERAi_M_ClearFlags(camera, 0x0018, 0x0001) // leave standby
            && CAMERAi_M_SetFlags(camera, 0x001A, 0x0200);  // start to output
//          && CAMERAi_M_Start(camera);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CResize

  Description:  resize CAMERA

  Arguments:    camera  : one of CameraSelect
                width   : width of output image
                height  : height of output image

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CResize(CameraSelect camera, u16 width, u16 height)
{
    return CAMERAi_M_WriteMCU(camera, 0x2703, width)    // width (A)
        && CAMERAi_M_WriteMCU(camera, 0x2705, height)   // height (A)
//        && CAMERAi_M_WriteMCU(camera, 0x2707, width)    // width (B)
//        && CAMERAi_M_WriteMCU(camera, 0x2709, height)   // height (B)
        && CAMERAi_M_Refresh(camera);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CFrameRate

  Description:  set CAMERA frame rate

  Arguments:    camera  : one of CameraSelect
                rate    : fps (0: auto)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CFrameRate(CameraSelect camera, int rate)
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
  Name:         CAMERAi_M_I2CEffect

  Description:  set CAMERA effect

  Arguments:    camera  : one of CameraSelect
                effect  : one of CameraEffect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CEffect(CameraSelect camera, CameraEffect effect)
{
    switch (effect)
    {
    case CAMERA_EFFECT_NONE:
        return CAMERAi_M_Effect_Off(camera);
//        return CAMERAi_M_Option_Effect_Off(camera);
    case CAMERA_EFFECT_MONO:
        return CAMERAi_M_Effect_Mono(camera);
//        return CAMERAi_M_Option_Effect_Mono(camera);
    case CAMERA_EFFECT_SEPIA:
        return CAMERAi_M_Effect_Sepia(camera);
//        return CAMERAi_M_Option_Effect_Sepia(camera);
    case CAMERA_EFFECT_NEGATIVE:
        return CAMERAi_M_WriteMCU(camera, 0x2759, 0x6443)  //NEVATIVE_SPEC_EFFECTS_A
            && CAMERAi_M_WriteMCU(camera, 0x275B, 0x6443)  //NEVATIVE_SPEC_EFFECTS_B
            && CAMERAi_M_Refresh(camera);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CFlip

  Description:  set CAMERA flip/mirror

  Arguments:    camera  : one of CameraSelect
                flip    : one of CameraFlip

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CFlip(CameraSelect camera, CameraFlip flip)
{
    (void)camera;
    switch (flip)
    {
    case CAMERA_FLIP_NONE:      // normal
    case CAMERA_FLIP_VERTICAL:  // vertical flip
    case CAMERA_FLIP_HORIZONTAL:// horizontal mirror
    case CAMERA_FLIP_REVERSE:   // turn over
        return FALSE;
    }
    return FALSE;
}
#if 0
/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CWhiteBalance

  Description:  set CAMERA white balance

  Arguments:    camera  : one of CameraSelect
                type    : preset number (0: auto)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CWhiteBalance(CameraSelect camera, int type)
{
    switch (type)
    {
    case 0:
        return CAMERAi_M_Manual_WB_To_Auto_WB(camera);
    case 1:
        return CAMERAi_M_Manual_White_Balance_P1(camera);
    case 2:
        return CAMERAi_M_Manual_White_Balance_P2(camera);
    case 3:
        return CAMERAi_M_Manual_White_Balance_P3(camera);
    case 4:
        return CAMERAi_M_Manual_White_Balance_P4(camera);
    case 5:
        return CAMERAi_M_Manual_White_Balance_P5(camera);
    case 6:
        return CAMERAi_M_Manual_White_Balance_P6(camera);
    case 7:
        return CAMERAi_M_Manual_White_Balance_P7(camera);
    case 8:
        return CAMERAi_M_Manual_White_Balance_P8(camera);
    }
    return FALSE;
}
/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CExposure

  Description:  set CAMERA exposure

  Arguments:    camera  : one of CameraSelect
                type    : preset number (0: auto)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CExposure(CameraSelect camera, int type)
{
    switch (type)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        return TRUE;
    }
    return FALSE;
}
#endif
