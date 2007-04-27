/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - aes
  File:     aes_sp.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_AES_SP_H_
#define TWL_AES_SP_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define     AES_MESSAGE_ARRAY_MAX       4       // �X���b�h�����p���b�Z�[�W�L���[�̃T�C�Y
#define     AES_THREAD_STACK_SIZE       256     // �X���b�h�̃X�^�b�N�T�C�Y

typedef enum AESLock
{
    AES_UNLOCKED = 0,
    AES_LOCKED_BY_ARM7,
    AES_LOCKED_BY_ARM9
} AESLock;

/*---------------------------------------------------------------------------*
    �\���̒�`
 *---------------------------------------------------------------------------*/
// AES���C�u�����p���[�N�\����

typedef struct AESWork
{
    OSMessageQueue msgQ;               // �X���b�h�����p���b�Z�[�W�L���[
    OSMessage msgArray[AES_MESSAGE_ARRAY_MAX];
    // ���b�Z�[�W���i�[����o�b�t�@
    OSThread thread;                   // AES�p�X���b�h
    u64     stack[AES_THREAD_STACK_SIZE / sizeof(u64)];
    // AES�p�X���b�h�̃X�^�b�N

    AESLock locked;                     // ���b�N

    u8      command;                    // �R�}���h���
    u8      current;                    // ��M�ς݃f�[�^�� (�o�C�g�P��)
    u8      total;                      // �ŏI�f�[�^�� (1 + �㑱�R�}���h*3)
    u8      data[AES_PXI_DATA_SIZE_MAX];
    // ARM9����̃f�[�^�ۑ��p
}
AESWork;

/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/
void    AES_Init(u32 priority);         // �������A�y�уX���b�h���J�n
void    AES_Lock(void);                 // ARM7���Ŏg�����߂Ƀ��b�N����
BOOL    AES_TryLock(void);              // ARM7���Ŏg�����߂Ƀ��b�N�����݂�
void    AES_Unlock(void);               // ARM7���̃��b�N����������

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_AES_SP_H_ */
