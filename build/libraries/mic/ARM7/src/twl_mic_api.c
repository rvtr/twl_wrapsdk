/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - twlmic
  File:     twl_mic_api.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  ���ݖ������̋@�\�Ƃ��Ĉȉ���2������܂��B
�@�EAuto Gain Control (�Q�C���������ߋ@�\�j
�@�EIIR Filter (�������o�̓t�B���^ : High/Low Pass Filter �Ȃǁj

�@�A���A�����̋@�\�� CODEC �̃Z�J���h�\�[�X�i�ł��݊�����ۂĂ邩
�@�ǂ����m�F���Ƃ�Ă��Ȃ����߁A�{�c��\��������܂��B
 ----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*

��TWL�ɂ�����}�C�N�f�[�^�̏E������

���݂� FIFO Half DMA�N������ �������B


���� FIFO Half DMA�N������ ����

�u���b�N�]���T�C�Y�@�@�F�W���[�h
�C���^�[�o���@�@�@�@�@�F�O
�]�����[�h�@�@�@�@�@�@�F�W���[�h
���]�����[�h�@�@�@�@�@�F�o�b�t�@�T�C�Y
�R���e�B�j���A�X���[�h�F�n�e�e

�������

FIFO�ɔ����f�[�^�����܂�x��DMA�N���v����������
�W���[�h�]�������B
�o�b�t�@�T�C�Y���̓]����������Ɋ��荞�݂�������̂�
������DMA���Đݒ�B

�������b�g��
�EFIFO��DMA�̃^�C�~���O����������ƍ����B
�E�F�X�Ȏ��g���ɑΉ����₷���B

���f�����b�g��
�EDMA���ǂ��܂Ői�񂾂�������Ȃ�����MIC_GetLastSamplingAddress
�@�Ő��m�ȃA�h���X�����Ȃ��B�`�b�N���g�p���đ�̂̈ʒu��
�@�\�����邱�Ƃ͉\�B

���� �C���^�[�o������ ����

�u���b�N�]���T�C�Y�@�@�F�W���[�h
�C���^�[�o���@�@�@�@�@�F�K�ؒl
�]�����[�h�@�@�@�@�@�@�F�o�b�t�@�T�C�Y
���]�����[�h�@�@�@�@�@�F�o�b�t�@�T�C�Y
�R���e�B�j���A�X���[�h�F�n�m

�������

���g������FIFO�Ƀf�[�^�����܂�^�C�~���O���t�Z���A
�C���^�[�o�����g���Č����݂Ńu���b�NDMA�����s����B
����������DMA��FIFO�̃^�C�~���O������Ă����\��
�����邪�A�o�b�t�@�T�C�Y�]�����ɍē������\�B

�������b�g��
�EDMA�̍Đݒ肪�K�v�Ȃ�

���f�����b�g��
�E����DMA�ɂ��FIFO�Ƃ̃^�C�~���O������Ă����\��������B
�E���̂��ߑ傫�ȃo�b�t�@�T�C�Y�ɂ͌����Ȃ�
�EDMA���ǂ��܂Ői�񂾂�������Ȃ�����MIC_GetLastSamplingAddress
�@�Ő��m�ȃA�h���X�����Ȃ��B�`�b�N���g�p���đ�̂̈ʒu��
�@�\�����邱�Ƃ͉\�B

���� ���荞�ݕp������ ����

�������

FIFO�Ƀf�[�^���������܂�x��MIC���荞�݂𔭐�����
FIFO����CPU�łW���[�h�擾����B

�������b�g��
�E�o�b�t�@�̂ǂ̈ʒu�܂Ői�񂾂��𐳊m�ɔc���\�B
�@MIC_GetLastSamplingAddress�Ő��m�ȃA�h���X��Ԃ����Ƃ��ł���B

���f�����b�g��
�E�p�ɂɊ��荞�݂���������B�Ƃ����Ă����̕p�x��DS�����1/16�ɂȂ�B
�E��������MIC_GetLastSamplingAddress�ɗL�p�����Ȃ��Ȃ炱�̕����͂��肦�Ȃ��B

 ----------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/mic.h>
#include <twl/cdc.h>
#include <twl/snd/ARM7/i2s.h>

/*---------------------------------------------------------------------------*
    �}�N����`
 *---------------------------------------------------------------------------*/

#define TWL_MIC_TICK_INIT_VALUE   0

/*---------------------------------------------------------------------------*
    �ÓI�ϐ���`
 *---------------------------------------------------------------------------*/

static TWLMICParam sMicParam;	// �p�����[�^�ۑ��p
static OSTick sMicTickStart;	// �T���v�����O�J�n��Tick
static OSTick sMicTickStop;		// �T���v�����O��~��Tick

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

static void TWL_MICi_ExDmaRecvAsync( u32 dmaNo, void *dest, u32 size );
static void TWL_MICi_ExDmaInterruptHandler( void );
static void TWL_MICi_FifoInterruptHandler( void );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSampling

  Description:  �P���i16bit�j�̃T���v�����O���s���܂��B

  Arguments:    buffer : �T���v�����O���ʂ��i�[����o�b�t�@

  Returns:      None
 *---------------------------------------------------------------------------*/
void
TWL_MIC_DoSampling( u16* buffer )
{
	u32 word;
	const u32 WAIT_MAX = 100;
	u32 i;

	// �����`�F�b�N
	SDK_NULL_ASSERT( buffer );

	// �T�E���h & I2S ��HON
    SND_Enable();

    // ���m�����T���v�����O�X�^�[�g
    reg_SND_MICCNT = REG_SND_MICCNT_FIFO_CLR_MASK;
    reg_SND_MICCNT = (u16)(REG_SND_MICCNT_E_MASK | REG_SND_MICCNT_NR_MASK | 
						   MIC_INTR_DISABLE | MIC_SMP_ALL );

	// �f�[�^��2�ȏ����̂�҂�
	for (i=0;i<WAIT_MAX;i++)
	{
		if (!(reg_SND_MICCNT & REG_SND_MICCNT_FIFO_EMP_MASK))
		{
			break;
		}
	}

	// FIFO�̓��[�h�A�N�Z�X
	word = reg_SND_MIC_FIFO;

	// �T���v�����O��~
    reg_SND_MICCNT &= ~REG_SND_MICCNT_E_MASK;

	// 16bit�l���i�[
	*buffer = (u16)(word & 0x0000ffff);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StartAutoSampling

  Description:  �I�[�g�T���v�����O���J�n���܂��B
				���݃��m�����Œ�̃T���v�����O�ɂȂ��Ă��܂��B

  Arguments:    param : �I�[�g�T���v�����O�p�̃p�����[�^

  Returns:      None
 *---------------------------------------------------------------------------*/
void
TWL_MIC_StartAutoSampling( TWLMICParam param )
{
    OSIntrMode enabled;
	u32 ch;

	// �����`�F�b�N
	SDK_ASSERT( param.dmaNo >= MI_EXDMA_CH_MIN && param.dmaNo <= MI_EXDMA_CH_MAX );
	SDK_NULL_ASSERT( param.buffer );

	// DMA��~
    TWL_MIC_StopAutoSampling();

	// ���荞�݋֎~
    enabled = OS_DisableInterrupts();

	// �T�E���h & I2S ��HON
    SND_Enable();

	// DMA�ݒ�
    TWL_MICi_ExDmaRecvAsync( param.dmaNo, param.buffer, param.size );

	// DMA���荞�݂̐ݒ�
    ch = (u32)param.dmaNo - MI_EXDMA_CH_MIN;
    OS_SetIrqFunction( OS_IE_DMA4 << ch, TWL_MICi_ExDmaInterruptHandler );
    reg_OS_IF  = (OS_IE_DMA4 << ch);
    reg_OS_IE |= (OS_IE_DMA4 << ch);

	// �}�C�NFIFO���荞�݂̐ݒ�
    OS_SetIrqFunction( OS_IE_MIC, TWL_MICi_FifoInterruptHandler );
    reg_OS_IF2  = (OS_IE_MIC >> 32);
    reg_OS_IE2 |= (OS_IE_MIC >> 32);

	// �X�^�[�gTick�ݒ�
	sMicTickStart = OS_GetTick();

	// �X�g�b�vTick�N���A
	sMicTickStop = TWL_MIC_TICK_INIT_VALUE;

    // ���m�����T���v�����O�X�^�[�g
    reg_SND_MICCNT = REG_SND_MICCNT_FIFO_CLR_MASK;		// 
    reg_SND_MICCNT = (u16)(	REG_SND_MICCNT_E_MASK 	| 	// 
							REG_SND_MICCNT_NR_MASK 	| 	// 
							MIC_INTR_OVERFLOW		| 	// FIFO�j�]�Ŋ��荞�ݔ���
							param.frequency );

	// �p�����[�^�ۑ�
	sMicParam = param;

	// ���荞�ݕ��A
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSampling

  Description:  �I�[�g�T���v�����O���~���܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void
TWL_MIC_StopAutoSampling( void )
{
    OSIntrMode enabled = OS_DisableInterrupts();
    u32 dmaNo = sMicParam.dmaNo;

	// �X�g�b�vTick�ݒ�
	sMicTickStop = OS_GetTick();

	// �}�C�N��~
    reg_SND_MICCNT &= ~REG_SND_MICCNT_E_MASK;

	// �}�C�NFIFO���荞�݋֎~
    reg_OS_IE2 &= ~(OS_IE_MIC >> 32);
    reg_OS_IF2  =  (OS_IE_MIC >> 32);

    if ( MI_EXDMA_CH_MIN <= dmaNo && dmaNo <= MI_EXDMA_CH_MAX )
    {
        u32 ch = dmaNo - MI_EXDMA_CH_MIN;

		// DMA��~
        MIi_StopExDma( dmaNo );

		// DMA���荞�݋֎~
        reg_OS_IE &= ~(OS_IE_DMA4 << ch);
        reg_OS_IF  =  (OS_IE_DMA4 << ch);
    }

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddress

  Description:  �ŐV�T���v�����O�f�[�^�̊i�[�A�h���X��Ԃ��܂��B
				�}�C�NFIFO����̃f�[�^�擾�ɂ�DMA��p���܂����A
				DMA�����݂ǂ̈ʒu���������������͒m���i������܂���B
				���̂��߃T���v�����O�o�ߎ��Ԃ�茻�݂̈ʒu��\�z���܂��B

  Arguments:    None

  Returns:      �ŐV�T���v�����O�f�[�^�̃A�h���X�i�\�z�j
 *---------------------------------------------------------------------------*/
void*
TWL_MIC_GetLastSamplingAddress( void )
{
	void* adress;					// �ŐV�f�[�^�A�h���X
	OSTick past_tick;				// �o�߃`�b�N
	u32 sampling_count;				// �\�z�T���v�����O�J�E���g
	f32 one_sampling_tick;

	// �T���v�����O��~���
	if (sMicTickStop != TWL_MIC_TICK_INIT_VALUE)
	{
		past_tick = sMicTickStop - sMicTickStart;
	}
	// �T���v�����O�i�s���
	else
	{
		past_tick = OS_GetTick() - sMicTickStart;
	}

	/*
	   �T���v�����O���[�g47.61kHz�̏ꍇ�A
	   �V�X�e���N���b�N��33.514MHz ��
	   Tick��64�����̃^�C�}�J�E���^�ł��邽��
	   1�T���v�����O�ɕK�v�ȃ`�b�N��
	   
	   33.514MHz / 47.61kHz / 64 = 10.9988 �`�b�N �ƂȂ�
	 
	   FPGA�{�[�h�ł͂���ɔ����ɂ���K�v������
	*/

	switch ( I2S_GetSamplingRate() )
	{
		case I2S_SAMPLING_RATE_32730:
			switch ( sMicParam.frequency )
			{
				case MIC_SMP_ALL:         // 32.73 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 1 / 32730 / 64;
					break;
    			case MIC_SMP_1_2:         // 16.36 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 2 / 32730 / 64;
					break;
    			case MIC_SMP_1_3:         // 10.91 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 3 / 32730 / 64;
					break;
    			case MIC_SMP_1_4:         // 8.18 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 4 / 32730 / 64;
					break;
			}
			break;
		case I2S_SAMPLING_RATE_47610:
			switch ( sMicParam.frequency )
			{
				case MIC_SMP_ALL:         // 47.61 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 1 / 47610 / 64;
					break;
    			case MIC_SMP_1_2:         // 23.81 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 2 / 47610 / 64;
					break;
    			case MIC_SMP_1_3:         // 15.87 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 3 / 47610 / 64;
					break;
    			case MIC_SMP_1_4:         // 11.90 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 4 / 47610 / 64;
					break;
			}
			break;
	}

	// ���݂̃T���v�����O�����v�Z
	sampling_count = (u32)(past_tick / one_sampling_tick);

	// �T���v�����O�����A�h���X���v�Z
	adress = (void*)((u16 *)sMicParam.buffer + sampling_count); 

	return adress;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGain

  Description:  TWL���[�h�̃}�C�N�Q�C��(PGAB)��ݒ肵�܂��B
�@�@�@�@�@�@�@�@PGAB��AutoGainControl�������ɂȂ��Ă���Ƃ��̂ݗL���ł��B

  Arguments:    gain : �ݒ�Q�C���i0�`119 = 0�`59.5dB�j

  Returns:      None
 *---------------------------------------------------------------------------*/
void
TWL_MIC_SetAmpGain( u8 gain )
{
	SDK_ASSERT( gain >= 0 && gain <= 119 );
	CDC_SetPGAB( gain );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetAmpGain

  Description:  TWL���[�h�̃}�C�N�Q�C��(PGAB)���擾���܂��B

  Arguments:    None

  Returns:      �ݒ�Q�C��
 *---------------------------------------------------------------------------*/
u8
TWL_MIC_GetAmpGain( void )
{
	return CDC_GetPGAB();
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_ExDmaRecvAsync

  Description:  receive data with DMA
                async version

  Arguments:    dmaNo   : DMA channel No.
                buffer  : destination address
                size    : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static void
TWL_MICi_ExDmaRecvAsync( u32 dmaNo, void *buffer, u32 size )
{
    MIi_ExDmaRecvAsyncCore( 
			dmaNo, 						// DMA�ԍ�
			(void*)REG_MIC_FIFO_ADDR, 	// �]�����A�h���X
			buffer, 					// �]����A�h���X
            (u32)size, 					// ���]���o�C�g��
			(u32)32, 					// �]���o�C�g��
            MI_EXDMA_BLOCK_32B, 		// �u���b�N�]���T�C�Y
			0,							// �C���^�[�o�� 
			MI_EXDMA_PRESCALER_1,		// �v���X�P�[��1
            MI_EXDMA_CONTINUOUS_OFF, 	// ��R���e�B�j���A�X
			MI_EXDMA_SRC_RLD_OFF, 		// Source Reload Don't Care
			MI_EXDMA_DEST_RLD_OFF,		// Destination Reload Off
            MI_EXDMA_TIMING_MIC );		// �}�C�NFIFO�n�[�t�N��
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_ExDmaInterruptHandler

  Description:  interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void 
TWL_MICi_ExDmaInterruptHandler( void )
{
	// ���[�v����H
	if ( sMicParam.loop )
	{
 		// DMA���Đݒ�
        TWL_MICi_ExDmaRecvAsync( sMicParam.dmaNo, sMicParam.buffer, sMicParam.size );

		// �X�^�[�gTick�Đݒ�
		sMicTickStart = OS_GetTick();
	}

	// DMA�����R�[���o�b�N�̌Ăяo��
	if ( sMicParam.callback )
	{
		sMicParam.callback( sMicParam.loop );
	}
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_FifoInterruptHandler

  Description:  �}�C�NFIFO�j�]���ɌĂяo����銄�荞�݃n���h��
				���݂̎����ł�FIFO�N���A��ɃT���v�����O���ăX�^�[�g
				�����Ă��܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void 
TWL_MICi_FifoInterruptHandler( void )
{
	// �}�C�N��~
    reg_SND_MICCNT &= ~REG_SND_MICCNT_E_MASK;

    // �}�C�NFIFO�N���A
    reg_SND_MICCNT = REG_SND_MICCNT_FIFO_CLR_MASK;

	// ���m�����T���v�����O�ăX�^�[�g
    reg_SND_MICCNT = (u16)(	REG_SND_MICCNT_E_MASK 	|
							REG_SND_MICCNT_NR_MASK 	| 
							MIC_INTR_OVERFLOW		| 	// FIFO�j�]�Ŋ��荞�ݔ���
							sMicParam.frequency );
}

