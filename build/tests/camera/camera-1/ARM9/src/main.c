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
#define NUMS        1           // バッファ数

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // 一回の転送ライン数
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // 一ラインの転送バイト数

static void VBlankIntr(void);
static void CameraIntr(void);

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
    CAMERA_Init();      // create camera thread
    CAMERA_PowerOn();   // wakeup camera module

    result = CAMERA_I2CInit(CAMERA_SELECT_IN);
    if (result != CAMERA_RESULT_SUCCESS_TRUE)
    {
        OS_TPrintf("CAMERA_I2CInit was failed. (%d)\n", result);
        CAMERA_PowerOff();
        OS_Terminate();
    }
    OS_TPrintf("CAMERA_I2CInit was done successfully.\n");

    result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
    if (result != CAMERA_RESULT_SUCCESS_TRUE)
    {
        OS_TPrintf("CAMERA_I2CActivate was failed. (%d)\n", result);
    }
#if 0
    result = CAMERA_I2CResize(CAMERA_SELECT_IN, 320, 240);
    if (result != CAMERA_RESULT_SUCCESS_TRUE)
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

    // カメラスタート (DMAはここで開始してもしなくても良い)
    CAMERA_ClearBuffer();
    CAMERA_DmaRecvAsync(DMA_NO, (void *)HW_LCDC_VRAM_A, BYTES_PER_LINE * LINES_AT_ONCE, BYTES_PER_LINE * HEIGHT);
    CAMERA_Start();
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
            OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_IN)... ");
            result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
        }
        if (trg & PAD_BUTTON_B)
        {
            OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_OUT)... ");
            result = CAMERA_I2CActivate(CAMERA_SELECT_OUT);
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
        }
        if (trg & PAD_BUTTON_R)
        {
            OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_NONE)... ");
            result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
        }
        if (trg & PAD_BUTTON_START) // start/stop to capture
        {
            if (OS_GetIrqMask() & OS_IE_CAM)
            {
                // clear the handler
                (void)OS_DisableIrqMask(OS_IE_CAM);
                OS_TPrintf("call CAMERA_Stop()... ");
                CAMERA_Stop();
                while (CAMERA_IsBusy())
                {
                }
                OS_TPrintf("Camera was stopped.\n");
            }
            else
            {
                // set the handler
                OS_SetIrqFunction(OS_IE_CAM, CameraIntr);
                (void)OS_EnableIrqMask(OS_IE_CAM);

                CAMERA_ClearBuffer();
                CAMERA_Start();
                OS_TPrintf("Camera is shooting a movie...\n");
            }
        }
        if (trg & PAD_BUTTON_SELECT)    // power on/off
        {
            if (reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK)
            {
                OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_NONE)... ");
                result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
                OS_TPrintf("call CAMERA_PowerOff()... ");
                CAMERA_PowerOff();
                OS_TPrintf("Done.\n");
            }
            else
            {
                OS_TPrintf("call CAMERA_PowerOn()... ");
                CAMERA_PowerOn();
                OS_TPrintf("Done.\n");
                OS_TPrintf("call CAMERA_I2CInit(CAMERA_SELECT_IN)... ");
                result = CAMERA_I2CInit(CAMERA_SELECT_IN);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
                // resize
                OS_TPrintf("call CAMERA_I2CResize(CAMERA_SELECT_IN)... ");
                result = CAMERA_I2CResize(CAMERA_SELECT_IN, 320, 240);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
                // activate inside camera first
                OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_IN)... ");
                result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
                // set misc registers
                CAMERA_SetTrimmingParamsCenter(WIDTH, HEIGHT, 320, 240);    // clipped by camera i/f
                CAMERA_SetTrimming(TRUE);
                CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
                CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));
                CAMERA_SetVsyncIntrrupt(CAMERA_INTR_VSYNC_POSITIVE_EDGE);   // almost end of vblank
                CAMERA_SetBufferErrorIntrrupt(TRUE);
                CAMERA_SetMasterIntrrupt(TRUE);
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
#endif
#define PRINT_RATE  32
void CameraIntr(void)
{
    OS_TPrintf(".");
    OS_SetIrqCheckFlag(OS_IE_CAM); // checking camera interrupt

    if (CAMERA_GetErrorStatus())    // error?
    {
        OS_TPrintf("Error was occurred.\n");
        // 停止処理
        CAMERA_Stop();          // カメラ停止
        MIi_StopExDma(DMA_NO);  // DMA停止
        while (CAMERA_IsBusy()) //        待ち
        {
        }
        // カメラ再開
        CAMERA_ClearBuffer();
        CAMERA_Start();
        return; // waiting next frame (skip current frame)
    }

    // 以降はVsync時の処理

    if (MIi_IsExDmaBusy(DMA_NO))    // NOT done to capture last frame?
    {
        OS_TPrintf("DMA was not done until VBlank.\n");
        return; // waiting next frame (skip current frame)
    }
    if (CAMERA_IsBusy() == FALSE)   // done to execute stop command?
    {
        OS_TPrintf("Finished receiving final frame.\n");
        // disable this handler?
    }
    else
    {
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
            OS_TPrintf("%2d.%03d fps\n", (int)(1000000LL / uspf), (int)(1000000000LL / uspf) % 1000);
            count = 0;
            begin = OS_GetTick();
        }
    }
}
