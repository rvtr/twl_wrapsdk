

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
void MI_DmaRecv32(u32 dmaNo, const void *src, volatile void *dest, u32 size);

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
void MI_DmaRecv16(u32 dmaNo, const void *src, volatile void *dest, u32 size);

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
                       MIDmaCallback callback, void *arg);


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
                       MIDmaCallback callback, void *arg);


