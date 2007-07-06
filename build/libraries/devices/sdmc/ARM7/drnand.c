/*---------------------------------------------------------------------------*
  Project:  TWL - rtfs interface for SD Memory Card
  File:     drnand.h

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include <twl/rtfs.h>
#include <portconf.h>
//#if (INCLUDE_SD)

#include "sdmc_config.h"
#include <twl/sdmc.h>
#include "sdif_ip.h"
#include "sdif_reg.h"

#if (SD_DEBUG_PRINT_ON == 1)
    #define PRINTDEBUG    OS_TPrintf
#else
    #define PRINTDEBUG( ...) ((void)0)
#endif


/*---------------------------------------------------------------------------*
  定数
 *---------------------------------------------------------------------------*/
#define NUM_SD_PAGES
#define SD_PAGE_SIZE


/*---------------------------------------------------------------------------*
  extern変数
 *---------------------------------------------------------------------------*/
extern int  rtfs_first_stat_flag[26];

/*SDメモリカードのスペック構造体*/
extern SdmcSpec    sdmc_current_spec;


/*---------------------------------------------------------------------------*
  extern関数
 *---------------------------------------------------------------------------*/
extern SDMC_ERR_CODE sdmcGoIdle(void (*func1)(),void (*func2)());
extern void i_sdmcCalcSize( void); //TODO:sdmc_current_specを構造体に入れること


/*---------------------------------------------------------------------------*
  static変数
 *---------------------------------------------------------------------------*/
static int     nand_drive_no;
static FATSpec NandFatSpec[4]; //FATパラメータ(パーティション0～3個別)
static int     nand_calculated_fat_params = 0;


/*---------------------------------------------------------------------------*
  static関数
 *---------------------------------------------------------------------------*/
static BOOL sdi_get_CHS_params( void);
static u32  sdi_get_ceil( u32 cval, u32 mval);
static BOOL sdi_get_nom( u32 min_nom);
static void sdi_get_fatparams( void);
static void sdi_build_partition_table( void);


#if 1    //アプリケーションでパーティション構成を決めたいとき

u32 NAND_FAT_PARTITION_COUNT;
u32 NAND_RAW_SECTORS;
u32 NAND_FAT0_SECTORS;
u32 NAND_FAT1_SECTORS;
u32 NAND_FAT2_SECTORS;
u32 NAND_FAT3_SECTORS;

void nandSetFormatRequest( u16 partition_num, u32* partition_sectors)
{
    NAND_RAW_SECTORS  = partition_sectors[0];
    NAND_FAT0_SECTORS = partition_sectors[1] + NAND_RAW_SECTORS;
    NAND_FAT1_SECTORS = partition_sectors[2];
    NAND_FAT2_SECTORS = partition_sectors[3];
    NAND_FAT3_SECTORS = partition_sectors[4];
    NAND_FAT_PARTITION_COUNT = partition_num;
}

#else    //パーティション構成を決め打ちするとき

#define NAND_FAT_PARTITION_COUNT (3)
#define NAND_RAW_SECTORS     (( 8*1024*1024)/512);
#define NAND_FAT0_SECTORS    (((16*1024*1024)/512) + NAND_RAW_SECTORS); //計算上RAWを含めておく
#define NAND_FAT1_SECTORS    ((16*1024*1024)/512);
#define NAND_FAT2_SECTORS    (( 8*1024*1024)/512);
#define NAND_FAT3_SECTORS    (0);

#endif


/*---------------------------------------------------------------------------*
  Name:         nandCheckMedia

  Description:  MBRのシグネチャおよび
                パーティションのフォーマット種別をチェックする

  Arguments:    

  Returns:      TRUE/FALSE
                （FALSEなら pc_format_media が必要）
 *---------------------------------------------------------------------------*/
BOOL nandCheckMedia( void) //TODO:nand partition仕様に対応させること
{
    u16             i;
    SdmcResultInfo  SdResult;
    u8*             bufp;
    u32             buffer[512/4];
    u8              systemid;

    /**/
    sdmcSelect( (u16)SDMC_PORT_NAND);
  
    /**/
    if( sdmcReadFifo( buffer, 1, 0, NULL, &SdResult)) {
        return( FALSE);
    }

    bufp = (u8*)buffer;

    /* Check the Signature Word. */
    if( (bufp[510]!=0x55) || (bufp[511]!=0xAA)) {
        return( FALSE);
    }
    /* Check the System ID of partition. */
    systemid = bufp[450];
    if( (systemid!=0x01) && (systemid!=0x04) && (systemid!=0x06) &&
        (systemid!=0x0B) && (systemid!=0x0C)) {
        return( FALSE);
    }

    return( TRUE);
}


/*---------------------------------------------------------------------------*
  Name:         nandRtfsIo

  Description:  上位層からのセクタリード／ライト要求を受ける

  Arguments:    driveno : ドライブ番号
                block : 開始ブロック番号
                buffer : 
                count : ブロック数
                reading : リード要求時にTRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
BOOL nandRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading)
{
    u16               result;
    SdmcResultInfo    SdResult;
    
    /**/
    sdmcSelect( (u16)SDMC_PORT_NAND);
  
    if( reading) {
        PRINTDEBUG( "DEVCTL_IO_READ ... block:%x, count:%x -> buf:%x\n", block, count, buffer);
        result = sdmcReadFifo( buffer, count, block, NULL, &SdResult);
//        result = sdmcRead( buffer, count, block, NULL, &SdResult);
    }else{
        PRINTDEBUG( "DEVCTL_IO_WRITE ... block:%x, count:%x <- buf:%x\n", block, count, buffer);
        result = sdmcWriteFifo( buffer, count, block, NULL, &SdResult);
//        result = sdmcWrite( buffer, count, block, NULL, &SdResult);
    }
    if( result) {
        return FALSE;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         nandRtfsCtrl

  Description:  上位層からのコントロール要求を受ける

  Arguments:    driveno : ドライブ番号
                opcode : 要求の種類
                pargs : 

  Returns:
  Memo :        DRIVE_FLAGS_REMOVABLEの場合、ドライバのIO関数を呼ぶ前に
                CTRL関数のDEVCTL_CHECK_STATUSが呼ばれる。
                DEVTEST_CHANGED→DEVTEST_NOCHANGEが確認されるとRTFSはセクタ0を
                読みに行き、FATパラメータを取得した上で目的のセクタを読みに行く。
                CTRL関数の前にはDEVCTL_CHECK_STATUSが呼ばれないので、DEVCTL_
                GET_GEOMETRYでは自前で再挿入をチェックする必要がある。
 *---------------------------------------------------------------------------*/
int nandRtfsCtrl( int driveno, int opcode, void* pargs)
{
    DDRIVE       *pdr;
    DEV_GEOMETRY gc;
    int          heads, secptrack;

    /**/
    sdmcSelect( (u16)SDMC_PORT_NAND);

    pdr = pc_drno_to_drive_struct( driveno);
    
    switch( opcode) {
      case DEVCTL_GET_GEOMETRY:    //formatまたはpartirionするときにRTFSが使うパラメータ
        PRINTDEBUG( "DEVCTL_GET_GEOMETRY\n");
      
        rtfs_memset( &gc, (byte)0, sizeof(gc));

        if( nand_calculated_fat_params == 0) {
            i_sdmcCalcSize();        //TODO:sdmc_current_specを構造体に入れること
            if( sdi_get_CHS_params() == FALSE) {    //最初に呼ぶこと
                return( -1);
            }
            sdi_get_fatparams();
            if( sdi_get_nom( NAND_RAW_SECTORS) == FALSE) {
                return( -1);
            }
            nand_calculated_fat_params = 1;
        }

        PRINTDEBUG( "memory capacity : 0x%x\n", NandFatSpec[pdr->partition_number].memory_capacity);
        PRINTDEBUG( "device capacity : 0x%x\n", NandFatSpec[pdr->partition_number].device_capacity);
        PRINTDEBUG( "adjusted capacity : 0x%x\n", NandFatSpec[pdr->partition_number].adjusted_memory_capacity);
        PRINTDEBUG( "volume cylinders : 0x%x\n", NandFatSpec[pdr->partition_number].volume_cylinders);
        PRINTDEBUG( "\n");
        PRINTDEBUG( "heads     : 0x%x\n", NandFatSpec[pdr->partition_number].heads);
        PRINTDEBUG( "secptrack : 0x%x\n", NandFatSpec[pdr->partition_number].secptrack);
        PRINTDEBUG( "cylinders : 0x%x\n", NandFatSpec[pdr->partition_number].cylinders);
        PRINTDEBUG( "SC        : 0x%x\n", NandFatSpec[pdr->partition_number].SC);
        PRINTDEBUG( "BU        : 0x%x\n", NandFatSpec[pdr->partition_number].BU);
        PRINTDEBUG( "RDE       : 0x%x\n", NandFatSpec[pdr->partition_number].RDE);
        PRINTDEBUG( "SS        : 0x%x\n", NandFatSpec[pdr->partition_number].SS);
        PRINTDEBUG( "RSC       : 0x%x\n", NandFatSpec[pdr->partition_number].RSC);
        PRINTDEBUG( "FATBITS   : 0x%x\n", NandFatSpec[pdr->partition_number].FATBITS);
        PRINTDEBUG( "SF        : 0x%x\n", NandFatSpec[pdr->partition_number].SF);
        PRINTDEBUG( "SSA       : 0x%x\n", NandFatSpec[pdr->partition_number].SSA);
        PRINTDEBUG( "NOM       : 0x%x\n", NandFatSpec[pdr->partition_number].NOM);
        
        /*デバイスの先頭から現在のパーティションの領域までを含む容量分をセットする*/
        gc.dev_geometry_lbas = (NandFatSpec[pdr->partition_number].begin_sect +
                                NandFatSpec[pdr->partition_number].memory_capacity);
        gc.dev_geometry_heads         = NandFatSpec[pdr->partition_number].heads;
        gc.dev_geometry_secptrack     = NandFatSpec[pdr->partition_number].secptrack;
                                
        /*デバイスの先頭から現在のパーティションの領域までを含む容量分をセットする*/
        gc.dev_geometry_cylinders     = gc.dev_geometry_lbas /
                                        (gc.dev_geometry_heads * gc.dev_geometry_secptrack);
        /**/
        gc.fmt_parms_valid   = TRUE;
        gc.fmt.oemname[0]    = 'T';
        gc.fmt.oemname[1]    = 'W';
        gc.fmt.oemname[2]    = 'L';
        gc.fmt.oemname[3]    = '\0';
        gc.fmt.secpalloc     = NandFatSpec[pdr->partition_number].SC;    /*sectors per cluster(FIX by capacity)*/
        gc.fmt.secreserved   = NandFatSpec[pdr->partition_number].RSC;//sdmc_current_spec.RSC;/*reserved sectors(FIX 1 at FAT12,16)*/
        gc.fmt.numfats       = 2;
        gc.fmt.secpfat       = NandFatSpec[pdr->partition_number].SF;
        gc.fmt.numhide       = NandFatSpec[pdr->partition_number].NOM;    /**/
        gc.fmt.numroot       = NandFatSpec[pdr->partition_number].RDE;    /*FIX*/
        gc.fmt.mediadesc     = 0xF8;
        gc.fmt.secptrk       = NandFatSpec[pdr->partition_number].secptrack;    //CHS Recommendation
        gc.fmt.numhead       = NandFatSpec[pdr->partition_number].heads;
        gc.fmt.numcyl        = gc.dev_geometry_cylinders;//NandFatSpec[pdr->partition_number].cylinders;
        gc.fmt.physical_drive_no = driveno;
        gc.fmt.binary_volume_label = BIN_VOL_LABEL;
        gc.fmt.text_volume_label[0] = '\0';

        PRINTDEBUG( "heads : 0x%x, secptrack : 0x%x, cylinders : 0x%x\n", gc.dev_geometry_heads, gc.dev_geometry_secptrack, gc.dev_geometry_cylinders);

        MI_CpuCopy8( &gc, pargs, sizeof(gc));
        return( 0);
        
      case DEVCTL_FORMAT:
        PRINTDEBUG( "DEVCTL_FORMAT\n");
        sdi_build_partition_table();    //MBRセクタ(パーティションテーブル含む)書き込み
        return( 0);

      case DEVCTL_REPORT_REMOVE:        //抜かれたとき
        PRINTDEBUG( "DEVCTL_REPORT_REMOVE\n");
        pdr->drive_flags &= ~DRIVE_FLAGS_INSERTED;
        return( 0);
        
      case DEVCTL_CHECKSTATUS:    //REMOVABLEの場合、毎回R/W前に呼ばれる
        PRINTDEBUG( "DEVCTL_CHECKSTATUS\n");
        if (!(pdr->drive_flags & DRIVE_FLAGS_REMOVABLE)) {    //リムーバブルでない場合
            return(DEVTEST_NOCHANGE);
        }
        if( rtfs_first_stat_flag[driveno]) {    //初回のCHECKSTATUS時
            rtfs_first_stat_flag[driveno] = 0;
            PRINTDEBUG( "CHANGED!\n");
            return(DEVTEST_CHANGED);
        }else{
            PRINTDEBUG( "DEVTEST_NOCHANGE\n");
            pdr->drive_flags |= DRIVE_FLAGS_INSERTED; //
            return( DEVTEST_NOCHANGE);
        }
        
      case DEVCTL_WARMSTART:    //attachのときしか呼ばれない
        PRINTDEBUG( "DEVCTL_WARMSTART\n");
        /*-- GoIdleセット --*/
        sdmcGoIdle( NULL, NULL);    //カード初期化シーケンス TODO:1ポートだけにする
        /*------------------*/
        pdr->drive_flags |= (DRIVE_FLAGS_VALID | DRIVE_FLAGS_REMOVABLE | DRIVE_FLAGS_PARTITIONED);
        pdr->drive_flags |= DRIVE_FLAGS_INSERTED;
        return( 0);
        
      case DEVCTL_POWER_RESTORE:
        PRINTDEBUG( "DEVCTL_POWER_RESTORE\n");
        break;
        
      case DEVCTL_POWER_LOSS:
        PRINTDEBUG( "DEVCTL_POWER_LOSS\n");
        break;
        
      default:
        PRINTDEBUG( "DEVCTL_unknown\n");
        break;
    }
    return( 0);
}

/*---------------------------------------------------------------------------*
  Name:         nandRtfsAttach

  Description:  sdmcドライバをドライブに割り当てる

  Arguments:    driveno : ドライブ番号

  Returns:      
 *---------------------------------------------------------------------------*/
BOOL nandRtfsAttach( int driveno, int partition_no)
{
    BOOLEAN   result;
    DDRIVE    pdr;

    if( partition_no >= NAND_FAT_PARTITION_COUNT) {
        return( FALSE);
    }
  
    pdr.dev_table_drive_io     = nandRtfsIo;
    pdr.dev_table_perform_device_ioctl = nandRtfsCtrl;
    pdr.register_file_address  = (dword) 0;    /* Not used */
    pdr.interrupt_number       = 0;            /* Not used */
    pdr.drive_flags            = (DRIVE_FLAGS_VALID | DRIVE_FLAGS_PARTITIONED);//DRIVE_FLAGS_FAILSAFE;
    pdr.partition_number       = partition_no; /* Not used */
    pdr.pcmcia_slot_number     = 0;            /* Not used */
    pdr.controller_number      = 0;
    pdr.logical_unit_number    = 0;

    switch( partition_no) {
      case 0:
        result = rtfs_attach( driveno, &pdr, "SD1p0"); //構造体がFSライブラリ側にコピーされる
        break;
      case 1:
        result = rtfs_attach( driveno, &pdr, "SD1p1"); //構造体がFSライブラリ側にコピーされる
        break;
      case 2:
        result = rtfs_attach( driveno, &pdr, "SD1p2"); //構造体がFSライブラリ側にコピーされる
        break;
      case 3:
        result = rtfs_attach( driveno, &pdr, "SD1p3"); //構造体がFSライブラリ側にコピーされる
        break;
      default:
        result = FALSE;
        break;
    }
    
    /*drivenoをグローバル変数に記憶*/
    nand_drive_no = driveno;

    return( result);
}


/*SD File System Specification(仕様書)に基づいた値を出す*/
static BOOL sdi_get_CHS_params( void)
{
    u16 i;
    int mbytes;
    u32 cumulative_capacity; //累計

    /**/
    NandFatSpec[0].memory_capacity = NAND_FAT0_SECTORS;
    NandFatSpec[1].memory_capacity = NAND_FAT1_SECTORS;
    NandFatSpec[2].memory_capacity = NAND_FAT2_SECTORS;
    NandFatSpec[3].memory_capacity = NAND_FAT3_SECTORS;


    cumulative_capacity = 0;
    for( i=0; i<(NAND_FAT_PARTITION_COUNT - 1); i++) {
        cumulative_capacity += NandFatSpec[i].memory_capacity;
    }
    /*容量破綻チェック*/
    if( cumulative_capacity >= sdmc_current_spec.memory_capacity) {
        return( FALSE);
    }

    /*最終パーティションは残りのセクタ全部*/
    NandFatSpec[NAND_FAT_PARTITION_COUNT -1].memory_capacity =
      sdmc_current_spec.memory_capacity - cumulative_capacity;

    /*無効なパーティションにサイズ0を設定*/
    for( i=NAND_FAT_PARTITION_COUNT; i<4; i++) {
        NandFatSpec[i].memory_capacity = 0;
    }
  

//    mbytes = (sdmc_current_spec.card_capacity / (1024 * 1024)) * 512;
    mbytes = (sdmc_current_spec.card_capacity >> 11); //TODO:forの中に入れてパーティション毎の値にするテストをすること

    for( i=0; i<NAND_FAT_PARTITION_COUNT; i++) {
    while( 1) {
        if( mbytes <= 2) {
            NandFatSpec[i].heads     = 2;
            NandFatSpec[i].secptrack = 16;
            break;
        }
        if( mbytes <= 16) {
            NandFatSpec[i].heads     = 2;
            NandFatSpec[i].secptrack = 32;
            break;
        }
        if( mbytes <= 32) {
            NandFatSpec[i].heads     = 4;
            NandFatSpec[i].secptrack = 32;
            break;
        }
        if( mbytes <= 128) {
            NandFatSpec[i].heads     = 8;
            NandFatSpec[i].secptrack = 32;
            break;
        }
        if( mbytes <= 256) {
            NandFatSpec[i].heads     = 16;
            NandFatSpec[i].secptrack = 32;
            break;
        }
        if( mbytes <= 504) {
            NandFatSpec[i].heads     = 16;
            NandFatSpec[i].secptrack = 63;
            break;
        }
        if( mbytes <= 1008) {
            NandFatSpec[i].heads     = 32;
            NandFatSpec[i].secptrack = 63;
            break;
        }
        if( mbytes <= 2016) {
            NandFatSpec[i].heads     = 64;
            NandFatSpec[i].secptrack = 63;
            break;
        }
        if( mbytes <= 2048) {
            NandFatSpec[i].heads     = 128;
            NandFatSpec[i].secptrack = 63;
            break;
        }
        if( mbytes <= 4032) {
            NandFatSpec[i].heads     = 128;
            NandFatSpec[i].secptrack = 63;
            break;
        }
        if( mbytes <= 32768) {
            NandFatSpec[i].heads     = 255;
            NandFatSpec[i].secptrack = 63;
            break;
        }
    }

        /*デバイス容量設定*/
        NandFatSpec[i].device_capacity = sdmc_current_spec.memory_capacity;

        /*---------- デバイス全体 ----------*/
        NandFatSpec[i].cylinders =    /*シリンダ数を計算*/
          (NandFatSpec[i].device_capacity /
           (NandFatSpec[i].heads * NandFatSpec[i].secptrack));

        /*memory_capacityを再計算してadjusted_memory_capacityに格納*/
        NandFatSpec[i].adjusted_device_capacity =
          NandFatSpec[i].cylinders *
            (NandFatSpec[i].heads * NandFatSpec[i].secptrack);
        /*----------------------------------*/

        /*---------- ボリューム ----------*/
        NandFatSpec[i].volume_cylinders =    /*シリンダ数を計算*/
          (NandFatSpec[i].memory_capacity /
           (NandFatSpec[i].heads * NandFatSpec[i].secptrack));

        /*memory_capacityを再計算してadjusted_memory_capacityに格納*/
        NandFatSpec[i].adjusted_memory_capacity =
          (NandFatSpec[i].volume_cylinders *
           (NandFatSpec[i].heads * NandFatSpec[i].secptrack));
        /*--------------------------------*/
    }
  
    PRINTDEBUG( "device_capacity:0x%x, adjusted:0x%x\n",
                NandFatSpec[0].device_capacity,
                NandFatSpec[0].adjusted_device_capacity);
    
    for( i=0; i<NAND_FAT_PARTITION_COUNT; i++) {
        PRINTDEBUG( "partition %d memory_capacity:0x%x, adjusted:0x%x\n",
                    i, NandFatSpec[i].memory_capacity,
                    NandFatSpec[i].adjusted_memory_capacity);
    }
    return( TRUE);
}


/*引数を超える最も小さい整数を算出する*/
static u32 sdi_get_ceil( u32 cval, u32 mval)
{
   return( (cval / mval) + (1 * (cval % mval != 0)));
}


/*マスターブートセクタのセクタ数を返す*/
static BOOL sdi_get_nom( u32 MIN_NOM)
{
    u32 RSC[4];
    u32 TS[4];
    u32 RDE = 512; //ルートディレクトリエントリ(FIX)
    u32 SS  = 512; //セクタサイズ(FIX)
    u32 SC, n, MAX, SFdash;
    u16 i;
    
    RSC[0] = 1; //FAT12,16では1
    RSC[1] = 1;
    RSC[2] = 1;
    RSC[3] = 1;

    TS[0] = NandFatSpec[0].adjusted_memory_capacity;
    TS[1] = NandFatSpec[1].adjusted_memory_capacity;
    TS[2] = NandFatSpec[2].adjusted_memory_capacity;
    TS[3] = NandFatSpec[3].adjusted_memory_capacity;

    NandFatSpec[0].begin_sect = 0;
    NandFatSpec[1].begin_sect = NandFatSpec[0].begin_sect + NandFatSpec[0].memory_capacity;
    NandFatSpec[2].begin_sect = NandFatSpec[1].begin_sect + NandFatSpec[1].memory_capacity;
    NandFatSpec[3].begin_sect = NandFatSpec[2].begin_sect + NandFatSpec[2].memory_capacity;


    for( i=0; i<NAND_FAT_PARTITION_COUNT; i++) {
      
        SC = NandFatSpec[i].SC;
    
        NandFatSpec[i].SF = sdi_get_ceil( TS[i]/SC * NandFatSpec[i].FATBITS, SS*8);

        /*-----------------------SDHCのとき----------------------------*/
        if( sdmc_current_spec.csd_ver2_flag) {
            PRINTDEBUG( "ERR! enter SDHC branch\n");
            /*nandの場合、NOMは少なくともMIN_NOM以上*/
            if( i==0) {
                NandFatSpec[i].NOM = sdi_get_ceil( MIN_NOM, NandFatSpec[i].BU) *
                                                   NandFatSpec[i].BU;
            }else{
                sdmc_current_spec.NOM = sdmc_current_spec.BU;
            }
            do {
                n = sdi_get_ceil( 2*NandFatSpec[i].SF, NandFatSpec[i].BU);
                NandFatSpec[i].RSC = (NandFatSpec[i].BU * n) - ( 2 * NandFatSpec[i].SF);
                if( NandFatSpec[i].RSC < 9) {
                    NandFatSpec[i].RSC += NandFatSpec[i].BU;
                }
                NandFatSpec[i].SSA = NandFatSpec[i].RSC + (2 * NandFatSpec[i].SF);
                do {
                    MAX = ((TS[i] - NandFatSpec[i].NOM - NandFatSpec[i].SSA) / SC) + 1;
                    SFdash = sdi_get_ceil( (2+(MAX-1)) * NandFatSpec[i].FATBITS, SS*8);
                    if( SFdash > NandFatSpec[i].SF) {
                        NandFatSpec[i].SSA += NandFatSpec[i].BU;
                        NandFatSpec[i].RSC += NandFatSpec[i].BU;
                    }else{
                        break;
                    }
                }while( 1);
                if( SFdash != NandFatSpec[i].SF) {
                    NandFatSpec[i].SF -= 1;
                }else{
                    break;
                }
            }while( 1);
        }else{    /*-------------------------SDのとき-------------------------------*/
            do {
                NandFatSpec[i].SSA = RSC[i] + ( 2 * NandFatSpec[i].SF) + sdi_get_ceil( 32*RDE, SS);
                n = sdi_get_ceil( NandFatSpec[i].SSA, NandFatSpec[i].BU);
              
                /*nand パーティション0の場合、NOMは少なくともMIN_NOM以上*/
                if( i==0) {
                    n+= sdi_get_ceil( MIN_NOM, NandFatSpec[i].BU);
                }
          
                NandFatSpec[i].NOM = (NandFatSpec[i].BU * n) - NandFatSpec[i].SSA;
                if( NandFatSpec[i].NOM != NandFatSpec[i].BU) {
                    NandFatSpec[i].NOM += NandFatSpec[i].BU;
                }
                do {
                    if( NandFatSpec[i].NOM >= sdmc_current_spec.memory_capacity) {
                      return( FALSE);
                    }
                    MAX = ((TS[i] - NandFatSpec[i].NOM - NandFatSpec[i].SSA) / SC) + 1;
                    SFdash = sdi_get_ceil( (2+(MAX-1)) * NandFatSpec[i].FATBITS, SS*8);
                    if( SFdash > NandFatSpec[i].SF) {
                        NandFatSpec[i].NOM += NandFatSpec[i].BU;
                    }else{
                        break;
                    }
                }while( 1);
                if( SFdash != NandFatSpec[i].SF) {
                    NandFatSpec[i].SF = SFdash;
                }else{
                    break;    //complete
                }
            }while( 1);
        }
    }

    for( i=0; i<NAND_FAT_PARTITION_COUNT; i++) {
        NandFatSpec[i].NOM += NandFatSpec[i].begin_sect; //各パーティションの先頭
        PRINTDEBUG( "before NOM:0x%x, begin_sect:0x%x\n", NandFatSpec[i].NOM, NandFatSpec[i].begin_sect);
        PRINTDEBUG( "partition %d  NOM:0x%x, SSA:0x%x, begin_sect:0x%x\n",
                    i, NandFatSpec[i].NOM, NandFatSpec[i].SSA, NandFatSpec[i].begin_sect);
    }

    return( TRUE);
}

/*FATのビット数を返す*/
static void sdi_get_fatparams( void)
{
    int i, mbytes;

    for( i=0; i<NAND_FAT_PARTITION_COUNT; i++) {
//        mbytes = (sdmc_current_spec.card_capacity / (1024 * 1024)) * 512;
        mbytes = (sdmc_current_spec.card_capacity >> 11);

    if( mbytes <= 64) {
        NandFatSpec[i].FATBITS = 12;
        NandFatSpec[i].RDE = 512;
        NandFatSpec[i].RSC = 1;
    }else{
        if( mbytes <= 2048) {
            NandFatSpec[i].FATBITS = 16;
            NandFatSpec[i].RDE = 512;
            NandFatSpec[i].RSC = 1;
        }else{
            NandFatSpec[i].FATBITS = 32;
            NandFatSpec[i].RDE = 0;    //FAT32のときは未使用。0にしておかないとRTFSが BAD FORMAT を返す。 
            NandFatSpec[i].RSC = 1;
        }
    }

    while( 1) {
        if( mbytes <= 8) {
            NandFatSpec[i].SC = 16;
            NandFatSpec[i].BU = 16;
            break;
        }
        if( mbytes <= 64) {
            NandFatSpec[i].SC = 32;
            NandFatSpec[i].BU = 32;
            break;
        }
        if( mbytes <= 256) {
            NandFatSpec[i].SC = 32;
            NandFatSpec[i].BU = 64;
            break;
        }
        if( mbytes <= 1024) {
            NandFatSpec[i].SC = 32;
            NandFatSpec[i].BU = 128;
            break;
        }
        if( mbytes <= 2048) {
            NandFatSpec[i].SC = 64;
            NandFatSpec[i].BU = 128;
            break;
        }
        if( mbytes <= 32768) {
            NandFatSpec[i].SC = 64;
            NandFatSpec[i].BU = 8192;
            break;
        }
        break;
    }
    }
}

/*MBRセクタ(パーティションセクタ含む)を生成して書き込む*/
static void sdi_build_partition_table( void)
{
    SdmcResultInfo SdResult;
    u16  MbrSectDat[512/2];
    u32  starting_head[4], starting_sect[4], starting_cyl[4];
    u32  ending_head[4],   ending_sect[4],   ending_cyl[4];
    u32  total_sect[4];
    u32  starting_data[4], ending_data[4];
    u32  systemid[4];
    u16  i;

    for( i=0; i<4; i++) {
        if( i < NAND_FAT_PARTITION_COUNT) {
            /**/
            starting_head[i] = NandFatSpec[i].NOM % (NandFatSpec[i].heads *
                                                     NandFatSpec[i].secptrack);
            starting_head[i] /= NandFatSpec[i].secptrack;

            /**/
            starting_sect[i] = (NandFatSpec[i].NOM % NandFatSpec[i].secptrack) + 1;

            /**/
            starting_cyl[i] = NandFatSpec[i].NOM / (NandFatSpec[i].heads *
                                                    NandFatSpec[i].secptrack);

            /**/
//            total_sect[i] = (NandFatSpec[i].adjusted_memory_capacity - NandFatSpec[i].NOM);
            total_sect[i] = (NandFatSpec[i].begin_sect +
                             NandFatSpec[i].adjusted_memory_capacity - NandFatSpec[i].NOM);
          
            ending_head[i] = (NandFatSpec[i].NOM + total_sect[i] - 1) %
                             (NandFatSpec[i].heads * NandFatSpec[i].secptrack);
            ending_head[i] /= NandFatSpec[i].secptrack;

            /**/
            ending_sect[i] = ((NandFatSpec[i].NOM + total_sect[i] - 1) %
                               NandFatSpec[i].secptrack) + 1;

            /**/
            ending_cyl[i] = (NandFatSpec[i].NOM + total_sect[i] - 1) /
                            (NandFatSpec[i].heads * NandFatSpec[i].secptrack);

            /**/
            if( NandFatSpec[i].FATBITS == 32) {    //FAT32のとき
                if( total_sect[i] < 0xFB0400) {        //8032.5MBが閾値(SD FileSystemSpec2.00参照)
                    systemid[i] = 0x0B;        /* FAT32 */
                }else{
                    systemid[i] = 0x0C;        /* FAT32(拡張INT13対応) */
                }
            }else{                                 //FAT12,FAT16のとき
                if( total_sect[i] < 32680) {
                    systemid[i] = 0x01;        /* FAT12 */
                }else if( total_sect[i] < 65536) {
                    systemid[i] = 0x04;        /* FAT16(16MB～32MB未満) */
                }else{
                    systemid[i] = 0x06;        /* FAT16(32MB～4GB) */
                }
            }
        }else{
            starting_head[i] = 0;
            starting_sect[i] = 0;
            starting_cyl[i]  = 0;
            total_sect[i]    = 0;
            ending_head[i]   = 0;
            ending_sect[i]   = 0;
            ending_cyl[i]    = 0;
            systemid[i]      = 0;
        }
    }
        
    /*MBRセクタ(パーティションテーブル含む)作成*/
    MI_CpuFill8( MbrSectDat, 0, 512);

    for( i=0; i<4; i++) {
        MbrSectDat[(446+(i*16))/2] = (starting_head[i]<<8);
        //上位8bit:starting_cylの下位8bit, 下位8bit:starting_cylの上位2bit + starting_sect 6bit.
        MbrSectDat[(448+(i*16))/2] = (starting_cyl[i]<<8) +
                                    ((starting_cyl[i]>>2) & 0xC0) + starting_sect[i];
        MbrSectDat[(450+(i*16))/2] = (ending_head[i]<<8) + systemid[i];
        //上位8bit:ending_cylの下位8bit, 下位8bit:ending_cylの上位2bit + ending_sect 6bit.
        MbrSectDat[(452+(i*16))/2] = (ending_cyl[i]<<8) +
                                    ((ending_cyl[i]>>2) & 0xC0) + ending_sect[i];
        MbrSectDat[(454+(i*16))/2] = NandFatSpec[i].NOM;
        MbrSectDat[(456+(i*16))/2] = (NandFatSpec[i].NOM>>16);
        MbrSectDat[(458+(i*16))/2] = total_sect[i];
        MbrSectDat[(460+(i*16))/2] = (total_sect[i]>>16);
    }
    MbrSectDat[510/2] = 0xAA55;
    /*セクタ0に書き込み*/
    sdmcWriteFifo( MbrSectDat, 1, 0, NULL, &SdResult);//MbrSectDatは2Byte alignかも知れないので危険
    
    /**/
    for( i=0; i<4; i++) {
        PRINTDEBUG( "---partition %d---\n", i);
        PRINTDEBUG( "total    sect : 0x%x\n", total_sect[i]);
        PRINTDEBUG( "starting head : 0x%x\n", starting_head[i]);
        PRINTDEBUG( "starting sect : 0x%x\n", starting_sect[i]);
        PRINTDEBUG( "starting cyl  : 0x%x\n", starting_cyl[i]);
        PRINTDEBUG( "ending   head : 0x%x\n", ending_head[i]);
        PRINTDEBUG( "ending   sect : 0x%x\n", ending_sect[i]);
        PRINTDEBUG( "ending   cyl  : 0x%x\n", ending_cyl[i]);
        PRINTDEBUG( "\n");
    }
}

//#endif /*(INCLUDE_SD)*/
