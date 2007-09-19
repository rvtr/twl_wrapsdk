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

#define OSi_IDLE_CHECKNUM_SIZE  ( sizeof(u32)*2 )
#define OSi_IDLE_SVC_SIZE  ( sizeof(u32)*16 )   // arm7 svc stacks 14 words
#define OSi_IDLE_THREAD_STACK_SIZE    ( OSi_IDLE_CHECKNUM_SIZE + OSi_IDLE_SVC_SIZE )
extern u32     OSi_IdleThreadStack[OSi_IDLE_THREAD_STACK_SIZE / sizeof(u32)];
extern OSThread OSi_IdleThread;

/*---------------------------------------------------------------------------*
  Name:         OSi_IdleThreadProc

  Description:  procedure of idle thread which system creates

  Arguments:    None

  Returns:      None (never return)
 *---------------------------------------------------------------------------*/
static void OSi_IdleThreadProc(void *)
{
    (void)OS_EnableInterrupts();
    while (1)
    {
        OS_Halt();
    }
    // never return
}

//  VBlank interrupt handler
static void VBlankIntr(void)
{
    //---- 割り込みチェックフラグ
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         TwlSpMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlSpMain()
{
    OS_Init();

    OS_Printf("ARM7 starts.\n");

    // create idle thread to sleep in main thread
    OS_CreateThread(&OSi_IdleThread,
                    OSi_IdleThreadProc,
                    (void *)NULL,
                    OSi_IdleThreadStack + OSi_IDLE_THREAD_STACK_SIZE / sizeof(u32),
                    OSi_IDLE_THREAD_STACK_SIZE,
                    OS_THREAD_PRIORITY_MAX /*pseudo. change at next line. */ );
    OSi_IdleThread.priority = OS_THREAD_PRIORITY_MAX + 1;       // lower priority than the lowest (=OS_THREAD_PRIORITY_MAX)
    OSi_IdleThread.state = OS_THREAD_STATE_READY;

    //---- Vブランク設定
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    OS_InitTick();

    while (1)
    {
        OSTick start, end;

        OS_WaitVBlankIntr();

        start = OS_GetTick();
        OS_SpinWait( MY_SPIN_CYCLES );
        end = OS_GetTick();

        OS_Printf( "SpinWait period of ARM7: %d ms\n", OS_TicksToMicroSeconds( end - start ) );
    }

    // done
    OS_TPrintf("\nARM7 ends.\n");
    OS_Terminate();
}
