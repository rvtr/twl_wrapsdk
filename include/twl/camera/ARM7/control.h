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

#ifndef TWL_CAMERA_CONTROL_H_
#define TWL_CAMERA_CONTROL_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define     CAMERA_MESSAGE_ARRAY_MAX       4       // スレッド同期用メッセージキューのサイズ
#define     CAMERA_THREAD_STACK_SIZE       256     // スレッドのスタックサイズ

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
// CAMERAライブラリ用ワーク構造体

typedef struct CAMERAWork
{
    OSMessageQueue msgQ;               // スレッド同期用メッセージキュー
    OSMessage msgArray[CAMERA_MESSAGE_ARRAY_MAX];
    // メッセージを格納するバッファ
    OSThread thread;                   // CAMERA用スレッド
    u64     stack[CAMERA_THREAD_STACK_SIZE / sizeof(u64)];
    // CAMERA用スレッドのスタック

    CAMERAPxiCommand    command;        // コマンド種別
    u8      current;                    // 受信済みデータ個数 (バイト単位)
    u8      total;                      // 最終データ個数 (1 + 後続コマンド*3)
    u8      data[CAMERA_PXI_DATA_SIZE_MAX];
    // ARM9からのデータ保存用
}
CAMERAWork;

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
void    CAMERA_Init(u32 priority);         // 初期化、及びスレッドを開始
void    CAMERA_Lock(void);                 // ARM7側で使うためにロックする
BOOL    CAMERA_TryLock(void);              // ARM7側で使うためにロックを試みる
void    CAMERA_Unlock(void);               // ARM7側のロックを解除する

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CAMERA_CONTROL_H_ */
