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
#define SND_I2S_MIXING_NITRO_MAX    8
#define SND_I2S_MIXING_NITRO_MIN    0
#define SND_I2S_MIXING_DSP_MAX      0
#define SND_I2S_MIXING_DSP_MIN      8

/******************************************************************************
    public function declaration
 ******************************************************************************/

#ifdef SDK_ARM7

void    SND_I2SEnable(void);
void    SND_I2SDisable(void);

void    SND_I2SShutdown(void);

void    SND_I2SBeginSleep(void);
void    SND_I2SEndSleep(void);

void    SND_I2SMute(BOOL isMute);

void    SND_I2SSetMixingRatio(int nitroRatio);
void    SND_I2SSetSamplingRatio(BOOL is47kHz);

#endif /* SDK_ARM7 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_SND_COMMON_I2S_H_ */
