/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - twlmic
  File:     twl_mic_server.c

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
  TWL Mic用に勝手にARM7側スレッドを作りました。
  都合よければSPIスレッドなどに吸収してください。
 ----------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/mic.h>

/*---------------------------------------------------------------------------*
    マクロ定義
 *---------------------------------------------------------------------------*/

// アライメント調整してコピーする
#define MIC_UNPACK_U16(d, s)    \
    (*(d) = (u16)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8)))
#define MIC_UNPACK_U32(d, s)    \
    (*(d) = (u32)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8) | (((u8*)s)[2] << 16) | (((u8*)s)[3] << 24)))

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/

static BOOL micInitialized;          	// 初期化確認フラグ
static TWLMICServerWork micWork;      	// ワーク変数をまとめた構造体

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

static void TWL_MICi_PxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void TWL_MICi_ReturnResult(TWLMICPxiCommand command, TWLMICPxiResult result);
static void TWL_MICi_ReturnResultEx(TWLMICPxiCommand command, TWLMICPxiResult result, u8 size, u8* data);
static void TWL_MICi_Thread(void *arg);
static void TWL_MICi_FullCallback( u8 loop );
static BOOL TWL_MICi_SetEntry(TWLMICPxiCommand command, u16 args, ...);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_InitServer

  Description:  ARM7側とやりとりを行うための準備を行います。

  Arguments:    priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
TWL_MIC_InitServer(u32 priority)
{
    // 初期化済みを確認
    if (micInitialized)
    {
        return;
    }
    micInitialized = 1;

    // 内部状態管理変数をクリア
    micWork.status = TWL_MIC_STATUS_READY;

    // PXI関連を初期化
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_TWL_MIC, TWL_MICi_PxiCallback);

    // 実処理を行うスレッドを作成＆起動
    OS_InitMessageQueue(&micWork.msgQ, micWork.msgArray, TWL_MIC_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&micWork.thread,
                    TWL_MICi_Thread,
                    0,
                    (void *)(micWork.stack + (TWL_MIC_THREAD_STACK_SIZE / sizeof(u64))),
                    TWL_MIC_THREAD_STACK_SIZE, priority);
    OS_WakeupThreadDirect(&micWork.thread);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_SetEntry

  Description:  TWL MIC スレッドに操作要求を予約する。

  Arguments:    command -   行うべき操作のIDを指定。
                args    -   操作に伴うパラメータの数を指定。
                ...     -   各パラメータをu32型で指定。

  Returns:      BOOL    -   予約に成功した場合にTRUEを、失敗した場合にFALSEを返す。
 *---------------------------------------------------------------------------*/
static BOOL 
TWL_MICi_SetEntry(TWLMICPxiCommand command, u16 args, ...)
{
    OSIntrMode e;
    void   *w;
    va_list vlist;
    s32     i;

    // 引数の数をチェック
    if (args > TWL_MIC_MESSAGE_ARGS_MAX)
    {
        return FALSE;
    }

    e = OS_DisableInterrupts();
    micWork.entry[micWork.entryIndex].command = command;

    // 指定数個の引数を追加
    va_start(vlist, args);
    for (i = 0; i < args; i++)
    {
        micWork.entry[micWork.entryIndex].arg[i] = va_arg(vlist, u32);
    }
    va_end(vlist);

    w = &(micWork.entry[micWork.entryIndex]);
    micWork.entryIndex = (u32)((micWork.entryIndex + 1) % TWL_MIC_MESSAGE_ARRAY_MAX);
    (void)OS_RestoreInterrupts(e);
    return OS_SendMessage(&(micWork.msgQ), w, OS_MESSAGE_NOBLOCK);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_PxiCallback

  Description:  PXI経由で受信したデータを解析する。

  Arguments:    tag -  PXI種別を示すタグ。
                data - 受信したデータ。下位26bitが有効。
                err -  PXI通信におけるエラーフラグ。
                       ARM9側にて同種別のPXIが初期化されていないことを示す。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWL_MICi_PxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    // PXI通信エラーをチェック
    if (err)
    {
        return;
    }
    // 先頭データ
    if (data & TWL_MIC_PXI_START_BIT)
    {
        micWork.total = (u8)((data & TWL_MIC_PXI_DATA_NUMS_MASK) >> TWL_MIC_PXI_DATA_NUMS_SHIFT);
        micWork.current = 0;
        micWork.command = (TWLMICPxiCommand)((data & TWL_MIC_PXI_COMMAND_MASK) >> TWL_MIC_PXI_COMMAND_SHIFT);
        micWork.data[micWork.current++] = (u8)((data & TWL_MIC_PXI_1ST_DATA_MASK) >> TWL_MIC_PXI_1ST_DATA_SHIFT);
    }
    // 後続データ
    else
    {
        micWork.data[micWork.current++] = (u8)((data & 0x00FF0000) >> 16);
        micWork.data[micWork.current++] = (u8)((data & 0x0000FF00) >> 8);
        micWork.data[micWork.current++] = (u8)((data & 0x000000FF) >> 0);
    }

    // パケット完成
    if (micWork.current >= micWork.total)   // 最大で2バイト余分に取得する
    {
        // 受信したコマンドを解析
        switch (micWork.command)
        {
			// 単発サンプリング
			case TWL_MIC_PXI_COMMAND_ONE_SAMPLING:
	            if (micWork.status != TWL_MIC_STATUS_READY)
	            {
					// コマンドを実行できる状態ではない
	                TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
	            }
				else
				{
					// TWL MIC スレッドに操作要求
					if (TWL_MICi_SetEntry( micWork.command, 0 ))
					{
						// 状態を"単発サンプリング開始待ち"へ
						micWork.status = TWL_MIC_STATUS_ONE_SAMPLING_START;  
					}
					else
					{
						// エントリー失敗（メッセージキューに空き無し）
    	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
					}
				}
				break;

            // オートサンプリング開始
			case TWL_MIC_PXI_COMMAND_AUTO_START:
	            if (micWork.status != TWL_MIC_STATUS_READY)
	            {
					// コマンドを実行できる状態ではない
	                TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
	            }
				else
				{
					u32 dmaNo, buffer, size, frequency, loop;
					
					dmaNo = micWork.data[0];
		            MIC_UNPACK_U32(&buffer, &micWork.data[1]);
					MIC_UNPACK_U32(&size, &micWork.data[5]);
					frequency = (u32)(micWork.data[9] << REG_SND_MICCNT_FIFO_SMP_SHIFT);
					loop  = micWork.data[10];

					// TWL MIC スレッドに操作要求
					if (TWL_MICi_SetEntry( micWork.command, 5, dmaNo, buffer, size, frequency, loop ))
					{
						// 状態を"自動サンプリング開始待ち"へ
						micWork.status = TWL_MIC_STATUS_AUTO_START;  
					}
					else
					{
						// エントリー失敗（メッセージキューに空き無し）
    	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
					}
				}
				break;

			// オートサンプリング停止
			case TWL_MIC_PXI_COMMAND_AUTO_STOP:
	            if (micWork.status != TWL_MIC_STATUS_AUTO_SAMPLING)
	            {
					// コマンドを実行できる状態ではない
	                TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
	            }
				else
				{
					// TWL MIC スレッドに操作要求
					if (TWL_MICi_SetEntry( micWork.command, 0 ))
					{
						// 状態を"自動サンプリング停止待ち"へ
						micWork.status = TWL_MIC_STATUS_AUTO_END;  	
					}
					else
					{
						// エントリー失敗（メッセージキューに空き無し）
    	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
					}
				}
            	break;

			// 最新サンプリングデータアドレス取得
			case TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS:

				// TWL MIC スレッドに操作要求
				if (!TWL_MICi_SetEntry( micWork.command, 0 ))
				{
					// エントリー失敗（メッセージキューに空き無し）
	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
				}
            	break;

            // プログラマブルアンプゲイン設定
			case TWL_MIC_PXI_COMMAND_SET_AMP_GAIN:
				{
					u32 gain = micWork.data[0];

					// TWL MIC スレッドに操作要求
					if (!TWL_MICi_SetEntry( micWork.command, 1, gain ))
					{
						// エントリー失敗（メッセージキューに空き無し）
    	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
					}
				}
				break;

			// プログラマブルアンプゲイン取得
			case TWL_MIC_PXI_COMMAND_GET_AMP_GAIN:

				// TWL MIC スレッドに操作要求
				if (!TWL_MICi_SetEntry( micWork.command, 0 ))
				{
					// エントリー失敗（メッセージキューに空き無し）
	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
				}
            	break;

            // 未知のコマンド
        	default:
            	TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_ReturnResult

  Description:  PXI経由で処理結果をARM9に送信する。

  Arguments:    command     - 対象コマンド
                result      - TWLMICPxiResultのひとつ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void 
TWL_MICi_ReturnResult(TWLMICPxiCommand command, TWLMICPxiResult result)
{
    u32 pxiData = (u32)(TWL_MIC_PXI_START_BIT | TWL_MIC_PXI_RESULT_BIT |
            ((command << TWL_MIC_PXI_COMMAND_SHIFT) & TWL_MIC_PXI_COMMAND_MASK) |
            ((1 << TWL_MIC_PXI_DATA_NUMS_SHIFT) & TWL_MIC_PXI_DATA_NUMS_MASK) |
            ((result << TWL_MIC_PXI_1ST_DATA_SHIFT) & TWL_MIC_PXI_1ST_DATA_MASK));
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TWL_MIC, pxiData, 0))
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_ReturnResultEx

  Description:  指定後続データをPXI経由でARM9に送信する。

  Arguments:    command     - 対象コマンド
                result      - TWLMICPxiResultのひとつ
                size        - 付加データサイズ
                data        - 付加データ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void 
TWL_MICi_ReturnResultEx(TWLMICPxiCommand command, TWLMICPxiResult result, u8 size, u8* data)
{
    u32 pxiData = (u32)(TWL_MIC_PXI_START_BIT | TWL_MIC_PXI_RESULT_BIT |
            ((command << TWL_MIC_PXI_COMMAND_SHIFT) & TWL_MIC_PXI_COMMAND_MASK) |
            (((size+1) << TWL_MIC_PXI_DATA_NUMS_SHIFT) & TWL_MIC_PXI_DATA_NUMS_MASK) |
            ((result << TWL_MIC_PXI_1ST_DATA_SHIFT) & TWL_MIC_PXI_1ST_DATA_MASK));
    int i;
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TWL_MIC, pxiData, 0))
    {
    }
    for (i = 0; i < size; i += 3)
    {
        pxiData = (u32)((data[i] << 16) | (data[i+1] << 8) | data[i+2]);
        while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TWL_MIC, pxiData, 0))
        {
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_Thread

  Description:  MIC操作の実処理を行うスレッド。

  Arguments:    arg - 使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWL_MICi_Thread(void *arg)
{
#pragma unused( arg )

    OSMessage msg;
    TWLMICMessageData* entry;

    while (TRUE)
    {
        // メッセージが発行されるまで寝る
        (void)OS_ReceiveMessage(&(micWork.msgQ), &msg, OS_MESSAGE_BLOCK);
        entry = (TWLMICMessageData *) msg;

        // コマンドに従って各種処理を実行
        switch (entry->command)
        {
		//--- 単発サンプリング開始
        case TWL_MIC_PXI_COMMAND_ONE_SAMPLING:
			if (micWork.status == TWL_MIC_STATUS_ONE_SAMPLING_START)
			{
				u16 temp;
				TWL_MIC_DoSampling( &temp );

				TWL_MICi_ReturnResultEx(entry->command, TWL_MIC_PXI_RESULT_SUCCESS, 
									2, (u8*)&temp );	// ARM9に処理の成功を通達

				micWork.status = TWL_MIC_STATUS_READY;     				// 状態を"通常操作待ち"へ
			}
			else
			{
	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
			}
			break;

		//--- オートサンプリング開始
        case TWL_MIC_PXI_COMMAND_AUTO_START:
			if (micWork.status == TWL_MIC_STATUS_AUTO_START)
			{
				TWLMICParam param;

				param.dmaNo     = (u8)entry->arg[0];
	            param.buffer    = (void *)entry->arg[1];
				param.size      = entry->arg[2];
				param.frequency = (MICSampleRate)entry->arg[3];
				param.loop      = (u8)entry->arg[4];
				param.callback = TWL_MICi_FullCallback;
				TWL_MIC_StartAutoSampling( param );

	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_SUCCESS); 	// ARM9に処理の成功を通達
				micWork.status = TWL_MIC_STATUS_AUTO_SAMPLING;     				// 状態を"自動サンプリング中"へ
			}
			else
			{
	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
			}
			break;

		//--- オートサンプリング停止
        case TWL_MIC_PXI_COMMAND_AUTO_STOP:
	        if ((micWork.status == TWL_MIC_STATUS_AUTO_END) || (micWork.status == TWL_MIC_STATUS_END_WAIT))
	        {
				TWL_MIC_StopAutoSampling();

	            // ARM9に処理の成功を通達
	            if (micWork.status == TWL_MIC_STATUS_AUTO_END)
	            {
		            TWL_MICi_ReturnResult(TWL_MIC_PXI_COMMAND_AUTO_STOP, TWL_MIC_PXI_RESULT_SUCCESS);
	            }
	            // 内部状態を更新
	            micWork.status = TWL_MIC_STATUS_READY;     // 状態を"通常操作待ち"へ
			}
	        else
	        {
	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
	        }
			break;

		//--- 最新サンプリングデータアドレス取得
        case TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS:
			{
				void* adress = TWL_MIC_GetLastSamplingAddress();
	
				TWL_MICi_ReturnResultEx(entry->command, TWL_MIC_PXI_RESULT_SUCCESS, 
									4, (u8*)&adress );	// ARM9に処理の成功を通達
			}
			break;

		//--- プログラマブルアンプゲイン設定
        case TWL_MIC_PXI_COMMAND_SET_AMP_GAIN:
			{
				u8 gain = (u8)entry->arg[0];
				TWL_MIC_SetAmpGain( gain );
	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_SUCCESS); 	// ARM9に処理の成功を通達
			}
			break;
		
		//--- プログラマブルアンプゲイン取得
		case TWL_MIC_PXI_COMMAND_GET_AMP_GAIN:
		{
			u8 gain = TWL_MIC_GetAmpGain();
			TWL_MICi_ReturnResultEx(entry->command, TWL_MIC_PXI_RESULT_SUCCESS, 
									1, (u8*)&gain );	// ARM9に処理の成功を通達
		}
        //--- サポートしないコマンド
        default:
            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_FullCallback

  Description:  

  Arguments:    

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWL_MICi_FullCallback( u8 loop )
{
	// FULL通知
	TWL_MICi_ReturnResult(TWL_MIC_PXI_COMMAND_MIC_BUFFER_FULL, TWL_MIC_PXI_RESULT_SUCCESS);

	if ( !loop )
	{
		// 自動サンプリング停止
		if (TWL_MICi_SetEntry( TWL_MIC_PXI_COMMAND_AUTO_STOP, 0 ))
		{
			micWork.status = TWL_MIC_STATUS_END_WAIT;	// 状態を"自動サンプリング完了待ち"へ
		}
		else
		{
			TWL_MICi_ReturnResult(TWL_MIC_PXI_COMMAND_AUTO_STOP, TWL_MIC_PXI_RESULT_FATAL_ERROR);
		}
	}
}


