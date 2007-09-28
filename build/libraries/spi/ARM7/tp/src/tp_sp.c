/*---------------------------------------------------------------------------*
  Project:  NitroSDK - libraries - spi
  File:     tp_sp.c

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: tp_sp.c,v $
  Revision 1.15  2006/01/18 02:12:29  kitase_hirotake
  do-indent

  Revision 1.14  2005/02/28 05:26:32  yosizaki
  do-indent.

  Revision 1.13  2004/12/29 02:04:13  takano_makoto
  SetStability関数のretryパラメータを廃止

  Revision 1.12  2004/12/20 00:41:05  takano_makoto
  自動サンプリング時のVAlarmのdelayを設定

  Revision 1.11  2004/12/15 12:02:51  takano_makoto
  コードの省サイズ化

  Revision 1.10  2004/12/15 09:12:19  takano_makoto
  rangeの自動調整を追加。tp_sp.hのSDK_TP_AUTO_ADJUST_RANGEによって有効になる。

  Revision 1.9  2004/10/21 04:02:57  terui
  LCDのライン数定義名を変更。

  Revision 1.8  2004/10/20 06:34:45  terui
  LCDのライン数定義名を変更

  Revision 1.7  2004/09/06 13:08:42  terui
  SPI処理予約方式の実装に伴う修正。

  Revision 1.6  2004/08/10 05:07:44  takano_makoto
  内部リファレンスでサンプリングの場合、TPi_ExecSamplingInSpaceTime()実行後に
  マイクデバイスの起動処理追加

  Revision 1.5  2004/08/09 13:19:32  takano_makoto
  MIC自動サンプリング時にTPの１回サンプリングが実行できるように修正

  Revision 1.4  2004/07/31 02:29:20  terui
  ICのreset処理を一部変更

  Revision 1.3  2004/07/29 13:09:13  takano_makoto
  TP_InitにICのリセット処理追加

  Revision 1.2  2004/06/03 11:12:10  terui
  SPIデバイス毎にスレッド優先度を調整する改造。

  Revision 1.1  2004/05/25 01:05:48  terui
  TPライブラリをSPIライブラリから分離

  Revision 1.2  2004/04/29 10:20:17  terui
  排他制御を共通関数で行うよう変更

  Revision 1.1  2004/04/14 06:27:50  terui
  SPIライブラリのソース整理に伴う更新

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*
	TP関連はCODECがDSモードとTWLモードのいずれであってもARM9から同じ関数が
　　使えるようにしてあります。というよりも、TWLモードだからといって特に良い
　　ことは増えてないといった方が正しいです。完全互換の方がよかったですね。

	現時点では CDC_IsTwlMode() の結果によって
	DS互換モードとTWLモードを分岐させています。
	ご自由に料理してください。
*/

#include    "tp_sp.h"
#include    <twl/cdc/ARM7/cdc_api.h>

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <nitro/os/common/printf.h>
#define DBG_PRINTF OS_TPrintf
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_modeAR( c )      ((void)0)
#endif

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static TPWork tpw;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void TpVAlarmHandler(void *arg);
static void SetStability(u16 range);

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TP_Init

  Description:  タッチパネルに関する内部管理変数を初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void TP_Init(void)
{
    s32     i;

    // 内部状態管理変数をクリア
    tpw.status = TP_STATUS_READY;
    tpw.range = SPI_TP_DEFAULT_STABILITY_RANGE;
    tpw.rangeMin = SPI_TP_DEFAULT_STABILITY_RANGE;

    // PXIコマンド退避用の配列をクリア
    for (i = 0; i < SPI_PXI_CONTINUOUS_PACKET_MAX; i++)
    {
        tpw.command[i] = 0x0000;
    }

    // Vカウントアラームを初期化
    if (!OS_IsVAlarmAvailable())
    {
        OS_InitVAlarm();
    }
    for (i = 0; i < SPI_TP_SAMPLING_FREQUENCY_MAX; i++)
    {
        OS_CreateVAlarm(&(tpw.vAlarm[i]));
        OS_SetVAlarmTag(&(tpw.vAlarm[i]), SPI_TP_VALARM_TAG);
    }

	if (CDC_IsTwlMode())
	{
    	TWL_TP_DisableNewBufferMode();							// NewBufferModeは使用せずTSC2101モードを使用する
	    TWL_TP_SetResolution( TP_RESOLUTION_12 );				// タッチパネルデータの精度は12bit
		TWL_TP_SetInterval(TP_INTERVAL_NONE);					// TSC2101モードでは無効	
		TWL_TP_SetConversionMode( 
				TP_CONVERSION_CONTROL_SELF, 
				TP_CONVERSION_MODE_XY,
				TP_CONVERSION_PIN_DATA_AVAILABLE	);			// 変換モードはXY座標
		TWL_TP_SetStabilizationTime( TP_SETUP_TIME_100US );		// 大きいと座標値の安定性が高まる代わりにサンプリング時間が増加
		TWL_TP_SetSenseTime(         TP_SETUP_TIME_0_1US );		// sense time 中は Pen Touch が Low にマスクされるので注意
		TWL_TP_SetPrechargeTime(     TP_SETUP_TIME_0_1US );		// precharge time 中は Pen Touch が Low にマスクされるので注意
		TWL_TP_SetDebounceTime( TP_DEBOUNCE_0US );				// 
	}
	else
	{
	    // TP制御ICの初期化
	    /*  8クロック送信後にすぐCSを上げても多分大丈夫とは思うが、
	       将来TPコントローラがコストダウン品に差替わることも考慮して
	       24サイクルでCSを上げる保証された通信方法をとっておく。 */
	    SPI_Wait();
	    TP_SPIChangeMode(SPI_TRANSMODE_CONTINUOUS);
	    SPI_SendWait(TP_COMMAND_DETECT_TOUCH);
	    SPI_DummyWait();
	    TP_SPIChangeMode(SPI_TRANSMODE_1BYTE);
	    SPI_DummyWait();
	}
}

/*---------------------------------------------------------------------------*
  Name:         TP_AnalyzeCommand

  Description:  タッチパネル関連PXIコマンドを解析し、処理の準備をする。
                ここではスレッドに処理要求を予約し、実際のSPI操作はスレッド内
                で行われる。

  Arguments:    data  - PXI経由で受け取ったARM9からの要求コマンド。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_AnalyzeCommand(u32 data)
{
    // 連続パケット開始をチェック
    if (data & SPI_PXI_START_BIT)
    {
        s32     i;

        // 連続パケット開始ならコマンド退避配列をクリア
        for (i = 0; i < SPI_PXI_CONTINUOUS_PACKET_MAX; i++)
        {
            tpw.command[i] = 0x0000;
        }
    }
    // 受信データをコマンド退避配列に退避
    tpw.command[(data & SPI_PXI_INDEX_MASK) >> SPI_PXI_INDEX_SHIFT] = (u16)((data &
                                                                             SPI_PXI_DATA_MASK) >>
                                                                            SPI_PXI_DATA_SHIFT);

    if (data & SPI_PXI_END_BIT)
    {
        u16     command;
        u16     wu16[2];

        // 受信データからコマンドを抽出
        command = (u16)((tpw.command[0] & 0xff00) >> 8);

        // コマンドを解析
        switch (command)
        {
            // サンプリング安定判定パラメータ変更
        case SPI_PXI_COMMAND_TP_SETUP_STABILITY:
            wu16[0] = (u16)(tpw.command[0] & 0x00FF);
            SetStability(wu16[0]);
            break;

            // 単体サンプリング
        case SPI_PXI_COMMAND_TP_SAMPLING:
            // TPサンプリング操作をスレッドに予約
            if (!SPIi_SetEntry(SPI_DEVICE_TYPE_TP, (u32)command, 0))
            {
                SPIi_ReturnResult(command, SPI_PXI_RESULT_EXCLUSIVE);
            }
            break;

            // 自動サンプリング開始
        case SPI_PXI_COMMAND_TP_AUTO_ON:
            // 内部状態をチェック
            if (tpw.status != TP_STATUS_READY)
            {
                // 既に自動サンプリング中の場合は不正な状態と見なす
                SPIi_ReturnResult(command, SPI_PXI_RESULT_ILLEGAL_STATUS);
                return;
            }
            // "frequency"パラメータをチェック
            wu16[0] = (u16)(tpw.command[0] & 0x00ff);
            if ((wu16[0] == 0) || (wu16[0] > SPI_TP_SAMPLING_FREQUENCY_MAX))
            {
                SPIi_ReturnResult(command, SPI_PXI_RESULT_INVALID_PARAMETER);
                return;
            }
            // "vCount"パラメータをチェック
            wu16[1] = tpw.command[1];
            if (wu16[1] >= HW_LCD_LINES)
            {
                SPIi_ReturnResult(command, SPI_PXI_RESULT_INVALID_PARAMETER);
                return;
            }
            // 自動サンプリング開始操作をスレッドに予約
            if (!SPIi_SetEntry(SPI_DEVICE_TYPE_TP, (u32)command, 2, (u32)wu16[0], (u32)wu16[1]))
            {                          // スレッドへの処理予約に失敗
                SPIi_ReturnResult(command, SPI_PXI_RESULT_EXCLUSIVE);
                return;
            }
            // 内部状態を更新
            tpw.status = TP_STATUS_AUTO_START;  // 状態を"自動サンプリング開始待ち"へ
            break;

            // 自動サンプリング停止
        case SPI_PXI_COMMAND_TP_AUTO_OFF:
            // 内部状態をチェック
            if (tpw.status != TP_STATUS_AUTO_SAMPLING)
            {
                SPIi_ReturnResult(command, SPI_PXI_RESULT_ILLEGAL_STATUS);
                return;
            }
            // 自動サンプリング停止操作をスレッドに予約
            if (!SPIi_SetEntry(SPI_DEVICE_TYPE_TP, (u32)command, 0))
            {                          // スレッドへの処理予約に失敗
                SPIi_ReturnResult(command, SPI_PXI_RESULT_EXCLUSIVE);
                return;
            }
            // 内部状態を更新
            tpw.status = TP_STATUS_AUTO_WAIT_END;       // 状態を"自動サンプリング停止待ち"へ
            break;

            // 不明なコマンド
        default:
            SPIi_ReturnResult(command, SPI_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

#ifdef SDK_TP_AUTO_ADJUST_RANGE        // rangeの自動調整スイッチ
/*---------------------------------------------------------------------------*
  Name:         TP_AutoAdjustRange

  Description:  タッチパネルのチャタリング対策パラメータを自動調整します。

  Arguments:    tpdata  サンプリングしたTPデータ
                density サンプリング時の座標密度

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TP_AutoAdjustRange(SPITpData *tpdata, u16 density)
{
    static u8 invalid_cnt = 0;
    static u8 valid_cnt = 0;
#define RANGE_MAX       35
#define RANGE_INC_CNT   4
#define RANGE_DEC_CNT   4
#define RANGE_DEC_CONDITION( d, r ) ( (d) < ( (r) >> 1 ) )

    if (!tpdata->e.touch)
    {
        // タッチされていない場合には、カウンタをリセット
        invalid_cnt = 0;
        valid_cnt = 0;
        return;
    }

    // タッチされている場合にはrangeの自動調整をします。

    if (tpdata->e.validity)
        // INVALIDの場合には invalid_cntをカウントアップします。
    {
        valid_cnt = 0;
        if (++invalid_cnt >= RANGE_INC_CNT)     // 一定回数連続してINVALIDを取得した場合にはrangeを調整
        {
            invalid_cnt = 0;
            if (tpw.range < RANGE_MAX)
            {
                tpw.range += 1;
            }
        }
    }
    else
    {
        // サンプリングした座標値が一定幅以上に収束していたらvalid_cntをカウントアップします。
        invalid_cnt = 0;
        if (!RANGE_DEC_CONDITION(density, tpw.range))
        {
            valid_cnt = 0;
            return;
        }

        if (++valid_cnt >= RANGE_DEC_CNT)
        {
            valid_cnt = 0;
            if (tpw.range > tpw.rangeMin)       // 一定回数連続して余分に収束していた場合にはrangeを調整
            {
                tpw.range -= 1;
                // rangeを減らしてみて駄目だった場合にはすぐに元の状態に戻れるようにしておく。
                invalid_cnt = RANGE_INC_CNT - 1;
            }
        }
    }
}
#endif

/*---------------------------------------------------------------------------*
  Name:         TP_ExecuteProcess

  Description:  タッチパネルに関する実際の処理を行う。
                この関数はSPIを一元管理するスレッドから呼び出される。

  Arguments:    entry   -   エントリー構造体へのポインタ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_ExecuteProcess(SPIEntry * entry)
{
    switch (entry->process)
    {
        // 自動サンプリング( VAlarmからのエントリー )
    case SPI_PXI_COMMAND_TP_AUTO_SAMPLING:
        if (tpw.status != TP_STATUS_AUTO_SAMPLING)
        {
            // 自動サンプリング中でない場合は何もせずに終了
            return;
        }
        // 単体サンプリング
    case SPI_PXI_COMMAND_TP_SAMPLING:

        // サンプリングを実行
        {
            SPITpData temp;
#ifdef SDK_TP_AUTO_ADJUST_RANGE        // rangeの自動調整スイッチ
            u16     density;
#endif
			// TWLモード
			if (CDC_IsTwlMode())
			{
#ifdef SDK_TP_AUTO_ADJUST_RANGE        // rangeの自動調整スイッチ
				if (TWL_TP_ReadBuffer(&temp, tpw.range, &density))
            	TP_AutoAdjustRange(&temp, density);
#else
				if (TWL_TP_ReadBuffer(&temp, tpw.range))
#endif
				{
		            // システム領域に書き出し( 2バイトアクセス )
		            *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
		            *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];
				}	
			}
			// DSモード
			else
			{
		        // 排他制御開始
		        {
		            OSIntrMode e;

		            e = OS_DisableInterrupts();
		            if (!SPIi_CheckException(SPI_DEVICE_TYPE_TP))
		            {
		                (void)OS_RestoreInterrupts(e);
		                // 外部スレッドからのSPI排他中
		                SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_EXCLUSIVE);
		                return;
		            }
		            SPIi_GetException(SPI_DEVICE_TYPE_TP);
		            (void)OS_RestoreInterrupts(e);
		        }

#ifdef SDK_TP_AUTO_ADJUST_RANGE        // rangeの自動調整スイッチ
            	TP_ExecSampling(&temp, tpw.range, &density);
            	TP_AutoAdjustRange(&temp, density);
#else
            	TP_ExecSampling(&temp, tpw.range);
#endif
	            // システム領域に書き出し( 2バイトアクセス )
	            *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
	            *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];
			}

	        // 排他制御終了
	        SPIi_ReleaseException(SPI_DEVICE_TYPE_TP);
        }
        // ARM9に処理の成功を通達
        if (entry->process == SPI_PXI_COMMAND_TP_SAMPLING)
        {
            // 単体サンプリングの応答
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_SUCCESS);
        }
        else
        {
            // 自動サンプリングのインジケート
            SPIi_ReturnResult((u16)(entry->process), (u16)(entry->arg[0] & 0x00ff));
        }
        break;

        // 自動サンプリング開始
    case SPI_PXI_COMMAND_TP_AUTO_ON:
        if (tpw.status == TP_STATUS_AUTO_START)
        {
            s32     i;

            // Vアラームを起動
            for (i = 0; i < entry->arg[0]; i++)
            {
                // 総ライン数をサンプリング頻度で分割し、各Vカウントを計算
                tpw.vCount[i] = (u16)((entry->arg[1] +
                                       ((i * HW_LCD_LINES) / entry->arg[0])) % HW_LCD_LINES);
                // Vカウントアラームを開始(割込みハンドラ外でないといけない)
                OS_SetPeriodicVAlarm(&(tpw.vAlarm[i]),
                                     (s16)(tpw.vCount[i]),
                                     TP_VALARM_DELAY_MAX, TpVAlarmHandler, (void *)i);
            }
            // ARM9に処理の成功を通達
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_SUCCESS);
            // 内部状態を更新
            tpw.status = TP_STATUS_AUTO_SAMPLING;       // 状態を"自動サンプリング中"へ
        }
        else
        {
            // ARM9に処理の失敗を通達
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_ILLEGAL_STATUS);
        }
        break;

        // 自動サンプリング停止
    case SPI_PXI_COMMAND_TP_AUTO_OFF:
        if (tpw.status == TP_STATUS_AUTO_WAIT_END)
        {
            // Vアラームを止める
            OS_CancelVAlarms(SPI_TP_VALARM_TAG);
            // ARM9に処理の成功を通達
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_SUCCESS);
            // 内部状態を更新
            tpw.status = TP_STATUS_READY;       // 状態を"通常操作待ち"へ
        }
        else
        {
            // ARM9に処理の失敗を通達
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_ILLEGAL_STATUS);
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         TpVAlarmHandler

  Description:  Vカウントアラームのハンドラ。
                一回サンプリングする度に止まるスレッドを再開させる。

  Arguments:    arg - 複数あるVカウントアラームのID

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TpVAlarmHandler(void *arg)
{
    // TPサンプリング操作をスレッドに予約
    if (!SPIi_SetEntry(SPI_DEVICE_TYPE_TP, SPI_PXI_COMMAND_TP_AUTO_SAMPLING, 1, (u32)arg))
    {                                  // スレッドへのサンプリング処理予約に失敗
        SPITpData temp;

        // サンプリングデータを偽装
        temp.e.validity = SPI_TP_VALIDITY_INVALID_XY;
        // システム領域に書き出し( 2バイトアクセス )
        *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
        *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];
        // 自動サンプリングのインジケート
        SPIi_ReturnResult(SPI_PXI_COMMAND_TP_AUTO_SAMPLING, (u16)((u32)arg & 0x00ff));
    }
}

/*---------------------------------------------------------------------------*
  Name:         SetStability

  Description:  タッチパネルの安定判定パラメータを変更。

  Arguments:    range 値の誤差の閾値.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetStability(u16 range)
{
    // rangeパラメータをチェック
    if (range == 0)
    {
        SPIi_ReturnResult(SPI_PXI_COMMAND_TP_SETUP_STABILITY, SPI_PXI_RESULT_INVALID_PARAMETER);
        return;
    }
    // 安定判定パラメータ(内部管理)を変更
    tpw.range = (s32)range;
    tpw.rangeMin = (s32)range;

    // ARM9に処理の成功を通達
    SPIi_ReturnResult(SPI_PXI_COMMAND_TP_SETUP_STABILITY, SPI_PXI_RESULT_SUCCESS);
    return;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
