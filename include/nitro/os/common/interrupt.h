/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     interrupt.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  Revision 1.62  2006/07/05 09:13:49  yosizaki
  fix about include header.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_INTERRUPT_H_
#define TWL_OS_INTERRUPT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/ioreg.h>
#include <nitro/os/common/thread.h>

//---- for DMA, TIMER
void    OSi_EnterDmaCallback(u32 dmaNo, void (*callback) (void *), void *arg);
void    OSi_EnterTimerCallback(u32 timerNo, void (*callback) (void *), void *arg);

//----------------------------------------------------------------------
//                  ENABLE/DISABLE flag for IME
//----------------------------------------------------------------------
#define OS_IME_DISABLE      (0UL << REG_OS_IME_IME_SHIFT)
#define OS_IME_ENABLE       (1UL << REG_OS_IME_IME_SHIFT)

//----------------------------------------------------------------------
//                  IE/IF flags
//----------------------------------------------------------------------
#define OS_IE_V_BLANK       (1UL << REG_OS_IE_VB_SHIFT) // VBlank
#define OS_IE_H_BLANK       (1UL << REG_OS_IE_HB_SHIFT) // HBlank
#define OS_IE_V_COUNT       (1UL << REG_OS_IE_VE_SHIFT) // VCounter
#define OS_IE_TIMER0        (1UL << REG_OS_IE_T0_SHIFT) // timer0
#define OS_IE_TIMER1        (1UL << REG_OS_IE_T1_SHIFT) // timer1
#define OS_IE_TIMER2        (1UL << REG_OS_IE_T2_SHIFT) // timer2
#define OS_IE_TIMER3        (1UL << REG_OS_IE_T3_SHIFT) // timer3
#define OS_IE_DMA0          (1UL << REG_OS_IE_D0_SHIFT) // DMA0
#define OS_IE_DMA1          (1UL << REG_OS_IE_D1_SHIFT) // DMA1
#define OS_IE_DMA2          (1UL << REG_OS_IE_D2_SHIFT) // DMA2
#define OS_IE_DMA3          (1UL << REG_OS_IE_D3_SHIFT) // DMA3
#define OS_IE_KEY           (1UL << REG_OS_IE_K_SHIFT)  // key
#define OS_IE_CARTRIDGE     (1UL << REG_OS_IE_I_D_SHIFT)        // cartridge
#define OS_IE_SUBP          (1UL << REG_OS_IE_A7_SHIFT) // sub processor
#define OS_IE_SPFIFO_SEND   (1UL << REG_OS_IE_IFE_SHIFT)        // sub processor send FIFO empty
#define OS_IE_SPFIFO_RECV   (1UL << REG_OS_IE_IFN_SHIFT)        // sub processor receive FIFO not empty
#define OS_IE_CARD_DATA     (1UL << REG_OS_IE_MC_SHIFT) // card data transfer finish
#define OS_IE_CARD_IREQ     (1UL << REG_OS_IE_MI_SHIFT) // card IREQ

#define OS_IE_CARD_DET      (1UL << REG_OS_IE_MC_DET_SHIFT)   // card detect
#define OS_IE_CARD_B_DATA   (1UL << REG_OS_IE_MC_B_SHIFT) // card B data transfer finish
#define OS_IE_CARD_B_IREQ   (1UL << REG_OS_IE_MI_B_SHIFT) // card B IREQ
#define OS_IE_CARD_B_DET    (1UL << REG_OS_IE_MC_B_DET_SHIFT) // card B detect
#define OS_IE_DMA4          (1UL << REG_OS_IE_D4_SHIFT) // DMA4
#define OS_IE_DMA5          (1UL << REG_OS_IE_D5_SHIFT) // DMA5
#define OS_IE_DMA6          (1UL << REG_OS_IE_D6_SHIFT) // DMA6
#define OS_IE_DMA7          (1UL << REG_OS_IE_D7_SHIFT) // DMA7

#ifdef SDK_ARM9
#define OS_IE_GXFIFO        (1UL << REG_OS_IE_GF_SHIFT) // geometry command FIFO

#define OS_IE_DSP           (1UL << REG_OS_IE_DSP_SHIFT) // DSP
#define OS_IE_CAM           (1UL << REG_OS_IE_CAM_SHIFT) // CAM
#define OS_IRQ_TABLE_MAX     32

#else  //SDK_ARM7
#define OS_IE_SIO           (1UL << 7                 ) // serial communication (will not occur)
#define OS_IE_POWERMAN      (1UL << REG_OS_IE_PM_SHIFT) // Power Management IC
#define OS_IE_SPI           (1UL << REG_OS_IE_SPI_SHIFT)        // SPI data transfer
#define OS_IE_WIRELESS      (1UL << REG_OS_IE_WL_SHIFT) // Wireless module

#define OS_IE_SD            (1ULL << (REG_OS_IE2_SD_SHIFT + 32))    // Memory SD
#define OS_IE_SDIO          (1ULL << (REG_OS_IE2_SDIO_SHIFT + 32))  // Memory SDIO
#define OS_IE_WSD           (1ULL << (REG_OS_IE2_WSD_SHIFT + 32))   // New wireless SD
#define OS_IE_WSDIO         (1ULL << (REG_OS_IE2_WSDIO_SHIFT + 32)) // New wireless SDIO
#define OS_IE_I2C           (1ULL << (REG_OS_IE2_I2C_SHIFT + 32)) // I2C
#define OS_IE_AES           (1ULL << (REG_OS_IE2_AES_SHIFT + 32)) // AES
#define OS_IE_MIC           (1ULL << (REG_OS_IE2_MIC_SHIFT + 32)) // MIC

#define OS_IE_GPIO18_0      (1ULL << (REG_OS_IE2_IO18_0_SHIFT + 32)) // GPIO18_0
#define OS_IE_GPIO18_1      (1ULL << (REG_OS_IE2_IO18_1_SHIFT + 32)) // GPIO18_1
#define OS_IE_GPIO18_2      (1ULL << (REG_OS_IE2_IO18_2_SHIFT + 32)) // GPIO18_2
#define OS_IE_GPIO33_0      (1ULL << (REG_OS_IE2_IO33_0_SHIFT + 32)) // GPIO33_0
#define OS_IE_GPIO33_1      (1ULL << (REG_OS_IE2_IO33_1_SHIFT + 32)) // GPIO33_1
#define OS_IE_GPIO33_2      (1ULL << (REG_OS_IE2_IO33_2_SHIFT + 32)) // GPIO33_2
#define OS_IE_GPIO33_3      (1ULL << (REG_OS_IE2_IO33_3_SHIFT + 32)) // GPIO33_3
#define OS_IRQ_TABLE_MAX     47

// Aliases to SUBP->MAINP
#define OS_IE_MAINP             OS_IE_SUBP
#define OS_IE_MPFIFO_SEND       OS_IE_SPFIFO_SEND
#define OS_IE_MPFIFO_RECV       OS_IE_SPFIFO_RECV
#define REG_OS_IE_A9_SHIFT      REG_OS_IE_A7_SHIFT
#endif

// Aliases to common naming
#define OS_IE_FIFO_SEND         OS_IE_SPFIFO_SEND
#define OS_IE_FIFO_RECV         OS_IE_SPFIFO_RECV

// Dummy irq check flag for user own use
#define OS_IE_USER_FLAG0_SHIFT  31
#define OS_IE_USER_FLAG1_SHIFT  30
#define OS_IE_USER_FLAG0        (1UL << OS_IE_USER_FLAG0_SHIFT) // user 0
#define OS_IE_USER_FLAG1        (1UL << OS_IE_USER_FLAG1_SHIFT) // user 1


//---- used for internal functions
#define OSi_IRQCALLBACK_NO_DMA0    0
#define OSi_IRQCALLBACK_NO_DMA1    1
#define OSi_IRQCALLBACK_NO_DMA2    2
#define OSi_IRQCALLBACK_NO_DMA3    3
#define OSi_IRQCALLBACK_NO_TIMER0  4
#define OSi_IRQCALLBACK_NO_TIMER1  5
#define OSi_IRQCALLBACK_NO_TIMER2  6
#define OSi_IRQCALLBACK_NO_TIMER3  7
#ifdef SDK_ARM7
#define OSi_IRQCALLBACK_NO_VBLANK  8
#endif
#ifdef SDK_ARM9
#define OSi_IRQCALLBACK_NUM       (7+1)
#else
#define OSi_IRQCALLBACK_NUM       (8+1)
#endif

//----------------------------------------------------------------
#define OS_IRQ_MAIN_BUFFER_SIZE         (0x200)


//---- irq factor type define
#ifndef OSi_OSIRQMASK_DEFINED
#ifdef SDK_ARM9
typedef u32 OSIrqMask;
#else // SDK_ARM7
typedef u64 OSIrqMask;
#endif
#define OSi_OSIRQMASK_DEFINED
#endif

//---- interrupt handler type
typedef void (*OSIrqFunction) (void);

//---- for irq callback (internal use)
typedef struct
{
    void    (*func) (void *);
    void   *arg;
    OSIrqMask     enable;
}
OSIrqCallbackInfo;

//---- table of irq functions
extern OSIrqFunction OS_IRQTable[];

//---- for DMA, TIMER, VBLANK(arm7) interrupt */
extern OSIrqCallbackInfo OSi_IrqCallbackInfo[OSi_IRQCALLBACK_NUM];


void    OS_IrqDummy(void);
void    OS_IrqHandler(void);
void    OS_IrqHandler_ThreadSwitch(void);

//================================================================================
//              IRQ MASTER ENABLE
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableIrq

  Description:  enable master enable

  Arguments:    None

  Returns:      previous state of master enable.
                OS_IME_ENABLE or OS_IME_DISABLE.
 *---------------------------------------------------------------------------*/
static inline BOOL OS_EnableIrq(void)
{
    u16     prep = reg_OS_IME;
    reg_OS_IME = OS_IME_ENABLE;
    return (BOOL)prep;
}

/*---------------------------------------------------------------------------*
  Name:         OS_DisableIrq

  Description:  disable master enable

  Arguments:    None

  Returns:      previous status of master enable.
                OS_IME_ENABLE or OS_IME_DISABLE.
 *---------------------------------------------------------------------------*/
static inline BOOL OS_DisableIrq(void)
{
    u16     prep = reg_OS_IME;
    reg_OS_IME = OS_IME_DISABLE;
    return (BOOL)prep;
}

/*---------------------------------------------------------------------------*
  Name:         OS_RestoreIrq

  Description:  set master enable.
                this function is mainly used for restore previous state
                from OS_EnableIrq() or OS_DisableIrq().

  Arguments:    enable    OS_IME_ENABLE or OS_IME_DISABLE

  Returns:      previous state of master enable
  
 *---------------------------------------------------------------------------*/
static inline BOOL OS_RestoreIrq(BOOL enable)
{
    u16     prep = reg_OS_IME;
    reg_OS_IME = (u16)enable;
    return (BOOL)prep;
}


//================================================================================
//              IRQ FACTORS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SetIrqMask

  Description:  set irq factor

  Arguments:    intr        irq factor

  Returns:      previous factors
 *---------------------------------------------------------------------------*/
OSIrqMask OS_SetIrqMask(OSIrqMask intr);

/*---------------------------------------------------------------------------*
  Name:         OS_GetIrqMask

  Description:  get irq factor

  Arguments:    None

  Returns:      irq factor which is set now
 *---------------------------------------------------------------------------*/
static inline OSIrqMask OS_GetIrqMask(void)
{
#ifdef SDK_ARM9
    return reg_OS_IE;
#else // SDK_ARM7
    return ((OSIrqMask)reg_OS_IE2 << 32) | reg_OS_IE;
#endif // SDK_ARM7
}

/*---------------------------------------------------------------------------*
  Name:         OS_EnableIrqMask

  Description:  set specified irq factor

  Arguments:    intr        irq factor

  Returns:      previous factors
 *---------------------------------------------------------------------------*/
OSIrqMask OS_EnableIrqMask(OSIrqMask intr);

/*---------------------------------------------------------------------------*
  Name:         OS_DisableIrqMask

  Description:  unset specified irq factor

  Arguments:    intr        irq factor

  Returns:      previous factors
 *---------------------------------------------------------------------------*/
OSIrqMask OS_DisableIrqMask(OSIrqMask intr);

//================================================================================
//              IF
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_ResetRequestIrqMask

  Description:  reset IF bit
                (setting bit causes to clear bit for interrupt)

  Arguments:    intr        irq factor

  Returns:      previous factors
 *---------------------------------------------------------------------------*/
OSIrqMask OS_ResetRequestIrqMask(OSIrqMask intr);

/*---------------------------------------------------------------------------*
  Name:         OS_GetReuestIrqMask

  Description:  get IF bit

  Arguments:    None

  Returns:      value of IF
 *---------------------------------------------------------------------------*/
static inline OSIrqMask OS_GetRequestIrqMask(void)
{
#ifdef SDK_ARM9
    return reg_OS_IF;
#else // SDK_ARM7
    return ((OSIrqMask)reg_OS_IF2 << 32) | reg_OS_IF;
#endif // SDK_ARM7
}

//================================================================================
//              IRQ HANDLER
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_InitIrqTable

  Description:  initialize irq table

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_InitIrqTable(void);

/*---------------------------------------------------------------------------*
  Name:         OS_SetIrqFunction

  Description:  set irq handler for specified interrupt

  Arguments:    intrBit    irq factor
                function   irq handler for specified interrupt

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetIrqFunction(OSIrqMask intrBit, OSIrqFunction function);


/*---------------------------------------------------------------------------*
  Name:         OS_GetIrqFunction

  Description:  get irq handler for specified interrupt

  Arguments:    intrBit    irq factor

  Returns:      irq handler for specified interrupt
 *---------------------------------------------------------------------------*/
OSIrqFunction OS_GetIrqFunction(OSIrqMask intrBit);


//================================================================================
//              IRQ CHEKE BUFFER
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SetIrqCheckFlag

  Description:  set irq flag to check being called

  Arguments:    irq factors to be set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void OS_SetIrqCheckFlag(OSIrqMask intr)
{
    *(vu32 *)HW_INTR_CHECK_BUF  |= (u32)intr;
#ifdef SDK_ARM7
    *(vu32 *)HW_INTR_CHECK2_BUF |= (u32)(intr >> 32);
#endif // SDK_ARM7
}

/*---------------------------------------------------------------------------*
  Name:         OS_ClearIrqCheckFlag

  Description:  clear irq flag stored in HW_INTR_CHECK_BUF

  Arguments:    irq factors to be cleared

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void OS_ClearIrqCheckFlag(OSIrqMask intr)
{
    *(vu32 *)HW_INTR_CHECK_BUF  &= ~(u32)intr;
#ifdef SDK_ARM7
    *(vu32 *)HW_INTR_CHECK2_BUF &= ~(u32)(intr >> 32);
#endif // SDK_ARM7
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetIrqCheckFlag

  Description:  get irq factors stored in HW_INTR_CHECK_BUF

  Arguments:    None

  Returns:      irq flags factors in HW_INTR_CHECK_BUG
 *---------------------------------------------------------------------------*/
static inline OSIrqMask OS_GetIrqCheckFlag(void)
{
    OSIrqMask intr = *(vu32 *)HW_INTR_CHECK_BUF;

#ifdef SDK_ARM7
    intr |= (OSIrqMask)(*(vu32 *)HW_INTR_CHECK2_BUF) << 32;
#endif // SDK_ARM7

    return intr;
}


//================================================================================
//              WAIT IRQ
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_WaitIrq

  Description:  wait specified interrupt
                the difference between OS_WaitIrq and OS_WaitInterrupt,
                in waiting interrupt
                OS_WaitIrq does switch thread,
                OS_WaitInterrupt doesn't switch thread.
                OS_WaitIrq wait by using OS_SleepThread() with threadQueue,
                OS_WaitInterrupt wait by using OS_Halt().
                if SDK_NO_THREAD defined, 2 functions become same.

  Arguments:    clear       TRUE if want to clear interrupt flag before wait
                            FALSE if not
                irqFlags    bit of interrupts to wait for

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_WaitIrq(BOOL clear, OSIrqMask irqFlags);

/*---------------------------------------------------------------------------*
  Name:         OS_WaitAnyIrq

  Description:  wait any interrupt
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_WaitAnyIrq(void);


//================================================================================
//              VBLANK COUNTER
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_GetVBlankCount

  Description:  get VBlankCount
  
  Arguments:    None

  Returns:      VBlankCount
 *---------------------------------------------------------------------------*/
static inline u32 OS_GetVBlankCount(void)
{
    return *(vu32 *)HW_VBLANK_COUNT_BUF;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_SetVBlankCount

  Description:  set VBlankCount
                *** internal function. don't use this.

  Arguments:    count : VBlankCount

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void OSi_SetVBlankCount(u32 count)
{
    *(u32 *)HW_VBLANK_COUNT_BUF = count;
}

//================================================================================
//              IRQ STACK CHECKER
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SetIrqStackWarningOffset

  Description:  Set warning level for irq stack checker

  Arguments:    offset     offset from stack top. must be multiple of 4

  Returns:      None
 *---------------------------------------------------------------------------*/
extern void OS_SetIrqStackWarningOffset(u32 offset);

/*---------------------------------------------------------------------------*
  Name:         OS_GetIrqStackStatus

  Description:  check irq stack. check each CheckNUM.
                return result.

  Arguments:    None

  Returns:      0 (OS_STACK_NO_ERROR)        no error
                OS_STACK_OVERFLOW            overflow
                OS_STACK_ABOUT_TO_OVERFLOW   about to overflow
                OS_STACK_UNDERFLOW           underflow
 *---------------------------------------------------------------------------*/
extern OSStackStatus OS_GetIrqStackStatus(void);

/*---------------------------------------------------------------------------*
  Name:         OS_SetIrqStackChecker

  Description:  set irq stack check number to irq stack

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
extern void OS_SetIrqStackChecker(void);

/*---------------------------------------------------------------------------*
  Name:         OS_CheckIrqStack

  Description:  check irq stack. check each CheckNum.
                if changed, display warning and halt.

  Arguments:    None

  Returns:      None.
                ( if error occurred, never return )
 *---------------------------------------------------------------------------*/
void    OSi_CheckIrqStack(char *file, int line);
#if !defined(SDK_FINALROM) && !defined(SDK_NO_MESSAGE)
#define  OS_CheckIrqStack()  OSi_CheckIrqStack( __FILE__, __LINE__ );
#else
#define  OS_CheckIrqStack()  ((void)0)
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_INTERRUPT_H_ */
#endif
