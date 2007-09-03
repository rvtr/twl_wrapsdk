/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - libraries
  File:     snd_global.c

  Copyright 2004-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: snd_global.c,v $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <nitro/snd/common/global.h>

#include <twl/os.h>
#include <twl/misc.h>
#include <twl/snd/ARM7/i2s.h>
#include <nitro/hw/ARM7/ioreg_SND.h>
#include <nitro/spi/common/pm_common.h>
#include <nitro/snd/common/channel.h>
#include <nitro/snd/common/capture.h>

/******************************************************************************
    macro definition
 ******************************************************************************/

#define SOUND_BIAS_WAIT_COUNT   128
#define SOUND_BIAS_LEVEL      0x200
#define SOUND_BIAS_CYCLE_PER_LOOP 4

/******************************************************************************
    external function declaration
 ******************************************************************************/

extern void PMi_SetControl(u8 sw);
extern void PMi_ResetControl(u8 sw);

/******************************************************************************
    public functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         SND_Enable

  Description:  Enable sound master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SND_Enable(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    I2S_Enable();
    reg_SND_SOUNDCNT_8 |= REG_SND_SOUNDCNT_8_E_MASK;
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         SND_Disable

  Description:  Disable sound master control

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void SND_Disable(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    I2S_Disable();
    reg_SND_SOUNDCNT_8 &= ~REG_SND_SOUNDCNT_8_E_MASK;
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         SND_Shutdown

  Description:  shutdown sound system

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_Shutdown(void)
{
    int     ch;

    SND_Disable();

    for (ch = 0; ch < SND_CHANNEL_NUM; ch++)
    {
        SND_StopChannel(ch, SND_CHANNEL_STOP_HOLD);
    }
    SND_StopCapture(SND_CAPTURE_0);
    SND_StopCapture(SND_CAPTURE_1);
}

/*---------------------------------------------------------------------------*
  Name:         SND_BeginSleep

  Description:  Begin sleep

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_BeginSleep(void)
{
    // adding process for TWL
    I2S_BeginSleep();

    // stop all sound
    SND_Disable();

    // bias level down
    SVC_ResetSoundBias(SOUND_BIAS_WAIT_COUNT);
    OS_SpinWait(SOUND_BIAS_CYCLE_PER_LOOP * SOUND_BIAS_WAIT_COUNT * SOUND_BIAS_LEVEL);

    // sound power off
    PMi_ResetControl(PMIC_CTL_SND_PWR);

    // sound clock stop
    reg_SND_POWCNT &= ~REG_SND_POWCNT_SPE_MASK;
}

/*---------------------------------------------------------------------------*
  Name:         SND_EndSleep

  Description:  End sleep

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_EndSleep(void)
{
    // sound clock start
    reg_SND_POWCNT |= REG_SND_POWCNT_SPE_MASK;

    // sound power on
    PMi_SetControl(PMIC_CTL_SND_PWR);

    // bias level recover
    SVC_SetSoundBias(SOUND_BIAS_WAIT_COUNT * 2);

    // wait 15msec
    OS_SpinWait(OS_MilliSecondsToTicks(15) * 64);

    // sound enable
    SND_Enable();

    // adding process for TWL
    I2S_EndSleep();
}

/*---------------------------------------------------------------------------*
  Name:         SND_SetMasterVolume

  Description:  Set master volume

  Arguments:    volume : master volume

  Returns:      None
 *---------------------------------------------------------------------------*/
void SND_SetMasterVolume(int volume)
{
    SDK_MINMAX_ASSERT(volume, 0, SND_MASTER_VOLUME_MAX);

    reg_SND_SOUNDCNT_VOL = (u8)volume;
}

/*---------------------------------------------------------------------------*
  Name:         SND_SetOutputSelector

  Description:  Set output selector

  Arguments:    left     : L-OUT selector
                right    : R-OUT selector
                channel1 : channel1 output setting
                channel3 : channel3 output setting

  Returns:      None
 *---------------------------------------------------------------------------*/
void SND_SetOutputSelector(SNDOutput left,
                           SNDOutput right, SNDChannelOut channel1, SNDChannelOut channel3)
{
    BOOL    enable = (reg_SND_SOUNDCNT_8 & REG_SND_SOUNDCNT_8_E_MASK) ? TRUE : FALSE;

    reg_SND_SOUNDCNT_8 = REG_SND_SOUNDCNT_8_FIELD(enable, channel3, channel1, right, left);
}

/*====== End of snd_global.c ======*/
