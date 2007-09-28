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
#include    <twl/cdc.h>     // for DS mode
#include    <twl/snd/ARM7/i2s.h>

#include <math.h>

#define OSi_IDLE_CHECKNUM_SIZE  ( sizeof(u32)*2 )
#define OSi_IDLE_SVC_SIZE  ( sizeof(u32)*16 )   // arm7 svc stacks 14 words
#define OSi_IDLE_THREAD_STACK_SIZE    ( OSi_IDLE_CHECKNUM_SIZE + OSi_IDLE_SVC_SIZE )
extern u32     OSi_IdleThreadStack[OSi_IDLE_THREAD_STACK_SIZE / sizeof(u32)];
extern OSThread OSi_IdleThread;


// ===== スレッド優先度 =====

#define THREAD_PRIO_SPI         2
#define THREAD_PRIO_SND         6

//#define MY_MIC_BUF_LEN    33504

// 約１フレームのサンプル数
// 32バイト単位が望ましい
// 32.73kHz / 60 = 558.592 -> 576
#define FRAME_SAMPLES (576)

// 何フレーム分のバッファを用意するか
#define REC_FRAMES    (300)

// サンプリングバッファサイズ
#define MY_MIC_BUF_LEN (FRAME_SAMPLES*REC_FRAMES)

//u16       micBuf[MY_MIC_BUF_LEN]  __attribute__ ((aligned (32)));
u16* micBuf = (u16 *)0x0203AE40;


///////////////////
#define OS_TPrintf( ...)    ((void)0)
///////////////////

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystem(void);
static void VBlankIntr(void);

/*---------------------------------------------------------------------------*
  Name:         OSi_IdleThreadProc

  Description:  procedure of idle thread which system creates

  Arguments:    None

  Returns:      None (never return)
 *---------------------------------------------------------------------------*/
static void OSi_IdleThreadProc(void *)
{
    (void)OS_EnableInterrupts();
    while (1)
    {
        OS_Halt();
    }
    // never return
}

/*---------------------------------------------------------------------------*
  Name:         TwlSpMain

  Description:  Initialize and do main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void TwlSpMain(void)
{
    OSHeapHandle heapHandle;

    OS_Init();
    OS_InitThread();

    // PXI初期化、ARM9と同期
    PXI_Init();

    // ヒープ領域設定
    heapHandle = InitializeAllocateSystem();

//    reg_CFG_DS_MDFY |= REG_CFG_DS_MDFY_SND_MASK;  // SOUND回路バグ修正 (default: off)
//    reg_CFG_DS_MDFY |= REG_CFG_DS_MDFY_SDMA_MASK; // SOUND-DMAバグ修正 (default: off)
//    reg_CFG_DS_EX &= ~REG_CFG_DS_EX_SDMA2_MASK;   // SOUND-DMA新回路 (default: on)

    // ボタン入力サーチ初期化
    (void)PAD_InitXYButton();

    // create idle thread to sleep in main thread
    OS_CreateThread(&OSi_IdleThread,
                    OSi_IdleThreadProc,
                    (void *)NULL,
                    OSi_IdleThreadStack + OSi_IDLE_THREAD_STACK_SIZE / sizeof(u32),
                    OSi_IDLE_THREAD_STACK_SIZE,
                    OS_THREAD_PRIORITY_MAX /*pseudo. change at next line. */ );
    OSi_IdleThread.priority = OS_THREAD_PRIORITY_MAX + 1;       // lower priority than the lowest (=OS_THREAD_PRIORITY_MAX)
    OSi_IdleThread.state = OS_THREAD_STATE_READY;

    // 割込み許可
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

	// CODEC初期化
	CDC_Init();

    // サウンド初期化
    SND_Init(THREAD_PRIO_SND);

    // マイク初期化
//  MICi_Init();
	CDC_InitMic();

///
TWL_MIC_InitServer(6);
///

	// サンプリング周波数設定
	SND_Disable();
//	I2S_SetSamplingRate(I2S_SAMPLING_RATE_32730);	// 32kHz
	I2S_SetSamplingRate(I2S_SAMPLING_RATE_47610);		// 48kHz
	SND_Enable();

    // SPI初期化
    SPI_Init(THREAD_PRIO_SPI);

    OS_TPrintf("\nARM7 starts.\n");

//    MICi_Start( MIC_SMP_ALL, MIC_DEFAULT_DMA_NO, micBuf, MY_MIC_BUF_LEN );

	while(1)
	{
//		OS_Printf("0x%x\n", TWL_MIC_GetLastSamplingAddress());
		OS_Sleep(16);
	}
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  メモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    OS_TPrintf("OS_GetWramSubPrivArenaLo() = %p\n", OS_GetWramSubPrivArenaLo());
    OS_TPrintf("OS_GetWramSubPrivArenaHi() = %p\n", OS_GetWramSubPrivArenaHi());
    OS_TPrintf("OS_GetWramSubArenaLo() = %p\n", OS_GetWramSubArenaLo());
    OS_TPrintf("OS_GetWramSubArenaHi() = %p\n", OS_GetWramSubArenaHi());
    OS_TPrintf("OS_GetSubPrivArenaLo() = %p\n", OS_GetSubPrivArenaLo());
    OS_TPrintf("OS_GetSubPrivArenaHi() = %p\n", OS_GetSubPrivArenaHi());

    OS_TPrintf("call OS_SetWramSubPrivArenaHi(0x0380f980); to fix arena.\n");
    OS_SetWramSubPrivArenaHi((void*)0x0380f980);

    // メモリ割当て初期化
    tempLo = OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV,
                          OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi(), 1);

    // アリーナを0クリア
    MI_CpuClear8(tempLo, (u32)OS_GetWramSubPrivArenaHi() - (u32)tempLo);

    // アリーナ下位アドレスを設定
    OS_SetArenaLo(OS_ARENA_WRAM_SUBPRIV, tempLo);

    // ヒープ作成
    hh = OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV,
                       OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi());

    if (hh < 0)
    {
        OS_Panic("ARM7: Fail to create heap.\n");
    }

    // カレントヒープに設定
    (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);

    return hh;
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef SDK_TEG

extern BOOL PMi_Initialized;
void    PM_SelfBlinkProc(void);

static void VBlankIntr(void)
{
    //---- LED blink system
    if (PMi_Initialized)
    {
        PM_SelfBlinkProc();
    }
}

#else

static void VBlankIntr(void)
{
}

#endif

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
