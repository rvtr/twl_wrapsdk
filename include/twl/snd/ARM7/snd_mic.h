/*---------------------------------------------------------------------------*
  Project:  CtrSDK - MIC - include
  File:     snd_mic.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_SND_MIC_H_
#define TWL_SND_MIC_H_

#include <twl/types.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MIC_INTR_DISABLE        =  (0x0UL << REG_SND_MICCNT_IM_SHIFT),
    MIC_INTR_HALF           =  (0x1UL << REG_SND_MICCNT_IM_SHIFT),
    MIC_INTR_OVERFLOW       =  (0x2UL << REG_SND_MICCNT_IM_SHIFT),
    MIC_INTR_HALF_OVERFLOW  =  (0x3UL << REG_SND_MICCNT_IM_SHIFT)
}
MICIntrCond;

typedef enum
{
    MIC_SMP_ALL             =  (0x0UL << REG_SND_MICCNT_FIFO_SMP_SHIFT),
    MIC_SMP_1_2             =  (0x1UL << REG_SND_MICCNT_FIFO_SMP_SHIFT),
    MIC_SMP_1_3             =  (0x2UL << REG_SND_MICCNT_FIFO_SMP_SHIFT),
    MIC_SMP_1_4             =  (0x3UL << REG_SND_MICCNT_FIFO_SMP_SHIFT)
}
MICSampleRate;


typedef struct
{
    u32     dmaNo;          // DMA No
    void*   buf;
    s32     bufSize;
}
MICWork;


#define MIC_DEFAULT_DMA_NO    6


/*---------------------------------------------------------------------------*
  Name:         MICi_Init

  Description:  initialize MIC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MICi_Init( void );

/*---------------------------------------------------------------------------*
  Name:         MICi_Start

  Description:  start MIC

  Arguments:    id : slave id

  Returns:      None
 *---------------------------------------------------------------------------*/
void MICi_Start( MICSampleRate smp, u32 dmaNo, void *dest, s32 size );

/*---------------------------------------------------------------------------*
  Name:         MICi_Stop

  Description:  stop MIC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MICi_Stop( void );


#ifdef __cplusplus
} /* extern "C" */

#endif

/* TWL_SND_MIC_H_ */
#endif
