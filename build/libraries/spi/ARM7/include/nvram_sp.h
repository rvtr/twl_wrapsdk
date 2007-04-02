/*---------------------------------------------------------------------------*
  Project:  NitroSDK - libraries - spi
  File:     nvram_sp.h

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: nvram_sp.h,v $
  Revision 1.6  2006/01/18 02:11:30  kitase_hirotake
  do-indent

  Revision 1.5  2005/02/28 05:26:27  yosizaki
  do-indent.

  Revision 1.4  2004/10/07 06:49:34  terui
  NVRAMについて別メーカー製の場合のコマンド追加に伴う修正。

  Revision 1.3  2004/09/16 04:56:40  terui
  NVRAMのステータスレジスタに関する定義を追加。

  Revision 1.2  2004/09/07 00:34:27  takano_makoto
  SDK_SMALL_BUILD定義時にSDK_NVRAM_USE_READ_HIGHER_SPEEDを未定義にするよう変更。

  Revision 1.1  2004/09/06 12:54:18  terui
  libraries/spi/includeからlibraries/spi/ARM7/includeに移動。
  SPI処理予約方式の実装に伴う修正。

  Revision 1.5  2004/05/25 00:58:01  terui
  SPI各デバイス用ライブラリ細分化に伴う修正

  Revision 1.4  2004/05/12 10:51:44  terui
  メインメモリへのアクセスをMI関数にて行うコンパイルスイッチを追加
  ReadHigherSpeedインストラクションのみ切り離すコンパイルスイッチを追加

  Revision 1.3  2004/04/29 10:26:09  terui
  関数定義の引数削除に伴う変更

  Revision 1.2  2004/04/14 06:26:46  terui
  SPIライブラリのソース整理に伴う更新

  Revision 1.1  2004/04/05 04:46:37  terui
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_NVRAM_SP_H_
#define LIBRARIES_NVRAM_SP_H_

#include    <nitro/types.h>
#include    "spi_sp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// 高速読み出しコマンドにも対応する場合はdefineする
#ifndef SDK_SMALL_BUILD
#define     SDK_NVRAM_USE_READ_HIGHER_SPEED
#endif
// LE25FW203Tに特有のinstructionが必要な場合はdefineする
#define     SDK_NVRAM_ANOTHER_MAKER

// メインメモリへのアクセスをMIのバイトアクセス関数を使って省コードする場合にdefine
#define     SDK_SPI_LOW_SPEED_LOW_CODE_SIZE

// NVRAM操作コマンド定義
#define     NVRAM_INSTRUCTION_DUMMY         0x00        // 応答取得用のダミー命令
#define     NVRAM_INSTRUCTION_WREN          0x06        // 書き込み許可
#define     NVRAM_INSTRUCTION_WRDI          0x04        // 書き込み禁止
#define     NVRAM_INSTRUCTION_RDSR          0x05        // ステータスレジスタ読み出し
#define     NVRAM_INSTRUCTION_READ          0x03        // 読み出し
#define     NVRAM_INSTRUCTION_FAST_READ     0x0b        // 高速読み出し
#define     NVRAM_INSTRUCTION_PW            0x0a        // ページ書き込み
#define     NVRAM_INSTRUCTION_PP            0x02        // ページ書き込み(条件付)
#define     NVRAM_INSTRUCTION_PE            0xdb        // ページ消去
#define     NVRAM_INSTRUCTION_SE            0xd8        // セクタ消去
#define     NVRAM_INSTRUCTION_DP            0xb9        // 省電力
#define     NVRAM_INSTRUCTION_RDP           0xab        // 省電力から復帰
#ifdef  SDK_NVRAM_ANOTHER_MAKER
#define     NVRAM_INSTRUCTION_CE            0xc7        // チップイレース
#define     NVRAM_INSTRUCTION_RSI           0x9f        // シリコンID読み出し
#define     NVRAM_INSTRUCTION_SR            0xff        // ソフトウェアリセット
#endif

// NVRAMステータスレジスタ内ビット定義
#define     NVRAM_STATUS_REGISTER_WIP       0x01
#define     NVRAM_STATUS_REGISTER_WEL       0x02
#ifdef  SDK_NVRAM_ANOTHER_MAKER
#define     NVRAM_STATUS_REGISTER_ERSER     0x20
#endif

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/

// NVRAM用ワーク構造体
typedef struct NVRAMWork
{
    u16     command[SPI_PXI_CONTINUOUS_PACKET_MAX];

}
NVRAMWork;


/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
void    NVRAM_Init();
void    NVRAM_AnalyzeCommand(u32 data);
void    NVRAM_ExecuteProcess(SPIEntry * entry);

void    NVRAM_WriteEnable(void);
void    NVRAM_WriteDisable(void);
void    NVRAM_ReadStatusRegister(u8 *buf);
void    NVRAM_ReadDataBytes(u32 address, u32 size, u8 *buf);
#ifdef  SDK_NVRAM_USE_READ_HIGHER_SPEED
void    NVRAM_ReadDataBytesAtHigherSpeed(u32 address, u32 size, u8 *buf);
#endif
void    NVRAM_PageWrite(u32 address, u16 size, const u8 *buf);
void    NVRAM_PageProgram(u32 address, u16 size, const u8 *buf);
void    NVRAM_PageErase(u32 address);
void    NVRAM_SectorErase(u32 address);
void    NVRAM_DeepPowerDown(void);
void    NVRAM_ReleaseFromDeepPowerDown(void);
#ifdef  SDK_NVRAM_ANOTHER_MAKER
void    NVRAM_ChipErase(void);
void    NVRAM_ReadSiliconId(u8 *buf);
void    NVRAM_SoftwareReset(void);
#endif


/*---------------------------------------------------------------------------*
    インライン関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NVRAM_SPIChangeMode

  Description:  SPIコントロールレジスタを編集してNVRAM用にコマンド転送準備を整える。
                この時同時に、SPI連続クロック発振モードを切り替える。

  Arguments:    continuous - SPI連続クロック発振可否。'1'で1byte通信の都度CSを上げ
                             ないモード。ただし連続転送の最後の1バイトでは'0'にして
                             1byte転送モードにしないと永久にCSが上がらない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void NVRAM_SPIChangeMode(SPITransMode continuous)
{
    reg_SPI_SPICNT = (u16)((0x0001 << REG_SPI_SPICNT_E_SHIFT) |
                           (0x0000 << REG_SPI_SPICNT_I_SHIFT) |
                           (SPI_COMMPARTNER_EEPROM << REG_SPI_SPICNT_SEL_SHIFT) |
                           (continuous << REG_SPI_SPICNT_MODE_SHIFT) |
                           (0x0000 << REG_SPI_SPICNT_BUSY_SHIFT) |
                           (SPI_BAUDRATE_4MHZ << REG_SPI_SPICNT_BAUDRATE_SHIFT));
}


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBRARIES_NVRAM_SP_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
