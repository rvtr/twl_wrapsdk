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
#include <twl/camera.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// 詰めてコピーする
#define CAMERA_PACK_U16(d, s)                   \
    ((d)[0] = (u8)((*((u16*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u16*)s) >>  8) & 0xFF))

#define CAMERA_PACK_U32(d, s)                   \
    ((d)[0] = (u8)((*((u32*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u32*)s) >>  8) & 0xFF),  \
     (d)[2] = (u8)((*((u32*)s) >> 16) & 0xFF),  \
     (d)[3] = (u8)((*((u32*)s) >> 24) & 0xFF))

// データ数を設定する
#define CAMERA_SUBSEQUENT_NUMS(a)  (((a) << CAMERA_PXI_SUBSEQUENT_NUMS_SHIFT) & CAMERA_PXI_SUBSEQUENT_NUMS_MASK)

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct CAMERAWork
{
    BOOL            lock;
    CAMERACallback  callback;
    CAMERAResult    result;             // 先頭データだけ別枠
    void            *callbackArg;
    CAMERAPxiCommand    command;        // コマンド種別
    CAMERAPxiResult     pxiResult;      // 先頭データだけ別枠
    u8      current;                    // 受信済みデータ個数 (バイト単位) (先頭を除く!!)
    u8      total;                      // 最終データ個数 (1 + 後続コマンド*3)
    u8      *data;                      // save API arg if any
    size_t  size;                       // save API arg if any
}
CAMERAWork;

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static BOOL cameraInitialized;
static CAMERAWork cameraWork;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static BOOL CameraSendPxiCommand(CAMERAPxiCommand command, u8 size, u8 data);
static void CameraSendPxiData(u8 *pData);
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void CameraSyncCallback(CAMERAResult result, void *arg);
static void CameraCallCallbackAndUnlock(CAMERAResult result);
static void CameraWaitBusy(void);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Init

  Description:  CAMERAライブラリを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_Init(void)
{
    // 初期化済みを確認
    if (cameraInitialized)
    {
        return;
    }
    cameraInitialized = 1;

    // 変数初期化
    cameraWork.lock = FALSE;
    cameraWork.callback = NULL;

    // PXI関連を初期化
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_CAMERA, PXI_PROC_ARM7))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CAMERA, CameraPxiCallback);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInitAsync

  Description:  initialize camera registers via I2C
                async version.

  Arguments:    camera      - one of CameraSelect
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CInitAsync(CameraSelect camera, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_INIT;
    const u8                size    = CAMERA_PXI_SIZE_INIT;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    return CameraSendPxiCommand(command, size, (u8)camera) ? CAMERA_RESULT_SUCCESS : CAMERA_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize camera registers via I2C
                sync version.

  Arguments:    camera      - one of CameraSelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CInit(CameraSelect camera)
{
    cameraWork.result = CAMERA_I2CInitAsync(camera, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivateAsync

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                async version

  Arguments:    camera      - one of CameraSelect
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CActivateAsync(CameraSelect camera, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_ACTIVATE;
    const u8                size    = CAMERA_PXI_SIZE_ACTIVATE;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_BOTH == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    return CameraSendPxiCommand(command, size, (u8)camera) ? CAMERA_RESULT_SUCCESS : CAMERA_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivate

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                sync version.

  Arguments:    camera      - one of CameraSelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CActivate(CameraSelect camera)
{
    cameraWork.result = CAMERA_I2CActivateAsync(camera, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResizeAsync

  Description:  resize CAMERA output image
                async version

  Arguments:    camera      - one of CameraResize
                width       - width of the image
                height      - height of the image
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CResizeAsync(CameraSelect camera, u16 width, u16 height, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_RESIZE;
    const u8                size    = CAMERA_PXI_SIZE_RESIZE;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    CAMERA_PACK_U16(&data[1], &width);
    CAMERA_PACK_U16(&data[3], &height);

    // コマンド送信
    if (CameraSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResize

  Description:  resize CAMERA output image
                sync version.

  Arguments:    camera      - one of CameraResize
                width       - width of the image
                height      - height of the image

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CResize(CameraSelect camera, u16 width, u16 height)
{
    cameraWork.result = CAMERA_I2CResizeAsync(camera, width, height, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRateAsync

  Description:  set CAMERA frame rate (0 means automatic)
                async version

  Arguments:    camera      - one of CameraSelect
                rate        - frame rate (fps)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFrameRateAsync(CameraSelect camera, int rate, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_FRAME_RATE;
    const u8                size    = CAMERA_PXI_SIZE_FRAME_RATE;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    if (rate < 0 || rate > 30)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)rate;

    // コマンド送信
    if (CameraSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRate

  Description:  set CAMERA frame rate (0 means automatic)
                sync version.

  Arguments:    camera      - one of CameraSelect
                rate        - frame rate (fps)

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFrameRate(CameraSelect camera, int rate)
{
    cameraWork.result = CAMERA_I2CFrameRateAsync(camera, rate, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffectAsync

  Description:  set CAMERA frame rate (0 means automatic)
                async version

  Arguments:    camera      - one of CameraSelect
                effect      - one of CameraEffect
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CEffectAsync(CameraSelect camera, CameraEffect effect, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_EFFECT;
    const u8                size    = CAMERA_PXI_SIZE_EFFECT;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)effect;

    // コマンド送信
    if (CameraSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffect

  Description:  set CAMERA frame rate (0 means automatic)
                sync version.

  Arguments:    camera      - one of CameraSelect
                effect      - one of CameraEffect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CEffect(CameraSelect camera, CameraEffect effect)
{
    cameraWork.result = CAMERA_I2CEffectAsync(camera, effect, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipAsync

  Description:  set CAMERA frame rate (0 means automatic)
                async version

  Arguments:    camera      - one of CameraSelect
                flip        - one of CameraFlip
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFlipAsync(CameraSelect camera, CameraFlip flip, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_FLIP;
    const u8                size    = CAMERA_PXI_SIZE_FLIP;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // コールバック設定
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // データ作成
    data[0] = (u8)camera;
    data[1] = (u8)flip;

    // コマンド送信
    if (CameraSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlip

  Description:  set CAMERA frame rate (0 means automatic)
                sync version.

  Arguments:    camera      - one of CameraSelect
                flip        - one of CameraFlip

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFlip(CameraSelect camera, CameraFlip flip)
{
    cameraWork.result = CAMERA_I2CFlipAsync(camera, flip, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}


/*---------------------------------------------------------------------------*
  Name:         CameraSendPxiCommand

  Description:  指定先頭コマンドをPXI経由でARM7に送信する。

  Arguments:    command     - 対象コマンド
                size        - 送信データサイズ (バイト単位)
                data        - 先頭データ (1バイトのみ)

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL CameraSendPxiCommand(CAMERAPxiCommand command, u8 size, u8 data)
{
    u32 pxiData = (u32)(CAMERA_PXI_START_BIT |
            ((command << CAMERA_PXI_COMMAND_SHIFT) & CAMERA_PXI_COMMAND_MASK) |
            ((size << CAMERA_PXI_DATA_NUMS_SHIFT) & CAMERA_PXI_DATA_NUMS_MASK) |
            ((data << CAMERA_PXI_1ST_DATA_SHIFT) & CAMERA_PXI_1ST_DATA_MASK));
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CameraSendPxiData

  Description:  指定後続データをPXI経由でARM7に送信する。

  Arguments:    pData   - 3バイトデータの先頭へのポインタ

  Returns:      None
 *---------------------------------------------------------------------------*/
static void CameraSendPxiData(u8 *pData)
{
    u32 pxiData = (u32)((pData[0] << 16) | (pData[1] << 8) | pData[2]);
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraPxiCallback

  Description:  非同期RTC関数用の共通コールバック関数。

  Arguments:    tag -  PXI tag which show message type.
                data - message from ARM7.
                err -  PXI transfer error flag.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    CAMERAResult   result;

    // PXI通信エラーを確認
    if (err)
    {
        // シーケンスを強制終了
        CameraCallCallbackAndUnlock(CAMERA_RESULT_FATAL_ERROR);
        return;
    }
    // 先頭データ
    if (data & CAMERA_PXI_START_BIT)
    {
        // 受信データを解析
        SDK_ASSERT((data & CAMERA_PXI_RESULT_BIT) == CAMERA_PXI_RESULT_BIT);
        cameraWork.total = (u8)((data & CAMERA_PXI_DATA_NUMS_MASK) >> CAMERA_PXI_DATA_NUMS_SHIFT);
        cameraWork.current = 0;
        cameraWork.command = (CAMERAPxiCommand)((data & CAMERA_PXI_COMMAND_MASK) >> CAMERA_PXI_COMMAND_SHIFT);
        cameraWork.pxiResult = (CAMERAPxiResult)((data & CAMERA_PXI_1ST_DATA_MASK) >> CAMERA_PXI_1ST_DATA_SHIFT);
    }
    // 後続データ
    else
    {
        if (cameraWork.data == NULL)
        {
            // シーケンスを強制終了
            CameraCallCallbackAndUnlock(CAMERA_RESULT_FATAL_ERROR);
            return;
        }
        if (cameraWork.current < cameraWork.size)
        {
            cameraWork.data[cameraWork.current++] = (u8)((data & 0xFF0000) >> 16);
        }
        if (cameraWork.current < cameraWork.size)
        {
            cameraWork.data[cameraWork.current++] = (u8)((data & 0x00FF00) >> 8);
        }
        if (cameraWork.current < cameraWork.size)
        {
            cameraWork.data[cameraWork.current++] = (u8)((data & 0x0000FF) >> 0);
        }
    }

    if (cameraWork.current >= cameraWork.total-1)   // > は無いはず
    {
        // 処理結果を確認
        switch (cameraWork.pxiResult)
        {
        case CAMERA_PXI_RESULT_SUCCESS:
            result = CAMERA_RESULT_SUCCESS;
            break;
        case CAMERA_PXI_RESULT_SUCCESS_TRUE:
            result = CAMERA_RESULT_SUCCESS_TRUE;
            break;
        case CAMERA_PXI_RESULT_SUCCESS_FALSE:
            result = CAMERA_RESULT_SUCCESS_FALSE;
            break;
        case CAMERA_PXI_RESULT_INVALID_COMMAND:
            result = CAMERA_RESULT_INVALID_COMMAND;
            break;
        case CAMERA_PXI_RESULT_INVALID_PARAMETER:
            result = CAMERA_RESULT_ILLEGAL_PARAMETER;
            break;
        case CAMERA_PXI_RESULT_ILLEGAL_STATUS:
            result = CAMERA_RESULT_ILLEGAL_STATUS;
            break;
        case CAMERA_PXI_RESULT_BUSY:
            result = CAMERA_RESULT_BUSY;
            break;
        default:
            result = CAMERA_RESULT_FATAL_ERROR;
        }

        // コールバックの呼び出し
        CameraCallCallbackAndUnlock(result);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraSyncCallback

  Description:  同期API用のコールバック

  Arguments:    result  - ARM7から送られた結果
                arg     - 未使用

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraSyncCallback(CAMERAResult result, void *arg)
{
#pragma unused(arg)
    cameraWork.result = result;
}

/*---------------------------------------------------------------------------*
  Name:         CameraCallCallbackAndUnlock

  Description:  コールバックの呼び出しとロックの解除を行う

  Arguments:    result  - ARM7から送られた結果

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraCallCallbackAndUnlock(CAMERAResult result)
{
    CAMERACallback cb;

    if (cameraWork.lock)
    {
        cameraWork.lock = FALSE;
    }
    if (cameraWork.callback)
    {
        cb = cameraWork.callback;
        cameraWork.callback = NULL;
        cb(result, cameraWork.callbackArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraWaitBusy

  Description:  CAMERAの非同期処理がロックされている間待つ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if 0
#include    <nitro/code32.h>
static asm void CameraWaitBusy(void)
{
    ldr     r12,    =cameraWork.lock
loop:
    ldr     r0,     [ r12,  #0 ]
    cmp     r0,     #TRUE
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>
#else
extern void PXIi_HandlerRecvFifoNotEmpty(void);
static void CameraWaitBusy(void)
{
    volatile BOOL *p = &cameraWork.lock;

    while (*p)
    {
        if (OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE)
        {
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}
#endif
