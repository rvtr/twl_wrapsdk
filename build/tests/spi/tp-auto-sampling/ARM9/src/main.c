/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - tests - channel
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
#include <nitro/spi/ARM9/tp.h>

void    VBlankIntr(void);
void    DrawPos(int x, int y);
void    DrawPosScreen(int x, int y);

#define     SAMPLING_FREQUENCE      4  // Touch panel sampling frequence per frame
#define     SAMPLING_BUFSIZE        (SAMPLING_FREQUENCE + 1)    // AutoSampling buffer size
#define     SAMPLING_START_VCOUNT   0  // base vcount value in auto sampling.

#define abs(x) (x) > 0 ? (x) : -(x)

typedef void*  (*Func)(void);

void* Sequence1(void);
void* Sequence2(void);
void* Sequence3(void);
void* Sequence4(void);

#define CALIBRATE_SCREEN_X1 20
#define CALIBRATE_SCREEN_Y1 20
#define CALIBRATE_SCREEN_X2 236
#define CALIBRATE_SCREEN_Y2 171

/*---------------------------------------------------------------------------*
    Static variables definition
 *---------------------------------------------------------------------------*/
static TPData gTpBuf[SAMPLING_BUFSIZE];
Func func = Sequence1;
u16 caliberate_raw_x1;
u16 caliberate_raw_y1;
u16 caliberate_raw_x2;
u16 caliberate_raw_y2;

TPCalibrateParam calibrate;

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    OS_InitPrintServer();

    // 初期化
    OS_Init();
    GX_Init();
    TP_Init();

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // Send parameter of revision noise.
    if (TP_RequestSetStability(3, 15) != 0)
    {
        OS_Panic("SetStability request err!\n");
    }

    GX_DispOff();
    GXS_DispOff();

    //---------------------------------------------------------------------------
    // All VRAM banks to LCDC
    //---------------------------------------------------------------------------
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    //---------------------------------------------------------------------------
    // Clear all LCDC space
    //---------------------------------------------------------------------------
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    //---------------------------------------------------------------------------
    // Set graphics mode VRAM display mode
    //---------------------------------------------------------------------------
    GX_SetGraphicsMode(GX_DISPMODE_VRAM_A,      // display VRAM-A
                       (GXBGMode)0,    			// dummy
                       (GXBG0As)0);    			// dummy

	GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
    GX_DispOn();
    GXS_DispOn();

    // send auto sampling start request.
    if (TP_RequestAutoSamplingStart(SAMPLING_START_VCOUNT, SAMPLING_FREQUENCE,
                                    gTpBuf, SAMPLING_BUFSIZE))
    {
        OS_Panic("auto sampling start reqeuest err!\n");
    }
    OS_Printf("Start auto sampling\n");


	// メインループ
    while (1)
    {
        OS_WaitVBlankIntr();

		// 画面全体クリア
		MI_DmaClear32(3, (void *)HW_LCDC_VRAM, 256*192*2);
		// シーケンス処理
		func = func();
    }
}

void* Sequence1(void)
{
	s32 last_idx = TP_GetLatestIndexInAuto();

	DrawPosScreen(CALIBRATE_SCREEN_X1, CALIBRATE_SCREEN_Y1);

	if (gTpBuf[last_idx].touch)
	{
		if (abs(caliberate_raw_x1 - gTpBuf[last_idx].x) < 1 &&
            abs(caliberate_raw_y1 - gTpBuf[last_idx].y) < 1 )
		{
			DrawPos(caliberate_raw_x1, caliberate_raw_y1);
			return Sequence2;
		}
		else
		{
			caliberate_raw_x1 = gTpBuf[last_idx].x;
			caliberate_raw_y1 = gTpBuf[last_idx].y;
		}
	}
	return Sequence1;
}

void* Sequence2(void)
{
	s32 last_idx = TP_GetLatestIndexInAuto();

	if (!gTpBuf[last_idx].touch)
	{
		return Sequence3;
	}
	return Sequence2;
}

void* Sequence3(void)
{
	s32 last_idx = TP_GetLatestIndexInAuto();

	DrawPosScreen(CALIBRATE_SCREEN_X2, CALIBRATE_SCREEN_Y2);

	if (gTpBuf[last_idx].touch)
	{
		if (abs(caliberate_raw_x2 - gTpBuf[last_idx].x) < 1 &&
            abs(caliberate_raw_y2 - gTpBuf[last_idx].y) < 1 )
		{
			DrawPos(caliberate_raw_x2, caliberate_raw_y2);

			TP_CalcCalibrateParam( &calibrate,
		                           caliberate_raw_x1,   caliberate_raw_y1,
		                           CALIBRATE_SCREEN_X1, CALIBRATE_SCREEN_Y1,
		                           caliberate_raw_x2,   caliberate_raw_y2,
		                           CALIBRATE_SCREEN_X2, CALIBRATE_SCREEN_Y2 );

			TP_SetCalibrateParam( &calibrate );

			return Sequence4;
		}
		else
		{
			caliberate_raw_x2 = gTpBuf[last_idx].x;
			caliberate_raw_y2 = gTpBuf[last_idx].y;
		}
	}
	return Sequence3;
}

void* Sequence4(void)
{
	s32 last_idx = TP_GetLatestIndexInAuto();

	if (gTpBuf[last_idx].touch)
	{
/*
		///////////
		if (gTpBuf[last_idx].x < 300)   { OS_Printf("0: %4d\n", gTpBuf[last_idx].x); }
		if (gTpBuf[last_idx].x > 3500)  { OS_Printf("0: %4d\n", gTpBuf[last_idx].x); }
		if (gTpBuf[last_idx].y < 300)   { OS_Printf("0: %4d\n", gTpBuf[last_idx].y); }
		if (gTpBuf[last_idx].y > 3500)  { OS_Printf("0: %4d\n", gTpBuf[last_idx].y); }
		///////////
*/
		TP_GetCalibratedPoint( &gTpBuf[last_idx], &gTpBuf[last_idx] );
/*
		///////////
		if (gTpBuf[last_idx].x < 1)   { OS_Printf("1: %4d\n", gTpBuf[last_idx].x); }
		if (gTpBuf[last_idx].x > 254) { OS_Printf("1: %4d\n", gTpBuf[last_idx].x); }
		if (gTpBuf[last_idx].y < 1)   { OS_Printf("1: %4d\n", gTpBuf[last_idx].y); }
		if (gTpBuf[last_idx].y > 190) { OS_Printf("1: %4d\n", gTpBuf[last_idx].y); }
		///////////
*/
		DrawPosScreen(gTpBuf[last_idx].x, gTpBuf[last_idx].y);
	}
	return Sequence4;
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

//--------------------------------------------------------------------------------
//    画面座標描画
//
void DrawPos(int x, int y)
{
    const int screen_width  = 256;
    const int screen_height = 191;
    const int axis_max = 4096;
	int adjust_x  = x * screen_width  / axis_max;
	int adjust_y  = y * screen_height / axis_max;
    int address;
	int i;

	address = HW_LCDC_VRAM + screen_width*2*adjust_y;
	MI_DmaFill16( 3, (void*)address, 0x7fff, 256*2 );

	for (i=0;i<192;i++)
	{
		address = HW_LCDC_VRAM + screen_width*2*i + 2*adjust_x;
		*(u16 *)(address) = 0x7fff;
	}
}

void DrawPosScreen(int x, int y)
{
    const int screen_width  = 256;
    int address;
	int i;

	address = HW_LCDC_VRAM + screen_width*2*y;
	MI_DmaFill16( 3, (void*)address, 0x7fff, 256*2 );

	for (i=0;i<192;i++)
	{
		address = HW_LCDC_VRAM + screen_width*2*i + 2*x;
		*(u16 *)(address) = 0x7fff;
	}
}
