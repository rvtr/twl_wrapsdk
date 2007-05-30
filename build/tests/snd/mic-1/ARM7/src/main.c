/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - snd - mic-1
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <twl_sp.h>
#include <twl/mic.h>

#include <math.h>

// ===== �X���b�h�D��x =====

#define THREAD_PRIO_SND         6

#define MY_MIC_BUF_LEN    0x100

u16       micBuf[MY_MIC_BUF_LEN]  __attribute__ ((aligned (32)));

void TwlSpMain(void)
{
    int i, ii;

    OS_Init();
    OS_InitThread();

//    reg_CFG_DS_MDFY |= REG_CFG_DS_MDFY_SND_MASK;  // SOUND��H�o�O�C�� (default: off)
//    reg_CFG_DS_MDFY |= REG_CFG_DS_MDFY_SDMA_MASK; // SOUND-DMA�o�O�C�� (default: off)
//    reg_CFG_DS_EX &= ~REG_CFG_DS_EX_SDMA2_MASK;   // SOUND-DMA�V��H (default: on)

    // �{�^�����̓T�[�`������
    (void)PAD_InitXYButton();

    // �����݋���
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // �T�E���h������
    SND_Init(THREAD_PRIO_SND);

    // �}�C�N������
//    MICi_Init();

    OS_TPrintf("\nARM7 starts.\n");

    MICi_Start( MIC_SMP_ALL, MIC_DEFAULT_DMA_NO, micBuf, sizeof(micBuf) );
    OS_TPrintf( "\nMIC starts.\n");

    OS_SpinWait( OS_MSEC_TO_CPUCYC( 60 ) );

    MICi_Stop();
    OS_TPrintf( "\nMIC stops.\n");

    OS_TPrintf( "\nDump mic buffer.\n" );
    for (i=0; i<MY_MIC_BUF_LEN/16; i++)
    {
        for (ii=0; ii<16; ii++)
        {
            OS_TPrintf( "%4.4x ", micBuf[i*16+ii] );
        }
        OS_TPrintf( "\n" );
    }
    OS_TPrintf( "\n" );

    OS_TPrintf("\nARM7 ends.\n");
    OS_Terminate();
}

