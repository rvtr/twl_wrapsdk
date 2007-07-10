/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera - include
  File:     types.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CAMERA_TYPES_H_
#define TWL_CAMERA_TYPES_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

typedef enum {
    CAMERA_SELECT_NONE  = 0,
    CAMERA_SELECT_IN    = (1<<0),
    CAMERA_SELECT_OUT   = (1<<1),
    CAMERA_SELECT_BOTH  = (CAMERA_SELECT_IN|CAMERA_SELECT_OUT)
} CameraSelect;

typedef enum {
    CAMERA_FUNC_INIT,
    CAMERA_FUNC_STANDBY,
    CAMERA_FUNC_RESIZE,
    CAMERA_FUNC_PRESLEEP,
    CAMERA_FUNC_POSTSLEEP,
    CAMERA_FUNC_MAX
} CameraFunct;

/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_CAMERA_TYPES_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
