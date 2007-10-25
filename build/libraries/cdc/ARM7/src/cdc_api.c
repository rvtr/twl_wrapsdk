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


/*
 
�@CODEC�̃��W�X�^�A�N�Z�X�ɂ̓y�[�W�؂�ւ������݂��܂��B
�@Page0,1,3,4,8,9,252,255 �����݂��A�e�y�[�W�ɈقȂ�@�\��
�@���W�X�^���񋓂���Ă��܂��B

�@"���݂̃y�[�W"���A���荞�݃n���h����ʃX���b�h�ɂ���ď�����
�@�r���ŕύX����Ă��܂��ƍ��������ƂɂȂ�܂��B


�@�y���ƂȂ��z


�@ �X���b�h�`
�@�i�D��x��j
�����������������@�@�@ �@�@�X���b�hB
���y�[�W�O�ց@���@�@�@�@�@�i�D��x���j
���@�@�@�@�@�@��������������������������
���@�@�@�@�@�@�������@�@���y�[�W�P�ց@��
��Access	  �� �@�� �@���@�@�@�@�@�@��
��Page 0�@  �@�� �@�� �@��Access�@�@�@��
��Registers   �� �@���@ ��Page 1�@�@�@��
���������������� �@�� �@��Registers�@ ��
                 �@ ��������������������
                

�@�����ŁACodec�ւ̃A�N�Z�X�Ɋւ���Mutex�ɂ��r��������s���܂��B
�@�i�Z���Ԃł���Ί��荞�݋֎~�ł����������j

�@�yMutex�ɂ��r������z


�@ �X���b�h�`
�@�i�D��x��j
����������������
��Mutex Lock  ��
���@�@�@�@�@�@���@�@�@ �@�@�X���b�hB
���y�[�W�O�ց@���@�@�@�@�@�i�D��x���j
���@�@�@�@�@�@��������������������������
��Access�@�@�@������������Mutex Lock�@��
��Page 0�@�@�@���@�@�������@�@�@�@�@�@��
��Registers	  �� �@�� �@���y�[�W�P�ց@��
��      �@  �@�� �@���@ ���@�@�@�@�@�@��
��Mutex Unlock������    ��Access�@�@�@��
���������������� �@�@ �@��Page 1�@    ��
�@                 �@ �@��Registers�@ ��
                 �@ �@�@����������������


�@Codec�ւ̃A�N�Z�X�ɂ�SPI�ʐM��������I2C�ʐM���g�p
�@���܂����A�����̊֐��ɂ����Ă�Mutex�ɂ��r������
�@���s���Ă��܂��B���̂��߁A�X���b�h�`��CODEC�p��
�@Mutex���擾���A�X���b�h�a��SPI�p��Mutex���擾����
�@�ꍇ�ɁA������f�b�h���b�N��ԂƂȂ�ꍇ������܂��B

�@�f�b�h���b�N���������Z�I���[��1�̃X���b�h�ɂ�����
�@������Mutex���擾���Ȃ����Ƃł����ACODEC�Ɋւ��Ă͂��ꂪ
�@������߁AMutex���擾���鏇�Ԃ𓝈ꂷ�邱�ƂŃf�b�h
�@���b�N��������܂��B

�@�y���j�z

�@�@ CODEC�ւ̃A�N�Z�X�Ɋւ���Mutex�ɂ��r��������s���B
�@�@ I2C/SPI�ʐM���s���ꍇ�A�����̊֐������ł�Mutex���g�p
�@�@ ����Ă��邱�Ƃɒ��ӂ���B�f�b�h���b�N��������邽�߁A
�@�@ Mutex�̎擾����CODEC����AI2C/SPI����Ƃ���B

�@�A ���荞�݃n���h�����ł͌���CODEC�A�N�Z�X�͍s��Ȃ��B
�@�@�i���荞�݃n���h�����ł�Mutex�擾�҂��̍ۂɃX���b�h�؂�ւ�
�@�@�@���������Ȃ�����Mutex���i�v�Ɏ擾�ł��Ȃ��\��������j
�@�@ ���荞�݃n���h���ł̓X���b�h���쐬�������͋N�������A
     ���̃X���b�h�ɂ�����CODEC�A�N�Z�X���s���B

*/

#include <twl.h>
#include <twl/cdc.h>
#include <pm_pmic.h>

u8 isADCOn 		  = FALSE;
u8 isDACOn 		  = FALSE;
u8 cdcIsTwlMode   = TRUE;

#define CDC_PLL_STABLE_WAIT_TIME                   20 // ��20ms�̃E�F�C�g���K�v
#define CDC_SCAN_MODE_TIMER_CLOCK_DIVIDER_VALUE    24

static void CDCi_PowerUpPLL( void );
static void CDCi_PowerDownPLL( void );

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
	reg_CFG_CLK |= REG_CFG_CLK_SND_MASK;

	CDC_InitMutex();

    CDC_Reset();

	CDC_SetPLL( CDC_PLL_PARAMETER_FOR_SAMPLING_RATE_47610 );

    CDC_SetScanModeTimerClockDivider( CDC_SCAN_MODE_TIMER_CLOCK_DIVIDER_VALUE );

    CDCi_PowerUpPLL();

    CDC_InitSound();
}

/*---------------------------------------------------------------------------*
  Name:         CDC_Reset

  Description:  codec SW reset

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_Reset( void )
{
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_RST_ADDR, CDC0_RST_E );
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
#if 1
    // Enable I2S
    reg_SND_I2SCNT |= REG_SND_I2SCNT_E_MASK;
#endif

    // setup High Pass Filter of 9.26Hz cutoff freq.
    CDC_Init1stOrderFilter( cdc1stCoef_HPF_9_26Hz,
                           CDC_FILTER_1ST_IIR_LDAC | CDC_FILTER_1ST_IIR_RDAC );

    // default, DACs are muted.
    // CDC_MuteDAC();

    // Setup DAC, Speaker Driver, Headphone Driver
    CDC_PowerUpDAC();
/*
    CDC_SetupDAC( CDC_HP_DRV_PWON_TM_DEFAULT,
                 CDC_HP_DRV_RAMPUP_TM_DEFAULT,
                 CDC_HPSP_DRV_RAMPDWN_TM_DEFAULT );
*/

    CDC_SetupDAC( CDC_HP_DRV_PWON_TM_DEFAULT,
                 CDC_HP_DRV_RAMPUP_TM_DEFAULT,
                 CDC_HPSP_DRV_RAMPDWN_TM_DEFAULT );


    CDC_EnableHeadphoneDriver();   // enable headphone driver

    CDC_EnableSpeakerDriver();     // enable speaker   driver

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
    CDC_WriteSpiRegisterEx( 1, REG_CDC1_MIC_BIAS_ADDR, CDC1_MIC_BIAS_AVDD );

#if 1 // ���̃R�[�h�͖{���AcdcInitSound�Ăяo�����[�`�����L�q���ׂ��R�[�h�B
    // Enable I2S
    reg_SND_I2SCNT |= REG_SND_I2SCNT_E_MASK;
#endif

    // setup High Pass Filter of 9.26Hz cutoff freq.
    CDC_Init1stOrderFilter( cdc1stCoef_HPF_9_26Hz, CDC_FILTER_1ST_IIR_ADC );

    // Setup ADC
    CDC_PowerUpADC();
    CDC_UnmuteADC();

    // AGC(Automatic Gain Control)�Ŏ擾�l�𒲐�������
	// ���C�u�����̐��퓮����m�F���ɂ������ߎb��I��disable�ɂ��Ă��܂��B
	// �ŏI�I��AGC��PGA�̂ǂ�����f�t�H���g�ɂ��邩�͖���ł��B

	// �Q�C���ݒ�
//	CDC_EnableAGC( CDC0_AGC_CTL1_DEFAULT_GAIN );
	CDC_DisableAGC();
	CDC_SetPGAB( 40 );
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
BOOL CDC_IsTwlMode( void )
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
	return CDC_ReadSpiRegisterEx( 0, REG_CDC0_VEND_ID_ADDR );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_GetRevisionId

  Description:  get Revision ID

  Arguments:    None

  Returns:      u8 Revision ID (3-bit value)
 *---------------------------------------------------------------------------*/
u8 CDC_GetRevisionId( void )
{
    return (u8)(( CDC_ReadSpiRegisterEx( 0, REG_CDC0_REV_ID_ADDR ) & CDC0_REV_ID_MASK ) >> CDC0_REV_ID_SHIFT);
}

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

#ifdef CDC_REVISION_A

    // CODEC-IC bug workaround
	CDC_PowerUpADC();
    CDC_UnmuteADC();

#endif // CDC_REVISION_A

    // �f�W�^���{�����[���ݒ� +7dB�i�]����2.5�{���������j
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_DIG_VOL_L_ADDR, 14 );
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_DIG_VOL_R_ADDR, 14 );

    // �}�C�N�o�C�A�X��ݒ肵�Ă����K�v������B
	// DS���[�h�ɓ����Ă���͂��̐ݒ���s����i���Ȃ��B
    CDC_WriteSpiRegisterEx( 1, REG_CDC1_MIC_BIAS_ADDR, CDC1_MIC_BIAS_AVDD );

	// PGA�C���s�[�_���X �ݒ�����l�i18.8k �ݒ��DS�Ɠ����̃Q�C����������j
    CDC_WriteSpiRegisterEx( 1, REG_CDC1_MIC_PGA_P_ADDR,  1 << CDC1_MIC_PGA_P_I_SHIFT);
    CDC_WriteSpiRegisterEx( 1, REG_CDC1_MIC_PGA_M_ADDR,  1 << CDC1_MIC_PGA_M_I_SHIFT);	

    // PLL �ݒ�� DS �p�ɕύX
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_PLL_J_ADDR,    21 );
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_NDAC_DIV_ADDR, CDC0_NDAC_DIV_PWR | 7 );
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_NADC_DIV_ADDR, CDC0_NADC_DIV_PWR | 7 );

    // READREADY �[�q������ TSC2046-PENINTERRUPT �ɕύX
    CDC_WriteSpiRegisterEx( 3, REG_CDC3_TP_CONV_MODE_ADDR, 0 );

    // enable DS-Mode             (via reg5 : current page=255)
    //
    // DS-mode default
    //   Master Sound Power OFF, MicBias OFF, MicPGA x40 times
    CDC_WriteSpiRegisterEx( 255, REG_CDC255_BKCMPT_MODE_ADDR, CDC255_BKCMPT_MODE_DS );

    //-------------------------------------------------------------------------
    // !! from now on, I2C cannot be used. Only DS-type PCSN,TCSN SPI can work.
    //-------------------------------------------------------------------------

    // enable Master Sound Power  (via reg0 : current page=255)
    CDC_DsmodeSetSpiFlags( REG_CDC255_AUD_CTL_ADDR, CDC255_AUD_CTL_PWR );

    // MicBias powered up
    // In Rev-A, MicBias must be powered up before enabling Master Sound Power
    CDC_DsmodeSetSpiFlags( REG_CDC255_DS_MIC_CTL_ADDR, CDC255_DS_MIC_CTL_BIAS_PWR );

    // change CODEC status variable
    cdcIsTwlMode = FALSE;
}

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

    if (enable_vcnt5)   work  = CDC0_PIN_CTL1_VCNT5_E;
    if (enable_sphp)    work |= CDC0_PIN_CTL1_SPHP_E;
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_PIN_CTL1_ADDR, work );

    work = 0;
    if (enable_pmoff)   work  = CDC0_PIN_CTL2_PMOFF_E;
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_PIN_CTL2_ADDR, work );
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

    work = CDC_ReadSpiRegisterEx( 0, REG_CDC0_PIN_CTL1_ADDR );
    if ((work & CDC0_PIN_CTL1_VCNT5_MASK) == CDC0_PIN_CTL1_VCNT5_E)
        *enable_vcnt5 = TRUE;
    if ((work & CDC0_PIN_CTL1_SPHP_MASK)  == CDC0_PIN_CTL1_SPHP_E)
        *enable_sphp = TRUE;

    work = CDC_ReadSpiRegisterEx( 0, REG_CDC0_PIN_CTL2_ADDR );
    if ((work & CDC0_PIN_CTL2_PMOFF_MASK) == CDC0_PIN_CTL2_PMOFF_E)
        *enable_pmoff = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_PowerUpPLL

  Description:  power up Internal PLL of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void CDCi_PowerUpPLL( void )
{
    // page 0, reg 5 �� P=2,R=1,PLL on �ݒ�
    CDC_WriteSpiRegisterEx( 0,
							REG_CDC0_PLL_P_R_ADDR,
                            CDC0_PLL_P_R_PWR |
                            (2 << CDC0_PLL_P_R_DIV_SHIFT) |
                            (1 << CDC0_PLL_P_R_MUL_SHIFT) );
}

/*---------------------------------------------------------------------------*
  Name:         CDCi_PowerDownPLL

  Description:  power down Internal PLL of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void CDCi_PowerDownPLL( void )
{
    // page 0, reg 5 �� PLL off �ݒ�
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_PLL_P_R_ADDR, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_SetPLL

  Description:  setup PLL parameter of the CODEC

  Note:  		PLL�����S�ɕύX���邽�߂ɂ�AD/DA���~�����Ȃ���΂Ȃ�Ȃ��B
                

  Arguments:    is48kHz : set 48 kHz if TRUE. set 32kHz if FALSE.

  Returns:      None
 *---------------------------------------------------------------------------*/
void 
CDC_SetPLL( CDCPllParameter param )
{
	CDC_Lock();
	SPI_Lock(123);

    CDCi_ChangePage( 0 );

	switch ( param )
	{
		case CDC_PLL_PARAMETER_FOR_SAMPLING_RATE_32730:
	        CDCi_WriteSpiRegister( REG_CDC0_PLL_J_ADDR,    21 );
	        CDCi_WriteSpiRegister( REG_CDC0_NDAC_DIV_ADDR, CDC0_NDAC_DIV_PWR | 7 );
	        CDCi_WriteSpiRegister( REG_CDC0_NADC_DIV_ADDR, CDC0_NADC_DIV_PWR | 7 );
			break;

		case CDC_PLL_PARAMETER_FOR_SAMPLING_RATE_47610:
	        CDCi_WriteSpiRegister( REG_CDC0_PLL_J_ADDR,    15 );
	        CDCi_WriteSpiRegister( REG_CDC0_NDAC_DIV_ADDR, CDC0_NDAC_DIV_PWR | 5 );
	        CDCi_WriteSpiRegister( REG_CDC0_NADC_DIV_ADDR, CDC0_NADC_DIV_PWR | 5 );
			break;
	}

	SPI_Unlock(123);
	CDC_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerUpDAC

  Description:  power up (both Left,Right channel of the) DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerUpDAC( void )
{
    // page 0, reg 63 �� Left/Right DAC On, datapath is straght-forward setting.
    CDC_WriteSpiRegisterEx( 0,
						REG_CDC0_DIG_PATH_ADDR,
                        CDC0_DIG_PATH_CH_PWR_L | (1 << CDC0_DIG_PATH_L_SHIFT) |
                        CDC0_DIG_PATH_CH_PWR_R | (1 << CDC0_DIG_PATH_R_SHIFT) );

    // PLL �� ADC, DAC ���N�������Ƃ��ɓ����o���炵���̂ŁA������ PLL ����̂��߂̃E�F�C�g���K�v
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
    // page 0, reg 63 �� Left/Right DAC Off
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_DIG_PATH_ADDR, 0 );
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
	CDC_Lock();
	SPI_Lock(123);

    // page 1, reg 33--35
    CDCi_ChangePage( 1 );
    CDCi_WriteSpiRegister( REG_CDC1_HP_DRV_TM_ADDR,    (u8)(hp_pwon_tm | hp_rmpup_tm) );
    CDCi_WriteSpiRegister( REG_CDC1_HPSP_RAMPDWN_ADDR, (u8)sphp_rmpdn_tm );
    CDCi_WriteSpiRegister( REG_CDC1_DAC_OUTPUT_ADDR,   CDC1_DAC_OUTPUT_E_R | CDC1_DAC_OUTPUT_E_L );

	SPI_Unlock(123);
	CDC_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         CDC_EnableHeadphoneDriver

  Description:  enable Headphone Driver (HP Driver On)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_EnableHeadphoneDriver( void )
{
	CDC_Lock();
	SPI_Lock(123);

    // page 1, reg 36--41
    CDCi_ChangePage( 1 );

    // Mute Analog Volume
    CDCi_WriteSpiRegister( REG_CDC1_HP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );
    CDCi_WriteSpiRegister( REG_CDC1_HP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );

    // Power Up Headphone Driver, with short-circuit protection
    CDCi_WriteSpiRegister( REG_CDC1_HP_DRV_ADDR, CDC1_HP_DRV_PWR_L | CDC1_HP_DRV_PWR_R |
                            CDC1_HP_CMN_MODE_VOL_1_65V | CDC1_HP_DRV_SHTC_PROTECT_E );

    // Un-mute Headphone
    CDCi_WriteSpiRegister( REG_CDC1_HP_DRV_L_ADDR, CDC1_HP_DRV_PDN_TRISTATE | CDC1_HP_DRV_MUTEN );
    CDCi_WriteSpiRegister( REG_CDC1_HP_DRV_R_ADDR, CDC1_HP_DRV_PDN_TRISTATE | CDC1_HP_DRV_MUTEN );

    // Un-mute Analog Volume
    CDCi_WriteSpiRegister( REG_CDC1_HP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX );
    CDCi_WriteSpiRegister( REG_CDC1_HP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX );

	SPI_Unlock(123);
	CDC_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DisableHeadphoneDriver

  Description:  disable Headphone Driver (HP Driver Off)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_DisableHeadphoneDriver( void )
{
	CDC_Lock();
	SPI_Lock(123);

    // page 1, reg 36--37,31
    CDCi_ChangePage( 1 );

    // Mute Analog Volume
    CDCi_WriteSpiRegister( REG_CDC1_HP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );
    CDCi_WriteSpiRegister( REG_CDC1_HP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );

    // Power Down Headphone Driver, with short-circuit protection
    CDCi_WriteSpiRegister( REG_CDC1_HP_DRV_ADDR, CDC1_HP_DRV_SHTC_PROTECT_E );

	SPI_Unlock(123);
	CDC_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         CDC_EnableSpeakerDriver

  Description:  enable Speaker Driver (SP Driver On)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_EnableSpeakerDriver( void )
{
	CDC_Lock();
	SPI_Lock(123);

    // page 1, reg 38-39,32,42-43
    CDCi_ChangePage( 1 );

    // Mute Analog Volume
    CDCi_WriteSpiRegister( REG_CDC1_SP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );
    CDCi_WriteSpiRegister( REG_CDC1_SP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );

    // Power Up Speaker Driver, with short-circuit protection
    CDCi_WriteSpiRegister( REG_CDC1_SP_DRV_ADDR, CDC1_SP_DRV_PWR_L | CDC1_SP_DRV_PWR_R |
                            CDC1_SP_DRV_SHTC_PROTECT_E );

    // Un-mute Speaker
    CDCi_WriteSpiRegister( REG_CDC1_SP_DRV_L_ADDR, CDC1_SP_DRV_MUTEN | CDC1_SP_DRV_GAIN_0DB );
    CDCi_WriteSpiRegister( REG_CDC1_SP_DRV_R_ADDR, CDC1_SP_DRV_MUTEN | CDC1_SP_DRV_GAIN_0DB );

    // Un-mute Analog Volume
    CDCi_WriteSpiRegister( REG_CDC1_SP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX );
    CDCi_WriteSpiRegister( REG_CDC1_SP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MAX );

	SPI_Unlock(123);
	CDC_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DisableSpeakerDriver

  Description:  disable Speaker Driver (SP Driver Off)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_DisableSpeakerDriver( void )
{
	CDC_Lock();
	SPI_Lock(123);

    // page 1, reg 38-39,32
    CDCi_ChangePage( 1 );

    // Mute Analog Volume
    CDCi_WriteSpiRegister( REG_CDC1_SP_ANGVOL_L_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );
    CDCi_WriteSpiRegister( REG_CDC1_SP_ANGVOL_R_ADDR, CDC1_ANGVOL_E | CDC1_ANGVOL_GAIN_MUTE );

    // Power Down Speaker Driver, with short-circuit protection
    CDCi_WriteSpiRegister( REG_CDC1_SP_DRV_ADDR, CDC1_SP_DRV_SHTC_PROTECT_E );

	SPI_Unlock(123);
	CDC_Unlock();
}

/*---------------------------------------------------------------------------*
  Name:         CDC_UnmuteDAC

  Description:  Un-mute DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_UnmuteDAC( void )
{
    // page 0, reg 64 �� Un-mute
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_DIG_VOL_M_ADDR, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_MuteDAC

  Description:  Mute DAC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_MuteDAC( void )
{
    // page 0, reg 64 �� Mute
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_DIG_VOL_M_ADDR, CDC0_DIG_VOL_M_MUTE_L | CDC0_DIG_VOL_M_MUTE_R );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_PowerUpADC

  Description:  power up ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_PowerUpADC( void )
{
    // page 0, reg 81 �� Power Up
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_ADC_PWR_STEP_ADDR, CDC0_ADC_PWR_STEP_PWRUP );

    // PLL �� ADC, DAC ���N�������Ƃ��ɓ����o���炵���̂ŁA������ PLL ����̂��߂̃E�F�C�g���K�v
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
    // page 0, reg 81 �� Power Down
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_ADC_PWR_STEP_ADDR, CDC0_ADC_PWR_STEP_PWRDN );
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
    // page 0, reg 82 �� Un-mute
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_ADC_MUTE_ADDR, CDC0_ADC_MUTE_D );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_MuteADC

  Description:  Mute ADC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_MuteADC( void )
{
    // page 0, reg 82 �� Mute
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_ADC_MUTE_ADDR, CDC0_ADC_MUTE_E );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_EnableAGC

  Description:  Enable AGC of the CODEC

  Arguments:    int target_gain : AGC Target Gain

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_EnableAGC( int target_gain )
{
    // page 0, reg 86 �� Enable
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_AGC_CTL1_ADDR, (u8)(CDC0_AGC_CTL1_E | target_gain) );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_DisableAGC

  Description:  Disable AGC of the CODEC

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_DisableAGC( void )
{
    // page 0, reg 86 �� Disable
    CDC_WriteSpiRegisterEx( 0, REG_CDC0_AGC_CTL1_ADDR, CDC0_AGC_CTL1_D );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_SetPGAB

  Description:  Setup PGAB of the CODEC
                PGAB is enabled when AGC is disabled.

  Arguments:    int target_gain : 0 �` 119 (0dB �` 59.5dB)

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetPGAB( u8 target_gain )
{
	CDC_WriteSpiRegisterEx( 1, REG_CDC1_MIC_ADC_PGA_ADDR, target_gain );
}

/*---------------------------------------------------------------------------*
  Name:         CDC_GetPGAB

  Description:  Get PGA of the CODEC
  
  Arguments:    None

  Returns:      Gain
 *---------------------------------------------------------------------------*/
u8 CDC_GetPGAB( void )
{
	return CDC_ReadSpiRegisterEx( 1, REG_CDC1_MIC_ADC_PGA_ADDR );
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
        CDC_WriteSpiRegistersEx( 4, REG_CDC4_ADC_C4_MSB_ADDR, coef, 6 );
    }
    if (filter_target & CDC_FILTER_1ST_IIR_LDAC)
    {
        CDC_WriteSpiRegistersEx( 9, REG_CDC9_DAC_C65_MSB_ADDR, coef, 6 );
    }
    if (filter_target & CDC_FILTER_1ST_IIR_RDAC)
    {
        CDC_WriteSpiRegistersEx( 9, REG_CDC9_DAC_C68_MSB_ADDR, coef, 6 );
    }
}

/*---------------------------------------------------------------------------*
  Name:         CDC_SetScanModeTimerClockDivider

  Description:  �X�L�������[�h�̃N���b�N�f�B�o�C�_�[��ݒ肵�܂��B
                ARM7���狟�������MCLK�i12.19MHz�j�̓f�B�o�C�_�[�̒l��
�@�@�@�@�@�@�@�@����ĕ�������܂��B

�@�@�@�@�@�@�@�@���ʂƂ��āA�f�B�o�C�_�[�̒l�ɔ�Ⴕ��
�@�@�@�@�@�@�@�@�E�C���^�[�o���^�C�}�[
�@�@�@�@�@�@�@�@�E�f�o�E���X�^�C�}�[
�@�@�@�@�@�@�@�@�̎��Ԃ��X�P�[������܂��B

                ��{�I�ɂ� 24 �Œ�Ƃ��܂��B

                MCLK           = 12.19MHz
                divider        = 24
                interval time  = 16ms  2ms  4ms  6ms   8ms  10ms  12ms   14ms
                de-bounce time =  0us 16us 32us 64us 128us 256us 512us 1024us

  Arguments:    value : 

  Returns:      None
 *---------------------------------------------------------------------------*/
void CDC_SetScanModeTimerClockDivider( u8 value )
{
    SDK_ASSERT( value < 128);
    CDC_WriteSpiRegisterEx( 3, REG_CDC3_TP_DELAY_CLK_ADDR, value );
}

