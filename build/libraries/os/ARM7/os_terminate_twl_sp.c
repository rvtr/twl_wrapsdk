/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS
  File:     os_terminate_sp.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: os_terminate_sp.c,v $
  Revision 1.2  2006/01/18 02:11:30  kitase_hirotake
  do-indent

  Revision 1.1  2006/01/10 05:58:35  okubata_ryoma
  os_terminate_sp.c‚Ì’Ç‰Á

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro/os/common/system.h>
#include <nitro/ctrdg/ARM7/ctrdg_sp.h>

//============================================================================
//          TERMINATE and HALT
//============================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_Terminate

  Description:  Halt CPU and loop
  
  Arguments:    None

  Returns:      --  (Never return)
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void OS_Terminate(void)
{
    //VIB_STOP
    CTRDG_VibPulseEdgeUpdate(NULL);

    while (1)
    {
        (void)OS_DisableInterrupts();
        OS_Halt();
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_Exit

  Description:  Display exit string and Terminate.
                This is useful for 'loadrun' tool command.                 
  
  Arguments:    status : exit status

  Returns:      --  (Never return)
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void OS_Exit(int status)
{
#ifdef SDK_FINALROM
#pragma unused( status )
#endif
    (void)OS_DisableInterrupts();
    OS_Printf("\n" OS_EXIT_STRING, status);
    OS_Terminate();
}

/*---------------------------------------------------------------------------*
  Name:         OS_Halt

  Description:  Halt CPU
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef SDK_DEBUGGER_ARM
#include <nitro/code32.h>
SDK_WEAK_SYMBOL asm void OS_Halt( void )
{
        mov     r0, #0
        mcr     p15, 0, r0, c7, c0, 4
        bx      lr
}
#include <nitro/codereset.h>
#endif
