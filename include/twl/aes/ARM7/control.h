/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - aes
  File:     aes_sp.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_AES_SP_H_
#define TWL_AES_SP_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     AES_MESSAGE_ARRAY_MAX       4       // スレッド同期用メッセージキューのサイズ
#define     AES_THREAD_STACK_SIZE       256     // スレッドのスタックサイズ

typedef enum AESLock
{
    AES_UNLOCKED = 0,
    AES_LOCKED_BY_ARM7,
    AES_LOCKED_BY_ARM9
} AESLock;

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
// AESライブラリ用ワーク構造体

typedef struct AESWork
{
    OSMessageQueue msgQ;               // スレッド同期用メッセージキュー
    OSMessage msgArray[AES_MESSAGE_ARRAY_MAX];
    // メッセージを格納するバッファ
    OSThread thread;                   // AES用スレッド
    u64     stack[AES_THREAD_STACK_SIZE / sizeof(u64)];
    // AES用スレッドのスタック

    AESLock locked;                     // ロック

    u8      command;                    // コマンド種別
    u8      current;                    // 受信済みデータ個数 (バイト単位)
    u8      total;                      // 最終データ個数 (1 + 後続コマンド*3)
    u8      data[AES_PXI_DATA_SIZE_MAX];
    // ARM9からのデータ保存用
}
AESWork;

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
void    AES_Init(u32 priority);         // 初期化、及びスレッドを開始
void    AES_Lock(void);                 // ARM7側で使うためにロックする
BOOL    AES_TryLock(void);              // ARM7側で使うためにロックを試みる
void    AES_Unlock(void);               // ARM7側のロックを解除する

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_AES_SP_H_ */
