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
    定数定義
 *---------------------------------------------------------------------------*/
// 詰めてコピーする
#define MIC_PACK_U16(d, s)                   \
    ((d)[0] = (u8)((*((u16*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u16*)s) >>  8) & 0xFF))

#define MIC_PACK_U32(d, s)                   \
    ((d)[0] = (u8)((*((u32*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u32*)s) >>  8) & 0xFF),  \
     (d)[2] = (u8)((*((u32*)s) >> 16) & 0xFF),  \
     (d)[3] = (u8)((*((u32*)s) >> 24) & 0xFF))

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct _TWLMICWork
{
    BOOL            lock;
    TwlMicCallback  callback;					// 非同期関数用コールバック
    void*			callbackArg;				// 上記関数用引数
    TwlMicCallback  full_callback;				// 非同期関数用コールバック
    void*			full_arg;					// 上記関数用引数
    TWLMICResult    result;             		// 先頭データだけ別枠
    TWLMICPxiCommand   command;        			// コマンド種別
    TWLMICPxiResult    pxiResult;      			// 先頭データだけ別枠
	u16*               pOneBuffer;				// 単発サンプリングバッファ保存用
	void**             pLastSamplingAddress;	// 最新サンプリングデータ格納アドレス
	u8*                pAmpGain;				// アンプゲイン格納アドレス
    u8      current;                    		// 受信済みデータ個数 (バイト単位) (先頭を除く!!)
    u8      total;                      		// 最終データ個数 (1 + 後続コマンド*3)
    u8      data[TWL_MIC_PXI_DATA_SIZE_MAX];    // ARM7からのデータ保存用
}
TWLMICWork;

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static BOOL micInitialized;
static TWLMICWork micWork;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static BOOL TWLMICi_SendPxiCommand(TWLMICPxiCommand command, u8 size, u8 data);
static void TWLMICi_SendPxiData(u8 *pData);
static void TWLMICi_PxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void TWLMICi_CallbackAndUnlock(TWLMICResult result);
static void TWLMICi_GetResultCallback(TWLMICResult result, void *arg);
static void TWLMICi_WaitBusy(void);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_Init

  Description:  MICライブラリを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TWL_MIC_Init(void)
{
    // 初期化済みを確認
    if (micInitialized)
    {
        return;
    }
    micInitialized = 1;

    // 変数初期化
    micWork.lock = FALSE;
    micWork.callback = NULL;

    // PXI関連を初期化
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_TWL_MIC, PXI_PROC_ARM7))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_TWL_MIC, TWLMICi_PxiCallback);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_End

  Description:  MICライブラリを終了する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TWL_MIC_End(void)
{
    // 初期化済みを確認
    if (micInitialized == 0)
    {
        return;
    }
    micInitialized = 0;

    // PXI関連停止
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_TWL_MIC, NULL);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSamplingAsync

  Description:  マイク単発サンプリング開始（非同期版）

  Arguments:    

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_DoSamplingAsync(u16* buf,  TwlMicCallback callback, void* callbackArg)
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_ONE_SAMPLING;
    const u8                size = TWL_MIC_PXI_SIZE_ONE_SAMPLING;	// バイト
    OSIntrMode enabled;

	// buffer NULLチェック
	// ToDo: 適切なアドレスかどうかチェック
	if (buf == NULL)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// ロック
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // 非同期関数用コールバック設定
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

	// バッファアドレス保存
	micWork.pOneBuffer = buf;

    // コマンド送信
    if (TWLMICi_SendPxiCommand(command, size, 0) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSampling

  Description:  マイク単発サンプリング開始（同期版）

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

  Description:  マイク自動サンプリング開始（非同期版）

  Arguments:    mic      - one of MicSelect

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StartAutoSamplingAsync(TwlMicAutoParam* param, TwlMicCallback callback, void* callbackArg)
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_AUTO_START;
    const u8                size = TWL_MIC_PXI_SIZE_AUTO_START;	// バイト
    OSIntrMode enabled;
    u8  data[size+2];
    int i;

	// DMA-Noチェック
    if ( param->dmaNo < MI_EXDMA_CH_MIN || MI_EXDMA_CH_MAX < param->dmaNo )
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// buffer NULLチェック & 4バイトアライメントチェック
	// ToDo: 適切なアドレスかどうかチェック
	if (param->buffer == NULL || (u32)param->buffer & 0x03)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// size 4バイトの倍数チェック
	// ToDo: buffer + size が適切なメモリ範囲か
	if (param->size & 0x03)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// frequencyチェック
	if ( param->frequency > TWL_MIC_FREQUENCY_1_4 )
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// ロック
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // 非同期関数用コールバック設定
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

	// FULLコールバック設定
	if (param->full_callback)
	{
		micWork.full_callback = param->full_callback;
		micWork.full_arg = param->full_arg;
	}

    // データ作成
    data[0] = (u8)param->dmaNo;
    MIC_PACK_U32(&data[1], &param->buffer);
    MIC_PACK_U32(&data[5], &param->size);
    data[9]  = (u8)param->frequency;
    data[10] = (u8)param->loop_enable;

    // コマンド送信
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

  Description:  マイク自動サンプリング開始（同期版）

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

  Description:  マイク自動サンプリング停止（非同期版）

  Arguments:    none

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_StopAutoSamplingAsync( TwlMicCallback callback, void* callbackArg )
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_AUTO_STOP;
    const u8                size = TWL_MIC_PXI_SIZE_AUTO_STOP;	// バイト
    OSIntrMode enabled;

	// ロック
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // コールバック設定
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

    // コマンド送信
    if (TWLMICi_SendPxiCommand(command, size, 0) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSampling

  Description:  マイク自動サンプリング停止（同期版）

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

  Description:  最新のサンプリングデータの格納アドレスを返します。
				但し、アドレスはサンプリング時間を元に理論的に計算された
				ものであるため誤差を含んでいます。
				（非同期版）

  Arguments:    adress      : アドレス格納ポインタのアドレス
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetLastSamplingAddressAsync( void** adress, TwlMicCallback callback, void* callbackArg )
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS;
    const u8                size = TWL_MIC_PXI_SIZE_GET_LAST_SAMPLING_ADDRESS;	// バイト
    OSIntrMode enabled;

	// ロック
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // コールバック設定
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

	// アドレス格納アドレス保存
	micWork.pLastSamplingAddress = adress;

    // コマンド送信
    if (TWLMICi_SendPxiCommand(command, size, 0) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddress

  Description:  最新のサンプリングデータの格納アドレスを返します。
				但し、アドレスはサンプリング時間を元に理論的に計算された
				ものであるため誤差を含んでいます。
				（同期版）

  Arguments:    adress      : アドレス格納ポインタのアドレス

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

  Description:  プログラマブルゲインアンプの設定を行います。（非同期版）
				この関数で設定したゲインはオートゲインコントロールが
　　　　　　　　無効になっているときのみ有効となることに注意してください。

  Arguments:    gain 		: 設定ゲイン（0～119 = 0～59.5dB）
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_SetAmpGainAsync( u8 gain, TwlMicCallback callback, void* callbackArg )
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_SET_AMP_GAIN;
    const u8                size = TWL_MIC_PXI_SIZE_SET_AMP_GAIN;	// バイト
    u8  data[size+2];
    OSIntrMode enabled;

	// 設定ゲイン範囲チェック
	if ( gain > 119)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// ロック
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // コールバック設定
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

    // データ作成
    data[0] = gain;

    // コマンド送信
    if (TWLMICi_SendPxiCommand(command, size, data[0]) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGain

  Description:  プログラマブルゲインアンプの設定を行います。（同期版）
				この関数で設定したゲインはオートゲインコントロールが
				無効になっているときのみ有効となることに注意してください。

  Arguments:    gain 		: 設定ゲイン（0～119 = 0～59.5dB）

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

  Description:  TWLモードのマイクゲイン(PGAB)を取得します。
				（非同期版）

  Arguments:    gain        : マイクゲイン値の格納アドレス
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetAmpGainAsync( u8* gain, TwlMicCallback callback, void* callbackArg )
{
    const TWLMICPxiCommand  command = TWL_MIC_PXI_COMMAND_GET_AMP_GAIN;
    const u8                size = TWL_MIC_PXI_SIZE_GET_AMP_GAIN;	// バイト
    OSIntrMode enabled;

	// NULLチェック
	if ( gain == NULL)
	{
        return TWL_MIC_RESULT_ILLEGAL_PARAMETER;
	}

	// ロック
    enabled = OS_DisableInterrupts();
    if (micWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return TWL_MIC_RESULT_BUSY;
    }
    micWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);

    // コールバック設定
    micWork.callback = callback;
    micWork.callbackArg = callbackArg;

	// ゲイン格納アドレス保存
	micWork.pAmpGain = gain;

    // コマンド送信
    if (TWLMICi_SendPxiCommand(command, size, 0) == FALSE)
    {
        return TWL_MIC_RESULT_SEND_ERROR;
    }

    return TWL_MIC_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetAmpGain

  Description:  TWLモードのマイクゲイン(PGAB)を取得します。
				（同期版）

  Arguments:    gain        : マイクゲイン値の格納アドレス

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

  Description:  指定先頭コマンドをPXI経由でARM7に送信する。

  Arguments:    command     - 対象コマンド
                size        - 送信データサイズ (バイト単位)
                data        - 先頭データ (1バイトのみ)

  Returns:      BOOL     - PXIに対して送信が完了した場合TRUEを、
                           PXIによる送信に失敗した場合FALSEを返す。
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

  Description:  指定後続データをPXI経由でARM7に送信する。

  Arguments:    pData   - 3バイトデータの先頭へのポインタ

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

  Description:  非同期関数用の共通コールバック関数。

  Arguments:    tag -  PXI tag which show message type.
                data - message from ARM7.
                err -  PXI transfer error flag.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWLMICi_PxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    TWLMICResult   result;

    // PXI通信エラーを確認
    if (err)
    {
        // シーケンスを強制終了
        TWLMICi_CallbackAndUnlock(TWL_MIC_RESULT_FATAL_ERROR);
        return;
    }
    // 先頭データ
    if (data & TWL_MIC_PXI_START_BIT)
    {
        // 受信データを解析
        SDK_ASSERT((data & TWL_MIC_PXI_RESULT_BIT) == TWL_MIC_PXI_RESULT_BIT);
        micWork.total = (u8)((data & TWL_MIC_PXI_DATA_NUMS_MASK) >> TWL_MIC_PXI_DATA_NUMS_SHIFT);
        micWork.current = 0;
        micWork.command = (TWLMICPxiCommand)((data & TWL_MIC_PXI_COMMAND_MASK) >> TWL_MIC_PXI_COMMAND_SHIFT);
        micWork.pxiResult = (TWLMICPxiResult)((data & TWL_MIC_PXI_1ST_DATA_MASK) >> TWL_MIC_PXI_1ST_DATA_SHIFT);
    }
    // 後続データ
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

    if (micWork.current >= micWork.total-1)   // > は無いはず
    {
        // 処理結果を確認
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
				// FULLコールバックの呼び出し
				if (micWork.full_callback)
				{
					micWork.full_callback( result, micWork.full_arg );
				}
				break;

			case TWL_MIC_PXI_COMMAND_ONE_SAMPLING:
				// 単発サンプリング結果格納
				*micWork.pOneBuffer = (u16)(micWork.data[0] | (micWork.data[1] << 8));
		        // 非同期関数用コールバックの呼び出し＆ロック解除
		        TWLMICi_CallbackAndUnlock(result);
				break;

			case TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS:
				// 最新サンプリングデータアドレス格納
				*micWork.pLastSamplingAddress = (void *)(micWork.data[0] | (micWork.data[1] << 8) | (micWork.data[2] << 16) | (micWork.data[3] << 24));
		        // 非同期関数用コールバックの呼び出し＆ロック解除
		        TWLMICi_CallbackAndUnlock(result);
				break;

			case TWL_MIC_PXI_COMMAND_GET_AMP_GAIN:
				// アンプゲイン格納
				*micWork.pAmpGain = micWork.data[0];
		        // 非同期関数用コールバックの呼び出し＆ロック解除
		        TWLMICi_CallbackAndUnlock(result);
				break;

			default:
		        // 非同期関数用コールバックの呼び出し＆ロック解除
		        TWLMICi_CallbackAndUnlock(result);
		}
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWLMICi_CallbackAndUnlock

  Description:  コールバックの呼び出しとロックの解除を行う

  Arguments:    result  - ARM7から送られた結果

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWLMICi_CallbackAndUnlock(TWLMICResult result)
{
    TwlMicCallback cb;

	// ロック解除
    if (micWork.lock)
    {
        micWork.lock = FALSE;
    }

	// 非同期関数用コールバック呼び出し
    if (micWork.callback)
    {
        cb = micWork.callback;
        micWork.callback = NULL;
        cb(result, micWork.callbackArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWLMICi_GetResultCallback

  Description:  同期関数が内部で実行する非同期関数の結果を取得するために
				使用する。

  Arguments:    result - 非同期関数の処理結果。
                arg    - 使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWLMICi_GetResultCallback(TWLMICResult result, void *arg)
{
#pragma unused( arg )

    micWork.result = result;
}

/*---------------------------------------------------------------------------*
  Name:         TWLMICi_WaitBusy

  Description:  MICの非同期処理がロックされている間待つ。

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
