/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS - demos - sleep-1
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

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    u16 trg;
    u16 old = 0;

    OS_Init();

    OS_Printf("ARM9 starts.\n");

    OS_InitTick();
    OS_InitAlarm();
    OS_EnableIrq();

    while (1)
    {
        u16 pad = PAD_Read();
        trg = (u16)(pad & ~old);
        old = pad;
        if (trg & PAD_BUTTON_A)
        {
            OS_TPrintf("call OS_Sleep(1000) ...");
            OS_Sleep(1000);
            OS_TPrintf(" Done.\n");
        }
        if (trg & PAD_BUTTON_START)
        {
            break;
        }
    }

    // done
    OS_TPrintf("\nARM9 ends.\n");
    OS_Terminate();
}
