/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera.c

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
#include <twl/camera.h>

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �ÓI�ϐ���`
 *---------------------------------------------------------------------------*/
static CameraSelect currentCamera;
static BOOL prestate;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SelectCamera

  Description:  set stbyn

  Arguments:    camera      one of CameraSelect

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SelectCamera( CameraSelect camera )
{
    if (currentCamera == camera)
    {
        return;
    }

    switch (camera)
    {
    case CAMERA_SELECT_FIRST:
        CAMERA_SetStbyn2(FALSE);
        break;
    case CAMERA_SELECT_SECOND:
        CAMERA_SetStbyn(FALSE);
        break;
    default:
        return;
    }
    currentCamera = camera;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetStbyn

  Description:  set STBYn for current camera

  Arguments:    BOOL    High/Low

  Returns:      BOOL    last state
 *---------------------------------------------------------------------------*/
BOOL CAMERA_SetStbyn( BOOL high )
{
    static BOOL prev = FALSE;
    BOOL temp = prev;
    switch (currentCamera)
    {
    case CAMERA_SELECT_FIRST:
        if (high)
        {
            reg_CAM_CAM_MCNT |= REG_CAM_CAM_MCNT_STBYN_MASK;
        }
        else
        {
            reg_CAM_CAM_MCNT &= ~REG_CAM_CAM_MCNT_STBYN_MASK;
        }
        break;
    case CAMERA_SELECT_SECOND:
        CAMERA_SetStbyn2(high);
        break;
    default:
        return temp;
    }
    prev = high;
    return temp;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PowerOn

  Description:  power camera on

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_PowerOn( void )
{
    reg_CFG_CLK |= REG_CFG_CLK_CAM_MASK;
    if ((reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK) == 0) {
        reg_CAM_CAM_MCNT |= (REG_CAM_CAM_MCNT_V28_MASK  // VDD2.8 POWER ON
                       | REG_CAM_CAM_MCNT_INI_MASK);// setup data line after CPU is powered on
        OS_SpinWaitSysCycles( 4 );              // wait to raise VDD2.8
        reg_CAM_CAM_MCNT &= ~REG_CAM_CAM_MCNT_V18_MASK; // VDD1.8 POWER ON
        OS_SpinWaitSysCycles( 4 );              // wait to raise VDD1.8
        reg_CAM_CAM_MCNT &= ~REG_CAM_CAM_MCNT_VIO_MASK; // VDDIO POWER ON
        OS_SpinWaitSysCycles( 4 );              // wait to raise VDDIO

        reg_CFG_CLK |= REG_CFG_CLK_CAM_CKI_MASK;// MCLK on
        OS_SpinWaitSysCycles( 100 );            // wait for over 100 MCLK cycles

        reg_CAM_CAM_MCNT |= REG_CAM_CAM_MCNT_RSTN_MASK; // RSTN => Hi
        CAMERA_SetStbyn(TRUE);                  // STBYN => Hi
        OS_SpinWaitSysCycles( 1800000 );        // wait for over 1800000 MCLK cycles (over 100msec!!!)

        reg_CAM_CAM_CNT = REG_CAM_CAM_CNT_CL_MASK;    // full reset CNT
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PowerOff

  Description:  power camera off

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CAMERA_PowerOff( void )
{
    if (reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK) {
        reg_CAM_CAM_CNT &= ~REG_CAM_CAM_CNT_E_MASK; // stop cmaera output

        CAMERA_SetStbyn(FALSE);             // STBYN => Lo
        OS_SpinWaitSysCycles( 20 );  // wait for over 20 MCLK cycles
        reg_CAM_CAM_MCNT &= ~REG_CAM_CAM_MCNT_RSTN_MASK;
        OS_SpinWaitSysCycles( 20 );  // wait for over 20 MCLK cycles

        reg_CFG_CLK  &= ~REG_CFG_CLK_CAM_CKI_MASK;  // MCLK off
        // no wait

        reg_CAM_CAM_MCNT |= REG_CAM_CAM_MCNT_VIO_MASK;  // VDDIO POWER OFF
        OS_SpinWaitSysCycles( 4 );              // wait a moment
        reg_CAM_CAM_MCNT |= REG_CAM_CAM_MCNT_V18_MASK;  // VDD1.8 POWER OFF
        OS_SpinWaitSysCycles( 4 );              // wait a moment
        reg_CAM_CAM_MCNT &= ~REG_CAM_CAM_MCNT_V28_MASK; // VDD2.8 POWER OFF
        OS_SpinWaitSysCycles( 4 );              // wait a moment
    }
    reg_CFG_CLK &= ~REG_CFG_CLK_CAM_MASK;   /* �K�v����H */
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PreSleep

  Description:  pre-sleep process for CAMERA without power off

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_PreSleep( void )
{
    if (reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK) {
        prestate = CAMERA_SetStbyn(FALSE);
        OS_SpinWaitSysCycles( 20 );  // wait for over 20 MCLK cycles
        // MCLK will stop automatically
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PostSleep

  Description:  pre-sleep process for CAMERA without power off

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_PostSleep( void )
{
    if (reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK) {
        // MCLK started automatically
        if (prestate)
        {
            CAMERA_SetStbyn(TRUE);
            OS_SpinWaitSysCycles( 100000 );  // wait for over 100000 MCLK cycles
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_IsBusy

  Description:  whether camera is busy

  Arguments:    None

  Returns:      TRUE if camera is busy
 *---------------------------------------------------------------------------*/
BOOL CAMERA_IsBusy( void )
{
    return (reg_CAM_CAM_CNT & REG_CAM_CAM_CNT_E_MASK) ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Start

  Description:  start to receive camera data

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_Start( void )
{
    reg_CAM_CAM_CNT |= REG_CAM_CAM_CNT_E_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Stop

  Description:  stop to receive camera data

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_Stop( void )
{
    reg_CAM_CAM_CNT &= ~REG_CAM_CAM_CNT_E_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetSyncLowActive

  Description:  set CAMERA sync polarity

  Arguments:    isLowActive     if low active, set TRUE

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetSyncLowActive( BOOL isLowActive )
{
    if (isLowActive) {
        reg_CAM_CAM_CNT |= (1 << REG_CAM_CAM_MCNT_SYNC_SHIFT);
    } else {
        reg_CAM_CAM_CNT &= ~(1 << REG_CAM_CAM_MCNT_SYNC_SHIFT);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetRclkNegativeEdge

  Description:  set CAMERA rclk edge.
                Should call while master clock is stopping.

  Arguments:    isNegativeEdge      if negative edge, set TRUE

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetRclkNegativeEdge( BOOL isNegativeEdge )
{
    if ((reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK) == 0) {
        if (isNegativeEdge) {
            reg_CAM_CAM_CNT |= (1 << REG_CAM_CAM_MCNT_IRCLK_SHIFT);
        } else {
            reg_CAM_CAM_CNT &= ~(1 << REG_CAM_CAM_MCNT_IRCLK_SHIFT);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTrimmingParamsCenter

  Description:  set camera trimming by centering
                expecting original image size is VGA.

  Arguments:    destWidth       width of image to output
                destHeight      height of image to output
                srcWidth        original width of image
                srcHeight       original height of image

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTrimmingParamsCenter(u16 destWidth, u16 destHeight, u16 srcWidth, u16 srcHeight)
{
    destWidth -= 2;
    destHeight -= 1;
    reg_CAM_SOFS_H = (u16)((srcWidth-destWidth) >> 1);
    reg_CAM_SOFS_V = (u16)((srcHeight-destHeight) >> 1);
    reg_CAM_EOFS_H = (u16)((srcWidth+destWidth) >> 1);
    reg_CAM_EOFS_V = (u16)((srcHeight+destHeight) >> 1);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTrimmingParams

  Description:  set camera trimming
                NOTE: widht = x2 - x1;  height = y2 - y1;

  Arguments:    x1      X of top-left trimming point (multiple of 2)
                y1      Y of top-left trimming point
                x2      X of bottom-right trimming point (multiple of 2)
                y2      Y of bottom-right trimming point

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTrimmingParams(u16 x1, u16 y1, u16 x2, u16 y2)
{
    reg_CAM_SOFS_H = x1;
    reg_CAM_SOFS_V = y1;
    reg_CAM_EOFS_H = (u16)(x2 - 2);
    reg_CAM_EOFS_V = (u16)(y2 - 1);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTrimming

  Description:  set trimming enable/disable

  Arguments:    enabled     TRUE if set trimming will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTrimming( BOOL enabled )
{
    if (enabled) {
        reg_CAM_CAM_CNT |= REG_CAM_CAM_CNT_T_MASK;
    } else {
        reg_CAM_CAM_CNT &= ~REG_CAM_CAM_CNT_T_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetOutputFormat

  Description:  set CAMERA output format.

  Arguments:    output      one of CameraOutput to set.

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetOutputFormat( CameraOutput output )
{
    switch (output) {
    case CAMERA_OUTPUT_YUV:
        reg_CAM_CAM_CNT &= ~(1 << REG_CAM_CAM_CNT_F_SHIFT);
        break;
    case CAMERA_OUTPUT_RGB:
        reg_CAM_CAM_CNT |= (1 << REG_CAM_CAM_CNT_F_SHIFT);
        break;
    default:
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetErrorStatus

  Description:  whether line buffer has occurred some errors or not

  Arguments:    None

  Returns:      TRUE if error has occurred
 *---------------------------------------------------------------------------*/
BOOL CAMERA_GetErrorStatus( void )
{
    return (reg_CAM_CAM_CNT & REG_CAM_CAM_CNT_ERR_MASK) ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ClearBuffer

  Description:  clear line buffer and error status

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_ClearBuffer( void )
{
    reg_CAM_CAM_CNT |= REG_CAM_CAM_CNT_CL_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetMasterIntrrupt

  Description:  set interrupt mode

  Arguments:    enabled     TRUE if set master interrupt will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetMasterIntrrupt( BOOL enabled )
{
    if (enabled) {
        reg_CAM_CAM_CNT |= REG_CAM_CAM_CNT_IREQ_I_MASK;
    } else {
        reg_CAM_CAM_CNT &= ~REG_CAM_CAM_CNT_IREQ_I_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetVsyncIntrrupt

  Description:  set vsync interrupt mode

  Arguments:    type        one of CameraIntrVsync to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetVsyncIntrrupt( CameraIntrVsync type )
{
    reg_CAM_CAM_CNT = (u16)((reg_CAM_CAM_CNT & ~REG_CAM_CAM_CNT_IREQ_VS_MASK) | type);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetBufferErrorIntrrupt

  Description:  set buffer error interrupt mode

  Arguments:    enabled     TRUE if set buffer error interrupt will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetBufferErrorIntrrupt( BOOL enabled )
{
    if (enabled) {
        reg_CAM_CAM_CNT |= REG_CAM_CAM_CNT_IREQ_BE_MASK;
    } else {
        reg_CAM_CAM_CNT &= ~REG_CAM_CAM_CNT_IREQ_BE_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTransferLines

  Description:  set number of lines to store the buffer at once.

  Arguments:    lines   number of lines

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTransferLines( int lines )
{
    if (lines >= 1 && lines <= 16)
    {
        u16 bits = (u16)((lines - 1) << REG_CAM_CAM_CNT_TL_SHIFT);
        reg_CAM_CAM_CNT = (u16)((reg_CAM_CAM_CNT & ~REG_CAM_CAM_CNT_TL_MASK) | bits);
    }
}