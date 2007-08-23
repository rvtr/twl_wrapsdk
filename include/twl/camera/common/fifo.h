/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera - include
  File:     fifo.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CAMERA_FIFO_H_
#define TWL_CAMERA_FIFO_H_

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
// �v���g�R���֘A��`
#define CAMERA_PXI_CONTINUOUS_PACKET_MAX       4           // �A���p�P�b�g�̍ő�A����
#define CAMERA_PXI_DATA_SIZE_MAX               ((CAMERA_PXI_CONTINUOUS_PACKET_MAX-1)*3+1) // �ő�f�[�^��

#define CAMERA_PXI_START_BIT                   0x02000000  // �擪�p�P�b�g���Ӗ�����
#define CAMERA_PXI_RESULT_BIT                  0x00008000  // PXI�̉���������

/* �擪�p�P�b�g�݂̂̋K�� */
#define CAMERA_PXI_DATA_NUMS_MASK              0x00ff0000  // �f�[�^���̈�
#define CAMERA_PXI_DATA_NUMS_SHIFT             16          // �f�[�^���ʒu
#define CAMERA_PXI_COMMAND_MASK                0x00007f00  // �R�}���h�i�[�����̃}�X�N
#define CAMERA_PXI_COMMAND_SHIFT               8           // �R�}���h�i�[�����̈ʒu
#define CAMERA_PXI_1ST_DATA_MASK               0x000000ff  // �擪�p�P�b�g�̃f�[�^�̈�
#define CAMERA_PXI_1ST_DATA_SHIFT              0           // �擪�p�P�b�g�̃f�[�^�ʒu

/* �㑱�p�P�b�g�݂̂̋K�� */
#define CAMERA_PXI_DATA_MASK                   0x00ffffff  // �f�[�^�̈�
#define CAMERA_PXI_DATA_SHIFT                  0           // �f�[�^�ʒu

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// PXI�R�}���h��`
typedef enum CAMERAPxiCommand
{
    CAMERA_PXI_COMMAND_INIT                         = 0x00, // ������

    CAMERA_PXI_COMMAND_ACTIVATE                     = 0x10, // �A�N�e�B�u�I��
    CAMERA_PXI_COMMAND_OUTPUT_WITH_DUAL_ACTIVATION  = 0x11, // �o�͑I��(�����A�N�e�B�u)

    CAMERA_PXI_COMMAND_RESIZE                       = 0x30, // �T�C�Y�ݒ�
    CAMERA_PXI_COMMAND_FRAME_RATE                   = 0x31, // �t���[�����[�g�ݒ�
    CAMERA_PXI_COMMAND_EFFECT                       = 0x32, // �G�t�F�N�g�ݒ�
    CAMERA_PXI_COMMAND_FLIP                         = 0x33, // ���]�ݒ�

    CAMERA_PXI_COMMAND_UNKNOWN
}
CAMERAPxiCommand;

// PXI�R�}���h�T�C�Y��`
typedef enum CAMERAPxiSize
{
    CAMERA_PXI_SIZE_INIT                            = 1,    // camera

    CAMERA_PXI_SIZE_ACTIVATE                        = 1,    // camera
    CAMERA_PXI_SIZE_OUTPUT_WITH_DUAL_ACTIVATION     = 1,    // camera

    CAMERA_PXI_SIZE_RESIZE                          = 5,    // camera, (u16)width, (u16)height
    CAMERA_PXI_SIZE_FRAME_RATE                      = 2,    // camera, rate
    CAMERA_PXI_SIZE_EFFECT                          = 2,    // camera, effect
    CAMERA_PXI_SIZE_FLIP                            = 2,    // camera, flip

    CAMERA_PXI_SIZE_UNKNOWN
}
CAMERAPxiSize;

// ������`
typedef enum CAMERAPxiResult
{
    CAMERA_PXI_RESULT_SUCCESS = 0,        // �������� (void/void*�^) // �ꍇ�ɂ��㑱�p�P�b�g����
    CAMERA_PXI_RESULT_SUCCESS_TRUE = 0,   // �������� (BOOL�^)
    CAMERA_PXI_RESULT_SUCCESS_FALSE,      // �������� (BOOL�^)
    CAMERA_PXI_RESULT_INVALID_COMMAND,    // �s����PXI�R�}���h
    CAMERA_PXI_RESULT_INVALID_PARAMETER,  // �s���ȃp�����[�^
    CAMERA_PXI_RESULT_ILLEGAL_STATUS,     // CAMERA�̏�Ԃɂ�菈�������s�s��
    CAMERA_PXI_RESULT_BUSY,               // ���̃��N�G�X�g�����s��
    CAMERA_PXI_RESULT_FATAL_ERROR,        // ���̑����炩�̌����ŏ����Ɏ��s
    CAMERA_PXI_RESULT_MAX
}
CAMERAPxiResult;


/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_CAMERA_FIFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
