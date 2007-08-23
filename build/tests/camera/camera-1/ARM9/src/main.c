/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - camera
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

#include <twl.h>
#include <twl/camera.h>

#define DMA_NO      5           // 使用するDMA番号(4-7)
#define WIDTH       256         // イメージの幅
#define HEIGHT      192         // イメージの高さ

#define DEFAULT_CAMERA  CAMERA_SELECT_IN
//#define DEFAULT_CAMERA  CAMERA_SELECT_OUT

//#define DUAL_MODE // 両方アクティブ状態での使用
#if 0
    DUAL_MODEはテストのために用意しているだけで、切り替えの瞬間に画面が
    崩れるのは仕様です。CAMERA_I2COutputWithDualActivation()の目的は、
    両者のFPSを固定しておき、ほぼ同時にStandbyから解除させることで、
    フレーム同期している状態にしておき、上下のLCDにIN/OUTの両方の画像を
    カメラに設定した半分のFPSで表示させるということが可能となるように
    するためのものです。VRAM表示は1画面しか使えないので、FPGAでは検証
    できません。(TSのタイミングで別サンプル作成予定)

    切り替え時の自動露光の時間がもったいないときにも利用可能ではあります
    が、消費電力の  観点から見て、CAMERA_I2COutputWithDualActivation()の
    利用は最低限に限定すべきです。
    (IN/OUT共通でフレームレート固定なので、片側の画面がかなり暗くなる
     恐れがあることも踏まえておく必要があります)

    最終的に、この仕様を残すかどうかは難しいところですね。
    (削るならARM7側の仕様をもう少し整理できるのですが)
#endif

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // 一回の転送ライン数
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // 一ラインの転送バイト数

static void VBlankIntr(void);
static void CameraIntr(void);

static BOOL startRequest = FALSE;
static CameraSelect current;
/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    CAMERAResult result;

    // 初期化
    OS_Init();
    OS_InitThread();
    GX_Init();
    OS_InitTick();

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableInterrupts();

    // VRAM表示モード
    GX_SetBankForLCDC(GX_VRAM_LCDC_A);
    MI_CpuClearFast((void *)HW_LCDC_VRAM_A, WIDTH*HEIGHT*2);
    GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, GX_BGMODE_0, GX_BG0_AS_2D);
    //GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
    OS_WaitVBlankIntr();
    GX_DispOn();

    // カメラ初期化
    {
        OSTick begin = OS_GetTick();
        CAMERA_Init();      // wakeup camera module
        OS_TPrintf("CAMERA_Init took %d msec\n", (int)OS_TicksToMilliSeconds(OS_GetTick()-begin));
    }
    current = DEFAULT_CAMERA;

#ifdef DUAL_MODE
    result = CAMERA_I2COutputWithDualActivation(current);
#else
    result = CAMERA_I2CActivate(current);
#endif
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CActivate was failed. (%d)\n", result);
    }
#if 0
    result = CAMERA_I2CResize(current, 320, 240);
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CResize was failed. (%d)\n", result);
    }
#endif
    CAMERA_SetTrimmingParamsCenter(WIDTH, HEIGHT, 320, 240);    // clipped by camera i/f
    CAMERA_SetTrimming(TRUE);
    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // カメラ割り込み設定
    //CAMERA_SetVsyncIntrrupt(CAMERA_INTR_VSYNC_POSITIVE_EDGE);   // almost end of vblank
    CAMERA_SetVsyncIntrrupt(CAMERA_INTR_VSYNC_NEGATIVE_EDGE);   // almost begin of vblank
    CAMERA_SetBufferErrorIntrrupt(TRUE);
    CAMERA_SetMasterIntrrupt(TRUE);
    OS_SetIrqFunction(OS_IE_CAM, CameraIntr);
    (void)OS_EnableIrqMask(OS_IE_CAM);

    // カメラスタート (割り込みハンドラで代用)
    startRequest = TRUE;
    CameraIntr();
    /*
      (シャープカメラのみ確認済み事項)
      Acitavate後、約30msec間VBlank期間が続く仕様なので、Activate直後は
      実際の開始処理をしてしまって問題ない
      (結局1枚目を捨てるつもりならフラグを立てるだけでいいけど・・・)
    */
    OS_TPrintf("Camera is shooting a movie...\n");

    while (1)
    {
        u16 pad;
        u16 trg;
        static u16 old = 0xffff;    // ignore the trigger by first data

        OS_WaitVBlankIntr();

        pad = PAD_Read();
        trg = (u16)(pad & ~old);
        old = pad;


        if (trg & PAD_BUTTON_A)
        {
        }
        if (trg & PAD_BUTTON_B)
        {
            current = (current == CAMERA_SELECT_IN ? CAMERA_SELECT_OUT : CAMERA_SELECT_IN);
            OS_TPrintf("call CAMERA_I2CActivate(%s)... ", (current == CAMERA_SELECT_IN ? "CAMERA_SELECT_IN" : "CAMERA_SELECT_OUT"));
#ifdef DUAL_MODE
            result = CAMERA_I2COutputWithDualActivation(current);
#else
            result = CAMERA_I2CActivate(current);
#endif
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
        }
        if (trg & PAD_BUTTON_R)
        {
            OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_NONE)... ");
            result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
        }
        if (trg & PAD_BUTTON_START) // start/stop to capture w/o Activate API
        {
            if (CAMERA_IsBusy())
            {
                OS_TPrintf("call CAMERA_Stop()... ");
                CAMERA_StopCapture();
                while (CAMERA_IsBusy())
                {
                }
                OS_TPrintf("Camera was stopped.\n");
                MIi_StopExDma(DMA_NO);
            }
            else
            {
                startRequest = TRUE;
                OS_TPrintf("Camera is shooting a movie...\n");
                while (CAMERA_IsBusy() == FALSE)
                {
                }
            }
        }
        if (trg & PAD_BUTTON_SELECT)    // power on/off
        {
            if (reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK)
            {
                OS_TPrintf("call CAMERA_End()... ");
                CAMERA_End();
                OS_TPrintf("Done.\n");
                MIi_StopExDma(DMA_NO);
            }
            else
            {
                OS_TPrintf("call CAMERA_Init()... ");
                CAMERA_Init();
                OS_TPrintf("Done.\n");
#if 0
                // resize
                OS_TPrintf("call CAMERA_I2CResize(current)... ");
                result = CAMERA_I2CResize(current, 320, 240);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
#endif
                // activate inside camera first
                OS_TPrintf("call CAMERA_I2CActivate(current)... ");
                result = CAMERA_I2CActivate(current);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS ? "SUCCESS" : "FAILED");
            }
        }
    }
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

//--------------------------------------------------------------------------------
//    カメラ割り込み処理 (エラー時とVsync時の両方で発生)
//
#if 0
    Start時にDMAを開始しない場合、Startのタイミングによっては、最初にエラー検知から始まる。
    Start時にDMAを開始している場合、Startのタイミングによっては、DMAが停止していないから始まる。
    DMA周りの管理をカメラのVsyncからDMA割り込みに変えても良いかも。

    カメラのVsync自体はCAMERA_Start/Stopとは関係なく発生している。(Acitavate(NONE)で停止する)
    StartやStopはメインルーチンではフラグを立てるだけとし、実際の処理はすべて割り込みハンドラで
    実行するというのもあり。(とりあえずStartだけフラグとして実装している)
    たとえAcitivate(NONE) でも I2CInit中はVsyncが発生するので注意！

    カメラ動作中にActivate(NONE)とした場合、Stopを呼んでもずっとBUSYのままとなるので注意。
    (Vsyncで状態が切り替わるのに、Vsyncがこなくなるので (未確認だけど))

    このサンプルでは、カメラ停止時に、DMAが動いている場合と止まっている場合がある。
#endif
#define PRINT_RATE  32
void CameraIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_CAM); // checking camera interrupt

    if (CAMERA_GetErrorStatus())    // error?
    {
        OS_TPrintf("Error was occurred.\n");
        // 停止処理
        CAMERA_StopCapture();   // カメラ停止
        CAMERA_ClearBuffer();   // クリア (すぐにできる？)
        MIi_StopExDma(DMA_NO);  // DMA停止
        startRequest = TRUE;    // カメラ再開要求
        return;     // waiting next frame (skip current frame)
    }

    // 以降はVsync時の処理

    if (startRequest)
    {
        CAMERA_ClearBuffer();
        CAMERA_StartCapture();
        startRequest = FALSE;
    }

    if (CAMERA_IsBusy() == FALSE)   // done to execute stop command?
    {
        //OS_TPrintf("while stopping the capture or just finished\n");
    }
    else
    {
        OS_TPrintf(".");
        if (MIi_IsExDmaBusy(DMA_NO))    // NOT done to capture last frame?
        {
            OS_TPrintf("DMA was not done until VBlank.\n");
            MIi_StopExDma(DMA_NO);  // DMA停止
//            return; // waiting next frame (skip current frame)
        }
        // start to capture for next frame
        CAMERA_DmaRecvAsync(DMA_NO, (void *)HW_LCDC_VRAM_A, BYTES_PER_LINE * LINES_AT_ONCE, BYTES_PER_LINE * HEIGHT);
    }

    // debug print
    {
        static OSTick begin = 0;
        static int count = 0;
        if (begin == 0) // first time
        {
            begin = OS_GetTick();
        }
        else if (++count == PRINT_RATE)
        {
            OSTick uspf = OS_TicksToMicroSeconds(OS_GetTick() - begin) / count;
            int mfps = (int)(1000000000LL / uspf);
            OS_TPrintf("%2d.%03d fps\n", mfps / 1000, mfps % 1000);
            count = 0;
            begin = OS_GetTick();
        }
    }
}
