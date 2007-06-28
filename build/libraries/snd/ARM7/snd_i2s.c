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

static void SNDi_I2SInit(void)
{
    static BOOL isInitialized = FALSE;
    if (isInitialized == FALSE)
    {
        isInitialized = TRUE;

        reg_SND_POWCNT |= REG_SND_POWCNT_SPE_MASK;

        if ((reg_CFG_CLK & REG_CFG_CLK_SND_MASK) == 0)
        {
            // initialize codec with enabling I2S
            CDC_Init();
        }

        reg_CFG_TWL_EX |= REG_CFG_TWL_EX_I2S_MASK;
        if (reg_CFG_TWL_EX & REG_CFG_TWL_EX_I2S_MASK)
        {
            isTwl = TRUE;
            // Set default values
            SND_I2SSetSamplingRatio(TRUE);
            SND_I2SSetMixingRatio(8);
            SND_I2SMute(FALSE);
        }
    }
}

/******************************************************************************
    public functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         SND_I2SEnable

  Description:  Enable sound master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SND_I2SEnable(void)
{
    SNDi_I2SInit();

    if (isTwl)
    {
        reg_SND_I2SCNT |= REG_SND_I2SCNT_E_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SND_I2SDisable

  Description:  Disable sound master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SND_I2SDisable(void)
{
    if (isTwl)
    {
        reg_SND_I2SCNT &= ~REG_SND_I2SCNT_E_MASK;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SND_I2SShutdown

  Description:  shutdown sound system

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_I2SShutdown(void)
{
}

/*---------------------------------------------------------------------------*
  Name:         SND_BeginSleep

  Description:  Begin sleep

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_I2SBeginSleep(void)
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
void SND_I2SEndSleep(void)
{
    if (isTwl)
    {
        reg_SND_I2SCNT = state;
    }
}

/*---------------------------------------------------------------------------*
  Name:         SND_I2SMute

  Description:  Set mute status

  Arguments:    isMute : TRUE if mute

  Returns:      None
 *---------------------------------------------------------------------------*/
void SND_I2SMute(BOOL isMute)
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
  Name:         SND_I2SIsMute

  Description:  Get mute status

  Arguments:    None

  Returns:      TRUE if mute
 *---------------------------------------------------------------------------*/
BOOL SND_I2SIsMute(void)
{
    return (BOOL)((reg_SND_I2SCNT & REG_SND_I2SCNT_MUTE_MASK) >> REG_SND_I2SCNT_MUTE_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         SND_I2SSetMixingRatio

  Description:  Set mixing ratio

  Arguments:    nitroRatio  : NITRO : DSP ratio.  (0-8)
                              if 8, nitro sound is all.
                              if 0, DSP sound is all.

  Returns:      None
 *---------------------------------------------------------------------------*/
void SND_I2SSetMixingRatio(int nitroRatio)
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
  Name:         SND_I2SGetMixingRatio

  Description:  Set mixing ratio

  Arguments:    None

  Returns:      NITRO : DSP ratio.  (0-8)
                              if 8, nitro sound is all.
                              if 0, DSP sound is all.
 *---------------------------------------------------------------------------*/
int SND_I2SGetMixingRatio(void)
{
    return (reg_SND_I2SCNT & REG_SND_I2SCNT_MIX_RATIO_MASK) >> REG_SND_I2SCNT_MIX_RATIO_SHIFT;
}

/*---------------------------------------------------------------------------*
  Name:         SND_I2SSetSamplingRatio

  Description:  Set I2S sampling ratio.
                It can be called while I2S is disabled.

  Arguments:    is48kHz : set 48 kHz if TRUE. set 32kHz if FALSE.

  Returns:      None
 *---------------------------------------------------------------------------*/
void SND_I2SSetSamplingRatio(BOOL is48kHz)
{
    if (isTwl)
    {
        if (is48kHz)
        {
            reg_SND_I2SCNT |= REG_SND_I2SCNT_CODEC_SMP_MASK;
        }
        else
        {
            reg_SND_I2SCNT &= ~REG_SND_I2SCNT_CODEC_SMP_MASK;
        }
        CDC_SetParamPLL(is48kHz);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SND_I2SIsSamplingRatio48kHz

  Description:  Get I2S sampling ratio.

  Arguments:    None

  Returns:      TRUE if 48 kHz. otherwise FALSE
 *---------------------------------------------------------------------------*/
BOOL SND_I2SIsSamplingRatio48kHz(void)
{
    return (BOOL)((reg_SND_I2SCNT & REG_SND_I2SCNT_CODEC_SMP_MASK) >> REG_SND_I2SCNT_CODEC_SMP_SHIFT);
}

/*====== End of snd_i2s.c ======*/
