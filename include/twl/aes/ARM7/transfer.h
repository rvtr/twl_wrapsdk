/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - aes
  File:     aes_transfer.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_AES_TRANSFER_H_
#define TWL_AES_TRANSFER_H_

#include <twl/types.h>
#include <twl/mi/exDma.h>
#include <twl/aes/common/assert.h>

#define AES_DMA_ONESHOT_SIZE    16
#define AES_DMA_BLOCK_SIZE      MI_EXDMA_BLOCK_16B
#define AES_DMA_INTERVAL        8
#define AES_DMA_PRESCALER       MI_EXDMA_PRESCALER_1

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         AES_DmaSend

  Description:  AES DMA send by AES input FIFO timing.
                Should call prior to the AES_Start*().
                Sync version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                src     : source address
                length    : transfer length (byte) (multiple of 16 bytes)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AES_DmaSend(u32 dmaNo, const void *src, u32 length)
{
    AES_ASSERT_DATA_LENGTH(length);
    MIi_ExDmaSendCore( dmaNo, src, (void*)REG_AES_IFIFO_ADDR, length, AES_DMA_ONESHOT_SIZE,
                AES_DMA_BLOCK_SIZE, AES_DMA_INTERVAL, AES_DMA_PRESCALER,
                MI_EXDMA_CONTINUOUS_OFF, MI_EXDMA_SRC_RLD_OFF, MI_EXDMA_DEST_RLD_OFF,
                MI_EXDMA_TIMING_AES_IN );
}

/*---------------------------------------------------------------------------*
  Name:         MIi_AesExDmaSendAsync

  Description:  AES DMA send by AES input FIFO timing.
                Should call prior to the AES_Start*().
                Async version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                src     : source address
                length    : transfer length (byte) (multiple of 16 bytes)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AES_DmaSendAsync(u32 dmaNo, const void *src, u32 length)
{
    AES_ASSERT_DATA_LENGTH(length);
    MIi_ExDmaSendAsyncCore( dmaNo, src, (void*)REG_AES_IFIFO_ADDR, length, AES_DMA_ONESHOT_SIZE,
                AES_DMA_BLOCK_SIZE, AES_DMA_INTERVAL, AES_DMA_PRESCALER,
                MI_EXDMA_CONTINUOUS_OFF, MI_EXDMA_SRC_RLD_OFF, MI_EXDMA_DEST_RLD_OFF,
                MI_EXDMA_TIMING_AES_IN );
}

/*---------------------------------------------------------------------------*
  Name:         AES_DmaRecv

  Description:  AES DMA receive by AES output FIFO timing.
                Should call prior to the AES_Start*().
                Sync version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                dest    : destination address
                length    : transfer length (byte) (multiple of 16 bytes)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AES_DmaRecv(u32 dmaNo, void *dest, u32 length)
{
    AES_ASSERT_DATA_LENGTH(length);
    MIi_ExDmaRecvCore( dmaNo, (void*)REG_AES_OFIFO_ADDR, dest, length, AES_DMA_ONESHOT_SIZE,
                AES_DMA_BLOCK_SIZE, AES_DMA_INTERVAL, AES_DMA_PRESCALER,
                MI_EXDMA_CONTINUOUS_OFF, MI_EXDMA_SRC_RLD_OFF, MI_EXDMA_DEST_RLD_OFF,
                MI_EXDMA_TIMING_AES_OUT );
}

/*---------------------------------------------------------------------------*
  Name:         AES_DmaRecvAsync

  Description:  AES DMA receive by AES output FIFO timing.
                Should call prior to the AES_Start*().
                Async version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                dest    : destination address
                length    : transfer length (byte) (multiple of 16 bytes)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AES_DmaRecvAsync(u32 dmaNo, void *dest, u32 length)
{
    AES_ASSERT_DATA_LENGTH(length);
    MIi_ExDmaRecvAsyncCore( dmaNo, (void*)REG_AES_OFIFO_ADDR, dest, length, AES_DMA_ONESHOT_SIZE,
                AES_DMA_BLOCK_SIZE, AES_DMA_INTERVAL, AES_DMA_PRESCALER,
                MI_EXDMA_CONTINUOUS_OFF, MI_EXDMA_SRC_RLD_OFF, MI_EXDMA_DEST_RLD_OFF,
                MI_EXDMA_TIMING_AES_OUT );
}

/*---------------------------------------------------------------------------*
  Name:         AES_CpuSend

  Description:  send data to AES input fifo by CPU.
                Should call prior to the AES_Start*().
                Sync version.

  Arguments:    src     : source address
                length    : transfer length (byte) (multiple of 16 bytes)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AES_CpuSend(const void *src, u32 length)
{
    const u32 *ptr = src;
    SDK_ASSERT(src && ((u32)src & 0x3) == 0);
    AES_ASSERT_DATA_LENGTH(length);
    while (length > 0) {
        AES_WaitInputFifoNotFull();
        reg_AES_AES_IFIFO = *ptr++;
        length -= sizeof(u32);
    }
}

/*---------------------------------------------------------------------------*
  Name:         AES_CpuRecv

  Description:  receive data from AES input fifo by CPU.
                Should call prior to the AES_Start*().
                Sync version.

  Arguments:    dest    : destination address
                length    : transfer length (byte) (multiple of 16 bytes)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void AES_CpuRecv(void *dest, u32 length)
{
    u32 *ptr = dest;
    SDK_ASSERT(dest && ((u32)dest & 0x3) == 0);
    AES_ASSERT_DATA_LENGTH(length);
    while (length > 0) {
        AES_WaitOutputFifoNotEmpty();
        *ptr++ = reg_AES_AES_OFIFO;
        length -= sizeof(u32);
    }
}

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_AES_TRANSFER_H_ */
