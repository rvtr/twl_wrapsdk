/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera
  File:     camera.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CAMERA_CAMERA_H_
#define TWL_CAMERA_CAMERA_H_

#include <twl/misc.h>
#include <twl/types.h>
#include <nitro/hw/ARM9/ioreg.h>
#include <twl/camera/ARM9/camera_api.h>

#ifndef MIN
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#endif
#define CAMERA_GET_MAX_LINES(width)     MIN((1024 / width), 16)
#define CAMERA_GET_LINE_BYTES(width)    (width << 1)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CAMERA_SELECT_FIRST,
    CAMERA_SELECT_SECOND
} CameraSelect;

typedef enum {
    CAMERA_OUTPUT_YUV,
    CAMERA_OUTPUT_RGB
} CameraOutput;

typedef enum {
    CAMERA_INTR_VSYNC_NONE          = (0 << REG_CAM_CAM_CNT_IREQ_VS_SHIFT),
    CAMERA_INTR_VSYNC_NEGATIVE_EDGE = (2 << REG_CAM_CAM_CNT_IREQ_VS_SHIFT),
    CAMERA_INTR_VSYNC_POSITIVE_EDGE = (3 << REG_CAM_CAM_CNT_IREQ_VS_SHIFT)
} CameraIntrVsync;

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SelectCamera

  Description:  set stbyn

  Arguments:    camera      one of CameraSelect

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SelectCamera( CameraSelect camera );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetStbyn

  Description:  set stbyn

  Arguments:    BOOL    High/Low

  Returns:      BOOL    last state
 *---------------------------------------------------------------------------*/
BOOL CAMERA_SetStbyn( BOOL high );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PowerOn

  Description:  power camera on

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_PowerOn( void );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PowerOff

  Description:  power camera off

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_PowerOff( void );

/*---------------------------------------------------------------------------*
  Name:         cameraPreSleep

  Description:  pre-sleep process for CAMERA without power off

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_PreSleep( void );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PostSleep

  Description:  pre-sleep process for CAMERA without power off

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_PostSleep( void );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_IsBusy

  Description:  whether camera is busy

  Arguments:    None

  Returns:      TRUE if camera is busy
 *---------------------------------------------------------------------------*/
BOOL CAMERA_IsBusy( void );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Start

  Description:  start to receive camera data

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_Start( void );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Stop

  Description:  stop to receive camera data

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_Stop( void );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetSyncLowActive

  Description:  set CAMERA sync polarity

  Arguments:    isLowActive     if low active, set TRUE

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetSyncLowActive( BOOL isLowActive );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetRclkNegativeEdge

  Description:  set CAMERA rclk edge.
                Should call while master clock is stopping.

  Arguments:    isNegativeEdge      if negative edge, set TRUE

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetRclkNegativeEdge( BOOL isNegativeEdge );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTrimmingParamsCenter

  Description:  set camera trimming parameters by centering
                expecting original image size is VGA.
                NOTE: should call CAMERA_SetTrimming to enable trimming

  Arguments:    destWidth       width of image to output
                destHeight      height of image to output
                srcWidth        original width of image
                srcHeight       original height of image

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTrimmingParamsCenter(u16 destWidth, u16 destHeight, u16 srcWidth, u16 srcHeight);

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
void CAMERA_SetTrimmingParams(u16 x1, u16 y1, u16 x2, u16 y2);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTrimming

  Description:  set trimming enable/disable

  Arguments:    enabled     TRUE if set trimming will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTrimming( BOOL enabled );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetOutputFormat

  Description:  set CAMERA output format.

  Arguments:    output      one of CameraOutput to set.

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetOutputFormat( CameraOutput output );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_GetErrorStatus

  Description:  whether line buffer has occurred some errors or not

  Arguments:    None

  Returns:      TRUE if error has occurred
 *---------------------------------------------------------------------------*/
BOOL CAMERA_GetErrorStatus( void );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ClearBuffer

  Description:  clear line buffer and error status

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_ClearBuffer( void );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetMasterIntrrupt

  Description:  set interrupt mode

  Arguments:    enabled     TRUE if set master interrupt will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetMasterIntrrupt( BOOL enabled );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetVsyncIntrrupt

  Description:  set vsync interrupt mode

  Arguments:    type        one of CameraIntrVsync to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetVsyncIntrrupt( CameraIntrVsync type );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetBufferErrorIntrrupt

  Description:  set buffer error interrupt mode

  Arguments:    enabled     TRUE if set buffer error interrupt will be enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetBufferErrorIntrrupt( BOOL enabled );

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetTransferLines

  Description:  set number of lines to store the buffer at once.

  Arguments:    lines   number of lines

  Returns:      None
 *---------------------------------------------------------------------------*/
void CAMERA_SetTransferLines( int lines );


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_CAMERA_CAMERA_H_ */
#endif
