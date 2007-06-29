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





extern void nandSetFormatRequest( u16 partition_num, u32* partition_sectors);


/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define PRINTDEBUG  OS_TPrintf

//配列partition_MB_sizeのインデックス
#define INDEX_RAW_PARTITION    0
#define INDEX_FAT0_PARTITION   1
#define INDEX_FAT1_PARTITION   2
#define INDEX_FAT2_PARTITION   3
#define INDEX_FAT3_PARTITION   4


/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystem(void);
static void VBlankIntr(void);

/*---------------------------------------------------------------------------*
    
 *---------------------------------------------------------------------------*/


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

static int pow10( int count)
{
  int i, result;
  
  result = 1;
  for( i=0; i<count; i++) {
    result *= 10;
  }
  return result;
}

/*入力された数字(3桁まで)を返す*/
static int get_number_prompt( void)
{
  int i = 0;
  char c;
  int j, keta, pow_num, result_num;
  int size_num[3]; //3桁MBytes
  
  while( (c = kmc_getchar()) != '\r') {
      if( (c >= '0')&&(c <= '9')) {
          kmc_putchar( c);
          size_num[i++] = ((int)c) % ((int)'0');
          if( i==3) { break;}
      }
      if( c == '\b') {
          if( i != 0) { i--;}
          kmc_putchar( c);
      }
  }
  keta = i;
  
  result_num = 0;
  for( j=0; j<keta; j++) {
      pow_num = pow10( keta-i);
      i--;
      result_num += (size_num[i] * pow_num);
  }
  return( result_num);
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
    byte TEST_FILENAME[] = "\\nand_p0_test.bin";
    byte VOLUME_LABEL[]  = "F:";
    u16 nand_fat_partition_num; //FATパーティション数
    u32 partition_MB_size[5];   //パーティション毎の容量
    /**/
    u32 block_buf[512/4];
    u32 arm9_ofs, arm9_size, arm7_ofs, arm7_size;
    u32 nand_firm_size = 0;
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

    PRINTDEBUG("\nnand_formatter_kmc starts.\n");
  
    /*----- RTFSが使うヒープ作成 -----*/
    {
      OSHeapHandle hh;
      OS_SetSubPrivArenaLo( OS_InitAlloc( OS_ARENA_MAIN_SUBPRIV, OS_GetSubPrivArenaLo(), OS_GetSubPrivArenaHi(), 1));
      hh = OS_CreateHeap( OS_ARENA_MAIN_SUBPRIV, OS_GetSubPrivArenaLo(), OS_GetSubPrivArenaHi());
      OS_SetCurrentHeap( OS_ARENA_MAIN_SUBPRIV, hh);
      if( rtfs_init() == FALSE) {
        PRINTDEBUG( "rtfs_init failed.\n");
      }
    }
    /*--------------------------------*/

    /*SDドライバ初期化*/
    result = sdmcInit( SDMC_NOUSE_DMA, NULL, NULL);
    if( result != SDMC_NORMAL) {
        PRINTDEBUG( "sdmcInit : failed\n");
        while( 1) {};
    }

    DBG_PRINTF( "NAND FLASH FORMAT?(y/n) -> ");
    if( FALSE == getchar_yes_no_prompt()) {
        PRINTDEBUG( "o\n");
        DBG_CHAR( '\n');
        goto NAND_FLASH_FORMAT_END;
    }
    DBG_PRINTF( "es\n");
    DBG_CHAR( '\n');
  

#if 1
    nand_fat_partition_num = 1;
    while( 1) {
        DBG_PRINTF( "RAW PARTITION SIZE?(MBytes) -> ");
        partition_MB_size[INDEX_RAW_PARTITION] = get_number_prompt();
        DBG_PRINTF( "  (%d MBytes)\n\n", partition_MB_size[INDEX_RAW_PARTITION]);

        DBG_PRINTF( "FAT PARTITION 0 SIZE?(MBytes) -> ");
        partition_MB_size[INDEX_FAT0_PARTITION] = get_number_prompt();
        DBG_PRINTF( "  (%d MBytes)\n\n", partition_MB_size[INDEX_FAT0_PARTITION]);
        if( partition_MB_size[INDEX_FAT0_PARTITION] == 0) {
            break;
        }
        nand_fat_partition_num++;

        DBG_PRINTF( "FAT PARTITION 1 SIZE?(MBytes) -> ");
        partition_MB_size[INDEX_FAT1_PARTITION] = get_number_prompt();
        DBG_PRINTF( "  (%d MBytes)\n\n", partition_MB_size[INDEX_FAT1_PARTITION]);
        if( partition_MB_size[INDEX_FAT1_PARTITION] == 0) {
            break;
        }
        nand_fat_partition_num++;

        DBG_PRINTF( "FAT PARTITION 2 SIZE?(MBytes) -> ");
        partition_MB_size[INDEX_FAT2_PARTITION] = get_number_prompt();
        DBG_PRINTF( "  (%d MBytes)\n\n", partition_MB_size[INDEX_FAT2_PARTITION]);
        if( partition_MB_size[INDEX_FAT2_PARTITION] == 0) {
            break;
        }
        nand_fat_partition_num++;
  
//        DBG_PRINTF( "FAT PARTITION 3 SIZE?(MBytes) -> ");
//        partition_MB_size[INDEX_FAT3_PARTITION] = get_number_prompt();
//        DBG_PRINTF( "  (%d MBytes)\n\n", partition_MB_size[INDEX_FAT3_PARTITION]);
        break;
    }
#endif
    DBG_PRINTF( "%d FAT Partitions.\n", nand_fat_partition_num);
  
  
    /*----- nandfirmチェック -----*/
//    pc_raw_read( 5, (byte*)block_buf, 1, 1, TRUE);
    nandRtfsIo( 5, 1, (byte*)block_buf, 1, TRUE);
    arm9_ofs  = *(u32*)(((u8*)block_buf)+0x20);
    arm9_size = *(u32*)(((u8*)block_buf)+0x2C);
    arm7_ofs  = *(u32*)(((u8*)block_buf)+0x30);
    arm7_size = *(u32*)(((u8*)block_buf)+0x3C);
//    DBG_PRINTF( "arm9: 0x%x, 0x%x\n", arm9_ofs, arm9_size);
//    DBG_PRINTF( "arm7: 0x%x, 0x%x\n", arm7_ofs, arm7_size);

    if( ((arm9_ofs + arm9_size) == arm7_ofs) && (arm9_ofs < arm7_ofs) &&
         (arm9_size != 0) && (arm7_size != 0)) {
        nand_firm_size = arm7_ofs + arm7_size;
        DBG_PRINTF( "nandfirm found. (size:0x%x bytes)\n", nand_firm_size);
        nand_firm_size = (nand_firm_size / 1024 / 1024) +
                         (((nand_firm_size % (1024*1024)) != 0)? 1:0);
//        DBG_PRINTF( "firm %dMB, raw %dMB\n", nand_firm_size, partition_MB_size[INDEX_RAW_PARTITION]);
      
        if( nand_firm_size > partition_MB_size[INDEX_RAW_PARTITION]) {
           
            DBG_PRINTF( "YOUR SETTING WILL ERASE NAND FIRM, OK?(y/n) -> ");
            if( FALSE == getchar_yes_no_prompt()) {
                PRINTDEBUG( "o\n");
                DBG_CHAR( '\n');
                DBG_PRINTF( "RAW PARTITION SETTING CHANGED (%dMB->%dMB)\n",
                            partition_MB_size[INDEX_RAW_PARTITION], nand_firm_size);
                partition_MB_size[INDEX_RAW_PARTITION] = nand_firm_size;
                goto NAND_FLASH_FORMAT_START;
            }
            DBG_PRINTF( "es\n");
            DBG_CHAR( '\n');
        }
    }else{
        DBG_PRINTF( "nandfirm is not found.\n");
    }
  
NAND_FLASH_FORMAT_START:
    /*------------------------------*/


    /*----- 最終確認 -----*/
    DBG_PRINTF( "\nFAT PARTITIONS : %d\n", nand_fat_partition_num);
    DBG_PRINTF( "DETAIL ... (RAW:%dMB)", partition_MB_size[INDEX_RAW_PARTITION]);
    for( i=0; i<nand_fat_partition_num-1; i++) {
        DBG_PRINTF( ", (FAT%d:%dMB)", i, partition_MB_size[INDEX_FAT0_PARTITION + i]);
    }
    DBG_PRINTF( ", (FAT%d:rest)", i);
    DBG_PRINTF( "\n");
    DBG_PRINTF( "ABOVE SETTING OK?(y/n) -> ");
    if( FALSE == getchar_yes_no_prompt()) {
        PRINTDEBUG( "o\n");
        DBG_CHAR( '\n');
        goto NAND_FLASH_FORMAT_END;
    }
    DBG_PRINTF( "es\n");
    DBG_CHAR( '\n');
    /*--------------------*/


    /*--- パーティション構成をセクタ単位にしてライブラリに要求 ---*/
    for( i=0; i<4; i++) {
        partition_MB_size[i] *= ((1024 * 1024) / 512);
        PRINTDEBUG( "p%d : %d\n", i, partition_MB_size[i]);
    }
    nandSetFormatRequest( nand_fat_partition_num, partition_MB_size);
    /*------------------------------------------------------------*/

    /*マウント*/
    if( nandRtfsAttach( 5, 0) == FALSE) {  //nandパーティション0をFドライブにする
        PRINTDEBUG( "nandRtfsAttach failed.\n");
        goto NAND_FLASH_FORMAT_END;
    }else{
        if( nandRtfsAttach( 5, 0) == FALSE) {
        }else{
            PRINTDEBUG( "nandRtfsAttach error!.\n");
            goto NAND_FLASH_FORMAT_END;
        }
    }

    if( !rtfs_pc_set_default_drive( (unsigned char*)"F:")) {
        PRINTDEBUG( "pc_set_default_drive failed\n");
        goto NAND_FLASH_FORMAT_END;
    }
  
  
    /**/
//    PRINTDEBUG( "pc_check_disk start. please wait.\n");
//    pc_check_disk( (byte*)"F:", &dstat, 0, 1, 1);
//    PRINTDEBUG( "pc_check_disk end.\n");


    /*--- MBR書き込み、パーティション0フォーマット ---*/
    if( !rtfs_pc_get_media_parms( (byte*)"F:", &geometry)) {
        PRINTDEBUG( "pc_get_media_parms failed\n");
        goto NAND_FLASH_FORMAT_END;
    }
  
    /**/
    if( !pc_format_media( (byte*)"F:", &geometry)) {
        PRINTDEBUG( "pc_format_media failed\n");
        goto NAND_FLASH_FORMAT_END;
    }
    PRINTDEBUG( "build MBR success.\n");
  
    /*ボリュームフォーマット*/
    if( !pc_format_volume( (byte*)"F:", &geometry)) {
        PRINTDEBUG( "pc_format_volume (p0) failed\n");
        goto NAND_FLASH_FORMAT_END;
    }
    PRINTDEBUG( "format FAT partition 0 success.\n");
    /*-------------------------------------------------*/


  
    /*マウント(F:p0, G:p1, H:p2, I:p3)*/
    for( i=1; i<nand_fat_partition_num; i++) {
        if( nandRtfsAttach( (5+i), i) == FALSE) {
            PRINTDEBUG( "nandRtfsAttach failed.\n");
            goto NAND_FLASH_FORMAT_END;
        }else{
            if( nandRtfsAttach( (5+i), i) == FALSE) {
            }else{
                PRINTDEBUG( "nandRtfsAttach error!.\n");
                goto NAND_FLASH_FORMAT_END;
            }
        }

    }
    /*-----------------------------*/


    /*ボリュームフォーマット*/
    for( i=1; i<nand_fat_partition_num; i++) {
        VOLUME_LABEL[0] = (byte)(((int)'F') + i);
        if( !rtfs_pc_get_media_parms( VOLUME_LABEL, &geometry)) {
            PRINTDEBUG( "pc_get_media_parms failed\n");
            goto NAND_FLASH_FORMAT_END;
        }
        if( !pc_format_volume( VOLUME_LABEL, &geometry)) {
            PRINTDEBUG( "pc_format_volume failed\n");
            goto NAND_FLASH_FORMAT_END;
        }
        PRINTDEBUG( "format FAT partition %d success.\n", i);
    }
    /*----------------------*/
    DBG_CHAR( '\n');
  
#if 0
    for( i=0; i<nand_fat_partition_num; i++) {
        VOLUME_LABEL[0] = (byte)(((int)'F') + i);
        /*---------- テストファイル作成 ----------*/
        if( !rtfs_pc_set_default_drive( VOLUME_LABEL)) {
            PRINTDEBUG( "pc_set_default_drive failed\n");
            while( 1){};
        }
        /*----------*/
        TEST_FILENAME[7] = (byte)(((int)'0')+i);
        fd = po_open( (byte*)"\\nand_p0_test.bin", (PO_CREAT|PO_BINARY|PO_WRONLY), PS_IWRITE);
        if( fd < 0) {
            PRINTDEBUG( "po_open (p0) failed.\n");
            while( 1) {};
        }
        /*----------*/

        /*----------*/
        if( po_close( fd) < 0) {
            PRINTDEBUG( "po_close (p0) failed.\n");
            while( 1) {};
        }
        /*----------*/
    }
    DBG_CHAR( '\n');
#endif


NAND_FLASH_FORMAT_END:
  
    PRINTDEBUG( "nand_formatter_kmc ends.\n");
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
