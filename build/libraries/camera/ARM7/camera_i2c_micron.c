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
#if 0
    BOOL rIn = TRUE;
    BOOL rOut = TRUE;
    // should not send init command same time (TODO:o—Í‚È‚µ‚Å‰Šú‰»‚Å‚«‚é‚È‚ç“¯Žž‚Éˆ—‚·‚é‚æ‚¤‚É•Ï‚¦‚é)
    if (camera & CAMERA_SELECT_IN)
    {
        rIn = CAMERAi_M_Default_Registers(CAMERA_SELECT_IN)
           && CAMERAi_M_WriteMCU(CAMERA_SELECT_IN, 0x2755, 0x0002)  // YUYV format (required to refresh)
           && CAMERAi_M_I2CStandby(CAMERA_SELECT_IN);
    }
    if (camera & CAMERA_SELECT_OUT)
    {
        rOut = CAMERAi_M_Default_Registers(CAMERA_SELECT_OUT)
            && CAMERAi_M_WriteMCU(CAMERA_SELECT_OUT, 0x2755, 0x0002)    // YUYV format (required to refresh)
            && CAMERAi_M_I2CStandby(CAMERA_SELECT_OUT);
    }
    return (rIn && rOut);
#else
    return CAMERAi_M_Default_Registers(camera)
        && CAMERAi_M_WriteMCU(camera, 0x2755, 0x0002)   // YUYV format (required to refresh)
        && CAMERAi_M_SetFlags(camera, 0x0018, 0x0001);  // goto standby
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CStandby

  Description:  goto standby

  Arguments:    camera  : one of CameraSelect (IN/OUT/BOTH) to goto standby

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CStandby(CameraSelect camera)
{
    return CAMERAi_M_ClearFlags(camera, 0x001A, 0x0200) // disable to output
        && CAMERAi_M_SetFlags(camera, 0x0018, 0x0001);  // goto standby
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CResume

  Description:  resume from standby

  Arguments:    camera  : one of CameraSelect (IN/OUT) to resume

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CResume(CameraSelect camera)
{
    if (camera == CAMERA_SELECT_BOTH)
    {
        return FALSE;
    }
    return CAMERAi_M_ClearFlags(camera, 0x0018, 0x0001)  // resume from standby
        && CAMERAi_M_SetFlags(camera, 0x001A, 0x0200);   // enable to output;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_M_I2CResumeBoth

  Description:  resume both CAMERAs, but only one will output

  Arguments:    camera  : one of CameraSelect (IN/OUT) to output

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERAi_M_I2CResumeBoth(CameraSelect camera)
{
    CameraSelect alternative = (camera == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN);
    if (camera == CAMERA_SELECT_BOTH)
    {
        return FALSE;
    }
    return CAMERAi_M_ClearFlags(alternative, 0x001A, 0x0200)        // disable to output
        && CAMERAi_M_SetFlags(camera, 0x001A, 0x0200)               // enable to output
        && CAMERAi_M_ClearFlags(CAMERA_SELECT_BOTH, 0x0018, 0x0001);// resume from standby
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
    case CAMERA_EFFECT_MONO:
        return CAMERAi_M_Effect_Mono(camera);
    case CAMERA_EFFECT_SEPIA:
        return CAMERAi_M_Effect_Sepia(camera);
    case CAMERA_EFFECT_NEGATIVE:
        return CAMERAi_M_WriteMCU(camera, 0x2759, 0x6443)  //NEGATIVE_SPEC_EFFECTS_A
            && CAMERAi_M_WriteMCU(camera, 0x275B, 0x6443)  //NEGATIVE_SPEC_EFFECTS_B
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
