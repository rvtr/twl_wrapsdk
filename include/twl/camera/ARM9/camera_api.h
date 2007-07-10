/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera_api.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_CAMERA_CAMERA_API_H_
#define TWL_CAMERA_CAMERA_API_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// 処理結果定義
typedef enum CAMERAResult
{
    CAMERA_RESULT_SUCCESS = 0,
    CAMERA_RESULT_SUCCESS_TRUE,
    CAMERA_RESULT_SUCCESS_FALSE,
    CAMERA_RESULT_BUSY,
    CAMERA_RESULT_ILLEGAL_PARAMETER,
    CAMERA_RESULT_SEND_ERROR,
    CAMERA_RESULT_INVALID_COMMAND,
    CAMERA_RESULT_ILLEGAL_STATUS,
    CAMERA_RESULT_FATAL_ERROR,
    CAMERA_RESULT_MAX
}
CAMERAResult;

// コールバック
typedef void (*CAMERACallback)(CAMERAResult result, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Init

  Description:  CAMERAライブラリを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_Init(void);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CSelectAsync

  Description:  select CAMERA to activate
                async version

  Arguments:    camera      - one of CameraSelect
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CSelectAsync(CameraSelect camera, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CSelect

  Description:  select CAMERA to activate
                sync version.

  Arguments:    camera      - one of CameraSelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CSelect(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_WriteRegistersAsync

  Description:  write CAMERA registers via I2C.
                async version.

  Arguments:    camera      - one of CameraSelect
                addr        - start address
                bufp        - buffer to write
                length      - length of bufp
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_WriteRegistersAsync(CameraSelect camera, u8 addr, const u8* bufp, size_t length, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_WriteRegisters

  Description:  write CAMERA registers via I2C.
                sync version.

  Arguments:    camera      - one of CameraSelect
                addr        - start address
                bufp        - buffer to write
                length      - length of bufp

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_WriteRegisters(CameraSelect camera, u8 addr, const u8* bufp, size_t length);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ReadRegistersAsync

  Description:  read CAMERA registers via I2C.

  Arguments:    camera      - one of CameraSelect
                addr        - start address
                bufp        - buffer to read
                length      - length of bufp
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_ReadRegistersAsync(CameraSelect camera, u8 addr, u8* bufp, size_t length, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ReadRegisters

  Description:  set CAMERA key normally
                sync version.

  Arguments:    camera      - one of CameraSelect
                addr        - start address
                bufp        - buffer to read
                length      - length of bufp

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_ReadRegisters(CameraSelect camera, u8 addr, u8* bufp, size_t length);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetParamsAsync

  Description:  set register as reg = (reg & ~mask) | (bits & mask);

  Arguments:    camera      - one of CameraSelect
                addr        - address to access
                bits        - bits to set
                mask        - mask to touch
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_SetParamsAsync(CameraSelect camera, u8 addr, u8 bits, u8 mask, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetParams

  Description:  set register as reg = (reg & ~mask) | (bits & mask);

  Arguments:    camera      - one of CameraSelect
                addr        - address to access
                bits        - bits to set
                mask        - mask to touch

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_SetParams(CameraSelect camera, u8 addr, u8 bits, u8 mask);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetFlagsAsync

  Description:  set register as reg |= bits;

  Arguments:    camera      - one of CameraSelect
                addr        - address to access
                bits        - bits to set
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_SetFlagsAsync(CameraSelect camera, u8 addr, u8 bits, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetFlags

  Description:  set register as reg |= bits;

  Arguments:    camera      - one of CameraSelect
                addr        - address to access
                bits        - bits to set

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_SetFlags(CameraSelect camera, u8 addr, u8 bits);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ClearFlagsAsync

  Description:  set register as reg &= ~bits;

  Arguments:    camera      - one of CameraSelect
                addr        - address to access
                bits        - bits to clear
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_ClearFlagsAsync(CameraSelect camera, u8 addr, u8 bits, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ClearFlags

  Description:  set register as reg &= ~bits;

  Arguments:    camera      - one of CameraSelect
                addr        - address to access
                bits        - bits to set

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_ClearFlags(CameraSelect camera, u8 addr, u8 bits);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInitAsync

  Description:  initialize camera registers via I2C
                async version.

  Arguments:    camera      - one of CameraSelect
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CInitAsync(CameraSelect camera, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize camera registers via I2C
                sync version.

  Arguments:    camera      - one of CameraSelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CInit(CameraSelect camera);
#if 0
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPresetAsync

  Description:  set camera registers with specified preset via I2C
                async version

  Arguments:    camera      - one of CameraSelect
                preset      - preset type
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CPresetAsync(CameraSelect camera, CameraPreset preset, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPreset

  Description:  set camera registers with specified preset via I2C
                sync version.

  Arguments:    camera      - one of CameraSelect
                preset      - preset type

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CPreset(CameraSelect camera, CameraPreset preset);
#endif
/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPreSleepAsync

  Description:  pre-sleep process in camera registers via I2C
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CPreSleepAsync(CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPreSleep

  Description:  pre-sleep process in camera registers via I2C
                sync version.

  Arguments:    None.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CPreSleep(void);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPostSleepAsync

  Description:  post-sleep process in camera registers via I2C
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CPostSleepAsync(CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPostSleep

  Description:  post-sleep process in camera registers via I2C
                sync version.

  Arguments:    None.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CPostSleep(void);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetCroppingAsync

  Description:  set offset and size

  Arguments:    camera      - one of CameraSelect
                x_off       - x offset to start capturing
                y_off       - y offset to start capturing
                width       - width of image
                height      - height of image
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_SetCroppingAsync(CameraSelect camera, u16 x_off, u16 y_off, u16 width, u16 height, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetCropping

  Description:  set offset and size

  Arguments:    camera      - one of CameraSelect
                x_off       - x offset to start capturing
                y_off       - y offset to start capturing
                width       - width of image
                height      - height of image

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_SetCropping(CameraSelect camera, u16 x_off, u16 y_off, u16 width, u16 height);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_PauseAsync

  Description:  pause camera via I2C
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_PauseAsync(CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Pause

  Description:  pause camera via I2C
                sync version.

  Arguments:    None.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_Pause(void);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ResumeAsync

  Description:  resume camera from pausing via I2C
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_ResumeAsync(CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Resume

  Description:  resume camera from pausing via I2C
                sync version.

  Arguments:    None.

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_Resume(void);

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CAMERA_CAMERA_API_H_ */
