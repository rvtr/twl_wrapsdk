/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - twlmic
  File:     twl_mic_api.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  現在未実装の機能として以下の2つがあります。
　・Auto Gain Control (ゲイン自動調節機能）
　・IIR Filter (音声入出力フィルタ : High/Low Pass Filter など）

　但し、これらの機能は CODEC のセカンドソース品でも互換性を保てるか
　どうか確認がとれていないため、ボツる可能性もあります。
 ----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*

≪TWLにおけるマイクデータの拾い方≫

現在は FIFO Half DMA起動方式 を実装。


★★ FIFO Half DMA起動方式 ★★

ブロック転送サイズ　　：８ワード
インターバル　　　　　：０
転送ワード　　　　　　：８ワード
総転送ワード　　　　　：バッファサイズ
コンティニュアスモード：ＯＦＦ

＜解説＞

FIFOに半分データが溜まる度にDMA起動要求がかかり
８ワード転送される。
バッファサイズ分の転送が完了後に割り込みがかかるので
そこでDMAを再設定。

＜メリット＞
・FIFOとDMAのタイミングがきっちりと合う。
・色々な周波数に対応しやすい。

＜デメリット＞
・DMAがどこまで進んだか分からないためMIC_GetLastSamplingAddress
　で正確なアドレスが取れない。チックを使用して大体の位置を
　予測することは可能。

★★ インターバル方式 ★★

ブロック転送サイズ　　：８ワード
インターバル　　　　　：適切値
転送ワード　　　　　　：バッファサイズ
総転送ワード　　　　　：バッファサイズ
コンティニュアスモード：ＯＮ

＜解説＞

周波数からFIFOにデータが溜まるタイミングを逆算し、
インターバルを使って見込みでブロックDMAを実行する。
すこしずつDMAとFIFOのタイミングがずれていく可能性
があるが、バッファサイズ転送毎に再同期が可能。

＜メリット＞
・DMAの再設定が必要ない

＜デメリット＞
・他のDMAによりFIFOとのタイミングがずれていく可能性がある。
・そのため大きなバッファサイズには向かない
・DMAがどこまで進んだか分からないためMIC_GetLastSamplingAddress
　で正確なアドレスが取れない。チックを使用して大体の位置を
　予測することは可能。

★★ 割り込み頻発方式 ★★

＜解説＞

FIFOにデータが半分溜まる度にMIC割り込みを発生させ
FIFOからCPUで８ワード取得する。

＜メリット＞
・バッファのどの位置まで進んだかを正確に把握可能。
　MIC_GetLastSamplingAddressで正確なアドレスを返すことができる。

＜デメリット＞
・頻繁に割り込みが発生する。といってもその頻度はDS時代の1/16になる。
・そもそもMIC_GetLastSamplingAddressに有用性がないならこの方式はありえない。

 ----------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/mic.h>
#include <twl/cdc.h>
#include <twl/snd/ARM7/i2s.h>

/*---------------------------------------------------------------------------*
    マクロ定義
 *---------------------------------------------------------------------------*/

#define TWL_MIC_TICK_INIT_VALUE   0

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/

static TWLMICParam sMicParam;	// パラメータ保存用
static OSTick sMicTickStart;	// サンプリング開始時Tick
static OSTick sMicTickStop;		// サンプリング停止時Tick

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

static void TWL_MICi_ExDmaRecvAsync( u32 dmaNo, void *dest, u32 size );
static void TWL_MICi_ExDmaInterruptHandler( void );
static void TWL_MICi_FifoInterruptHandler( void );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSampling

  Description:  単発（16bit）のサンプリングを行います。

  Arguments:    buffer : サンプリング結果を格納するバッファ

  Returns:      None
 *---------------------------------------------------------------------------*/
void
TWL_MIC_DoSampling( u16* buffer )
{
	u32 word;
	const u32 WAIT_MAX = 100;
	u32 i;

	// 引数チェック
	SDK_NULL_ASSERT( buffer );

	// サウンド & I2S 回路ON
    SND_Enable();

    // モノラルサンプリングスタート
    reg_SND_MICCNT = REG_SND_MICCNT_FIFO_CLR_MASK;
    reg_SND_MICCNT = (u16)(REG_SND_MICCNT_E_MASK | REG_SND_MICCNT_NR_MASK | 
						   MIC_INTR_DISABLE | MIC_SMP_ALL );

	// データが2個以上入るのを待つ
	for (i=0;i<WAIT_MAX;i++)
	{
		if (!(reg_SND_MICCNT & REG_SND_MICCNT_FIFO_EMP_MASK))
		{
			break;
		}
	}

	// FIFOはワードアクセス
	word = reg_SND_MIC_FIFO;

	// サンプリング停止
    reg_SND_MICCNT &= ~REG_SND_MICCNT_E_MASK;

	// 16bit値を格納
	*buffer = (u16)(word & 0x0000ffff);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StartAutoSampling

  Description:  オートサンプリングを開始します。
				現在モノラル固定のサンプリングになっています。

  Arguments:    param : オートサンプリング用のパラメータ

  Returns:      None
 *---------------------------------------------------------------------------*/
void
TWL_MIC_StartAutoSampling( TWLMICParam param )
{
    OSIntrMode enabled;
	u32 ch;

	// 引数チェック
	SDK_ASSERT( param.dmaNo >= MI_EXDMA_CH_MIN && param.dmaNo <= MI_EXDMA_CH_MAX );
	SDK_NULL_ASSERT( param.buffer );

	// DMA停止
    TWL_MIC_StopAutoSampling();

	// 割り込み禁止
    enabled = OS_DisableInterrupts();

	// サウンド & I2S 回路ON
    SND_Enable();

	// DMA設定
    TWL_MICi_ExDmaRecvAsync( param.dmaNo, param.buffer, param.size );

	// DMA割り込みの設定
    ch = (u32)param.dmaNo - MI_EXDMA_CH_MIN;
    OS_SetIrqFunction( OS_IE_DMA4 << ch, TWL_MICi_ExDmaInterruptHandler );
    reg_OS_IF  = (OS_IE_DMA4 << ch);
    reg_OS_IE |= (OS_IE_DMA4 << ch);

	// マイクFIFO割り込みの設定
    OS_SetIrqFunction( OS_IE_MIC, TWL_MICi_FifoInterruptHandler );
    reg_OS_IF2  = (OS_IE_MIC >> 32);
    reg_OS_IE2 |= (OS_IE_MIC >> 32);

	// スタートTick設定
	sMicTickStart = OS_GetTick();

	// ストップTickクリア
	sMicTickStop = TWL_MIC_TICK_INIT_VALUE;

    // モノラルサンプリングスタート
    reg_SND_MICCNT = REG_SND_MICCNT_FIFO_CLR_MASK;		// 
    reg_SND_MICCNT = (u16)(	REG_SND_MICCNT_E_MASK 	| 	// 
							REG_SND_MICCNT_NR_MASK 	| 	// 
							MIC_INTR_OVERFLOW		| 	// FIFO破綻で割り込み発生
							param.frequency );

	// パラメータ保存
	sMicParam = param;

	// 割り込み復帰
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSampling

  Description:  オートサンプリングを停止します。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void
TWL_MIC_StopAutoSampling( void )
{
    OSIntrMode enabled = OS_DisableInterrupts();
    u32 dmaNo = sMicParam.dmaNo;

	// ストップTick設定
	sMicTickStop = OS_GetTick();

	// マイク停止
    reg_SND_MICCNT &= ~REG_SND_MICCNT_E_MASK;

	// マイクFIFO割り込み禁止
    reg_OS_IE2 &= ~(OS_IE_MIC >> 32);
    reg_OS_IF2  =  (OS_IE_MIC >> 32);

    if ( MI_EXDMA_CH_MIN <= dmaNo && dmaNo <= MI_EXDMA_CH_MAX )
    {
        u32 ch = dmaNo - MI_EXDMA_CH_MIN;

		// DMA停止
        MIi_StopExDma( dmaNo );

		// DMA割り込み禁止
        reg_OS_IE &= ~(OS_IE_DMA4 << ch);
        reg_OS_IF  =  (OS_IE_DMA4 << ch);
    }

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddress

  Description:  最新サンプリングデータの格納アドレスを返します。
				マイクFIFOからのデータ取得にはDMAを用いますが、
				DMAが現在どの位置を書き換え中かは知る手段がありません。
				そのためサンプリング経過時間より現在の位置を予想します。

  Arguments:    None

  Returns:      最新サンプリングデータのアドレス（予想）
 *---------------------------------------------------------------------------*/
void*
TWL_MIC_GetLastSamplingAddress( void )
{
	void* adress;					// 最新データアドレス
	OSTick past_tick;				// 経過チック
	u32 sampling_count;				// 予想サンプリングカウント
	f32 one_sampling_tick;

	// サンプリング停止状態
	if (sMicTickStop != TWL_MIC_TICK_INIT_VALUE)
	{
		past_tick = sMicTickStop - sMicTickStart;
	}
	// サンプリング進行状態
	else
	{
		past_tick = OS_GetTick() - sMicTickStart;
	}

	/*
	   サンプリングレート47.61kHzの場合、
	   システムクロックが33.514MHz で
	   Tickは64分周のタイマカウンタであるため
	   1サンプリングに必要なチックは
	   
	   33.514MHz / 47.61kHz / 64 = 10.9988 チック となる
	 
	   FPGAボードではさらに半分にする必要がある
	*/

	switch ( I2S_GetSamplingRate() )
	{
		case I2S_SAMPLING_RATE_32730:
			switch ( sMicParam.frequency )
			{
				case MIC_SMP_ALL:         // 32.73 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 1 / 32730 / 64;
					break;
    			case MIC_SMP_1_2:         // 16.36 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 2 / 32730 / 64;
					break;
    			case MIC_SMP_1_3:         // 10.91 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 3 / 32730 / 64;
					break;
    			case MIC_SMP_1_4:         // 8.18 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 4 / 32730 / 64;
					break;
			}
			break;
		case I2S_SAMPLING_RATE_47610:
			switch ( sMicParam.frequency )
			{
				case MIC_SMP_ALL:         // 47.61 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 1 / 47610 / 64;
					break;
    			case MIC_SMP_1_2:         // 23.81 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 2 / 47610 / 64;
					break;
    			case MIC_SMP_1_3:         // 15.87 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 3 / 47610 / 64;
					break;
    			case MIC_SMP_1_4:         // 11.90 kHz
					one_sampling_tick = (f32)OS_SYSTEM_CLOCK * 4 / 47610 / 64;
					break;
			}
			break;
	}

	// 現在のサンプリング数を計算
	sampling_count = (u32)(past_tick / one_sampling_tick);

	// サンプリング数よりアドレスを計算
	adress = (void*)((u16 *)sMicParam.buffer + sampling_count); 

	return adress;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGain

  Description:  TWLモードのマイクゲイン(PGAB)を設定します。
　　　　　　　　PGABはAutoGainControlが無効になっているときのみ有効です。

  Arguments:    gain : 設定ゲイン（0～119 = 0～59.5dB）

  Returns:      None
 *---------------------------------------------------------------------------*/
void
TWL_MIC_SetAmpGain( u8 gain )
{
	SDK_ASSERT( gain >= 0 && gain <= 119 );
	CDC_SetPGAB( gain );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetAmpGain

  Description:  TWLモードのマイクゲイン(PGAB)を取得します。

  Arguments:    None

  Returns:      設定ゲイン
 *---------------------------------------------------------------------------*/
u8
TWL_MIC_GetAmpGain( void )
{
	return CDC_GetPGAB();
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_ExDmaRecvAsync

  Description:  receive data with DMA
                async version

  Arguments:    dmaNo   : DMA channel No.
                buffer  : destination address
                size    : size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
static void
TWL_MICi_ExDmaRecvAsync( u32 dmaNo, void *buffer, u32 size )
{
    MIi_ExDmaRecvAsyncCore( 
			dmaNo, 						// DMA番号
			(void*)REG_MIC_FIFO_ADDR, 	// 転送元アドレス
			buffer, 					// 転送先アドレス
            (u32)size, 					// 総転送バイト数
			(u32)32, 					// 転送バイト数
            MI_EXDMA_BLOCK_32B, 		// ブロック転送サイズ
			0,							// インターバル 
			MI_EXDMA_PRESCALER_1,		// プリスケール1
            MI_EXDMA_CONTINUOUS_OFF, 	// 非コンティニュアス
			MI_EXDMA_SRC_RLD_OFF, 		// Source Reload Don't Care
			MI_EXDMA_DEST_RLD_OFF,		// Destination Reload Off
            MI_EXDMA_TIMING_MIC );		// マイクFIFOハーフ起動
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_ExDmaInterruptHandler

  Description:  interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void 
TWL_MICi_ExDmaInterruptHandler( void )
{
	// ループする？
	if ( sMicParam.loop )
	{
 		// DMAを再設定
        TWL_MICi_ExDmaRecvAsync( sMicParam.dmaNo, sMicParam.buffer, sMicParam.size );

		// スタートTick再設定
		sMicTickStart = OS_GetTick();
	}

	// DMA完了コールバックの呼び出し
	if ( sMicParam.callback )
	{
		sMicParam.callback( sMicParam.loop );
	}
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_FifoInterruptHandler

  Description:  マイクFIFO破綻時に呼び出される割り込みハンドラ
				現在の実装ではFIFOクリア後にサンプリングを再スタート
				させています。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void 
TWL_MICi_FifoInterruptHandler( void )
{
	// マイク停止
    reg_SND_MICCNT &= ~REG_SND_MICCNT_E_MASK;

    // マイクFIFOクリア
    reg_SND_MICCNT = REG_SND_MICCNT_FIFO_CLR_MASK;

	// モノラルサンプリング再スタート
    reg_SND_MICCNT = (u16)(	REG_SND_MICCNT_E_MASK 	|
							REG_SND_MICCNT_NR_MASK 	| 
							MIC_INTR_OVERFLOW		| 	// FIFO破綻で割り込み発生
							sMicParam.frequency );
}

