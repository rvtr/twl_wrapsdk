/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     control.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_MIC_SERVER_H_
#define TWL_MIC_SERVER_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     TWL_MIC_MESSAGE_ARRAY_MAX       4       // スレッド同期用メッセージキューのサイズ
#define     TWL_MIC_THREAD_STACK_SIZE       512     // スレッドのスタックサイズ
#define     TWL_MIC_MESSAGE_ARGS_MAX    	7		// メッセージ引数の最大値

// マイクに関する内部的な状態
typedef enum _TWLMICStatus
{
    TWL_MIC_STATUS_READY = 0,              // 通常操作待ち状態
    TWL_MIC_STATUS_ONE_SAMPLING_START,     // 単発サンプリング開始待ち
    TWL_MIC_STATUS_AUTO_START,             // 自動サンプリング開始待ち
    TWL_MIC_STATUS_AUTO_SAMPLING,          // 自動サンプリング中
    TWL_MIC_STATUS_AUTO_END,               // 自動サンプリング終了待ち
    TWL_MIC_STATUS_END_WAIT                // 自動サンプリング完了待ち(バッファFULL時の自動終了)
}
TWLMICStatus;

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
 
// TWLマイク用メッセージデータ構造体
typedef struct _TWLMICMessageData
{
    TWLMICPxiCommand     command;
    u32     arg[TWL_MIC_MESSAGE_ARGS_MAX];
}
TWLMICMessageData;
 
// MICライブラリ用ワーク構造体

typedef struct _TWLMICServerWork
{
    OSMessageQueue msgQ;               							// スレッド同期用メッセージキュー
    OSMessage msgArray[TWL_MIC_MESSAGE_ARRAY_MAX];				// メッセージを格納するバッファ
    TWLMICMessageData entry[TWL_MIC_MESSAGE_ARRAY_MAX];			// メッセージの実体
    u32     entryIndex;											// エントリーインデックス
    OSThread thread;                   							// TWL-MIC用スレッド
    u64     stack[TWL_MIC_THREAD_STACK_SIZE / sizeof(u64)];		// TWL-MIC用スレッドのスタック（小さくできる）
    TWLMICStatus status;                  						// マイク内部状態管理変数
    TWLMICPxiCommand    command;        						// ARM9からのコマンド保存用
    u8      data[TWL_MIC_PXI_DATA_SIZE_MAX];					// ARM9からのデータ保存用
    u8      current;                    						// 受信済みデータ個数 (バイト単位)
    u8      total;                      						// 最終データ個数 (1 + 後続コマンド*3)
}
TWLMICServerWork;

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_InitServer

  Description:  ARM7側とやりとりを行うための準備を行います。

  Arguments:    priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TWL_MIC_InitServer( u32 priority );	// 初期化、及びスレッドを開始

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_MIC_SERVER_H_ */
