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

// �������ʒ�`
typedef enum CAMERAResult
{
    CAMERA_RESULT_SUCCESS = 0,
    CAMERA_RESULT_SUCCESS_TRUE = 0,
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

// �R�[���o�b�N
typedef void (*CAMERACallback)(CAMERAResult result, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Init

  Description:  CAMERA���C�u����������������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_Init(void);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_End

  Description:  CAMERA���C�u�������I������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_End(void);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Start

  Description:  �L���v�`�����J�n������API�B�؂�ւ��ɂ��g����B
                sync version only

  Arguments:    camera      - one of CameraSelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_Start(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Stop

  Description:  �L���v�`�����~������API�B
                sync version only

  Arguments:    None

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_Stop(void);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInitAsync

  Description:  initialize camera registers via I2C
                async version.

  Arguments:    camera      - one of CameraSelect
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

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

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivateAsync

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                async version

  Arguments:    camera      - one of CameraSelect
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CActivateAsync(CameraSelect camera, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivate

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                sync version.

  Arguments:    camera      - one of CameraSelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CActivate(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CResizeAsync

  Description:  resize CAMERA output image
                async version

  Arguments:    camera      - one of CameraResize
                width       - width of the image
                height      - height of the image
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CResizeAsync(CameraSelect camera, u16 width, u16 height, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERAi_I2CResize

  Description:  resize CAMERA output image
                sync version.

  Arguments:    camera      - one of CameraResize
                width       - width of the image
                height      - height of the image

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CResize(CameraSelect camera, u16 width, u16 height);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRateAsync

  Description:  set CAMERA frame rate (0 means automatic)
                async version

  Arguments:    camera      - one of CameraSelect
                rate        - frame rate (fps)
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFrameRateAsync(CameraSelect camera, int rate, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRate

  Description:  set CAMERA frame rate (0 means automatic)
                sync version.

  Arguments:    camera      - one of CameraSelect
                rate        - frame rate (fps)

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFrameRate(CameraSelect camera, int rate);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffectAsync

  Description:  set CAMERA frame rate (0 means automatic)
                async version

  Arguments:    camera      - one of CameraSelect
                effect      - one of CameraEffect
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CEffectAsync(CameraSelect camera, CameraEffect effect, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffect

  Description:  set CAMERA frame rate (0 means automatic)
                sync version.

  Arguments:    camera      - one of CameraSelect
                effect      - one of CameraEffect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CEffect(CameraSelect camera, CameraEffect effect);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipAsync

  Description:  set CAMERA frame rate (0 means automatic)
                async version

  Arguments:    camera      - one of CameraSelect
                flip        - one of CameraFlip
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFlipAsync(CameraSelect camera, CameraFlip flip, CAMERACallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlip

  Description:  set CAMERA frame rate (0 means automatic)
                sync version.

  Arguments:    camera      - one of CameraSelect
                flip        - one of CameraFlip

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFlip(CameraSelect camera, CameraFlip flip);

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CAMERA_CAMERA_API_H_ */
