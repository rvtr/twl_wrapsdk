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

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    TPData  raw_point;

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

    GX_DispOn();
    GXS_DispOn();

    while (1)
    {
        OS_WaitVBlankIntr();

		if (TP_RequestRawSampling(&raw_point) == 0)
		{
			DrawPos(raw_point.x, raw_point.y);
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
//    画面座標描画
//
void DrawPos(int x, int y)
{
    const int screen_width  = 256;
    const int screen_height = 191;
    const int axis_max = 4096;
	int adjust_x  = x * screen_width  / axis_max;
	int adjust_y  = y * screen_height / axis_max;
    int address = HW_LCDC_VRAM + screen_width*2*adjust_y + 2*adjust_x;

	*(u16 *)(address) = 0x7fff;
}
