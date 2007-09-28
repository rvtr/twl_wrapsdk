/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - twl-mic
  File:     twl_mic_api.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_MIC_MIC_API_H_
#define TWL_MIC_MIC_API_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// �������ʒ�`
typedef enum _TWLMICResult
{
    TWL_MIC_RESULT_SUCCESS = 0,
    TWL_MIC_RESULT_ILLEGAL_PARAMETER,
    TWL_MIC_RESULT_INVALID_COMMAND,
    TWL_MIC_RESULT_ILLEGAL_STATUS,
    TWL_MIC_RESULT_BUSY,
    TWL_MIC_RESULT_SEND_ERROR,
    TWL_MIC_RESULT_FATAL_ERROR,
    TWL_MIC_RESULT_MAX
}
TWLMICResult;

typedef enum
{
    TWL_MIC_FREQUENCY_ALL             =  0x0,		// 47.61 / 32.73 kHz
    TWL_MIC_FREQUENCY_1_2             =  0x1,		// 23.81 / 16.36 kHz
    TWL_MIC_FREQUENCY_1_3             =  0x2,		// 15.87 / 10.91 kHz
    TWL_MIC_FREQUENCY_1_4             =  0x3,		// 11.90 /  8.18 kHz
    TWL_MIC_FREQUENCY_NUM			  =  0x4
}
TwlMicFrequency;

// �R�[���o�b�N
typedef void (*TwlMicCallback)(TWLMICResult result, void* arg);

// �I�[�g�T���v�����O�p�ݒ��`
typedef struct TwlMicAutoParam
{
	u32    			dmaNo;				// DMA No
    void   			*buffer;            // ���ʊi�[�o�b�t�@�ւ̃|�C���^(DMA���g������4byte�A���C�����g�j
    u32     		size;               // �o�b�t�@�T�C�Y(DMA���g������4byte�P�ʁj
    TwlMicFrequency frequency;          // �T���v�����O�p�x
    BOOL    		loop_enable;        // �o�b�t�@�t�����̃��[�v��
    TwlMicCallback 	full_callback;     	// �o�b�t�@�t�����̃R�[���o�b�N
    void*			full_arg;   		// ��L�R�[���o�b�N�Ɏw�肷�����
}
TwlMicAutoParam;


/*---------------------------------------------------------------------------*
  Name:         MIC_Init

  Description:  MIC���C�u����������������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
TWL_MIC_Init(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_End

  Description:  MIC���C�u�������I������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
TWL_MIC_End(void);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSamplingAsync

  Description:  �}�C�N�P���T���v�����O�J�n�i�񓯊��Łj

  Arguments:    

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_DoSamplingAsync(u16* buf,  TwlMicCallback callback, void* callbackArg);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSampling

  Description:  �}�C�N�P���T���v�����O�J�n�i�����Łj

  Arguments:    

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_DoSampling(u16* buf);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StartAutoSamplingAsync

  Description:  �}�C�N�����T���v�����O�J�n�i�񓯊��Łj

  Arguments:    param

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_StartAutoSamplingAsync(TwlMicAutoParam* param, TwlMicCallback callback, void* callbackArg);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StartAutoSampling

  Description:  �}�C�N�����T���v�����O�J�n�i�����Łj

  Arguments:    param

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StartAutoSampling(TwlMicAutoParam* param);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSampling

  Description:  �}�C�N�����T���v�����O��~�i�����Łj

  Arguments:    param

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StopAutoSampling( void );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSamplingAsync

  Description:  �}�C�N�����T���v�����O��~�i�񓯊��Łj

  Arguments:    none

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StopAutoSamplingAsync(  TwlMicCallback callback, void* callbackArg );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddressAsync

  Description:  �ŐV�̃T���v�����O�f�[�^�̊i�[�A�h���X��Ԃ��܂��B
				�A���A�A�h���X�̓T���v�����O���Ԃ����ɎZ�o�������̂ł���
				���ߌ덷���܂�ł��܂��B�����ARM7����ARM9�ւƒʒm���Ă�
				��ԂɃT���v�����O�͐i��ł���\��������܂��B
				�i�񓯊��Łj

  Arguments:    adress      : �A�h���X�i�[�|�C���^�̃A�h���X
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetLastSamplingAddressAsync( void** adress, TwlMicCallback callback, void* callbackArg );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddress

  Description:  �ŐV�̃T���v�����O�f�[�^�̊i�[�A�h���X��Ԃ��܂��B
				�A���A�A�h���X�̓T���v�����O���Ԃ����ɎZ�o�������̂ł���
				���ߌ덷���܂�ł��܂��B�����ARM7����ARM9�ւƒʒm���Ă�
				��ԂɃT���v�����O�͐i��ł���\��������܂��B
				�i�����Łj

  Arguments:    adress      : �A�h���X�i�[�|�C���^�̃A�h���X

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetLastSamplingAddress( void** adress );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGainAsync

  Description:  �v���O���}�u���Q�C���A���v�̐ݒ���s���܂��B�i�񓯊��Łj
				���̊֐��Őݒ肵���Q�C���̓I�[�g�Q�C���R���g���[����
				�����ɂȂ��Ă���Ƃ��̂ݗL���ƂȂ邱�Ƃɒ��ӂ��Ă��������B

  Arguments:    gain 		: �ݒ�Q�C���i0�`119 = 0�`59.5dB�j
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_SetAmpGainAsync( u8 gain, TwlMicCallback callback, void* callbackArg );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGain

  Description:  �v���O���}�u���Q�C���A���v�̐ݒ���s���܂��B�i�����Łj
				���̊֐��Őݒ肵���Q�C���̓I�[�g�Q�C���R���g���[����
				�����ɂȂ��Ă���Ƃ��̂ݗL���ƂȂ邱�Ƃɒ��ӂ��Ă��������B

  Arguments:    gain 		: �ݒ�Q�C���i0�`119 = 0�`59.5dB�j

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_SetAmpGain( u8 gain );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetAmpGainAsync

  Description:  TWL���[�h�̃}�C�N�Q�C��(PGAB)���擾���܂��B
				�i�񓯊��Łj

  Arguments:    gain        : �}�C�N�Q�C���l�̊i�[�A�h���X
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetAmpGainAsync( u8* gain, TwlMicCallback callback, void* callbackArg );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetAmpGain

  Description:  TWL���[�h�̃}�C�N�Q�C��(PGAB)���擾���܂��B
				�i�����Łj

  Arguments:    gain        : �}�C�N�Q�C���l�̊i�[�A�h���X

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetAmpGain( u8* gain );

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_MIC_MIC_API_H_ */
