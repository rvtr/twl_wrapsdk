/*---------------------------------------------------------------------------*
  Project:  TwlSDK - mic - include
  File:     fifo.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MIC_FIFO_H_
#define TWL_MIC_FIFO_H_

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// プロトコル関連定義
#define TWL_MIC_PXI_CONTINUOUS_PACKET_MAX   			10  // 連続パケットの最大連続回数
#define TWL_MIC_PXI_DATA_SIZE_MAX           ((TWL_MIC_PXI_CONTINUOUS_PACKET_MAX-1)*3+1) // 最大データ数

#define TWL_MIC_PXI_START_BIT                   0x02000000  // 先頭パケットを意味する
#define TWL_MIC_PXI_RESULT_BIT                  0x00008000  // PXIの応答を示す

/* 先頭パケットのみの規則 */
#define TWL_MIC_PXI_DATA_NUMS_MASK              0x00ff0000  // データ数領域
#define TWL_MIC_PXI_DATA_NUMS_SHIFT             16          // データ数位置
#define TWL_MIC_PXI_COMMAND_MASK                0x00007f00  // コマンド格納部分のマスク
#define TWL_MIC_PXI_COMMAND_SHIFT               8           // コマンド格納部分の位置
#define TWL_MIC_PXI_1ST_DATA_MASK               0x000000ff  // 先頭パケットのデータ領域
#define TWL_MIC_PXI_1ST_DATA_SHIFT              0           // 先頭パケットのデータ位置

/* 後続パケットのみの規則 */
#define TWL_MIC_PXI_DATA_MASK                   0x00ffffff  // データ領域
#define TWL_MIC_PXI_DATA_SHIFT                  0           // データ位置

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// PXIコマンド定義
typedef enum _TWLMICPxiCommand
{
	TWL_MIC_PXI_COMMAND_ONE_SAMPLING			  = 0x34, // 単発サンプリング
    TWL_MIC_PXI_COMMAND_AUTO_START                = 0x35, // 自動サンプリングスタート
    TWL_MIC_PXI_COMMAND_AUTO_STOP                 = 0x36, // 自動サンプリングストップ
	TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS = 0x37, // 最新サンプリングデータのアドレス取得
	TWL_MIC_PXI_COMMAND_SET_AMP_GAIN 			  = 0x38, // PGAB設定
	TWL_MIC_PXI_COMMAND_GET_AMP_GAIN              = 0x39, // PGAB取得
	TWL_MIC_PXI_COMMAND_MIC_BUFFER_FULL			  = 0x50, // バッファフルの通知
    TWL_MIC_PXI_COMMAND_UNKNOWN
}
TWLMICPxiCommand;

// PXIコマンド付加データサイズ定義
typedef enum TWLMICPxiSize
{
	TWL_MIC_PXI_SIZE_ONE_SAMPLING				 = 0,    // 0
    TWL_MIC_PXI_SIZE_AUTO_START                  = 11,   // dmaNo(u8) buffer(u32) size(u32) frequency(u8) loop_erable(u8)
    TWL_MIC_PXI_SIZE_AUTO_STOP                   = 0,    // 0
    TWL_MIC_PXI_SIZE_GET_LAST_SAMPLING_ADDRESS   = 0,	 // 0
    TWL_MIC_PXI_SIZE_SET_AMP_GAIN   			 = 1,	 // gain(u8)
    TWL_MIC_PXI_SIZE_GET_AMP_GAIN   			 = 0,	 // 0
    TWL_MIC_PXI_SIZE_UNKNOWN
}
TWLMICPxiSize;

// 応答定義
typedef enum TWLMICPxiResult
{
    TWL_MIC_PXI_RESULT_SUCCESS = 0,        // 処理成功 (void/void*型) // 場合により後続パケットあり
    TWL_MIC_PXI_RESULT_INVALID_COMMAND,    // 不正なPXIコマンド
    TWL_MIC_PXI_RESULT_INVALID_PARAMETER,  // 不正なパラメータ
    TWL_MIC_PXI_RESULT_ILLEGAL_STATUS,     // 状態により処理を実行不能
    TWL_MIC_PXI_RESULT_BUSY,               // 他のリクエストを実行中
    TWL_MIC_PXI_RESULT_FATAL_ERROR,        // その他何らかの原因で処理に失敗
    TWL_MIC_PXI_RESULT_MAX
}
TWLMICPxiResult;

/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_MIC_FIFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
