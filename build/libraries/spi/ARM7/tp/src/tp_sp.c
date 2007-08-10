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

#include    "tp_sp.h"
#include    <tp_reg.h>				    //////////////
#include    <twl/cdc/ARM7/cdc.h>	    //////////////
#include    <twl/cdc/ARM7/cdc_api.h>	//////////////


#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <nitro/os/common/printf.h>
#define DBG_PRINTF OS_TPrintf
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_CHAR( c )      ((void)0)
#endif


/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static TPWork tpw;

///////////////////////////  TWL
tpData_t    tpData;
///////////////////////////

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

// CODECの電源は現在SndInitでONになっていたはず。
// CDCInitでONにするようにするか？

// ここでCODECの状態がDSモードかTWLモードかによって
// 分岐させるという考えもある。
// スレッドを別にする。
// TPのスレッドはそういえばSPIスレッドとしてまとめられているんだった。
// TP_AnalyzeCommandwを別にするだけでいい？
// いやTP_ExecuteProcessでしょ。


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
/*
	    tpSetPrechargeTime(     TP_SETUP_TIME_0_1US );
	    tpSetSenseTime(         TP_SETUP_TIME_0_1US );
	    tpSetStabilizationTime( TP_SETUP_TIME_0_1US );				// 済み
	    
	    tpSetDebounceTime( TP_DEBOUNCE_16US );
	    
	    tpSetResolution( TP_RESOLUTION_12 );						// 済み
	    tpSetInterval( TP_INTERVAL_4MS );							// 済み

	    tpSetTouchPanelDataDepth( TP_DATA_SAMPLE_DEPTH_DEFAULT );	// 済み

	    tpSetConvertChannel( TP_CHANNEL_XY );						// 済み
	    
	    tpEnableNewBufferMode();									// 済み
*/

    	TWL_TP_DisableNewBufferMode();							// for TSC2101 mode
	    TWL_TP_SetResolution( TP_RESOLUTION_12 );
		TWL_TP_SetInterval(TP_INTERVAL_NONE);					// for TSC2101 mode	
	    TWL_TP_SetTouchPanelDataDepth( 5 /*TP_DATA_SAMPLE_DEPTH_DEFAULT*/ );
		TWL_TP_SetConvertChannel( TP_CHANNEL_XY );
		TWL_TP_SetStabilizationTime( TP_SETUP_TIME_100US );		// Yの座標が全部同じになることがあったため0.1us->1us
		TWL_TP_SetSenseTime(         TP_SETUP_TIME_3US );		// Yの座標が全部同じになることがあったため0.1us->1us
		TWL_TP_SetPrechargeTime(     TP_SETUP_TIME_3US );		// Yの座標が全部同じになることがあったため0.1us->1us

		// シングルショットモードに設定する
		//    CDC_ChangePage( 3 );
		//	CDC_SetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_CONVERSION_MODE_SINGLESHOT, TP_CONVERSION_MODE_MASK );

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

// 排他はCDC_ReadSpiRegister(s)内部でのSPI_Lock()->SPIi_GetExceptionで実現する
/*
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
*/
        // サンプリングを実行
        {
            SPITpData temp;

			if (CDC_IsTwlMode())
			{
//    			OSTick tick = OS_GetTick();	///////////////////////
				
				if (TWL_TP_ReadBuffer(&temp))
				{
		            // システム領域に書き出し( 2バイトアクセス )
		            *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
		            *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];
				}
	
//				DBG_PRINTF("TWL_TP_ReadBuffer = %6d us\n", OS_TicksToMicroSeconds(OS_GetTick() - tick));	/////////
			}
			else
			{
//    			OSTick tick = OS_GetTick();	/////////////////

#ifdef SDK_TP_AUTO_ADJUST_RANGE        // rangeの自動調整スイッチ
            	u16     density;
            	TP_ExecSampling(&temp, tpw.range, &density);
            	TP_AutoAdjustRange(&temp, density);
#else
            	TP_ExecSampling(&temp, tpw.range);
#endif
	            // システム領域に書き出し( 2バイトアクセス )
	            *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
	            *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];

//				DBG_PRINTF("TP_ExecSampling = %6d us\n", OS_TicksToMicroSeconds(OS_GetTick() - tick));	/////////
			}
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
        // 排他制御終了
//      SPIi_ReleaseException(SPI_DEVICE_TYPE_TP);
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
  Name:         TWL_TP_SetTouchPanelDataDepth

  Description:  set touch-panel data depth (1 <= depth <= 8)
  
  Arguments:    int depth : data depth (1<= depth <= 8)

  Returns:      None
 *---------------------------------------------------------------------------*/ //TODO: depthの切り替えはPage3, Reg14, D7 is "0"の状態で行うように修正する
void TWL_TP_SetTouchPanelDataDepth( u8 depth )
{
    u8 tmp;
    
    SDK_ASSERT( (1 <= depth) && (depth <= 8) );
    
    tmp = (u8)(depth << TP_DATA_DEPTH_SHIFT);
    if (depth == 8) tmp = 0;
    
	CDC_ChangePage( 3 );
    CDC_SetI2cParams( REG_TP_DATA_DEPTH, tmp, TP_DATA_DEPTH_MASK );

    tpData.tpDepth = depth;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetConvertChannel

  Description:  set ADC target channel
  
  Arguments:    TpChannel_t ch : Convert Channel

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetConvertChannel( TpChannel_t ch )
{
    SDK_ASSERT( (ch == TP_CHANNEL_NONE)   || (ch == TP_CHANNEL_XY)       ||
                (ch == TP_CHANNEL_XYZ)    || (ch == TP_CHANNEL_X)        ||
                (ch == TP_CHANNEL_Y)      || (ch == TP_CHANNEL_Z)        ||
                (ch == TP_CHANNEL_AUX3)   || (ch == TP_CHANNEL_AUX2)     ||
                (ch == TP_CHANNEL_AUX1)   || (ch == TP_CHANNEL_AUTO_AUX) ||
                (ch == TP_CHANNEL_AUX123) || (ch == TP_CHANNEL_XP_XM)    ||
                (ch == TP_CHANNEL_YP_YM)  || (ch == TP_CHANNEL_YP_XM)      );
    
	CDC_ChangePage( 3 );

//  i_tpWriteSpiRegister( REG_TP_CHANNEL, ch );
//	cdcWriteI2cRegister( REG_TP_CHANNEL, ch );
//	CDC_WriteI2cRegister( REG_TP_CHANNEL, ch );	// TODO: マスク書き換えに修正する

//	CDC_WriteI2cRegister( REG_TP_CHANNEL, (ch & 0x7f) );	// 強引にホストコントロールモード

	CDC_WriteI2cRegister( REG_TP_CHANNEL, (u8)(ch & 0xfd) );	// 2101 & self
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetInterval

  Description:  set Touch-Panel / AUX Interval Time
                Either Touch-Panel or AUX can be enabled, the last setting
                is only valid. Normally, Touch-Panel is enabled.
  
  Arguments:    tpInterval_t interval : interval time between sampling

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetInterval( tpInterval_t interval )
{
    SDK_ASSERT( (interval == TP_INTERVAL_NONE) || 
                (interval == TP_INTERVAL_8MS)  || (interval == TP_AUX_INTERVAL_1_12M) ||
                (interval == TP_INTERVAL_1MS)  || (interval == TP_AUX_INTERVAL_3_36M) ||
                (interval == TP_INTERVAL_2MS)  || (interval == TP_AUX_INTERVAL_5_59M) ||
                (interval == TP_INTERVAL_3MS)  || (interval == TP_AUX_INTERVAL_7_83M) ||
                (interval == TP_INTERVAL_4MS)  || (interval == TP_AUX_INTERVAL_10_01M) ||
                (interval == TP_INTERVAL_5MS)  || (interval == TP_AUX_INTERVAL_12_30M) ||
                (interval == TP_INTERVAL_6MS)  || (interval == TP_AUX_INTERVAL_14_54M) ||
                (interval == TP_INTERVAL_7MS)  || (interval == TP_AUX_INTERVAL_16_78M)
              );
    
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );

//  i_tpWriteSpiRegister( REG_TP_INTERVAL, interval );
//  cdcWriteI2cRegister( REG_TP_INTERVAL, interval );
	CDC_WriteI2cRegister( REG_TP_INTERVAL, interval );
}

/*---------------------------------------------------------------------------*
  Name:         tpEnableNewBufferMode

  Description:  enable new buffer mode
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_EnableNewBufferMode( void )
{
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );

//  i_tpSetSpiParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_E, TP_NEW_BUFFER_MODE_MASK );
//  i_tpSetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_E, TP_NEW_BUFFER_MODE_MASK );
    CDC_SetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_E, TP_NEW_BUFFER_MODE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         tpDisableNewBufferMode

  Description:  disable new buffer mode
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_DisableNewBufferMode( void )
{
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );

//  i_tpSetSpiParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_D, TP_NEW_BUFFER_MODE_MASK );
//  i_tpSetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_D, TP_NEW_BUFFER_MODE_MASK );
    CDC_SetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_D, TP_NEW_BUFFER_MODE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetResolution

  Description:  set AD Converting Resolution (8, 10, or 12-bit)
  
  Arguments:    TpResolution_t res : Converting Resolution

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetResolution( TpResolution_t res )
{
    SDK_ASSERT( (res == TP_RESOLUTION_12) || 
                (res == TP_RESOLUTION_8)  ||
                (res == TP_RESOLUTION_10) );
    
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );


//  i_tpSetSpiParams( REG_TP_RESOLUTION, res, TP_RESOLUTION_MASK );
//  i_tpSetI2cParams( REG_TP_RESOLUTION, res, TP_RESOLUTION_MASK );
    CDC_SetI2cParams( REG_TP_RESOLUTION, res, TP_RESOLUTION_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_GetResolution

  Description:  get AD Converting Resolution (8, 10, or 12-bit)
  
  Arguments:    TpResolution_t *res : Converting Resolution

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_GetResolution( TpResolution_t *res )
{
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );

//  *res = (TpResolution_t)(i_tpReadSpiRegister( REG_TP_RESOLUTION ) & TP_RESOLUTION_MASK);
//  *res = (TpResolution_t)( cdcReadI2cRegister( REG_TP_RESOLUTION ) & TP_RESOLUTION_MASK);
    *res = (TpResolution_t)( CDC_ReadI2cRegister( REG_TP_RESOLUTION ) & TP_RESOLUTION_MASK);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetStabilizationTime

  Description:  set ADC stabilization time before touch detection
  
  Arguments:    TpSetupTime_t time : stabilization time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetStabilizationTime( TpSetupTime_t time )
{
    SDK_ASSERT( (TP_SETUP_TIME_0_1US <= time) || (time <= TP_SETUP_TIME_1MS) );
    
	CDC_ChangePage( 3 );
	CDC_SetI2cParams( REG_TP_STABILIZATION_TIME, time, TP_STABILIZATION_TIME_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetPrechargeTime

  Description:  set ADC precharge time before touch detection
  
  Arguments:    TpSetupTime_t time : precharge time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetPrechargeTime( TpSetupTime_t time )
{
    SDK_ASSERT( (TP_SETUP_TIME_0_1US <= time) || (time <= TP_SETUP_TIME_1MS) );
    
    CDC_ChangePage( 3 );
    CDC_SetI2cParams( REG_TP_PRECHARGE, (u8)(time << TP_PRECHARGE_SHIFT), TP_PRECHARGE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetSenseTime

  Description:  set ADC sense time before touch detection
  
  Arguments:    TpSetupTime_t time : sense time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetSenseTime( TpSetupTime_t time )
{
    SDK_ASSERT( (TP_SETUP_TIME_0_1US <= time) || (time <= TP_SETUP_TIME_1MS) );
    
    CDC_ChangePage( 3 );
    CDC_SetI2cParams( REG_TP_SENSE_TIME, time, TP_SENSE_TIME_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_IS_TOUCH

  Description:  タッチパネル接触判定
 
  Arguments:    none

  Returns:      BOOL : if touched, return TRUE. otherwise FALSE.
 *---------------------------------------------------------------------------*/
static BOOL TWL_TP_IS_TOUCH( void )
{
	vu8 penup = 0;

	penup = CDC_ReadSpiRegister( 9 );
	if ((penup & 0x80) == 0x00)
	{
		penup = CDC_ReadSpiRegister( 9 );
		if ((penup & 0x80) == 0x00)
		{
			penup = CDC_ReadSpiRegister( 9 );
			if ((penup & 0x80) == 0x00)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


#define ABS(x) ( ( (x) >= 0 ) ? (x) : ( -(x) ) )
/*---------------------------------------------------------------------------*
  Name:         TWL_TP_ReadBuffer

  Description:  read Touch-Panel Buffer
  
  Arguments:    data : データ格納ポインタ

  Returns:      BOOL : if read success, return TRUE. otherwise FALSE.
 *---------------------------------------------------------------------------*/
BOOL TWL_TP_ReadBuffer( SPITpData *data )
{
    int i;
    int target_index = 0;   // 今からデータを格納する領域の先頭インデックス
    u8  buf[32];
	u8 not_readready;

//	(void)cdcLock();	// CODECデバイスの操作権利を取得

    CDC_ChangePage( 3 );


//  ペンアップ判定
	if (!TWL_TP_IS_TOUCH())
	{
        data->e.touch    = FALSE;
		data->e.validity = FALSE;
        return TRUE;	// ここはペンアップとしてシステム領域に書き出す
	}

/*
	//----- Availableチェック
	not_readready = (u8)(CDC_ReadI2cRegister( 9 ));
	if ((not_readready && 0x0c) == 0x0c)
	{
		(void)cdcUnlock();	// CODECデバイスの操作権利を解放 (忘れずに）
		return FALSE;
	}
*/
    
    if (tpData.tpIndex == 0)
        target_index = TP_DATA_SAMPLE_DEPTH_MAX;


	for (i=0;i<tpData.tpDepth;i++)
	{
//		OS_SpinWait(100);		// サンプリング時間が不足しているようなら追加も検討（ここは本来レディチェックするべき）

		// XYサンプリング
		CDC_ReadSpiRegisters( 42, &buf[i*4], 4 );

		//  サンプリング後のペンアップ判定 (サンプリング時間も稼げる）
		if (!TWL_TP_IS_TOUCH())
		{
		    return FALSE;	// システム領域には書き出さない
		}
	}


    for (i=0; i<tpData.tpDepth; i++)
    {
        tpData.xBuf[target_index+i]  = (u16)((buf[i*4]     << 8) | buf[i*4 + 1]);
        tpData.yBuf[target_index+i]  = (u16)((buf[i*4 + 2] << 8) | buf[i*4 + 3]);
    }
    
    tpData.tpIndex = target_index;

//	(void)cdcUnlock();	// CODECデバイスの操作権利を解放

	{
	    int i, j, index;
	    int xSum = 0;
		int ySum = 0;
		int same_required = (tpData.tpDepth >> 1) + 1;
		int same_chance   = tpData.tpDepth - same_required + 1;
		int same_count = 0;

	    index  = tpData.tpIndex;

/*
		// ペンアップbitチェック
	    for (i=0; i<tpData.tpDepth; i++, index++)
	    {
	        if ((tpData.xBuf[index] & TP_NOT_TOUCH_MASK) || (tpData.yBuf[index] & TP_NOT_TOUCH_MASK))
	        {
	            data->e.touch    = FALSE;
				data->e.validity = FALSE;
	            return TRUE;	// ここはペンアップとしてシステム領域に書き出す
	        }
//	        xSum += tpData.xBuf[index];
//	        ySum += tpData.yBuf[index];
	    }
*/

	    index  = tpData.tpIndex;

	    // サンプリングした内の半数以上がrange以内であればvalidなデータとする。
		for (i=0; i<same_chance; i++)
		{
			same_count = 0;
			xSum = tpData.xBuf[index + i];
			ySum = tpData.yBuf[index + i];

			for (j=0; j<tpData.tpDepth; j++)
			{
				if (i==j) { continue; }
				if ((ABS( tpData.xBuf[index + i] - tpData.xBuf[index + j] ) < 5) && 
					(ABS( tpData.yBuf[index + i] - tpData.yBuf[index + j] ) < 5))
				{
					same_count++;
					xSum += tpData.xBuf[index + j];
					ySum += tpData.yBuf[index + j];
				}
			}

			if (same_count >= same_required) { break; }
		}

		if (same_count < same_required)
		{
	            return FALSE;	// システム領域には書き出さない
		}

		data->e.x        = xSum / (same_count+1);
		data->e.y        = ySum / (same_count+1);
	    data->e.touch    = TRUE;
	    data->e.validity = TRUE;

//DBG_PRINTF("x : %4d %4d %4d %4d %4d %4d %4d %4d -> %4d\n",   tpData.xBuf[index], tpData.xBuf[index + 1], tpData.xBuf[index + 2], tpData.xBuf[index + 3], tpData.xBuf[index + 4], tpData.xBuf[index + 5], tpData.xBuf[index + 6], tpData.xBuf[index + 7], data->e.x);   
//DBG_PRINTF("y : %4d %4d %4d %4d %4d %4d %4d %4d -> %4d\n\n", tpData.yBuf[index], tpData.yBuf[index + 1], tpData.yBuf[index + 2], tpData.yBuf[index + 3], tpData.yBuf[index + 4], tpData.yBuf[index + 5], tpData.yBuf[index + 6], tpData.yBuf[index + 7], data->e.y);   

	}

    return TRUE;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
