/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_if.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_DSP_IF_H_
#define TWL_DSP_IF_H_

#include <twl/types.h>
#include <nitro/hw/ARM9/ioreg_DSP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
typedef enum
{
    DSP_FIFO_MEMSEL_DATA    = (0x0 << REG_DSP_DSP_CONFIG_FIFO_MEMSEL_SHIFT),
    DSP_FIFO_MEMSEL_MMIO    = (0x1 << REG_DSP_DSP_CONFIG_FIFO_MEMSEL_SHIFT),
    DSP_FIFO_MEMSEL_PROGRAM = (0x5 << REG_DSP_DSP_CONFIG_FIFO_MEMSEL_SHIFT)
}
DSPFifoMemSel;

typedef enum
{
    DSP_FIFO_RECV_LEN_2B            = (0x0 << REG_DSP_DSP_CONFIG_FIFO_RECV_LEN_SHIFT),
    DSP_FIFO_RECV_LEN_16B           = (0x1 << REG_DSP_DSP_CONFIG_FIFO_RECV_LEN_SHIFT),
    DSP_FIFO_RECV_LEN_32B           = (0x2 << REG_DSP_DSP_CONFIG_FIFO_RECV_LEN_SHIFT),
    DSP_FIFO_RECV_LEN_CONTINUOUS    = (0x3 << REG_DSP_DSP_CONFIG_FIFO_RECV_LEN_SHIFT)
}
DSPFifoRecvLength;

typedef enum
{
    DSP_FIFO_FLAG_SRC_INC   = (0UL << 0),
    DSP_FIFO_FLAG_SRC_FIX   = (1UL << 0),

    DSP_FIFO_FLAG_DEST_INC  = (0UL << 1),
    DSP_FIFO_FLAG_DEST_FIX  = (1UL << 1),

    DSP_FIFO_FLAG_RECV_UNIT_CONTINUOUS  = (0UL << 8),
    DSP_FIFO_FLAG_RECV_UNIT_2B          = (1UL << 8),
    DSP_FIFO_FLAG_RECV_UNIT_16B         = (2UL << 8),
    DSP_FIFO_FLAG_RECV_UNIT_32B         = (3UL << 8),
    DSP_FIFO_FLAG_RECV_MASK             = (3UL << 8)
}
DSPFifoFlag;

typedef enum
{
    DSP_FIFO_INTR_RECV_FULL         = (1 << REG_DSP_DSP_CONFIG_FIFO_IE_SHIFT),
    DSP_FIFO_INTR_RECV_NOT_EMPTY    = (2 << REG_DSP_DSP_CONFIG_FIFO_IE_SHIFT),
    DSP_FIFO_INTR_SEND_FULL         = (4 << REG_DSP_DSP_CONFIG_FIFO_IE_SHIFT),
    DSP_FIFO_INTR_SEND_EMPTY        = (8 << REG_DSP_DSP_CONFIG_FIFO_IE_SHIFT)
}
DSPFifoIntr;

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOn

  Description:  power DSP block on but reset yet.
                you should call DSP_ResetOff() to boot DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_PowerOn(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOff

  Description:  power DSP block off

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_PowerOff(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOn

  Description:  reset DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOn(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOff

  Description:  boot DSP if in reset state.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOff(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableRecvDataInterrupt

  Description:  enable interrupt for receive data from DSP.

  Arguments:    dataNo:     target data register (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_EnableRecvDataInterrupt(u32 dataNo);

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableRecvDataInterrupt

  Description:  disable interrupt for receive data from DSP.

  Arguments:    dataNo:     target data register (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DisableRecvDataInterrupt(u32 dataNo);

/*---------------------------------------------------------------------------*
  Name:         DSP_SendDataIsEmpty

  Description:  whether DSP is received sending data.

  Arguments:    dataNo:     target data register (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL DSP_SendDataIsEmpty(u32 dataNo);

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvDataIsReady

  Description:  whether there is sent data from DSP.

  Arguments:    dataNo:     target data register (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL DSP_RecvDataIsReady(u32 dataNo);

/*---------------------------------------------------------------------------*
  Name:         DSP_SendData

  Description:  send data to DSP

  Arguments:    dataNo:     target data register (0-2)
                data:       data to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SendData(u32 dataNo, u16 data);

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvData

  Description:  receive data from DSP

  Arguments:    dataNo:     target data register (0-2)

  Returns:      receiving data
 *---------------------------------------------------------------------------*/
u16 DSP_RecvData(u32 dataNo);

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableFifoInterrupt

  Description:  enable interrupt for FIFO.

  Arguments:    type:       one of DSPFifoIntr

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_EnableFifoInterrupt(DSPFifoIntr type);

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableFifoInterrupt

  Description:  disable interrupt for FIFO.

  Arguments:    type:       one of DSPFifoIntr

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DisableFifoInterrupt(DSPFifoIntr type);

/*---------------------------------------------------------------------------*
  Name:         DSP_SendFifo

  Description:  write data into DSP memory space.

  Arguments:    memsel: one of DSPFifoMemSel
                dest:   destination address (in half words).
                        if you want to set high address, ask DSP to set
                        DMA register.
                src:    data to send.
                size:   data length to send (in half words).
                flags:  bitOR of DSPFifoFlag to specify special mode

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SendFifo(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size, u16 flags);

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvFifo

  Description:  read data into DSP memory space.

  Arguments:    memsel: one of DSPFifoMemSel without PROGRAM area
                addr:   source address (in half words).
                        if you want to set high address, ask DSP to set
                        DMA register.
                bufp:   data to receive.
                size:   data length to receive (in half words).
                        ignore unless continuous mode
                flags:  bitOR of DSPFifoFlag to specify special mode

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_RecvFifo(DSPFifoMemSel memsel, u16 addr, u16 *bufp, int size, u16 flags);

/*---------------------------------------------------------------------------*
  Name:         DSP_SetSemaphore

  Description:  set semaphore to signal to DSP.
                NOTE: received semaphore is individual register

  Arguments:    mask:       bit mask to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetSemaphore(u16 mask);

/*---------------------------------------------------------------------------*
  Name:         DSP_GetSemaphore

  Description:  get semaphore to be recieved from DSP.
                NOTE: sending semaphore is individual register

  Arguments:    None.

  Returns:      bit mask is set by DSP
 *---------------------------------------------------------------------------*/
u16 DSP_GetSemaphore(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_ClearSemaphore

  Description:  clear semaphore to be recieved from DSP.

  Arguments:    mask:       bit mask to clar

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ClearSemaphore(u16 mask);

/*---------------------------------------------------------------------------*
  Name:         DSP_MaskSemaphore

  Description:  mask semaphore to interrupt to ARM9.

  Arguments:    mask:       bit mask to disable to interrupt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_MaskSemaphore(u16 mask);

/*---------------------------------------------------------------------------*
  Name:         DSP_CheckSemaphoreRequest

  Description:  whether there is requested interrupt by semaphore

  Arguments:    None.

  Returns:      TRUE if requested.
 *---------------------------------------------------------------------------*/
BOOL DSP_CheckSemaphoreRequest(void);


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_DSP_H_ */
