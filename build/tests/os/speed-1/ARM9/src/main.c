/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS - demos - speed-1
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

#define MY_SPIN_CYCLES  0x00080000

//  VBlank interrupt handler
static void VBlankIntr(void)
{
    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    OS_Init();

    OS_Printf("ARM9 starts.\n");

    OS_InitTick();

    //---- Vブランク設定
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    while (1)
    {
        OSTick start, end;

        OS_WaitVBlankIntr();

        OS_ChangeSpeedOfARM9( OS_SPEED_ARM9_X1, (void*)(HW_ITCM + HW_ITCM_SIZE/2) );
        start = OS_GetTick();
        OS_SpinWait( MY_SPIN_CYCLES );
        end = OS_GetTick();

        OS_Printf( "SpinWait period of ARM9 X1: %d us\n", OS_TicksToMicroSeconds( end - start ) );

        OS_WaitVBlankIntr();

        OS_ChangeSpeedOfARM9( OS_SPEED_ARM9_X2, (void*)(HW_ITCM + HW_ITCM_SIZE/2) );
        start = OS_GetTick();
        OS_SpinWait( MY_SPIN_CYCLES );
        end = OS_GetTick();

        OS_Printf( "SpinWait period of ARM9 X2: %d us\n", OS_TicksToMicroSeconds( end - start ) );
    }

    // done
    OS_TPrintf("\nARM9 ends.\n");
    OS_Terminate();
}

