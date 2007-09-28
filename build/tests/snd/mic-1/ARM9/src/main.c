/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - tests - mic-1
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
#include <twl.h>
#include <twl/mic.h>
#include "organ_det.g3.pcm16.h"
#include "wihaho.pcm16.h"

// PSGの周波数を計算するマクロ
#define FreqToTimer(freq) (SND_TIMER_CLOCK / ( 8 * (freq) ))
#define KeyToTimer(key) (SND_CalcTimer(FreqToTimer(440), ((key)-69)*64))

#define PCM_PLAY_CHANNEL1   4
#define PCM_PLAY_CHANNEL2   5
#define PSG_PLAY_CHANNEL    8
#define NOISE_PLAY_CHANNEL  14

#define CENTER_PAN 64

u16     Cont;
u16     Trg;
u8      key = 60;

void    VBlankIntr(void);
void    DrawPos(int x, int y);
void Line(int x1, int y1, int x2, int y2);

//////////////
// 約１フレームのサンプル数
// 32バイト単位が望ましい
// 32.73kHz / 60 = 545.5 -> 544
// 47.61kHz / 60 = 793.5 -> 800
#define FRAME_SAMPLES (800)

// 何フレーム分のバッファを用意するか
#define REC_FRAMES    (300)

// サンプリングバッファサイズ
#define MY_MIC_BUF_LEN (FRAME_SAMPLES*REC_FRAMES)


s16 wave[MY_MIC_BUF_LEN/2]   __attribute__ ((aligned (32)));

int view_x[FRAME_SAMPLES];
int view_y[FRAME_SAMPLES];

s32 wave_index = 0;
//////////////

void FullCallback(TWLMICResult result, void* arg)
{
//	OS_Printf("FullCallback was called.\n");
//	OS_Printf("result = %d\n", result);
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
	int i;
	TwlMicAutoParam param;

    OS_InitPrintServer();

    OS_Init();
    GX_Init();
    SND_Init();

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // print usage
    OS_Printf("=================================\n");
    OS_Printf("USAGE:\n");
    OS_Printf(" A, B, X, Y : start sound\n");
    OS_Printf(" START : stop sound\n");
    OS_Printf("=================================\n");

    // チャンネルをロックする
    SND_LockChannel((1 << PCM_PLAY_CHANNEL1) | (1 << PCM_PLAY_CHANNEL2) | (1 << PSG_PLAY_CHANNEL) |
                    (1 << NOISE_PLAY_CHANNEL), 0);


    GX_DispOff();
    GXS_DispOff();

    //---------------------------------------------------------------------------
    // All VRAM banks to LCDC
    //---------------------------------------------------------------------------
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    //---------------------------------------------------------------------------
    // Clear all LCDC space
    //---------------------------------------------------------------------------
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    //---------------------------------------------------------------------------
    // Set graphics mode VRAM display mode
    //---------------------------------------------------------------------------
    GX_SetGraphicsMode(GX_DISPMODE_VRAM_A,      // display VRAM-A
                       (GXBGMode)0,    			// dummy
                       (GXBG0As)0);    			// dummy

    GX_DispOn();
    GXS_DispOn();

	//////////////////////////////////// 

	TWL_MIC_Init();

	{
		param.dmaNo         = 6;
		param.buffer        = wave;
		param.size          = MY_MIC_BUF_LEN;
		param.frequency      = 0; //MIC_SMP_ALL;
		param.loop_enable   = 1;
		param.full_callback 		= FullCallback;
		param.full_arg   = 0;

		TWL_MIC_StartAutoSampling( &param );
	}

	////////////////////////////////////

    while (1)
    {
        u16     ReadData;

        OS_WaitVBlankIntr();

        // ＡＲＭ７コマンド応答受信
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }

		//////////////////////// 単発サンプリングテスト
/*
		{
			u16 buf;
			TWLMICResult result = TWL_MIC_DoSampling(&buf);
			if (result == TWL_MIC_RESULT_SUCCESS)
			{
				OS_Printf("%x\n", buf);
			}
			else
			{
				OS_Printf("fail\n");
			}
		}
*/
		////////////////////////

		{
			static u8 gain;
			u8 now;

	        if (Trg & PAD_KEY_UP)
			{
				TWL_MIC_SetAmpGain( ++gain );
				TWL_MIC_GetAmpGain( &now );
				OS_Printf("gain = %d", now);
			}
			else if (Trg & PAD_KEY_DOWN)
			{
				TWL_MIC_SetAmpGain( --gain );
				TWL_MIC_GetAmpGain( &now );
				OS_Printf("gain = %d", now);
			}
		}

        if (Trg & PAD_KEY_RIGHT)
		{
			void* address;
			TWL_MIC_GetLastSamplingAddress( &address );
			OS_Printf("0x%x\n", address);
		}

        ReadData = PAD_Read();
        Trg = (u16)(ReadData & (ReadData ^ Cont));
        Cont = ReadData;

        // PCM再生
        if (Trg & PAD_BUTTON_A)
        {
            OS_Printf("A\n");
            SND_SetupChannelPcm(PCM_PLAY_CHANNEL1,
                                ORGAN_DET_G3_PCM16_FORMAT,
                                organ_det_g3_pcm16,
                                ORGAN_DET_G3_PCM16_LOOPFLAG ? SND_CHANNEL_LOOP_REPEAT :
                                SND_CHANNEL_LOOP_1SHOT, ORGAN_DET_G3_PCM16_LOOPSTART,
                                ORGAN_DET_G3_PCM16_LOOPLEN, 127, SND_CHANNEL_DATASHIFT_NONE,
                                SND_CalcTimer(ORGAN_DET_G3_PCM16_TIMER, ((key) - 67) * 64),
                                CENTER_PAN);
            SND_StartTimer(1 << PCM_PLAY_CHANNEL1, 0, 0, 0);
        }

        if (Trg & PAD_BUTTON_R)
        {
            OS_Printf("R\n");
			///////////////////////////////////
			TWL_MIC_StartAutoSampling( &param );
			///////////////////////////////////
        }

        if (Trg & PAD_BUTTON_L)
        {
            OS_Printf("L\n");
			///////////////////////////////////
			TWL_MIC_StopAutoSampling();
			///////////////////////////////////
        }

        if (Trg & PAD_BUTTON_B)
        {
            OS_Printf("B\n");
/*
            SND_SetupChannelPcm(PCM_PLAY_CHANNEL2,
                                WIHAHO_PCM16_FORMAT,
                                wihaho_pcm16,
                                WIHAHO_PCM16_LOOPFLAG ? SND_CHANNEL_LOOP_REPEAT :
                                SND_CHANNEL_LOOP_1SHOT, WIHAHO_PCM16_LOOPSTART,
                                WIHAHO_PCM16_LOOPLEN, 127, SND_CHANNEL_DATASHIFT_NONE,
                                WIHAHO_PCM16_TIMER, CENTER_PAN);
*/
            SND_SetupChannelPcm(PCM_PLAY_CHANNEL2,
                                SND_WAVE_FORMAT_PCM16,
                                (void *)wave,
                                SND_CHANNEL_LOOP_REPEAT, // SND_CHANNEL_LOOP_1SHOT,
								0,
                                MY_MIC_BUF_LEN/4, 
								127,
								SND_CHANNEL_DATASHIFT_NONE,
                                379, // 524, //379, 
								CENTER_PAN);
            SND_StartTimer(1 << PCM_PLAY_CHANNEL2, 0, 0, 0);
        }



        // 停止
        if (Trg & PAD_BUTTON_START)
        {
            SND_StopTimer((1 << PCM_PLAY_CHANNEL1) | (1 << PCM_PLAY_CHANNEL2) |
                          (1 << PSG_PLAY_CHANNEL) | (1 << NOISE_PLAY_CHANNEL), 0, 0, 0);
        }

    	MI_CpuClearFast((void *)HW_LCDC_VRAM, 2*256*192);

		DC_InvalidateRange( wave, MY_MIC_BUF_LEN );
/*
		for (i=0;i<192;i++)
		{
			s16 x = *(s16 *)(wave + i*2);
			DrawPos(128 + x , i);
		}
*/

		for (i=0;i<FRAME_SAMPLES;i++)
		{
			// 時間軸(0～255)
			view_x[i] = i * 0.444;
			// サンプリング値（±32768 → ±100）/327.68
			// サンプリング値（±32768 → ±100）/327.68
			view_y[i] = *(s16 *)((u32)wave + (wave_index + i)*2) / 32.768 + 96;
		}

		wave_index += FRAME_SAMPLES;
		if (wave_index >= MY_MIC_BUF_LEN/2)
		{
			wave_index = 0;
		}

		for (i=0;i<FRAME_SAMPLES-1;i++)
		{
			Line(view_x[i], view_y[i], view_x[i+1], view_y[i+1]);
		}

        // コマンドフラッシュ
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

    }


    OS_TPrintf("\nARM9 starts.\n");
    OS_TPrintf("\nARM9 ends.\n");
    OS_Terminate();
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

//--------------------------------------------------------------------------------
//    画面座標描画
//
void DrawPos(int x, int y)
{
    const int screen_width  = 256;
    const int screen_height = 191;
	int address;

// if agc
//	x >>= 10;
//

    address = HW_LCDC_VRAM + screen_width*2*y + 2*x;

	*(u16 *)(address) = 0x7fff;
}


int ABS(x){ return (x>=0)? (x) : (-x); }

void Line(int x1, int y1, int x2, int y2)
{
	int dx, dy, s, step;

	dx = ABS(x2 - x1);
	dy = ABS(y2 - y1);
	if (dx > dy)
	{
		if (x1 > x2)
		{
			step = (y1 > y2) ? 1 : -1;
			s = x1; x1 = x2; x2 = s; y1 = y2;
		} else step = (y1 < y2) ? 1 : -1;

		DrawPos(x1, y1);
		s = dx >> 1;
		while (++x1 <= x2)
		{
			if ((s -= dy) < 0)
			{
				s  += dx;
				y1 += step;
			}
			DrawPos(x1, y1);
		}
	}
	else
	{
		if (y1 > y2)
		{
			step = (x1 > x2) ? 1 : -1;
			s = y1; y1 = y2; y2 = s; x1 = x2;
		} else step = (x1 < x2) ? 1 : -1;

		DrawPos(x1, y1);
		s = dy >> 1;
		while (++y1 <= y2)
		{
			if ((s -= dx) < 0)
			{
				s += dy; x1 += step;
			}
			DrawPos(x1, y1);
		}
	}
}

