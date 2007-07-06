/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - snd - mic-2
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


//#define ENABLE_PSG
//#define ENABLE_OLD_SND

// ===== スレッド優先度 =====

#define THREAD_PRIO_SND         6

#define MY_MIC_BUF_LEN    0x100

#define MPI  3.14159265358979323846

#define MY_SND_16

#ifdef MY_SND_16
#define MY_SND_BYTES     2
#define MY_SND_DATA_MAX  0x7fff
#define MY_SND_WAVE_FMT  SND_WAVE_FORMAT_PCM16
#define MY_SND_CAP_FMT   SND_CAPTURE_FORMAT_PCM16
typedef s16 mySndType;
#else
#define MY_SND_BYTES     1
#define MY_SND_DATA_MAX  0x7f
#define MY_SND_WAVE_FMT  SND_WAVE_FORMAT_PCM8
#define MY_SND_CAP_FMT   SND_CAPTURE_FORMAT_PCM8
typedef s8 mySndType;
#endif

#define MY_SND_FREQ         0x160  // 0x200

#define MY_SND_BUF_LEN      (0x0400/MY_SND_BYTES)
#define MY_SND_CAPTURE_LEN  (0x1000/MY_SND_BYTES)

u16       micBuf[MY_MIC_BUF_LEN]  __attribute__ ((aligned (32)));

mySndType wavBuf[2][MY_SND_BUF_LEN]  __attribute__ ((aligned (32)));
mySndType capBuf[2][MY_SND_CAPTURE_LEN]  __attribute__ ((aligned (32)));


static void MY_SndInit( void )
{
    OSIntrMode enabled;
    int i;

    for (i=0; i<MY_SND_BUF_LEN; i++)
    {
        wavBuf[0][i] = (mySndType)(sin( 2 * MPI * i / MY_SND_BUF_LEN ) * MY_SND_DATA_MAX);
    }
    for (i=0; i<MY_SND_BUF_LEN-1; i++)
    {
        wavBuf[1][i] = wavBuf[0][i+1];
    }
    wavBuf[1][MY_SND_BUF_LEN-1] = wavBuf[0][0];

    enabled = OS_DisableInterrupts();

    SND_Enable();
    SND_SetMasterVolume( SND_MASTER_VOLUME_MAX );
    SND_SetOutputSelector( SND_OUTPUT_MIXER, SND_OUTPUT_MIXER, SND_CHANNEL_OUT_BYPASS, SND_CHANNEL_OUT_BYPASS );

    SND_SetupCapture( SND_CAPTURE_0, MY_SND_CAP_FMT, &capBuf[0], sizeof(capBuf[0])/4, TRUE,
                     SND_CAPTURE_IN_MIXER, SND_CAPTURE_OUT_NORMAL );

    SND_SetupCapture( SND_CAPTURE_1, MY_SND_CAP_FMT, &capBuf[1], sizeof(capBuf[1])/4, TRUE,
                     SND_CAPTURE_IN_MIXER, SND_CAPTURE_OUT_NORMAL );
    SND_StartCaptureBoth();

#ifdef ENABLE_PSG

    SND_SetupChannelPsg( 8, SND_DUTY_4_8, SND_CHANNEL_VOLUME_MAX, SND_CHANNEL_DATASHIFT_NONE, 0x0000, SND_CHANNEL_PAN_CENTER );

    SND_StartChannel( 8 );

#else  // ENABLE_PCM

    SND_SetupChannelPcm( 1, &capBuf[0], MY_SND_WAVE_FMT, SND_CHANNEL_LOOP_REPEAT, 0, sizeof(capBuf[0])/4,
                        SND_CHANNEL_VOLUME_MAX, SND_CHANNEL_DATASHIFT_NONE, MY_SND_FREQ, SND_CHANNEL_PAN_MIN );
    SND_SetupChannelPcm( 3, &capBuf[1], MY_SND_WAVE_FMT, SND_CHANNEL_LOOP_REPEAT, 0, sizeof(capBuf[1])/4,
                        SND_CHANNEL_VOLUME_MAX, SND_CHANNEL_DATASHIFT_NONE, MY_SND_FREQ, SND_CHANNEL_PAN_MAX );

    SND_SetupChannelPcm( 0, &wavBuf[0], MY_SND_WAVE_FMT, SND_CHANNEL_LOOP_REPEAT, 0, sizeof(wavBuf[0])/4,
                        SND_CHANNEL_VOLUME_MAX, SND_CHANNEL_DATASHIFT_NONE, MY_SND_FREQ, SND_CHANNEL_PAN_MIN );
    SND_SetupChannelPcm( 2, &wavBuf[1], MY_SND_WAVE_FMT, SND_CHANNEL_LOOP_REPEAT, 0, sizeof(wavBuf[1])/4,
                        SND_CHANNEL_VOLUME_MAX, SND_CHANNEL_DATASHIFT_NONE, MY_SND_FREQ, SND_CHANNEL_PAN_MAX );

    SND_StartChannel( 1 );
    SND_StartChannel( 3 );

    SND_StartChannel( 0 );
    SND_StartChannel( 2 );

#endif  // ENABLE_PCM

    (void)OS_RestoreInterrupts(enabled);
}


static void MY_SndTerminate( void )
{
    int i;
    OSIntrMode enabled = OS_DisableInterrupts();

    SND_StopCapture( SND_CAPTURE_0 );
    SND_StopCapture( SND_CAPTURE_1 );

    for (i=0; i<16; i++)
    {
        SND_StopChannel( i, 0 );
    }

    (void)OS_RestoreInterrupts(enabled);
}

static void PrintfCaptureBuf( int startIdx )
{
    int i, ii;

    for (i=0; i<2; i++)
    {
        OS_TPrintf( "%s : ", i ==0 ? "L" : "R" );
        for (ii=0; ii<16; ii++)
        {
            OS_TPrintf( MY_SND_BYTES == 2 ? "%4.4x " : "%2.2x ", (u16)capBuf[i][startIdx + ii] );
        }
        OS_TPrintf( "\n" );
    }
}

static void CheckSound( void )
{
   int i, ii;

    OS_TPrintf( "\nDump reverb buffer.\n" );
    OS_TPrintf( "[Top]\n" );
    PrintfCaptureBuf( 0 );
    OS_TPrintf( "[Bottom]\n" );
    PrintfCaptureBuf( MY_SND_CAPTURE_LEN - 16 );

    OS_TPrintf( "\nDump mic buffer.\n" );
    for (i=0; i<2; i++)
    {
        for (ii=0; ii<16; ii++)
        {
            OS_TPrintf( "%4.4x ", micBuf[i*16+ii] );
        }
        OS_TPrintf( "\n" );
    }
}

static void TestFunc( void )
{
    MY_SndInit();
    OS_TPrintf( "\nSound starts.\n" );

    MICi_Start( MIC_SMP_ALL, MIC_DEFAULT_DMA_NO, micBuf, sizeof(micBuf) );
    OS_TPrintf( "\nMIC starts.\n");

    OS_SpinWait( OS_MSEC_TO_CPUCYC( 60 ) );

    MICi_Stop();
    OS_TPrintf( "\nMIC stops.\n");

    MY_SndTerminate();
    OS_TPrintf( "\nSound stops.\n" );

    CheckSound();
}

void TwlSpMain(void)
{
    OS_Init();
    OS_InitThread();

#ifdef ENABLE_OLD_SND
    reg_CFG_DS_MDFY |= REG_CFG_DS_MDFY_SND_MASK;  // SOUND回路バグ修正 (default: off)
    reg_CFG_DS_MDFY |= REG_CFG_DS_MDFY_SDMA_MASK; // SOUND-DMAバグ修正 (default: off)
    reg_CFG_DS_EX &= ~REG_CFG_DS_EX_SDMA2_MASK;   // SOUND-DMA新回路 (default: on)
#endif

    // ボタン入力サーチ初期化
    (void)PAD_InitXYButton();

    // 割込み許可
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // サウンド初期化
    SND_Init(THREAD_PRIO_SND);

    // マイク初期化
    MICi_Init();

    OS_TPrintf("\nARM7 starts.\n");

    // round robin dma test
    OS_TPrintf( "\nChange Round Robin Mode.\n" );

    MIi_SetExDmaArbitration( MI_EXDMAGBL_ARB_PRIORITY );

    TestFunc();

    OS_SpinWait( OS_MSEC_TO_CPUCYC( 60 ) );

    // priority dma test
    OS_TPrintf( "\nChange Priority Mode.\n" );

    MIi_SetExDmaArbitration( MI_EXDMAGBL_ARB_ROUND_ROBIN );
    MIi_SetExDmaYieldCycles( MI_EXDMAGBL_YLD_CYCLE_DEFAULT );

    TestFunc();

    OS_TPrintf("\nARM7 ends.\n");
    OS_Terminate();
}
