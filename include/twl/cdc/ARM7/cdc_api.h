/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CDC - include
  File:     CDC__api.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CDC_CDC_API_H_
#define TWL_CDC_CDC_API_H_

#include <twl/cdc/ARM7/cdc.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum _CDCPllParameter
{
	CDC_PLL_PARAMETER_FOR_SAMPLING_RATE_32730,
	CDC_PLL_PARAMETER_FOR_SAMPLING_RATE_47610
} CDCPllParameter;

//================================================================================
//        INIT APIs
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_Init

  Description:  initialize codec

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_Init( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_Reset

  Description:  codec SW reset

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_Reset( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_InitSound

  Description:  initialize output sound(speaker/headphone) logic

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_InitSound( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_InitMic

  Description:  initialize microphone logic

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_InitMic( void );

//================================================================================
//        Query/Check APIs
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_IsTwlMode

  Description:  check TWL-mode (True) or DS-mode (False)

  Arguments:    None

  Returns:      TRUE : TWL-mode, FALSE : DS-mode
 *---------------------------------------------------------------------------*/
BOOL CDC_IsTwlMode( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_GetVendorId

  Description:  get Vendor ID

  Arguments:    None

  Returns:      u8 Vendor ID
 *---------------------------------------------------------------------------*/
u8 CDC_GetVendorId( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_GetRevisionId

  Description:  get Revision ID

  Arguments:    None

  Returns:      u8 Revision ID (3-bit value)
 *---------------------------------------------------------------------------*/
u8 CDC_GetRevisionId( void );


//================================================================================
//        State Transition/Check APIs
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_GoDsMode

  Description:  transit from TWL-mode to DS-mode
                (never come back to TWL-mode without HW Reset)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_GoDsMode( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_SetInputPinControl

  Description:  set if input pin control enabled or not.
                Usually, PMOFF should not be disabled.

  Arguments:    BOOL enable_vcnt5 : set TRUE to enable VCNT5(LCD backlight) pin
                BOOL enable_sphp  : set TRUE to enable SP#HP switching pin
                BOOL enable_pmoff : set TRUE to enable PMOFF pin

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetInputPinControl( BOOL  enable_vcnt5, BOOL  enable_sphp, BOOL  enable_pmoff );

/*---------------------------------------------------------------------------*
  Name:         CDC_GetInputPinControl

  Description:  get if input pin control enabled or not.

  Arguments:    BOOL *enable_vcnt5 : get TRUE if VCNT5(LCD backlight) pin is enabled
                BOOL *enable_sphp  : get TRUE if SP#HP switching pin is enabled
                BOOL *enable_pmoff : get TRUE if PMOFF pin is enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_GetInputPinControl( BOOL *enable_vcnt5, BOOL *enable_sphp, BOOL *enable_pmoff );

/*---------------------------------------------------------------------------*
  Name:         CDC_SetPLL

  Description:  setup PLL parameter of the CODEC

  Arguments:    param: parameter type

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetPLL( CDCPllParameter param );

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerUpDAC

  Description:  power up (both Left,Right channel of the) DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerUpDAC( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerDownDAC

  Description:  power down (both Left,Right channel of the) DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerDownDAC( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_SetupDAC

  Description:  setup DAC depop value of the CODEC

  Arguments:    int hp_pwon_tm    : Headphone Power-on time
                int hp_rmpup_tm   : Headphone Ramp-up step time
                int sphp_rmpdn_tm : Speaker/Headphonw Ramp-down step time

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetupDAC( int hp_pwon_tm, int hp_rmpup_tm, int sphp_rmpdn_tm );

/*---------------------------------------------------------------------------*
  Name:         CDC_EnableHeadphoneDriver

  Description:  enable Headphone Driver (HP Driver On)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_EnableHeadphoneDriver( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_DisableHeadphoneDriver

  Description:  disable Headphone Driver (HP Driver Off)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_DisableHeadphoneDriver( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_EnableSpeakerDriver

  Description:  enable Speaker Driver (SP Driver On)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_EnableSpeakerDriver( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_DisableSpeakerDriver

  Description:  disable Speaker Driver (SP Driver Off)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_DisableSpeakerDriver( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_UnmuteDAC

  Description:  Un-mute DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_UnmuteDAC( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_MuteDAC

  Description:  Mute DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_MuteDAC( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerUpADC

  Description:  power up ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerUpADC( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerDownADC

  Description:  power down ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerDownADC( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_UnmuteADC

  Description:  Un-mute ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_UnmuteADC( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_MuteADC

  Description:  Mute ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_MuteADC( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_EnableAGC

  Description:  Enable AGC of the CODEC

  Arguments:    int target_gain : AGC Target Gain

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_EnableAGC( int target_gain );

/*---------------------------------------------------------------------------*
  Name:         CDC_DisableAGC

  Description:  Disable AGC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_DisableAGC( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_SetPGAB

  Description:  Setup PGA of the CODEC
                PGA is enabled when AGC is disabled.

  Arguments:    int target_gain : 0 ～ 119 (0dB ～ 59.5dB)

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetPGAB( u8 target_gain );

/*---------------------------------------------------------------------------*
  Name:         CDC_GetPGAB

  Description:  Get PGA of the CODEC
  
  Arguments:    None

  Returns:      Gain
 *---------------------------------------------------------------------------*/
u8 CDC_GetPGAB( void );

/*---------------------------------------------------------------------------*
  Name:         CDC_Init1stOrderFilter

  Description:  initialize 1st order filter coeffient

  Arguments:    u8 *coef          : 1st order coefficient (6 bytes)
                int filter_target : target filter to be setup

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_Init1stOrderFilter( u8 *coef, int filter_target );

/*---------------------------------------------------------------------------*
  Name:         CDC_SetScanModeTimerClockDivider

  Description:  スキャンモードのクロックディバイダーを設定します。
				ARM7から供給されるMCLK（12.19MHz）はディバイダーの値に
                よって分周されます。

				結果として、ディバイダーの値に比例して
				・インターバルタイマー
				・デバウンスタイマー
				の時間がスケールされます。

				基本的には 24 固定とします。

				MCLK           = 12.19MHz
				divider        = 24
				interval time  = 16ms  2ms  4ms  6ms   8ms  10ms  12ms   14ms
				de-bounce time =  0us 16us 32us 64us 128us 256us 512us 1024us

  Arguments:    value : 

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetScanModeTimerClockDivider( u8 value );


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_CDC_CDC_API_H_ */
#endif
