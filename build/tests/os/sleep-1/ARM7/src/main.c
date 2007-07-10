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

/*---------------------------------------------------------------------------*
  Name:         TwlSpMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlSpMain()
{
    u16 trg;
    u16 old = 0;

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

    OS_InitTick();
    OS_InitAlarm();
    OS_EnableIrq();
    OS_EnableInterrupts();

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
    OS_TPrintf("\nARM7 ends.\n");
    OS_Terminate();
}
