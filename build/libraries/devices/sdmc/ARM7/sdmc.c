/*---------------------------------------------------------------------------*
  Project:  TWL - SD port driver
  File:     sdmc.c

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "sdmc_config.h"
#include "sdif_reg.h"           /*  IP 対応レジスタ定義 */
#include <twl/sdmc.h>
#include "sdif_ip.h"            /*  IP 対応フラグ定義 */

//        #define PRINTDEBUG    OS_TPrintf
    #define PRINTDEBUG( ...) ((void)0)


/***********************************************************************
 定数
***********************************************************************/
#define SD_STACK_SIZE        (4096*2)
#define SD_THREAD_PRIO       (10)
#define SD_INTR_THREAD_PRIO  (SD_THREAD_PRIO - 1)

#define SD_OPERATION_INIT               (0)
#define SD_OPERATION_READ               (1)
#define SD_OPERATION_READ_WITH_FIFO     (2)
#define SD_OPERATION_WRITE              (3)
#define SD_OPERATION_WRITE_WITH_FIFO    (4)

/*
TODO:
SD_INTR_THREAD_PRIOを高くしてSleepをなくせば動く。
Sleepを使うには、Wakeupの前にSleepさせるためSD_INTR_THREAD_PRIOを低く
する必要があるが、Sleepの前にSD割り込みが入らなかった場合なぜかずっと
割り込みが入らなくなり動かなくなる。Sleepの前に割り込みが入れば動く。
*/


/***********************************************************************
 extern変数
***********************************************************************/
extern u16 BgBak[32*32];
//u16 sdcard_request_flag;            //カードからのデータ転送要求の有無フラグ
static BOOL    thread_flag;


/***********************************************************************
 global変数
***********************************************************************/
BOOL    sdmc_tsk_created = FALSE;
#if (TARGET_OS_CTR == 1)
ER_ID   sdmc_tsk_id;        //SDタスクID
ER_ID   sdmc_dtq_id;
ER_ID   sdmc_result_dtq_id;
ER_ID   sdmc_alm_id;
ER_ID   sdmc_intr_tsk_id;
#else
OSThread       sdmc_tsk;
OSMessageQueue sdmc_dtq;
OSMessage      sdmc_dtq_array[1];
OSMessageQueue sdmc_result_dtq;
OSMessage      sdmc_result_dtq_array[1];
OSAlarm        sdmc_alm;
OSThread       sdmc_intr_tsk;
#endif

u16 sdmc_wakeup_count = 0; //OS_WakeupThreadのキューイング代わり
u16 sdmc_intr_wakeup_count = 0;

#if (TARGET_OS_CTR == 1)
#else
u64            sd_stack[SD_STACK_SIZE / sizeof(u64)];
u64            sd_intr_stack[SD_STACK_SIZE / sizeof(u64)];
#endif

u16         sdmc_dma_no = SDMC_NOUSE_DMA;
/* drsdmc.cでも参照 */
SdmcSpec    sdmc_current_spec;    //TODO:ポート切り替え時、Port0とPort1に保存するように

typedef struct {                  //OSMessage
    void*           buf;
    u32             bufsize;
    u32             offset;
    void            (*func)(void);
    SdmcResultInfo* info;
    u32             operation;
    void            (*func2)(void);
} SDCARDMsg;


/***********************************************************************
 static関数の宣言
***********************************************************************/
void i_sdmcCalcSize( void);
static void SDCARD_Backup_port0(void);
static void SDCARD_Backup_port1(void);
static void SDCARD_Restore_port0(void);
static void SDCARD_Restore_port1(void);

static SDMC_ERR_CODE SDCARDi_ReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
static SDMC_ERR_CODE SDCARDi_WriteFifo( const void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info);

static SDMC_ERR_CODE SDCARDi_Read(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info);
static SDMC_ERR_CODE SDCARDi_Write( const void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info);

int     MMCP_SetBusWidth( BOOL b4bit);       /* ビット幅の選択(MMCplus, eMMC, moviNAND) */

static void SDCARD_Thread( void* arg);        //SDスレッド
static void SDCARD_Intr_Thread( void* arg);        //SD割り込み処理スレッド

SDMC_ERR_CODE sdmcGoIdle( void (*func1)(),void (*func2)());
static SDMC_ERR_CODE i_sdmcInit( void);
static SDMC_ERR_CODE SDCARD_Layer_Init(void);
static SDMC_ERR_CODE i_sdmcMPInit( void);     /* カードドライバ初期化(マルチポート対応) */

static u16 i_sdmcErrProcess(void);            /* エラー時の処理 */
static u16 i_sdmcGetResid(u32 *pResid);       /* 書きこみ完了セクタ数の取得 */
static u16 i_sdmcCheckWP(void);               /* SDCARD ライトプロテクトチェック */

static void i_sdmcEnable( void);              /* SD使用のためのOS準備 */
static void i_sdmcDisable( void);

static u16 i_sdmcSendSCR(void);               /* SCRの読み出し */
static u16 SDCARD_SD_Status(void);            /* SD_STATUSの取得 */
static u32 SDCARD_GetR1Status(void);          /* Normal response command カードステータスを取得 */

static void SDCARD_Dmy_Handler( void);        /* 何もしない */
static void SDCARD_Timer_irq(void* arg);      /* タイムアウト割り込みハンドラ */
static void SDCARD_irq_Handler( void);        /* SD-IPからの割り込みハンドラ */
static void SDCARD_FPGA_irq(void);            /* カードリードライト割り込み処理 */
static void SYSFPGA_irq(void);                /* SYSFPGAエラー割り込み処理 */
static void SDCARD_ReadyToEnd( void);

/*ポート1は無線固定なのでポート選択関数は公開しない*/
static u16 i_sdmcSelectedNo(void);            /* カードポートの選択 */
static u16 i_sdmcSelect(u16 select);          /* 現在のカードポート番号のチェック */




/***********************************************************************
    外部参照変数
***********************************************************************/
static void SDCARD_TimerStart(u32 tim);    /* タイムアウト計測スタート */
static void SDCARD_TimerStop(void);        /* タイムアウト計測停止 */

extern u16  SD_CID[8];                /* CID値保存用 */
extern u16  SD_CSD[8];                /* CSD値保存用 */
extern u16  SD_OCR[2];                /* OCR値保存用 */
extern u16  SD_SCR[4];                /* SCR値保存用 */
extern u16  SD_RCA;                   /* RCA値保存用 */

extern s16  SDCARD_MMCFlag;           /* MMCカードフラグ */
extern s16  SDCARD_SDHCFlag;          /* SDHCカードフラグ */
extern u16  SD_port_number;           /* 現在ポート番号  */

//static BOOL init_io_exist;            /* SDIO存在フラグ */
//static BOOL init_mem_exist;           /* メモリ存在フラグ */


/***********************************************************************
    グローバル
***********************************************************************/
u16  SD_SDSTATUS[32];            /* SD_STATUSレジスタ保存用 */
u16  SDCARD_WP_FLAG0;            /* カードライトプロテクトフラグ。0=なし、1=有り    */
u16  SDCARD_WP_FLAG1;            /* カードライトプロテクトフラグ。0=なし、1=有り    */
u16  SDCARD_WP_PERMANENT;        /* カードライトプロテクト永久フラグ。0=なし、1=有り    */
u16  SDCARD_WP_TEMPORARY;        /* カードライトプロテクト一時フラグ。0=なし、1=有り    */

u16* pSDCARD_BufferAddr;         /* 保存用データ格納バッファアドレス */

u32  ulSDCARD_SectorCount;       /* 転送セクタ数 */
u32  ulSDCARD_RestSectorCount;   /* 残り転送セクタ数 */
u32  SDCARD_SectorSize;          /* セクタサイズ デフォルト 512bytes */

u16  SD_INFO1_VALUE;             /* SD_INFO1レジスタ取得用変数 */
u16  SD_INFO1_MASK_VALUE;        /* SD_INFO1割込みマスク用変数(0で許可, 1で禁止) */
u16  SD_INFO2_VALUE;             /* SD_INFO2レジスタ取得用変数 */
u16  SD_INFO2_MASK_VALUE;        /* SD_INFO2割り込みマスク用変数(0で許可, 1で禁止) */
u16  SD_INFO_ERROR_VALUE;        /* SD_INFO2, SD_INFO1のエラービット確認用変数 */

u16  SD_port_en_numbers;         /* サポートするポート数 */



/*ポート状態保存*/
SDPortContext SDPort0Context;
SDPortContext SDPort1Context;
//SDPortContext *SDPortCurrentContext = &SDPort0Context; /*TODO*/



u32    ulSDCARD_Size;                    /* カード全セクタ数 */

volatile s16    SDCARD_EndFlag;          /* 転送処理完了フラグ */

SDMC_ERR_CODE   SDCARD_ErrStatus;        /* エラーステータス */
vu32            SDCARD_Status;           /* カードステータス */
s16             SDCARD_SDFlag;           /* SDカードフラグ */

volatile s16    SDCARD_OutFlag;          /* カード排出発生判定フラグ */
SdmcResultInfo  *pSDCARD_info;           /* 保存用実行結果構造体ポインタ */

u16             SDCARD_IO_Port;          /* カード挿入/排出割り込み発生時のポート番号 */

void (*func_SDCARD_In)(void);            /* カード挿入イベント用コールバック保存用 */
void (*func_SDCARD_Out)(void);           /* カード排出イベント用コールバック保存用 */
/* void (*func_SDCARD_CallBack)(SdmcResultInfo *info);     処理結果通知用コールバック保存用 */





/*---------------------------------------------------------------------------*
  Name:         MyCpuRecv*

  Description:  CPUで同じアドレスからデータを受け取ります

  Arguments:    src : 転送元アドレス、dest : 転送先アドレス、size : 転送サイズ(Bytes)

  Returns:      None
 *---------------------------------------------------------------------------*/
static void MyCpuRecv32( const u32* src, u32* dest, u32 size)
{
    u32 i;
    for( i=0; i<(size/sizeof(u32)); i++) {
        *(u32*)dest++ = *(u32*)src;
    }
}
static void MyCpuRecv16( const u16* src, u16* dest, u32 size)
{
    u32 i;
    for( i=0; i<(size/sizeof(u16)); i++) {
        *(u16*)dest++ = *(u16*)src;
    }
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_Backup_port0

  Description:  backup registers and variables of port0.
                ポート0のレジスタや変数をバックアップする

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Backup_port0(void)
{
    /* registers */
    SD_GetFPGA(SDPort0Context.SD_CLK_CTRL_VALUE,SD_CLK_CTRL);
    SD_GetFPGA(SDPort0Context.SD_OPTION_VALUE,  SD_OPTION);

    /* variables */
    SDPort0Context.SD_RCA    = SD_RCA;
    SDPort0Context.ErrStatus = SDCARD_ErrStatus;
    SDPort0Context.Status    = SDCARD_Status;
    SDPort0Context.MMCFlag   = SDCARD_MMCFlag;
    SDPort0Context.SDHCFlag  = SDCARD_SDHCFlag;
    SDPort0Context.SDFlag    = SDCARD_SDFlag;

    SDPort0Context.OutFlag   = SDCARD_OutFlag;

    /*media registers*/
    MI_CpuCopy8( SD_CID, SDPort0Context.SD_CID, 16);
    MI_CpuCopy8( SD_CSD, SDPort0Context.SD_CSD, 16);
    MI_CpuCopy8( SD_OCR, SDPort0Context.SD_OCR,  4);
    MI_CpuCopy8( SD_SCR, SDPort0Context.SD_SCR,  8);
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Backup_port1

  Description:  backup registers and variables of port1.
                ポート1のレジスタや変数をバックアップする

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Backup_port1(void)
{
    /* registers */
    SD_GetFPGA(SDPort1Context.SD_CLK_CTRL_VALUE,SD_CLK_CTRL);
    SD_GetFPGA(SDPort1Context.SD_OPTION_VALUE,  SD_OPTION);

    /* variables */
    SDPort1Context.SD_RCA       = SD_RCA;
    SDPort1Context.ErrStatus = SDCARD_ErrStatus;
    SDPort1Context.Status    = SDCARD_Status;
    SDPort1Context.MMCFlag   = SDCARD_MMCFlag;
    SDPort1Context.SDHCFlag  = SDCARD_SDHCFlag;
    SDPort1Context.SDFlag    = SDCARD_SDFlag;

    SDPort1Context.OutFlag   = SDCARD_OutFlag;

    /*media registers*/
    MI_CpuCopy8( SD_CID, SDPort1Context.SD_CID, 16);
    MI_CpuCopy8( SD_CSD, SDPort1Context.SD_CSD, 16);
    MI_CpuCopy8( SD_OCR, SDPort1Context.SD_OCR,  4);
    MI_CpuCopy8( SD_SCR, SDPort1Context.SD_SCR,  8);
}


/*---------------------------------------------------------------------------*
  Name:         i_sdmcEnable

  Description:  assign OS resouce for using SD memory card.
                SDカードを使うための準備をする

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void i_sdmcEnable( void)
{
    /*SD割り込みのIF解除*/
    *(vu16*)CTR_INT_IF = CTR_IE_SD_MASK;
    OS_SetIrqFunction( OS_IE_SD, SDCARD_irq_Handler);
    OS_EnableIrqMask( OS_IE_SD);
}

/*---------------------------------------------------------------------------*
  Name:         i_sdmcDisable

  Description:  under construction
                工事中

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void i_sdmcDisable( void)
{
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_irq_Handler

  Description:  SD interrupt handler
                SD割り込みハンドラ

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SDCARD_irq_Handler( void)
{
//    OS_TPrintf( "SD irq!\n");
    OS_SetIrqCheckFlag( OS_IE_SD);
//  sdmc_intr_wakeup_count++;
//    OS_WakeupThreadDirect( &sdmc_intr_tsk);
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Dmy_Handler

  Description:  dmy handler for timer interrupt.
                タイマー割り込み用ダミーハンドラ

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Dmy_Handler( void)
{
}

/*---------------------------------------------------------------------------*
  Name:         sdmcInit

  Description:  Initialize SD interface and SD card.
                初期化

  Arguments:    dma_no : DMA番号
                func1 : カード挿入時コールバック関数
                func2 : カード排出時コールバック関数

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcInit( SDMC_DMA_NO dma_no, void (*func1)(),void (*func2)())
{
    SDMC_ERR_CODE    api_result;

    /*--- DMA / CPU 選択 ---*/
    if( (dma_no < 4) || (dma_no == SDMC_NOUSE_DMA)) {
        sdmc_dma_no = dma_no;
    }else{
        return( SDMC_ERR_PARAM);
    }
    /*----------------------*/
  
    if( sdmc_tsk_created == FALSE) {
        /*---------- OS準備 ----------*/
        if( !OS_IsAlarmAvailable()) {	/* アラームチェック(OS_InitAlarm済みか?) */
            SDCARD_ErrStatus |= SDMC_ERR_END;
        }else{
            OS_CreateAlarm( &sdmc_alm);	//使用可能であれば初期化
        }
        
        /* メッセージ初期化 */
        OS_InitMessageQueue( &sdmc_dtq,        &sdmc_dtq_array[0],        1);
        OS_InitMessageQueue( &sdmc_result_dtq, &sdmc_result_dtq_array[0], 1);

        /* SDスレッドの立ち上げ */
        OS_CreateThread( &sdmc_tsk, SDCARD_Thread, NULL,
                         (sd_stack+SD_STACK_SIZE / sizeof(u64)), SD_STACK_SIZE, SD_THREAD_PRIO);
        OS_WakeupThreadDirect( &sdmc_tsk);
        PRINTDEBUG( "sdmc_tsk:0x%x\n", &sdmc_tsk);
        

        /* SD割り込み処理スレッドの立ち上げ */
        (void)OS_ClearIrqCheckFlag( OS_IE_SD);
        OS_CreateThread( &sdmc_intr_tsk, SDCARD_Intr_Thread, NULL,
                         (sd_intr_stack+SD_STACK_SIZE / sizeof(u64)), SD_STACK_SIZE, SD_INTR_THREAD_PRIO);
        OS_WakeupThreadDirect( &sdmc_intr_tsk);
        PRINTDEBUG( "sdmc_intr_tsk:0x%x\n", &sdmc_intr_tsk);
        /*----------------------------*/

        /**/
        sdmc_tsk_created = TRUE;
    }
    
    /**/
    func_SDCARD_In = func1;         /* カード挿入イベント用関数のアドレスを設定 */
    func_SDCARD_Out = func2;        /* カード排出イベント用関数のアドレスを設定 */
    api_result = sdmcGoIdle( func1, func2);
//    api_result = SDMC_NORMAL;
    
    return api_result;
}

/* カードが入れ換わったときなどに初期化（RTFS用）*/
SDMC_ERR_CODE sdmcGoIdle( void (*func1)(),void (*func2)())
{
    SDCARDMsg        SdMsg;
    OSMessage        init_msg;
    SDMC_ERR_CODE    api_result;
    
    func_SDCARD_In  = func1;        /* カード挿入イベント用関数のアドレスを設定 */
    func_SDCARD_Out = func2;        /* カード排出イベント用関数のアドレスを設定 */
    
    /*----- SDスレッドと通信 -----*/
    SdMsg.operation = SD_OPERATION_INIT;
//    SdMsg.func  = func1;
//    SdMsg.func2 = func2;

    init_msg = (OSMessage)&SdMsg;
    OS_SendMessage( &sdmc_dtq, init_msg, OS_MESSAGE_BLOCK);

    /* 返り値待ち */
    OS_ReceiveMessage( &sdmc_result_dtq, &init_msg, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)init_msg;
    /*----------------------------------*/

    return api_result;
}


/*---------------------------------------------------------------------------*
  Name:         i_sdmcInit

  Description:  Initialize SD interface and SD card.
                初期化

  Arguments:    

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE i_sdmcInit( void)
{
    i_sdmcEnable();
    
    /* SD初期化 */
    SDCARD_ErrStatus = sdmcReset();

    if(!SDCARD_ErrStatus) {
        SDCARD_ErrStatus = i_sdmcMPInit();
    }

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcReset

  Description:  reset SD card.
                リセット

  Arguments:    

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcReset( void)
{
    OSIntrMode irq_core_flag;
    ulSDCARD_Size   = 0;               /* カード全セクタ数クリア */
    SDCARD_MMCFlag  = FALSE;           /* MMCカード判定フラグクリア */
    SDCARD_SDHCFlag = FALSE;
    SDCARD_SDFlag   = FALSE;           /* SDカード判定フラグクリア */

    /*** カードステータスをクリア ***/
    SDCARD_ErrStatus = SDMC_NORMAL;
    SDCARD_Status    = SDMC_NORMAL;
    
    /*** カードCSD WPビットをクリア ***/
    SDCARD_WP_FLAG0 = 0;
    SDCARD_WP_FLAG1 = 0;
    SDCARD_WP_PERMANENT = 0;
    SDCARD_WP_TEMPORARY = 0;
    
    pSDCARD_info   = NULL;
    SDCARD_OutFlag = FALSE;            /* カード排出発生判定フラグクリア */
    
    irq_core_flag = OS_DisableInterrupts();    /* 割込み禁止 */

        *SDIF_CNT_L = (SDIF_CNT_FCLR | SDIF_CNT_USEFIFO); //ラッパーレジスタ
        *SDIF_CNT_L = 0x0000;
        *SDIF_FDS_L = 0;
        *SDIF_FSC_L = 1;
        SD_Init();                                 /* SD Card I/F 初期化処理 */
        SD_AndFPGA( SD_OPTION, SD_CD_DETECT_TIME); /* CD 検出タイムをゼロクリア */

        SD_port_en_numbers = SDCARD_PORT_NO_MAX; /*** サポートするポート数をデフォルトに設定 ***/
        SD_port_number = 0;                      /*** 現在のポート番号をデフォルトに設定      ***/

        SDCARD_Backup_port0();                   /* port0 backup    */
        SDCARD_Backup_port1();                   /* port1 backup    */
    
    OS_RestoreInterrupts( irq_core_flag);      /* 割り込み設定を元に戻す */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Layer_Init

  Description:  initialize sequence for SD card.
                SDカード規定の初期化手順

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARD_Layer_Init(void)
{
//    SYSTIM  wait_tim, limit_tim;

//    u16 memory_exist, function_number;
    SDCARD_Status = SDMC_NORMAL;    /* カードステータスをクリア */
/*    func_SDCARD_CallBack = NULL;    */
    pSDCARD_info = NULL;
    SDCARD_EndFlag = TRUE;          /* 転送処理完了フラグセット */
    SDCARD_MMCFlag = FALSE;         /* MMCカード判定フラグクリア */
    SDCARD_SDHCFlag = FALSE;
    SDCARD_SDFlag = FALSE;          /* SDカード判定フラグクリア */
    SDCARD_OutFlag = FALSE;         /* カード排出発生判定フラグクリア */
    ulSDCARD_Size = 0;              /* カード全セクタ数クリア */
//    init_io_exist = 0;
//    init_mem_exist = 0;

    SD_SetFPGA(SD_CLK_CTRL,(SD_CLK_CTRL_128)); /* SDクロックの周波数 261KHz(初期化時は100〜400khz) */
    SD_EnableClock();                          /* SDカードのクロックをイネーブルにする */

    /* SD I/F部ダミー80クロック(1mSec)転送待ち */
#if (TARGET_OS_CTR == 1)
    dly_tsk( 1);
#else
//    OS_Sleep( 1);
    SVC_WaitByLoop( 17900); //179*4サイクル=716サイクル=10024ns=10us
#endif
    
    SDCARD_ErrStatus = SDMC_NORMAL;            /* エラーステータスをクリア */

    SDCARD_TimerStop();                        /* タイムアウト判定用タイマストップ */

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RESET_TIMEOUT);   /* タイムアウト判定用タイマスタート */
#endif

    /***********************ここからSDIOテスト*************************/
//    SDIO_RWDirectCommand();
/*    while( 1) {
        SDIO_ReadOCR();
        if(SDCARD_ErrStatus){                       // エラーステータスの確認（エラー有り？）
            init_io_exist = 0;
            break;                                  //メモリ初期化へ
        }
        memory_exist = SD_OCR[1] & SD_OCR_MEMORY_PRESENT;
        function_number = (SD_OCR[1] & SD_OCR_NUM_OF_FUNC)>>12;
        if( function_number > 0) {                  // function number が 0でないか?
            if( (SD_OCR[0] == 0x0000)&&((SD_OCR[1]&SD_OCR_OCR_23_16)==0x0000)) {    //OCR invalidか?
                if( memory_exist) {                 //メモリが存在するか?
                    init_mem_exist = 1;
                    break;                          //メモリカード初期化へ
                }else{
                    init_mem_exist = 0;
                    return SDCARD_ErrStatus;        //エラー(OCR invalid & メモリなし)
                }
            }
            SDIO_WriteOCR();
            if(SDCARD_ErrStatus){                   // エラーステータスの確認（エラー有り？）
                init_io_exist = 0;
                return SDCARD_ErrStatus;
            }
            init_io_exist = 1;
            memory_exist = SD_OCR[1] & SD_OCR_MEMORY_PRESENT;
            if( memory_exist) {
                break;                //メモリ初期化へ
            }else{
                //IO初期化の続き(CMD3)へ
            }
        }
    }
*/
    /******************************************************************/

    SDCARD_TimerStop();                         /* タイムアウト判定用タイマストップ */

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RESET_TIMEOUT);    /* タイムアウト判定用タイマスタート */
#endif

    PRINTDEBUG( " CMD0(GO_IDLE_STATE)\n");
    SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));   /* タイムアウト以外のエラーをクリア */
    {
/*PRINTDEBUG( "SD_INFO1      : 0x%x\n", SD_INFO1);
PRINTDEBUG( "SD_INFO2      : 0x%x\n", SD_INFO2);
PRINTDEBUG( "SD_INFO1_MASK : 0x%x\n", SD_INFO1_MASK);
PRINTDEBUG( "SD_INFO2_MASK : 0x%x\n", SD_INFO2_MASK);
PRINTDEBUG( "SD_CLK_CTRL   : 0x%x\n", SD_CLK_CTRL);
PRINTDEBUG( "SD_SIZE       : 0x%x\n", SD_SIZE);

PRINTDEBUG( "SD_INFO1_MASK : 0x%x\n", (*(vu32 *)(SD_IP_BASE + 0x20)));*/
    }
    SD_Command(SD_CMD_CMD | GO_IDLE_STATE);     /* CMD0発行、レスポンス確認 */
    if(SDCARD_ErrStatus){                       /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }

    /*------- idle state -------*/
#if (TARGET_OS_CTR == 1)
    dly_tsk( 1);            /* 1ms待ち */
#else
//    OS_Sleep( 1);
    SVC_WaitByLoop( 17900); //179*4サイクル=716サイクル=10024ns=10us
#endif
    SD_SendIfCond();                            /* CMD8発行、レスポンス確認 */
    if( !SDCARD_SDHCFlag) {                     /* SDHC以外は失敗してるはずなので */
        SDCARD_ErrStatus = SDMC_NORMAL;         /* エラーフラグをクリアしておく */
    } //注:CMD8によりここで割り込みが入る！

    while(!(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT)){ /* タイムアウトになったら抜ける */
        SD_ClrErr((u16)(~SDMC_ERR_FPGA_TIMEOUT));

        SD_RCA = 0;                             /* RCA = 0をセット */
        if(!SDCARD_MMCFlag){                    /* MMCカードフラグが 0(OFF) か? */
            if(!SD_AppCommand()){               /* CMD55 発行処理が正常か? */
                SDCARD_MMCFlag = FALSE;         /* MMCカードフラグクリア */
                if(!SD_AppOpCond()){            /* ACMD41発行処理が正常か?(OCR31bit = L の時 No Response) */
                    SDCARD_SDFlag = TRUE;       /* SDカードフラグセット */
                    break;
                }
            }else{                                        /* CMD55 が正常終了しない */
                if(SDCARD_ErrStatus == SDMC_ERR_TIMEOUT){ /* タイムアウト(==No Response)か? */
                    SDCARD_MMCFlag = TRUE;                /* MMCカードフラグセット */
                }else{
//                    break;                //コメントアウトしないとSDカードの初期化に失敗する
                }
            }
        }
        if(SDCARD_MMCFlag){                     /* MMCカードフラグが 1(ON) のとき */
            SD_RCA = 1;                         /* RCA = 1をセット */
            if(!SD_SendOpCond()){               /* CMD1発行処理が正常か? */
                break;
            }
        }
    }
/*
    if( SDCARD_SDHCFlag) {
        SD_ReadOCR();
    }
*/    
    SDCARD_TimerStop();                               /* タイムアウト判定用タイマストップ */
    if(SDCARD_ErrStatus){                             /* エラーステータスの確認（エラー有り？）*/
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーかチェック */
            SD_ClrErr(SDMC_ERR_FPGA_TIMEOUT);         /* タイムアウトエラーの設定クリア */
            SD_SetErr(SDMC_ERR_RESET);                /* 初期化カードリセットコマンド時1.5秒タイムアウトエラーの設定 */
        }
        SDCARD_MMCFlag = FALSE;                       /* MMCカードフラグクリア */
        return SDCARD_ErrStatus;
    }

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_INITIAL_TIMEOUT); /* タイムアウト判定用タイマスタート */
#endif
    SD_SendCID();                            /* CMD2発行 レスポンス確認 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }
    while(1){
        SD_SendRelativeAddr();               /* CMD3発行 レスポンス確認 正常終了時 RCA<-ResのRCA */
        if(SDCARD_ErrStatus){                /* エラーステータスの確認（エラー有り？） */
            return SDCARD_ErrStatus;
        }
        if(SD_RCA != 0){
            break;
        }
    }
    /*------- standby state -------*/
    SD_SendCSD();                            /* CMD9発行 レスポンス確認 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }
    SDCARD_WP_PERMANENT = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_PERMANENT_BIT));
    SDCARD_WP_TEMPORARY = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_TEMPORARY_BIT));

    /* 転送速度設定 */
    SD_ClockDivSet(SD_RSP5);                 /* SDカードの動作クロック設定 (CSD[5]) */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }
    /* Command toggles acard between the Stand-by and Transfer states */
    SD_SelectCard();                         /* CMD7発行 レスポンス確認 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }

    /*------- translate state -------*/

    SDCARD_SectorSize = SECTOR_SIZE;         /* セクタサイズ デフォルト 512bytes */
    SD_SetBlockLength(SDCARD_SectorSize);    /* CMD16 ブロックサイズの設定 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？） */
        return SDCARD_ErrStatus;
    }

#if (SCR_ON == 1)
    SD_SelectBitWidth(FALSE);                /* CMD55->ACMD6 ビット幅の選択 1bit */

    /* ACMD51 発行 SD configuration register (SCR) */
    if(SDCARD_SDFlag){                       /* SDカードフラグ ON かチェック */
        SDCARD_TimerStop();                  /* タイムアウト判定用タイマストップ */
        i_sdmcSendSCR();
        if(SDCARD_ErrStatus){                /* エラーステータスの確認（エラー有り？） */
            return SDCARD_ErrStatus;
        }
#if TIMEOUT
        SDCARD_TimerStart(SDCARD_CLOCK_WAIT);/* タイムアウト判定用タイマスタート */
#endif
    }
#endif
    SD_EnableClock();                        /* SD-CLK Enable */

    if(SDCARD_MMCFlag){                      /* MMCカード ON かチェック */
        if( ((SD_CSD[7] & 0x3C)>>2) >= 4) {
            MMCP_SetBusWidth( TRUE);
        }
//        SD_SelectBitWidth(FALSE);            /* CMD55->ACMD6 ビット幅の選択 1bit */
    }else{
        SD_SelectBitWidth(TRUE);             /* CMD55->ACMD6 ビット幅の選択 4bit */
    }

    SDCARD_TimerStop();                      /* タイムアウト判定用タイマストップ */

    if(SDCARD_SDFlag){                       /* SDカードフラグ ON かチェック */
        if (SDCARD_SD_Status())              /* CMD55->ACMD13 カードステータスを取得 */
            return SDCARD_ErrStatus;
        if(SD_SDSTATUS[1] & SD_MEMORY_CARD){
            SDCARD_SDFlag = FALSE;           /* SDカードフラグクリア */
        }
    }

    i_sdmcCalcSize();

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         i_sdmcCalcSize

  Description:  

  Arguments:    

  Returns:      None
 *---------------------------------------------------------------------------*/
void i_sdmcCalcSize( void)
{
    u32     ulCSize;
    u16     read_block_len_val, mult_val;
  
    /*--------------カードサイズの算出---------------*/
    if( ((SD_CSD[7] & CSD_STRUCT_BIT_127_126) >> 6) == 0x1) {    //SDHCのとき
        sdmc_current_spec.csd_ver2_flag = 1;
        ulCSize = (u32)((((u32)(SD_CSD[3]) & CSD_C_SIZE_BIT_69_56) << 8) +
                    ((SD_CSD[2] & CSD_C_SIZE_BIT_55_48) >> 8) + 1);
        ulCSize = ulCSize * 1024;    //もともと512KByte単位なのを512Byte単位にする
        /* データ領域サイズ算出 */
        sdmc_current_spec.memory_capacity = ulCSize;
        ulSDCARD_Size = ulCSize;
        /* プロテクト領域サイズ算出 */
        sdmc_current_spec.protected_capacity = (u32)((((SD_SwapByte( &SD_SDSTATUS[2])) << 16) +
                                            (SD_SwapByte( &SD_SDSTATUS[3]))) / 0x200);
        /*トータルサイズ算出 */
        sdmc_current_spec.card_capacity = sdmc_current_spec.memory_capacity +
                                            sdmc_current_spec.protected_capacity;
        
    }else{                                                      //従来SDカードのとき
        sdmc_current_spec.csd_ver2_flag = 0;
        ulCSize = (u32)(((SD_CSD[3] & CSD_C_SIZE_BIT_71_62) >> 6) + 
                    ((SD_CSD[4] & CSD_C_SIZE_BIT_73_72) << 10) + 1);
        mult_val = (u16)(((SD_CSD[2] & CSD_C_SIZE_MULT) >> 7) + 2);    //2の乗数
        ulCSize = ulCSize << mult_val;
        if(SDCARD_MMCFlag){                        /* MMCカードフラグON かチェック */
            read_block_len_val = (u16)((SD_CSD[4] & CSD_READ_BL_LEN) >> 8);
            ulCSize = (ulCSize << read_block_len_val);
        }else{                                    /* SDカードフラグ(SDCARD_SDFlag)ON のはず */
            read_block_len_val = (u16)(((SD_CSD[1] & CSD_WRITE_BL_LEN_BIT_25_24) << 2) |
                        ((SD_CSD[0] & CSD_WRITE_BL_LEN_BIT_23_22) >> 14));
            ulCSize = (ulCSize << read_block_len_val);
        }
        /* データ領域サイズ算出 */
        ulCSize /= SDCARD_SectorSize;            /* 全セクタ数の算出 */
        sdmc_current_spec.memory_capacity = ulCSize;
        ulSDCARD_Size += ulCSize;                /* 全セクタ数のセット */
        /* プロテクト領域サイズ算出 */
        sdmc_current_spec.protected_capacity = (u32)((SD_SwapByte( &SD_SDSTATUS[2])) << 16) +
                                                     (SD_SwapByte( &SD_SDSTATUS[3]));
        sdmc_current_spec.protected_capacity <<= mult_val;
        sdmc_current_spec.protected_capacity <<= read_block_len_val;
        sdmc_current_spec.protected_capacity /= SDCARD_SectorSize;    //TODO:構造体にまとめること
        /*トータルサイズ算出 */
        sdmc_current_spec.card_capacity = sdmc_current_spec.memory_capacity +
                                            sdmc_current_spec.protected_capacity;
    }
    sdmc_current_spec.SS = SDCARD_SectorSize;

    PRINTDEBUG( "SD memory capacity : 0x%x\n", sdmc_current_spec.memory_capacity);
    PRINTDEBUG( "SD protected capacity : 0x%x\n", sdmc_current_spec.protected_capacity);
    PRINTDEBUG( "SD total capacity : 0x%x\n", sdmc_current_spec.card_capacity);
}

/*---------------------------------------------------------------------------*
  Name:         i_sdmcMPInit

  Description:  initialize SD card in multi ports.
                マルチポートのSDカード初期化

  Arguments:    

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE i_sdmcMPInit( void)
{
    /*ポート選択状態、挿抜状態
    SD_port_number, SD_CheckFPGAReg(SD_INFO1,SD_INFO1_DETECT),
    SD_CheckFPGAReg(EXT_CD,EXT_CD_PORT1_DETECT)
    */
  
    if( SD_CheckFPGAReg( SD_INFO1, SD_INFO1_DETECT)) {
        sdmcSelect( (u16)SDMC_PORT_CARD);
        SDCARD_ErrStatus = SDCARD_Layer_Init();
    }
    if( SD_CheckFPGAReg( EXT_CD, EXT_CD_PORT1_DETECT)) {
        sdmcSelect( (u16)SDMC_PORT_NAND);
        SDCARD_ErrStatus = SDCARD_Layer_Init();
    }
    SDCARD_OutFlag = FALSE;     /* 排出フラグをリセット */

    SDCARD_TimerStop();         /* タイムアウト判定用タイマストップ */
    SD_DisableClock();          /* SD-CLK Disable */
    SD_EnableInfo();            /* SD Card  挿抜 割り込み許可 */
    
    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         sdmcGetStatus

  Description:  get card status
                カードの状態を取得する
                bit15 SDカード判別ビット(検出したら1)
                bit14 MMCカード判別ビット(検出したら1)
                bit10 IO3 card detect(検出したら1)     ※CTRではプルアップのため使えない
                bit9 IO3 card inserted(挿入動作で1) ※CTRではプルアップのため使えない
                bit8 IO3 card removed(脱動作で1)     ※CTRではプルアップのため使えない
                bit7 write protect(書き込み禁止の場合1)
                bit5 card detect(検出したら1)
                bit4 card inserted(挿入動作で1)
                bit3 card removed(脱動作で1)
                bit2 R/W access all end
                bit0 Response end

  Arguments:    *status : カードの状態を格納する変数へのポインタ

  Returns:      0 : success
                > 0 : error code
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------
SD_INFO1レジスタ
bit[10,9,8,7] = DAT3CD, DAT3IN, DAT3OUT, WP
bit[5, 4, 3, 2] = CD, INS, REM, ALLEND
bit0 = RESEND
-------------------------------------------------
EXT_CDレジスタ
bit[2,1,0] = P1CD, P1INS, P1REM
-------------------------------------------------
EXT_CD_DAT3レジスタ
bit[2, 1, 0] = P1DCD, P1DINS, P1DREM
-------------------------------------------------
EXT_WPレジスタ
bit0 = P1WP
-----------------------------------------------*/
SDMC_ERR_CODE sdmcGetStatus(u16 *status)
{
    u16 SD_INFO1_STATUS;
    
    SD_INFO1_STATUS = SD_INFO1;         /* SD_INFO1レジスタ読み出し */
    *status = SD_INFO1_STATUS;          /* 論理反転 */

    /*--- ポート0のとき ---*/
    if(SD_port_number == SDCARD_PORT0)
    {
        *status &= SDCARD_FLAG_CLR;     /* SD/MMCフラグクリア */
    }
    /*--- ポート1のとき ---*/
    else if (SD_port_number == SDCARD_PORT1)
    {
        *status &= SDCARD_PORT1_CLR;    /* port1に関係ない部分をクリア */
        SD_INFO1_STATUS = (u16)((EXT_CD & 0x0007) << 3);
        SD_INFO1_STATUS |= ((EXT_CD_DAT3 & 0x0007) << 8);
        SD_INFO1_STATUS |= ((EXT_WP & 0x0001) << 7);
        *status |= SD_INFO1_STATUS;     /* カードport1フラグ設定   */
    }
    /*--- SD/MMCフラグをセット ---*/
    if( SDCARD_MMCFlag) {               /* 検出したカードがMMCカードの時 */
        *status |= SDCARD_FLAG_MMC;     /* カード判定部分MMCカード */
    }
    if( SDCARD_SDFlag) {                /* 検出したカードがSDカードの時 */
        *status |= SDCARD_FLAG_SD;      /* カード判定部分SDカード */
    }

    return SDMC_NORMAL;
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_GetR1Status

  Description:  get the card status of R1 response.
                R1レスポンスのカードステータスを取得する。

  Arguments:    None

  Returns:      SDCARD_Status : R1の[39:8]
 *---------------------------------------------------------------------------*/
static u32 SDCARD_GetR1Status(void)
{
    /* SD_CheckStatusでSDCARD_Statusに値が入る */
    return SDCARD_Status;
}


/*---------------------------------------------------------------------------*
  Name:         sdmcReadFifo

  Description:  read from card.
                ラッパーのFIFOを使用してカードからの読み出し。

  Arguments:    buf : 読み出したデータを格納するためのバッファのアドレス
                bufsize : 読み出しサイズ（セクタ数）
                offset : 読み出し開始オフセット（セクタ番号）
                info : 実行結果を格納するための構造体へのアドレス

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info)
{
    SDCARDMsg     SdMsg;
    OSMessage     recv_dat;
    SDMC_ERR_CODE api_result;                    //SDCARD関数の返り値

    if( bufsize == 0) {
       return( SDMC_ERR_PARAM);
    }
  
    SdMsg.buf       = buf;
    SdMsg.bufsize   = bufsize;
    SdMsg.offset    = offset;
    SdMsg.func      = func;
    SdMsg.info      = info;
    SdMsg.operation = SD_OPERATION_READ_WITH_FIFO;

    recv_dat = (OSMessage)&SdMsg; //SdMsgのアドレスを伝える
    OS_SendMessage( &sdmc_dtq, recv_dat, OS_MESSAGE_BLOCK);
    
    /* 返り値待ち */
    OS_ReceiveMessage( &sdmc_result_dtq, &recv_dat, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)recv_dat;

    return api_result;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARDi_ReadFifo

  Description:  read from card.
                ラッパーのFIFOを使用してカードからの読み出し。

  Arguments:    buf : 読み出したデータを格納するためのバッファのアドレス
                bufsize : 読み出しサイズ（セクタ数）
                offset : 読み出し開始オフセット（セクタ番号）
                info : 実行結果を格納するための構造体へのアドレス

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARDi_ReadFifo(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info)
{
    u32* DmaReg;
    SDMC_ERR_CODE result;
    
    if( sdmc_dma_no == SDMC_NOUSE_DMA) {
        /* FIFO Empty割り込み無効、FIFO Full割り込み有効 */
        *(SDIF_CNT) = (*(SDIF_CNT) & (~SDIF_CNT_FEIE)) | SDIF_CNT_FFIE;
    }else{
        *(SDIF_CNT) = (*(SDIF_CNT) & (~(SDIF_CNT_FFIE | SDIF_CNT_FEIE)));
    }
    *(SDIF_FDS) = (u16)SDCARD_SectorSize;   /* FIFOのデータサイズ */
    *(SDIF_FSC) = bufsize;
    *(SDIF_CNT) |= (SDIF_CNT_FCLR | SDIF_CNT_USEFIFO);        /* FIFO使用フラグON */
    CC_EXT_MODE = CC_EXT_MODE_DMA;          /* DMAモードON */

    if( sdmc_dma_no != SDMC_NOUSE_DMA) {
        DmaReg = (u32*)(REG_DMA4SAD_ADDR + (0x1C * sdmc_dma_no));
        *(u32*)(DmaReg+6) =  0;            //reg_MI_DMA4CNT
        *(u32*)(DmaReg++) =  (u32)SDIF_FI; //reg_MI_DMA4SAD
        *(u32*)(DmaReg++) =  (u32)buf;     //reg_MI_DMA4DAD
        *(u32*)(DmaReg++) = ((SDCARD_SectorSize / sizeof(u32)) * bufsize); //総転送ワード数(reg_MI_DMA4TCNT)
        *(u32*)(DmaReg++) = (SDCARD_SectorSize / sizeof(u32));             //転送ワード数(reg_MI_DMA4WCNT)
        *(u32*)(DmaReg++) = 0;                                             //インターバル(reg_MI_DMA4BCNT)
        *(u32*)(DmaReg+1) = REG_MI_DMA4CNT_E_MASK | 0x08000000 | 0x00070000 | 0x4000; //SD起動、128WC, SRC-FIX(reg_MI_DMA4CNT)
    }

    result = SDCARDi_Read( buf, bufsize, offset, func, info);

    /* FIFO無効に */
    *(SDIF_CNT) &= (~SDIF_CNT_USEFIFO);     /* FIFO使用フラグOFF */
    CC_EXT_MODE = CC_EXT_MODE_PIO;          /* PIOモード(DMAモードOFF) */
    
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcRead

  Description:  read from card.
                カードからの読み出し。

  Arguments:    buf : 読み出したデータを格納するためのバッファのアドレス
                bufsize : 読み出しサイズ（セクタ数）
                offset : 読み出し開始オフセット（セクタ番号）
                info : 実行結果を格納するための構造体へのアドレス

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcRead(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info)
{
    SDCARDMsg     SdMsg;
    OSMessage     recv_dat;
    SDMC_ERR_CODE api_result;

    if( bufsize == 0) {
       return( SDMC_ERR_PARAM);
    }
  
    SdMsg.buf       = buf;
    SdMsg.bufsize   = bufsize;
    SdMsg.offset    = offset;
    SdMsg.func      = func;
    SdMsg.info      = info;
    SdMsg.operation = SD_OPERATION_READ;

    recv_dat = (OSMessage)&SdMsg;
    OS_SendMessage( &sdmc_dtq, recv_dat, OS_MESSAGE_BLOCK);
    
    /* 返り値待ち */
    OS_ReceiveMessage( &sdmc_result_dtq, &recv_dat, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)recv_dat;

    return api_result;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARDi_Read

  Description:  read from card.
                カードからの読み出し。

  Arguments:    buf : 読み出したデータを格納するためのバッファのアドレス
                bufsize : 読み出しサイズ（セクタ数）
                offset : 読み出し開始オフセット（セクタ番号）
                info : 実行結果を格納するための構造体へのアドレス

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARDi_Read(void* buf,u32 bufsize,u32 offset,void(*func)(void),SdmcResultInfo *info)
{
    s16           nRetryCount;                  /* リトライ回数カウント */
    SDMC_ERR_CODE SaveErrStatus;                /* エラーステータス保存用 */
    u32           SaveStatus;                   /* カードステータス保存用 */
#pragma unused( func)

    for( nRetryCount=0; nRetryCount<SDCARD_RETRY_COUNT; nRetryCount++) {

        SD_EnableClock();                       /* クロック供給 */

    /*    func_SDCARD_CallBack = func;    */
        pSDCARD_info = info;                    /* 保存用実行結果構造体のアドレスを設定 */
        ulSDCARD_SectorCount = bufsize;         /* 転送セクタ数の設定 */
        ulSDCARD_RestSectorCount = bufsize;     /* 残り転送セクタ数の設定 */
        pSDCARD_BufferAddr = buf;               /* データ格納バッファのアドレスを設定 */

        SDCARD_EndFlag = FALSE;                 /* 転送処理完了フラグクリア */
        SDCARD_ErrStatus = SDMC_NORMAL;         /* エラーステータスのクリア */

#if TIMEOUT
        SDCARD_TimerStart(SDCARD_RW_TIMEOUT);   /* タイムアウト判定用タイマスタート(4000msec) */
#endif
        /* IPのSD_SECCNTレジスタ有効化、転送セクタ数設定(自動CMD12発行のため) */
        SD_EnableSeccnt( ulSDCARD_RestSectorCount);

        thread_flag = TRUE;
        /*--- リードコマンド発行 ---*/
        if( SDCARD_SDHCFlag) {
            SD_MultiReadBlock( offset);         /* 引数：offset(SDHCはセクタサイズ512Byte固定) */
        }else{
            SD_MultiReadBlock( offset * SDCARD_SectorSize); /* 引数：offset × セクタサイズ */
        }
        /*--------------------------*/

        /**/
        PRINTDEBUG( "==Slp Tsk==\n");
#if (TARGET_OS_CTR == 1)
        //can_wup( 0);
        slp_tsk();
#else
        /*--------------------*/
//        OS_TPrintf( "sleep %d\n", __LINE__);
//        OS_SleepThread( NULL);
        /*--------------------*/
#endif
        PRINTDEBUG( "waked\n");

        thread_flag = FALSE;
        while( !SDCARD_EndFlag) {                          /* SDカード R/W アクセス終了フラグの ON（終了）待ち */
            if( SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか? */
                return SDCARD_ErrStatus;                   /* エラー終了 */
            }
        }
        /* エラーが発生していないか、タイムアウト以外のエラーの場合 */
        if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){      /* コマンドレスポンス(R1)のカードステータスがエラーでないか? */
            SD_CheckStatus(TRUE);                          /* コマンドレスポンス(R1)の Card Status チェック */
            if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){  /* コマンドレスポンス(R1)のカードステータスがエラーでないか? */
                SD_SendStatus();                           /* カードステータスの取得コマンド発行 */
                SD_CheckStatus(TRUE);                      /* コマンドレスポンス(R1)の Card Status チェック */
            }
        }
        SaveStatus = SDCARD_Status;                   /* カードステータスの保存 */
        SaveErrStatus = SDCARD_ErrStatus;             /* エラーステータスの保存 */

        if( SDCARD_ErrStatus) {                       /* エラーステータスの確認(エラー有り?) */
            i_sdmcErrProcess();                       /* エラー時の処理(status取得、強制停止) */
        }
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか確認 */
            return SDCARD_ErrStatus;                  /* エラー終了 */
        }
        SDCARD_Status = SaveStatus;                   /* カードステータスの復帰 */
        SDCARD_ErrStatus = SaveErrStatus;             /* エラーステータスの復帰 */


        if(SDCARD_ErrStatus == SDMC_NORMAL){          /* エラーステータスの確認（エラー無し？）*/
            break;                                    /* リトライループから抜ける */
        }
    }

    SD_DisableClock();                                /* クロック供給停止 */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_FPGA_irq

  Description:  read/write between Card, when BRE or BWE interrupt.
                カードからBRE(リード要求)またはBWE(ライト要求)割り込みが発生
                したとき、実際にリードまたはライトを行う。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_FPGA_irq(void)
{
    BOOL    bRead;

    /* FIFOを使うとき：FIFO割り込み許可状態を見て、ReadかWriteか判別する*/
    /* FIFOを使わないとき：BREかBWEかを調べ、INFO2の当該割り込み要求フラグをクリアする */
    bRead = SD_FPGA_irq();

        /*--- SDカードからのリード時 ---*/
        if(bRead){
            if( ulSDCARD_RestSectorCount > 0) {
//              if( ulSDCARD_RestSectorCount == 1) { TTT();}
                if( SDCARD_UseFifoFlag) {                            /*--- FIFOを使うとき ---*/
                    MyCpuRecv32( (u32*)SDIF_FI, (u32*)pSDCARD_BufferAddr, SDCARD_SectorSize);
                }else{                                               /*--- FIFOを使わないとき ---*/
                    MyCpuRecv16( (u16*)SD_BUF0, pSDCARD_BufferAddr, SDCARD_SectorSize);
                }
                ulSDCARD_RestSectorCount--;                 /* 残りセクタ数をデクリメント */
            }
            if( ulSDCARD_RestSectorCount <= 0) {                 /* 要求セクタ数リード完了したか? */
                if(SD_CheckFPGAReg(SD_STOP,SD_STOP_SEC_ENABLE)){ /* SD_SECCNTレジスタがEnableか? */
//                    SD_DisableSeccnt();                        /* SD_SECCNTレジスタ無効設定 */
                }else{                                           /* SD_SECCNTレジスタがDisableのとき */
                    SD_StopTransmission();                       /* カード転送終了をFPGAに通知（CMD12発行） */
                }
            }
        }else{    /*--- SDカードへのライト時 ---*/
            if( ulSDCARD_RestSectorCount > 0) {
              if( ulSDCARD_RestSectorCount == 1) { SDCARD_ReadyToEnd();}
                if( SDCARD_UseFifoFlag) {                            /*--- FIFOを使うとき ---*/
                    MI_CpuSend32( pSDCARD_BufferAddr, SDIF_FI, SDCARD_SectorSize);
                }else{                                               /*--- FIFOを使わないとき ---*/
                    MI_CpuSend16( pSDCARD_BufferAddr, SD_BUF0, SDCARD_SectorSize);
                }
                ulSDCARD_RestSectorCount--;                 /* 残りセクタ数をデクリメント */
             }
            if( ulSDCARD_RestSectorCount <= 0){                  /* 要求セクタ数ライト完了? */
                if( SDCARD_UseFifoFlag) {                        /* FIFOを使用するときは */
                    while( (*SDIF_CNT & SDIF_CNT_NEMP)) {};      /* FIFOにデータが残っているうちは終了しない */
                }
                if(SD_CheckFPGAReg(SD_STOP,SD_STOP_SEC_ENABLE)){ /* SD_SECCNTレジスタがEnableか? */
//                    SD_DisableSeccnt();                        /* SD_SECCNTレジスタ無効設定 */
                }else{                                           /* SD_SECCNTレジスタがDisableのとき */
                    SD_StopTransmission();                       /* カード転送終了をFPGAに通知（CMD12発行） */
                }
            }
        }
        /* リード/ライト共通処理 */
        SDCARD_TimerStart(SDCARD_RW_TIMEOUT);   /* タイムアウト判定用タイマスタート(4000msec) */
        pSDCARD_BufferAddr+=(512/2);            /* 書込みデータのバッファアドレスをインクリメント */
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_ReadyToEnd

  Description:  
                カードとの最終セクタ転送前に行う処理。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_ReadyToEnd( void)
{
        *(SDIF_CNT) = (*(SDIF_CNT) & (~(SDIF_CNT_FFIE | SDIF_CNT_FEIE)));
        SD_OrFPGA( SD_INFO2_MASK, SD_INFO2_MASK_BWE);   /* SDカードからのデータ書込み要求割込み禁止 */
        SD_OrFPGA( SD_INFO2_MASK, SD_INFO2_MASK_BRE);   /* SDカードからのデータ書込み要求割込み禁止 */
        SDCARD_TimerStop();             /* タイムアウト判定用タイマストップ */
}

/*---------------------------------------------------------------------------*
  Name:         SYSFPGA_irq

  Description:  insert/remove/error/access end interrupt handler.
                BREやBWE割り込みを除く割り込みのハンドラ。挿抜、エラー発生、
                アクセス終了の割り込み発生時にそれぞれの処理を行う。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SYSFPGA_irq(void)
{
    /*--- ポート0の挿抜割り込みチェックとコールバック起動 ---*/
    if(!SD_CheckFPGAReg( SD_INFO1_MASK, SD_INFO1_MASK_REMOVE)){ /* SD Card 抜け 割込み許可状態か? */
        if( SD_CheckFPGAReg( SD_INFO1, SD_INFO1_REMOVE)){       /* SD Card 抜け 発生か? */
            SD_AndFPGA( SD_INFO1,(~SD_INFO1_REMOVE));           /* INFO1の抜けフラグを落とす */
            SDCARD_OutFlag = TRUE;                              /* 排出フラグセット */
            if(func_SDCARD_Out){                                /* コールバック関数のNullチェック */
                SDCARD_IO_Port = SDCARD_PORT0;                  /* カード抜けポート番号を設定 */
                func_SDCARD_Out();                              /* カード抜けコールバック関数呼び出し */
            }
        }
    }
    if(!SD_CheckFPGAReg( SD_INFO1_MASK, SD_INFO1_MASK_INSERT)){ /* SD Card 挿入 割込み許可状態か? */
        if( SD_CheckFPGAReg( SD_INFO1, SD_INFO1_INSERT)){       /* SD Card 挿入 発生か? */
            SD_AndFPGA( SD_INFO1, (~SD_INFO1_INSERT));          /* INFO1の挿入フラグを落とす */
            SDCARD_OutFlag = FALSE;                             /* 排出フラグリセット */
            if(func_SDCARD_In){                                 /* コールバック関数のNullチェック */
                SDCARD_IO_Port = SDCARD_PORT0;                  /* カード挿入ポート番号を設定 */
                func_SDCARD_In();                               /* カード挿入コールバック関数呼び出し */
            }
        }
    }
    /* (CTRはポート1のCD端子が未接続(常に挿入状態)なので、ポート1の挿抜チェックは行わない) */

    /*--- 割り込み要求と割り込みマスクを保存 ---*/
    SD_GetFPGA( SD_INFO1_VALUE,         SD_INFO1);
    SD_GetFPGA( SD_INFO1_MASK_VALUE,    SD_INFO1_MASK);
    SD_GetFPGA( SD_INFO2_VALUE,         SD_INFO2);
    SD_GetFPGA( SD_INFO2_MASK_VALUE,    SD_INFO2_MASK);
    /*------------------------------------------*/

    /*--- SD_INFO2のエラーフラグ作成 ---*/
    SD_INFO_ERROR_VALUE = (u16)(SD_INFO2_VALUE & (~SD_INFO2_MASK_VALUE));
    /*--- エラーステータス作成 (RESTIMEOUTとILAエラーのフラグは反映しない) ---*/
    SDCARD_ErrStatus |= SD_INFO_ERROR_VALUE & (~(SD_INFO2_ERR_RESTIMEOUT)) &
                        (~(SD_INFO2_ERR_ILA)) & SD_INFO2_MASK_ERRSET;

    /*--- RESTIMEOUTとILAエラーはフラグの位置をずらして反映する ---*/
    if( SD_INFO_ERROR_VALUE & SD_INFO2_ERR_ILA) {
        SDCARD_ErrStatus |= SDMC_ERR_ILA;                       /* イリーガルアクセスエラー発生 */
    }
    if( SD_INFO_ERROR_VALUE & SD_INFO2_ERR_RESTIMEOUT) {
        SDCARD_ErrStatus |= SDMC_ERR_TIMEOUT;                   /* Response Time out エラー発生 */
    }/*------------------------------------------------------------*/
    
    SD_AndFPGA( SD_INFO2,(~(SD_INFO2_ERROR_SET)));              /* SD_INFO2のエラーフラグを全て落とす */
    if ( SDCARD_ErrStatus) {                                    /* 何らかのエラーが発生しているか? */
        SD_OrFPGA( SD_INFO2_MASK, SD_INFO2_MASK_ERRSET);        /* 全てのエラー割り込みを禁止 */
    }
    /*--- SD_INFO2のエラーチェック終了 ---*/
    
    
    /*--- SD_INFO1の割り込み発生状況フラグ作成 ---*/
    SD_INFO_ERROR_VALUE = (u16)(SD_INFO1_VALUE & (~SD_INFO1_MASK_VALUE));

    if( SD_INFO_ERROR_VALUE & SD_INFO1_MASK_ALL_END) {          /* R/W access all end 割込み発生か? */
        SD_OrFPGA( SD_INFO1_MASK, SD_INFO1_MASK_ALL_END);       /* INFO1の access all end 割込み禁止 */

//        OS_TPrintf( "SYS\n");
            SDCARD_TimerStop();                                 /* タイムアウト判定用タイマストップ */
            SD_TransEndFPGA();                                  /* 転送終了処理(割り込みマスクを禁止に戻す) */
            if( SDCARD_EndFlag == FALSE) {                      /* 転送が終了していないか? */
                SDCARD_EndFlag = TRUE;                          /* 転送処理完了フラグセット */
                if( pSDCARD_info) {                             /* Nullチェック */
                    pSDCARD_info->result = SDCARD_ErrStatus;    /* SdmcResultInfo に情報設定 */
                    pSDCARD_info->resid = (ulSDCARD_SectorCount - ulSDCARD_RestSectorCount) *
                                            SDCARD_SectorSize;  /* SdmcResultInfo に処理セクタ数設定 */
                }
            }    /* 転送が終了済みのとき */

            return;

        if( SDCARD_ErrStatus != SDMC_NORMAL) {                  /* エラーが発生している場合 */
            SDCARD_TimerStop();                                 /* タイムアウト判定用タイマストップ */
            if( SDCARD_EndFlag == FALSE) {                      /* 転送が終了していないか? */
                SD_TransEndFPGA();                              /* カード転送の終了処理 */
                SDCARD_EndFlag = TRUE;                          /* 転送処理完了フラグセット */
            /*    SD_Init();    */                              /* SD Cardインターフェース部をリセット&初期設定 */
                if( pSDCARD_info) {                             /* Nullチェック */
                    pSDCARD_info->result = SDCARD_ErrStatus;    /* SdmcResultInfo に情報設定 */
                    pSDCARD_info->resid = (ulSDCARD_SectorCount - ulSDCARD_RestSectorCount) *
                                            SDCARD_SectorSize;  /* SdmcResultInfo に処理セクタ数設定 */
                }
            }
        }
    }                                                           /* R/W access all end 割り込み未発生 */
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_TimerStart

  Description:  start timer for measure timeout.
                タイムアウト計測を開始する

  Arguments:    tim : ms単位のタイムアウト時間
                (50msを超える値の場合は50ms単位になる)

  Returns:      None
 *---------------------------------------------------------------------------*/
static    void    SDCARD_TimerStart(u32 tim)
{
#if (TARGET_OS_CTR == 1)
    sta_alm( sdmc_alm_id, tim);
    PRINTDEBUG( "Timer Started.\n");
#else
    OSTick tim_tick;
    
    tim_tick = OS_MilliSecondsToTicks( tim);        //us単位からTick単位へ
    
    OS_CancelAlarm( &sdmc_alm);                     //アラーム破棄
    OS_SetAlarm( &sdmc_alm, tim_tick, SDCARD_Timer_irq, NULL);   //アラームセット
#endif
} 

/*---------------------------------------------------------------------------*
  Name:         SDCARD_TimerStop

  Description:  stop timer
                タイムアウト計測を停止する

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static    void    SDCARD_TimerStop(void)
{
#if (TARGET_OS_CTR == 1)
    stp_alm( sdmc_alm_id);
    PRINTDEBUG( "Timer Stopped.\n");

    //memo:割り込み禁止状態にしてstp_almを呼んでも停止しないので注意
#else
    OS_DisableIrq();
    OS_CancelAlarm( &sdmc_alm);
    OS_EnableIrq();
#endif
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Timer_irq

  Description:  timer interrupt handler.
                タイマー割り込みハンドラ

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void    SDCARD_Timer_irq(void* arg)
{
#pragma unused( arg)

#if (SD_DEBUG_PRINT_ON == 1)
    u16 tmp;
    
    PRINTDEBUG( ">>>Timer intr(Timeout)\n");
    tmp = SD_INFO1;
    PRINTDEBUG( "SD_INFO1      : 0x%x\n", tmp);
    tmp = SD_INFO1_MASK;
    PRINTDEBUG( "SD_INFO1_MASK : 0x%x\n", tmp);
    tmp = SD_INFO2;
    PRINTDEBUG( "SD_INFO2      : 0x%x\n", tmp);
    tmp = SD_INFO2_MASK;
    PRINTDEBUG( "SD_INFO2_MASK : 0x%x\n", tmp);
    tmp = SD_ERR_STS1;
    PRINTDEBUG( "SD_ERR_STS1   : 0x%x\n", tmp);
    tmp = SD_ERR_STS2;
    PRINTDEBUG( "SD_ERR_STS2   : 0x%x\n", tmp);
    tmp = *(vu16 *)(SD_IF_BASE+0x00);
    PRINTDEBUG( "SD_CNT        : 0x%x\n", tmp);
    tmp = SD_SECCNT;
    PRINTDEBUG( "SD_SECCNT     : 0x%x\n", tmp);
#endif
  
    SDCARD_ErrStatus |= SDMC_ERR_FPGA_TIMEOUT;    /* タイムアウトエラービットの設定 */

    if(SDCARD_EndFlag == FALSE){                    /* 転送処理完了フラグの確認（クリア？）*/
        SDCARD_EndFlag = TRUE;                      /* 転送処理完了フラグをセット */
    /*    SD_TransEndFPGA();    */                  /* カード転送の終了処理 */
    /*    SD_StopTransmission();    */              /* カード転送終了設定 */
    /*    SD_Init();    */                          /* SD Cardインターフェース部をリセット&初期設定 */
        if(pSDCARD_info){                           /* Nullチェック */
            pSDCARD_info->result = SDCARD_ErrStatus;/* SdmcResultInfo に情報設定 */
            pSDCARD_info->resid = (ulSDCARD_SectorCount - ulSDCARD_RestSectorCount) *
                SDCARD_SectorSize;                  /* SdmcResultInfo に処理セクタ数設定 */
        }
    }

    /**/
    PRINTDEBUG( "--Wup sdTsk(Time)--\n");
#if (TARGET_OS_CTR == 1)
    iwup_tsk( sdmc_tsk_id);
    PRINTDEBUG( "id : 0x%x\n", sdmc_tsk_id);
#else
    sdmc_wakeup_count++;
    PRINTDEBUG( "wakeup\n");
    OS_WakeupThreadDirect( &sdmc_tsk);
#endif
}


/*---------------------------------------------------------------------------*
  Name:         i_sdmcSendSCR

  Description:  get SCR register.
                SCRを取得する（DATライン経由で8バイト送られてくる）。
                MultiBlock R/W と異なり、DATライン経由で転送されてくるSDカードの
                レジスタは、MSBから先に送られてくることに注意。
                (Physical Layer Specification 2.00 p12-13参照)

  Arguments:    None

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static u16 i_sdmcSendSCR(void)
{
    SDMC_ERR_CODE SaveErrStatus;          /* エラーステータス保存用 */
    u32           SaveStatus;             /* カードステータス保存用 */
    u32           ulSave_SectorSize;      /* セクタサイズ保存用 */

    SD_EnableClock();                        /* SD-CLK Enable */

    /*    ブロックサイズの設定 */
    ulSave_SectorSize = SDCARD_SectorSize;   /* セクタサイズの保存 */
    SDCARD_SectorSize = 8;                   /* SCR レジスタ 転送サイズ 8bytes */
    SD_SetBlockLength(SDCARD_SectorSize);    /* SDカードデータ転送サイズ 8byte 設定 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？）*/
        return SDCARD_ErrStatus;
    }

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RW_TIMEOUT);    /* タイムアウト判定用タイマスタート */
#endif

    if(SD_AppCommand()){                  /* RCA設定後 CMD55発行処理が正常終了しない? */
        SD_DisableClock();                /* SD-CLK Disable */
        return SDCARD_ErrStatus;          /* エラー終了 */
    }

/*    func_SDCARD_CallBack = NULL;    */
    pSDCARD_info = NULL;
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1; /* 残りセクタサイズ、セクタカウントに１を設定 */
    pSDCARD_BufferAddr = SD_SCR;                         /* データ格納バッファのアドレスを設定 */

    /* 転送前の準備処理 */
    SDCARD_EndFlag = FALSE;                  /* 転送処理完了フラグクリア */
    SDCARD_ErrStatus = SDMC_NORMAL;          /* エラーステータスのクリア */

#if (SCR_ON == 1)
    thread_flag = TRUE;
    SD_SendSCR();                            /*    SCRの取得コマンド発行 */
    PRINTDEBUG( "==Slp Tsk==\n");
#if (TARGET_OS_CTR == 1)
    //can_wup( 0);
    slp_tsk();
#else
    /*--------------------*/
//        OS_TPrintf( "sleep %d\n", __LINE__);
//        OS_SleepThread( NULL);
    /*--------------------*/
#endif
    PRINTDEBUG( "waked\n");
    thread_flag = FALSE;

    while(!SDCARD_EndFlag){                            /* カードアクセス終了待ち */
      PRINTDEBUG( "k\n");
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){  /* タイムアウトエラーか確認 */
            return SDCARD_ErrStatus;
        }
    }

    if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){     /* コマンドレスポンス(R1)のカードステータスがエラーでないか確認 */
        SD_CheckStatus(FALSE);                        /* コマンドレスポンス(R1)の Card Status チェック */
        if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){ /* コマンドレスポンス(R1)のカードステータスがエラーでないか確認 */
            SD_SendStatus();                      /* カードステータスの取得コマンド発行処理 */
            SD_CheckStatus(FALSE);                /* コマンドレスポンス(R1)の Card Status チェック */
        }
    }
    SaveStatus = SDCARD_Status;                   /* カードステータスの保存 */
    SaveErrStatus = SDCARD_ErrStatus;             /* エラーステータスの保存 */
    if(SDCARD_ErrStatus){                         /* エラーステータスの確認（エラー有り？）*/
        i_sdmcErrProcess();                       /* エラー時の処理 */
    }
    if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか確認 */
        return SDCARD_ErrStatus;
    }
    SDCARD_Status = SaveStatus;                   /* カードステータスの復帰 */
    SDCARD_ErrStatus = SaveErrStatus;             /* エラーステータスの復帰 */
#endif

    SDCARD_SectorSize = ulSave_SectorSize;        /* 保存していたセクタサイズを戻す（default:512bytes）*/
    SD_SetBlockLength(SDCARD_SectorSize);         /* SDカードデータ転送サイズ 512bytes 設定 */

    SD_DisableClock();                            /* SD-CLK Disable */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         i_sdmcErrProcess

  Description:  when error is occured, get Card Status to check and stop the
                transfer.
                エラー発生時の処理。カードステータスを取得し、データ転送中で
                あればストップさせる。

  Arguments:    None

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static u16 i_sdmcErrProcess(void)
{
    u16 usRSP0;
    
    SDCARD_ErrStatus = SDMC_NORMAL;    /* エラーステータスのクリア */

    SDCARD_TimerStop();                /* タイムアウト判定用タイマストップ */

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_ERRPROC_TIMEOUT); /* タイムアウト判定用タイマスタート(2000msec) */
#endif

    SD_SendStatus();                   /* CMD13 addressed card sends its status register 発行、レスポンス待ち */
    
    if(!SDCARD_ErrStatus){             /* エラーステータスの確認（エラー有り？）*/
        SD_GetFPGA( usRSP0, SD_RSP0);
        usRSP0 = (u16)(( usRSP0 & RSP_R1_CURRENT_STATE) >> 1);               /* カレントステートを取り出す */
        if((usRSP0 == CURRENT_STATE_DATA) || (usRSP0 == CURRENT_STATE_RCV)){ /* SDCARD Status が data rcv の時 */
            SD_Command(SD_CMD_CMD | STOP_TRANSMISSION);                      /* CMD12（StopTransmission）発行処理 */
        }
    }

    SDCARD_TimerStop();                /* タイムアウト判定用タイマストップ */

    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         sdmcGetCardSize

  Description:  get card size (number of sectors).
                カードのセクタ数を取得する

  Arguments:    None

  Returns:      number of sectors in the SD card which inserted.
 *---------------------------------------------------------------------------*/
u32 sdmcGetCardSize(void)
{
    /* カード全セクタ数 (SDCARD_Layer_Init関数内で算出される) */
    return ulSDCARD_Size;
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_SD_Status

  Description:  get SD Status.
                SDステータスを取得する（DATライン経由で64バイト送られてくる）。
                カードステータスではないことを留意。
                MultiBlock R/W と異なり、DATライン経由で転送されてくるSDカードの
                レジスタは、MSBから先に送られてくることに注意。
                (Physical Layer Specification 2.00 p12-13参照)

  Arguments:    None

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static u16 SDCARD_SD_Status(void)
{
    SDMC_ERR_CODE SaveErrStatus;             /* エラーステータス保存用 */
    u32           SaveStatus;                /* カードステータス保存用 */
    u32           ulSave_SectorSize;         /* セクタサイズ保存用 */
    u32           ulSaveRestSectorCount;     /* 残りセクタサイズ保存用 */

    SD_EnableClock();                        /* SD-CLK Enable */

    /* ブロックサイズの設定 */
    ulSave_SectorSize = SDCARD_SectorSize;   /* セクタサイズの保存 */
    SDCARD_SectorSize = 64;                  /* SD_STATUS 転送サイズ 64bytes */
    SD_SetBlockLength(SDCARD_SectorSize);    /* SDカードデータ転送サイズ 64byte 設定 */
    if(SDCARD_ErrStatus){                    /* エラーステータスの確認（エラー有り？）*/
        return SDCARD_ErrStatus;
    }

#if TIMEOUT
    SDCARD_TimerStart(SDCARD_RW_TIMEOUT);    /* タイムアウト判定用タイマスタート(4000msec) */
#endif

    if(SD_AppCommand()){                     /* RCA設定後 CMD55発行処理 が正常終了しない?*/
        SD_DisableClock();                   /* SD-CLK Disable */
        return SDCARD_ErrStatus;             /* エラー終了 */
    }

/*    func_SDCARD_CallBack = NULL;    */
    pSDCARD_info = NULL;
    ulSaveRestSectorCount = ulSDCARD_RestSectorCount;    /* 残りセクタサイズを保存(TODO:いらない?） */
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1; /* 残りセクタサイズ、セクタカウントを１に設定 */
    pSDCARD_BufferAddr = SD_SDSTATUS;                    /* データ格納バッファのアドレスを設定 */

    /* 転送前の準備処理 */
    SDCARD_EndFlag   = FALSE;                /* 転送処理完了フラグクリア */
    SDCARD_ErrStatus = SDMC_NORMAL;          /* エラーステータスのクリア */

    thread_flag = TRUE;
    SD_SDStatus();                           /* ACMD13 SD_STATUSの取得コマンド発行処理 */
    PRINTDEBUG( "==Slp Tsk==\n");
#if (TARGET_OS_CTR == 1)
    //can_wup( 0);
    slp_tsk();
#else
    /*--------------------*/
//        OS_TPrintf( "sleep %d\n", __LINE__);
//        OS_SleepThread( NULL);
    /*--------------------*/
#endif
    PRINTDEBUG( "waked\n");
    thread_flag = FALSE;

    while(!SDCARD_EndFlag){                           /* カードアクセス終了待ち */
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか確認 */
            return SDCARD_ErrStatus;
        }
    }

    if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){     /* コマンドレスポンス(R1)のカードステータスが何らかのエラーでないか確認 */
        SD_CheckStatus(FALSE);                        /* コマンドレスポンス(R1)の Card Status チェック */
        if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){ /* コマンドレスポンス(R1)のカードステータスがエラーでないか確認 */
            SD_SendStatus();                      /* カードステータス取得コマンド発行、レスポンス(R1)待ち */
            SD_CheckStatus(FALSE);                /* コマンドレスポンス(R1)の Card Status チェック */
        }
    }
    SaveStatus = SDCARD_Status;                   /* カードステータスの保存 */
    SaveErrStatus = SDCARD_ErrStatus;             /* エラーステータスの保存 */
    if(SDCARD_ErrStatus){                         /* エラーステータスの確認（エラー有り？）*/
        i_sdmcErrProcess();                       /* エラー時の処理 */
    }
    if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか確認 */
        return SDCARD_ErrStatus;
    }
    SDCARD_Status = SaveStatus;                   /* カードステータスの復帰 */
    SDCARD_ErrStatus = SaveErrStatus;             /* エラーステータスの復帰 */

    SDCARD_SectorSize = ulSave_SectorSize;            /* 保存していたセクタサイズを戻す（default:512bytes）*/
    SD_SetBlockLength(SDCARD_SectorSize);             /* SDカードデータ転送サイズ 512bytes 設定 */
    ulSDCARD_RestSectorCount = ulSaveRestSectorCount; /* 保存していた残りセクタサイズを戻す(TODO:いらない?)*/

    SD_DisableClock();                            /* SD-CLK Disable */

    return SDCARD_ErrStatus;
}

/*******************************************************************************/
int MMCP_SetBusWidth( BOOL b4bit)
{
#if 0
    u32 ulSave_SectorSize;                          /* セクタサイズ保存用 */
//    u16 TestData;
    u16 Resid;
#endif

    SD_EnableClock();                        /* SD-CLK Enable */
    
#if 0
    TestData = 0x5A;//0xA5;
    
    /* ブロックサイズの設定 */
    ulSave_SectorSize = SDCARD_SectorSize;             /* セクタサイズの保存 */
    SDCARD_SectorSize = 4;                             /* 転送サイズ 1バイト */
    SD_SetBlockLength( SDCARD_SectorSize);             /* SDカードデータ転送サイズ 1byte 設定 */
#endif
    /*コマンド6発行*/
    MMCP_WriteBusWidth( b4bit);
    SD_AndFPGA(SD_OPTION,(~SD_OPTION_WIDTH_1BIT)); /* IPにビット幅の設定(4bit幅) */
    
#if 0
    /**/
    pSDCARD_info = NULL;
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1;
    pSDCARD_BufferAddr = &TestData;            /* データ格納バッファのアドレスを設定 */
    SDCARD_EndFlag   = FALSE;                  /* 転送処理完了フラグクリア */
    SDCARD_ErrStatus = SDMC_NORMAL;            /* エラーステータスのクリア */
    
#if TIMEOUT
        SDCARD_TimerStart(SDCARD_RW_TIMEOUT);      /* タイムアウト判定用タイマスタート(4000msec) */
#endif
    
    /* IPのSD_SECCNTレジスタ有効化、転送セクタ数設定(自動CMD12発行のため) */
//    SD_EnableSeccnt( ulSDCARD_RestSectorCount);

    /*バステスト*/
    MMCP_BusTest( FALSE);
    
    /**/
    while( !SDCARD_EndFlag) {                         /* カードアクセス終了待ち */
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか? */
            return SDCARD_ErrStatus;                  /* エラー終了 */
        }
    }
    

    /**/
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1;/* 残りセクタサイズ、セクタカウントに１を設定 */
    pSDCARD_BufferAddr = &Resid;                         /* データ格納バッファのアドレスを設定 */
    SDCARD_EndFlag   = FALSE;                          /* 転送処理完了フラグクリア */
    SDCARD_ErrStatus = SDMC_NORMAL;                    /* エラーステータスのクリア */

#if TIMEOUT
        SDCARD_TimerStart(SDCARD_RW_TIMEOUT);      /* タイムアウト判定用タイマスタート(4000msec) */
#endif

    /**/
    MMCP_BusTest( TRUE);

    /**/
    while( !SDCARD_EndFlag) {                          /* カードアクセス終了待ち */
        if( SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか? */
            return SDCARD_ErrStatus;                   /* エラー終了 */
        }
    }

    /*バステスト合格ならバス幅を4bitに拡張*/
//    if( TestData == (~(Resid))) {
    if( Resid == 0xA5) {
        SD_AndFPGA(SD_OPTION,(~SD_OPTION_WIDTH_1BIT)); /* IPにビット幅の設定(4bit幅) */
    }else{
        SD_OrFPGA(SD_OPTION,(SD_OPTION_WIDTH_1BIT)); /* IPにビット幅の設定(1bit幅) */
    }
    
    SDCARD_SectorSize = ulSave_SectorSize;             /* 保存していたセクタサイズを設定（デフォルト 512bytesに戻す）*/
    SD_SetBlockLength( SDCARD_SectorSize);             /* SDカードデータ転送サイズ 512bytes 設定 */
#endif
    return( 0);
}
/*******************************************************************************/


/*---------------------------------------------------------------------------*
  Name:         i_sdmcCheckWP

  Description:  check the write protect bit in the SD_INFO1 register.
                SD_INFO1レジスタのライトプロテクトビットを調べる
                （0:ライトプロテクトされている、1:ライトプロテクトされていない）。
                CTRではポート1は常に1。

  Arguments:    None

  Returns:      number of sectors in the SD card which inserted.
 *---------------------------------------------------------------------------*/
static u16  i_sdmcCheckWP(void)
{
    if (SD_port_number == SDCARD_PORT0)            /* ポート0のとき */
    {
        if(!(SD_CheckFPGAReg(SD_INFO1,SD_INFO1_WRITEPROTECT))) { //WPフラグが立っていないか?
            SDCARD_ErrStatus |= SDMC_ERR_WP;    //エラーフラグのWPエラービットを立てる
        }else{                                  //WPフラグが立っていたとき
            SDCARD_ErrStatus &= ~SDMC_ERR_WP;   //エラーフラグのWPエラービットを落とす
        }
        return    SDCARD_ErrStatus;
    }
    else if (SD_port_number == SDCARD_PORT1)       /* ポート1のとき */
    {  /*TWLのポート1はライトプロテクトビットが常に0（プロテクト状態）なので反転して評価*/
        if((SD_CheckFPGAReg(EXT_WP,EXT_WP_PORT1))) { //WPフラグが立っていないか?
            SDCARD_ErrStatus |= SDMC_ERR_WP;          //エラーフラグのWPエラービットを立てる
        }else{                                        //WPフラグが立っていたとき
            SDCARD_ErrStatus &= ~SDMC_ERR_WP;         //エラーフラグのWPエラービットを落とす
        }
        return    SDCARD_ErrStatus;
    }
    return SDMC_ERR_END;    //ここには来ない
}

/*---------------------------------------------------------------------------*
  Name:         sdmcWriteFifo

  Description:  write to card.
                ラッパーのFIFOを使用してカードへの書き込み。

  Arguments:    buf : 書き込みデータが格納されているバッファのアドレス
                bufsize : 書き込むサイズ（セクタ数）
                offset : 書き込み開始オフセット（セクタ番号）
                info : 実行結果を格納するための構造体へのアドレス

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcWriteFifo( const void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info)
{
    SDCARDMsg     SdMsg;
    OSMessage     recv_dat;
    SDMC_ERR_CODE api_result;

    if( bufsize == 0) {
       return( SDMC_ERR_PARAM);
    }
  
    SdMsg.buf       = (void*)buf;
    SdMsg.bufsize   = bufsize;
    SdMsg.offset    = offset;
    SdMsg.func      = func;
    SdMsg.info      = info;
    SdMsg.operation = SD_OPERATION_WRITE_WITH_FIFO;

    recv_dat = (OSMessage)&SdMsg;
    OS_SendMessage( &sdmc_dtq, recv_dat, OS_MESSAGE_BLOCK);

    /* 返り値待ち */
    OS_ReceiveMessage( &sdmc_result_dtq, &recv_dat, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)recv_dat;
    
    return api_result;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARDi_WriteFifo

  Description:  write to card.
                ラッパーのFIFOを使用してカードへの書き込み。

  Arguments:    buf : 書き込みデータが格納されているバッファのアドレス
                bufsize : 書き込むサイズ（セクタ数）
                offset : 書き込み開始オフセット（セクタ番号）
                info : 実行結果を格納するための構造体へのアドレス

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARDi_WriteFifo( const void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info)
{
    u32* DmaReg;
    SDMC_ERR_CODE result;

#if (SD_FIFO_EMPTY_FLAG_NEW == 1)
  if( sdmc_dma_no == SDMC_NOUSE_DMA) {
        /* FIFO Full割り込み無効、FIFO Empty割り込み有効 */
        *(SDIF_CNT) = (*(SDIF_CNT) & (~SDIF_CNT_FFIE)) | SDIF_CNT_FEIE;
    }else{
        *(SDIF_CNT) = (*(SDIF_CNT) & (~(SDIF_CNT_FFIE | SDIF_CNT_FEIE)));
    }
#else
    /* FIFO割り込み禁止 */
    *(SDIF_CNT) = (*(SDIF_CNT) & (~(SDIF_CNT_FFIE | SDIF_CNT_FEIE)));
#endif
  
    *(SDIF_FDS) = (u16)SDCARD_SectorSize;   /* FIFOのデータサイズ */
    *(SDIF_FSC) = bufsize;
    *(SDIF_CNT) |= (SDIF_CNT_FCLR | SDIF_CNT_USEFIFO);        /* FIFO使用フラグON */
    CC_EXT_MODE = CC_EXT_MODE_DMA;          /* DMAモードON */

  if( sdmc_dma_no != SDMC_NOUSE_DMA) {
        DmaReg = (u32*)(REG_DMA4SAD_ADDR + (0x1C * sdmc_dma_no));
        *(u32*)(DmaReg+6) =  0;                              //reg_MI_DMA4CNT
        *(u32*)(DmaReg++) =  (u32)buf + SDCARD_SectorSize;   //reg_MI_DMA4SAD (new FIFO emptyのときは1セクタ後から)
        *(u32*)(DmaReg++) =  (u32)SDIF_FI; //reg_MI_DMA4DAD
        *(u32*)(DmaReg++) = ((SDCARD_SectorSize / sizeof(u32)) * (bufsize-1)); //総転送ワード数(reg_MI_DMA4TCNT)
        *(u32*)(DmaReg++) = (SDCARD_SectorSize / sizeof(u32));             //転送ワード数(reg_MI_DMA4WCNT)
        *(u32*)(DmaReg++) = 0;                                             //インターバル(reg_MI_DMA4BCNT)
        *(u32*)(DmaReg+1) = REG_MI_DMA4CNT_E_MASK | 0x08000000 | 0x00070000 | 0x0800; //SD起動、128WC, DEST-FIX(reg_MI_DMA4CNT)
    }

    result = SDCARDi_Write( buf, bufsize, offset, func, info);

    /* FIFO無効に */
    *(SDIF_CNT) &= (~SDIF_CNT_USEFIFO);     /* FIFO使用フラグOFF */
    CC_EXT_MODE = CC_EXT_MODE_PIO;          /* PIOモード(DMAモードOFF) */

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcWrite

  Description:  write to card.
                カードへの書き込み。

  Arguments:    buf : 書き込みデータが格納されているバッファのアドレス
                bufsize : 書き込むサイズ（セクタ数）
                offset : 書き込み開始オフセット（セクタ番号）
                info : 実行結果を格納するための構造体へのアドレス

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcWrite( const void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info)
{
    SDCARDMsg     SdMsg;
    OSMessage     recv_dat;
    SDMC_ERR_CODE api_result;

    if( bufsize == 0) {
       return( SDMC_ERR_PARAM);
    }
  
    SdMsg.buf       = (void*)buf;
    SdMsg.bufsize   = bufsize;
    SdMsg.offset    = offset;
    SdMsg.func      = func;
    SdMsg.info      = info;
    SdMsg.operation = SD_OPERATION_WRITE;

    recv_dat = (OSMessage)&SdMsg;
    OS_SendMessage( &sdmc_dtq, recv_dat, OS_MESSAGE_BLOCK);
    
    /* 返り値待ち */
    OS_ReceiveMessage( &sdmc_result_dtq, &recv_dat, OS_MESSAGE_BLOCK);
    api_result = (SDMC_ERR_CODE)recv_dat;
    
    return api_result;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARDi_Write

  Description:  write to card.
                カードへの書き込み。

  Arguments:    buf : 書き込みデータが格納されているバッファのアドレス
                bufsize : 書き込むサイズ（セクタ数）
                offset : 書き込み開始オフセット（セクタ番号）
                info : 実行結果を格納するための構造体へのアドレス

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static SDMC_ERR_CODE SDCARDi_Write( const void* buf,u32 bufsize,u32 offset,void(*func)(),SdmcResultInfo *info)
{
    s16           nRetryCount;
    u32           ulResid;
    SDMC_ERR_CODE SaveErrStatus;  /* エラーステータス保存用 */
    u32           SaveStatus;     /* カードステータス保存用 */

    if( func != NULL){            /* コールバック関数のNullチェック */
        return SDMC_ERR_PARAM;    /* コマンドパラメータエラー */
    }

#if (WP_ena == 1)
    if( i_sdmcCheckWP()) {
        return SDMC_ERR_WP;       /*** ライトプロテクトのチェック    ***/
    }
#endif
    for(nRetryCount = 0;nRetryCount < SDCARD_RETRY_COUNT;nRetryCount++){

    SD_EnableClock();             /* SD-CLK Enable */

/*    func_SDCARD_CallBack = func;    */
        pSDCARD_info = info;
        ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = bufsize;
        pSDCARD_BufferAddr = (void*)buf;                  /* データ格納バッファのアドレスを設定 */

        SDCARD_EndFlag = FALSE;                    /* 転送処理完了フラグクリア */
        SDCARD_ErrStatus = SDMC_NORMAL;            /* エラーステータスのクリア */

#if (TIMEOUT == 1)
        SDCARD_TimerStart(SDCARD_RW_TIMEOUT);      /* タイムアウト判定用タイマスタート(4000msec) */
#endif

        /* IPのSD_SECCNTレジスタ有効化、転送セクタ数設定(自動CMD12発行のため) */
        SD_EnableSeccnt( ulSDCARD_RestSectorCount);

      
#if (SD_FIFO_EMPTY_FLAG_NEW == 1)      
        if( SDCARD_UseFifoFlag) {                            /*--- FIFOを使うとき ---*/
            ulSDCARD_RestSectorCount--;
            MI_CpuSend32( pSDCARD_BufferAddr, SDIF_FI, SDCARD_SectorSize);
            pSDCARD_BufferAddr += (512/2);
        }
#endif
      

        /*--- ライトコマンド発行 ---*/
        if( SDCARD_SDHCFlag) {
            SD_MultiWriteBlock( offset);           /* ライトコマンド発行(引数：オフセット) */
        }else{
            SD_MultiWriteBlock( offset * SDCARD_SectorSize); /* ライトコマンド発行(引数：オフセット*セクタサイズ) */
        }
        /*--------------------------*/

        /**/
        while( !SDCARD_EndFlag) {                         /* カードアクセス終了待ち */
            if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか? */
                return SDCARD_ErrStatus;                  /* エラー終了 */
            }
        }
        
        /* エラーが発生していないか、タイムアウト以外のエラーの場合 */
        if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){     /* コマンドレスポンス(R1)のカードステータスがエラーでないか確認 */
            SD_CheckStatus(FALSE);                        /* コマンドレスポンス(R1)の Card Status チェック */
            if(!(SDCARD_ErrStatus & SDMC_ERR_R1_STATUS)){ /* コマンドレスポンス(R1)のカードステータスがエラーでないか確認 */
                SD_SendStatus();                      /* カードステータスの取得コマンド発行処理 */
                SD_CheckStatus(FALSE);                /* コマンドレスポンス(R1)の Card Status チェック */
            }
        }
        SaveStatus = SDCARD_Status;                   /* カードステータスの保存 */
        SaveErrStatus = SDCARD_ErrStatus;             /* エラーステータスの保存 */
        if( SDCARD_ErrStatus) {                       /* エラーステータスの確認(エラー有り?)*/
            i_sdmcErrProcess();                       /* エラー時の処理(status取得、強制停止) */
        }
        if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか確認 */
            return SDCARD_ErrStatus;                  /* エラー終了 */
        }
#if RESID
        if( SDCARD_SDFlag) {                          /* SDカード(MMCでない)の場合 */
            if( SDCARD_UseFifoFlag) {                 /*--- FIFOを使用しているときは ---*/
                *(SDIF_CNT) &= (~SDIF_CNT_USEFIFO);   /* 一時的にFIFO未使用モードにする */
                CC_EXT_MODE = CC_EXT_MODE_PIO;
                i_sdmcGetResid(&ulResid);             /* 書き込み完了セクタ数の取得 */
                *(SDIF_CNT) |= SDIF_CNT_USEFIFO;      /* FIFO使用モードに戻す */
                CC_EXT_MODE = CC_EXT_MODE_DMA;
            }else{                                    /*--- FIFOを使用していないとき ---*/
                i_sdmcGetResid(&ulResid);             /* 書き込み完了セクタ数の取得 */
            }
            if(info){
                info->resid = ulResid * SDCARD_SectorSize;/*** pSDCARD_info->resid をinfo->resid に修正　柏　2000.08.31. ***/
            }
            if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか確認 */
                return SDCARD_ErrStatus;
            }
        }
#endif
        SDCARD_Status = SaveStatus;                   /* カードステータスの復帰 */
        SDCARD_ErrStatus = SaveErrStatus;             /* エラーステータスの復帰 */
#if RESID
        if( SDCARD_SDFlag) {                          /* SDカード(MMCでない)の場合 */
            if( bufsize != ulResid){                  /* ライト済みセクタ数が正しくないか? */
                SD_SetErr( SDMC_ERR_NUM_WR_SECTORS);  /* エラーフラグセット */
            }
        }
#endif
        if( SDCARD_ErrStatus == SDMC_NORMAL) {        /* エラーステータスの確認(エラー無し?)*/
            break;
        }
    }

    SD_DisableClock();                                /* クロック供給停止 */

    return SDCARD_ErrStatus;
}


/*---------------------------------------------------------------------------*
  Name:         i_sdmcGetResid

  Description:  get the numbers of the well written(without errors) blocks.
                書き込みが完了したセクタの数を取得する。
                

  Arguments:    pResid : 書き込み完了セクタ数を返す変数へのポインタ

  Returns:      0 : success
                > 0 : error
 *---------------------------------------------------------------------------*/
static u16 i_sdmcGetResid(u32 *pResid)
{
    u16    Resid[2];
    u32    ulSave_SectorSize;                          /* セクタサイズ保存用 */

    SDCARD_ErrStatus = SDMC_NORMAL;                    /* エラーステータスをクリア */

    /* ブロックサイズの設定 */
    ulSave_SectorSize = SDCARD_SectorSize;             /* セクタサイズの保存 */
    SDCARD_SectorSize = 4;                             /* 転送サイズ 4バイト */
    SD_SetBlockLength( SDCARD_SectorSize);             /* SDカードデータ転送サイズ 4byte 設定 */
    if(SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){      /* タイムアウトエラーか? */
        return SDCARD_ErrStatus;
    }

#if (TIMEOUT == 1)
    SDCARD_TimerStart(SDCARD_RW_TIMEOUT);              /* タイムアウト判定用タイマスタート(4000msec) */
#endif

    if( SD_AppCommand()) {                             /* RCA設定後 CMD55発行処理 */
        return SDCARD_ErrStatus;                       /* エラー発生(CMD55が正常終了しない)なら戻る */
    }

/*    func_SDCARD_CallBack = NULL;    */
    pSDCARD_info = NULL;
    ulSDCARD_RestSectorCount = ulSDCARD_SectorCount = 1;/* 残りセクタサイズ、セクタカウントに１を設定 */
    pSDCARD_BufferAddr = Resid;                         /* データ格納バッファのアドレスを設定 */

    SDCARD_EndFlag   = FALSE;                          /* 転送処理完了フラグクリア */
    SDCARD_ErrStatus = SDMC_NORMAL;                    /* エラーステータスのクリア */

    thread_flag = TRUE;
    /*--- ACMD22 ライト済みセクタ数取得コマンド発行 ---*/
    SD_SendNumWRSectors();                                
    /*-------------------------------------------------*/
    PRINTDEBUG( "==Slp Tsk==\n");
#if (TARGET_OS_CTR == 1)
//    can_wup( 0);
    slp_tsk();
#else
    /*--------------------*/
//        OS_TPrintf( "sleep %d\n", __LINE__);
//        OS_SleepThread( NULL);
    /*--------------------*/
#endif
    PRINTDEBUG( "waked\n");
    thread_flag = FALSE;

    while( !SDCARD_EndFlag) {                          /* カードアクセス終了待ち */
        if( SDCARD_ErrStatus & SDMC_ERR_FPGA_TIMEOUT){ /* タイムアウトエラーか? */
            return SDCARD_ErrStatus;                   /* エラー終了 */
        }
    }
    /* エラーが発生していないか、タイムアウト以外のエラーの場合 */
    if( SDCARD_ErrStatus) {                            /* (タイムアウト以外に)エラー発生か? */
        i_sdmcErrProcess();                            /* エラー時の処理 */
        *pResid = 0L;                                  /* ライト済みセクタ数に 0 を設定 */
    }else{                                             /* エラーが発生していない場合 */
        /* SDカードのレジスタはMSBから送られてくるため並べ替えを行う */
        Resid[1] = SD_SwapByte(&Resid[1]);             /* 上位 1byte と下位 1byte を入れ替える */
        (((LELONG *)pResid)->dt2word.low) = Resid[1];  /* Resid[1]の設定 */
        Resid[0] = SD_SwapByte(&Resid[0]);             /* 上位 1byte と下位 1byte を入れ替える */
        (((LELONG *)pResid)->dt2word.high) = Resid[0]; /* Resid[0]の設定 */
    }
    
    SDCARD_SectorSize = ulSave_SectorSize;             /* 保存していたセクタサイズを設定（デフォルト 512bytesに戻す）*/
    SD_SetBlockLength( SDCARD_SectorSize);             /* SDカードデータ転送サイズ 512bytes 設定 */
    /* SetBlockLengthでエラーが出たらどうする? */

    return SDCARD_ErrStatus;
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Restore_port0

  Description:  restore registers and variables of port0.
                ポート0のレジスタや変数を復帰する。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Restore_port0(void)
{
    /* registers */
    SD_SetFPGA( SD_CLK_CTRL, SDPort0Context.SD_CLK_CTRL_VALUE);
    SD_SetFPGA( SD_OPTION,   SDPort0Context.SD_OPTION_VALUE);

    /* variables */
    SD_RCA           = SDPort0Context.SD_RCA;
    SDCARD_ErrStatus = SDPort0Context.ErrStatus;
    SDCARD_Status    = SDPort0Context.Status;
    SDCARD_MMCFlag   = SDPort0Context.MMCFlag;
    SDCARD_SDHCFlag  = SDPort0Context.SDHCFlag;
    SDCARD_SDFlag    = SDPort0Context.SDFlag;

    SDCARD_OutFlag = SDPort0Context.OutFlag;
    pSDCARD_info   = NULL;

    /*media registers*/
    MI_CpuCopy8( SDPort0Context.SD_CID, SD_CID, 16);
    MI_CpuCopy8( SDPort0Context.SD_CSD, SD_CSD, 16);
    MI_CpuCopy8( SDPort0Context.SD_OCR, SD_OCR,  4);
    MI_CpuCopy8( SDPort0Context.SD_SCR, SD_SCR,  8);
  
    /*recalc*/
    SDCARD_WP_PERMANENT = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_PERMANENT_BIT));
    SDCARD_WP_TEMPORARY = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_TEMPORARY_BIT));
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Restore_port1

  Description:  restore registers and variables of port0.
                ポート1のレジスタや変数を復帰する。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Restore_port1(void)
{
    /* registers */
    SD_SetFPGA( SD_CLK_CTRL, SDPort1Context.SD_CLK_CTRL_VALUE);
    SD_SetFPGA( SD_OPTION,   SDPort1Context.SD_OPTION_VALUE);

    /* variables */
    SD_RCA           = SDPort1Context.SD_RCA;
    SDCARD_ErrStatus = SDPort1Context.ErrStatus;
    SDCARD_Status    = SDPort1Context.Status;
    SDCARD_MMCFlag   = SDPort1Context.MMCFlag;
    SDCARD_SDHCFlag  = SDPort1Context.SDHCFlag;
    SDCARD_SDFlag    = SDPort1Context.SDFlag;

    SDCARD_OutFlag = SDPort1Context.OutFlag;
    pSDCARD_info   = NULL;

    /*media registers*/
    MI_CpuCopy8( SDPort1Context.SD_CID, SD_CID, 16);
    MI_CpuCopy8( SDPort1Context.SD_CSD, SD_CSD, 16);
    MI_CpuCopy8( SDPort1Context.SD_OCR, SD_OCR,  4);
    MI_CpuCopy8( SDPort1Context.SD_SCR, SD_SCR,  8);
  
    /*recalc*/
    SDCARD_WP_PERMANENT = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_PERMANENT_BIT));
    SDCARD_WP_TEMPORARY = (u16)(SD_CSD[0] & (u16)(SDCARD_WP_TEMPORARY_BIT));
}


/*---------------------------------------------------------------------------*
  Name:         sdmcSelectedNo

  Description:  get selected port number.
                選択されているポート番号を取得する

  Arguments:    None

  Returns:      [15:8]port numbers which supported（サポートされているポート数）
                [7:0]port number which selected now（選択されているポート番号）
 *---------------------------------------------------------------------------*/
u16 sdmcSelectedNo(void)
{
    u16 i_sdmcSelect_Value;

    SD_GetFPGA(i_sdmcSelect_Value,SD_PORTSEL);    /* SD_PORTSELレジスタ値を取得 */
    
    return i_sdmcSelect_Value;
}

/*---------------------------------------------------------------------------*
  Name:         sdmcSelect

  Description:  select port.
                ポートを選択する

  Arguments:    select : [15:8]port numbers which supported（サポートされているポート数）
                         [7:0]port number which selected now（選択するポート番号）
  Returns:      0 : success
                >0 : error
 *---------------------------------------------------------------------------*/
SDMC_ERR_CODE sdmcSelect(u16 select)
{
    union
    {
        u16    Val;
        struct     
        {
            u8    L;        /* 指定したいポート */
            u8    H;        /* サポートされているポート */
        }    PSel;
    }    SDCARD_PSel;

    //TODO : transferモードからstand-byモードにする?
/*    if( !SDCARD_EndFlag) {                      // 転送が残っている場合はエラー
        return SDMC_ERR_END;
    }
*/
    SDCARD_PSel.Val = select;
    if ((SDCARD_PSel.PSel.H > SDCARD_PORT_NO_MAX) 
        | (SDCARD_PSel.PSel.H < SDCARD_PORT_NO_MIN)
        | (SDCARD_PSel.PSel.L > SDCARD_PORT_SELECT_NO))
    {
        return SDMC_ERR_PARAM;                    /* コマンドパラメータエラー        */
    }else{
        SD_port_en_numbers = SDCARD_PSel.PSel.H;  /* 新しくサポートするポート数を保存 */
        SD_port_number = SDCARD_PSel.PSel.L;      /* 新しく選択するポート番号を保存 */
        
        SD_GetFPGA(SDCARD_PSel.Val,SD_PORTSEL);   /* 現在のレジスタを取得 */
        
        if (SDCARD_PSel.PSel.L == 0)              /* 現在選択されているポートが 0 ? */
        {
            SDCARD_Backup_port0();
        }else{
            SDCARD_Backup_port1();
        }
        
        SDCARD_PSel.Val= select;
        
        if (SDCARD_PSel.PSel.L == 0)              /* 新しく選択されたポートが 0 ? */
        {
            SDCARD_Restore_port0();
        }else{
            SDCARD_Restore_port1();
        }
    
        SD_SetFPGA(SD_PORTSEL,select);            /* レジスタへセット    */
        return SDMC_NORMAL;                       /* 0　リターン */
    }
}


/*---------------------------------------------------------------------------*
  Name:         SDCARD_Thread

  Description:  SDメモリカードメインタスク

  Arguments:    

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Thread( void* arg)
{
    SDCARDMsg*    SdMsg;
    OSMessage     current_dat;
    SDMC_ERR_CODE api_result;
#pragma unused( arg)

    while( TRUE) {
        /* メッセージ待ち */
        PRINTDEBUG( "rcv mes sdThread\n");
        OS_ReadMessage( &sdmc_dtq, &current_dat, OS_MESSAGE_BLOCK);
        SdMsg = (SDCARDMsg*)current_dat;
        PRINTDEBUG( "sd task : receive command : %d\n", SdMsg->operation);

        switch( SdMsg->operation) {
          case SD_OPERATION_INIT:
            api_result = i_sdmcInit();
            break;
          case SD_OPERATION_READ:
            PRINTDEBUG( "from:0x%x, sectors:0x%x\n", SdMsg->offset, SdMsg->bufsize);
            api_result = SDCARDi_Read( SdMsg->buf, SdMsg->bufsize,
                                       SdMsg->offset, SdMsg->func,
                                       SdMsg->info);
            break;
          case SD_OPERATION_READ_WITH_FIFO:
            PRINTDEBUG( "from:0x%x, sectors:0x%x\n", SdMsg->offset, SdMsg->bufsize);
            api_result = SDCARDi_ReadFifo( SdMsg->buf, SdMsg->bufsize,
                                           SdMsg->offset, SdMsg->func,
                                           SdMsg->info);
            break;
          case SD_OPERATION_WRITE:
            PRINTDEBUG( "from:0x%x, sectors:0x%x\n", SdMsg->offset, SdMsg->bufsize);
            api_result = SDCARDi_Write( SdMsg->buf, SdMsg->bufsize,
                                        SdMsg->offset, SdMsg->func,
                                        SdMsg->info);
            break;
          case SD_OPERATION_WRITE_WITH_FIFO:
            PRINTDEBUG( "from:0x%x, sectors:0x%x\n", SdMsg->offset, SdMsg->bufsize);
            api_result = SDCARDi_WriteFifo( SdMsg->buf, SdMsg->bufsize,
                                            SdMsg->offset, SdMsg->func,
                                            SdMsg->info);
            break;
          default:
            PRINTDEBUG( "sdmc-thread error : undefined command.\n");
            api_result = SDMC_ERR_COMMAND;
            break;
        }

        PRINTDEBUG( "sd task : operation ends(result : 0x%x), send message begin\n", api_result);

        /*メッセージ返送*/
        current_dat = (OSMessage)api_result;
        OS_SendMessage( &sdmc_result_dtq, current_dat, OS_MESSAGE_BLOCK);
        OS_ReceiveMessage( &sdmc_dtq, &current_dat, OS_MESSAGE_BLOCK);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SDCARD_Intr_Thread

  Description:  SDメモリカード割り込み処理タスク

  Arguments:    

  Returns:      None
 *---------------------------------------------------------------------------*/
static void SDCARD_Intr_Thread( void* arg)
{
   static i = 0;
    u16        sd_info1;//, sd_info2;
//    OSIntrMode enabled;
#pragma unused( arg)

    OS_EnableInterrupts();
    OS_EnableIrq();
  
    while( 1) {
      i++;
        PRINTDEBUG( "Slp sdIntr\n");

        OS_WaitIrq( FALSE, OS_IE_SD);
      
        (void)OS_ClearIrqCheckFlag( OS_IE_SD);
        *(vu16*)CTR_INT_IF = CTR_IE_SD_MASK;          /*SD割り込みのIF解除*/
      
//        OS_TPrintf( "sdIntr waked\n");
      
      while( 1) {
        /*--- FIFOを使うとき ---*/
        if( SDCARD_UseFifoFlag) {
            sd_info1 = SD_INFO1;
            if( (((*SDIF_CNT & SDIF_CNT_FULL)&&(*SDIF_CNT & SDIF_CNT_FFIE)) ||
                ((!(*SDIF_CNT & SDIF_CNT_NEMP))&&(*SDIF_CNT & SDIF_CNT_FEIE)))
                ){
            
//                OS_TPrintf( ">>>SD Intr(FIFO) %d\n", i);// Full or Empty)\n");
                OS_DisableIrqMask( OS_IE_SD);
                SDCARD_FPGA_irq();                        /*カードからのリードライト要求割り込み*/
                OS_EnableIrqMask( OS_IE_SD);
//                break;
              
#if (SD_FIFO_EMPTY_FLAG_NEW == 1)
#else
            }else{
                /*----- FIFO-EmptyFlag未修正の場合、WriteはBWE割り込みに頼る必要がある -----*/
                if( SD_CheckFPGAReg( SD_INFO2, (SD_INFO2_MASK_BRE | SD_INFO2_MASK_BWE))) {
//                    OS_TPrintf( ">>>SD Intr(R/W Req)\n");
                    //ここで自動的にラッパーのFIFO<->SD_BUF0間で通信が行われる
                    OS_DisableIrqMask( OS_IE_SD);
                    SDCARD_FPGA_irq();
                    OS_EnableIrqMask( OS_IE_SD);
                    break;
                }
                /*----------*/
#endif              
            }
          
            /* ALL_END */
            sd_info1 = SD_INFO1;
            if( (SD_CheckFPGAReg( sd_info1, SD_INFO1_ALL_END))&&
                ((ulSDCARD_RestSectorCount<=0) || (sdmc_dma_no != SDMC_NOUSE_DMA))) {
//                    while( (*SDIF_CNT & SDIF_CNT_NEMP)) {};      /* FIFOにデータが残っているうちは終了しない */
                ulSDCARD_RestSectorCount = 0; //DMA使用のときはデクリメントされてないので
//                OS_TPrintf( ">>>SD Intr(End or Err) %d\n", i);
                (void)OS_ClearIrqCheckFlag( OS_IE_SD);
                *(vu16*)CTR_INT_IF = CTR_IE_SD_MASK;  /*SD割り込みのIF解除*/
                OS_DisableIrqMask( OS_IE_SD);
                SYSFPGA_irq();
                OS_EnableIrqMask( OS_IE_SD);
                if( thread_flag) {
                    PRINTDEBUG( "--Wup sdThread!--\n");
                    sdmc_wakeup_count++;
                    PRINTDEBUG( "wakeup\n");
                    OS_WakeupThreadDirect( &sdmc_tsk);
                }
                break;
            }
          break;
        /*--- FIFOを使わないとき ---*/
        }else{
            if( SD_CheckFPGAReg( SD_INFO2, (SD_INFO2_MASK_BRE | SD_INFO2_MASK_BWE))) {
                PRINTDEBUG( ">>>SD Intr(R/W Req)\n");
                OS_DisableIrqMask( OS_IE_SD);
                SDCARD_FPGA_irq();                        /*カードからのリードライト要求割り込み*/
                OS_EnableIrqMask( OS_IE_SD);
            }else{
                PRINTDEBUG( ">>SD Intr(End or Err)\n");
                OS_DisableIrqMask( OS_IE_SD);
                SYSFPGA_irq();                            /*完了またはエラー割り込み*/
                OS_EnableIrqMask( OS_IE_SD);
                /**/
                if( thread_flag) {
                    PRINTDEBUG( "--Wup sdThread!--\n");
                    sdmc_wakeup_count++;
                    OS_WakeupThreadDirect( &sdmc_tsk);
                }
            }
            break;
        }
      }
    }
}
