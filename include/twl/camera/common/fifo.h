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
    CAMERA_PXI_COMMAND_SET_STBYN                    = 0x00, // STBYN����
    // I2C��{����
    CAMERA_PXI_COMMAND_WRITE_REGISTERS              = 0x10,
    CAMERA_PXI_COMMAND_READ_REGISTERS               = 0x11,
    CAMERA_PXI_COMMAND_SET_PARAMS                   = 0x12,
    CAMERA_PXI_COMMAND_SET_FLAGS                    = 0x13,
    CAMERA_PXI_COMMAND_CLEAR_FLAGS                  = 0x14,
    // I2C���p����
    CAMERA_PXI_COMMAND_I2C_INIT                     = 0x20, // �ėp������
    CAMERA_PXI_COMMAND_I2C_PRESET                   = 0x21, // �e��v���Z�b�g

    CAMERA_PXI_COMMAND_I2C_PRE_SLEEP                = 0x28, // �X���[�v�O����
    CAMERA_PXI_COMMAND_I2C_POST_SLEEP               = 0x29, // �X���[�v�㏈��

    CAMERA_PXI_COMMAND_I2C_SET_CROPPING             = 0x30, // �ʒu�ƃT�C�Y�ݒ�

    CAMERA_PXI_COMMAND_I2C_PAUSE                    = 0x38, // �ꎞ��~
    CAMERA_PXI_COMMAND_I2C_RESUME                   = 0x39  // ���A
}
CAMERAPxiCommand;

// PXI�R�}���h�T�C�Y��`
typedef enum CAMERAPxiSize
{
    CAMERA_PXI_SIZE_SET_STBYN                       = 1, // BOOL
    // I2C��{����
    CAMERA_PXI_SIZE_WRITE_REGISTERS                 = 3,    // addr, size, data...
    CAMERA_PXI_SIZE_READ_REGISTERS                  = 2,    // addr, size
    CAMERA_PXI_SIZE_SET_PARAMS                      = 3,    // addr, bits, mask
    CAMERA_PXI_SIZE_SET_FLAGS                       = 2,    // addr, bits
    CAMERA_PXI_SIZE_CLEAR_FLAGS                     = 2,    // addr, bits
    // I2C���p����
    CAMERA_PXI_SIZE_I2C_INIT                        = 0,
    CAMERA_PXI_SIZE_I2C_PRESET                      = 1,    // preset

    CAMERA_PXI_SIZE_I2C_PRE_SLEEP                   = 0,
    CAMERA_PXI_SIZE_I2C_POST_SLEEP                  = 0,

    CAMERA_PXI_SIZE_I2C_SET_CROPPING                = 8,    // (u16)x_offset, (u16)y_offset, (u16)width, (u16)height

    CAMERA_PXI_SIZE_I2C_PAUSE                       = 0,
    CAMERA_PXI_SIZE_I2C_RESUME                      = 0
}
CAMERAPxiSize;

// ������`
typedef enum CAMERAPxiResult
{
    CAMERA_PXI_RESULT_SUCCESS = 0,        // �������� (void/void*�^) // �ꍇ�ɂ��㑱�p�P�b�g����
    CAMERA_PXI_RESULT_SUCCESS_TRUE,       // �������� (BOOL�^)
    CAMERA_PXI_RESULT_SUCCESS_FALSE,      // �������� (BOOL�^)
    CAMERA_PXI_RESULT_INVALID_COMMAND,    // �s����PXI�R�}���h
    CAMERA_PXI_RESULT_INVALID_PARAMETER,  // �s���ȃp�����[�^
    CAMERA_PXI_RESULT_ILLEGAL_STATUS,     // RTC�̏�Ԃɂ�菈�������s�s�\
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
