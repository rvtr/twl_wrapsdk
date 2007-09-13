/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - capture
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

static void VBlankIntr(void);

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    // 初期化
    OS_Init();
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
