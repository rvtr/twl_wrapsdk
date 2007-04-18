/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_exDma.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <twl.h>

static BOOL isArbitrated = FALSE;

static u32 intervalTable[] =
{
    8, 8, 8, 8,
};

//================================================================================
//            memory oparation using DMA (sync)
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_SetExDmaArbiter

  Description:  set DMA arbitration

  Arguments:    arb : arbitration algorism
                yld : yield cycles for round robin

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_SetExDmaArbiter( MIEDmaArbitration arb, MIEDmaYieldCycles yld )
{
    OSIntrMode enabled = OS_DisableInterrupts();

    reg_MI_DMAGBL = (u32)(arb | yld);

    isArbitrated = TRUE;

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaFill

  Description:  fill memory with specified data.
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaFill( u32 dmaNo, void *dest, u32 data, u32 size )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaBlockSize blockSize = MI_EDMA_BLOCK_32B;
        u32 interval = intervalTable[idx];
        MIEDmaPrescaler prescale = MI_EDMA_PRESCALER_1;

        MIi_ExDmaFillCore( dmaNo, dest, data, size, size,
                blockSize, interval, prescale,
                MI_EDMA_CONTINUOUS_OFF, MI_EDMA_SRC_RLD_OFF, MI_EDMA_DEST_RLD_OFF,
                MI_EDMA_TIMING_IMM );
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaCopy

  Description:  copy memory with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaCopy( u32 dmaNo, const void *src, void *dest, u32 size )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaBlockSize blockSize = MI_EDMA_BLOCK_32B;
        u32 interval = intervalTable[idx];
        MIEDmaPrescaler prescale = MI_EDMA_PRESCALER_1;

        MIi_ExDmaCopyCore( dmaNo, src, dest, size, size,
                blockSize, interval, prescale,
                MI_EDMA_CONTINUOUS_OFF, MI_EDMA_SRC_RLD_OFF, MI_EDMA_DEST_RLD_OFF,
                MI_EDMA_TIMING_IMM );
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaSend

  Description:  send data with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaSend( u32 dmaNo, const void *src, void *dest, u32 size )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaBlockSize blockSize = MI_EDMA_BLOCK_32B;
        u32 interval = intervalTable[idx];
        MIEDmaPrescaler prescale = MI_EDMA_PRESCALER_1;

        MIi_ExDmaSendCore( dmaNo, src, dest, size, size,
                blockSize, interval, prescale,
                MI_EDMA_CONTINUOUS_OFF, MI_EDMA_SRC_RLD_OFF, MI_EDMA_DEST_RLD_OFF,
                MI_EDMA_TIMING_IMM );
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaRecv

  Description:  receive data with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaRecv( u32 dmaNo, const void *src, void *dest, u32 size )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaBlockSize blockSize = MI_EDMA_BLOCK_32B;
        u32 interval = intervalTable[idx];
        MIEDmaPrescaler prescale = MI_EDMA_PRESCALER_1;

        MIi_ExDmaRecvCore( dmaNo, src, dest, size, size,
                blockSize, interval, prescale,
                MI_EDMA_CONTINUOUS_OFF, MI_EDMA_SRC_RLD_OFF, MI_EDMA_DEST_RLD_OFF,
                MI_EDMA_TIMING_IMM );
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaFillAsync

  Description:  fill memory with specified data.
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaFillAsync( u32 dmaNo, void *dest, u32 data, u32 size )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaBlockSize blockSize = MI_EDMA_BLOCK_32B;
        u32 interval = intervalTable[idx];
        MIEDmaPrescaler prescale = MI_EDMA_PRESCALER_1;

        MIi_ExDmaFillAsyncCore( dmaNo, dest, data, size, size,
                blockSize, interval, prescale,
                MI_EDMA_CONTINUOUS_OFF, MI_EDMA_SRC_RLD_OFF, MI_EDMA_DEST_RLD_OFF,
                MI_EDMA_TIMING_IMM );
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaCopyAsync

  Description:  copy memory with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaCopyAsync( u32 dmaNo, const void *src, void *dest, u32 size )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaBlockSize blockSize = MI_EDMA_BLOCK_32B;
        u32 interval = intervalTable[idx];
        MIEDmaPrescaler prescale = MI_EDMA_PRESCALER_1;

        MIi_ExDmaCopyAsyncCore( dmaNo, src, dest, size, size,
                blockSize, interval, prescale,
                MI_EDMA_CONTINUOUS_OFF, MI_EDMA_SRC_RLD_OFF, MI_EDMA_DEST_RLD_OFF,
                MI_EDMA_TIMING_IMM);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaSendAsync

  Description:  send data with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaSendAsync( u32 dmaNo, const void *src, void *dest, u32 size )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaBlockSize blockSize = MI_EDMA_BLOCK_32B;
        u32 interval = intervalTable[idx];
        MIEDmaPrescaler prescale = MI_EDMA_PRESCALER_1;

        MIi_ExDmaSendAsyncCore( dmaNo, src, dest, size, size,
                blockSize, interval, prescale,
                MI_EDMA_CONTINUOUS_OFF, MI_EDMA_SRC_RLD_OFF, MI_EDMA_DEST_RLD_OFF,
                MI_EDMA_TIMING_IMM );
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaRecvAsync

  Description:  receive data with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaRecvAsync( u32 dmaNo, const void *src, void *dest, u32 size )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaBlockSize blockSize = MI_EDMA_BLOCK_32B;
        u32 interval = intervalTable[idx];
        MIEDmaPrescaler prescale = MI_EDMA_PRESCALER_1;

        MIi_ExDmaRecvAsyncCore( dmaNo, src, dest, size, size,
                blockSize, interval, prescale,
                MI_EDMA_CONTINUOUS_OFF, MI_EDMA_SRC_RLD_OFF, MI_EDMA_DEST_RLD_OFF,
                MI_EDMA_TIMING_IMM );
    }
}

//----------------- internel functions -------------------

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaFillCore

  Description:  fill memory with specified data.
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                data  : fill data
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaFillCore( u32 dmaNo, void *dest, u32 data, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing )
{
    MIi_ExDmaFillAsyncCore( dmaNo, dest, data, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing );

    MIi_WaitExDma( dmaNo );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaCopyCore

  Description:  copy memory with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaCopyCore( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing )
{
    MIi_ExDmaCopyAsyncCore( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing );

    MIi_WaitExDma( dmaNo );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaSendCore

  Description:  copy memory with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaSendCore( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing )
{
    MIi_ExDmaSendAsyncCore( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing );

    MIi_WaitExDma( dmaNo );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaRecvCore

  Description:  copy memory with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaRecvCore( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing )
{
    MIi_ExDmaRecvAsyncCore( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing );

    MIi_WaitExDma( dmaNo );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaFillAsyncCore

  Description:  fill memory with specified data.
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaFillAsyncCore( u32 dmaNo, void *dest, u32 data, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing )
{
    const void *src = NULL;
    u32 srcDir = MI_EDMA_SRC_FILLREG;

    MIi_WaitExDma( dmaNo );

    MIi_SetExDmaParams( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing,
                data, srcDir, MI_EDMA_DEST_INC );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaCopyAsyncCore

  Description:  copy memory with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaCopyAsyncCore( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing )
{
    MIi_WaitExDma( dmaNo );

    MIi_SetExDmaParams( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing,
                0, MI_EDMA_SRC_INC, MI_EDMA_DEST_INC );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaSendAsyncCore

  Description:  copy memory with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaSendAsyncCore( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing )
{
    MIi_WaitExDma( dmaNo );

    MIi_SetExDmaParams( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing,
                0, MI_EDMA_SRC_INC, MI_EDMA_DEST_FIX );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaRecvAsyncCore

  Description:  copy memory with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaRecvAsyncCore( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing )
{
    MIi_WaitExDma( dmaNo );

    MIi_SetExDmaParams( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing,
                0, MI_EDMA_SRC_FIX, MI_EDMA_DEST_INC );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaBypassAsyncCore

  Description:  copy memory with DMA
                async version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaBypassAsyncCore( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing )
{
    MIi_WaitExDma( dmaNo );

    MIi_SetExDmaParams( dmaNo, src, dest, size, oneShotSize,
                blockSize, interval, prescale,
                continuous, srcRld, destRld,
                timing,
                0, MI_EDMA_SRC_FIX, MI_EDMA_DEST_FIX );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_SetExDmaParams

  Description:  copy memory with DMA
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_SetExDmaParams( u32 dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing,
                u32 fillData, u32 srcDir, u32 destDir )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    if ( idx < MI_EDMA_CH_NUM )
    {
        OSIntrMode enabled = OS_DisableInterrupts();

        MIEDmaChanRegs *reg = &((MIEDmaChanRegs*)REG_DMA4SAD_ADDR)[idx];

        reg->src = src;
        reg->dest = dest;
        reg->fillData = fillData;
        reg->totalCount = size / 4;
        reg->wordCount = oneShotSize / 4;
        reg->blockInterval = (interval << MI_EDMABCNT_INTERVAL_SHIFT) | prescale;
        reg->ctrl = blockSize
                  | srcDir | destDir
                  | srcRld | destRld
                  | continuous
                  | timing
                  | MI_EDMA_ENABLE | MI_EDMA_IF_ENABLE;

        (void)OS_RestoreInterrupts(enabled);
    }
}

//================================================================================
//       DMA WAIT/STOP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_IsExDmaBusy

  Description:  check whether extended DMA is busy or not

  Arguments:    dmaNo : DMA channel No.

  Returns:      TRUE if extended DMA is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL MIi_IsExDmaBusy( u32 dmaNo )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaChanRegs *reg = &((MIEDmaChanRegs*)REG_DMA4SAD_ADDR)[idx];

        return (BOOL)((reg->ctrl & REG_MI_DMA4CNT_E_MASK) >> REG_MI_DMA4CNT_E_SHIFT);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_WaitExDma

  Description:  wait while extended DMA is busy

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_WaitExDma( u32 dmaNo )
{
    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    {
        MIEDmaChanRegs *reg = &((MIEDmaChanRegs*)REG_DMA4SAD_ADDR)[idx];

        while (reg->ctrl & REG_MI_DMA4CNT_E_MASK)
        {
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MIi_StopDma

  Description:  stop extended DMA

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_StopExDma( u32 dmaNo )
{
    OSIntrMode enabled = OS_DisableInterrupts();

    u32 idx = dmaNo - MI_EDMA_CH_MIN;

    if ( idx < MI_EDMA_CH_NUM )
    {
        MIEDmaChanRegs *reg = &((MIEDmaChanRegs*)REG_DMA4SAD_ADDR)[idx];

        reg->ctrl &= ~MI_EDMA_ENABLE;
    }

    (void)OS_RestoreInterrupts(enabled);
}

