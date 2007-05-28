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
    CAMERA_FLIPMODE_NONE,
    CAMERA_FLIPMODE_HORIZONTAL,
    CAMERA_FLIPMODE_VERTICAL,
    CAMERA_FLIPMODE_HORIZONTAL_VERTICAL,
    CAMERA_FLIPMODE_MAX,

    CAMERA_FLIPMODE_DEFAULT = CAMERA_FLIPMODE_HORIZONTAL
} CameraFlipMode;

typedef enum {
    CAMERA_SPECIALMODE_NONE,
    CAMERA_SPECIALMODE_NEVATIVE,
    CAMERA_SPECIALMODE_SEPIA,
    CAMERA_SPECIALMODE_BLUISH,
    CAMERA_SPECIALMODE_REDDISH,
    CAMERA_SPECIALMODE_GREENISH,
    CAMERA_SPECIALMODE_MAX,

    CAMERA_SPECIALMODE_AQUA = CAMERA_SPECIALMODE_BLUISH,
    CAMERA_SPECIALMODE_DEFAULT = CAMERA_SPECIALMODE_NONE
} CameraSpecialMode;

typedef enum {
//    CAMERA_PRESET_HVGA_20,
//    CAMERA_PRESET_DS_30,
    CAMERA_PRESET_VGA_20,
//    CAMERA_PRESET_QVGA_20,
//    CAMERA_PRESET_QVGA_30,
    CAMERA_PRESET_QVGA_30SD,
    CAMERA_PRESET_QVGA_30SS,
    CAMERA_PRESET_MAX,

    CAMERA_PRESET_DEFAULT = CAMERA_PRESET_QVGA_30SD
} CameraPreset;


/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_CAMERA_TYPES_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
