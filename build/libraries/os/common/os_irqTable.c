/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - OS
  File:     os_irqTable.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro/os.h>

void    OSi_IrqCallback(int dmaNo);

void    OSi_IrqTimer0(void);
void    OSi_IrqTimer1(void);
void    OSi_IrqTimer2(void);
void    OSi_IrqTimer3(void);

void    OSi_IrqDma0(void);
void    OSi_IrqDma1(void);
void    OSi_IrqDma2(void);
void    OSi_IrqDma3(void);

#ifdef SDK_ARM7
void    OSi_IrqVBlank(void);
#endif

//---------------- Default jump table for IRQ interrupt
#ifdef SDK_ARM9
#include    <nitro/dtcm_begin.h>
#endif
OSIrqFunction OS_IRQTable[OS_IRQ_TABLE_MAX] = {
#ifdef SDK_ARM9
    OS_IrqDummy,                       // VBlank (for ARM9)
#else
    OSi_IrqVBlank,                     // VBlank (for ARM7)
#endif
    OS_IrqDummy,                       // HBlank
    OS_IrqDummy,                       // VCounter
    OSi_IrqTimer0,                     // timer0
    OSi_IrqTimer1,                     // timer1
    OSi_IrqTimer2,                     // timer2
    OSi_IrqTimer3,                     // timer3
    OS_IrqDummy,                       // serial communication (will not occur)
    OSi_IrqDma0,                       // DMA0
    OSi_IrqDma1,                       // DMA1
    OSi_IrqDma2,                       // DMA2
    OSi_IrqDma3,                       // DMA3
    OS_IrqDummy,                       // key
    OS_IrqDummy,                       // cartridge
    OS_IrqDummy,                       // (not used)
    OS_IrqDummy,                       // (not used)
    OS_IrqDummy,                       // sub processor
    OS_IrqDummy,                       // sub processor send FIFO empty
    OS_IrqDummy,                       // sub processor receive FIFO not empty
    OS_IrqDummy,                       // card data transfer finish
    OS_IrqDummy,                       // card IREQ
#ifdef SDK_ARM9
    OS_IrqDummy,                       // geometry command FIFO
    OS_IrqDummy,                       // (not used)
    OS_IrqDummy,                       // (not used)
    OS_IrqDummy,                       // DSP
    OS_IrqDummy,                       // CAM
#else
    OS_IrqDummy,                       // (not used)
    OS_IrqDummy,                       // Power Management IC
    OS_IrqDummy,                       // SPI data transfer
    OS_IrqDummy,                       // Wireless module
    OS_IrqDummy,                       // (not used)
#endif
    OS_IrqDummy,                       // card B data transfer finish
    OS_IrqDummy,                       // card B IREQ
    OS_IrqDummy,                       // DMA4
    OS_IrqDummy,                       // DMA5
    OS_IrqDummy,                       // DMA6
    OS_IrqDummy,                       // DMA7
#ifdef SDK_ARM7
    OS_IrqDummy,                       // GPIO18_0
    OS_IrqDummy,                       // GPIO18_1
    OS_IrqDummy,                       // GPIO18_2
    OS_IrqDummy,                       // (not used)
    OS_IrqDummy,                       // GPIO33_0
    OS_IrqDummy,                       // GPIO33_1
    OS_IrqDummy,                       // GPIO33_2
    OS_IrqDummy,                       // GPIO33_3
    OS_IrqDummy,                       // Memory SD
    OS_IrqDummy,                       // Memory SDIO
    OS_IrqDummy,                       // New wireless SD
    OS_IrqDummy,                       // New wireless SDIO
    OS_IrqDummy,                       // I2C
    OS_IrqDummy,                       // AES
    OS_IrqDummy,                       // MIC
#endif
};
#ifdef SDK_ARM9
#include    <nitro/dtcm_end.h>
#endif

#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/dtcm_begin.h>
#endif
//---------------- Jump table for DMA & TIMER & VBLANK interrupts
OSIrqCallbackInfo OSi_IrqCallbackInfo[OSi_IRQCALLBACK_NUM] = {
    {NULL, 0, 0,},                     // dma0
    {NULL, 0, 0,},                     // dma1
    {NULL, 0, 0,},                     // dma2
    {NULL, 0, 0,},                     // dma3

    {NULL, 0, 0,},                     // timer0
    {NULL, 0, 0,},                     // timer1
    {NULL, 0, 0,},                     // timer2
    {NULL, 0, 0,},                     // timer3
#ifdef SDK_ARM7
    {NULL, 0, 0,}                      // vblank
#endif
};

//----------------
static u16 OSi_IrqCallbackInfoIndex[OSi_IRQCALLBACK_NUM] = {
    REG_OS_IE_D0_SHIFT, REG_OS_IE_D1_SHIFT, REG_OS_IE_D2_SHIFT, REG_OS_IE_D3_SHIFT,
    REG_OS_IE_T0_SHIFT, REG_OS_IE_T1_SHIFT, REG_OS_IE_T2_SHIFT, REG_OS_IE_T3_SHIFT,
#ifdef SDK_ARM7
    REG_OS_IE_VB_SHIFT
#endif
};
#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/dtcm_end.h>
#endif


#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_begin.h>
#endif
/*---------------------------------------------------------------------------*
  Name:         OS_IrqDummy

  Description:  Dummy interrupt handler

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_IrqDummy(void)
{
    // do nothing
}


/*---------------------------------------------------------------------------*
  Name:         OSi_IrqCallback

  Description:  System interrupt handler

  Arguments:    system irq index

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_IrqCallback(int index)
{
    OSIrqMask imask = (1UL << OSi_IrqCallbackInfoIndex[index]);
    void    (*callback) (void *) = OSi_IrqCallbackInfo[index].func;

//OS_Printf( "irq %d\n", index  );
    //---- clear callback
    OSi_IrqCallbackInfo[index].func = NULL;

    //---- call callback
    if (callback)
    {
        (callback) (OSi_IrqCallbackInfo[index].arg);
    }

    //---- check IRQMask
    OS_SetIrqCheckFlag(imask);

    //---- restore IRQEnable
    if (!OSi_IrqCallbackInfo[index].enable)
    {
        (void)OS_DisableIrqMask(imask);
    }
}

/*---------------------------------------------------------------------------*
  Name:         OSi_IrqDma0 - OSi_IrqDma3

  Description:  DMA0 - DMA3 interrupt handler

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_IrqDma0(void)
{
    OSi_IrqCallback(OSi_IRQCALLBACK_NO_DMA0);
}

/*- - - - - - - - - - - - - - - - - - - - - - - */
void OSi_IrqDma1(void)
{
    OSi_IrqCallback(OSi_IRQCALLBACK_NO_DMA1);
}

/*- - - - - - - - - - - - - - - - - - - - - - - */
void OSi_IrqDma2(void)
{
    OSi_IrqCallback(OSi_IRQCALLBACK_NO_DMA2);
}

/*- - - - - - - - - - - - - - - - - - - - - - - */
void OSi_IrqDma3(void)
{
    OSi_IrqCallback(OSi_IRQCALLBACK_NO_DMA3);
}

/*---------------------------------------------------------------------------*
  Name:         OSi_IrqTimer0 - OSi_IrqTimer3

  Description:  TIMER0 - TIMER3 interrupt handler

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
/*- - - - - - - - - - - - - - - - - - - - - - - */
void OSi_IrqTimer0(void)
{
    OSi_IrqCallback(OSi_IRQCALLBACK_NO_TIMER0);
}

/*- - - - - - - - - - - - - - - - - - - - - - - */
void OSi_IrqTimer1(void)
{
    OSi_IrqCallback(OSi_IRQCALLBACK_NO_TIMER1);
}

/*- - - - - - - - - - - - - - - - - - - - - - - */
void OSi_IrqTimer2(void)
{
    OSi_IrqCallback(OSi_IRQCALLBACK_NO_TIMER2);
}

/*- - - - - - - - - - - - - - - - - - - - - - - */
void OSi_IrqTimer3(void)
{
    OSi_IrqCallback(OSi_IRQCALLBACK_NO_TIMER3);
}

#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_end.h>
#endif


/*---------------------------------------------------------------------------*
  Name:         OSi_VBlank

  Description:  VBLANK interrupt handler (for ARM7)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM7
void OSi_IrqVBlank(void)
{
    void    (*callback) (void) =
        (void (*)(void))OSi_IrqCallbackInfo[OSi_IRQCALLBACK_NO_VBLANK].func;

    //---- vblank counter
    (*(u32 *)HW_VBLANK_COUNT_BUF)++;

    //---- call callback
    if (callback)
    {
        (callback) ();
    }

    //---- check IRQMask
    OS_SetIrqCheckFlag(1UL << REG_OS_IE_VB_SHIFT);
}
#endif
