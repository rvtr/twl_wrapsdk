/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     exDma.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MI_EXDMA_H_
#define TWL_MI_EXDMA_H_

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
//---- DMA channel
typedef enum
{
    MI_EDMA_NONE = -1,
    MI_EDMA_0 = 0,
    MI_EDMA_1 = 1,
    MI_EDMA_2 = 2,
    MI_EDMA_3 = 3,
    MI_EDMA_NO_MIN = MI_EDMA_0,
    MI_EDMA_NO_MAX = MI_EDMA_3
}
MIExDmaNo;

//---- timing
typedef enum
{
    MI_EDMA_TIMING_IMM        = (0x10UL << REG_MI_DMA4CNT_TIMING_SHIFT),  // start immediately
    MI_EDMA_TIMING_TM0        = (0x0UL << REG_MI_DMA4CNT_TIMING_SHIFT),   // timer 0
    MI_EDMA_TIMING_TM1        = (0x1UL << REG_MI_DMA4CNT_TIMING_SHIFT),   // timer 1
    MI_EDMA_TIMING_TM2        = (0x2UL << REG_MI_DMA4CNT_TIMING_SHIFT),   // timer 2
    MI_EDMA_TIMING_TM3        = (0x3UL << REG_MI_DMA4CNT_TIMING_SHIFT),   // timer 3
    MI_EDMA_TIMING_V_BLANK    = (0x4UL << REG_MI_DMA4CNT_TIMING_SHIFT),   // VBlank
    MI_EDMA_TIMING_GCD        = (0x7UL << REG_MI_DMA4CNT_TIMING_SHIFT),   // card
    MI_EDMA_TIMING_SD         = (0x8UL << REG_MI_DMA4CNT_TIMING_SHIFT),   // SD
    MI_EDMA_TIMING_CAMERA     = (0x9UL << REG_MI_DMA4CNT_TIMING_SHIFT),   // camera
    MI_EDMA_TIMING_AES_IN     = (0xAUL << REG_MI_DMA4CNT_TIMING_SHIFT),   // AES input
    MI_EDMA_TIMING_AES_OUT    = (0xBUL << REG_MI_DMA4CNT_TIMING_SHIFT),   // AES output
    MI_EDMA_TIMING_MIC        = (0xCUL << REG_MI_DMA4CNT_TIMING_SHIFT)    // MIC
}
MIEDmaTiming;

//---- block size
typedef enum
{
    MI_EDMA_BLOCK_4B           = (0x0UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_8B           = (0x1UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_16B          = (0x2UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_32B          = (0x3UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_64B          = (0x4UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_128B         = (0x5UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_256B         = (0x6UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_512B         = (0x7UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_1KB          = (0x8UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_2KB          = (0x9UL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_4KB          = (0xAUL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_8KB          = (0xBUL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_16KB         = (0xCUL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_32KB         = (0xDUL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_64KB         = (0xEUL << REG_MI_DMA4CNT_BSIZE_SHIFT),
    MI_EDMA_BLOCK_128KB        = (0xFUL << REG_MI_DMA4CNT_BSIZE_SHIFT)
}
MIEDmaBlockSize;

//---- pre-scaler
typedef enum
{
    MI_EDMA_PRESCALER_1 = (0UL << REG_MI_DMA4BCNT_PS_SHIFT),  // x 1
    MI_EDMA_PRESCALER_4 = (1UL << REG_MI_DMA4BCNT_PS_SHIFT),  // x 4
    MI_EDMA_PRESCALER_16 = (2UL << REG_MI_DMA4BCNT_PS_SHIFT), // x 16
    MI_EDMA_PRESCALER_64 = (3UL << REG_MI_DMA4BCNT_PS_SHIFT)  // x 64
}
MIEDmaPrescaler;

//---- yield cycle
typedef enum
{
    MI_EDMAGBL_YLD_CYCLE_0      = (0x0UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_1      = (0x1UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_2      = (0x2UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_4      = (0x3UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_8      = (0x4UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_16     = (0x5UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_32     = (0x6UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_64     = (0x7UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_128    = (0x8UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_256    = (0x9UL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_512    = (0xAUL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_1K     = (0xBUL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_2K     = (0xCUL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_4K     = (0xDUL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_8K     = (0xEUL << REG_MI_DMAGBL_YLD_SHIFT),
    MI_EDMAGBL_YLD_CYCLE_16K    = (0xFUL << REG_MI_DMAGBL_YLD_SHIFT)
}
MIEDmaYieldCycles;

//---- arbotration
typedef u32 MIEDmaArbitration;

#define MI_EDMAGBL_ARB_PRIORITY     (0UL << REG_MI_DMAGBL_ARB_SHIFT)     // arbotration by priority
#define MI_EDMAGBL_ARB_ROUND_ROBIN  (1UL << REG_MI_DMAGBL_ARB_SHIFT)     // arbotration by round robin


//---- registers
typedef struct
{
    const void * src;
    void * dest;
    u32 totalCount;
    u32 wordCount;
    u32 blockInterval;
    u32 fillData;
    u32 ctrl;
}
t_MIEDmaChanRegs;

typedef volatile t_MIEDmaChanRegs MIEDmaChanRegs;


//================================================================================
//                    DMA control definition
//================================================================================
//---- maximum DMA channel No.
#define MI_EDMA_MAX_NUM             3

//---- enable
#define MI_EDMA_ENABLE              (1UL << REG_MI_DMA4CNT_E_SHIFT)      // DMA enable
#define MI_EDMA_IF_ENABLE           (1UL << REG_MI_DMA4CNT_I_SHIFT)      // interrupt enable

//---- continuous mode
#define MI_EDMA_CONTINUOUS_OFF      (0UL << REG_MI_DMA4CNT_CM_SHIFT)      // continuous mode off
#define MI_EDMA_CONTINUOUS_ON       (1UL << REG_MI_DMA4CNT_CM_SHIFT)      // continuous mode on

//---- DMA timing
#if 0
#  define MI_EDMA_TIMING_MASK       (REG_MI_DMA4CNT_TIMING_MASK) // mask  of start field
#  define MI_EDMA_TIMING_SHIFT      (REG_MI_DMA4CNT_TIMING_SHIFT)        // shift of start field
#  define MI_EDMA_TIMING_IMM        (0UL << REG_MI_DMA4CNT_TIMING_SHIFT) // start immediately
#  define MI_EDMA_TIMING_V_BLANK    (1UL << REG_MI_DMA4CNT_TIMING_SHIFT) // start by VBlank
#  define MI_EDMA_TIMING_CARD       (2UL << REG_MI_DMA4CNT_TIMING_SHIFT) // card
#  define MI_EDMA_TIMING_WIRELESS   (3UL << REG_MI_DMA4CNT_TIMING_SHIFT) // DMA4,2:wireless interrupt
#  define MI_EDMA_TIMING_CARTRIDGE  MI_DMA_TIMING_WIRELESS       // DMA1,3:cartridge warning
#endif

//---- block size
#define MI_EDMA_BLOCK_SIZE_MASK     (REG_MI_DMA4CNT_BSIZE_MASK)          // mask  of block size
#define MI_EDMA_BLOCK_SIZE_SHIFT    (REG_MI_DMA4CNT_BSIZE_SHIFT)         // shift of block size

//---- direction of src/destination address
#define MI_EDMA_SRC_INC             (0UL << REG_MI_DMA4CNT_SAR_SHIFT)    // increment source address
#define MI_EDMA_SRC_DEC             (1UL << REG_MI_DMA4CNT_SAR_SHIFT)    // decrement source address
#define MI_EDMA_SRC_FIX             (2UL << REG_MI_DMA4CNT_SAR_SHIFT)    // fix source address
#define MI_EDMA_SRC_FILLREG         (3UL << REG_MI_DMA4CNT_SAR_SHIFT)    // source is fill data register
#define MI_EDMA_DEST_INC            (0UL << REG_MI_DMA4CNT_DAR_SHIFT)    // imcrement destination address
#define MI_EDMA_DEST_DEC            (1UL << REG_MI_DMA4CNT_DAR_SHIFT)    // decrement destination address
#define MI_EDMA_DEST_FIX            (2UL << REG_MI_DMA4CNT_DAR_SHIFT)    // fix destination address

//---- reload of src/destination address
#define MI_EDMA_SRC_RLD_OFF         (0UL << REG_MI_DMA4CNT_SRLD_SHIFT)   // source address reload off
#define MI_EDMA_SRC_RLD_ON          (1UL << REG_MI_DMA4CNT_SRLD_SHIFT)   // source address reload on
#define MI_EDMA_DEST_RLD_OFF        (0UL << REG_MI_DMA4CNT_DRLD_SHIFT)   // destination address reload off
#define MI_EDMA_DEST_RLD_ON         (1UL << REG_MI_DMA4CNT_DRLD_SHIFT)   // destination address reload on

//================================================================================
//                    DMA block interval control definition
//================================================================================

//---- block interval
#define MI_EDMABCNT_INTERVAL_MASK   (REG_MI_DMA4BCNT_BI_MASK)            // mask  of block interval
#define MI_EDMABCNT_INTERVAL_SHIFT  (REG_MI_DMA4BCNT_BI_SHIFT)           // shift of block interval

//---- block interval pre-scaler
#define MI_EDMABCNT_PRESCALER_MASK  (REG_MI_DMA4BCNT_PS_MASK)            // mask  of pre-scaler
#define MI_EDMABCNT_PRESCALER_SHIFT (REG_MI_DMA4BCNT_PS_SHIFT)           // shift of pre-scaler


//================================================================================
//       DMA WAIT
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_IsExDmaBusy

  Description:  check whether extended DMA is busy or not

  Arguments:    dmaNo : DMA channel No.

  Returns:      TRUE if extended DMA is busy, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL    MIi_IsExDmaBusy( MIExDmaNo dmaNo );

/*---------------------------------------------------------------------------*
  Name:         MIi_WaitExDma

  Description:  wait while extended DMA is busy

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_WaitExDma( MIExDmaNo dmaNo );

/*---------------------------------------------------------------------------*
  Name:         MIi_StopExDma

  Description:  stop extended DMA

  Arguments:    dmaNo : DMA channel No.

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MIi_StopExDma( MIExDmaNo dmaNo );

//================================================================================
//            memory operation using DMA
//================================================================================
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
void    MIi_ExDmaFill( MIExDmaNo dmaNo, void *dest, u32 data, u32 size );

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
void MIi_ExDmaCopy( MIExDmaNo dmaNo, const void *src, void *dest, u32 size );

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
void MIi_ExDmaSend( MIExDmaNo dmaNo, const void *src, void *dest, u32 size );

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
void MIi_ExDmaRecv( MIExDmaNo dmaNo, const void *src, void *dest, u32 size );

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
void MIi_ExDmaFillAsync( MIExDmaNo dmaNo, void *dest, u32 data, u32 size );

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
void MIi_ExDmaCopyAsync( MIExDmaNo dmaNo, const void *src, void *dest, u32 size );

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaSendAsync

  Description:  send data with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaSendAsync( MIExDmaNo dmaNo, const void *src, void *dest, u32 size );

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaRecvAsync

  Description:  receive data with DMA
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaRecvAsync( MIExDmaNo dmaNo, const void *src, void *dest, u32 size );

//----------------- internel functions -------------------

/*---------------------------------------------------------------------------*
  Name:         MIi_ExDmaFillCore

  Description:  fill memory with specified data.
                sync version

  Arguments:    dmaNo : DMA channel No.
                dest  : destination address
                src   : source address
                size  : size (byte)
                blockSize : block size

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_ExDmaFillCore( MIExDmaNo dmaNo, void *dest, u32 data, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing );

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
void MIi_ExDmaCopyCore( MIExDmaNo dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing );

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
void MIi_ExDmaSendCore( MIExDmaNo dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing );

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
void MIi_ExDmaRecvCore( MIExDmaNo dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing );

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
void MIi_ExDmaFillAsyncCore( MIExDmaNo dmaNo, void *dest, u32 data, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing );

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
void MIi_ExDmaCopyAsyncCore( MIExDmaNo dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing );

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
void MIi_ExDmaSendAsyncCore( MIExDmaNo dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing );

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
void MIi_ExDmaRecvAsyncCore( MIExDmaNo dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing );

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
void MIi_ExDmaBypassAsyncCore( MIExDmaNo dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing );

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
void MIi_SetExDmaParams( MIExDmaNo dmaNo, const void *src, void *dest, u32 size, u32 oneShotSize,
                MIEDmaBlockSize blockSize, u32 interval, MIEDmaPrescaler prescale,
                u32 continuous, u32 srcRld, u32 destRld,
                MIEDmaTiming timing,
                u32 fillData, u32 srcDir, u32 destDir);

/*---------------------------------------------------------------------------*
  Name:         MIi_SetExDmaArbiter

  Description:  set DMA arbitration

  Arguments:    arb : arbitration algorism
                yld : yield cycles for round robin

  Returns:      None
 *---------------------------------------------------------------------------*/
void MIi_SetExDmaArbiter( MIEDmaArbitration arb, MIEDmaYieldCycles yld );


#ifdef __cplusplus
} /* extern "C" */

#endif

/* TWL_MI_EXDMA_H_ */
#endif
