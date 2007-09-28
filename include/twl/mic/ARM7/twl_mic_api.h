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

#ifndef TWL_MIC_API_H_
#define TWL_MIC_API_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/
 
typedef void (*TWLMICFullCallback) (u8 loop);

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

typedef enum
{
    MIC_INTR_DISABLE        =  (0x0UL << REG_SND_MICCNT_IM_SHIFT),
    MIC_INTR_HALF           =  (0x1UL << REG_SND_MICCNT_IM_SHIFT),
    MIC_INTR_OVERFLOW       =  (0x2UL << REG_SND_MICCNT_IM_SHIFT),
    MIC_INTR_HALF_OVERFLOW  =  (0x3UL << REG_SND_MICCNT_IM_SHIFT)
}
MICIntrCond;

typedef enum
{
    MIC_SMP_ALL             =  (0x0UL << REG_SND_MICCNT_FIFO_SMP_SHIFT),
    MIC_SMP_1_2             =  (0x1UL << REG_SND_MICCNT_FIFO_SMP_SHIFT),
    MIC_SMP_1_3             =  (0x2UL << REG_SND_MICCNT_FIFO_SMP_SHIFT),
    MIC_SMP_1_4             =  (0x3UL << REG_SND_MICCNT_FIFO_SMP_SHIFT)
}
MICSampleRate;

// �}�C�N�p�^�C�}�[��`
#define     reg_MIC_TMCNT_L             reg_OS_TM3CNT_L
#define     reg_MIC_TMCNT_H             reg_OS_TM3CNT_H
#define     MIC_TMCNT_H_E_MASK          REG_OS_TM3CNT_H_E_MASK


/*---------------------------------------------------------------------------*
    �\���̒�`
 *---------------------------------------------------------------------------*/
// MIC���C�u�����p���[�N�\����

typedef struct _TWLMICParam
{
	u8      dmaNo;						// DMA No
	MICSampleRate frequency;			// �T���v�����O�p�x
	u8      loop;						// ���[�v
    void*   buffer;                     // �f�[�^�X�g�A��̃o�b�t�@
    u32     size;                       // �����T���v�����O�i�[��o�b�t�@�T�C�Y
    TWLMICFullCallback callback;
}
TWLMICParam;

/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSampling

  Description:  start MIC

  Arguments:    

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_MIC_DoSampling( u16* buffer );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StartAutoSampling

  Description:  start MIC

  Arguments:    id : slave id

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_MIC_StartAutoSampling( TWLMICParam param );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSampling

  Description:  stop MIC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_MIC_StopAutoSampling( void );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddress

  Description:  stop MIC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void* TWL_MIC_GetLastSamplingAddress( void );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGain

  Description:  TWL���[�h�̃}�C�N�Q�C��(PGAB)��ݒ肵�܂��B
				PGAB��AutoGainControl�������ɂȂ��Ă���Ƃ��̂ݗL���ł��B

  Arguments:    gain : �ݒ�Q�C���i0�`119 = 0�`59.5dB�j

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_MIC_SetAmpGain( u8 gain );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetAmpGain

  Description:  TWL���[�h�̃}�C�N�Q�C��(PGAB)���擾���܂��B

  Arguments:    None

  Returns:      gain
 *---------------------------------------------------------------------------*/
u8 TWL_MIC_GetAmpGain( void );

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_MIC_API_H_ */
