/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - CDC_
  File:     CDC__api.c

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/cdc.h>

//#define MEASUREMENT_BY_TICK
#ifdef  MEASUREMENT_BY_TICK
#include <twl/vlink.h>

u64 tick_cdcInit_head           = 0;
u64 tick_cdcPowerUpPLL_head     = 0;
u64 tick_cdcInitSound_head      = 0;
u64 tick_cdcPowerUpDAC_head     = 0;
u64 tick_cdcSetupDAC_head       = 0;
u64 tick_cdcEnableHeadphoneDriver_head  = 0;
u64 tick_cdcEnableSpeakerDriver_head    = 0;
u64 tick_cdcUnmuteDAC_head              = 0;
u64 tick_cdcInit_tail           = 0;
#endif

BOOL isADCOn = FALSE;
BOOL isDACOn = FALSE;
#define CDC_PLL_STABLE_WAIT_TIME    18

void CDCi_PowerUpPLL( void );
void CDCi_PowerDownPLL( void );

//================================================================================
//        INIT APIs
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_Init

  Description:  initialize codec

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_Init( void )
{
#ifdef MEASUREMENT_BY_TICK
   OS_InitTimer();
   OS_InitTick();
//    tick_cdcInit_head =OS_GetTick();
#endif

    reg_CFG_CLK |= REG_CFG_CLK_SND_MASK;

    CDC_Reset();

    cdcRevisionID = CDC_GetRevisionId();

#ifdef MEASUREMENT_BY_TICK
    tick_cdcPowerUpPLL_head =OS_GetTick();
#endif
    CDCi_PowerUpPLL();

#ifdef MEASUREMENT_BY_TICK
    tick_cdcInitSound_head =OS_GetTick();
#endif
    CDC_InitSound();
#ifdef MEASUREMENT_BY_TICK
    tick_cdcInit_tail =OS_GetTick();

    OS_TPrintf("cdcInit                  = %llu, %6d\n", tick_cdcInit_head,
                                                                OS_TICK_TO_USEC(tick_cdcInit_head));
    OS_TPrintf("cdcPowerUpPLL            = %llu, %6d\n", tick_cdcPowerUpPLL_head,
                                                                OS_TICK_TO_USEC(tick_cdcPowerUpPLL_head));
    OS_TPrintf("cdcInitSound             = %llu, %6d\n", tick_cdcInitSound_head,
                                                                OS_TICK_TO_USEC(tick_cdcInitSound_head));
    OS_TPrintf("cdcPowerUpDAC            = %llu, %6d\n", tick_cdcPowerUpDAC_head,
                                                                OS_TICK_TO_USEC(tick_cdcPowerUpDAC_head));
    OS_TPrintf("cdcSetupDAC              = %llu, %6d\n", tick_cdcSetupDAC_head,
                                                                OS_TICK_TO_USEC(tick_cdcSetupDAC_head));
    OS_TPrintf("cdcEnableHeadphoneDriver = %llu, %6d\n", tick_cdcEnableHeadphoneDriver_head,
                                                                OS_TICK_TO_USEC(tick_cdcEnableHeadphoneDriver_head));
    OS_TPrintf("cdcEnableSpeakerDriver   = %llu, %6d\n", tick_cdcEnableSpeakerDriver_head,
                                                                OS_TICK_TO_USEC(tick_cdcEnableSpeakerDriver_head));
    OS_TPrintf("cdcUnmuteDAC             = %llu, %6d\n", tick_cdcUnmuteDAC_head,
                                                                OS_TICK_TO_USEC(tick_cdcUnmuteDAC_head));
    OS_TPrintf("cdcInit             tail = %llu, %6d\n", tick_cdcInit_tail,
                                                                OS_TICK_TO_USEC(tick_cdcInit_tail));
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CDC_Reset

  Description:  codec SW reset

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_Reset( void )
{
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_RST_ADDR, CDC0_RST_E );
    CDC_SetInputPinControl( TRUE, TRUE, TRUE );  // enable VCNT5,SP#HP,PMOFF pin
    OS_Sleep(1);
}

/*---------------------------------------------------------------------------*
  Name:         CDC_InitSound

  Description:  initialize output sound(speaker/headphone) logic

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_InitSound( void )
{
#if 1 // このコードは本来、cdcInitSound呼び出しルーチンが記述すべきコード。
    // Enable I2S
    reg_SND_I2SCNT |= REG_SND_I2SCNT_E_MASK;
#endif

    // setup High Pass Filter of 9.26Hz cutoff freq.
    CDC_Init1stOrderFilter( cdc1stCoef_HPF_9_26Hz,
                           CDC_FILTER_1ST_IIR_LDAC | CDC_FILTER_1ST_IIR_RDAC );

    // default, DACs are muted.
    // CDC_MuteDAC();

    // Setup DAC, Speaker Driver, Headphone Driver
#ifdef MEASUREMENT_BY_TICK
    tick_cdcPowerUpDAC_head =OS_GetTick();
#endif
    CDC_PowerUpDAC();

#ifdef MEASUREMENT_BY_TICK
    tick_cdcSetupDAC_head =OS_GetTick();
#endif
    CDC_SetupDAC( CDC_HP_DRV_PWON_TM_DEFAULT,
                 CDC_HP_DRV_RAMPUP_TM_DEFAULT,
                 CDC_HPSP_DRV_RAMPDWN_TM_DEFAULT );

#ifdef MEASUREMENT_BY_TICK
    tick_cdcEnableHeadphoneDriver_head =OS_GetTick();
#endif
    CDC_EnableHeadphoneDriver();   // enable headphone driver

#ifdef MEASUREMENT_BY_TICK
    tick_cdcEnableSpeakerDriver_head =OS_GetTick();
#endif
    CDC_EnableSpeakerDriver();     // enable speaker   driver

#ifdef MEASUREMENT_BY_TICK
    tick_cdcUnmuteDAC_head =OS_GetTick();
#endif
    CDC_UnmuteDAC();
}

/*---------------------------------------------------------------------------*
  Name:         CDC_InitMic

  Description:  initialize microphone logic

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_InitMic( void )
{
    // setup Mic Bias
    CDC_ChangePage( 1 );
    CDC_WriteI2cRegister( REG_CDC1_MIC_BIAS_ADDR, CDC1_MIC_BIAS_2_5V );

#if 1 // このコードは本来、cdcInitSound呼び出しルーチンが記述すべきコード。
    // Enable I2S
    reg_SND_I2SCNT |= REG_SND_I2SCNT_E_MASK;
#endif

    // setup High Pass Filter of 9.26Hz cutoff freq.
    CDC_Init1stOrderFilter( cdc1stCoef_HPF_9_26Hz, CDC_FILTER_1ST_IIR_ADC );

    // Setup ADC
    CDC_PowerUpADC();
    CDC_UnmuteADC();

    CDC_EnableAGC( CDC0_AGC_CTL1_DEFAULT_GAIN );
}


//================================================================================
//        Query APIs
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_IsTwlMode

  Description:  check CTR-mode (True) or DS-mode (False)

  Arguments:    None

  Returns:      TRUE : CTR-mode, FALSE : DS-mode
 *---------------------------------------------------------------------------*/
static inline BOOL CDC_IsTwlMode( void )
{
    return cdcIsTwlMode;
}

/*---------------------------------------------------------------------------*
  Name:         CDC_GetVendorId

  Description:  get Vendor ID

  Arguments:    None

  Returns:      u8 Vendor ID
 *---------------------------------------------------------------------------*/
u8 CDC_GetVendorId( void )
{
    CDC_ChangePage( 0 );
    return CDC_ReadI2cRegister( REG_CDC0_VEND_ID_ADDR );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_GetRevisionId

  Description:  get Revision ID

  Arguments:    None

  Returns:      u8 Revision ID (3-bit value)
 *---------------------------------------------------------------------------*/
u8 CDC_GetRevisionId( void )
{
    CDC_ChangePage( 0 );
    return (u8)(( CDC_ReadI2cRegister( REG_CDC0_REV_ID_ADDR ) & CDC0_REV_ID_MASK ) >> CDC0_REV_ID_SHIFT);
}
#if 0
//================================================================================
//        State Transition APIs
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CDC_GoDsMode

  Description:  transit from CTR-mode to DS-mode
                (never come back to CTR-mode without HW Reset)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_GoDsMode( void )
{
    CDC_ChangePage( 0 );

//#ifdef CDC_REVISION_A

    // CODEC-IC bug workaround
    CDC_WriteI2cRegister( REG_CDC0_ADC_PWR_STEP_ADDR, CDC0_ADC_PWR_STEP_PWRUP );
    CDC_WriteI2cRegister( REG_CDC0_ADC_MUTE_ADDR, CDC0_ADC_MUTE_D );

//#endif // CDC_REVISION_A

    // マイクバイアスを設定しておく必要がある。DSモードに入ってからは
    // この設定を行う手段がない。
    CDC_ChangePage( 1 );
    CDC_WriteI2cRegister( REG_CDC1_MIC_BIAS_ADDR, CDC1_MIC_BIAS_2_5V );

    // PLL 設定を DS 用に変更
    CDC_WriteI2cRegister( REG_CDC0_PLL_J_ADDR,    21 );
    CDC_WriteI2cRegister( REG_CDC0_NDAC_DIV_ADDR, CDC0_NDAC_DIV_PWR | 7 );
    CDC_WriteI2cRegister( REG_CDC0_NADC_DIV_ADDR, CDC0_NADC_DIV_PWR | 7 );

    CDC_ChangePage( 3 );

    // READREADY 端子属性を TSC2046-PENINTERRUPT に変更
    CDC_WriteI2cRegister( REG_CDC3_TP_CONV_MODE_ADDR, 0 );

    CDC_ChangePage( 255 );

    // enable DS-Mode             (via reg5 : current page=255)
    //
    // DS-mode default
    //   Master Sound Power OFF, MicBias OFF, MicPGA x40 times
    CDC_WriteI2cRegister( REG_CDC255_BKCMPT_MODE_ADDR, CDC255_BKCMPT_MODE_DS );

    //-------------------------------------------------------------------------
    // !! from now on, I2C cannot be used. Only DS-type PCSN,TCSN SPI can work.
    //-------------------------------------------------------------------------

    if (cdcRevisionID < CDC_REVISION_C)
    {
        // MicBias powered up
        // In Rev-A, MicBias must be powered up before enabling Master Sound Power
        dsmodeSetSpiFlags( REG_CDC255_DS_MIC_CTL_ADDR, CDC255_DS_MIC_CTL_BIAS_PWR );

        // enable Master Sound Power  (via reg0 : current page=255)
        //
        // note: In Rev-A, if Master Sound Power is off, touch-panel logic does
        //       not work.
        //
        //       CODEC PCSN is connected to IO-board Analog Key CS.
        //       CODEC PCSN is associated with TouchPanel now (for revision A).
        //
        dsmodeSetSpiFlags( REG_CDC255_AUD_CTL_ADDR, CDC255_AUD_CTL_PWR );
    }
    else
    {
        // MicBias powered up
        pmSetFlags( REG_CDC255_DS_MIC_CTL_ADDR, CDC255_DS_MIC_CTL_BIAS_PWR );
    }

    // change CODEC status variable
    CDCi_IsTwlMode = FALSE;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         CDC_SetInputPinControl

  Description:  set if input pin control enabled or not.
                Usually, PMOFF should not be disabled.

  Arguments:    BOOL enable_vcnt5 : set TRUE to enable VCNT5(LCD backlight) pin
                BOOL enable_sphp  : set TRUE to enable SP#HP switching pin
                BOOL enable_pmoff : set TRUE to enable PMOFF pin

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetInputPinControl( BOOL  enable_vcnt5, BOOL  enable_sphp, BOOL  enable_pmoff )
{
    u8 work = 0;

    CDC_ChangePage( 0 );

    if (enable_vcnt5)   work  = CDC0_PIN_CTL1_VCNT5_E;
    if (enable_sphp)    work |= CDC0_PIN_CTL1_SPHP_E;
    CDC_WriteI2cRegister( REG_CDC0_PIN_CTL1_ADDR, work );

    work = 0;
    if (enable_pmoff)   work  = CDC0_PIN_CTL2_PMOFF_E;
    CDC_WriteI2cRegister( REG_CDC0_PIN_CTL2_ADDR, work );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_GetInputPinControl

  Description:  get if input pin control enabled or not.

  Arguments:    BOOL *enable_vcnt5 : get TRUE if VCNT5(LCD backlight) pin is enabled
                BOOL *enable_sphp  : get TRUE if SP#HP switching pin is enabled
                BOOL *enable_pmoff : get TRUE if PMOFF pin is enabled

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_GetInputPinControl( BOOL *enable_vcnt5, BOOL *enable_sphp, BOOL *enable_pmoff )
{
    u8 work;

    *enable_vcnt5 = FALSE;
    *enable_sphp  = FALSE;
    *enable_pmoff = FALSE;

    CDC_ChangePage( 0 );

    work = CDC_ReadI2cRegister( REG_CDC0_PIN_CTL1_ADDR );
    if ((work & CDC0_PIN_CTL1_VCNT5_MASK) == CDC0_PIN_CTL1_VCNT5_E)
        *enable_vcnt5 = TRUE;
    if ((work & CDC0_PIN_CTL1_SPHP_MASK)  == CDC0_PIN_CTL1_SPHP_E)
        *enable_sphp = TRUE;

    work = CDC_ReadI2cRegister( REG_CDC0_PIN_CTL2_ADDR );
    if ((work & CDC0_PIN_CTL2_PMOFF_MASK) == CDC0_PIN_CTL2_PMOFF_E)
        *enable_pmoff = TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         CDCi_PowerUpPLL

  Description:  power up Internal PLL of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_PowerUpPLL( void )
{
    // IOP からの MCLK を check / enable

    // page 0, reg 5 で P=2,R=1,PLL on 設定
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_PLL_P_R_ADDR,
                            CDC0_PLL_P_R_PWR | (2 << CDC0_PLL_P_R_DIV_SHIFT) |
                            (1 << CDC0_PLL_P_R_MUL_SHIFT) );
// ADC / DAC のパワーアップ時が問題？
//    dly_tsk( 15 );
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_PowerDownPLL

  Description:  power down Internal PLL of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDCi_PowerDownPLL( void )
{
    // page 0, reg 5 で PLL off 設定
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_PLL_P_R_ADDR, 0 );
//    dly_tsk( 15 );

    // IOP からの MCLK を check / disable
}

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerUpDAC

  Description:  power up (both Left,Right channel of the) DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerUpDAC( void )
{
    // page 0, reg 63 で Left/Right DAC On, datapath is straght-forward setting.
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_DIG_PATH_ADDR,
                        CDC0_DIG_PATH_CH_PWR_L | (1 << CDC0_DIG_PATH_L_SHIFT) |
                        CDC0_DIG_PATH_CH_PWR_R | (1 << CDC0_DIG_PATH_R_SHIFT) );

    // PLL は ADC, DAC が起動したときに動き出すらしいので、ここに PLL 安定のためのウェイトが必要
    if ((!isADCOn) && (!isDACOn))
        OS_Sleep( CDC_PLL_STABLE_WAIT_TIME );

    isDACOn = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerDownDAC

  Description:  power down (both Left,Right channel of the) DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerDownDAC( void )
{
    // page 0, reg 63 で Left/Right DAC Off
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_DIG_PATH_ADDR, 0 );

    isDACOn = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CDC_SetupDAC

  Description:  setup DAC depop value of the CODEC

  Arguments:    int hp_pwon_tm    : Headphone Power-on time
                int hp_rmpup_tm   : Headphone Ramp-up step time
                int sphp_rmpdn_tm : Speaker/Headphonw Ramp-down step time

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetupDAC( int hp_pwon_tm, int hp_rmpup_tm, int sphp_rmpdn_tm )
{
    // page 1, reg 33--35
    CDC_ChangePage( 1 );
    CDC_WriteI2cRegister( REG_CDC1_HP_DRV_TM_ADDR,    (u8)(hp_pwon_tm | hp_rmpup_tm) );
    CDC_WriteI2cRegister( REG_CDC1_HPSP_RAMPDWN_ADDR, (u8)sphp_rmpdn_tm );
    CDC_WriteI2cRegister( REG_CDC1_DAC_OUTPUT_ADDR,   CDC1_DAC_OUTPUT_E_R | CDC1_DAC_OUTPUT_E_L );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_EnableHeadphoneDriver

  Description:  enable Headphone Driver (HP Driver On)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_EnableHeadphoneDriver( void )
{
    // page 1, reg 36--41
    CDC_ChangePage( 1 );

    // Mute Analog Volume
    CDC_WriteI2cRegister( REG_CDC1_HP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );
    CDC_WriteI2cRegister( REG_CDC1_HP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );

    // Power Up Headphone Driver, with short-circuit protection
    CDC_WriteI2cRegister( REG_CDC1_HP_DRV_ADDR, CDC1_HP_DRV_PWR_L | CDC1_HP_DRV_PWR_R |
                            CDC1_HP_CMN_MODE_VOL_1_65V | CDC1_HP_DRV_SHTC_PROTECT_E );

    // Un-mute Headphone
    CDC_WriteI2cRegister( REG_CDC1_HP_DRV_L_ADDR, CDC1_HP_DRV_PDN_TRISTATE | CDC1_HP_DRV_MUTEN );
    CDC_WriteI2cRegister( REG_CDC1_HP_DRV_R_ADDR, CDC1_HP_DRV_PDN_TRISTATE | CDC1_HP_DRV_MUTEN );

    // Un-mute Analog Volume
    CDC_WriteI2cRegister( REG_CDC1_HP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX );
    CDC_WriteI2cRegister( REG_CDC1_HP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DisableHeadphoneDriver

  Description:  disable Headphone Driver (HP Driver Off)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_DisableHeadphoneDriver( void )
{
    // page 1, reg 36--37,31
    CDC_ChangePage( 1 );

    // Mute Analog Volume
    CDC_WriteI2cRegister( REG_CDC1_HP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );
    CDC_WriteI2cRegister( REG_CDC1_HP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );

    // Power Down Headphone Driver, with short-circuit protection
    CDC_WriteI2cRegister( REG_CDC1_HP_DRV_ADDR, CDC1_HP_DRV_SHTC_PROTECT_E );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_EnableSpeakerDriver

  Description:  enable Speaker Driver (SP Driver On)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_EnableSpeakerDriver( void )
{
    // page 1, reg 38-39,32,42-43
    CDC_ChangePage( 1 );

    // Mute Analog Volume
    CDC_WriteI2cRegister( REG_CDC1_SP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );
    CDC_WriteI2cRegister( REG_CDC1_SP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );

    // Power Up Speaker Driver, with short-circuit protection
    CDC_WriteI2cRegister( REG_CDC1_SP_DRV_ADDR, CDC1_SP_DRV_PWR_L | CDC1_SP_DRV_PWR_R |
                            CDC1_SP_DRV_SHTC_PROTECT_E );

    // Un-mute Speaker
    CDC_WriteI2cRegister( REG_CDC1_SP_DRV_L_ADDR, CDC1_SP_DRV_MUTEN | CDC1_SP_DRV_GAIN_0DB );
    CDC_WriteI2cRegister( REG_CDC1_SP_DRV_R_ADDR, CDC1_SP_DRV_MUTEN | CDC1_SP_DRV_GAIN_0DB );

    // Un-mute Analog Volume
    CDC_WriteI2cRegister( REG_CDC1_SP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX );
    CDC_WriteI2cRegister( REG_CDC1_SP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DisableSpeakerDriver

  Description:  disable Speaker Driver (SP Driver Off)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_DisableSpeakerDriver( void )
{
    // page 1, reg 38-39,32
    CDC_ChangePage( 1 );

    // Mute Analog Volume
    CDC_WriteI2cRegister( REG_CDC1_SP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );
    CDC_WriteI2cRegister( REG_CDC1_SP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );

    // Power Down Speaker Driver, with short-circuit protection
    CDC_WriteI2cRegister( REG_CDC1_SP_DRV_ADDR, CDC1_SP_DRV_SHTC_PROTECT_E );
}


/*---------------------------------------------------------------------------*
  Name:         CDC_UnmuteDAC

  Description:  Un-mute DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_UnmuteDAC( void )
{
    // page 0, reg 64 で Un-mute
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_DIG_VOL_M_ADDR, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_MuteDAC

  Description:  Mute DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_MuteDAC( void )
{
    // page 0, reg 64 で Mute
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_DIG_VOL_M_ADDR, CDC0_DIG_VOL_M_MUTE_L | CDC0_DIG_VOL_M_MUTE_R );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerUpADC

  Description:  power up ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerUpADC( void )
{
    // page 0, reg 81 で Power Up
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_ADC_PWR_STEP_ADDR, CDC0_ADC_PWR_STEP_PWRUP );

    // PLL は ADC, DAC が起動したときに動き出すらしいので、ここに PLL 安定のためのウェイトが必要
    if ((!isADCOn) && (!isDACOn))
        OS_Sleep( CDC_PLL_STABLE_WAIT_TIME );

    isADCOn = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerDownADC

  Description:  power down ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerDownADC( void )
{
    // page 0, reg 81 で Power Down
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_ADC_PWR_STEP_ADDR, CDC0_ADC_PWR_STEP_PWRDN );

    isADCOn = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         CDC_UnmuteADC

  Description:  Un-mute ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_UnmuteADC( void )
{
    // page 0, reg 82 で Un-mute
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_ADC_MUTE_ADDR, CDC0_ADC_MUTE_D );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_MuteADC

  Description:  Mute ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_MuteADC( void )
{
    // page 0, reg 82 で Mute
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_ADC_MUTE_ADDR, CDC0_ADC_MUTE_E );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_EnableAGC

  Description:  Enable AGC of the CODEC

  Arguments:    int target_gain : AGC Target Gain

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_EnableAGC( int target_gain )
{
    // page 0, reg 86 で Enable
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_AGC_CTL1_ADDR, (u8)(CDC0_AGC_CTL1_E | target_gain) );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DisableAGC

  Description:  Disable AGC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_DisableAGC( void )
{
    // page 0, reg 86 で Disable
    CDC_ChangePage( 0 );
    CDC_WriteI2cRegister( REG_CDC0_AGC_CTL1_ADDR, CDC0_AGC_CTL1_D );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_Init1stOrderFilter

  Description:  initialize 1st order filter coeffient

  Arguments:    u8 *coef          : 1st order coefficient (6 bytes)
                int filter_target : target filter to be setup

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_Init1stOrderFilter( u8 *coef, int filter_target )
{
    if (filter_target & CDC_FILTER_1ST_IIR_ADC)
    {
        CDC_ChangePage( 4 );
        CDC_WriteI2cRegisters( REG_CDC4_ADC_C4_MSB_ADDR, coef, 6 );
    }
    if (filter_target & CDC_FILTER_1ST_IIR_LDAC)
    {
        CDC_ChangePage( 9 );
        CDC_WriteI2cRegisters( REG_CDC9_DAC_C65_MSB_ADDR, coef, 6 );
    }
    if (filter_target & CDC_FILTER_1ST_IIR_RDAC)
    {
        CDC_ChangePage( 9 );
        CDC_WriteI2cRegisters( REG_CDC9_DAC_C68_MSB_ADDR, coef, 6 );
    }
}

