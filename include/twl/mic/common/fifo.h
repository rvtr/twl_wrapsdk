/*---------------------------------------------------------------------------*
  Project:  TwlSDK - mic - include
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
#ifndef TWL_MIC_FIFO_H_
#define TWL_MIC_FIFO_H_

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
// �v���g�R���֘A��`
#define TWL_MIC_PXI_CONTINUOUS_PACKET_MAX   			10  // �A���p�P�b�g�̍ő�A����
#define TWL_MIC_PXI_DATA_SIZE_MAX           ((TWL_MIC_PXI_CONTINUOUS_PACKET_MAX-1)*3+1) // �ő�f�[�^��

#define TWL_MIC_PXI_START_BIT                   0x02000000  // �擪�p�P�b�g���Ӗ�����
#define TWL_MIC_PXI_RESULT_BIT                  0x00008000  // PXI�̉���������

/* �擪�p�P�b�g�݂̂̋K�� */
#define TWL_MIC_PXI_DATA_NUMS_MASK              0x00ff0000  // �f�[�^���̈�
#define TWL_MIC_PXI_DATA_NUMS_SHIFT             16          // �f�[�^���ʒu
#define TWL_MIC_PXI_COMMAND_MASK                0x00007f00  // �R�}���h�i�[�����̃}�X�N
#define TWL_MIC_PXI_COMMAND_SHIFT               8           // �R�}���h�i�[�����̈ʒu
#define TWL_MIC_PXI_1ST_DATA_MASK               0x000000ff  // �擪�p�P�b�g�̃f�[�^�̈�
#define TWL_MIC_PXI_1ST_DATA_SHIFT              0           // �擪�p�P�b�g�̃f�[�^�ʒu

/* �㑱�p�P�b�g�݂̂̋K�� */
#define TWL_MIC_PXI_DATA_MASK                   0x00ffffff  // �f�[�^�̈�
#define TWL_MIC_PXI_DATA_SHIFT                  0           // �f�[�^�ʒu

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// PXI�R�}���h��`
typedef enum _TWLMICPxiCommand
{
	TWL_MIC_PXI_COMMAND_ONE_SAMPLING			  = 0x34, // �P���T���v�����O
    TWL_MIC_PXI_COMMAND_AUTO_START                = 0x35, // �����T���v�����O�X�^�[�g
    TWL_MIC_PXI_COMMAND_AUTO_STOP                 = 0x36, // �����T���v�����O�X�g�b�v
	TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS = 0x37, // �ŐV�T���v�����O�f�[�^�̃A�h���X�擾
	TWL_MIC_PXI_COMMAND_SET_AMP_GAIN 			  = 0x38, // PGAB�ݒ�
	TWL_MIC_PXI_COMMAND_GET_AMP_GAIN              = 0x39, // PGAB�擾
	TWL_MIC_PXI_COMMAND_MIC_BUFFER_FULL			  = 0x50, // �o�b�t�@�t���̒ʒm
    TWL_MIC_PXI_COMMAND_UNKNOWN
}
TWLMICPxiCommand;

// PXI�R�}���h�t���f�[�^�T�C�Y��`
typedef enum TWLMICPxiSize
{
	TWL_MIC_PXI_SIZE_ONE_SAMPLING				 = 0,    // 0
    TWL_MIC_PXI_SIZE_AUTO_START                  = 11,   // dmaNo(u8) buffer(u32) size(u32) frequency(u8) loop_erable(u8)
    TWL_MIC_PXI_SIZE_AUTO_STOP                   = 0,    // 0
    TWL_MIC_PXI_SIZE_GET_LAST_SAMPLING_ADDRESS   = 0,	 // 0
    TWL_MIC_PXI_SIZE_SET_AMP_GAIN   			 = 1,	 // gain(u8)
    TWL_MIC_PXI_SIZE_GET_AMP_GAIN   			 = 0,	 // 0
    TWL_MIC_PXI_SIZE_UNKNOWN
}
TWLMICPxiSize;

// ������`
typedef enum TWLMICPxiResult
{
    TWL_MIC_PXI_RESULT_SUCCESS = 0,        // �������� (void/void*�^) // �ꍇ�ɂ��㑱�p�P�b�g����
    TWL_MIC_PXI_RESULT_INVALID_COMMAND,    // �s����PXI�R�}���h
    TWL_MIC_PXI_RESULT_INVALID_PARAMETER,  // �s���ȃp�����[�^
    TWL_MIC_PXI_RESULT_ILLEGAL_STATUS,     // ��Ԃɂ�菈�������s�s�\
    TWL_MIC_PXI_RESULT_BUSY,               // ���̃��N�G�X�g�����s��
    TWL_MIC_PXI_RESULT_FATAL_ERROR,        // ���̑����炩�̌����ŏ����Ɏ��s
    TWL_MIC_PXI_RESULT_MAX
}
TWLMICPxiResult;

/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_MIC_FIFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
