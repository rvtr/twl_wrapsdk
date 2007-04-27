/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - include
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
#ifndef TWL_AES_FIFO_H_
#define TWL_AES_FIFO_H_

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// プロトコル関連定義
#define AES_PXI_CONTINUOUS_PACKET_MAX       20          // 連続パケットの最大連続回数
#define AES_PXI_DATA_SIZE_MAX               ((AES_PXI_CONTINUOUS_PACKET_MAX-1)*3+1) // 最大データ数

#define AES_PXI_START_BIT                   0x02000000  // 先頭パケットを意味する
#define AES_PXI_RESULT_BIT                  0x00008000  // PXIの応答を示す

/* 先頭パケットのみの規則 */
#define AES_PXI_DATA_NUMS_MASK              0x00ff0000  // データ数領域
#define AES_PXI_DATA_NUMS_SHIFT             16          // データ数位置
#define AES_PXI_COMMAND_MASK                0x00007f00  // コマンド格納部分のマスク
#define AES_PXI_COMMAND_SHIFT               8           // コマンド格納部分の位置
#define AES_PXI_1ST_DATA_MASK               0x000000ff  // 先頭パケットのデータ領域
#define AES_PXI_1ST_DATA_SHIFT              0           // 先頭パケットのデータ位置

/* 後続パケットのみの規則 */
#define AES_PXI_DATA_MASK                   0x00ffffff  // データ領域
#define AES_PXI_DATA_SHIFT                  0           // データ位置

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// PXIコマンド定義
typedef enum AESPxiCommand
{
    AES_PXI_COMMAND_RESET                           = 0x00, // リセット
    AES_PXI_COMMAND_IS_BUSY                         = 0x01,
    AES_PXI_COMMAND_WAIT                            = 0x02,
    AES_PXI_COMMAND_INPUT_FIFO_IS_FULL              = 0x03,
    AES_PXI_COMMAND_OUTPUT_FIFO_IS_EMPTY            = 0x04,
    AES_PXI_COMMAND_WAIT_INPUT_FIFO_NOT_FULL        = 0x05,
    AES_PXI_COMMAND_WAIT_OUTPUT_FIFO_NOT_EMPTY      = 0x06,
    AES_PXI_COMMAND_IS_VALID                        = 0x07,
    AES_PXI_COMMAND_SELECT_KEY                      = 0x10,
    AES_PXI_COMMAND_SET_KEY                         = 0x11,
    AES_PXI_COMMAND_SET_ID                          = 0x12,
    AES_PXI_COMMAND_SET_SEED                        = 0x13,
    AES_PXI_COMMAND_SET_KEY2                        = 0x14,
    AES_PXI_COMMAND_START_CCM_DEC                   = 0x20,
    AES_PXI_COMMAND_START_CCM_DEC_NOMAC             = 0x21,
    AES_PXI_COMMAND_START_CCM_ENC                   = 0x22,
    AES_PXI_COMMAND_START_CTR                       = 0x23,
    AES_PXI_COMMAND_START_DMA_SEND                  = 0x30,
    AES_PXI_COMMAND_START_DMA_RECV                  = 0x31,
    AES_PXI_COMMAND_CPU_SEND                        = 0x32,
    AES_PXI_COMMAND_CPU_RECV                        = 0x33,

    AES_PXI_COMMAND_TRY_LOCK                        = 0x40,
    AES_PXI_COMMAND_UNLOCK                          = 0x41
}
AESPxiCommand;

// 引数のサイズ定義
typedef enum AESPxiSize
{
    AES_PXI_SIZE_RESET                          = 0,
    AES_PXI_SIZE_IS_BUSY                        = 0,
    AES_PXI_SIZE_WAIT                           = 0,
    AES_PXI_SIZE_INPUT_FIFO_IS_FULL             = 0,
    AES_PXI_SIZE_OUTPUT_FIFO_IS_EMPTY           = 0,
    AES_PXI_SIZE_WAIT_INPUT_FIFO_NOT_FULL       = 0,
    AES_PXI_SIZE_WAIT_OUTPUT_FIFO_NOT_EMPTY     = 0,
    AES_PXI_SIZE_IS_VALID                       = 0,
    AES_PXI_SIZE_SELECT_KEY                     = 1,    // keyNo
    AES_PXI_SIZE_SET_KEY                        = 17,   // keyNo, pKey(16)
    AES_PXI_SIZE_SET_ID                         = 17,   // keyNo, pId(16)
    AES_PXI_SIZE_SET_SEED                       = 17,   // keyNo, pSeed(16)
    AES_PXI_SIZE_SET_KEY2                       = 33,   // keyNo, pId(16), pSeed(16)
    AES_PXI_SIZE_START_CCM_DEC                  = 37,   // nonce(12), mac(16), alen(4), plen(4), isA
    AES_PXI_SIZE_START_CCM_DEC_NOMAC            = 21,   // nonce(12), alen(4), plen(4), isA
    AES_PXI_SIZE_START_CCM_ENC                  = 21,   // nonce(12), alen(4), plen(4), isA
    AES_PXI_SIZE_START_CTR                      = 20,   // iv(16), len(4)
    AES_PXI_SIZE_START_DMA_SEND                 = 9,    // no, src(4), size(4)
    AES_PXI_SIZE_START_DMA_RECV                 = 9,    // no, dest(4), size(4)
    AES_PXI_SIZE_CPU_SEND                       = 8,    // src(4), size(4)
    AES_PXI_SIZE_CPU_RECV                       = 8,    // dest(4), size(4)

    AES_PXI_SIZE_TRY_LOCK                       = 0,
    AES_PXI_SIZE_UNLOCK                         = 0
}
AESPxiSize;

// 応答定義
typedef enum AESPxiResult
{
    AES_PXI_RESULT_SUCCESS = 0,        // 処理成功 (void型)
    AES_PXI_RESULT_SUCCESS_TRUE,       // 処理成功 (BOOL型)
    AES_PXI_RESULT_SUCCESS_FALSE,      // 処理成功 (BOOL型)
    AES_PXI_RESULT_INVALID_COMMAND,    // 不正なPXIコマンド
    AES_PXI_RESULT_INVALID_PARAMETER,  // 不正なパラメータ
    AES_PXI_RESULT_ILLEGAL_STATUS,     // RTCの状態により処理を実行不能
    AES_PXI_RESULT_BUSY,               // 他のリクエストを実行中
    AES_PXI_RESULT_FATAL_ERROR,        // その他何らかの原因で処理に失敗
    AES_PXI_RESULT_MAX
}
AESPxiResult;


/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_AES_FIFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
