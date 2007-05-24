/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CDC - include
  File:     cdc_filter_coefficient.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CDC_CDC_FILTER_COEFFICIENT_H_
#define TWL_CDC_CDC_FILTER_COEFFICIENT_H_

#include <twl/misc.h>
#include <twl/types.h>


#ifdef __cplusplus
extern "C" {
#endif


//================================================================================
//        Typical Filter Coefficients (1st order)
//================================================================================
// ADC Filters for Microphone
#define     CDC_FILTER_1ST_IIR_ADC      0x00000001   // page4  reg8  -- 13
#define     CDC_FILTER_BIQUAD_ADC_A     0x00000002   //        reg14 -- 23
#define     CDC_FILTER_BIQUAD_ADC_B     0x00000004   //        reg24 -- 33
#define     CDC_FILTER_BIQUAD_ADC_C     0x00000008   //        reg34 -- 43
#define     CDC_FILTER_BIQUAD_ADC_D     0x00000010   //        reg44 -- 53
#define     CDC_FILTER_BIQUAD_ADC_E     0x00000020   //        reg54 -- 63

// DAC Filters for SP#HP (left)
#define     CDC_FILTER_1ST_IIR_LDAC     0x00000100   // page9  reg2  -- 7
#define     CDC_FILTER_BIQUAD_LDAC_B    0x00000200   // page8  reg12 -- 21
#define     CDC_FILTER_BIQUAD_LDAC_C    0x00000400   //        reg22 -- 31
#define     CDC_FILTER_BIQUAD_LDAC_D    0x00000800   //        reg32 -- 41
#define     CDC_FILTER_BIQUAD_LDAC_E    0x00001000   //        reg42 -- 51
#define     CDC_FILTER_BIQUAD_LDAC_F    0x00002000   //        reg52 -- 61

// DAC Filters for SP#HP (right)
#define     CDC_FILTER_1ST_IIR_RDAC     0x00010000   // page9  reg8  -- 13
#define     CDC_FILTER_BIQUAD_RDAC_B    0x00020000   // page8  reg76 -- 85
#define     CDC_FILTER_BIQUAD_RDAC_C    0x00040000   //        reg86 -- 95
#define     CDC_FILTER_BIQUAD_RDAC_D    0x00080000   //        reg96 -- 105
#define     CDC_FILTER_BIQUAD_RDAC_E    0x00100000   //        reg106-- 115
#define     CDC_FILTER_BIQUAD_RDAC_F    0x00200000   //        reg116-- 125

#define     CDC_FILTER_ADC              0x0000003F
#define     CDC_FILTER_LDAC             0x00003F00
#define     CDC_FILTER_RDAC             0x003F0000


//================================================================================
//        Typical Filter Coefficients (1st order)
//================================================================================
/*---------------------------------------------------------------------------*
  HPF, cut-off(-3dB) point = 0x00021*Fs = 9.26Hz (@Fs = 44.1kHz)
 *---------------------------------------------------------------------------*/
extern u8 cdc1stCoef_HPF_9_26Hz[6];




#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_CDC_CDC_FILTER_COEFFICIENT_H_ */
#endif
