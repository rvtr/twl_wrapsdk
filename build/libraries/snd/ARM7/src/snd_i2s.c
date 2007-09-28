/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - libraries
  File:     snd_i2s.c

  Copyright 2004-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: snd_i2s.c,v $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl/snd/ARM7/i2s.h>

#include <twl/os.h>
#include <twl/misc.h>
#include <twl/cdc.h>
#include <nitro/hw/ARM7/ioreg_SND.h>

/******************************************************************************
    static typedef declaration
 ******************************************************************************/

/******************************************************************************
    static variables
 ******************************************************************************/

static u16  state;
static BOOL isTwl = FALSE;

/******************************************************************************
    static functions
 ******************************************************************************/

static void I2Si_Init(void)
{
    static BOOL isInitialized = FALSE;
    if (isInitialized == FALSE)
    {
        isInitialized = TRUE;

        reg_SND_POWCNT |= REG_SND_POWCNT_SPE_MASK;
        reg_CFG_TWL_EX |= REG_CFG_TWL_EX_I2S_MASK;
        if (reg_CFG_TWL_EX & REG_CFG_TWL_EX_I2S_MASK)
        {
            isTwl = TRUE;
            // Set default values
//          I2S_SetSamplingRatio(TRUE);		�����48k�ɂ��ꂽ�獢��
            I2S_SetMixingRatio(8);
            I2S_Mute(FALSE);
        }
    }
}

/******************************************************************************
    public functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         I2S_Enable

  Description:  Enable sound master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void I2S_Enable(void)
{
    I2Si_Init();

    if (isTwl)
    {
        reg_SND_I2SCNT |= REG_SND_I2SCNT_E_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         I2S_Disable

  Description:  Disable sound master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void I2S_Disable(void)
{
    if (isTwl)
    {
        reg_SND_I2SCNT &= ~REG_SND_I2SCNT_E_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         I2S_Shutdown

  Description:  shutdown sound system

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void I2S_Shutdown(void)
{
}

/*---------------------------------------------------------------------------*
  Name:         SND_BeginSleep

  Description:  Begin sleep

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void I2S_BeginSleep(void)
{
    if (isTwl)
    {
        state = reg_SND_I2SCNT;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SND_EndSleep

  Description:  End sleep

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void I2S_EndSleep(void)
{
    if (isTwl)
    {
        reg_SND_I2SCNT = state;
    }
}

/*---------------------------------------------------------------------------*
  Name:         I2S_Mute

  Description:  Set mute status

  Arguments:    isMute : TRUE if mute

  Returns:      None
 *---------------------------------------------------------------------------*/
void I2S_Mute(BOOL isMute)
{
    if (isTwl)
    {
        if (isMute)
        {
            reg_SND_I2SCNT |= REG_SND_I2SCNT_MUTE_MASK;
        }
        else
        {
            reg_SND_I2SCNT &= ~REG_SND_I2SCNT_MUTE_MASK;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         I2S_IsMute

  Description:  Get mute status

  Arguments:    None

  Returns:      TRUE if mute
 *---------------------------------------------------------------------------*/
BOOL I2S_IsMute(void)
{
    return (BOOL)((reg_SND_I2SCNT & REG_SND_I2SCNT_MUTE_MASK) >> REG_SND_I2SCNT_MUTE_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         I2S_SetMixingRatio

  Description:  Set mixing ratio

  Arguments:    nitroRatio  : NITRO : DSP ratio.  (0-8)
                              if 8, nitro sound is all.
                              if 0, DSP sound is all.

  Returns:      None
 *---------------------------------------------------------------------------*/
void I2S_SetMixingRatio(int nitroRatio)
{
    if (isTwl)
    {
        if (nitroRatio >= 0 && nitroRatio <= 8)
        {
            reg_SND_I2SCNT &= ~REG_SND_I2SCNT_MIX_RATIO_MASK;
            reg_SND_I2SCNT = (u8)((reg_SND_I2SCNT & ~REG_SND_I2SCNT_MIX_RATIO_MASK) | (nitroRatio << REG_SND_I2SCNT_MIX_RATIO_SHIFT));
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         I2S_GetMixingRatio

  Description:  Set mixing ratio

  Arguments:    None

  Returns:      NITRO : DSP ratio.  (0-8)
                              if 8, nitro sound is all.
                              if 0, DSP sound is all.
 *---------------------------------------------------------------------------*/
int I2S_GetMixingRatio(void)
{
    return (reg_SND_I2SCNT & REG_SND_I2SCNT_MIX_RATIO_MASK) >> REG_SND_I2SCNT_MIX_RATIO_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         I2S_SetSamplingRate

  Description:  Set I2S sampling ratio.
                It can be called while I2S is disabled.

  Note:         ���̊֐��ł�CODEC��PLL�̒l��ύX���܂����APLL�̕ύX��
				���S�̂���CODEC��~���ɍs���ׂ��Ƃ̘b���`�b�v���[�J�[
�@�@�@�@�@�@�@�@���畷���Ă��܂��B

				�T���v�����O���[�g�ύX�̋�̓I�ȃV�[�P���X�͈ȉ��̂Ƃ���B

				1.CPU��I2S�o�͂��~
				2.Codec��������̒�~�yCODEC�z
				3.CPU��MCLK(�Ⴆ��12MHz)��~
				4.PLL�̍Đݒ�yCODEC�z
				5.CPU��MCLK(�Ⴆ��8MHz)�o��
				6.Codec��������̍ĊJ�yCODEC�z
				7.CPU��I2S�o�͊J�n

				�T���v�����O���[�g��ύX����ۂɂ́A���Ȃ��Ƃ�
�@�@�@�@�@�@�@�@�T�E���h/�^�b�`�p�l��/�}�C�N���~������K�v������܂��B				
�@�@�@�@�@�@�@�@AD/DA��OFF�ɂ���K�v�����邩�ɂ��ă��[�J�[�Ɋm�F���܂��B

				���݂̎����ł͉����΍���u���Ă��܂���B

  Arguments:    rate : sampling rate

  Returns:      None
 *---------------------------------------------------------------------------*/
void I2S_SetSamplingRate(I2SSamplingRate rate)
{
    if (isTwl)
    {
		switch (rate)
		{
			case I2S_SAMPLING_RATE_32730:
	            reg_SND_I2SCNT &= ~REG_SND_I2SCNT_CODEC_SMP_MASK;
		        CDC_SetPLL( CDC_PLL_PARAMETER_FOR_SAMPLING_RATE_32730 );
				break;
			case I2S_SAMPLING_RATE_47610:
	            reg_SND_I2SCNT |= REG_SND_I2SCNT_CODEC_SMP_MASK;
		        CDC_SetPLL( CDC_PLL_PARAMETER_FOR_SAMPLING_RATE_47610 );
				break;
		}
    }
}

/*---------------------------------------------------------------------------*
  Name:         I2S_GetSamplingRate

  Description:  Get I2S sampling ratio.

  Arguments:    None

  Returns:      
 *---------------------------------------------------------------------------*/
I2SSamplingRate I2S_GetSamplingRate( void )
{
	if (reg_SND_I2SCNT & REG_SND_I2SCNT_CODEC_SMP_MASK)
	{
		return I2S_SAMPLING_RATE_47610;
	}
	else
	{
		return I2S_SAMPLING_RATE_32730;
	}
}

/*====== End of snd_i2s.c ======*/
