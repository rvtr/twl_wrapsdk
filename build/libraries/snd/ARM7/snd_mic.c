/*---------------------------------------------------------------------------*
  Project:  CtrSDK - MIC
  File:     mic.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/cdc.h>
#include <twl/snd/ARM7/snd_mic.h>


static void MICi_ExDmaRecvAsync( u32 dmaNo, void *dest, s32 size );
void MICi_ExDmaInterruptHandler( void );

void MICi_DmaInterruptHandler( void );
void MICi_FifoInterruptHandler( void );

static MICWork micWork;


/*---------------------------------------------------------------------------*
  Name:         MICi_Init

  Description:  initialize MIC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MICi_Init( void )
{
    CDC_InitMic();
}

/*---------------------------------------------------------------------------*
  Name:         MICi_Start

  Description:  start MIC

  Arguments:    

  Returns:      None
 *---------------------------------------------------------------------------*/
void MICi_Start( MICSampleRate smp, u32 dmaNo, void *dest, s32 size )
{
    MICWork *wp = &micWork;

    OSIntrMode enabled;

    wp->dmaNo = dmaNo;
    wp->buf = dest;
    wp->bufSize = size;

    MICi_Stop();

    enabled = OS_DisableInterrupts();

    if ( dest != NULL )
    {
        if ( MI_EXDMA_CH_MIN <= dmaNo && dmaNo <= MI_EXDMA_CH_MAX )
        {
            u32 ch = dmaNo + MI_EXDMA_CH_MIN;

            MIi_StopExDma( dmaNo );

            MICi_ExDmaRecvAsync( dmaNo, dest, size );

            OS_SetIrqFunction( OS_IE_DMA4 + ch, MICi_ExDmaInterruptHandler );

            reg_OS_IF  = (OS_IE_DMA4 << ch);
            reg_OS_IE |= (OS_IE_DMA4 << ch);        // enable mic dma interrupt
        }
    }

    SND_Enable();

    // start monoral sampling
    reg_SND_MICCNT = REG_SND_MICCNT_FIFO_CLR_MASK;
    reg_SND_MICCNT = (u16)(REG_SND_MICCNT_E_MASK | REG_SND_MICCNT_NR_MASK | MIC_INTR_OVERFLOW
                    | smp);

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         MICi_Stop

  Description:  stop MIC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MICi_Stop( void )
{
    MICWork *wp = &micWork;

    OSIntrMode enabled = OS_DisableInterrupts();

    if ( reg_SND_MICCNT & REG_SND_MICCNT_E_MASK )
    {
        u32 dmaNo = wp->dmaNo;

        reg_SND_MICCNT &= ~REG_SND_MICCNT_E_MASK;

        if ( MI_EXDMA_CH_MIN <= dmaNo && dmaNo <= MI_EXDMA_CH_MAX )
        {
            u32 ch = dmaNo + MI_EXDMA_CH_MIN;

            MIi_StopExDma( dmaNo );

            reg_OS_IE &= ~(OS_IE_DMA4 << ch);       // disable mic dma interrupt
            reg_OS_IF  =  (OS_IE_DMA4 << ch);
        }
        else if ( dmaNo > MI_EXDMA_CH_MAX )
        {
            reg_OS_IE2 &= ~(OS_IE_MIC >> 32);       // disable mic fifo interrupt
            reg_OS_IF2  =  (OS_IE_MIC >> 32);
        }
    }

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         MICi_ExDmaRecvAsync

  Description:  receive data with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static void MICi_ExDmaRecvAsync( u32 dmaNo, void *dest, s32 size )
{
    u32 interval = (0x2C0 * 16) - 16;
    MIExDmaPrescaler prescale = MI_EXDMA_PRESCALER_1;

#ifdef TWL_PLATFORM_BB
    interval /= 2;
#endif // TWL_PLATFORM_BB

    MIi_ExDmaRecvAsyncCore( dmaNo, (void*)REG_MIC_FIFO_ADDR, dest, 
                (u32)size, (u32)size, 
                MI_EXDMA_BLOCK_32B, interval, prescale,
                MI_EXDMA_CONTINUOUS_ON, MI_EXDMA_SRC_RLD_ON, MI_EXDMA_DEST_RLD_ON,
                MI_EXDMA_TIMING_MIC );
}

/*---------------------------------------------------------------------------*
  Name:         MICi_ExDmaInterruptHandler

  Description:  interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MICi_ExDmaInterruptHandler( void )
{
//    OS_TPrintf( "*" );

#if 0
    MICWork *wp = &micWork;

    MICi_ExDmaRecvAsync( wp->dmaNo, wp->buf, wp->bufSize );
#endif
}

/*---------------------------------------------------------------------------*
  Name:         MICi_FifoInterruptHandler

  Description:  interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MICi_FifoInterruptHandler( void )
{
//    OS_TPrintf( "X" );

#if 0
    MICWork *wp = &micWork;

    MIi_CpuSend32( (void*)REG_MIC_FIFO_ADDR, wp->buf, (u32)wp->bufSize );
#endif
}


