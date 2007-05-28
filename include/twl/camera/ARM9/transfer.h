/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera_transfer.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_CAMERA_TRANSFER_H_
#define TWL_CAMERA_TRANSFER_H_

#include <twl/types.h>
#include <twl/mi/exDma.h>

#define CAMERA_DMA_BLOCK_SIZE      MI_EXDMA_BLOCK_64B
#define CAMERA_DMA_INTERVAL        8
#define CAMERA_DMA_PRESCALER       MI_EXDMA_PRESCALER_1

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         CAMERA_DmaRecv

  Description:  receiving a frame data from CAMERA buffer.
                Sync version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                dest    : destination address
                unit    : transfer length at once (byte)(width * lines at once)
                length  : transfer length (byte) (frame size)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CAMERA_DmaRecv(u32 dmaNo, void *dest, u32 unit, u32 length)
{
    MIi_ExDmaRecvCore( dmaNo, (void*)REG_CAM_DAT_ADDR, dest, length, unit,
                CAMERA_DMA_BLOCK_SIZE, CAMERA_DMA_INTERVAL, CAMERA_DMA_PRESCALER,
                MI_EXDMA_CONTINUOUS_OFF, MI_EXDMA_SRC_RLD_OFF, MI_EXDMA_DEST_RLD_OFF,
                MI_EXDMA_TIMING_CAMERA );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_DmaRecvAsync

  Description:  receiving a frame data from CAMERA buffer.
                Async version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                dest    : destination address
                unit    : transfer length at once (byte)(width * lines at once)
                length  : transfer length (byte) (frame size)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CAMERA_DmaRecvAsync(u32 dmaNo, void *dest, u32 unit, u32 length)
{
    MIi_ExDmaRecvAsyncCore( dmaNo, (void*)REG_CAM_DAT_ADDR, dest, length, unit,
                CAMERA_DMA_BLOCK_SIZE, CAMERA_DMA_INTERVAL, CAMERA_DMA_PRESCALER,
                MI_EXDMA_CONTINUOUS_OFF, MI_EXDMA_SRC_RLD_OFF, MI_EXDMA_DEST_RLD_OFF,
                MI_EXDMA_TIMING_CAMERA );
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_DmaRecvInfinity

  Description:  receiving data from CAMERA buffer.
                Once starting DMA, it will transfer every frame automatically.
                You should call MIi_StopExDma(dmaNo) to stop

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                dest    : destination address
                unit    : transfer length at once (byte)(width * lines at once)
                length  : transfer length (byte) (frame size)

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CAMERA_DmaRecvInfinity(u32 dmaNo, void *dest, u32 unit, u32 length)
{
    MIi_ExDmaRecvAsyncCore( dmaNo, (void*)REG_CAM_DAT_ADDR, dest, length, unit,
                CAMERA_DMA_BLOCK_SIZE, CAMERA_DMA_INTERVAL, CAMERA_DMA_PRESCALER,
                MI_EXDMA_CONTINUOUS_ON, MI_EXDMA_SRC_RLD_OFF, MI_EXDMA_DEST_RLD_ON,
                MI_EXDMA_TIMING_CAMERA );
}

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CAMERA_TRANSFER_H_ */
