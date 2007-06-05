/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - snd - channel
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
#include    <twl/fatfs/ARM7/rtfs.h>
#include    <twl/devices/sdmc/ARM7/sdmc.h>

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
u32 BlockBuf[512/4];
u32 BlockBuf2[512/4];


/*---------------------------------------------------------------------------*
  Name:         TwlSpMain

  Description:  Initialize and do main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlSpMain(void)
{
    OSHeapHandle heapHandle;
    SDMC_ERR_CODE result;
    SdmcResultInfo SdResult;

    // OS初期化
    OS_Init();

    OS_InitTick();
    OS_InitAlarm();
    
    OS_InitThread();

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


    /**/
    PRINTDEBUG("Sample program starts.\n");
//    rtfs_init();
    
    /*SDドライバ初期化*/
    result = sdmcInit( NULL, NULL);
    if( result != 0) {
        PRINTDEBUG( "sdmcInit : failed\n");
        while( 1) {};
    }else{
        PRINTDEBUG( "sdmcInit : success\n");
    }

    /*SDからブロックリード*/
    result = sdmcReadFifo( BlockBuf, 1, 0, NULL, &SdResult);
    if( result != 0) {
    PRINTDEBUG( "sdmcReadFifo failed.\n");    
    }
    PRINTDEBUG( "sdmcReadFifo success.\n");    


    /*SDへブロックライト*/
    MI_CpuFill8( BlockBuf2, 0xA5, 512);
    result = sdmcWriteFifo( BlockBuf2, 1, 0, NULL, &SdResult);
    if( result != 0) {
    PRINTDEBUG( "sdmcWriteFifo failed.\n");    
    }
    PRINTDEBUG( "sdmcWriteFifo success.\n");    


    /*SDからブロックリード*/
    result = sdmcReadFifo( BlockBuf2, 1, 0, NULL, &SdResult);
    if( result != 0) {
    PRINTDEBUG( "sdmcReadFifo failed.\n");    
    }
    PRINTDEBUG( "sdmcReadFifo success.\n");    

    
    /*SDへブロックライト*/
    result = sdmcWriteFifo( BlockBuf, 1, 0, NULL, &SdResult);
    if( result != 0) {
    PRINTDEBUG( "sdmcWriteFifo failed.\n");    
    }
    PRINTDEBUG( "sdmcWriteFifo success.\n");    

    /*デバイスドライバの登録*/
/*    if( sdmcRtfsAttach( 4) == FALSE) {  //sdmcをEドライブにする
        PRINTDEBUG( "sdmcRtfsAttach failed.\n");
    }else{
        if( sdmcRtfsAttach( 4) == FALSE) {
            PRINTDEBUG( "sdmcRtfsAttach success.\n");
        }else{
            PRINTDEBUG( "sdmcRtfsAttach error!.\n");
        }
    }

    if( !rtfs_pc_set_default_drive( (unsigned char*)"E:")) {
        PRINTDEBUG( "pc_set_default_drive failed\n");
        while( 1){};
    }
    PRINTDEBUG( "pc_set_default_drive success\n");*/

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
