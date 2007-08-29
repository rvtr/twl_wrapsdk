/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - debugLED
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
    int i;

    OS_Init();

    OS_Printf("ARM7 starts.\n");

    OS_InitDebugLED();
    for (i = 0; i < 0x100; i++)
    {
        OS_SetDebugLED((u8)i);
        SVC_WaitByLoop(0x20000);
    }

    // done
    OS_TPrintf("\nARM7 ends.\n");
    OS_Terminate();
}
