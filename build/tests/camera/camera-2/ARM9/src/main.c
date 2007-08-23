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

#define DMA_NO  5           // 使用するDMA番号(4-7)
#define DMA_IE  OS_IE_DMA5  // 対応する割り込み
#define WIDTH   256         // イメージの幅
#define HEIGHT  192         // イメージの高さ

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // 一回の転送ライン数
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // 一ラインの転送バイト数

#if 0
    カメラから送られてきたデータを、フレーム単位より小さい単位で処理したい場合、
    CAMERA_DmaRecvInfinity() が利用できる。というか利用するしかない。
    (DMAなしで安全にカメラからのデータを取得する手段がないことに注意！！！)
#endif

static void VBlankIntr(void);
static void CameraDmaIntr(void);
static void CameraErrIntr(void);

static int lineNumber = 0;
static BOOL effectFlag = FALSE;
static u16 pipeBuffer[BYTES_PER_LINE * LINES_AT_ONCE / sizeof(u16)] ATTRIBUTE_ALIGN(32);
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
    CAMERA_Init();      // wakeup camera module

    result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CActivate was failed. (%d)\n", result);
    }
#if 0
    result = CAMERA_I2CResize(CAMERA_SELECT_IN, 320, 240);
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CResize was failed. (%d)\n", result);
    }
#endif
    CAMERA_SetTrimmingParamsCenter(WIDTH, HEIGHT, 320, 240);    // clipped by camera i/f
    CAMERA_SetTrimming(TRUE);
    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // DMA割り込み設定
    OS_SetIrqFunction(DMA_IE, CameraDmaIntr);
    (void)OS_EnableIrqMask(DMA_IE);

    // DMAスタート (明示的に止めない限り永遠に有効)
    MIi_StopExDma(DMA_NO);
    CAMERA_DmaPipeInfinity(DMA_NO, pipeBuffer, BYTES_PER_LINE * LINES_AT_ONCE);

    // カメラ割り込み設定
    CAMERA_SetBufferErrorIntrrupt(TRUE);
    CAMERA_SetMasterIntrrupt(TRUE);
    OS_SetIrqFunction(OS_IE_CAM, CameraErrIntr);
    (void)OS_EnableIrqMask(OS_IE_CAM);

    // カメラスタート
    lineNumber = 0;
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
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
            effectFlag ^= 1;
            OS_TPrintf("Effect %s\n", effectFlag ? "ON" : "OFF");
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
//    DMA割り込み処理 (重すぎ？)
//
#define PRINT_RATE  32
void CameraDmaIntr(void)
{
    static BOOL effect = FALSE;
    OS_SetIrqCheckFlag(DMA_IE); // checking dma interrupt

    // 必要な処理をしてフレームバッファにコピーする
    if (effect)
    {
        int i;
        u32 *src = (u32*)pipeBuffer;
        u32 *dest = (u32*)HW_LCDC_VRAM_A + lineNumber * WIDTH / 2;
        for (i = 0; i < WIDTH * LINES_AT_ONCE / 2; i++) // pack 2 pixel data
        {
            dest[i] = ~src[i];  // reverse (if using RGBA format, should bitORed with 0x10001000)
        }
    }
    else
    {
        MI_CpuCopy8(pipeBuffer, (u16*)HW_LCDC_VRAM_A + lineNumber * WIDTH, sizeof(pipeBuffer));
    }
    DC_InvalidateRange(pipeBuffer, sizeof(pipeBuffer));

    lineNumber += LINES_AT_ONCE;
    if (lineNumber >= HEIGHT)
    {
        static OSTick begin = 0;
        static int count = 0;
        // 必要ならフレームバッファのスワップなど

        effect = effectFlag;    // このタイミングで反映

        // debug print
        OS_TPrintf(".");
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

        lineNumber = 0;
    }
}

//--------------------------------------------------------------------------------
//    カメラ割り込み処理 (エラー時しか発生しないはず)
//
#if 0
    DMAは放置プレイで良い？
#endif
void CameraErrIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_CAM);  // checking camera interrupt

    OS_TPrintf("Error was occurred.\n");

    // カメラ停止
    CAMERA_StopCapture();
    lineNumber = 0;

    // カメラ再開
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
}
