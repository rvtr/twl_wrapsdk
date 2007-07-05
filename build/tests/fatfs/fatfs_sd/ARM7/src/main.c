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


#define DEBUG_DECLARE   OSTick begin
#define DEBUG_BEGIN()   (begin=OS_GetTick())
#define DEBUG_END(str)  OS_TPrintf( "\n%s was consumed %d msec.\n", #str, (int)OS_TicksToMilliSeconds(OS_GetTick()-begin))



/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystem(void);
static void VBlankIntr(void);

/*---------------------------------------------------------------------------*
    
 *---------------------------------------------------------------------------*/
u32 BlockBuf[(512*3)/4];
u32 BlockBuf2[(512*3)/4];

/*バッファオフセット*/
static u32 my_buf_offset = 0;

/*ユーザ転送関数*/
void MY_SdTransfer( void* sd_adr, u32 size, BOOL read_flag)
{
    if( read_flag) { //リード
        MI_DmaRecv32( 2, sd_adr, (void*)((u32)BlockBuf2 + my_buf_offset), size);
    }else{           //ライト
        MI_DmaSend32( 2, (void*)((u32)BlockBuf + my_buf_offset), sd_adr, size);
    }
    my_buf_offset += size;
}

/*バッファオフセットのリセット関数*/
void MY_SdTransferRewind( void)
{
    my_buf_offset = 0;
}

static void dump(const char *str, const void *buf, unsigned int len)
{
    int i;
    const unsigned char *ptr = buf;
    OS_TPrintf("\n%s (%d bytes):\n", str, len);
    for (i = 0; i < len; i++)
    {
        if ((i&0xf) == 0) OS_TPrintf("\n%08X: ", i);
        else        OS_TPrintf(" ");
        OS_TPrintf("%02X", *ptr++);
    }
    OS_TPrintf("\n");
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
    PCFD           fd;
    CHKDISK_STATS  dstat;
    DEBUG_DECLARE;

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
    result = sdmcInit( SDMC_USE_DMA_4/*SDMC_NOUSE_DMA*/, NULL, NULL);
    if( result != SDMC_NORMAL) {
        PRINTDEBUG( "sdmcInit : failed\n");
        while( 1) {};
    }else{
        PRINTDEBUG( "sdmcInit : success\n");
    }

#if 0
    /*--- SDへブロックライト／リード ---*/
//    result = sdmcSelect( (u16)SDMC_PORT_CARD);
    if( result != 0) {
        PRINTDEBUG( "sdmcSelect failed.\n");    
    }else{
        PRINTDEBUG( "sdmcSelect success.\n");    
    }
    
    result = sdmcWriteFifo( BlockBuf, 3, 10, NULL, &SdResult);
    if( result != 0) {
        PRINTDEBUG( "sdmcWriteFifo failed.\n");    
    }else{
        PRINTDEBUG( "sdmcWriteFifo success.\n");
    }
    
    result = sdmcReadFifo( BlockBuf2, 3, 10, NULL, &SdResult);
    if( result != 0) {
        PRINTDEBUG( "sdmcReadFifo failed.\n");    
    }else{
        PRINTDEBUG( "sdmcReadFifo success.\n");
    }
    /*----------------------------*/
#if 0
    /*NANDからブロックライト／リード*/
    result = sdmcSelect( (u16)SDMC_PORT_NAND);
    if( result != 0) {
        PRINTDEBUG( "sdmcSelect failed.\n");    
    }else{
        PRINTDEBUG( "sdmcSelect success.\n");    
    }
    
    MY_SdTransferRewind();
    result = sdmcWriteFifoDirect( (sdmcTransferFunction)&MY_SdTransfer, 3, 10, NULL, &SdResult);
    if( result != 0) {
        PRINTDEBUG( "sdmcWriteFifoDirect failed.\n");    
    }else{
        PRINTDEBUG( "sdmcWriteFifoDirect success.\n");
    }
    
    MY_SdTransferRewind();
    result = sdmcReadFifoDirect( (sdmcTransferFunction)&MY_SdTransfer, 3, 10, NULL, &SdResult);
    if( result != 0) {
        PRINTDEBUG( "sdmcReadFifoDirect failed.\n");    
    }else{
        PRINTDEBUG( "sdmcReadFifoDirect success.\n");
    }
#endif
    /*----------------------------*/

#else

    sdmcSelect( (u16)SDMC_PORT_CARD);
    DEBUG_BEGIN();
    result = sdmcReadFifo( (void*)BlockBuf, 2, 1, NULL, &SdResult);
    if( result != 0) {
        PRINTDEBUG( "sdmcReadFifo failed.\n");    
    }else{
        DEBUG_END(sdmcReadFifo);
        PRINTDEBUG( "sdmcReadFifo success.\n");
    }

    MI_DmaFill32( 2, BlockBuf,  0x55AA55AA, 512*3);
    MI_DmaFill32( 2, BlockBuf2, 0x00FF00FF, 512*3);

    DEBUG_BEGIN();
    result = sdmcWriteFifo( (void*)0x02004000, 10000, 1, NULL, &SdResult);
    if( result != 0) {
        PRINTDEBUG( "sdmcWriteFifo failed.\n");    
    }else{
        DEBUG_END(sdmcWriteFifo);
        PRINTDEBUG( "sdmcWriteFifo success.\n");
    }


  
    /*デバイスドライバの登録*/
    PRINTDEBUG( "attach start\n");
    if( sdmcRtfsAttach( 4) == FALSE) {  //sdmcをEドライブにする
        PRINTDEBUG( "sdmcRtfsAttach failed.\n");
    }else{
        if( sdmcRtfsAttach( 4) == FALSE) {
            PRINTDEBUG( "sdmcRtfsAttach success.\n");
        }else{
            PRINTDEBUG( "sdmcRtfsAttach error!.\n");
        }
    }
    if( nandRtfsAttach( 5, 0) == FALSE) {  //nandをFドライブにする
        PRINTDEBUG( "nandRtfsAttach failed.\n");
    }else{
        if( nandRtfsAttach( 5, 0) == FALSE) {
            PRINTDEBUG( "nandRtfsAttach success.\n");
        }else{
            PRINTDEBUG( "nandRtfsAttach error!.\n");
        }
    }

  for( i=0; i<2; i++) {
      if( (i % 2) == 0) {
          /*SDからブロックライト／リード*/
          if( !rtfs_pc_set_default_drive( (unsigned char*)"E:")) {
              PRINTDEBUG( "pc_set_default_drive (E) failed\n");
              while( 1){};
          }
          PRINTDEBUG( "pc_set_default_drive (E) success\n");
          /**/
          PRINTDEBUG( "pc_check_disk start. please wait.\n");
          DEBUG_BEGIN();
          pc_check_disk( (byte*)"E:", &dstat, 0, 1, 1);
          DEBUG_END(pc_check_disk);
          PRINTDEBUG( "pc_check_disk end.\n");
      }else{
          /*NANDからブロックライト／リード*/
          if( !rtfs_pc_set_default_drive( (unsigned char*)"F:")) {
              PRINTDEBUG( "pc_set_default_drive (F) failed\n");
              while( 1){};
          }
          PRINTDEBUG( "pc_set_default_drive (F) success\n");
          /**/
          PRINTDEBUG( "pc_check_disk start. please wait.\n");
          pc_check_disk( (byte*)"F:", &dstat, 0, 1, 1);
          PRINTDEBUG( "pc_check_disk end.\n");
      }


      /*----------*/
      fd = po_open( (byte*)"\\sdmc_twl_test.bin", (PO_CREAT|PO_BINARY|PO_WRONLY), PS_IWRITE);
      if( fd < 0) {
          PRINTDEBUG( "po_open failed.\n");
          while( 1) {};
      }
      PRINTDEBUG( "po_open success.\n");
      /*----------*/

      dump( "BlockBuf", BlockBuf, 0x200);
      PRINTDEBUG( "po_write : 0x%x\n", po_write( fd, (u8*)BlockBuf2, 512));
      dump( "BlockBuf2(write data)", BlockBuf2, 0x200);

      po_lseek( fd, 0, PSEEK_SET);
      
      PRINTDEBUG( "po_read  : 0x%x\n", po_read( fd, (u8*)BlockBuf, 512));
      dump( "BlockBuf(read data)", BlockBuf, 0x200);

      /*----------*/
      if( po_close( fd) < 0) {
          PRINTDEBUG( "po_close failed.\n");
          while( 1) {};
      }
      PRINTDEBUG( "po_close success.\n");
      /*----------*/

      DEBUG_BEGIN();
      if( pc_regression_test( (u8*)"E:", FALSE) == FALSE) {
          PRINTDEBUG( "pc_regression_test failed.\n");
      }else{
          DEBUG_END(pc_regression_test);
          PRINTDEBUG( "pc_regression_test success.\n");
      }
    }
#endif

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
