/*---------------------------------------------------------------------------*
  Project:  TwlSDK - mcu - include
  File:     intr.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-02-15#$
  $Rev: 4187 $
  $Author: yutaka $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MCU_INTR_H_
#define TWL_MCU_INTR_H_

#include <twl/types.h>
#include <twl/mcu/ARM7/mcu_reg.h>

/*
    CPU割り込み
*/
#define MCU_IE  OS_IE_GPIO33_2

/*
    MCU割り込みビット数
*/
#define MCU_IRQ_TABLE_MAX   8

/*
    MCU割り込み種別
*/
// 外部DCプラグの挿抜が発生した (アプリケーションユーティリティ)
//#define MCU_IE_EXTERNAL_DC_TRIGGER      MCU_REG_IRQ_EXTDC_MASK
// バッテリ切れ最終予告水域になった (アプリケーションユーティリティ)
#define MCU_IE_BATTERY_LOW_TRIGGER      MCU_REG_IRQ_BATTLOW_MASK
// バッテリ切れ水域になった (電源OFFと同様の処理開始)
#define MCU_IE_BATTERY_EMPTY_TRIGGER    MCU_REG_IRQ_BATTEMP_MASK
// 電源ボタンが不感時間を超えて押された (リセット/電源OFFのどちらかの発生が確定)
#define MCU_IE_POWER_SWITCH_PRESSED     MCU_REG_IRQ_PWSW_MASK
// 電源ボタンによる電源OFF指示
#define MCU_IE_POWER_OFF_REQUEST        MCU_REG_IRQ_PWOFF_MASK
// 電源ボタンによるリセット指示
#define MCU_IE_RESET_REQUEST            MCU_REG_IRQ_RESET_MASK

/*
    ブロック設定
*/
#define MCU_BLOCK   OS_MESSAGE_BLOCK
#define MCU_NOBLOCK OS_MESSAGE_NOBLOCK

#ifdef _cplusplus
extern "C" {
#endif

/*
    割り込みハンドラ
*/
typedef void (*MCUIrqFunction)(void);

/*
    電源ボタンの状態
*/
typedef enum MCUPwswStatus
{
    MCU_PWSW_UNKNOWN,       // 押されていない
    MCU_PWSW_IN_PROGRESS,   // 押しているが未確定
    MCU_PWSW_RESET,         // リセットと確定した
    MCU_PWSW_POWER_OFF,     // 電源OFFと確定した

    MCU_PWSW_MAX
}
MCUPwswStatus;

/*---------------------------------------------------------------------------*
  Name:         MCU_InitIrq

  Description:  MCUの割り込みを利用可能とする

  Arguments:    priority        割り込み処理スレッドの優先度(初回のみ有効)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MCU_InitIrq(u32 priority);

/*---------------------------------------------------------------------------*
  Name:         MCU_SetIrqFunction

  Description:  MCU割り込みハンドラの登録

  Arguments:    intrBit         設定するMCU割り込み要因
                function        割り込みハンドラ

  Returns:      None
 *---------------------------------------------------------------------------*/
void MCU_SetIrqFunction(u8 intrBit, MCUIrqFunction function);

/*---------------------------------------------------------------------------*
  Name:         MCU_GetIrqFunction

  Description:  MCU割り込みハンドラの取得

  Arguments:    intrBit         設定するMCU割り込み要因

  Returns:      設定済みの割り込みハンドラ
 *---------------------------------------------------------------------------*/
MCUIrqFunction MCU_GetIrqFunction(u8 intrBit);

/*---------------------------------------------------------------------------*
  Name:         MCU_CallIrqFunction

  Description:  引数の割り込みに対応したコールバックを呼び出します。
                PwswStatusは更新しません。

  Arguments:    intrBit         割り込みハンドラを呼び出したい
                                MCU割り込み要因のビットOR

  Returns:      None
 *---------------------------------------------------------------------------*/
void MCU_CallIrqFunction(u8 intrBit);

/*---------------------------------------------------------------------------*
  Name:         MCU_CheckIrq

  Description:  MCUの割り込みが発生していないか確認する。
                MCU_REG_IRQ_ADDRを直接Readする代わりにこれを使用してください。
                遅延して割り込みハンドラを呼び出したい場合は、その
                タイミングでMCU_CallIrqFunction()を呼び出してください。

  Arguments:    callHandler     登録済みハンドラを呼び出すかどうか

  Returns:      発生していた割り込みに対応するMCU_IE_*のビットOR
 *---------------------------------------------------------------------------*/
u8 MCU_CheckIrq(BOOL callHandler);

/*---------------------------------------------------------------------------*
  Name:         MCU_GetPwswStatus

  Description:  MCUの電源ボタンの状態を取得する
                ブロックする場合、IN_PROGRESSであれば、RESETまたはPOWER_OFFに
                なるまでOS_Sleep(1)する

  Arguments:    block           確定していないなら確定するまで待つかどうか
                                MCU_BLOCK: 待つ
                                MCU_NOBLOCK: 待たない

  Returns:      one of MCUPwswStatus
 *---------------------------------------------------------------------------*/
MCUPwswStatus MCU_GetPwswStatus(s32 block);

#if 0
/*---------------------------------------------------------------------------*
  Name:         MCU_ResetPwswStatus

  Description:  MCUの電源ボタンの状態をクリアする (なかったことにする)
                MCU_PWSW_IN_PROGRESSの場合は無視される

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MCU_ResetPwswStatus(void);
#endif

#ifdef _cplusplus
} /* extern "C" */
#endif

#endif  // TWL_MCU_INTR_H_
