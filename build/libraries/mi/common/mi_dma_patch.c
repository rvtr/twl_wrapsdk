
#include <nitro.h>
#include <mi_dma.h>


#define MI_CNT_RECV16(size)        ( MI_DMA_IMM16ENABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/2) )
#define MI_CNT_RECV32(size)        ( MI_DMA_IMM32ENABLE | MI_DMA_SRC_FIX | MI_DMA_DEST_INC | ((size)/4) )
#define MI_CNT_RECV16_IF(size)     ( MI_CNT_RECV16((size)) | MI_DMA_IF_ENABLE )
#define MI_CNT_RECV32_IF(size)     ( MI_CNT_RECV32((size)) | MI_DMA_IF_ENABLE )


/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv32

  Description:  send u32 data to fixed address
                sync 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_DmaRecv32(u32 dmaNo, const void *src, volatile void *dest, u32 size)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    if (size == 0)
    {
        return;
    }

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    MIi_DmaSetParams_wait(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV32(size));
    MIi_Wait_AfterDMA(dmaCntp);
}


/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv16

  Description:  send u16 data to fixed address
                sync 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_DmaRecv16(u32 dmaNo, const void *src, volatile void *dest, u32 size)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    if (size == 0)
    {
        return;
    }

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    MIi_DmaSetParams_wait(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV16(size));
    MIi_Wait_AfterDMA(dmaCntp);
}


/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv32Async

  Description:  send u32 data to fixed address
                async 32bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_DmaRecv32Async(u32 dmaNo, const void *src, volatile void *dest, u32 size,
                       MIDmaCallback callback, void *arg)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL4(size);
    MIi_ASSERT_SIZE(dmaNo, size / 4);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
            MIi_DmaSetParams(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV32_IF(size));
        }
        else
        {
            MIi_DmaSetParams(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV32(size));
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         MI_DmaRecv16Async

  Description:  send u16 data to fixed address
                async 16bit version

  Arguments:    dmaNo : DMA channel No.
                src   : data stream to send
                dest  : destination address. not incremented
                size  : size (byte)
                callback : callback function called finish DMA
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_DmaRecv16Async(u32 dmaNo, const void *src, volatile void *dest, u32 size,
                       MIDmaCallback callback, void *arg)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_MUL2(size);
    MIi_ASSERT_SIZE(dmaNo, size / 2);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    if (size == 0)
    {
        MIi_CallCallback(callback, arg);
    }
    else
    {
        MI_WaitDma(dmaNo);

        if (callback)
        {
            OSi_EnterDmaCallback(dmaNo, callback, arg);
            MIi_DmaSetParams(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV16_IF(size));
        }
        else
        {
            MIi_DmaSetParams(dmaNo, (u32)src, (u32)dest, MI_CNT_RECV16(size));
        }
    }
}



