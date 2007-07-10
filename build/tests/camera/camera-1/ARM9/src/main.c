/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - capture
  File:     main.c

  Copyright 2005,2006 Nintendo.  All rights reserved.

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

#define DMA_NO  5
#define WIDTH   256
#define HEIGHT  192

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
    {
        u16 bg_color = GX_RGB(31, 0, 0);
        GX_LoadBGPltt(&bg_color, 0, sizeof(u16));
    }

    // VRAM表示モード
    GX_SetBankForLCDC(GX_VRAM_LCDC_A);
    MI_CpuClearFast((void *)HW_LCDC_VRAM_A, WIDTH*HEIGHT*2);
    GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
    OS_WaitVBlankIntr();
    GX_DispOn();

    // カメラ初期化
    CAMERA_Init();
    CAMERA_PowerOn();

    result = CAMERA_I2CInit(CAMERA_SELECT_BOTH);
    if (result != CAMERA_RESULT_SUCCESS_TRUE)
    {
        OS_TPrintf("CAMERA_I2CInit was failed. (%d)\n", result);
    }
    else
    {
        OS_TPrintf("CAMERA_I2CInit was done.\n");
        CAMERA_PowerOff();
        OS_Terminate();
    }

    CAMERA_SetTrimmingParamsCenter(WIDTH, HEIGHT, 640, 480);    // clipped by camera i/f
    CAMERA_SetTrimming(TRUE);
    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // カメラ割り込み設定
    CAMERA_SetVsyncIntrrupt(CAMERA_INTR_VSYNC_NEGATIVE_EDGE);
    CAMERA_SetBufferErrorIntrrupt(TRUE);
    CAMERA_SetMasterIntrrupt(TRUE);
    OS_SetIrqFunction(OS_IE_CAM, CameraIntr);
    (void)OS_EnableIrqMask(OS_IE_CAM);

    // カメラスタート
    CAMERA_ClearBuffer();
    // DMA will be started by CameraIntr()
    CAMERA_Start();

    while (1)
    {
        OS_WaitVBlankIntr();
    }
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}
#define PRINT_RATE  32
void CameraIntr(void)
{
    if (CAMERA_GetErrorStatus())
    {
        OS_TPrintf("Error was occurred.\n");
        CAMERA_Stop();
        MIi_StopExDma(DMA_NO);
        while (CAMERA_IsBusy())
        {
        }
        CAMERA_ClearBuffer();
        CAMERA_DmaRecvAsync(DMA_NO, (void *)HW_LCDC_VRAM_A, CAMERA_GET_LINE_BYTES(WIDTH) * CAMERA_GET_MAX_LINES(WIDTH), CAMERA_GET_LINE_BYTES(WIDTH) * HEIGHT);
        CAMERA_Start();
    }
    else
    {
        static int count = 0;
        static OSTick prev = 0;
        static OSTick save[PRINT_RATE];
        OSTick current = OS_GetTick();
        if (MIi_IsExDmaBusy(DMA_NO))
        {
            OS_TPrintf("DMA was not done until VBlank.\n");
            OS_SetIrqCheckFlag(OS_IE_CAM);
            return;
        }
        CAMERA_DmaRecvAsync(DMA_NO, (void *)HW_LCDC_VRAM_A, CAMERA_GET_LINE_BYTES(WIDTH) * CAMERA_GET_MAX_LINES(WIDTH), CAMERA_GET_LINE_BYTES(WIDTH) * HEIGHT);
        if (count == PRINT_RATE)
        {
            int i;
            OSTick uspf = 0;
            for (i = 0; i < count; i++)
            {
                uspf += OS_TicksToMicroSeconds(save[i]);
            }
            uspf /= count;
            OS_TPrintf("%2d.%03d fps\n", (int)(1000000LL / uspf), (int)(1000000000LL / uspf) % 1000);
            count = 0;
        }
        if (prev)
            save[count++] = current - prev;
        prev = current;
    }
    OS_SetIrqCheckFlag(OS_IE_CAM); // checking camera interrupt
}
