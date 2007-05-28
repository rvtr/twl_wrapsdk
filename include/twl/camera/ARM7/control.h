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

#ifndef TWL_CAMERA_CONTROL_H_
#define TWL_CAMERA_CONTROL_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define     CAMERA_MESSAGE_ARRAY_MAX       4       // �X���b�h�����p���b�Z�[�W�L���[�̃T�C�Y
#define     CAMERA_THREAD_STACK_SIZE       256     // �X���b�h�̃X�^�b�N�T�C�Y

/*---------------------------------------------------------------------------*
    �\���̒�`
 *---------------------------------------------------------------------------*/
// CAMERA���C�u�����p���[�N�\����

typedef struct CAMERAWork
{
    OSMessageQueue msgQ;               // �X���b�h�����p���b�Z�[�W�L���[
    OSMessage msgArray[CAMERA_MESSAGE_ARRAY_MAX];
    // ���b�Z�[�W���i�[����o�b�t�@
    OSThread thread;                   // CAMERA�p�X���b�h
    u64     stack[CAMERA_THREAD_STACK_SIZE / sizeof(u64)];
    // CAMERA�p�X���b�h�̃X�^�b�N

    CAMERAPxiCommand    command;        // �R�}���h���
    u8      current;                    // ��M�ς݃f�[�^�� (�o�C�g�P��)
    u8      total;                      // �ŏI�f�[�^�� (1 + �㑱�R�}���h*3)
    u8      data[CAMERA_PXI_DATA_SIZE_MAX];
    // ARM9����̃f�[�^�ۑ��p
}
CAMERAWork;

/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/
void    CAMERA_Init(u32 priority);         // �������A�y�уX���b�h���J�n
void    CAMERA_Lock(void);                 // ARM7���Ŏg�����߂Ƀ��b�N����
BOOL    CAMERA_TryLock(void);              // ARM7���Ŏg�����߂Ƀ��b�N�����݂�
void    CAMERA_Unlock(void);               // ARM7���̃��b�N����������

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CAMERA_CONTROL_H_ */
