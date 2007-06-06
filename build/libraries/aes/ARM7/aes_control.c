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
    定数定義
 *---------------------------------------------------------------------------*/
#define AES_PXI_SIZE_CHECK(nums)                                            \
    if (aesWork.total != (nums)) {                                          \
        aesWork.locked = AES_UNLOCKED;                                      \
        AesReturnResult(aesWork.command, AES_PXI_RESULT_INVALID_PARAMETER); \
        break;                                                              \
    }

// アライメント調整してコピーする
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
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static BOOL aesInitialized;             // 初期化確認フラグ
static AESWork aesWork;                 // ワーク変数をまとめた構造体

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void AesPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void AesReturnResult(u8 command, AESPxiResult result);
static void AesThread(void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_Init

  Description:  AESライブラリを初期化する。

  Arguments:    priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Init(u32 priority)
{
    // 初期化済みを確認
    if (aesInitialized)
    {
        return;
    }
    aesInitialized = 1;

    // 変数初期化
    aesWork.locked = AES_UNLOCKED;

    // PXI関連を初期化
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_AES, AesPxiCallback);

    // 実処理を行うスレッドを作成
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

  Description:  AESライブラリをARM9から使われないようにする。

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

  Description:  AESライブラリをARM9から使われないように試みる。

  Arguments:    None.

  Returns:      BOOL - ロックできた場合はTRUEを返す
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

  Description:  AESライブラリのARM7側のロックを解除する

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

  Description:  PXI経由で受信したデータを解析する。

  Arguments:    tag -  PXI種別を示すタグ。
                data - 受信したデータ。下位26bitが有効。
                err -  PXI通信におけるエラーフラグ。
                       ARM9側にて同種別のPXIが初期化されていないことを示す。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AesPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    // PXI通信エラーをチェック
    if (err)
    {
        return;
    }
    // 先頭データ
    if (data & AES_PXI_START_BIT)
    {
        aesWork.total = (u8)((data & AES_PXI_DATA_NUMS_MASK) >> AES_PXI_DATA_NUMS_SHIFT);
        aesWork.current = 0;
        aesWork.command = (u8)((data & AES_PXI_COMMAND_MASK) >> AES_PXI_COMMAND_SHIFT);
        aesWork.data[aesWork.current++] = (u8)((data & AES_PXI_1ST_DATA_MASK) >> AES_PXI_1ST_DATA_SHIFT);
//OS_TPrintf("START_BIT (total=%d, command=%X).\n", aesWork.total, aesWork.command);
    }
    // 後続データ
    else
    {
        aesWork.data[aesWork.current++] = (u8)((data & 0xFF0000) >> 16);
        aesWork.data[aesWork.current++] = (u8)((data & 0x00FF00) >> 8);
        aesWork.data[aesWork.current++] = (u8)((data & 0x0000FF) >> 0);
    }

    // パケット完成
    if (aesWork.current >= aesWork.total)   // 最大で2つ余分に取得する
    {
        // 受信したコマンドを解析
        OSIntrMode enabled;

        switch (aesWork.command)
        {
            // 既知のコマンド群
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
            // スレッドを再開
            if (!OS_SendMessage(&aesWork.msgQ, NULL, OS_MESSAGE_NOBLOCK))
            {
                AesReturnResult(aesWork.command, AES_PXI_RESULT_FATAL_ERROR);
            }
            break;

            // ロック：特殊コマンド (ここで処理する)
        case AES_PXI_COMMAND_TRY_LOCK:
            enabled = OS_DisableInterrupts();
            if (aesWork.locked == AES_UNLOCKED)
            {
                // 排他ロック施錠
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

            // アンロック：特殊コマンド (ここで処理する)
        case AES_PXI_COMMAND_UNLOCK:
            enabled = OS_DisableInterrupts();
            if (aesWork.locked == AES_LOCKED_BY_ARM9)
            {
                // 排他ロック開錠
                aesWork.locked = AES_UNLOCKED;
            }
            (void)OS_RestoreInterrupts(enabled);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);
            break;

            // 未知のコマンド
        default:
            AesReturnResult(aesWork.command, AES_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         AesReturnResult

  Description:  PXI経由で処理結果をARM9に送信する。

  Arguments:    command - 処理したコマンド。
                result -  処理結果。

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

  Description:  AES操作の実処理を行うスレッド。

  Arguments:    arg - 使用しない。

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
        // メッセージが発行されるまで寝る
        (void)OS_ReceiveMessage(&(aesWork.msgQ), &msg, OS_MESSAGE_BLOCK);

        // コマンドに従って各種処理を実行
        switch (aesWork.command)
        {
            // AESエンジンのリセット
        case AES_PXI_COMMAND_RESET:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_RESET);
            AES_Reset();
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

            // AESエンジンのビジーチェック
        case AES_PXI_COMMAND_IS_BUSY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_IS_BUSY);
            result = AES_IsBusy();
            AesReturnResult(aesWork.command, result ?
                AES_PXI_RESULT_SUCCESS_TRUE : AES_PXI_RESULT_SUCCESS_FALSE);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_WAIT:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_WAIT);
            AES_Wait();
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_INPUT_FIFO_IS_FULL:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_INPUT_FIFO_IS_FULL);
            result = AES_InputFifoIsFull();
            AesReturnResult(aesWork.command, result ?
                AES_PXI_RESULT_SUCCESS_TRUE : AES_PXI_RESULT_SUCCESS_FALSE);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_OUTPUT_FIFO_IS_EMPTY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_OUTPUT_FIFO_IS_EMPTY);
            result = AES_OutputFifoIsEmpty();
            AesReturnResult(aesWork.command, result ?
                AES_PXI_RESULT_SUCCESS_TRUE : AES_PXI_RESULT_SUCCESS_FALSE);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_WAIT_INPUT_FIFO_NOT_FULL:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_WAIT_INPUT_FIFO_NOT_FULL);
            AES_WaitInputFifoNotFull();
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_WAIT_OUTPUT_FIFO_NOT_EMPTY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_WAIT_OUTPUT_FIFO_NOT_EMPTY);
            AES_WaitOutputFifoNotEmpty();
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_IS_VALID:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_IS_VALID);
            result = AES_IsValid();
            AesReturnResult(aesWork.command, result ?
                AES_PXI_RESULT_SUCCESS_TRUE : AES_PXI_RESULT_SUCCESS_FALSE);     // ARM9に処理の成功を通達
            break;
#if 0
        case AES_PXI_COMMAND_SELECT_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SELECT_KEY);
            AES_SelectKey(aesWork.data[0]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_SET_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[1]);
            AES_SetKey(aesWork.data[0], &data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_SET_ID:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_ID);
            AES_UNPACK_U128(&data128, &aesWork.data[1]);
            AES_SetId(aesWork.data[0], &data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_SET_SEED:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_SEED);
            AES_UNPACK_U128(&data128, &aesWork.data[1]);
            AES_SetSeed(aesWork.data[0], &data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_SET_KEY2:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_KEY2);
            AES_UNPACK_U128(&data128, &aesWork.data[1]);
            AES_UNPACK_U128(&data128b, &aesWork.data[17]);
            AES_SetKey2(aesWork.data[1], &data128, &data128b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;
#else
        case AES_PXI_COMMAND_SET_GENERAL_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_GENERAL_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_SetGeneralKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_SET_SYSTEM_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_SYSTEM_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_SetSystemKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_SET_GAME_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_GAME_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_SetGameKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_SET_SPECIAL_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_SPECIAL_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_SetSpecialKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_SET_ALTERNATIVE_KEY:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_SET_ALTERNATIVE_KEY);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AESi_SetAlternativeKey(&data128);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

#endif
        case AES_PXI_COMMAND_START_CCM_DEC:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_CCM_DEC);
            AES_UNPACK_U96(&data96, &aesWork.data[0]);
            AES_UNPACK_U128(&data128, &aesWork.data[12]);
            AES_UNPACK_U32(&data32a, &aesWork.data[28]);
            AES_UNPACK_U32(&data32b, &aesWork.data[32]);
            AES_StartCcmDec(&data96, &data128, data32a, data32b, (BOOL)aesWork.data[36]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_START_CCM_DEC_NOMAC:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_CCM_DEC_NOMAC);
            AES_UNPACK_U96(&data96, &aesWork.data[0]);
            AES_UNPACK_U32(&data32a, &aesWork.data[12]);
            AES_UNPACK_U32(&data32b, &aesWork.data[16]);
            AES_StartCcmDec(&data96, NULL, data32a, data32b, (BOOL)aesWork.data[20]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_START_CCM_ENC:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_CCM_ENC);
            AES_UNPACK_U96(&data96, &aesWork.data[0]);
            AES_UNPACK_U32(&data32a, &aesWork.data[12]);
            AES_UNPACK_U32(&data32b, &aesWork.data[16]);
            AES_StartCcmEnc(&data96, data32a, data32b, (BOOL)aesWork.data[20]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_START_CTR:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_CTR);
            AES_UNPACK_U128(&data128, &aesWork.data[0]);
            AES_UNPACK_U32(&data32a, &aesWork.data[16]);
            AES_StartCtrDec(&data128, data32a);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_START_DMA_SEND:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_DMA_SEND);
            AES_UNPACK_U32(&data32a, &aesWork.data[1]);
            AES_UNPACK_U32(&data32b, &aesWork.data[5]);
            AES_DmaSendAsync(aesWork.data[0], (void*)data32a, data32b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_START_DMA_RECV:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_DMA_RECV);
            AES_UNPACK_U32(&data32a, &aesWork.data[1]);
            AES_UNPACK_U32(&data32b, &aesWork.data[5]);
            AES_DmaRecvAsync(aesWork.data[0], (void*)data32a, data32b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_WAIT_DMA:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_WAIT_DMA);
            MIi_WaitExDma(aesWork.data[0]);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_CPU_SEND:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_DMA_SEND);
            AES_UNPACK_U32(&data32a, &aesWork.data[0]);
            AES_UNPACK_U32(&data32b, &aesWork.data[4]);
            AES_CpuSend((void*)data32a, data32b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

        case AES_PXI_COMMAND_CPU_RECV:
            AES_PXI_SIZE_CHECK(AES_PXI_SIZE_START_DMA_RECV);
            AES_UNPACK_U32(&data32a, &aesWork.data[0]);
            AES_UNPACK_U32(&data32b, &aesWork.data[4]);
            AES_CpuRecv((void*)data32a, data32b);
            AesReturnResult(aesWork.command, AES_PXI_RESULT_SUCCESS);     // ARM9に処理の成功を通達
            break;

            // サポートしないコマンド
        default:
            AesReturnResult(aesWork.command, AES_PXI_RESULT_INVALID_COMMAND);
        }
    }
}
