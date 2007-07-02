/*---------------------------------------------------------------------------*
  Project:  CTR - ELF Loader Configuration
  File:     el_config.h

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef __ELF_LOADER_CONFIG_H__
#define __ELF_LOADER_CONFIG_H__

/***********************************************************************
 デバッグプリント ON/OFF
***********************************************************************/
#define DEBUG_PRINT_ON     (0)

/***********************************************************************
 ターゲットOS指定
***********************************************************************/
#define TARGET_OS_NITRO    (1)
#define TARGET_OS_CTR      (TARGET_OS_NITRO ^ 1)

/***********************************************************************
 ターゲットARMアーキテクチャ指定（ARM7TDMIはV4にする）
***********************************************************************/
#ifdef SDK_ARM7
  #define TARGET_ARM_V4      (1)
#else
  #define TARGET_ARM_V4      (0)
#endif

#define TARGET_ARM_V5      (TARGET_ARM_V4 ^ 1)


/***********************************************************************
 NITRO OSのとき 
***********************************************************************/
#if (TARGET_OS_NITRO == 1)

    #if( DEBUG_PRINT_ON == 1)
        #define PRINTDEBUG          OS_TPrintf
    #else
        #define PRINTDEBUG( ...)    ((void)0)
    #endif


    #define OSAPI_CPUFILL8         MI_CpuFill8
    #define OSAPI_CPUCOPY8         MI_CpuCopy8
    #define OSAPI_MALLOC           OS_Alloc
    #define OSAPI_FREE             OS_Free
    #define OSAPI_STRLEN           STD_GetStringLength
    #define OSAPI_STRNCMP          STD_CompareNString
    #define OSAPI_STRCMP           STD_CompareString
    #define OSAPI_FLUSHCACHEALL    DC_FlushAll
    #define OSAPI_WAITCACHEBUF     DC_WaitWriteBufferEmpty


/***********************************************************************
 CTR OSのとき
***********************************************************************/
#else

    #if (DEBUG_PRINT_ON == 1)
        #if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
        #define PRINTDEBUG          osTPrintf
        #else
        #include <ctr/vlink.h>
        #define PRINTDEBUG          vlink_dos_printf
        #endif
    #else
        #define PRINTDEBUG( ...)    ((void)0)
    #endif


    #define OSAPI_CPUFILL8         miCpuFill8
    #define OSAPI_CPUCOPY8         miCpuCopy8
    #define OSAPI_MALLOC           i_elAlloc
    #define OSAPI_FREE             i_elFree
    #define OSAPI_STRLEN           strlen
    #define OSAPI_STRNCMP          strncmp
    #define OSAPI_STRCMP           strcmp
    #define OSAPI_FLUSHCACHEALL    osFlushDCacheAll
    #define OSAPI_WAITCACHEBUF     osWaitWriteBufferEmpty


#endif


#endif /*__ELF_LOADER_CONFIG_H__*/
