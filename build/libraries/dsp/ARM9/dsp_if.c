/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_if.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl/dsp.h>
#include <nitro/hw/ARM9/ioreg_CFG.h>
#include <nitro/os/common/system.h>
#include <nitro/misc.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct DSPData
{
    u16 send;
    u16 recv;
}
DSPData;

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static volatile DSPData *const dspData = (DSPData*)REG_DSP_SEND_DATA_0_ADDR;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOn

  Description:  power DSP block on but reset yet.
                you should call DSP_ResetOff() to boot DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_PowerOn(void)
{
    reg_CFG_DSP_RST &= ~REG_CFG_DSP_RST_OFF_MASK;   // DSPブロックのリセット確認
    reg_CFG_CLK |= REG_CFG_CLK_DSP_MASK;            // DSPブロックの電源On
    OS_SpinWaitSysCycles(2);                        // wait 8 cycle @ 134MHz
    reg_CFG_DSP_RST |= REG_CFG_DSP_RST_OFF_MASK;    // DSPブロックのリセット解除
    DSP_ResetOn();                                  // DSPコアのリセット設定
}
/*---------------------------------------------------------------------------*
  Name:         DSP_PowerOff

  Description:  power DSP block off

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_PowerOff(void)
{
    reg_CFG_DSP_RST &= ~REG_CFG_DSP_RST_OFF_MASK;   // DSPブロックのリセット設定
    reg_CFG_CLK &= ~REG_CFG_CLK_DSP_MASK;           // DSPブロックの電源Off
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOn

  Description:  reset DSP.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOn(void)
{
    reg_DSP_DSP_CONFIG |= REG_DSP_DSP_CONFIG_RESET_MASK;
    while ( reg_DSP_DSP_STATUS & REG_DSP_DSP_STATUS_RESET_BUSY_MASK )
    {
    }
}
/*---------------------------------------------------------------------------*
  Name:         DSP_ResetOff

  Description:  boot DSP if in reset state.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetOff(void)
{
    while ( reg_DSP_DSP_STATUS & REG_DSP_DSP_STATUS_RESET_BUSY_MASK )
    {
    }
    reg_DSP_DSP_CONFIG &= ~REG_DSP_DSP_CONFIG_RESET_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ResetInterface

  Description:  reset interface registers.
                it should be called while reset state.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ResetInterface(void)
{
    u16 dummy;
    reg_DSP_DSP_CONFIG &= ~REG_DSP_DSP_CONFIG_RECV_DATA_IE_MASK;
    reg_DSP_DSP_SEM_SEND_DATA = 0;
    reg_DSP_DSP_SEM_RECV_CLEAR = 0xFFFF;
    dummy = dspData[0].recv;
    dummy = dspData[1].recv;
    dummy = dspData[2].recv;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableRecvDataInterrupt

  Description:  enable interrupt for receive data from DSP.

  Arguments:    dataNo:     target data register (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_EnableRecvDataInterrupt(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    reg_DSP_DSP_CONFIG |= (1 << dataNo) << REG_DSP_DSP_CONFIG_RECV_DATA_IE_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableRecvDataInterrupt

  Description:  disable interrupt for receive data from DSP.

  Arguments:    dataNo:     target data register (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DisableRecvDataInterrupt(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    reg_DSP_DSP_CONFIG &= ~((1 << dataNo) << REG_DSP_DSP_CONFIG_RECV_DATA_IE_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendDataIsEmpty

  Description:  whether DSP is received sending data.

  Arguments:    dataNo:     target data register (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL DSP_SendDataIsEmpty(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    return (reg_DSP_DSP_STATUS & ((1 << dataNo) << REG_DSP_DSP_STATUS_SEND_DATA_FULL_SHIFT)) ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvDataIsReady

  Description:  whether there is sent data from DSP.

  Arguments:    dataNo:     target data register (0-2)

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL DSP_RecvDataIsReady(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    return (reg_DSP_DSP_STATUS & ((1 << dataNo) << REG_DSP_DSP_STATUS_RECV_DATA_EMP_SHIFT)) ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SendData

  Description:  send data to DSP

  Arguments:    dataNo:     target data register (0-2)
                data:       data to send

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SendData(u32 dataNo, u16 data)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    while (DSP_SendDataIsEmpty(dataNo) == FALSE)
    {
    }
    dspData[dataNo].send = data;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_RecvData

  Description:  receive data from DSP

  Arguments:    dataNo:     target data register (0-2)

  Returns:      receiving data
 *---------------------------------------------------------------------------*/
u16 DSP_RecvData(u32 dataNo)
{
    SDK_ASSERT(dataNo >= 0 && dataNo <= 2);
    while (DSP_RecvDataIsReady(dataNo) == FALSE)
    {
    }
    return dspData[dataNo].recv;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_EnableFifoInterrupt

  Description:  enable interrupt for FIFO.

  Arguments:    type:       one of DSPFifoIntr

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_EnableFifoInterrupt(DSPFifoIntr type)
{
    reg_DSP_DSP_CONFIG |= type;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_DisableFifoInterrupt

  Description:  disable interrupt for FIFO.

  Arguments:    type:       one of DSPFifoIntr

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DisableFifoInterrupt(DSPFifoIntr type)
{
    reg_DSP_DSP_CONFIG &= ~type;
}

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
void DSP_SendFifo(DSPFifoMemSel memsel, u16 dest, const u16 *src, int size, u16 flags)
{
    u16 incmode = (u16)((flags & DSP_FIFO_FLAG_SRC_FIX) ? 0 : REG_DSP_DSP_CONFIG_FIFO_INC_MODE_MASK);

    reg_DSP_DSP_FIFO_ADDR = dest;
    reg_DSP_DSP_CONFIG = (u16)((reg_DSP_DSP_CONFIG & ~(REG_DSP_DSP_CONFIG_FIFO_MEMSEL_MASK|REG_DSP_DSP_CONFIG_FIFO_INC_MODE_MASK))
                        | memsel | incmode);

    if (flags & DSP_FIFO_FLAG_SRC_FIX)
    {
        while (size-- > 0)
        {
            while (reg_DSP_DSP_STATUS & REG_DSP_DSP_STATUS_FIFO_SEND_FULL_MASK)
            {
            }
            reg_DSP_DSP_FIFO_DATA = *src;
        }
    }
    else
    {
        while (size-- > 0)
        {
            while (reg_DSP_DSP_STATUS & REG_DSP_DSP_STATUS_FIFO_SEND_FULL_MASK)
            {
            }
            reg_DSP_DSP_FIFO_DATA = *src++;
        }
    }
}

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
void DSP_RecvFifo(DSPFifoMemSel memsel, u16 addr, u16 *bufp, int size, u16 flags)
{
    DSPFifoRecvLength len;
    u16 incmode = (u16)((flags & DSP_FIFO_FLAG_SRC_FIX) ? 0 : REG_DSP_DSP_CONFIG_FIFO_INC_MODE_MASK);

    SDK_ASSERT(memsel != DSP_FIFO_MEMSEL_PROGRAM);

    switch (flags & DSP_FIFO_FLAG_RECV_MASK)
    {
    case DSP_FIFO_FLAG_RECV_UNIT_2B:
        len = DSP_FIFO_RECV_LEN_2B;
        size = 1;
        break;
    case DSP_FIFO_FLAG_RECV_UNIT_16B:
        len = DSP_FIFO_RECV_LEN_16B;
        size = 8;
        break;
    case DSP_FIFO_FLAG_RECV_UNIT_32B:
        len = DSP_FIFO_RECV_LEN_32B;
        size = 16;
        break;
    default:
        len = DSP_FIFO_RECV_LEN_CONTINUOUS;
        break;
    }

    reg_DSP_DSP_FIFO_ADDR = addr;
    reg_DSP_DSP_CONFIG = (u16)((reg_DSP_DSP_CONFIG & ~(REG_DSP_DSP_CONFIG_FIFO_MEMSEL_MASK|REG_DSP_DSP_CONFIG_FIFO_RECV_LEN_MASK|REG_DSP_DSP_CONFIG_FIFO_INC_MODE_MASK))
                        | memsel | len | incmode | REG_DSP_DSP_CONFIG_FIFO_RECV_E_MASK);

    if (flags & DSP_FIFO_FLAG_DEST_FIX)
    {
        while (size-- > 0)
        {
            while ((reg_DSP_DSP_STATUS & REG_DSP_DSP_STATUS_FIFO_RECV_NEMP_MASK) == 0)
            {
            }
            *bufp = reg_DSP_DSP_FIFO_DATA;
        }
    }
    else
    {
        while (size-- > 0)
        {
            while ((reg_DSP_DSP_STATUS & REG_DSP_DSP_STATUS_FIFO_RECV_NEMP_MASK) == 0)
            {
            }
            *bufp++ = reg_DSP_DSP_FIFO_DATA;
        }
    }
    reg_DSP_DSP_CONFIG &= ~REG_DSP_DSP_CONFIG_FIFO_RECV_E_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetSemaphore

  Description:  set semaphore to signal to DSP.
                NOTE: received semaphore is individual register

  Arguments:    mask:       bit mask to set

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetSemaphore(u16 mask)
{
    reg_DSP_DSP_SEM_SEND_DATA |= mask;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetSemaphore

  Description:  get semaphore to be recieved from DSP.
                NOTE: sending semaphore is individual register

  Arguments:    None.

  Returns:      bit mask is set by DSP
 *---------------------------------------------------------------------------*/
u16 DSP_GetSemaphore(void)
{
    return reg_DSP_DSP_SEM_RECV_DATA;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ClearSemaphore

  Description:  clear semaphore to be recieved from DSP.

  Arguments:    mask:       bit mask to clar

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ClearSemaphore(u16 mask)
{
    reg_DSP_DSP_SEM_RECV_CLEAR |= mask;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_MaskSemaphore

  Description:  mask semaphore to interrupt to ARM9.

  Arguments:    mask:       bit mask to disable to interrupt

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_MaskSemaphore(u16 mask)
{
    reg_DSP_DSP_SEM_RECV_MASK |= mask;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_CheckSemaphoreRequest

  Description:  whether there is requested interrupt by semaphore

  Arguments:    None.

  Returns:      TRUE if requested.
 *---------------------------------------------------------------------------*/
BOOL DSP_CheckSemaphoreRequest(void)
{
    return (reg_DSP_DSP_STATUS & REG_DSP_DSP_STATUS_SEM_RECV_IF_MASK) >> REG_DSP_DSP_STATUS_SEM_RECV_IF_SHIFT;
}

