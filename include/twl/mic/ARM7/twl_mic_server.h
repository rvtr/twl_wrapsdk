/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     control.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_MIC_SERVER_H_
#define TWL_MIC_SERVER_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define     TWL_MIC_MESSAGE_ARRAY_MAX       4       // �X���b�h�����p���b�Z�[�W�L���[�̃T�C�Y
#define     TWL_MIC_THREAD_STACK_SIZE       512     // �X���b�h�̃X�^�b�N�T�C�Y
#define     TWL_MIC_MESSAGE_ARGS_MAX    	7		// ���b�Z�[�W�����̍ő�l

// �}�C�N�Ɋւ�������I�ȏ��
typedef enum _TWLMICStatus
{
    TWL_MIC_STATUS_READY = 0,              // �ʏ푀��҂����
    TWL_MIC_STATUS_ONE_SAMPLING_START,     // �P���T���v�����O�J�n�҂�
    TWL_MIC_STATUS_AUTO_START,             // �����T���v�����O�J�n�҂�
    TWL_MIC_STATUS_AUTO_SAMPLING,          // �����T���v�����O��
    TWL_MIC_STATUS_AUTO_END,               // �����T���v�����O�I���҂�
    TWL_MIC_STATUS_END_WAIT                // �����T���v�����O�����҂�(�o�b�t�@FULL���̎����I��)
}
TWLMICStatus;

/*---------------------------------------------------------------------------*
    �\���̒�`
 *---------------------------------------------------------------------------*/
 
// TWL�}�C�N�p���b�Z�[�W�f�[�^�\����
typedef struct _TWLMICMessageData
{
    TWLMICPxiCommand     command;
    u32     arg[TWL_MIC_MESSAGE_ARGS_MAX];
}
TWLMICMessageData;
 
// MIC���C�u�����p���[�N�\����

typedef struct _TWLMICServerWork
{
    OSMessageQueue msgQ;               							// �X���b�h�����p���b�Z�[�W�L���[
    OSMessage msgArray[TWL_MIC_MESSAGE_ARRAY_MAX];				// ���b�Z�[�W���i�[����o�b�t�@
    TWLMICMessageData entry[TWL_MIC_MESSAGE_ARRAY_MAX];			// ���b�Z�[�W�̎���
    u32     entryIndex;											// �G���g���[�C���f�b�N�X
    OSThread thread;                   							// TWL-MIC�p�X���b�h
    u64     stack[TWL_MIC_THREAD_STACK_SIZE / sizeof(u64)];		// TWL-MIC�p�X���b�h�̃X�^�b�N�i�������ł���j
    TWLMICStatus status;                  						// �}�C�N������ԊǗ��ϐ�
    TWLMICPxiCommand    command;        						// ARM9����̃R�}���h�ۑ��p
    u8      data[TWL_MIC_PXI_DATA_SIZE_MAX];					// ARM9����̃f�[�^�ۑ��p
    u8      current;                    						// ��M�ς݃f�[�^�� (�o�C�g�P��)
    u8      total;                      						// �ŏI�f�[�^�� (1 + �㑱�R�}���h*3)
}
TWLMICServerWork;

/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_InitServer

  Description:  ARM7���Ƃ��Ƃ���s�����߂̏������s���܂��B

  Arguments:    priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TWL_MIC_InitServer( u32 priority );	// �������A�y�уX���b�h���J�n

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_MIC_SERVER_H_ */
