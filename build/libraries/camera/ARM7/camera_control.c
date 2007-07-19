/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera_sp.c

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
#define CAMERA_PXI_SIZE_CHECK(nums)                                                     \
    if (cameraWork.total != (nums)) {                                                   \
        CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_INVALID_PARAMETER);    \
        break;                                                                          \
    }

// アライメント調整してコピーする
#define CAMERA_UNPACK_U16(d, s)    \
    (*(d) = (u16)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8)))
#define CAMERA_UNPACK_U32(d, s)    \
    (*(d) = (u32)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8) | (((u8*)s)[2] << 16) | (((u8*)s)[3] << 24)))

#define CAMERA_SET_GPIO(r)      ((r) |= CAMERA_STBYN_MASK)
#define CAMERA_CLEAR_GPIO(r)    ((r) &= ~CAMERA_STBYN_MASK)

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static BOOL cameraInitialized;          // 初期化確認フラグ
static CAMERAWork cameraWork;           // ワーク変数をまとめた構造体

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void CameraReturnResult(CAMERAPxiCommand command, CAMERAPxiResult result);
static void CameraReturnResultEx(CAMERAPxiCommand command, CAMERAPxiResult result, u8 size, u8* data);
static void CameraThread(void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Init

  Description:  CAMERAライブラリを初期化する。

  Arguments:    priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_Init(u32 priority)
{
    // 初期化済みを確認
    if (cameraInitialized)
    {
        return;
    }
    cameraInitialized = 1;

    // ワーク初期化
    cameraWork.camera = CAMERA_SELECT_NONE;

    // PXI関連を初期化
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CAMERA, CameraPxiCallback);

    // 実処理を行うスレッドを作成
    OS_InitMessageQueue(&cameraWork.msgQ, cameraWork.msgArray, CAMERA_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&cameraWork.thread,
                    CameraThread,
                    0,
                    (void *)(cameraWork.stack + (CAMERA_THREAD_STACK_SIZE / sizeof(u64))),
                    CAMERA_THREAD_STACK_SIZE, priority);
    OS_WakeupThreadDirect(&cameraWork.thread);
}

/*---------------------------------------------------------------------------*
  Name:         CameraPxiCallback

  Description:  PXI経由で受信したデータを解析する。

  Arguments:    tag -  PXI種別を示すタグ。
                data - 受信したデータ。下位26bitが有効。
                err -  PXI通信におけるエラーフラグ。
                       ARM9側にて同種別のPXIが初期化されていないことを示す。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    // PXI通信エラーをチェック
    if (err)
    {
        return;
    }
    // 先頭データ
    if (data & CAMERA_PXI_START_BIT)
    {
        cameraWork.total = (u8)((data & CAMERA_PXI_DATA_NUMS_MASK) >> CAMERA_PXI_DATA_NUMS_SHIFT);
        cameraWork.current = 0;
        cameraWork.command = (CAMERAPxiCommand)((data & CAMERA_PXI_COMMAND_MASK) >> CAMERA_PXI_COMMAND_SHIFT);
        cameraWork.data[cameraWork.current++] = (u8)((data & CAMERA_PXI_1ST_DATA_MASK) >> CAMERA_PXI_1ST_DATA_SHIFT);
//OS_TPrintf("START_BIT (total=%d, command=%X).\n", cameraWork.total, cameraWork.command);
    }
    // 後続データ
    else
    {
        cameraWork.data[cameraWork.current++] = (u8)((data & 0xFF0000) >> 16);
        cameraWork.data[cameraWork.current++] = (u8)((data & 0x00FF00) >> 8);
        cameraWork.data[cameraWork.current++] = (u8)((data & 0x0000FF) >> 0);
    }

    // パケット完成
    if (cameraWork.current >= cameraWork.total)   // 最大で2つ余分に取得する
    {
        // 受信したコマンドを解析
        switch (cameraWork.command)
        {
            // 既知のコマンド群
        case CAMERA_PXI_COMMAND_INIT:
        case CAMERA_PXI_COMMAND_ACTIVATE:
        case CAMERA_PXI_COMMAND_RESIZE:
        case CAMERA_PXI_COMMAND_FRAME_RATE:
        case CAMERA_PXI_COMMAND_EFFECT:
        case CAMERA_PXI_COMMAND_FLIP:
            // スレッドを再開
            if (!OS_SendMessage(&cameraWork.msgQ, NULL, OS_MESSAGE_NOBLOCK))
            {
                CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_FATAL_ERROR);
            }
            break;

            // 未知のコマンド
        default:
            CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraReturnResult

  Description:  PXI経由で処理結果をARM9に送信する。

  Arguments:    command     - 対象コマンド
                result      - CAMERAPxiResultのひとつ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraReturnResult(CAMERAPxiCommand command, CAMERAPxiResult result)
{
    u32 pxiData = (u32)(CAMERA_PXI_START_BIT | CAMERA_PXI_RESULT_BIT |
            ((command << CAMERA_PXI_COMMAND_SHIFT) & CAMERA_PXI_COMMAND_MASK) |
            ((1 << CAMERA_PXI_DATA_NUMS_SHIFT) & CAMERA_PXI_DATA_NUMS_MASK) |
            ((result << CAMERA_PXI_1ST_DATA_SHIFT) & CAMERA_PXI_1ST_DATA_MASK));
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraReturnResultEx

  Description:  指定後続データをPXI経由でARM7に送信する。

  Arguments:    command     - 対象コマンド
                result      - CAMERAPxiResultのひとつ
                size        - 付加データサイズ
                data        - 付加データ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraReturnResultEx(CAMERAPxiCommand command, CAMERAPxiResult result, u8 size, u8* data)
{
    u32 pxiData = (u32)(CAMERA_PXI_START_BIT | CAMERA_PXI_RESULT_BIT |
            ((command << CAMERA_PXI_COMMAND_SHIFT) & CAMERA_PXI_COMMAND_MASK) |
            (((size+1) << CAMERA_PXI_DATA_NUMS_SHIFT) & CAMERA_PXI_DATA_NUMS_MASK) |
            ((result << CAMERA_PXI_1ST_DATA_SHIFT) & CAMERA_PXI_1ST_DATA_MASK));
    int i;
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
    }
    for (i = 0; i < size; i += 3)
    {
        pxiData = (u32)((data[i] << 16) | (data[i+1] << 8) | data[i+2]);
        while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
        {
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraThread

  Description:  CAMERA操作の実処理を行うスレッド。

  Arguments:    arg - 使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraThread(void *arg)
{
#pragma unused( arg )

    OSMessage msg;
    BOOL result;
    u16 data16a;
    u16 data16b;

    while (TRUE)
    {
        // メッセージが発行されるまで寝る
        (void)OS_ReceiveMessage(&(cameraWork.msgQ), &msg, OS_MESSAGE_BLOCK);

        // コマンドに従って各種処理を実行
        switch (cameraWork.command)
        {
        case CAMERA_PXI_COMMAND_INIT:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_INIT);
            result = CAMERA_I2CInit((CameraSelect)cameraWork.data[0]);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9に処理の成功を通達
            break;

        case CAMERA_PXI_COMMAND_ACTIVATE:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_ACTIVATE);
            if (cameraWork.camera != cameraWork.data[0])
            {
                if (cameraWork.camera != CAMERA_SELECT_NONE)
                {
                    if (FALSE == CAMERA_I2CStandby(cameraWork.camera, TRUE))
                    {
                        CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_SUCCESS_FALSE);    // ARM9に処理の失敗を通達
                    }
                }
                cameraWork.camera = (CameraSelect)cameraWork.data[0];
                if (cameraWork.camera != CAMERA_SELECT_NONE)
                {
                    if (FALSE == CAMERA_I2CStandby(cameraWork.camera, FALSE))
                    {
                        CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_SUCCESS_FALSE);    // ARM9に処理の失敗を通達
                    }
                }
            }
            CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_SUCCESS_TRUE);     // ARM9に処理の成功を通達
            break;

        case CAMERA_PXI_COMMAND_RESIZE:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_RESIZE);
            CAMERA_UNPACK_U16(&data16a, &cameraWork.data[1]);
            CAMERA_UNPACK_U16(&data16b, &cameraWork.data[3]);
            result = CAMERA_I2CResize((CameraSelect)cameraWork.data[0], data16a, data16b);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9に処理の成功を通達
            break;

        case CAMERA_PXI_COMMAND_FRAME_RATE:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_FRAME_RATE);
            result = CAMERA_I2CFrameRate((CameraSelect)cameraWork.data[0], cameraWork.data[1]);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9に処理の成功を通達
            break;

        case CAMERA_PXI_COMMAND_EFFECT:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_EFFECT);
            result = CAMERA_I2CEffect((CameraSelect)cameraWork.data[0], (CameraEffect)cameraWork.data[1]);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9に処理の成功を通達
            break;

        case CAMERA_PXI_COMMAND_FLIP:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_FLIP);
            result = CAMERA_I2CFlip((CameraSelect)cameraWork.data[0], (CameraFlip)cameraWork.data[1]);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9に処理の成功を通達
            break;

            // サポートしないコマンド
        default:
            CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_INVALID_COMMAND);
        }
    }
}
