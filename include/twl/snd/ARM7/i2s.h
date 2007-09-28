/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - include
  File:     i2s.h

  Copyright 2004-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: i2s.h,v $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_SND_COMMON_I2S_H_
#define TWL_SND_COMMON_I2S_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
    public macro variables
 ******************************************************************************/
#define I2S_MIXING_NITRO_MAX    8
#define I2S_MIXING_NITRO_MIN    0
#define I2S_MIXING_DSP_MAX      0
#define I2S_MIXING_DSP_MIN      8

typedef enum _I2SSamplingRate
{
	I2S_SAMPLING_RATE_32730 = 0,
	I2S_SAMPLING_RATE_47610 = 1,
	I2S_SAMPLING_RATE_NUM   = 2
} I2SSamplingRate;

/******************************************************************************
    public function declaration
 ******************************************************************************/

#ifdef SDK_ARM7

void    I2S_Enable(void);
void    I2S_Disable(void);

void    I2S_Shutdown(void);

void    I2S_BeginSleep(void);
void    I2S_EndSleep(void);

void    I2S_Mute(BOOL isMute);
BOOL    I2S_IsMute(void);

void    I2S_SetMixingRatio(int nitroRatio);
int     I2S_GetMixingRatio(void);

void    I2S_SetSamplingRate( I2SSamplingRate rate );
I2SSamplingRate    I2S_GetSamplingRate( void );

#endif /* SDK_ARM7 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_SND_COMMON_I2S_H_ */
