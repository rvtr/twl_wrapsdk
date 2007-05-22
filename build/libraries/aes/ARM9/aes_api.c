/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - aes
  File:     aes.c

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
// 詰めてコピーする
#define AES_PACK_U32(d, s)                      \
    ((d)[0] = (u8)((*((u32*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u32*)s) >>  8) & 0xFF),  \
     (d)[2] = (u8)((*((u32*)s) >> 16) & 0xFF),  \
     (d)[3] = (u8)((*((u32*)s) >> 24) & 0xFF))

#define AES_PACK_U96(d, s)                      \
    (AES_PACK_U32((d) + 0, ((u96*)s)->e + 0),   \
     AES_PACK_U32((d) + 4, ((u96*)s)->e + 1),   \
     AES_PACK_U32((d) + 8, ((u96*)s)->e + 2))

#define AES_PACK_U128(d, s)                     \
    (AES_PACK_U32((d) +  0, ((u128*)s)->e + 0), \
     AES_PACK_U32((d) +  4, ((u128*)s)->e + 1), \
     AES_PACK_U32((d) +  8, ((u128*)s)->e + 2), \
     AES_PACK_U32((d) + 12, ((u128*)s)->e + 3))

// データ数を設定する
#define AES_SUBSEQUENT_NUMS(a)  (((a) << AES_PXI_SUBSEQUENT_NUMS_SHIFT) & AES_PXI_SUBSEQUENT_NUMS_MASK)

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct AESWork
{
    BOOL        lock;
    AESCallback callback;
    AESResult   result;
    void        *callbackArg;
}
AESWork;

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static BOOL aesInitialized;
static AESWork aesWork;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static BOOL AesSendPxiCommand(AESPxiCommand command, u8 size, u8 data);
static BOOL AesSendPxiData(u8 *pData);
static void AesPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void AesSyncCallback(AESResult result, void *arg);
static void AesCallCallbackAndUnlock(AESResult result);
static void AesWaitBusy(void);

/*---------------------------------------------------------------------------*
  Name:         AES_Init

  Description:  AESライブラリを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Init(void)
{
    // 初期化済みを確認
    if (aesInitialized)
    {
        return;
    }
    aesInitialized = 1;

    // 変数初期化
    aesWork.lock = FALSE;
    aesWork.callback = NULL;

    // PXI関連を初期化
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_AES, PXI_PROC_ARM7))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_AES, AesPxiCallback);
}

/*---------------------------------------------------------------------------*
  Name:         AES_ResetAsync

  Description:  stop and reset AES block. but key resisters do not clear.
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_ResetAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_RESET;
    const u8            size    = AES_PXI_SIZE_RESET;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_Reset

  Description:  stop and reset AES block. but key resisters do not clear.
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_Reset(void)
{
    aesWork.result = AES_ResetAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_IsBusyAsync

  Description:  check whether AES is busy or not
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_IsBusyAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_IS_BUSY;
    const u8            size    = AES_PXI_SIZE_IS_BUSY;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_IsBusy

  Description:  check whether AES is busy or not
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_IsBusy(void)
{
    aesWork.result = AES_IsBusyAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_WaitAsync

  Description:  wait while AES is busy
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_WAIT;
    const u8            size    = AES_PXI_SIZE_WAIT;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_Wait

  Description:  wait while AES is busy
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_Wait(void)
{
    aesWork.result = AES_WaitAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_InputFifoIsFullAsync

  Description:  check whether AES input fifo is full or not
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_InputFifoIsFullAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_INPUT_FIFO_IS_FULL;
    const u8            size    = AES_PXI_SIZE_INPUT_FIFO_IS_FULL;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_InputFifoIsFull

  Description:  check whether AES input fifo is full or not
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_InputFifoIsFull(void)
{
    aesWork.result = AES_InputFifoIsFullAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_OutputFifoIsEmptyAsync

  Description:  check whether AES output fifo is empty or not
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_OutputFifoIsEmptyAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_OUTPUT_FIFO_IS_EMPTY;
    const u8            size    = AES_PXI_SIZE_OUTPUT_FIFO_IS_EMPTY;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_OutputFifoIsEmpty

  Description:  check whether AES output fifo is empty or not
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_OutputFifoIsEmpty(void)
{
    aesWork.result = AES_OutputFifoIsEmptyAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_WaitInputFifoNotFullAsync

  Description:  wait while AES input fifo is full.
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitInputFifoNotFullAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_WAIT_INPUT_FIFO_NOT_FULL;
    const u8            size    = AES_PXI_SIZE_WAIT_INPUT_FIFO_NOT_FULL;
    OSIntrMode enabled;


    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_WaitInputFifoNotFull

  Description:  wait while AES input fifo is full.
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitInputFifoNotFull(void)
{
    aesWork.result = AES_WaitInputFifoNotFullAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_WaitOutputFifoNotEmptyAsync

  Description:  wait while AES output fifo is empty.
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitOutputFifoNotEmptyAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_WAIT_OUTPUT_FIFO_NOT_EMPTY;
    const u8            size    = AES_PXI_SIZE_WAIT_OUTPUT_FIFO_NOT_EMPTY;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_WaitOutputFifoNotEmpty

  Description:  wait while AES output fifo is empty.
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitOutputFifoNotEmpty(void)
{
    aesWork.result = AES_WaitOutputFifoNotEmptyAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_IsValidAsync

  Description:  check whether CCM decryption is valid or not.
                it may return TRUE just after CCM decryption has been completed.
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_IsValidAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_IS_VALID;
    const u8            size    = AES_PXI_SIZE_IS_VALID;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_IsValid

  Description:  check whether CCM decryption is valid or not.
                it may return TRUE just after CCM decryption has been completed.
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_IsValid(void)
{
    aesWork.result = AES_IsValidAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

#if 0
/*---------------------------------------------------------------------------*
  Name:         AES_SelectKeyAsync

  Description:  select key from one of four key registers
                Note: SHOULD be called after AES_Set*() prior to AES_Start*().
                async version

  Arguments:    keyNo       - key group number.
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SelectKeyAsync(u32 keyNo, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SELECT_KEY;
    const u8            size    = AES_PXI_SIZE_SELECT_KEY;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, (u8)keyNo) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SelectKey

  Description:  select key from one of four key registers
                Note: SHOULD be called after AES_Set*() prior to AES_Start*().
                sync version.

  Arguments:    keyNo   - key group number.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SelectKey(u32 keyNo)
{
    aesWork.result = AES_SelectKeyAsync(keyNo, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetKeyAsync

  Description:  set key data into key register
                Note: SHOULD be called after AES_Set*() prior to AES_Start*().
                async version.

  Arguments:    keyNo   - key group number.
                pKey    - pointer to key data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetKeyAsync(u32 keyNo, const u128 *pKey, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SET_KEY;
    const u8            size    = AES_PXI_SIZE_SET_KEY;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    AES_ASSERT_KEYNO(keyNo);
    SDK_NULL_ASSERT(pKey);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)keyNo;
    AES_PACK_U128(&data[1], pKey);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetKey

  Description:  set key data into key register
                sync version.

  Arguments:    keyNo   - key group number.
                pKey    - pointer to key data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetKey(u32 keyNo, const u128 *pKey)
{
    aesWork.result = AES_SetKeyAsync(keyNo, pKey, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetIdAsync

  Description:  set id data into id register
                Note: never set key register with id and seed
                async version.

  Arguments:    keyNo       - key group number.
                pId         - pointer to id data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetIdAsync(u32 keyNo, const u128 *pId, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SET_ID;
    const u8            size    = AES_PXI_SIZE_SET_ID;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    AES_ASSERT_KEYNO(keyNo);
    SDK_NULL_ASSERT(pId);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)keyNo;
    AES_PACK_U128(&data[1], pId);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetId

  Description:  set id data into id register
                Note: never set key register with id and seed
                sync version.

  Arguments:    keyNo   - key group number.
                pId     - pointer to id data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetId(u32 keyNo, const u128 *pId)
{
    aesWork.result = AES_SetIdAsync(keyNo, pId, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetSeedAsync

  Description:  set id data into id register
                Note: automatically set associated key register with id and seed
                async version.

  Arguments:    keyNo       - key group number.
                pSeed       - pointer to seed data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetSeedAsync(u32 keyNo, const u128 *pSeed, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SET_SEED;
    const u8            size    = AES_PXI_SIZE_SET_SEED;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    AES_ASSERT_KEYNO(keyNo);
    SDK_NULL_ASSERT(pSeed);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)keyNo;
    AES_PACK_U128(&data[1], pSeed);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetSeed

  Description:  set id data into id register
                Note: automatically set associated key register with id and seed
                sync version.

  Arguments:    keyNo   - key group number.
                pSeed   - pointer to seed data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetSeed(u32 keyNo, const u128 *pSeed)
{
    aesWork.result = AES_SetSeedAsync(keyNo, pSeed, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetKey2Async

  Description:  set seed/id data into seed/id register
                Note: automatically set associated key register with id and seed
                async version.

  Arguments:    keyNo       - key group number.
                pId         - pointer to id data
                pSeed       - pointer to seed data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetKey2Async(u32 keyNo, const u128 *pId, const u128 *pSeed, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SET_KEY2;
    const u8            size    = AES_PXI_SIZE_SET_KEY2;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    AES_ASSERT_KEYNO(keyNo);
    SDK_NULL_ASSERT(pId);
    SDK_NULL_ASSERT(pSeed);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)keyNo;
    AES_PACK_U128(&data[1], pId);
    AES_PACK_U128(&data[17], pSeed);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetKey2

  Description:  set seed/id data into seed/id register
                Note: automatically set associated key register with id and seed
                sync version.

  Arguments:    keyNo   - key group number.
                pId      - pointer to id data
                pSeed    - pointer to seed data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetKey2(u32 keyNo, const u128 *pId, const u128 *pSeed)
{
    aesWork.result = AES_SetKey2Async(keyNo, pId, pSeed, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}
#else
/*---------------------------------------------------------------------------*
  Name:         AES_SetGeneralKeyAsync

  Description:  set AES key normally

  Arguments:    pKey        - pointer to key data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetGeneralKeyAsync(const u128 *pKey, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SET_GENERAL_KEY;
    const u8            size    = AES_PXI_SIZE_SET_GENERAL_KEY;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(pKey);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    AES_PACK_U128(&data[0], pKey);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetGeneralKey

  Description:  set AES key normally
                sync version.

  Arguments:    pKey        - pointer to key data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetGeneralKey(const u128 *pKey)
{
    aesWork.result = AES_SetGeneralKeyAsync(pKey, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetSystemKeyAsync

  Description:  set AES key to be restricted to the system (device)
                NOTE: if data encrypted this key, other system cannot
                decrypt with this key. but another key can decrypt it if
                another key was found for another system.

  Arguments:    pKey        - pointer to key data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetSystemKeyAsync(const u128 *pKey, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SET_SYSTEM_KEY;
    const u8            size    = AES_PXI_SIZE_SET_SYSTEM_KEY;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(pKey);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    AES_PACK_U128(&data[0], pKey);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetSystemKey

  Description:  set AES key to be restricted to the system (device)
                NOTE: if data encrypted this key, other system cannot
                decrypt with this key. but another key can decrypt it if
                another key was found for another system.
                sync version.

  Arguments:    pKey        - pointer to key data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetSystemKey(const u128 *pKey)
{
    aesWork.result = AES_SetSystemKeyAsync(pKey, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetGameKeyAsync

  Description:  set AES key to be restricted to the application (initial code).
                NOTE: if data encrypted this key, other application cannot
                decrypt with this key. but another key can decrypt it if
                another key was found for another application.

  Arguments:    pKey        - pointer to key data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetGameKeyAsync(const u128 *pKey, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SET_GAME_KEY;
    const u8            size    = AES_PXI_SIZE_SET_GAME_KEY;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(pKey);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    AES_PACK_U128(&data[0], pKey);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetGameKey

  Description:  set AES key to be restricted to the application (initial code).
                NOTE: if data encrypted this key, other application cannot
                decrypt with this key. but another key can decrypt it if
                another key was found for another application.
                sync version.

  Arguments:    pKey        - pointer to key data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetGameKey(const u128 *pKey)
{
    aesWork.result = AES_SetGameKeyAsync(pKey, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetSpecialKeyAsync

  Description:  set AES key to be restricted to the application and the system.
                NOTE: if data encrypted this key, other application or other
                system cannot decrypt with this key. but another key can
                decrypt it if another key was found for another application
                and/or another system.

  Arguments:    pKey        - pointer to key data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetSpecialKeyAsync(const u128 *pKey, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SET_SPECIAL_KEY;
    const u8            size    = AES_PXI_SIZE_SET_SPECIAL_KEY;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(pKey);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    AES_PACK_U128(&data[0], pKey);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetSpecialKey

  Description:  set AES key to be restricted to the application and the system.
                NOTE: if data encrypted this key, other application or other
                system cannot decrypt with this key. but another key can
                decrypt it if another key was found for another application
                and/or another system.
                sync version.

  Arguments:    pKey        - pointer to key data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetSpecialKey(const u128 *pKey)
{
    aesWork.result = AES_SetSpecialKeyAsync(pKey, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

#if 0   // for loader
/*---------------------------------------------------------------------------*
  Name:         AESi_SetAlternativeKeyAsync

  Description:  set mangled AES key

  Arguments:    pKey        - pointer to key data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AESi_SetAlternativeKeyAsync(const u128 *pKey, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_SET_ALTERNATIVE_KEY;
    const u8            size    = AES_PXI_SIZE_SET_ALTERNATIVE_KEY;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(pKey);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    AES_PACK_U128(&data[0], pKey);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetAlternativeKey

  Description:  set mangled AES key
                sync version.

  Arguments:    pKey        - pointer to key data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetAlternativeKey(const u128 *pKey)
{
    aesWork.result = AES_SetAlternativeKeyAsync(pKey, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}
#endif
#endif

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmDecAsync

  Description:  start AES engine for AES-CCM decryption.
                async version.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                mac         - pointer to 128-bit mac data.
                              if NULL, it assumes the mac will be sent from
                              the input FIFO.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              it excludes mac length even if the mac will be
                              sent from the input FIFO.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCcmDecAsync(const u96 *nonce, const u128 *mac, u32 adataLength, u32 pdataLength, BOOL isDistA, AESCallback callback, void *arg)
{
    OSIntrMode enabled;
    int i;

    SDK_NULL_ASSERT(nonce);
    AES_ASSERT_DATA_LENGTH(adataLength);
    AES_ASSERT_DATA_LENGTH(pdataLength);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    if (mac)
    {
        const AESPxiCommand command = AES_PXI_COMMAND_START_CCM_DEC;
        const u8            size    = AES_PXI_SIZE_START_CCM_DEC;
        u8  data[size];

        // データ作成
        AES_PACK_U96(&data[0], nonce);
        AES_PACK_U128(&data[12], mac);
        AES_PACK_U32(&data[28], &adataLength);
        AES_PACK_U32(&data[32], &pdataLength);
        data[36] = (u8)isDistA;

        // コマンド送信
        if (AesSendPxiCommand(command, size, data[0]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
        for (i = 1; i < size; i+=3) {
            if (AesSendPxiData(&data[i]) == FALSE)
            {
                return AES_RESULT_SEND_ERROR;
            }
        }
    }
    else
    {
        const AESPxiCommand command = AES_PXI_COMMAND_START_CCM_DEC_NOMAC;
        const u8            size    = AES_PXI_SIZE_START_CCM_DEC_NOMAC;
        u8  data[size];

        // データ作成
        AES_PACK_U96(&data[0], nonce);
        AES_PACK_U32(&data[12], &adataLength);
        AES_PACK_U32(&data[16], &pdataLength);
        data[20] = (u8)isDistA;

        // コマンド送信
        if (AesSendPxiCommand(command, size, data[0]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
        for (i = 1; i < size; i+=3) {
            if (AesSendPxiData(&data[i]) == FALSE)
            {
                return AES_RESULT_SEND_ERROR;
            }
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmDec

  Description:  start AES engine for AES-CCM decryption.
                sync version.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                mac         - pointer to 128-bit mac data.
                              if NULL, it assumes the mac will be sent from
                              the input FIFO.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              it excludes mac length even if the mac will be
                              sent from the input FIFO.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCcmDec(const u96 *nonce, const u128 *mac, u32 adataLength, u32 pdataLength, BOOL isDistA)
{
    aesWork.result = AES_StartCcmDecAsync(nonce, mac, adataLength, pdataLength, isDistA, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmEncAsync

  Description:  start AES engine for AES-CCM encryption.
                async version.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              note that output length will be extended for
                              the mac data.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCcmEncAsync(const u96 *nonce, u32 adataLength, u32 pdataLength, BOOL isDistA, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_START_CCM_ENC;
    const u8            size    = AES_PXI_SIZE_START_CCM_ENC;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(nonce);
    AES_ASSERT_DATA_LENGTH(adataLength);
    AES_ASSERT_DATA_LENGTH(pdataLength);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    AES_PACK_U96(&data[0], nonce);
    AES_PACK_U32(&data[12], &adataLength);
    AES_PACK_U32(&data[16], &pdataLength);
    data[20] = (u8)isDistA;

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmEnc

  Description:  start AES engine for AES-CCM encryption.
                sync version.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              note that output length will be extended for
                              the mac data.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCcmEnc(const u96 *nonce, u32 adataLength, u32 pdataLength, BOOL isDistA)
{
    aesWork.result = AES_StartCcmEncAsync(nonce, adataLength, pdataLength, isDistA, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartCtrDecAsync

  Description:  start AES engine for AES-CTR encryption/decryption.
                async version.

  Arguments:    iv          - pointer to 128-bit iv data.
                length      - length of the data.
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCtrDecAsync(const u128 *iv, u32 length, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_START_CTR;
    const u8            size    = AES_PXI_SIZE_START_CTR;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(iv);
    AES_ASSERT_DATA_LENGTH(length);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    AES_PACK_U128(&data[0], iv);
    AES_PACK_U32(&data[16], &length);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartCtrDec

  Description:  start AES engine for AES-CTR encryption/decryption.
                sync version.

  Arguments:    iv          - pointer to 128-bit iv data.
                length      - length of the data.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCtrDec(const u128 *iv, u32 length)
{
    aesWork.result = AES_StartCtrDecAsync(iv, length, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartDmaSendAsync

  Description:  AES DMA send by AES input FIFO timing.
                Should call prior to the AES_Start*().
                async version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                src     : source address
                length  : transfer size (byte) (multiple of 16 bytes)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartDmaSendAsync(u32 dmaNo, const void *src, u32 length, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_START_DMA_SEND;
    const u8            size    = AES_PXI_SIZE_START_DMA_SEND;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(src);
    AES_ASSERT_DATA_LENGTH(length);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)dmaNo;
    AES_PACK_U32(&data[1], &src);
    AES_PACK_U32(&data[5], &length);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartDmaSend

  Description:  AES DMA send by AES input FIFO timing.
                Should call prior to the AES_Start*().
                sync version.
                NOTE: never wait to done to transfer!

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                src     : source address
                length  : transfer size (byte) (multiple of 16 bytes)

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartDmaSend(u32 dmaNo, const void *src, u32 length)
{
    aesWork.result = AES_StartDmaSendAsync(dmaNo, src, length, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartDmaRecvAsync

  Description:  AES DMA receive by AES output FIFO timing.
                Should call prior to the AES_Start*().
                async version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                dest    : destination address
                length  : transfer size (byte) (multiple of 16 bytes)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartDmaRecvAsync(u32 dmaNo, const void *dest, u32 length, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_START_DMA_RECV;
    const u8            size    = AES_PXI_SIZE_START_DMA_RECV;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(dest);
    AES_ASSERT_DATA_LENGTH(length);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)dmaNo;
    AES_PACK_U32(&data[1], &dest);
    AES_PACK_U32(&data[5], &length);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartDmaRecv

  Description:  AES DMA receive by AES output FIFO timing.
                Should call prior to the AES_Start*().
                sync version.
                NOTE: never wait to done to transfer!

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                dest    : destination address
                length  : transfer size (byte) (multiple of 16 bytes)

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartDmaRecv(u32 dmaNo, const void *dest, u32 length)
{
    aesWork.result = AES_StartDmaRecvAsync(dmaNo, dest, length, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}
/*---------------------------------------------------------------------------*
  Name:         AES_WaitDmaAsync

  Description:  Waiting ARM7 side DMA is done.
                It may move to MI.
                async version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitDmaAsync(u32 dmaNo, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_WAIT_DMA;
    const u8            size    = AES_PXI_SIZE_WAIT_DMA;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, (u8)dmaNo) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_WaitDma

  Description:  Waiting ARM7 side DMA is done.
                It may move to MI.
                sync version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitDma(u32 dmaNo)
{
    aesWork.result = AES_WaitDmaAsync(dmaNo, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_CpuSendAsync

  Description:  send data to AES input fifo by CPU.
                Should call prior to the AES_Start*().
                async version.
                NOTE: callback will be called after to done to transfer!
                CAUTION: Cannot use AES_CPUSend*() and AES_CPURecv*() at same time.

  Arguments:    src     : source address
                length  : transfer size (byte) (multiple of 16 bytes)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_CpuSendAsync(const void *src, u32 length, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_CPU_SEND;
    const u8            size    = AES_PXI_SIZE_CPU_SEND;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(src);
    AES_ASSERT_DATA_LENGTH(length);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    AES_PACK_U32(&data[0], &src);
    AES_PACK_U32(&data[4], &length);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_CpuSend

  Description:  send data to AES input fifo by CPU.
                Should call prior to the AES_Start*().
                sync version.
                CAUTION: Cannot use AES_CPUSend*() and AES_CPURecv*() at same time.

  Arguments:    src     : source address
                length  : transfer size (byte) (multiple of 16 bytes)

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_CpuSend(const void *src, u32 length)
{
    aesWork.result = AES_CpuSendAsync(src, length, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_CpuRecvAsync

  Description:  receive data from AES input fifo by CPU.
                Should call prior to the AES_Start*().
                async version.
                NOTE: callback will be called after to done to transfer!
                CAUTION: Cannot use AES_CPUSend*() and AES_CPURecv*() at same time.

  Arguments:    dest    : destination address
                length  : transfer size (byte) (multiple of 16 bytes)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_CpuRecvAsync(const void *dest, u32 length, AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_CPU_RECV;
    const u8            size    = AES_PXI_SIZE_CPU_RECV;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(dest);
    AES_ASSERT_DATA_LENGTH(length);
    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    // データ作成
    AES_PACK_U32(&data[0], &dest);
    AES_PACK_U32(&data[4], &length);

    // コマンド送信
    if (AesSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return AES_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        if (AesSendPxiData(&data[i]) == FALSE)
        {
            return AES_RESULT_SEND_ERROR;
        }
    }

    return AES_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         AES_CpuRecv

  Description:  receive data from AES input fifo by CPU.
                Should call prior to the AES_Start*().
                sync version.
                CAUTION: Cannot use AES_CPUSend*() and AES_CPURecv*() at same time.

  Arguments:    dest    : destination address
                length  : transfer size (byte) (multiple of 16 bytes)

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_CpuRecv(const void *dest, u32 length)
{
    aesWork.result = AES_CpuRecvAsync(dest, length, AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_TryLockAsync

  Description:  AESライブラリをARM7から使われないように試みる。
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_TryLockAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_TRY_LOCK;
    const u8            size    = AES_PXI_SIZE_TRY_LOCK;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_TryLock

  Description:  AESライブラリをARM7から使われないように試みる。
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_TryLock(void)
{
    aesWork.result = AES_TryLockAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AES_UnlockAsync

  Description:  AESライブラリのARM9側のロックを解除する
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_UnlockAsync(AESCallback callback, void *arg)
{
    const AESPxiCommand command = AES_PXI_COMMAND_UNLOCK;
    const u8            size    = AES_PXI_SIZE_UNLOCK;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    enabled = OS_DisableInterrupts();
    if (aesWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return AES_RESULT_BUSY;
    }
    aesWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    aesWork.callback = callback;
    aesWork.callbackArg = arg;

    return AesSendPxiCommand(command, size, 0) ? AES_RESULT_SUCCESS : AES_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         AES_Unlock

  Description:  AESライブラリをARM7から使われないように試みる。
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_Unlock(void)
{
    aesWork.result = AES_UnlockAsync(AesSyncCallback, 0);
    if (aesWork.result == AES_RESULT_SUCCESS)
    {
        AesWaitBusy();
    }
    return aesWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         AesSendPxiCommand

  Description:  指定先頭コマンドをPXI経由でARM7に送信する。

  Arguments:    command     - 対象コマンド
                size        - 送信データサイズ (バイト単位)
                data        - 先頭データ (1バイトのみ)

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL AesSendPxiCommand(AESPxiCommand command, u8 size, u8 data)
{
    u32 pxiData = (u32)(AES_PXI_START_BIT |
            ((command << AES_PXI_COMMAND_SHIFT) & AES_PXI_COMMAND_MASK) |
            ((size << AES_PXI_DATA_NUMS_SHIFT) & AES_PXI_DATA_NUMS_MASK) |
            ((data << AES_PXI_1ST_DATA_SHIFT) & AES_PXI_1ST_DATA_MASK));
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_AES, pxiData, 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         AesSendPxiData

  Description:  指定後続データをPXI経由でARM7に送信する。

  Arguments:    pData   - 3バイトデータの先頭へのポインタ

  Returns:      BOOL    - PXIに対して送信が完了した場合TRUEを、
                          PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL AesSendPxiData(u8 *pData)
{
    u32 pxiData = (u32)((pData[0] << 16) | (pData[1] << 8) | pData[2]);
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_AES, pxiData, 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         AesPxiCallback

  Description:  非同期RTC関数用の共通コールバック関数。

  Arguments:    tag -  PXI tag which show message type.
                data - message from ARM7.
                err -  PXI transfer error flag.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AesPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    AESResult   result;
    AESPxiResult pxiResult;
    u8          command;

    // PXI通信エラーを確認
    if (err)
    {
        // シーケンスを強制終了
        AesCallCallbackAndUnlock(AES_RESULT_FATAL_ERROR);
        return;
    }

    // 受信データを解析
    SDK_ASSERT((data & (AES_PXI_START_BIT | AES_PXI_RESULT_BIT)) == (AES_PXI_START_BIT | AES_PXI_RESULT_BIT));

    command = (u8)((data & AES_PXI_COMMAND_MASK) >> AES_PXI_COMMAND_SHIFT);
    pxiResult = (AESPxiResult)((data & AES_PXI_1ST_DATA_MASK) >> AES_PXI_1ST_DATA_SHIFT);

    // 処理結果を確認
    switch (pxiResult)
    {
    case AES_PXI_RESULT_SUCCESS:
        result = AES_RESULT_SUCCESS;
        break;
    case AES_PXI_RESULT_SUCCESS_TRUE:
        result = AES_RESULT_SUCCESS_TRUE;
        break;
    case AES_PXI_RESULT_SUCCESS_FALSE:
        result = AES_RESULT_SUCCESS_FALSE;
        break;
    case AES_PXI_RESULT_INVALID_COMMAND:
        result = AES_RESULT_INVALID_COMMAND;
        break;
    case AES_PXI_RESULT_INVALID_PARAMETER:
        result = AES_RESULT_ILLEGAL_PARAMETER;
        break;
    case AES_PXI_RESULT_ILLEGAL_STATUS:
        result = AES_RESULT_ILLEGAL_STATUS;
        break;
    case AES_PXI_RESULT_BUSY:
        result = AES_RESULT_BUSY;
        break;
    default:
        result = AES_RESULT_FATAL_ERROR;
    }

    // コールバックの呼び出し
    AesCallCallbackAndUnlock(result);
}

/*---------------------------------------------------------------------------*
  Name:         AesSyncCallback

  Description:  同期API用のコールバック

  Arguments:    result  - ARM7から送られた結果
                arg     - 未使用

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AesSyncCallback(AESResult result, void *arg)
{
#pragma unused(arg)
    aesWork.result = result;
}

/*---------------------------------------------------------------------------*
  Name:         AesCallCallbackAndUnlock

  Description:  コールバックの呼び出しとロックの解除を行う

  Arguments:    result  - ARM7から送られた結果

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AesCallCallbackAndUnlock(AESResult result)
{
    AESCallback cb;

    if (aesWork.lock)
    {
        aesWork.lock = FALSE;
    }
    if (aesWork.callback)
    {
        cb = aesWork.callback;
        aesWork.callback = NULL;
        cb(result, aesWork.callbackArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         AesWaitBusy

  Description:  AESの非同期処理がロックされている間待つ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if 0
#include    <nitro/code32.h>
static asm void AesWaitBusy(void)
{
    ldr     r12,    =aesWork.lock
loop:
    ldr     r0,     [ r12,  #0 ]
    cmp     r0,     #TRUE
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>
#else
extern void PXIi_HandlerRecvFifoNotEmpty(void);
static void AesWaitBusy(void)
{
    volatile BOOL *p = &aesWork.lock;

    while (*p)
    {
        if (OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE)
        {
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}
#endif
