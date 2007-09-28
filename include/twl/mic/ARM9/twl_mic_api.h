/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - twl-mic
  File:     twl_mic_api.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_MIC_MIC_API_H_
#define TWL_MIC_MIC_API_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// 処理結果定義
typedef enum _TWLMICResult
{
    TWL_MIC_RESULT_SUCCESS = 0,
    TWL_MIC_RESULT_ILLEGAL_PARAMETER,
    TWL_MIC_RESULT_INVALID_COMMAND,
    TWL_MIC_RESULT_ILLEGAL_STATUS,
    TWL_MIC_RESULT_BUSY,
    TWL_MIC_RESULT_SEND_ERROR,
    TWL_MIC_RESULT_FATAL_ERROR,
    TWL_MIC_RESULT_MAX
}
TWLMICResult;

typedef enum
{
    TWL_MIC_FREQUENCY_ALL             =  0x0,		// 47.61 / 32.73 kHz
    TWL_MIC_FREQUENCY_1_2             =  0x1,		// 23.81 / 16.36 kHz
    TWL_MIC_FREQUENCY_1_3             =  0x2,		// 15.87 / 10.91 kHz
    TWL_MIC_FREQUENCY_1_4             =  0x3,		// 11.90 /  8.18 kHz
    TWL_MIC_FREQUENCY_NUM			  =  0x4
}
TwlMicFrequency;

// コールバック
typedef void (*TwlMicCallback)(TWLMICResult result, void* arg);

// オートサンプリング用設定定義
typedef struct TwlMicAutoParam
{
	u32    			dmaNo;				// DMA No
    void   			*buffer;            // 結果格納バッファへのポインタ(DMAを使うため4byteアライメント）
    u32     		size;               // バッファサイズ(DMAを使うため4byte単位）
    TwlMicFrequency frequency;          // サンプリング頻度
    BOOL    		loop_enable;        // バッファフル時のループ可否
    TwlMicCallback 	full_callback;     	// バッファフル時のコールバック
    void*			full_arg;   		// 上記コールバックに指定する引数
}
TwlMicAutoParam;


/*---------------------------------------------------------------------------*
  Name:         MIC_Init

  Description:  MICライブラリを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
TWL_MIC_Init(void);

/*---------------------------------------------------------------------------*
  Name:         MIC_End

  Description:  MICライブラリを終了する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
TWL_MIC_End(void);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSamplingAsync

  Description:  マイク単発サンプリング開始（非同期版）

  Arguments:    

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_DoSamplingAsync(u16* buf,  TwlMicCallback callback, void* callbackArg);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_DoSampling

  Description:  マイク単発サンプリング開始（同期版）

  Arguments:    

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_DoSampling(u16* buf);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StartAutoSamplingAsync

  Description:  マイク自動サンプリング開始（非同期版）

  Arguments:    param

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_StartAutoSamplingAsync(TwlMicAutoParam* param, TwlMicCallback callback, void* callbackArg);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StartAutoSampling

  Description:  マイク自動サンプリング開始（同期版）

  Arguments:    param

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StartAutoSampling(TwlMicAutoParam* param);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSampling

  Description:  マイク自動サンプリング停止（同期版）

  Arguments:    param

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StopAutoSampling( void );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_StopAutoSamplingAsync

  Description:  マイク自動サンプリング停止（非同期版）

  Arguments:    none

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult 
TWL_MIC_StopAutoSamplingAsync(  TwlMicCallback callback, void* callbackArg );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddressAsync

  Description:  最新のサンプリングデータの格納アドレスを返します。
				但し、アドレスはサンプリング時間を元に算出したものである
				ため誤差を含んでいます。さらにARM7からARM9へと通知してい
				る間にサンプリングは進んでいる可能性があります。
				（非同期版）

  Arguments:    adress      : アドレス格納ポインタのアドレス
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetLastSamplingAddressAsync( void** adress, TwlMicCallback callback, void* callbackArg );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetLastSamplingAddress

  Description:  最新のサンプリングデータの格納アドレスを返します。
				但し、アドレスはサンプリング時間を元に算出したものである
				ため誤差を含んでいます。さらにARM7からARM9へと通知してい
				る間にサンプリングは進んでいる可能性があります。
				（同期版）

  Arguments:    adress      : アドレス格納ポインタのアドレス

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetLastSamplingAddress( void** adress );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGainAsync

  Description:  プログラマブルゲインアンプの設定を行います。（非同期版）
				この関数で設定したゲインはオートゲインコントロールが
				無効になっているときのみ有効となることに注意してください。

  Arguments:    gain 		: 設定ゲイン（0～119 = 0～59.5dB）
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_SetAmpGainAsync( u8 gain, TwlMicCallback callback, void* callbackArg );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_SetAmpGain

  Description:  プログラマブルゲインアンプの設定を行います。（同期版）
				この関数で設定したゲインはオートゲインコントロールが
				無効になっているときのみ有効となることに注意してください。

  Arguments:    gain 		: 設定ゲイン（0～119 = 0～59.5dB）

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_SetAmpGain( u8 gain );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetAmpGainAsync

  Description:  TWLモードのマイクゲイン(PGAB)を取得します。
				（非同期版）

  Arguments:    gain        : マイクゲイン値の格納アドレス
				callback    :
				callbackArg :  

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetAmpGainAsync( u8* gain, TwlMicCallback callback, void* callbackArg );

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_GetAmpGain

  Description:  TWLモードのマイクゲイン(PGAB)を取得します。
				（同期版）

  Arguments:    gain        : マイクゲイン値の格納アドレス

  Returns:      TWLMICResult
 *---------------------------------------------------------------------------*/
TWLMICResult
TWL_MIC_GetAmpGain( u8* gain );

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_MIC_MIC_API_H_ */
