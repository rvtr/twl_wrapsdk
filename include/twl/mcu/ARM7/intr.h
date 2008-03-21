/*---------------------------------------------------------------------------*
  Project:  TwlSDK - mcu - include
  File:     intr.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-02-15#$
  $Rev: 4187 $
  $Author: yutaka $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MCU_INTR_H_
#define TWL_MCU_INTR_H_

#include <twl/types.h>
#include <twl/mcu/ARM7/mcu_reg.h>

/*
    CPU���荞��
*/
#define MCU_IE  OS_IE_GPIO33_2

/*
    MCU���荞�݃r�b�g��
*/
#define MCU_IRQ_TABLE_MAX   8

/*
    MCU���荞�ݎ��
*/
// �O��DC�v���O�̑}������������ (�A�v���P�[�V�������[�e�B���e�B)
//#define MCU_IE_EXTERNAL_DC_TRIGGER      MCU_REG_IRQ_EXTDC_MASK
// �o�b�e���؂�ŏI�\������ɂȂ��� (�A�v���P�[�V�������[�e�B���e�B)
#define MCU_IE_BATTERY_LOW_TRIGGER      MCU_REG_IRQ_BATTLOW_MASK
// �o�b�e���؂ꐅ��ɂȂ��� (�d��OFF�Ɠ��l�̏����J�n)
#define MCU_IE_BATTERY_EMPTY_TRIGGER    MCU_REG_IRQ_BATTEMP_MASK
// �d���{�^�����s�����Ԃ𒴂��ĉ����ꂽ (���Z�b�g/�d��OFF�̂ǂ��炩�̔������m��)
#define MCU_IE_POWER_SWITCH_PRESSED     MCU_REG_IRQ_PWSW_MASK
// �d���{�^���ɂ��d��OFF�w��
#define MCU_IE_POWER_OFF_REQUEST        MCU_REG_IRQ_PWOFF_MASK
// �d���{�^���ɂ�郊�Z�b�g�w��
#define MCU_IE_RESET_REQUEST            MCU_REG_IRQ_RESET_MASK

/*
    �u���b�N�ݒ�
*/
#define MCU_BLOCK   OS_MESSAGE_BLOCK
#define MCU_NOBLOCK OS_MESSAGE_NOBLOCK

#ifdef _cplusplus
extern "C" {
#endif

/*
    ���荞�݃n���h��
*/
typedef void (*MCUIrqFunction)(void);

/*
    �d���{�^���̏��
*/
typedef enum MCUPwswStatus
{
    MCU_PWSW_UNKNOWN,       // ������Ă��Ȃ�
    MCU_PWSW_IN_PROGRESS,   // �����Ă��邪���m��
    MCU_PWSW_RESET,         // ���Z�b�g�Ɗm�肵��
    MCU_PWSW_POWER_OFF,     // �d��OFF�Ɗm�肵��

    MCU_PWSW_MAX
}
MCUPwswStatus;

/*---------------------------------------------------------------------------*
  Name:         MCU_InitIrq

  Description:  MCU�̊��荞�݂𗘗p�\�Ƃ���

  Arguments:    priority        ���荞�ݏ����X���b�h�̗D��x(����̂ݗL��)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MCU_InitIrq(u32 priority);

/*---------------------------------------------------------------------------*
  Name:         MCU_SetIrqFunction

  Description:  MCU���荞�݃n���h���̓o�^

  Arguments:    intrBit         �ݒ肷��MCU���荞�ݗv��
                function        ���荞�݃n���h��

  Returns:      None
 *---------------------------------------------------------------------------*/
void MCU_SetIrqFunction(u8 intrBit, MCUIrqFunction function);

/*---------------------------------------------------------------------------*
  Name:         MCU_GetIrqFunction

  Description:  MCU���荞�݃n���h���̎擾

  Arguments:    intrBit         �ݒ肷��MCU���荞�ݗv��

  Returns:      �ݒ�ς݂̊��荞�݃n���h��
 *---------------------------------------------------------------------------*/
MCUIrqFunction MCU_GetIrqFunction(u8 intrBit);

/*---------------------------------------------------------------------------*
  Name:         MCU_CallIrqFunction

  Description:  �����̊��荞�݂ɑΉ������R�[���o�b�N���Ăяo���܂��B
                PwswStatus�͍X�V���܂���B

  Arguments:    intrBit         ���荞�݃n���h�����Ăяo������
                                MCU���荞�ݗv���̃r�b�gOR

  Returns:      None
 *---------------------------------------------------------------------------*/
void MCU_CallIrqFunction(u8 intrBit);

/*---------------------------------------------------------------------------*
  Name:         MCU_CheckIrq

  Description:  MCU�̊��荞�݂��������Ă��Ȃ����m�F����B
                MCU_REG_IRQ_ADDR�𒼐�Read�������ɂ�����g�p���Ă��������B
                �x�����Ċ��荞�݃n���h�����Ăяo�������ꍇ�́A����
                �^�C�~���O��MCU_CallIrqFunction()���Ăяo���Ă��������B

  Arguments:    callHandler     �o�^�ς݃n���h�����Ăяo�����ǂ���

  Returns:      �������Ă������荞�݂ɑΉ�����MCU_IE_*�̃r�b�gOR
 *---------------------------------------------------------------------------*/
u8 MCU_CheckIrq(BOOL callHandler);

/*---------------------------------------------------------------------------*
  Name:         MCU_GetPwswStatus

  Description:  MCU�̓d���{�^���̏�Ԃ��擾����
                �u���b�N����ꍇ�AIN_PROGRESS�ł���΁ARESET�܂���POWER_OFF��
                �Ȃ�܂�OS_Sleep(1)����

  Arguments:    block           �m�肵�Ă��Ȃ��Ȃ�m�肷��܂ő҂��ǂ���
                                MCU_BLOCK: �҂�
                                MCU_NOBLOCK: �҂��Ȃ�

  Returns:      one of MCUPwswStatus
 *---------------------------------------------------------------------------*/
MCUPwswStatus MCU_GetPwswStatus(s32 block);

#if 0
/*---------------------------------------------------------------------------*
  Name:         MCU_ResetPwswStatus

  Description:  MCU�̓d���{�^���̏�Ԃ��N���A���� (�Ȃ��������Ƃɂ���)
                MCU_PWSW_IN_PROGRESS�̏ꍇ�͖��������

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MCU_ResetPwswStatus(void);
#endif

#ifdef _cplusplus
} /* extern "C" */
#endif

#endif  // TWL_MCU_INTR_H_
