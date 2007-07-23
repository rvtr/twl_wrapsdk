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
    定数定義
 *---------------------------------------------------------------------------*/
#define SYNC_TYPE   (0 << REG_CAM_CAM_MCNT_SYNC_SHIFT)  // 1 if low active
#define RCLK_TYPE   (0 << REG_CAM_CAM_MCNT_IRCLK_SHIFT) // 1 if negative edge

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static inline void CAMERAi_Wait(u32 clocks)
{
    OS_SpinWaitSysCycles(clocks << 1);
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
    if ((reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK) == 0)
    {
        // (re)set polarities first
        reg_CAM_CAM_MCNT = (u16)((reg_CAM_CAM_MCNT & ~(REG_CAM_CAM_MCNT_SYNC_MASK | REG_CAM_CAM_MCNT_IRCLK_MASK))
                                 | SYNC_TYPE | RCLK_TYPE);

        reg_CAM_CAM_MCNT |= REG_CAM_CAM_MCNT_V28_MASK;  // VDD2.8 POWER ON
        CAMERAi_Wait( 10 );                             // wait for over 10 MCLKs (M:10-20)(S:10ns?)
        reg_CFG_CLK |= REG_CFG_CLK_CAM_CKI_MASK;        // MCLK on
        CAMERAi_Wait( 32 );                             // wait for over 32 MCLKs (M:20-10)(S:32)
        reg_CAM_CAM_MCNT |= REG_CAM_CAM_MCNT_RSTN_MASK; // RSTN => Hi
        CAMERAi_Wait( 6000 );                           // wait for over 6000 MCLKs

        CAMERA_StopCapture();                           // stop cmaera output
        while (CAMERA_IsBusy() != FALSE)
        {
        }
        CAMERA_ClearBuffer();                           // clear buffer and error
    }
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PowerOff

  Description:  power camera off

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_PowerOff( void )
{
    if (reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK)
    {
        CAMERA_StopCapture();                           // stop cmaera output
        while (CAMERA_IsBusy() != FALSE)
        {
        }

        reg_CAM_CAM_MCNT &= ~REG_CAM_CAM_MCNT_RSTN_MASK;// RSTN => Lo
        CAMERAi_Wait( 10 );                             // wait for over 10 MCLKs (M:10)(S:10ns?)

        reg_CFG_CLK  &= ~REG_CFG_CLK_CAM_CKI_MASK;      // MCLK off
        CAMERAi_Wait( 20 );                             // wait for over 20 MCLKs (M:20)(S:0?)

        reg_CAM_CAM_MCNT &= ~REG_CAM_CAM_MCNT_V28_MASK; // VDD2.8 POWER OFF
    }
    reg_CFG_CLK &= ~REG_CFG_CLK_CAM_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_IsBusy

  Description:  whether camera is busy

  Arguments:    None

  Returns:      TRUE if camera is busy
 *---------------------------------------------------------------------------*/
BOOL CAMERA_IsBusy( void )
{
    return (reg_CAM_CAM_CNT & REG_CAM_CAM_CNT_E_MASK) >> REG_CAM_CAM_CNT_E_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_StartCapture

  Description:  start to receive camera data

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_StartCapture( void )
{
    reg_CAM_CAM_CNT |= REG_CAM_CAM_CNT_E_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_StopCapture

  Description:  stop to receive camera data

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_StopCapture( void )
{
    reg_CAM_CAM_CNT &= ~REG_CAM_CAM_CNT_E_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTrimmingParamsCenter

  Description:  set camera trimming parameters by centering
                NOTE: should call CAMERA_SetTrimming to enable trimming

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

  Description:  set camera trimming parameters
                NOTE: widht = x2 - x1;  height = y2 - y1;
                NOTE: should call CAMERA_SetTrimming to enable trimming

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

  Description:  set trimming to be enabled/disabled

  Arguments:    enabled     TRUE if set trimming will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTrimming( BOOL enabled )
{
    u16 value = reg_CAM_CAM_CNT;
    reg_CAM_CAM_CNT = enabled ? (u16)(value | REG_CAM_CAM_CNT_T_MASK)
                              : (u16)(value & ~REG_CAM_CAM_CNT_T_MASK);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetOutputFormat

  Description:  set CAMERA output format.

  Arguments:    output      one of CameraOutput to set.

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetOutputFormat( CameraOutput output )
{
    u16 value = reg_CAM_CAM_CNT;
    switch (output)
    {
    case CAMERA_OUTPUT_YUV:
        reg_CAM_CAM_CNT = (u16)(value & ~REG_CAM_CAM_CNT_F_MASK);
        break;
    case CAMERA_OUTPUT_RGB:
        reg_CAM_CAM_CNT = (u16)(value | REG_CAM_CAM_CNT_F_MASK);
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
    return (reg_CAM_CAM_CNT & REG_CAM_CAM_CNT_ERR_MASK) >> REG_CAM_CAM_CNT_ERR_SHIFT;
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
    u16 value = reg_CAM_CAM_CNT;
    reg_CAM_CAM_CNT = enabled ? (u16)(value | REG_CAM_CAM_CNT_IREQ_I_MASK)
                              : (u16)(value & ~REG_CAM_CAM_CNT_IREQ_I_MASK);
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
    u16 value = reg_CAM_CAM_CNT;
    reg_CAM_CAM_CNT = enabled ? (u16)(value | REG_CAM_CAM_CNT_IREQ_BE_MASK)
                              : (u16)(value & ~REG_CAM_CAM_CNT_IREQ_BE_MASK);
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
