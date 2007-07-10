/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - camera
  File:     camera_i2c_micron.c

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

// for micron
BOOL CAMERAi_M_Default_Registers( CameraSelect camera );
BOOL CAMERAi_M_Initialize_Camera( CameraSelect camera );
BOOL CAMERAi_M_Image_Setting_ExtClk_6_75MHz_Op_Pix_27_5MHz_15fps( CameraSelect camera );
BOOL CAMERAi_M_Image_Setting_ExtClk_16_76MHz_Op_Pix_27_5MHz_15fps( CameraSelect camera );
BOOL CAMERAi_M_Viewfinder_ON( CameraSelect camera );
BOOL CAMERAi_M_Viewfinder_OFF( CameraSelect camera );
BOOL CAMERAi_M_Video_Capture_ON( CameraSelect camera );
BOOL CAMERAi_M_Video_Capture_OFF( CameraSelect camera );
BOOL CAMERAi_M_Lens_Calibration_Setup( CameraSelect camera );
BOOL CAMERAi_M_Lens_Calibration_Exit( CameraSelect camera );
BOOL CAMERAi_M_Fixed_15fps( CameraSelect camera );
BOOL CAMERAi_M_Refresh( CameraSelect camera );
BOOL CAMERAi_M_Auto_Exposure( CameraSelect camera );
BOOL CAMERAi_M_Gamma_Correction( CameraSelect camera );
BOOL CAMERAi_M_Auto_White_Balance( CameraSelect camera );
BOOL CAMERAi_M_Lens_Correction( CameraSelect camera );
BOOL CAMERAi_M_Image_Size_VGA( CameraSelect camera );
BOOL CAMERAi_M_Image_Size_QVGA( CameraSelect camera );
BOOL CAMERAi_M_Image_Size_CIF( CameraSelect camera );
BOOL CAMERAi_M_Image_Size_QCIF( CameraSelect camera );
BOOL CAMERAi_M_Effect_Off( CameraSelect camera );
BOOL CAMERAi_M_Effect_Mono( CameraSelect camera );
BOOL CAMERAi_M_Effect_Sepia( CameraSelect camera );
BOOL CAMERAi_M_Manual_WB_To_Auto_WB( CameraSelect camera );
BOOL CAMERAi_M_Manual_White_Balance_P1( CameraSelect camera );
BOOL CAMERAi_M_Manual_White_Balance_P2( CameraSelect camera );
BOOL CAMERAi_M_Manual_White_Balance_P3( CameraSelect camera );
BOOL CAMERAi_M_Manual_White_Balance_P4( CameraSelect camera );
BOOL CAMERAi_M_Manual_White_Balance_P5( CameraSelect camera );
BOOL CAMERAi_M_Manual_White_Balance_P6( CameraSelect camera );
BOOL CAMERAi_M_Manual_White_Balance_P7( CameraSelect camera );
BOOL CAMERAi_M_Manual_White_Balance_P8( CameraSelect camera );
BOOL CAMERAi_M_Sharpness_0( CameraSelect camera );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CInit(CameraSelect camera)
{
    return CAMERAi_M_Default_Registers(camera);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CStandby

  Description:  standby or resume CAMERA

  Arguments:    camera  : one of CameraSelect
                standby : TRUE if goto standby mode

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CStandby(CameraSelect camera, BOOL standby)
{
    if (standby)
    {
    }
    else
    {
    }
    (void)camera;
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CResize

  Description:  resize CAMERA

  Arguments:    camera  : one of CameraSelect
                width   : width of output image
                height  : height of output image

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CResize(CameraSelect camera, u16 width, u16 height)
{
    return CAMERAi_M_WriteRegister(camera, 0x98c, 0x2703)   // width (A)
        && CAMERAi_M_WriteRegister(camera, 0x990, width)
        && CAMERAi_M_WriteRegister(camera, 0x98c, 0x2705)   // height (A)
        && CAMERAi_M_WriteRegister(camera, 0x990, height)
        && CAMERAi_M_WriteRegister(camera, 0x98c, 0x2707)   // width (B)
        && CAMERAi_M_WriteRegister(camera, 0x990, width)
        && CAMERAi_M_WriteRegister(camera, 0x98c, 0x2709)   // height (B)
        && CAMERAi_M_WriteRegister(camera, 0x990, height);
    // anyone else???
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CPreSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CPreSleep(CameraSelect camera)
{
    (void)camera;
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_M_I2CPostSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_M_I2CPostSleep(CameraSelect camera)
{
    (void)camera;
    return FALSE;
}
