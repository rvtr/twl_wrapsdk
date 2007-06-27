/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nand_formatter_kmc
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include    <twl_sp.h>
#include    <twl/vlink.h>
#include    <twl/fatfs/ARM7/rtfs.h>
#include    <twl/devices/sdmc/ARM7/sdmc.h>

#define DBG_PRINTF OS_TPrintf
#define DBG_CHAR OS_PutChar
#define kmc_getchar() ((char)vlink_dos_get_console())
#define kmc_putchar(x) vlink_dos_put_console((char)(x))

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define PRINTDEBUG  OS_TPrintf

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystem(void);
static void VBlankIntr(void);

/*---------------------------------------------------------------------------*
    
 *---------------------------------------------------------------------------*/
u32 BlockBuf[(512*3)/4];
u32 BlockBuf2[(512*3)/4];


static BOOL getchar_yes_no_prompt(void)
{
  char c;
  while( 1 ) {
    c = kmc_getchar();
    kmc_putchar(c);
    if( c == 'y' ) {
      return TRUE;
    }
    else if(  c == 'n' ) {
      return FALSE;
    }
    else if(  c == 'q' ) {
      DBG_PRINTF("\nEnter infinite loop\n");
      while( 1 ) { ; }
    }
    else {
      DBG_PRINTF("\nInput y or n\n");
    }
  }
}



/*---------------------------------------------------------------------------*
  Name:         TwlSpMain

  Description:  Initialize and do main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlSpMain(void)
{
    u16 i;
    OSHeapHandle   heapHandle;
    SDMC_ERR_CODE  result;
    SdmcResultInfo SdResult;
    /**/
    PCFD           fd;
    CHKDISK_STATS dstat;
    DEV_GEOMETRY  geometry;
  

    // OS初期化
    OS_Init();

    OS_InitTick();
    OS_InitAlarm();
    
    // PXI初期化、ARM9と同期
    PXI_Init();

    // ヒープ領域設定
    heapHandle = InitializeAllocateSystem();


    // ボタン入力サーチ初期化
    (void)PAD_InitXYButton();

    // 割込み許可
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    OS_InitThread();


    /**/
    PRINTDEBUG("Sample program starts.\n");
    {
      OSHeapHandle hh;
      OS_SetSubPrivArenaLo( OS_InitAlloc( OS_ARENA_MAIN_SUBPRIV, OS_GetSubPrivArenaLo(), OS_GetSubPrivArenaHi(), 1));
      hh = OS_CreateHeap( OS_ARENA_MAIN_SUBPRIV, OS_GetSubPrivArenaLo(), OS_GetSubPrivArenaHi());
      OS_SetCurrentHeap( OS_ARENA_MAIN_SUBPRIV, hh);
      if( rtfs_init() == FALSE) {
        PRINTDEBUG( "rtfs_init failed.\n");
      }else{
        PRINTDEBUG( "rtfs_init success.\n");
      }
    }

    MI_DmaFill32( 2, BlockBuf,  0x55AA55AA, 512*3);
    MI_DmaFill32( 2, BlockBuf2, 0x00FF00FF, 512*3);
    
    /*SDドライバ初期化*/
    result = sdmcInit( SDMC_NOUSE_DMA, NULL, NULL);
    if( result != SDMC_NORMAL) {
        PRINTDEBUG( "sdmcInit : failed\n");
        while( 1) {};
    }else{
        PRINTDEBUG( "sdmcInit : success\n");
    }

    /*デバイスドライバの登録*/
    PRINTDEBUG( "attach start\n");
    if( nandRtfsAttach( 5) == FALSE) {  //nandをFドライブにする
        PRINTDEBUG( "nandRtfsAttach failed.\n");
    }else{
        if( nandRtfsAttach( 5) == FALSE) {
            PRINTDEBUG( "nandRtfsAttach success.\n");
        }else{
            PRINTDEBUG( "nandRtfsAttach error!.\n");
        }
    }

    if( !rtfs_pc_set_default_drive( (unsigned char*)"F:")) {
        PRINTDEBUG( "pc_set_default_drive failed\n");
        while( 1){};
    }
    PRINTDEBUG( "pc_set_default_drive success\n");

    /**/
    PRINTDEBUG( "pc_check_disk start. please wait.\n");
    pc_check_disk( (byte*)"F:", &dstat, 0, 1, 1);
    PRINTDEBUG( "pc_check_disk end.\n");


    if( !rtfs_pc_get_media_parms( (byte*)"F:", &geometry)) {
        PRINTDEBUG( "pc_get_media_parms failed\n");
    }
  
    if( !pc_format_media( (byte*)"F:", &geometry)) {
        PRINTDEBUG( "pc_format_media failed\n");
    }

    if( !pc_format_volume( (byte*)"F:", &geometry)) {
        PRINTDEBUG( "pc_format_volume failed\n");
    }

  
    PRINTDEBUG( "Sample program ends.\n");


    while (TRUE)
    {
        OS_Halt();

        //---- check reset
        if (OS_IsResetOccurred())
        {
            OS_ResetSystem();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  メモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    OS_TPrintf("OS_GetWramSubPrivArenaLo() = %p\n", OS_GetWramSubPrivArenaLo());
    OS_TPrintf("OS_GetWramSubPrivArenaHi() = %p\n", OS_GetWramSubPrivArenaHi());
    OS_TPrintf("OS_GetWramSubArenaLo() = %p\n", OS_GetWramSubArenaLo());
    OS_TPrintf("OS_GetWramSubArenaHi() = %p\n", OS_GetWramSubArenaHi());
    OS_TPrintf("OS_GetSubPrivArenaLo() = %p\n", OS_GetSubPrivArenaLo());
    OS_TPrintf("OS_GetSubPrivArenaHi() = %p\n", OS_GetSubPrivArenaHi());

    OS_TPrintf("call OS_SetWramSubPrivArenaHi(0x0380f980); to fix arena.\n");
    OS_SetWramSubPrivArenaHi((void*)0x0380f980);

    // メモリ割当て初期化
    tempLo = OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV,
                          OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi(), 1);

    // アリーナを0クリア
    MI_CpuClear8(tempLo, (u32)OS_GetWramSubPrivArenaHi() - (u32)tempLo);

    // アリーナ下位アドレスを設定
    OS_SetArenaLo(OS_ARENA_WRAM_SUBPRIV, tempLo);

    // ヒープ作成
    hh = OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV,
                       OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi());

    if (hh < 0)
    {
        OS_Panic("ARM7: Fail to create heap.\n");
    }

    // カレントヒープに設定
    (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);

    return hh;
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef SDK_TEG

extern BOOL PMi_Initialized;
void    PM_SelfBlinkProc(void);

static void VBlankIntr(void)
{
    //---- LED blink system
    if (PMi_Initialized)
    {
        PM_SelfBlinkProc();
    }
}

#else

static void VBlankIntr(void)
{
}

#endif

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
