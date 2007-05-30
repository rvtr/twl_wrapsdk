
#ifndef __RTFS_TARGET_OS_H__
#define __RTFS_TARGET_OS_H__


/***********************************************************************
 ターゲットOS指定
***********************************************************************/
#define TARGET_OS_NITRO    (1)
#define TARGET_OS_CTR      (TARGET_OS_NITRO ^ 1)


/***********************************************************************
 NITRO OSのとき 
***********************************************************************/
#if (TARGET_OS_NITRO == 1)

/*    #if( DEBUG_PRINT_ON == 1)
        #define PRINTDEBUG          OS_TPrintf
    #else
        #define PRINTDEBUG( ...)    ((void)0)
    #endif
*/

    #define OSAPI_CPUFILL8         MI_CpuFill8
    #define OSAPI_CPUCOPY8         MI_CpuCopy8
    #define OSAPI_MALLOC           OS_Alloc
    #define OSAPI_FREE             OS_Free
    #define OSAPI_STRLEN           STD_GetStringLength
    #define OSAPI_STRNCMP          STD_CompareNString
    #define OSAPI_STRCMP           STD_CompareString
    #define OSAPI_FLUSHCACHEALL    DC_FlushAll
    #define OSAPI_WAITCACHEBUF     DC_WaitWriteBufferEmpty
    #define OSAPI_ENABLEINTR       OS_EnableInterrupts
    #define OSAPI_DISABLEINTR      OS_DisableInterrupts
    #define OSAPI_RESTOREINTR      OS_RestoreInterrupts
    #define OSAPI_RTCINIT          RTC_Init


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
    #define OSAPI_ENABLEINTR       osEnableInterrupts
    #define OSAPI_DISABLEINTR      osDisableInterrupts
    #define OSAPI_RESTOREINTR      osRestoreInterrupts
    #define OSAPI_RTCINIT          rtcInit


#endif

#endif /*__RTFS_TARGET_OS_H__*/
