/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS - include
  File:     systemWork.h

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: systemWork.h,v $
  Revision 1.2  2006/08/10 00:05:54  okubata_ryoma
  カートリッジの活線挿抜に関する不具合修正

  Revision 1.1  2006/07/05 09:16:15  yosizaki
  initial upload. (moved from hw/common/mmap_shared.h)

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_OS_COMMON_SYSTEMWORK_H_
#define NITRO_OS_COMMON_SYSTEMWORK_H_


/* if include from Other Environment for exsample VC or BCB, */
/* please define SDK_FROM_TOOL                               */
#if !(defined(SDK_WIN32) || defined(SDK_FROM_TOOL))
//
//--------------------------------------------------------------------
#ifndef SDK_ASM
#include        <nitro/types.h>

#include        <nitro/hw/common/mmap_shared.h>

#include        <nitro/os/common/thread.h>
#include        <nitro/os/common/spinLock.h>
#include        <nitro/os/common/arena.h>

typedef union
{
    u32     b32;
    u16     b16;
}
OSDmaClearSrc;
typedef struct
{
    u8      bootCheckInfo[0x20];       // 000-01f:   32byte boot check info
    u32     resetParameter;            // 020-023:    4byte reset parameter
    u8      padding5[0x8];             // 024-02c:  (8byte)
    u32     romBaseOffset;             // 02c-02f:    4byte ROM offset of own program
    u8      cartridgeModuleInfo[12];   // 030-03b:   12byte cartridge module info
    u32     vblankCount;               // 03c-03f:    4byte Vブランクカウント
    u8      wmBootBuf[0x40];           // 040-07f:   64byte WM のマルチブート用バッファ
    u8      nvramUserInfo[0x100];      // 080-17f: 256bytes NVRAM user info
    u8      isd_reserved1[0x20];       // 180-19f:  32bytes ISDebugger 予約
    u8      arenaInfo[0x48];           // 1a0-1e7:  72bytte アリーナ情報
    u8      real_time_clock[8];        // 1e8-1ef:   8bytes RTC
    u32     dmaClearBuf[4];            // 1f0-1ff:  16bytes DMA クリア情報バッファ (ARM9-TEG用)
    u8      rom_header[0x160];         // 200-35f: 352bytes ROM 内登録エリア情報退避バッファ
    u8      isd_reserved2[32];         // 360-37f:  32bytes ISDebugger 予約
    u32     pxiSignalParam[2];         // 380-387:   8bytes Param for PXI Signal
    u32     pxiHandleChecker[2];       // 388-38f:   8bytes Flag  for PXI Command Handler Installed
    u32     mic_last_address;          // 390-393:   4bytes マイク 最新サンプリング結果の格納アドレス
    u16     mic_sampling_data;         // 394-395:   2bytes マイク 単体サンプリング結果
    u16     wm_callback_control;       // 396-397:   2bytes WM コールバック同期用パラメータ
    u16     wm_rssi_pool;              // 398-399:   2bytes WM 受信強度による乱数源
    u8      ctrdg_SetModuleInfoFlag;   // 39a-39a:   1byte  set ctrdg module info flag
    u8      ctrdg_IsExisting;          // 39b-39b:   1byte  ctrdg exist flag
    u32     component_param;           // 39c-39f:   4bytes Component 同期用パラメータ
    OSThreadInfo *threadinfo_mainp;    // 3a0-3a3:   4bytes ARM9 スレッド情報へのポインタ 初期値0であること
    OSThreadInfo *threadinfo_subp;     // 3a4-3a7:   4bytes ARM7 スレッド情報へのポインタ 初期値0であること
    u16     button_XY;                 // 3a8-3a9:   2bytes XY ボタン情報格納位置
    u8      touch_panel[4];            // 3aa-3ad:   4bytes タッチパネル情報格納位置
    u16     autoloadSync;              // 3ae-3af:   2bytes autoload sync between processors
    u32     lockIDFlag_mainp[2];       // 3b0-3b7:   8bytes lockID管理フラグ(ARM9用)
    u32     lockIDFlag_subp[2];        // 3b8-3bf:   8bytes lockID管理フラグ(ARM7用)
    struct OSLockWord lock_VRAM_C;     // 3c0-3c7:   8bytes           Ｃ・ロックバッファ
    struct OSLockWord lock_VRAM_D;     // 3c8-3cf:   8bytes ＶＲＡＭ－Ｄ・ロックバッファ
    struct OSLockWord lock_WRAM_BLOCK0; // 3d0-3d7:   8bytes   ブロック０・ロックバッファ
    struct OSLockWord lock_WRAM_BLOCK1; // 3d8-3df:   8bytes ＣＰＵ内部ワークＲＡＭ・ブロック１・ロックバッファ
    struct OSLockWord lock_CARD;       // 3e0-3e7:   8bytes カード・ロックバッファ
    struct OSLockWord lock_CARTRIDGE;  // 3e8-3ef:   8bytes カートリッジ・ロックバッファ
    struct OSLockWord lock_INIT;       // 3f0-3f7:   8bytes 初期化ロックバッファ
    u16     mmem_checker_mainp;        // 3f8-3f9:   2bytes MainMomory Size Checker for Main processor
    u16     mmem_checker_subp;         // 3fa-3fb:   2bytes MainMomory Size Checker for Sub processor
    u8      padding4[2];               // 3fc-3fd: (2bytes)
    u16     command_area;              // 3fe-3ff:   2bytes Command Area
}
OSSystemWork;

#define OS_GetSystemWork()      ((OSSystemWork *)HW_MAIN_MEM_SYSTEM)
#endif


// マウントデバイス
typedef enum OSMountDevice {
    OS_MOUNT_DEVICE_SD   = 0,
    OS_MOUNT_DEVICE_NAND = 1,
    OS_MOUNT_DEVICE_MAX  = 2
}OSMountDevice;


// マウント対象
typedef enum OSMountTarget {
    OS_MOUNT_TGT_ROOT = 0,
    OS_MOUNT_TGT_FILE = 1,
    OS_MOUNT_TGT_DIR  = 2,
    OS_MOUNT_TGT_MAX  = 3
}OSMountTarget;


// パーミッション
typedef enum OSMountPermission {
    OS_MOUNT_USR_X = 0x01,
    OS_MOUNT_USR_W = 0x02,
    OS_MOUNT_USR_R = 0x04
}OSMountPermission;


// リソースの配置先
typedef enum OSMountResource {
    OS_MOUNT_RSC_MMEM = 0,
    OS_MOUNT_RSC_WRAM = 1
}OSMountResource;


#define OS_MOUNT_PARTITION_MAX_NUM      3           // マウント可能なパーティションのMax.No.
#define OS_MOUNT_DRIVE_START            'A'         // ドライブ名の先頭（大文字'A'-'Z'のみ指定可能）
#define OS_MOUNT_DRIVE_END              'Z'         // ドライブ名の最後
#define OS_MOUNT_ARCHIVE_NAME_LEN       16          // アーカイブ名のMax.length
#define OS_MOUNT_PATH_LEN               64          // パスのMax.length
#define OS_MOUNT_INFO_MAX               (size_t)((HW_TWL_FS_BOOT_SRL_PATH_BUF - HW_TWL_FS_MOUNT_INFO_BUF) / sizeof(OSMountInfo))


// アーカイブマウント情報構造体
typedef struct OSMountInfo {
    u8      drive[ 1 ];
    u8      device : 3;
    u8      target : 2;
    u8      partitionIndex : 2;
    u8      resource : 1;
    u8      userPermission : 3;                         // ユーザーがRW可能かどうかを指定する
    u8      rsv_A : 5;
    u8      rsv_B;
    char    archiveName[ OS_MOUNT_ARCHIVE_NAME_LEN ];   // \0終端込みのサイズ
    char    path[ OS_MOUNT_PATH_LEN ];                  // \0終端込みのサイズ
}OSMountInfo;   // 84bytes


// タイトルIDリスト構造体
typedef struct OSTitleIDList {
	u8		num;
	u8		rsv[ 15 ];
	u8		publicFlag [ 16 ];	// publicセーブデータがあるかどうかのフラグ
	u8		privateFlag[ 16 ];	// priateセーブデータがあるかどうかのフラグ
	u8		appJumpFlag[ 16 ];	// アプリジャンプ可能かどうかのフラグ
	u8		sameMakerFlag[ 16 ];	// 同じメーカーかどうかのフラグ
	u64		TitleID[ 118 ];
}OSTitleIDList; // 1024bytes


/*---------------------------------------------------------------------------*
  Name:         OS_GetMountInfo

  Description:  マウント情報を取得します。

  Arguments:    なし

  Returns:      マウント情報リストの先頭ポインタを返します。
 *---------------------------------------------------------------------------*/
inline const OSMountInfo *OS_GetMountInfo( void )
{
    return (const OSMountInfo *)HW_TWL_FS_MOUNT_INFO_BUF;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetBootSRLPath

  Description:  自分自身のSRLパス情報を取得します。

  Arguments:    なし

  Returns:      自分自身のSRLパスへのポインタを返します。
 *---------------------------------------------------------------------------*/
inline const char *OS_GetBootSRLPath( void )
{
    return (const char *)HW_TWL_FS_BOOT_SRL_PATH_BUF;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetTitleId

  Description:  get apprication's title ID

  Arguments:    None

  Returns:      title ID as u64 value
 *---------------------------------------------------------------------------*/
inline u64 OS_GetTitleId( void )
{
    return *(u64 *)(HW_TWL_ROM_HEADER_BUF + 0x230);
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetMakerCode

  Description:  get apprication's maker code

  Arguments:    None

  Returns:      maker code as u16 value
 *---------------------------------------------------------------------------*/
inline u16 OS_GetMakerCode( void )
{
    return *(u16 *)(HW_TWL_ROM_HEADER_BUF + 0x10);
}


#endif // SDK_FROM_TOOL


/* NITRO_OS_COMMON_SYSTEMWORK_H_ */
#endif
