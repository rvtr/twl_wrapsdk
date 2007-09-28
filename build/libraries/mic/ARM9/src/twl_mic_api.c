/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/mic.h>

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
// �l�߂ăR�s�[����
#define MIC_PACK_U16(d, s)                   \
    ((d)[0] = (u8)((*((u16*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u16*)s) >>  8) & 0xFF))

#define MIC_PACK_U32(d, s)                   \
    ((d)[0] = (u8)((*((u32*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u32*)s) >>  8) & 0xFF),  \
     (d)[2] = (u8)((*((u32*)s) >> 16) & 0xFF),  \
     (d)[3] = (u8)((*((u32*)s) >> 24) & 0xFF))

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/
typedef struct _TWLMICWork
{
    BOOL            lock;
    TwlMicCallback  callback;					// �񓯊��֐��p�R�[���o�b�N
    void*			callbackArg;				// ��L�֐��p����
    TwlMicCallback  full_callback;				// �񓯊��֐��p�R�[���o�b�N
    void*			full_arg;					// ��L�֐��p����
    TWLMICResult    result;             		// �擪�f�[�^�����ʘg
    TWLMICPxiCommand   command;        			// �R�}���h���
    TWLMICPxiResult    pxiResult;      			// �擪�f�[�^�����ʘg
	u16*               pOneBuffer;				// �P���T���v�����O�o�b�t�@�ۑ��p
	void**             pLastSamplingAddress;	// �ŐV�T���v�����O�f�[�^�i�[�A�h���X
	u8*                pAmpGain;				// �A���v�Q�C���i�[�A�h���X
    u8      current;                    		// ��M�ς݃f�[�^�� (�o�C�g�P��) (�擪������!!)
    u8      total;                      		// �ŏI�f�[�^�� (1 + �㑱�R�}���h*3)
    u8      data[TWL_MIC_PXI_DATA_SIZE_MAX];    // ARM7����̃f�[�^�ۑ��p
}
TWLMICWork;

/*---------------------------------------------------------------------------*
    �ÓI�ϐ���`
 *---------------------------------------------------------------------------*/
static BOOL micInitialized;
static TWLMICWork micWork;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static BOOL TWLMICi_SendPxiCommand(TWLMICPxiCommand command, u8 size, u8 data);
static void TWLMICi_SendPxiData(u8 *pData);
static void TWLMICi_PxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void TWLMICi_CallbackAndUnlock(TWLMICResult result);
static void TWLMICi_GetResultCallback(TWLMICResult result, void *arg);
static void TWLMICi_WaitBusy(void);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_Init

  Description:  MIC���C�u����������������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TWL_MIC_Init(void)
{
    // �������ς݂��m�F
    if (micInitialized)
    {
        return;
    }
    micInitialized = 1;

    // �ϐ�������
    micWork.lock = FALSE;
    micWork.callback = NULL;

    // PXI�֘A��������
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_TWL_MIC, PXI_PROC_ARM7))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_TWL_MIC, TWLMICi_PxiCallback);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_End

  Description:  MIC���C�u�������I������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TWL_MIC_End(void)
{
    // �������ς݂��m�F
    if (micInitialized == 0)
    {
        return;
    }
    micInitialized = 0;

    // PXI�֘A��~
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_TWL_MIC, NULL);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSamplingAsync

  Description:  �}�C�N�P���T���v�����O�J�n�i�񓯊��Łj

  Arguments:    

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_DoSamplingAsync(u16* buf,  TwlMicCallback callback, void* callbackArg)
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_ONE_SAMPLING;
    const u8                size = TWL_MIC_PXI_SIZE_ONE_SAMPLING;	// �o�C�g
    OSIntrMode enabled;

	// buffer NULL�`�F�b�N
	// ToDo: �K�؂ȃA�h���X���ǂ����`�F�b�N
	if (buf == NULL)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// ���b�N
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // �񓯊��֐��p�R�[���o�b�N�ݒ�
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

	// �o�b�t�@�A�h���X�ۑ�
	micWork.pOneBuffer = buf;

    // �R�}���h���M
    if (TWLMICi_SendPxiCommand(command, size, 0) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSampling

  Description:  �}�C�N�P���T���v�����O�J�n�i�����Łj

  Arguments:    

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_DoSampling(u16* buf)
{
    micWork.result = TWL_MIC_DoSamplingAsync(buf, TWLMICi_GetResultCallback, 0);
    if (micWork.result == TWL_MIC_RESULT_SUCCESS)
    {
        TWLMICi_WaitBusy();
    }
    return micWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StartAutoSamplingAsync

  Description:  �}�C�N�����T���v�����O�J�n�i�񓯊��Łj

  Arguments:    mic      - one of MicSelect

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StartAutoSamplingAsync(TwlMicAutoParam* param, TwlMicCallback callback, void* callbackArg)
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_AUTO_START;
    const u8                size = TWL_MIC_PXI_SIZE_AUTO_START;	// �o�C�g
    OSIntrMode enabled;
    u8  data[size+2];
    int i;

	// DMA-No�`�F�b�N
    if ( param->dmaNo < MI_EXDMA_CH_MIN || MI_EXDMA_CH_MAX < param->dmaNo )
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// buffer NULL�`�F�b�N & 4�o�C�g�A���C�����g�`�F�b�N
	// ToDo: �K�؂ȃA�h���X���ǂ����`�F�b�N
	if (param->buffer == NULL || (u32)param->buffer & 0x03)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// size 4�o�C�g�̔{���`�F�b�N
	// ToDo: buffer + size ���K�؂ȃ������͈͂�
	if (param->size & 0x03)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// frequency�`�F�b�N
	if ( param->frequency > TWL_MIC_FREQUENCY_1_4 )
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// ���b�N
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // �񓯊��֐��p�R�[���o�b�N�ݒ�
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

	// FULL�R�[���o�b�N�ݒ�
	if (param->full_callback)
	{
		micWork.full_callback = param->full_callback;
		micWork.full_arg = param->full_arg;
	}

    // �f�[�^�쐬
    data[0] = (u8)param->dmaNo;
    MIC_PACK_U32(&data[1], &param->buffer);
    MIC_PACK_U32(&data[5], &param->size);
    data[9]  = (u8)param->frequency;
    data[10] = (u8)param->loop_enable;

    // �R�}���h���M
    if (TWLMICi_SendPxiCommand(command, size, data[0]) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        TWLMICi_SendPxiData(&data[i]);
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StartAutoSampling

  Description:  �}�C�N�����T���v�����O�J�n�i�����Łj

  Arguments:    param

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StartAutoSampling(TwlMicAutoParam* param)
{
    micWork.result = TWL_MIC_StartAutoSamplingAsync(param, TWLMICi_GetResultCallback, 0);
    if (micWork.result == TWL_MIC_RESULT_SUCCESS)
    {
        TWLMICi_WaitBusy();
    }
    return micWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSamplingAsync

  Description:  �}�C�N�����T���v�����O��~�i�񓯊��Łj

  Arguments:    none

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_StopAutoSamplingAsync( TwlMicCallback callback, void* callbackArg )
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_AUTO_STOP;
    const u8                size = TWL_MIC_PXI_SIZE_AUTO_STOP;	// �o�C�g
    OSIntrMode enabled;

	// ���b�N
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // �R�[���o�b�N�ݒ�
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

    // �R�}���h���M
    if (TWLMICi_SendPxiCommand(command, size, 0) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSampling

  Description:  �}�C�N�����T���v�����O��~�i�����Łj

  Arguments:    param

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StopAutoSampling( void )
{
    micWork.result = TWL_MIC_StopAutoSamplingAsync(TWLMICi_GetResultCallback, 0);
    if (micWork.result == TWL_MIC_RESULT_SUCCESS)
    {
        TWLMICi_WaitBusy();
    }
    return micWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddressAsync

  Description:  �ŐV�̃T���v�����O�f�[�^�̊i�[�A�h���X��Ԃ��܂��B
				�A���A�A�h���X�̓T���v�����O���Ԃ����ɗ��_�I�Ɍv�Z���ꂽ
				���̂ł��邽�ߌ덷���܂�ł��܂��B
				�i�񓯊��Łj

  Arguments:    adress      : �A�h���X�i�[�|�C���^�̃A�h���X
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetLastSamplingAddressAsync( void** adress, TwlMicCallback callback, void* callbackArg )
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS;
    const u8                size = TWL_MIC_PXI_SIZE_GET_LAST_SAMPLING_ADDRESS;	// �o�C�g
    OSIntrMode enabled;

	// ���b�N
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // �R�[���o�b�N�ݒ�
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

	// �A�h���X�i�[�A�h���X�ۑ�
	micWork.pLastSamplingAddress = adress;

    // �R�}���h���M
    if (TWLMICi_SendPxiCommand(command, size, 0) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddress

  Description:  �ŐV�̃T���v�����O�f�[�^�̊i�[�A�h���X��Ԃ��܂��B
				�A���A�A�h���X�̓T���v�����O���Ԃ����ɗ��_�I�Ɍv�Z���ꂽ
				���̂ł��邽�ߌ덷���܂�ł��܂��B
				�i�����Łj

  Arguments:    adress      : �A�h���X�i�[�|�C���^�̃A�h���X

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetLastSamplingAddress( void** adress )
{
    micWork.result = TWL_MIC_GetLastSamplingAddressAsync( adress, TWLMICi_GetResultCallback, 0);
    if (micWork.result == TWL_MIC_RESULT_SUCCESS)
    {
        TWLMICi_WaitBusy();
    }
    return micWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGainAsync

  Description:  �v���O���}�u���Q�C���A���v�̐ݒ���s���܂��B�i�񓯊��Łj
				���̊֐��Őݒ肵���Q�C���̓I�[�g�Q�C���R���g���[����
�@�@�@�@�@�@�@�@�����ɂȂ��Ă���Ƃ��̂ݗL���ƂȂ邱�Ƃɒ��ӂ��Ă��������B

  Arguments:    gain 		: �ݒ�Q�C���i0�`119 = 0�`59.5dB�j
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_SetAmpGainAsync( u8 gain, TwlMicCallback callback, void* callbackArg )
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_SET_AMP_GAIN;
    const u8                size = TWL_MIC_PXI_SIZE_SET_AMP_GAIN;	// �o�C�g
    u8  data[size+2];
    OSIntrMode enabled;

	// �ݒ�Q�C���͈̓`�F�b�N
	if ( gain > 119)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// ���b�N
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // �R�[���o�b�N�ݒ�
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

    // �f�[�^�쐬
    data[0] = gain;

    // �R�}���h���M
    if (TWLMICi_SendPxiCommand(command, size, data[0]) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGain

  Description:  �v���O���}�u���Q�C���A���v�̐ݒ���s���܂��B�i�����Łj
				���̊֐��Őݒ肵���Q�C���̓I�[�g�Q�C���R���g���[����
				�����ɂȂ��Ă���Ƃ��̂ݗL���ƂȂ邱�Ƃɒ��ӂ��Ă��������B

  Arguments:    gain 		: �ݒ�Q�C���i0�`119 = 0�`59.5dB�j

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_SetAmpGain( u8 gain )
{
    micWork.result = TWL_MIC_SetAmpGainAsync( gain, TWLMICi_GetResultCallback, 0);
    if (micWork.result == TWL_MIC_RESULT_SUCCESS)
    {
        TWLMICi_WaitBusy();
    }
    return micWork.result;
}

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
TWL_MIC_GetAmpGainAsync( u8* gain, TwlMicCallback callback, void* callbackArg )
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_GET_AMP_GAIN;
    const u8                size = TWL_MIC_PXI_SIZE_GET_AMP_GAIN;	// �o�C�g
    OSIntrMode enabled;

	// NULL�`�F�b�N
	if ( gain == NULL)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// ���b�N
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // �R�[���o�b�N�ݒ�
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

	// �Q�C���i�[�A�h���X�ۑ�
	micWork.pAmpGain = gain;

    // �R�}���h���M
    if (TWLMICi_SendPxiCommand(command, size, 0) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetAmpGain

  Description:  TWL���[�h�̃}�C�N�Q�C��(PGAB)���擾���܂��B
				�i�����Łj

  Arguments:    gain        : �}�C�N�Q�C���l�̊i�[�A�h���X

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetAmpGain( u8* gain )
{
    micWork.result = TWL_MIC_GetAmpGainAsync( gain, TWLMICi_GetResultCallback, 0);
    if (micWork.result == TWL_MIC_RESULT_SUCCESS)
    {
        TWLMICi_WaitBusy();
    }
    return micWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         TWLMICi_SendPxiCommand

  Description:  �w��擪�R�}���h��PXI�o�R��ARM7�ɑ��M����B

  Arguments:    command     - �ΏۃR�}���h
                size        - ���M�f�[�^�T�C�Y (�o�C�g�P��)
                data        - �擪�f�[�^ (1�o�C�g�̂�)

  Returns:      BOOL     - PXI�ɑ΂��đ��M�����������ꍇTRUE���A
                           PXI�ɂ�鑗�M�Ɏ��s�����ꍇFALSE��Ԃ��B
 *---------------------------------------------------------------------------*/
static BOOL TWLMICi_SendPxiCommand(TWLMICPxiCommand command, u8 size, u8 data)
{
    u32 pxiData = (u32)(TWL_MIC_PXI_START_BIT |
            ((command << TWL_MIC_PXI_COMMAND_SHIFT) & TWL_MIC_PXI_COMMAND_MASK) |
            ((size << TWL_MIC_PXI_DATA_NUMS_SHIFT) & TWL_MIC_PXI_DATA_NUMS_MASK) |
            ((data << TWL_MIC_PXI_1ST_DATA_SHIFT) & TWL_MIC_PXI_1ST_DATA_MASK));
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TWL_MIC, pxiData, 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         TWLMICi_SendPxiData

  Description:  �w��㑱�f�[�^��PXI�o�R��ARM7�ɑ��M����B

  Arguments:    pData   - 3�o�C�g�f�[�^�̐擪�ւ̃|�C���^

  Returns:      None
 *---------------------------------------------------------------------------*/
static void TWLMICi_SendPxiData(u8 *pData)
{
    u32 pxiData = (u32)((pData[0] << 16) | (pData[1] << 8) | pData[2]);
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TWL_MIC, pxiData, 0))
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWLMICi_PxiCallback

  Description:  �񓯊��֐��p�̋��ʃR�[���o�b�N�֐��B

  Arguments:    tag -  PXI tag which show message type.
                data - message from ARM7.
                err -  PXI transfer error flag.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWLMICi_PxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    TWLMICResult   result;

    // PXI�ʐM�G���[���m�F
    if (err)
    {
        // �V�[�P���X�������I��
        TWLMICi_CallbackAndUnlock(TWL_MIC_RESULT_FATAL_ERROR);
        return;
    }
    // �擪�f�[�^
    if (data & TWL_MIC_PXI_START_BIT)
    {
        // ��M�f�[�^�����
        SDK_ASSERT((data & TWL_MIC_PXI_RESULT_BIT) == TWL_MIC_PXI_RESULT_BIT);
        micWork.total = (u8)((data & TWL_MIC_PXI_DATA_NUMS_MASK) >> TWL_MIC_PXI_DATA_NUMS_SHIFT);
        micWork.current = 0;
        micWork.command = (TWLMICPxiCommand)((data & TWL_MIC_PXI_COMMAND_MASK) >> TWL_MIC_PXI_COMMAND_SHIFT);
        micWork.pxiResult = (TWLMICPxiResult)((data & TWL_MIC_PXI_1ST_DATA_MASK) >> TWL_MIC_PXI_1ST_DATA_SHIFT);
    }
    // �㑱�f�[�^
    else
    {
        if (micWork.current < TWL_MIC_PXI_DATA_SIZE_MAX)
        {
            micWork.data[micWork.current++] = (u8)((data & 0xFF0000) >> 16);
        }
        if (micWork.current < TWL_MIC_PXI_DATA_SIZE_MAX)
        {
            micWork.data[micWork.current++] = (u8)((data & 0x00FF00) >> 8);
        }
        if (micWork.current < TWL_MIC_PXI_DATA_SIZE_MAX)
        {
            micWork.data[micWork.current++] = (u8)((data & 0x0000FF) >> 0);
        }
    }

    if (micWork.current >= micWork.total-1)   // > �͖����͂�
    {
        // �������ʂ��m�F
        switch (micWork.pxiResult)
        {
	        case TWL_MIC_PXI_RESULT_SUCCESS:
	            result = TWL_MIC_RESULT_SUCCESS;
	            break;
	        case TWL_MIC_PXI_RESULT_INVALID_COMMAND:
	            result = TWL_MIC_RESULT_INVALID_COMMAND;
	            break;
	        case TWL_MIC_PXI_RESULT_INVALID_PARAMETER:
	            result = TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	            break;
	        case TWL_MIC_PXI_RESULT_ILLEGAL_STATUS:
	            result = TWL_MIC_RESULT_ILLEGAL_STATUS;
	            break;
	        case TWL_MIC_PXI_RESULT_BUSY:
	            result = TWL_MIC_RESULT_BUSY;
	            break;
	        default:
	            result = TWL_MIC_RESULT_FATAL_ERROR;
        }

		switch (micWork.command)
		{
			case TWL_MIC_PXI_COMMAND_MIC_BUFFER_FULL:
				// FULL�R�[���o�b�N�̌Ăяo��
				if (micWork.full_callback)
				{
					micWork.full_callback( result, micWork.full_arg );
				}
				break;

			case TWL_MIC_PXI_COMMAND_ONE_SAMPLING:
				// �P���T���v�����O���ʊi�[
				*micWork.pOneBuffer = (u16)(micWork.data[0] | (micWork.data[1] << 8));
		        // �񓯊��֐��p�R�[���o�b�N�̌Ăяo�������b�N����
		        TWLMICi_CallbackAndUnlock(result);
				break;

			case TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS:
				// �ŐV�T���v�����O�f�[�^�A�h���X�i�[
				*micWork.pLastSamplingAddress = (void *)(micWork.data[0] | (micWork.data[1] << 8) | (micWork.data[2] << 16) | (micWork.data[3] << 24));
		        // �񓯊��֐��p�R�[���o�b�N�̌Ăяo�������b�N����
		        TWLMICi_CallbackAndUnlock(result);
				break;

			case TWL_MIC_PXI_COMMAND_GET_AMP_GAIN:
				// �A���v�Q�C���i�[
				*micWork.pAmpGain = micWork.data[0];
		        // �񓯊��֐��p�R�[���o�b�N�̌Ăяo�������b�N����
		        TWLMICi_CallbackAndUnlock(result);
				break;

			default:
		        // �񓯊��֐��p�R�[���o�b�N�̌Ăяo�������b�N����
		        TWLMICi_CallbackAndUnlock(result);
		}
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWLMICi_CallbackAndUnlock

  Description:  �R�[���o�b�N�̌Ăяo���ƃ��b�N�̉������s��

  Arguments:    result  - ARM7���瑗��ꂽ����

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWLMICi_CallbackAndUnlock(TWLMICResult result)
{
    TwlMicCallback cb;

	// ���b�N����
    if (micWork.lock)
    {
        micWork.lock = FALSE;
    }

	// �񓯊��֐��p�R�[���o�b�N�Ăяo��
    if (micWork.callback)
    {
        cb = micWork.callback;
        micWork.callback = NULL;
        cb(result, micWork.callbackArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWLMICi_GetResultCallback

  Description:  �����֐��������Ŏ��s����񓯊��֐��̌��ʂ��擾���邽�߂�
				�g�p����B

  Arguments:    result - �񓯊��֐��̏������ʁB
                arg    - �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWLMICi_GetResultCallback(TWLMICResult result, void *arg)
{
#pragma unused( arg )

    micWork.result = result;
}

/*---------------------------------------------------------------------------*
  Name:         TWLMICi_WaitBusy

  Description:  MIC�̔񓯊����������b�N����Ă���ԑ҂B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if 0
#include    <nitro/code32.h>
static asm void TWLMICi_WaitBusy(void)
{
    ldr     r12,    =micWork.lock
loop:
    ldr     r0,     [ r12,  #0 ]
    cmp     r0,     #TRUE
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>
#else
extern void PXIi_HandlerRecvFifoNotEmpty(void);
static void TWLMICi_WaitBusy(void)
{
    volatile BOOL *p = &micWork.lock;

    while (*p)
    {
        if (OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE)
        {
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}
#endif
