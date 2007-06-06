/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - aes
  File:     aes_control.c

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

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define AES_PXI_SIZE_CHECK(nums)                                            \
    if (aesWork.total != (nums)) {                                          \
        aesWork.locked = AES_UNLOCKED;                                      \
        AesReturnResult(aesWork.command, AES_PXI_RESULT_INVALID_PARAMETER); \
        break;                                                              \
    }

// �A���C�����g�������ăR�s�[����
#define AES_UNPACK_U32(d, s)    \
    (*(d) = (u32)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8) | (((u8*)s)[2] << 16) | (((u8*)s)[3] << 24)))

#define AES_UNPACK_U96(d, s) \
    (AES_UNPACK_U32((d)->e + 0, ((u8*)s) + 0), \
     AES_UNPACK_U32((d)->e + 1, ((u8*)s) + 4), \
     AES_UNPACK_U32((d)->e + 2, ((u8*)s) + 8))

#define AES_UNPACK_U128(d, s) \
    (AES_UNPACK_U32((d)->e + 0, ((u8*)s) + 0), \
     AES_UNPACK_U32((d)->e + 1, ((u8*)s) + 4), \
     AES_UNPACK_U32((d)->e + 2, ((u8*)s) + 8), \
     AES_UNPACK_U32((d)->e + 3, ((u8*)s) + 12))

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �ÓI�ϐ���`
 *---------------------------------------------------------------------------*/
static BOOL aesInitialized;             // �������m�F�t���O
static AESWork aesWork;                 // ���[�N�ϐ����܂Ƃ߂��\����

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void AesPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void AesReturnResult(u8 command, AESPxiResult result);
static void AesThread(void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_Init

  Description:  AES���C�u����������������B

  Arguments:    priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Init(u32 priority)
{
    // �������ς݂��m�F
    if (aesInitialized)
    {
        return;
    }
    aesInitialized = 1;

    // �ϐ�������
    aesWork.locked = AES_UNLOCKED;

    // PXI�֘A��������
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_AES, AesPxiCallback);

    // ���������s���X���b�h���쐬
    OS_InitMessageQueue(&aesWork.msgQ, aesWork.msgArray, AES_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&aesWork.thread,
                    AesThread,
                    0,
                    (void *)(aesWork.stack + (AES_THREAD_STACK_SIZE / sizeof(u64))),
                    AES_THREAD_STACK_SIZE, priority);
    OS_WakeupThreadDirect(&aesWork.thread);
}

/*---------------------------------------------------------------------------*
  Name:         AES_Lock

  Description:  AES���C�u������ARM9����g���Ȃ��悤�ɂ���B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Lock(void)
{
    OSIntrMode enabled;
    while (TRUE)
    {
        enabled = OS_DisableInterrupts();
        if (aesWork.locked == AES_UNLOCKED)
        {
            aesWork.locked = AES_LOCKED_BY_ARM7;
            (void)OS_RestoreInterrupts(enabled);
            return;
        }
        (void)OS_RestoreInterrupts(enabled);
    }
}

/*---------------------------------------------------------------------------*
  Name:         AES_TryLock

  Description:  AES���C�u������ARM9����g���Ȃ��悤�Ɏ��݂�B

  Arguments:    None.

  Returns:      BOOL - ���b�N�ł����ꍇ��TRUE��Ԃ�
 *---------------------------------------------------------------------------*/
BOOL AES_TryLock(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    if (aesWork.locked == AES_UNLOCKED)
    {
        aesWork.locked = AES_LOCKED_BY_ARM7;
        (void)OS_RestoreInterrupts(enabled);
        return TRUE;
    }
    (void)OS_RestoreInterrupts(enabled);
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         AES_Unlock

  Description:  AES���C�u������ARM7���̃��b�N����������

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Unlock(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    if (aesWork.locked == AES_LOCKED_BY_ARM7) {
        aesWork.locked = AES_UNLOCKED;
    }
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         AesPxiCallback

  Description:  PXI�o�R�Ŏ�M�����f�[�^����͂���B

  Arguments:    tag -  PXI��ʂ������^�O�B
                data - ��M�����f�[�^�B����26bit���L���B
                err -  PXI�ʐM�ɂ�����G���[�t���O�B
                       ARM9���ɂē���ʂ�PXI������������Ă��Ȃ����Ƃ������B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AesPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    // PXI�ʐM�G���[���`�F�b�N
    if (err)
    {
        return;
    }
    // �擪�f�[�^
    if (data & AES_PXI_START_BIT)
    {
        aesWork.total = (u8)((data & AES_PXI_DATA_NUMS_MASK) >> AES_PXI_DATA_NUMS_SHIFT);
        aesWork.current = 0;
        aesWork.command = (u8)((data & AES_PXI_COMMAND_MASK) >> AES_PXI_COMMAND_SHIFT);
        aesWork.data[aesWork.current++] = (u8)((data & AES_PXI_1ST_DATA_MASK) >> AES_PXI_1ST_DATA_SHIFT);
//OS_TPrintf("START_BIT (total=%d, command=%X).\n", aesWork.total, aesWork.command);
    }
    // �㑱�f�[�^
    else
    {
        aesWork.data[aesWork.current++] = (u8)((data & 0xFF0000) >> 16);
        aesWork.data[aesWork.current++] = (u8)((data & 0x00FF00) >> 8);
        aesWork.data[aesWork.current++] = (u8)((data & 0x0000FF) >> 0);
    }

    // �p�P�b�g����
    if (aesWork.current >= aesWork.total)   // �ő��2�]���Ɏ擾����
    {
        // ��M�����R�}���h�����
        OSIntrMode enabled;

        switch (aesWork.command)
        {
            // ���m�̃R�}���h�Q
        case AES_PXI_COMMAND_RESET:
        case AES_PXI_COMMAND_IS_BUSY:
        case AES_PXI_COMMAND_WAIT:
        case AES_PXI_COMMAND_INPUT_FIFO_IS_FULL:
        case AES_PXI_COMMAND_OUTPUT_FIFO_IS_EMPTY:
        case AES_PXI_COMMAND_WAIT_INPUT_FIFO_NOT_FULL:
        case AES_PXI_COMMAND_WAIT_OUTPUT_FIFO_NOT_EMPTY:
        case AES_PXI_COMMAND_IS_VALID:
#if 0
        case AES_PXI_COMMAND_SELECT_KEY:
        case AES_PXI_COMMAND_SET_KEY:
        case AES_PXI_COMMAND_SET_ID:
        case AES_PXI_COMMAND_SET_SEED:
        case AES_PXI_COMMAND_SET_KEY2:
#else
        case AES_PXI_COMMAND_SET_GENERAL_KEY:
        case AES_PXI_COMMAND_SET_SYSTEM_KEY:
        case AES_PXI_COMMAND_SET_GAME_KEY:
        case AES_PXI_COMMAND_SET_SPECIAL_KEY:
        case AES_PXI_COMMAND_SET_ALTERNATIVE_KEY:
#endif
        case AES_PXI_COMMAND_START_CCM_DEC:
        case AES_PXI_COMMAND_START_CCM_DEC_NOMAC:
        case AES_PXI_COMMAND_START_CCM_ENC:
        case AES_PXI_COMMAND_START_CTR:
        case AES_PXI_COMMAND_START_DMA_SEND:
        case AES_PXI_COMMAND_START_DMA_RECV:
        case AES_PXI_COMMAND_WAIT_DMA:
        case AES_PXI_COMMAND_CPU_SEND:
        case AES_PXI_COMMAND_CPU_RECV:
            // �X���b�h���ĊJ
            if (!OS_SendMessage(&aesWork.msgQ, NULL, OS_MESSAGE_NOBLOCK))
            {
                AesReturnResult(aesWork.command, AES_PXI_RESULT_FATAL_ERROR);
            }
            break;

            // ���b�N�F����R�}���h (�����ŏ�������)
        case AES_PXI_COMMAND_TRY_LOCK:
            enabled = OS_DisableInterrupts();
            if (aesWork.locked == AES_UNLOCKED)
            {
                // �r�����b�N�{��
                aesWork.locked = AES_LOCKED_BY_ARM9;
                (void)OS_RestoreInterrupts(enabled);
                AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS_TRUE);
            }
            else
            {
                (void)OS_RestoreInterrupts(enabled);
                AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS_FALSE);
            }
            break;

            // �A�����b�N�F����R�}���h (�����ŏ�������)
        case AES_PXI_COMMAND_UNLOCK:
            enabled = OS_DisableInterrupts();
            if (aesWork.locked == AES_LOCKED_BY_ARM9)
            {
                // �r�����b�N�J��
                aesWork.locked = AES_UNLOCKED;
            }
            (void)OS_RestoreInterrupts(enabled);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);
            break;

            // ���m�̃R�}���h
        default:
            AesReturnResult(aesWork.command, AES_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         AesReturnResult

  Description:  PXI�o�R�ŏ������ʂ�ARM9�ɑ��M����B

  Arguments:    command - ���������R�}���h�B
                result -  �������ʁB

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AesReturnResult(u8 command, AESPxiResult result)
{
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_AES,
                                  (u32)(AES_PXI_START_BIT | AES_PXI_RESULT_BIT |
                                        ((command << AES_PXI_COMMAND_SHIFT) & AES_PXI_COMMAND_MASK)
                                        | ((result << AES_PXI_1ST_DATA_SHIFT) & AES_PXI_1ST_DATA_MASK)),
                                  0))
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         AesThread

  Description:  AES����̎��������s���X���b�h�B

  Arguments:    arg - �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AesThread(void *arg)
{
#pragma unused( arg )

    OSMessage msg;
    BOOL result;
    u128 data128;
#if 0
    u128 data128b;
#endif
    u96 data96;
    u32 data32a;
    u32 data32b;

    while (TRUE)
    {
        // ���b�Z�[�W�����s�����܂ŐQ��
        (void)OS_ReceiveMessage(&(aesWork.msgQ), &msg, OS_MESSAGE_BLOCK);

        // �R�}���h�ɏ]���Ċe�폈�������s
        switch (aesWork.command)
        {
            // AES�G���W���̃��Z�b�g
        case AES_PXI_COMMAND_RESET:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_RESET);
            AES_Reset();
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

            // AES�G���W���̃r�W�[�`�F�b�N
        case AES_PXI_COMMAND_IS_BUSY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_IS_BUSY);
            result = AES_IsBusy();
            AesReturnResult(aesWork.command, result ?
                AES_PXI_RESULT_SUCCESS_TRUE : AES_PXI_RESULT_SUCCESS_FALSE);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_WAIT:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_WAIT);
            AES_Wait();
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_INPUT_FIFO_IS_FULL:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_INPUT_FIFO_IS_FULL);
            result = AES_InputFifoIsFull();
            AesReturnResult(aesWork.command, result ?
                AES_PXI_RESULT_SUCCESS_TRUE : AES_PXI_RESULT_SUCCESS_FALSE);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_OUTPUT_FIFO_IS_EMPTY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_OUTPUT_FIFO_IS_EMPTY);
            result = AES_OutputFifoIsEmpty();
            AesReturnResult(aesWork.command, result ?
                AES_PXI_RESULT_SUCCESS_TRUE : AES_PXI_RESULT_SUCCESS_FALSE);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_WAIT_INPUT_FIFO_NOT_FULL:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_WAIT_INPUT_FIFO_NOT_FULL);
            AES_WaitInputFifoNotFull();
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_WAIT_OUTPUT_FIFO_NOT_EMPTY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_WAIT_OUTPUT_FIFO_NOT_EMPTY);
            AES_WaitOutputFifoNotEmpty();
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_IS_VALID:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_IS_VALID);
            result = AES_IsValid();
            AesReturnResult(aesWork.command, result ?
                AES_PXI_RESULT_SUCCESS_TRUE : AES_PXI_RESULT_SUCCESS_FALSE);     // ARM9�ɏ����̐�����ʒB
            break;
#if 0
        case AES_PXI_COMMAND_SELECT_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SELECT_KEY);
            AES_SelectKey(aesWork.data[0]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_SET_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[1]);
            AES_SetKey(aesWork.data[0], &data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_SET_ID:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_ID);
            AES_UNPACK_U128(&data128, &aesWork.data[1]);
            AES_SetId(aesWork.data[0], &data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_SET_SEED:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_SEED);
            AES_UNPACK_U128(&data128, &aesWork.data[1]);
            AES_SetSeed(aesWork.data[0], &data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_SET_KEY2:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_KEY2);
            AES_UNPACK_U128(&data128, &aesWork.data[1]);
            AES_UNPACK_U128(&data128b, &aesWork.data[17]);
            AES_SetKey2(aesWork.data[1], &data128, &data128b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;
#else
        case AES_PXI_COMMAND_SET_GENERAL_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_GENERAL_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_SetGeneralKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_SET_SYSTEM_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_SYSTEM_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_SetSystemKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_SET_GAME_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_GAME_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_SetGameKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_SET_SPECIAL_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_SPECIAL_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_SetSpecialKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_SET_ALTERNATIVE_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_ALTERNATIVE_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AESi_SetAlternativeKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

#endif
        case AES_PXI_COMMAND_START_CCM_DEC:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_CCM_DEC);
            AES_UNPACK_U96(&data96, &aesWork.data[0]);
            AES_UNPACK_U128(&data128, &aesWork.data[12]);
            AES_UNPACK_U32(&data32a, &aesWork.data[28]);
            AES_UNPACK_U32(&data32b, &aesWork.data[32]);
            AES_StartCcmDec(&data96, &data128, data32a, data32b, (BOOL)aesWork.data[36]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_START_CCM_DEC_NOMAC:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_CCM_DEC_NOMAC);
            AES_UNPACK_U96(&data96, &aesWork.data[0]);
            AES_UNPACK_U32(&data32a, &aesWork.data[12]);
            AES_UNPACK_U32(&data32b, &aesWork.data[16]);
            AES_StartCcmDec(&data96, NULL, data32a, data32b, (BOOL)aesWork.data[20]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_START_CCM_ENC:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_CCM_ENC);
            AES_UNPACK_U96(&data96, &aesWork.data[0]);
            AES_UNPACK_U32(&data32a, &aesWork.data[12]);
            AES_UNPACK_U32(&data32b, &aesWork.data[16]);
            AES_StartCcmEnc(&data96, data32a, data32b, (BOOL)aesWork.data[20]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_START_CTR:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_CTR);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_UNPACK_U32(&data32a, &aesWork.data[16]);
            AES_StartCtrDec(&data128, data32a);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_START_DMA_SEND:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_DMA_SEND);
            AES_UNPACK_U32(&data32a, &aesWork.data[1]);
            AES_UNPACK_U32(&data32b, &aesWork.data[5]);
            AES_DmaSendAsync(aesWork.data[0], (void*)data32a, data32b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_START_DMA_RECV:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_DMA_RECV);
            AES_UNPACK_U32(&data32a, &aesWork.data[1]);
            AES_UNPACK_U32(&data32b, &aesWork.data[5]);
            AES_DmaRecvAsync(aesWork.data[0], (void*)data32a, data32b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_WAIT_DMA:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_WAIT_DMA);
            MIi_WaitExDma(aesWork.data[0]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_CPU_SEND:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_DMA_SEND);
            AES_UNPACK_U32(&data32a, &aesWork.data[0]);
            AES_UNPACK_U32(&data32b, &aesWork.data[4]);
            AES_CpuSend((void*)data32a, data32b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

        case AES_PXI_COMMAND_CPU_RECV:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_DMA_RECV);
            AES_UNPACK_U32(&data32a, &aesWork.data[0]);
            AES_UNPACK_U32(&data32b, &aesWork.data[4]);
            AES_CpuRecv((void*)data32a, data32b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9�ɏ����̐�����ʒB
            break;

            // �T�|�[�g���Ȃ��R�}���h
        default:
            AesReturnResult(aesWork.command, AES_PXI_RESULT_INVALID_COMMAND);
        }
    }
}
