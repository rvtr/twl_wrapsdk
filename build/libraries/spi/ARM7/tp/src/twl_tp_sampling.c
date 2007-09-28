/*---------------------------------------------------------------------------*
  Project:  NitroSDK - libraries - spi
  File:     twl_tp_sampling.c

  Copyright 2003-2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include    "tp_sp.h"
#include    <twl/cdc/ARM7/cdc.h>

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <nitro/os/common/printf.h>
#define DBG_PRINTF OS_TPrintf
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_CHAR( c )      ((void)0)
#endif

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetConversionMode

  Description:  set ADC target channel
  
  Arguments:    TpConversionMode_t mode : Conversion Mode

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetConversionMode( TpCnversionControl_t control, TpConversionMode_t mode, TpCnversionPin_t pin )
{
    SDK_ASSERT( (control == TP_CONVERSION_CONTROL_HOST) || (control == TP_CONVERSION_CONTROL_SELF) );
    SDK_ASSERT( (mode == TP_CONVERSION_MODE_NONE)   || (mode == TP_CONVERSION_MODE_XY)       ||
                (mode == TP_CONVERSION_MODE_XYZ)    || (mode == TP_CONVERSION_MODE_X)        ||
                (mode == TP_CONVERSION_MODE_Y)      || (mode == TP_CONVERSION_MODE_Z)        ||
                (mode == TP_CONVERSION_MODE_AUX3)   || (mode == TP_CONVERSION_MODE_AUX2)     ||
                (mode == TP_CONVERSION_MODE_AUX1)   || (mode == TP_CONVERSION_MODE_AUTO_AUX) ||
                (mode == TP_CONVERSION_MODE_AUX123) || (mode == TP_CONVERSION_MODE_XP_XM)    ||
                (mode == TP_CONVERSION_MODE_YP_YM)  || (mode == TP_CONVERSION_MODE_YP_XM)      );
    SDK_ASSERT( (pin >= TP_CONVERSION_PIN_INTERRUPT) || (pin <= TP_CONVERSION_PIN_NBM) );
    
	CDC_WriteSpiRegisterEx( 3, REG_TP_CONVERSION_MODE, (u8)( control | mode | pin ) );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetInterval

  Description:  set Touch-Panel / AUX Interval Time
                Either Touch-Panel or AUX can be enabled, the last setting
                is only valid. Normally, Touch-Panel is enabled.
  
  Arguments:    tpInterval_t interval : interval time between sampling

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetInterval( tpInterval_t interval )
{
    SDK_ASSERT( (interval == TP_INTERVAL_NONE) || 
                (interval == TP_INTERVAL_8MS)  || (interval == TP_AUX_INTERVAL_1_12M) ||
                (interval == TP_INTERVAL_1MS)  || (interval == TP_AUX_INTERVAL_3_36M) ||
                (interval == TP_INTERVAL_2MS)  || (interval == TP_AUX_INTERVAL_5_59M) ||
                (interval == TP_INTERVAL_3MS)  || (interval == TP_AUX_INTERVAL_7_83M) ||
                (interval == TP_INTERVAL_4MS)  || (interval == TP_AUX_INTERVAL_10_01M) ||
                (interval == TP_INTERVAL_5MS)  || (interval == TP_AUX_INTERVAL_12_30M) ||
                (interval == TP_INTERVAL_6MS)  || (interval == TP_AUX_INTERVAL_14_54M) ||
                (interval == TP_INTERVAL_7MS)  || (interval == TP_AUX_INTERVAL_16_78M)
              );
    
	CDC_WriteSpiRegisterEx( 3, REG_TP_INTERVAL, interval );
}

/*---------------------------------------------------------------------------*
  Name:         tpEnableNewBufferMode

  Description:  enable new buffer mode
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_EnableNewBufferMode( void )
{
    CDC_SetSpiParamsEx( 3, REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_E, TP_NEW_BUFFER_MODE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         tpDisableNewBufferMode

  Description:  disable new buffer mode
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_DisableNewBufferMode( void )
{
    CDC_SetSpiParamsEx( 3, REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_D, TP_NEW_BUFFER_MODE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetResolution

  Description:  set AD Converting Resolution (8, 10, or 12-bit)
  
  Arguments:    TpResolution_t res : Converting Resolution

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetResolution( TpResolution_t res )
{
    SDK_ASSERT( (res == TP_RESOLUTION_12) || 
                (res == TP_RESOLUTION_8)  ||
                (res == TP_RESOLUTION_10) );

    CDC_SetSpiParamsEx( 3, REG_TP_RESOLUTION, res, TP_RESOLUTION_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_GetResolution

  Description:  get AD Converting Resolution (8, 10, or 12-bit)
  
  Arguments:    TpResolution_t *res : Converting Resolution

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_GetResolution( TpResolution_t *res )
{
    *res = (TpResolution_t)( CDC_ReadSpiRegisterEx( 3, REG_TP_RESOLUTION ) & TP_RESOLUTION_MASK);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetStabilizationTime

  Description:  set ADC stabilization time before touch detection
  
  Arguments:    TpSetupTime_t time : stabilization time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetStabilizationTime( TpSetupTime_t time )
{
    SDK_ASSERT( (TP_SETUP_TIME_0_1US <= time) || (time <= TP_SETUP_TIME_1MS) );
	CDC_SetSpiParamsEx( 3, REG_TP_STABILIZATION_TIME, time, TP_STABILIZATION_TIME_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetPrechargeTime

  Description:  set ADC precharge time before touch detection
  
  Arguments:    TpSetupTime_t time : precharge time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetPrechargeTime( TpSetupTime_t time )
{
    SDK_ASSERT( (TP_SETUP_TIME_0_1US <= time) || (time <= TP_SETUP_TIME_1MS) );
    CDC_SetSpiParamsEx( 3, REG_TP_PRECHARGE, (u8)(time << TP_PRECHARGE_SHIFT), TP_PRECHARGE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetSenseTime

  Description:  set ADC sense time before touch detection
  
  Arguments:    TpSetupTime_t time : sense time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetSenseTime( TpSetupTime_t time )
{
    SDK_ASSERT( (TP_SETUP_TIME_0_1US <= time) || (time <= TP_SETUP_TIME_1MS) );
    CDC_SetSpiParamsEx( 3, REG_TP_SENSE_TIME, time, TP_SENSE_TIME_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetDebounceTime

  Description:  set debounce time
  
  Arguments:    TpSetupTime_t time : sense time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetDebounceTime( tpDebounce_t time )
{
    SDK_ASSERT( (TP_DEBOUNCE_0US <= time) || (time <= TP_DEBOUNCE_1024US) );
    CDC_SetSpiParamsEx( 3, REG_TP_DEBOUNCE_TIME, time, TP_DEBOUNCE_TIME_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_IS_TOUCH

  Description:  タッチパネル接触判定
 
  Note: この関数はNewBufferMode有効時は正しい値を返しません。
　　　　Pen touch を3回確認しているのは、タイミングによって接触中でも
		非接触と判定される可能性があるためです。（のちに詳細説明追加）

  Arguments:    none

  Returns:      BOOL : if touched, return TRUE. otherwise FALSE.
 *---------------------------------------------------------------------------*/

static BOOL TWL_TP_IS_TOUCH( void )
{
	vu8 penup = 0;
	BOOL result = TRUE;

	CDC_Lock();
	SPI_Lock(123);

    CDCi_ChangePage( 3 );

	penup = CDCi_ReadSpiRegister( 9 );
	if ((penup & 0x80) == 0x00)
	{
		penup = CDCi_ReadSpiRegister( 9 );
		if ((penup & 0x80) == 0x00)
		{
			penup = CDCi_ReadSpiRegister( 9 );
			if ((penup & 0x80) == 0x00)
			{
				result = FALSE;
			}
		}
	}

	SPI_Unlock(123);
	CDC_Unlock();
	
	return result;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_ReadBuffer

  Description:  read Touch-Panel Buffer
  
  Arguments:    data : データ格納ポインタ

  Returns:      BOOL : if read success, return TRUE. otherwise FALSE.
 *---------------------------------------------------------------------------*/
#ifdef SDK_TP_AUTO_ADJUST_RANGE        // range値の自動調整スイッチ
BOOL TWL_TP_ReadBuffer( SPITpData *data, u8 range, u16* density )
#else
BOOL TWL_TP_ReadBuffer( SPITpData *data, u8 range )
#endif
{
#define TRY_COUNT       5
#define SAME_REQUIRED   ((TRY_COUNT>>1)+1)
#define SAME_CHANCE		(TRY_COUNT-SAME_REQUIRED+1)
#define ABS(x) ( ( (x) >= 0 ) ? (x) : ( -(x) ) )

    int i,j;
    u8  buf[32];
    u16 xBuf[TRY_COUNT];
    u16 yBuf[TRY_COUNT];

//  ペンアップ判定
	if (!TWL_TP_IS_TOUCH())
	{
        data->e.touch    = SPI_TP_TOUCH_OFF;
		data->e.validity = SPI_TP_VALIDITY_INVALID_XY;
        return TRUE;	// ここはペンアップとしてシステム領域に書き出す
	}

	for (i=0;i<TRY_COUNT;i++)
	{
		// XYサンプリング
		CDC_ReadSpiRegistersEx( 3, 42, &buf[i*4], 4 );

		//  サンプリング後のペンアップ判定 (サンプリング時間も稼げる）
		if (!TWL_TP_IS_TOUCH())
		{
		    return FALSE;	// システム領域には書き出さない
		}
	}

	// 処理し易いようにXY分離
    for (i=0; i<TRY_COUNT; i++)
    {
        xBuf[i]  = (u16)((buf[i*4]     << 8) | buf[i*4 + 1]);
        yBuf[i]  = (u16)((buf[i*4 + 2] << 8) | buf[i*4 + 3]);
    } 

#ifdef SDK_TP_AUTO_ADJUST_RANGE        // range値の自動調整スイッチ
    {
        s32     maxRange = 0;
        for (i = 0; i < TRY_COUNT - 1; i++)
        {
            for (j = i + 1; j < TRY_COUNT; j++)
            {
                s32 xRange = ABS(xBuf[i] - xBuf[j]);
                s32 yRange = ABS(yBuf[i] - yBuf[j]);
				s32 range = (xRange > yRange) ? xRange : yRange;

                if ( range > maxRange)
                {
                    maxRange = range;
                }
            }
        }
        *density = (u16)(maxRange);
    }
#endif

	{
	    int i, j;
	    int xSum = 0;
		int ySum = 0;
		int same_count = 0;

	    // サンプリングした内の半数以上がrange以内であればvalidなデータとする。
		for (i=0; i<SAME_CHANCE; i++)
		{
			same_count = 0;
			xSum = xBuf[i];
			ySum = yBuf[i];

			for (j=0; j<TRY_COUNT; j++)
			{
				if (i==j) { continue; }
				if ((ABS( xBuf[i] - xBuf[j] ) < range) && 
					(ABS( yBuf[i] - yBuf[j] ) < range))
				{
					same_count++;
					xSum += xBuf[j];
					ySum += yBuf[j];
				}
			}

			if (same_count >= SAME_REQUIRED) { break; }
		}

		data->e.x        = xSum / (same_count+1);
		data->e.y        = ySum / (same_count+1);
	    data->e.touch    = SPI_TP_TOUCH_ON;
		data->e.validity = (same_count >= SAME_REQUIRED) ? SPI_TP_VALIDITY_VALID : SPI_TP_VALIDITY_INVALID_XY;
	}

    return TRUE;	// システム領域に書き出す
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
