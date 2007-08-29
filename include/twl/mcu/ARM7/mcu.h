/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - mcu
  File:     i2c.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MCU_ARM7_MCU_H_
#define TWL_MCU_ARM7_MCU_H_

#include <twl/types.h>
#include <twl/mcu/ARM7/i2c.h>
#include <twl/mcu/ARM7/mcu_reg.h>

typedef enum
{
    MCU_SYSTEMMODE_NITRO    = 0,
    MCU_SYSTEMMODE_TWL      = 1
}
MCUSystemMode;

typedef enum
{
    MCU_CAMERA_PATTERN_NONE     = 0,
    MCU_CAMERA_PATTERN_BLINK    = 1
}
MCUCameraPattern;

#define MCU_CAMLED_BLINK    1

#ifdef _cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         MCU_IsResetRequest

  Description:  get reset button state

  Arguments:    None

  Returns:      TRUE if pressed
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_IsResetRequest( void )
{
    u8 data;
    if ( MCU_ReadRegisters( MCU_REG_IRQ_ADDR, &data, 1 ) )
    {
        return (data & MCU_REG_IRQ_RESET_MASK) >> MCU_REG_IRQ_RESET_SHIFT;
    }
    return FALSE;   // error
}

/*---------------------------------------------------------------------------*
  Name:         MCU_CacselReset

  Description:  cancel reset state

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_CancelReset( void )
{
    return MCU_ClearFlags( MCU_REG_IRQ_ADDR, MCU_REG_IRQ_RESET_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GoDsMode

  Description:  set system mode to MCU_SYSTEMMODE_NITRO in MCU.

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_GoDsMode( void )
{
    return MCU_SetParams( MCU_REG_MODE_ADDR, ( MCU_SYSTEMMODE_NITRO << MCU_REG_MODE_SYSTEM_SHIFT ), MCU_REG_MODE_SYSTEM_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_IsWifi

  Description:  get wifi LED status

  Arguments:    None

  Returns:      TRUE is on
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_IsWifi( void )
{
    u8 data;
    if ( MCU_ReadRegisters( MCU_REG_WIFI_ADDR, &data, 1 ) )
    {
        return ( data & MCU_REG_WIFI_MASK ) >> MCU_REG_WIFI_SHIFT;
    }
    return FALSE;   // error
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetWifiLed

  Description:  set wifi LED status

  Arguments:    enabled     TRUE if enabled

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetWifi( BOOL enabled )
{
    return MCU_SetParams( MCU_REG_WIFI_ADDR, (u8)( enabled ? MCU_REG_WIFI_MASK : 0 ), MCU_REG_WIFI_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetCameraPattern

  Description:  get camera LED status.

  Arguments:    None

  Returns:      camera LED pattern
 *---------------------------------------------------------------------------*/
static inline MCUCameraPattern MCU_GetCameraPattern( void )
{
    u8 data;
    if ( MCU_ReadRegisters( MCU_REG_CAMERA_ADDR, &data, 1 ) )
    {
        return (MCUCameraPattern)( ( data & MCU_REG_CAMERA_PATTERN_MASK ) >> MCU_REG_CAMERA_PATTERN_SHIFT );
    }
    return MCU_CAMERA_PATTERN_NONE; // error
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetCameraPattern

  Description:  set camera LED status.

  Arguments:    pattern     one of camera LED pattern

  Returns:      TRUE if sucess
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetCameraPattern( MCUCameraPattern pattern )
{
    return MCU_SetParams( MCU_REG_CAMERA_ADDR, (u8)( pattern << MCU_REG_CAMERA_PATTERN_SHIFT ), MCU_REG_CAMERA_PATTERN_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetVolume

  Description:  get current volume.

  Arguments:    None

  Returns:      0xFF if error, otherwise volume (0-31)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetVolume( void )
{
    u8 data;
    if ( MCU_ReadRegisters( MCU_REG_VOLUME_ADDR, &data, 1 ) )
    {
        return (u8)( ( data & MCU_REG_VOLUME_MASK ) >> MCU_REG_VOLUME_SHIFT );
    }
    return 0xFF;    // error
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetVolume

  Description:  set current volume.

  Arguments:    volume      volume (0-31)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetVolume( u8 volume )
{
    if ( ( volume & ~31 ) == 0 )
    {
        return MCU_SetParams( MCU_REG_VOLUME_ADDR, (u8)( volume << MCU_REG_VOLUME_SHIFT ), MCU_REG_VOLUME_MASK );
    }
    return FALSE;   // invalid parameters
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetVersion

  Description:  get version.

  Arguments:    None

  Returns:      0xFF if error, otherwise version (0-3)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetVersion( void )
{
    u8 data;
    if ( MCU_ReadRegisters( MCU_REG_INFO_ADDR, &data, 1 ) )
    {
        return (u8)( ( data & MCU_REG_INFO_VERSION_MASK ) >> MCU_REG_INFO_VERSION_SHIFT );
    }
    return 0xFF;    // error
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetRevision

  Description:  get revision.

  Arguments:    None

  Returns:      0xFF if error, otherwise revision (0-3)
 *---------------------------------------------------------------------------*/
static inline u8 MCU_GetRevision( void )
{
    u8 data;
    if ( MCU_ReadRegisters( MCU_REG_INFO_ADDR, &data, 1 ) )
    {
        return (u8)( ( data & MCU_REG_INFO_REVISION_MASK ) >> MCU_REG_INFO_REVISION_SHIFT );
    }
    return 0xFF;    // error
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetFreeRegisters

  Description:  get free registers.

  Arguments:    offset          offset bytes (0-15)
                bufp            buffer to store data
                nums            number of bytes to read

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_GetFreeRegisters( u8 offset, u8 *bufp, u32 nums )
{
    if ( ( ( offset + nums - 1 ) & ~15 ) == 0 )
    {
        return MCU_ReadRegisters( (u8)( MCU_REG_TEMP_ADDR + offset ), bufp, nums );
    }
    return FALSE;   // invalid parameters
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetFreeRegisters

  Description:  set free registers.

  Arguments:    offset          offset bytes (0-15)
                bufp            buffer to store data
                nums            number of bytes to write (16-1)

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetFreeRegisters( u8 offset, const u8 *bufp, u32 nums )
{
    if ( ( ( offset + nums - 1 ) & ~15 ) == 0 )
    {
        return MCU_WriteRegisters( (u8)( MCU_REG_TEMP_ADDR + offset ), bufp, nums );
    }
    return FALSE;   // invalid parameters
}

#ifdef _cplusplus
} /* extern "C" */
#endif

/* TWL_MCU_ARM7_MCU_H_ */
#endif
