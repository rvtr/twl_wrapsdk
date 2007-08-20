/*---------------------------------------------------------------------------*
  Project:  NitroSDK - libraries - spi
  File:     tp_sp.h

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: tp_sp.h,v $
  Revision 1.7  2006/01/18 02:12:27  kitase_hirotake
  do-indent

  Revision 1.6  2005/02/28 05:26:27  yosizaki
  do-indent.

  Revision 1.5  2004/12/29 02:03:59  takano_makoto
  SetStability関数のretryパラメータを廃止, rangeの自動調整スイッチを有効に変更

  Revision 1.4  2004/12/20 00:40:55  takano_makoto
  自動サンプリング時のVAlarmのdelayを設定

  Revision 1.3  2004/12/15 09:12:40  takano_makoto
  rangeの自動調整を追加。SDK_TP_AUTO_ADJUST_RANGEによって有効になる。

  Revision 1.2  2004/11/05 05:47:24  terui
  SPIコマンドについての制約事項をコメントに追加。

  Revision 1.1  2004/09/06 12:54:18  terui
  libraries/spi/includeからlibraries/spi/ARM7/includeに移動。
  SPI処理予約方式の実装に伴う修正。

  Revision 1.9  2004/08/09 13:23:25  takano_makoto
  MIC自動サンプリングの合間にTPの１回サンプリングが実行できるように修正

  Revision 1.8  2004/07/31 08:06:45  terui
  サンプリングコマンドをターゲットHWにより振り分ける仕様を追加

  Revision 1.7  2004/06/17 11:07:43  terui
  接触判定コマンド定義を変更。0x94 -> 0x84

  Revision 1.6  2004/05/25 00:58:01  terui
  SPI各デバイス用ライブラリ細分化に伴う修正

  Revision 1.5  2004/05/14 04:50:36  yosiokat
  TEGとTSの両方に対応するよう変更

  Revision 1.4  2004/04/29 10:25:59  terui
  関数定義の引数削除に伴う変更

  Revision 1.3  2004/04/14 06:26:46  terui
  SPIライブラリのソース整理に伴う更新

  Revision 1.2  2004/04/05 04:47:05  terui
  Change composition.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_TP_SP_H_
#define LIBRARIES_TP_SP_H_

#include    <nitro/types.h>
#include    "spi_sp.h"
/////////////////////////////// TWL
#include    "tp_reg.h"
///////////////////////////////


#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
/* 2004/11/04 照井
 *
 * 新規別デバイスのために、以下のコマンド以外は予約とします。
 * ARM7がTPコントローラにこれら以外のコマンドを発行する必要がある場合は
 * 予約されていることを考慮して注意して使用して下さい。
 *
 * 0x0000   ダミー命令
 * 0x0084   接触判定
 * 0x0091   Y方向サンプリング
 * 0x0093   Y方向サンプリング(旧バージョン)
 * 0x00D1   X方向サンプリング
 * 0x00D3   X方向サンプリング(旧バージョン)
 *
 */
#ifdef  SDK_TEG
    // 内部リファレンスを使用
#define     TP_COMMAND_SAMPLING_X       0x00D3  // X方向サンプリング命令
#define     TP_COMMAND_SAMPLING_Y       0x0093  // Y方向サンプリング命令
#else  // SDK_TS
#if ( SDK_TS_VERSION >= 100 )          // 青箱Ver.D以降
        // 外部リファレンスを使用
#define     TP_COMMAND_SAMPLING_X       0x00D1  // X方向サンプリング命令
#define     TP_COMMAND_SAMPLING_Y       0x0091  // Y方向サンプリング命令
#else  // 青箱Ver.D未満
        // 内部リファレンスを使用
#define     TP_COMMAND_SAMPLING_X       0x00D3  // X方向サンプリング命令
#define     TP_COMMAND_SAMPLING_Y       0x0093  // Y方向サンプリング命令
#endif
#endif

#define     TP_COMMAND_DETECT_TOUCH     0x0084  // 接触判定命令

#define     TP_VALID_BIT_MASK           0x7ff8  // 有効データビット
#define     TP_VALID_BIT_SHIFT          3       // xooooooo oooooxxx

#define     TP_VALARM_DELAY_MAX         10

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
// タッチパネルに関する内部的な状態
typedef enum TPStatus
{
    TP_STATUS_READY = 0,               // 通常操作待ち状態
    TP_STATUS_AUTO_START,              // 自動サンプリング開始待ち状態
    TP_STATUS_AUTO_SAMPLING,           // 自動サンプリング中
    TP_STATUS_AUTO_WAIT_END            // 自動サンプリング停止待ち状態
}
TPStatus;

// タッチパネル用ワーク構造体
typedef struct TPWork
{
    u16     command[SPI_PXI_CONTINUOUS_PACKET_MAX];
    TPStatus status;                   // タッチパネル内部状態管理変数
    s32     range;                     // 安定判定時の許容する振れ幅のデフォルト値
    s32     rangeMin;                  // 安定判定時の許容する振れ幅の最小値(range自動調整で使用)
    OSVAlarm vAlarm[SPI_TP_SAMPLING_FREQUENCY_MAX];     // タッチパネル用Vアラーム
    u16     vCount[SPI_TP_SAMPLING_FREQUENCY_MAX];      // タッチパネル用Vカウント退避エリア

}
TPWork;


/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
void    TP_Init(void);
void    TP_AnalyzeCommand(u32 data);
void    TP_ExecuteProcess(SPIEntry * entry);

//////////////////////////// TWL
void    TWL_TP_Init(void);
//void    TWL_TP_AnalyzeCommand(u32 data);
//void    TWL_TP_ExecuteProcess(SPIEntry * entry);

void    TWL_TP_SetStabilizationTime( TpSetupTime_t time );
void    TWL_TP_SetPrechargeTime( TpSetupTime_t time );
void    TWL_TP_SetSenseTime( TpSetupTime_t time );
void    TWL_TP_SetResolution( TpResolution_t res );
void    TWL_TP_GetResolution( TpResolution_t *res );
void    TWL_TP_SetTouchPanelDataDepth(u8 depth);
void    TWL_TP_SetConvertChannel( TpChannel_t ch );
void    TWL_TP_SetInterval( tpInterval_t interval );
void    TWL_TP_EnableNewBufferMode( void );
void    TWL_TP_DisableNewBufferMode( void );
BOOL    TWL_TP_ReadBuffer( SPITpData *data );
////////////////////////////

#define SDK_TP_AUTO_ADJUST_RANGE       // range自動調整スイッチ

#ifdef SDK_TP_AUTO_ADJUST_RANGE        // range自動調整スイッチ
void    TP_ExecSampling(SPITpData *data, s32 range, u16 *density);
#else
void    TP_ExecSampling(SPITpData *data, s32 range);
#endif

/*---------------------------------------------------------------------------*
    インライン関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TP_SPIChangeMode

  Description:  SPIコントロールレジスタを編集してタッチパネル用コマンド転送準備を
                整える。この時同時に、SPI連続クロック発振モードを切り替える。
                
                CODECの制限により、
                DSモードではSPIクロックは2MHzが限度となるので注意。
                TWLモードではSPIクロックは4MHzとなるため、
                この関数ではなくCDCi_ChangeSpiModeを使用する。

  Arguments:    continuous - SPI連続クロック発振可否。'1'で1byte通信の都度CSを上げ
                             ないモード。ただし連続転送の最後の1バイトでは'0'にして
                             1byte転送モードにしないと永久にCSが上がらない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void TP_SPIChangeMode(SPITransMode continuous)
{
    reg_SPI_SPICNT = (u16)((0x0001 << REG_SPI_SPICNT_E_SHIFT) | (0x0000 << REG_SPI_SPICNT_I_SHIFT) | (SPI_COMMPARTNER_TP << REG_SPI_SPICNT_SEL_SHIFT) | // 吉岡変更
//      ( SPI_COMMPARTNER_PMIC << REG_SPI_SPICNT_SEL_SHIFT ) |              // 吉岡変更前
                           (continuous << REG_SPI_SPICNT_MODE_SHIFT) |
                           (0x0000 << REG_SPI_SPICNT_BUSY_SHIFT) |
                           (SPI_BAUDRATE_2MHZ << REG_SPI_SPICNT_BAUDRATE_SHIFT));
}



/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBRARIES_TP_SP_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
