/*---------------------------------------------------------------------------*
  Project:  TwlSDK - mcu - include
  File:     control.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-03-06#$
  $Rev: 4551 $
  $Author: yutaka $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MCU_CONTROL_H_
#define TWL_MCU_CONTROL_H_

#include <twl/mcu/ARM7/mcu_reg.h>
#include <twl/mcu/ARM7/i2c.h>

/*
    MCUÇÕÅASDK_TS_VERSIONÇ…ÇÊÇÁÇ∏ 300 à»ç~Ç…ëŒâûÇµÇ‹Ç∑ÅB
    SDK_TS_VERSION <= 200 ÇÃèÍçáÅAà»â∫ÇÃAPIÇ…å¿ÇË 200 å›ä∑Ç∆Ç»ÇËÇ‹Ç∑ÅB
    Supported: GetVersion, GetRevision, GetFreeRegister, SetFreeRegister
*/
#if SDK_TS_VERSION <= 200
#define MCU_OLD_REG_TEMP_ADDR               0x18
#endif

/*
    ÇªÇÃëº
*/
#define MCU_VERSION_MIN         (2 << MCU_REG_VER_INFO_VERSION_SHIFT)

#define MCU_BL_BRIGHTNESS_MAX   4
#define MCU_VOLUME_MAX          31
#define MCU_FREE_REG_NUMBER_MAX (MCU_REG_TEMP_LAST_ADDR - MCU_REG_TEMP_ADDR)

/*
    alias
*/
#define MCU_GetHotBootFlag()            (MCU_GetFreeRegister(0) & OS_MCU_RESET_VALUE_BUF_HOTBT_MASK ? TRUE : FALSE)
#define MCU_SetHotBootFlag(hotboot)     MCU_SetFreeRegister(0, (u8)(hotboot ? OS_MCU_RESET_VALUE_BUF_HOTBT_MASK : 0))


#ifdef _cplusplus
extern "C" {
#endif

typedef enum
{
    MCU_SYSTEMMODE_NITRO        = (0 << MCU_REG_MODE_SYSTEM_SHIFT),
    MCU_SYSTEMMODE_TWL          = (1 << MCU_REG_MODE_SYSTEM_SHIFT)
}
MCUSystemMode;

typedef enum
{
    MCU_CAMERA_LED_OFF          = (0 << MCU_REG_CAMERA_LED_SHIFT),
    MCU_CAMERA_LED_ON           = (1 << MCU_REG_CAMERA_LED_SHIFT),
    MCU_CAMERA_LED_BLINK        = (2 << MCU_REG_CAMERA_LED_SHIFT),
    MCU_CAMERA_LED_RESERVED1    = (3 << MCU_REG_CAMERA_LED_SHIFT)
}
MCUCameraLed;

typedef enum
{
    MCU_GPIO_DIR_OUTPUT         = 0,
    MCU_GPIO_DIR_INPUT          = 1
}
MCUGpioDir;

#ifdef SDK_ARM7
/*---------------------------------------------------------------------------*
  Name:         MCU_GetVersion

  Description:  get MCU version.

  Arguments:    None

  Returns:      MCU version (0-15)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetVersion( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_VER_INFO_ADDR );
    if ( data < MCU_VERSION_MIN )
    {
#if SDK_TS_VERSION <= 200
        OS_TWarning("MCU version is too old to support this library.");
#else
        OS_TPanic("MCU version is too old to support this library.");
#endif
    }
    return (u8)( ( data & MCU_REG_VER_INFO_VERSION_MASK ) >> MCU_REG_VER_INFO_VERSION_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetRevision

  Description:  get MCU revision.

  Arguments:    None

  Returns:      MCU revision (0-15)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetRevision( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_VER_INFO_ADDR );
    if ( data < MCU_VERSION_MIN )
    {
#if SDK_TS_VERSION <= 200
        OS_TWarning("MCU version is too old to support this library.");
#else
        OS_TPanic("MCU version is too old to support this library.");
#endif
    }
    return (u8)( ( data & MCU_REG_VER_INFO_REVISION_MASK ) >> MCU_REG_VER_INFO_REVISION_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetPmicManufacturer

  Description:  get PMIC manufacturer.

  Arguments:    None

  Returns:      manufacturer ID (0-7)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetPmicManufacturer( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_PMIC_INFO_ADDR );
    return (u8)( ( data & MCU_REG_PMIC_INFO_MAKER_MASK ) >> MCU_REG_PMIC_INFO_MAKER_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetPmicVersion

  Description:  get PMIC version.

  Arguments:    None

  Returns:      MCU version (0-3)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetPmicVersion( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_PMIC_INFO_ADDR );
    return (u8)( ( data & MCU_REG_VER_INFO_REVISION_MASK ) >> MCU_REG_VER_INFO_REVISION_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetBatteryManufacturer

  Description:  get battery manufacturer.

  Arguments:    None

  Returns:      manufacturer ID (0-7)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetBatteryManufacturer( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_BATT_INFO_ADDR );
    return (u8)( ( data & MCU_REG_BATT_INFO_MAKER_MASK ) >> MCU_REG_BATT_INFO_MAKER_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_NotifyShutdown

  Description:  notify system is going to shutdown

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_NotifyShutdown( void )
{
    return MCU_WriteRegister( MCU_REG_COMMAND_ADDR, MCU_REG_COMMAND_PWOFF_MASK );
//  return MCU_SetFlags( MCU_REG_COMMAND_ADDR, MCU_REG_COMMAND_PWOFF_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_HardwareReset

  Description:  execute hardware reset routine

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_HardwareReset( void )
{
    return MCU_WriteRegister( MCU_REG_COMMAND_ADDR, MCU_REG_COMMAND_RESET_MASK );
//  return MCU_SetFlags( MCU_REG_COMMAND_ADDR, MCU_REG_COMMAND_RESET_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GoDsMode

  Description:  set system mode to MCU_SYSTEMMODE_NITRO in MCU.

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_GoDsMode( void )
{
    return MCU_WriteRegister( MCU_REG_MODE_ADDR, MCU_SYSTEMMODE_NITRO );
//    return MCU_SetParams( MCU_REG_MODE_ADDR, MCU_SYSTEMMODE_NITRO, MCU_REG_MODE_SYSTEM_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GoTwlMode

  Description:  set system mode to MCU_SYSTEMMODE_TWL in MCU.

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_GoTwlMode( void )
{
    return MCU_WriteRegister( MCU_REG_MODE_ADDR, MCU_SYSTEMMODE_TWL );
//    return MCU_SetParams( MCU_REG_MODE_ADDR, MCU_SYSTEMMODE_TWL, MCU_REG_MODE_SYSTEM_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_IsExternalDC

  Description:  check external DC ( AC adapter ) is plugged.
                NOTE: it's not meaning that power is supplied or not.

  Arguments:    None

  Returns:      TRUE if plugged.
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_IsExternalDC( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_POWER_INFO_ADDR );
    return (BOOL)( (data & MCU_REG_POWER_INFO_EXTDC_MASK) >> MCU_REG_POWER_INFO_EXTDC_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_IsExtesionMode

  Description:  check extension mode is available.

  Arguments:    None

  Returns:      TRUE if extension mode is available.
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_IsExtesionMode( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_POWER_MODE_ADDR );
    return (BOOL)( (data & MCU_REG_POWER_MODE_EXT_MASK) >> MCU_REG_POWER_MODE_EXT_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetExtesionMode

  Description:  enable/disable extension mode.

  Arguments:    enabled         enable/dislabe

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetExtesionMode( BOOL enabled )
{
    return MCU_WriteRegister( MCU_REG_POWER_MODE_ADDR, (u8)(enabled ? MCU_REG_POWER_MODE_EXT_MASK : 0) );
//    return MCU_SetParams( MCU_REG_POWER_MODE_ADDR, (u8)(enabled ? MCU_REG_POWER_MODE_EXT_MASK : 0), MCU_REG_POWER_MODE_EXT_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetBatteryLevel

  Description:  get battery level

  Arguments:    None

  Returns:      battery level

                     0(Empty)       -0 %    Red (blink)
                     1(Low)        0-5 %    Red (blink)
                     3            1-12 %    Red
                     7           10-25 %    Blue
                    11           20-50 %    Blue
                    15          40-100 %    Blue
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetBatteryLevel( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_POWER_INFO_ADDR );
    return (u8)( (data & MCU_REG_POWER_INFO_LEVEL_MASK) >> MCU_REG_POWER_INFO_LEVEL_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetWifiLedStatus

  Description:  get wifi LED status

  Arguments:    None

  Returns:      TRUE is on
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_GetWifiLedStatus( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_WIFI_ADDR );
    return (BOOL)( ( data & MCU_REG_WIFI_LED_MASK ) >> MCU_REG_WIFI_LED_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetWifiLedStatus

  Description:  set wifi LED status

  Arguments:    enabled     TRUE if enabled

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetWifiLedStatus( BOOL enabled )
{
    return MCU_SetParams( MCU_REG_WIFI_ADDR, (u8)(enabled ? MCU_REG_WIFI_LED_MASK : 0), MCU_REG_WIFI_LED_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetWifiStatus

  Description:  whether wifi is enabled

  Arguments:    None

  Returns:      TRUE means enabled, otherwise in hardware reset
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_GetWifiStatus( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_WIFI_ADDR );
    return (BOOL)( ( data & MCU_REG_WIFI_NRESET_MASK ) >> MCU_REG_WIFI_NRESET_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetWifiStatus

  Description:  set wifi to be enabled/disabled

  Arguments:    enabled     TRUE if set enabled

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetWifiStatus( BOOL enabled )
{
    return MCU_SetParams( MCU_REG_WIFI_ADDR, (u8)( enabled ? MCU_REG_WIFI_NRESET_MASK : 0), MCU_REG_WIFI_NRESET_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetCameraLedStatus

  Description:  get camera LED status

  Arguments:    None

  Returns:      one of MCUCameraLed
 *---------------------------------------------------------------------------*/
static inline MCUCameraLed MCU_GetCameraLedStatus( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_CAMERA_ADDR );
    return (MCUCameraLed)( ( data & MCU_REG_CAMERA_LED_MASK ) >> MCU_REG_CAMERA_LED_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetCameraLedStatus

  Description:  set camera LED status

  Arguments:    led         one of MCUCameraLed

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetCameraLedStatus( MCUCameraLed led )
{
    return MCU_WriteRegister( MCU_REG_CAMERA_ADDR, led );
//    return MCU_SetParams( MCU_REG_CAMERA_ADDR, led, MCU_REG_CAMERA_LED_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetGpioDirection

  Description:  get GPIO direction

  Arguments:    pin         pin number (only 0 is suppined)

  Returns:      one of MCUGpioDir
 *---------------------------------------------------------------------------*/
static inline MCUGpioDir MCU_GetGpioDirection( u8 pin )
{
    u8 data = MCU_ReadRegister( MCU_REG_GPIO_DIR_ADDR );
    SDK_ASSERT( pin == 0 );
    return (MCUGpioDir)( ( data >> pin ) & 1 );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetGpioDirection

  Description:  set GPIO direction

  Arguments:    pin         pin number (only 0 is suppined)
                dir         one of MCUGpioDir

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetGpioDirection( u8 pin, MCUGpioDir dir )
{
    SDK_ASSERT( pin == 0 );
    return MCU_SetParams( MCU_REG_GPIO_DIR_ADDR, (u8)( dir << pin ), (u8)( 1 << pin ) );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetGpioData

  Description:  get GPIO data

  Arguments:    pin         pin number (only 0 is suppined)

  Returns:      1 bit data
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_GetGpioData( u8 pin )
{
    u8 data = MCU_ReadRegister( MCU_REG_GPIO_DATA_ADDR );
    SDK_ASSERT( pin == 0 );
    return (BOOL)( ( data >> pin ) & 1 );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetGpioData

  Description:  set GPIO data

  Arguments:    pin         pin number (only 0 is suppined)
                data        0(FALSE) or 1(TRUE)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetGpioData( u8 pin, BOOL data )
{
    SDK_ASSERT( pin == 0 );
    return MCU_SetParams( MCU_REG_GPIO_DATA_ADDR, (u8)( (data & 1) << pin ), (u8)( 1 << pin ) );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetVolume

  Description:  get current volume.

  Arguments:    None

  Returns:      volume (0-31)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetVolume( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_VOLUME_ADDR );
    return (u8)( ( data & MCU_REG_VOLUME_VOLUME_MASK ) >> MCU_REG_VOLUME_VOLUME_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetVolume

  Description:  set current volume.

  Arguments:    volume      volume (0-31)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetVolume( u8 volume )
{
    if ( volume <= MCU_VOLUME_MAX )
    {
        return MCU_WriteRegister( MCU_REG_VOLUME_ADDR, (u8)( volume << MCU_REG_VOLUME_VOLUME_SHIFT ) );
//        return MCU_SetParams( MCU_REG_VOLUME_ADDR, (u8)( volume << MCU_REG_VOLUME_VOLUME_SHIFT ), MCU_REG_VOLUME_VOLUME_MASK );
    }
    return FALSE;   // invalid parameters
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetBackLightBrightness

  Description:  get current back light brightness.

  Arguments:    None

  Returns:      brightness (0-MCU_BL_BRIGHTNESS_MAX)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetBackLightBrightness( void )
{
    u8 data = MCU_ReadRegister( MCU_REG_BL_ADDR );
    return (u8)( ( data & MCU_REG_BL_BRIGHTNESS_MASK ) >> MCU_REG_BL_BRIGHTNESS_SHIFT );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetBackLightBrightness

  Description:  set current back light brightness.

  Arguments:    brightness  back light brightness (0-MCU_BL_BRIGHTNESS_MAX)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetBackLightBrightness( u8 brightness )
{
    if ( brightness <= MCU_BL_BRIGHTNESS_MAX )
    {
        return MCU_WriteRegister( MCU_REG_BL_ADDR, (u8)( brightness << MCU_REG_BL_BRIGHTNESS_SHIFT ) );
//        return MCU_SetParams( MCU_REG_BL_ADDR, (u8)( brightness << MCU_REG_BL_BRIGHTNESS_SHIFT ), MCU_REG_BL_BRIGHTNESS_MASK );
    }
    return FALSE;   // invalid parameter
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetFreeRegister

  Description:  get free register.

  Arguments:    number          register number (0-7)

  Returns:      data
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetFreeRegister( u8 number )
{
    if ( number <= MCU_FREE_REG_NUMBER_MAX )
    {
#if SDK_TS_VERSION <= 200
        u8 data = MCU_ReadRegister( (u8)( MCU_OLD_REG_TEMP_ADDR + number ) );
        if ( data == 0x5a )
        {
            return MCU_ReadRegister( (u8)( MCU_REG_TEMP_ADDR + number ) );
        }
        return data;
#else
        return MCU_ReadRegister( (u8)( MCU_REG_TEMP_ADDR + number ) );
#endif
    }
    return 0x00;   // invalid parameter
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetFreeRegister

  Description:  set free register.

  Arguments:    number          register number (0-7)
                data            buffer to store data

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetFreeRegister( u8 number, u8 value )
{
    if ( number <= MCU_FREE_REG_NUMBER_MAX )
    {
#if SDK_TS_VERSION <= 200
        BOOL result = MCU_WriteRegister( (u8)( MCU_OLD_REG_TEMP_ADDR + number ), value );
        return MCU_WriteRegister( (u8)( MCU_REG_TEMP_ADDR + number ), value ) & result;
#else
        return MCU_WriteRegister( (u8)( MCU_REG_TEMP_ADDR + number ), value );
#endif
    }
    return FALSE;   // invalid parameter
}

/*
    çHèÍÇÃê∂éYçHíˆÇ≈ÇÃÇ›égÇÌÇÍÇÈó\íËÇÃAPI (TODO: ARM9Ç©ÇÁåƒÇ—èoÇπÇÈïKóvÇ†ÇË)
*/
/*---------------------------------------------------------------------------*
  Name:         MCU_StartBatteryCalibration

  Description:  execute battery calibration ( un-official command )

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_StartBatteryCalibration( void )
{
    return MCU_WriteRegister( MCU_REG_BATT_CALIB_ADDR, MCU_REG_BATT_CALIB_MODE_MASK );
//  return MCU_SetFlags( MCU_REG_BATT_CALIB_ADDR, MCU_REG_BATT_CALIB_MODE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetBatteryCalibrationStatus

  Description:  whether battery calibration was done ( un-official command )

  Arguments:    None

  Returns:      1 if done, 2 or 3 if error
 *---------------------------------------------------------------------------*/
static inline u8 MCU_IsBatteryCalibrationDone( void )
{
    return MCU_ReadRegister( MCU_REG_CALIB_STATUS_ADDR );
}

/*
    Ç∑ÇÆÇ…çÌèúÇ≥ÇÍÇÈó\íËÇÃAPI
*/
/*---------------------------------------------------------------------------*
  Name:         MCU_SetPowerSwitchThreshould

  Description:  set reset/no sense threshould time ( temporary command )

  Arguments:    ticks    time parameter

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetPowerSwitchThreshould( u8 ticks )
{
    return MCU_WriteRegister( MCU_REG_RESET_TIME_ADDR, (u8)(ticks << MCU_REG_RESET_TIME_VALUE_SHIFT) );
//    return MCU_SetParams( MCU_REG_RESET_TIME_ADDR, (u8)(ticks << MCU_REG_RESET_TIME_VALUE_SHIFT), MCU_REG_RESET_TIME_VALUE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetPowerOffThreshould

  Description:  set reset/power off threshould time ( temporary command )

  Arguments:    ticks   time parameter

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetPowerOffThreshould( u8 ticks )
{
    return MCU_WriteRegister( MCU_REG_PWOFF_TIME_ADDR, (u8)(ticks << MCU_REG_PWOFF_TIME_VALUE_SHIFT) );
//    return MCU_SetParams( MCU_REG_PWOFF_TIME_ADDR, (u8)(ticks << MCU_REG_PWOFF_TIME_VALUE_SHIFT), MCU_REG_PWOFF_TIME_VALUE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_MilliSecondsToThreshouldTicks

  Description:  calculate threshould value from msec

  Arguments:    msec   msec value

  Returns:      threshould ticks
 *---------------------------------------------------------------------------*/
static inline u8 MCU_MilliSecondsToThreshouldTicks( u32 msec )
{
    SDK_ASSERT( msec > 20 );
    return (u8)((msec - 20) / 5);
}

#endif // SDK_ARM7

#ifdef _cplusplus
} /* extern "C" */
#endif

#endif /* TWL_MCU_CONTROL_H_ */
