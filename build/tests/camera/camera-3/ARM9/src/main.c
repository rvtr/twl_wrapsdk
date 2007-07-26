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

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // 一回の転送ライン数
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // 一ラインの転送バイト数

static void VBlankIntr(void);
static void CameraIntr(void);

static BOOL startRequest = FALSE;

#if 0
    描画よりも表示のFPSが大きい前提でのトリプルバッファシーケンス
#endif

static int wp;  // カメラからデータを取り込み中のバッファ
static int rp;  // 表示中のバッファ
static BOOL wp_pending; // 取り込みを中断した (再び同じバッファに取り込む)
static GXDispMode dispMode[] =
{
    GX_DISPMODE_VRAM_A,
    GX_DISPMODE_VRAM_C,
    GX_DISPMODE_VRAM_D
};
static void* dispAddr[] =
{
    (void*)HW_LCDC_VRAM_A,
    (void*)HW_LCDC_VRAM_C,
    (void*)HW_LCDC_VRAM_D
};
static void DebugReport(void)
{
    const char* const str[] =
    {
        "VRAM[A]",
        "VRAM[C]",
        "VRAM[D]"
    };
    OSIntrMode enabled = OS_DisableInterrupts();
    int wp_bak = wp;
    int rp_bak = rp;
    OS_RestoreInterrupts(enabled);
    OS_TPrintf("\nCapture to %s\tDisplay from %s\n", str[wp_bak], str[rp_bak]);
}
static GXDispMode GetNextDispMode(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    GXDispMode mode;
    int next = (rp + 1) % 3;
    if (next != wp) // 次バッファが書き込み中でない
    {
        rp = next;
    }
    mode = dispMode[rp];
    OS_RestoreInterrupts(enabled);
    //DebugReport();
    return mode;
}
static void* GetNextCaptureAddr(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    void* addr;
    int next = (wp + 1) % 3;
    if (wp_pending)         // 同じバッファに取り込みし直す
    {
        wp_pending = FALSE;
    }
    else if (next != rp)    // 次バッファが読み込み中でない
    {
        wp = next;
    }
    else    // 取り込むべきバッファがない (fpsからみてあり得ない)
    {
        OS_TPrintf("ERROR: there is no unused frame to capture.\n");
    }
    addr = dispAddr[wp];
    OS_RestoreInterrupts(enabled);
    //DebugReport();
    return addr;
}
static void PendingCapture(void)
{
    wp_pending = TRUE;
}
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
    GX_SetBankForLCDC(GX_VRAM_LCDC_C);
    GX_SetBankForLCDC(GX_VRAM_LCDC_D);
    MI_CpuClearFast(dispAddr[1], WIDTH*HEIGHT*2);   // first display point (before to capture)
    MI_CpuClearFast(dispAddr[2], WIDTH*HEIGHT*2);   // second display point (before to capture)
    wp = 0;
    rp = 2;
    wp_pending = TRUE;
    GX_SetGraphicsMode(GetNextDispMode(), GX_BGMODE_0, GX_BG0_AS_2D);
    //GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
    OS_WaitVBlankIntr();
    GX_DispOn();

    // カメラ初期化
    CAMERA_Init();      // wakeup camera module

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

    // カメラスタート (リクエストのみ)
    startRequest = TRUE;
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
            DebugReport();
        }
    }
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
    GX_SetGraphicsMode(GetNextDispMode(), GX_BGMODE_0, GX_BG0_AS_2D);
}

//--------------------------------------------------------------------------------
//    カメラ割り込み処理 (エラー時とVsync時の両方で発生)
//
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
        PendingCapture();       // 次回も同じフレームを使用する
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
            return; // waiting next frame (skip current frame)
        }
        // start to capture for next frame
        CAMERA_DmaRecvAsync(DMA_NO, GetNextCaptureAddr(), BYTES_PER_LINE * LINES_AT_ONCE, BYTES_PER_LINE * HEIGHT);
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
