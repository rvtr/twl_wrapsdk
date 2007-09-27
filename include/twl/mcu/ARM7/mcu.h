/*---------------------------------------------------------------------------*
  Project:  TwlSDK - mcu - include
  File:     control.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef TWL_MCU_CONTROL_H_
#define TWL_MCU_CONTROL_H_

#include <twl/types.h>
#include <twl/mcu/ARM7/i2c.h>
#include <twl/mcu/ARM7/mcu_reg.h>

#ifdef _cplusplus
extern "C" {
#endif

typedef enum
{
    MCU_SYSTEMMODE_NITRO    = 0,
    MCU_SYSTEMMODE_TWL      = 1
}
MCUSystemMode;

typedef enum
{
    MCU_CAMLED_PATTERN_NONE     = 0,
    MCU_CAMLED_PATTERN_BLINK    = 1
}
MCUCameraLedPattern;

typedef enum
{
    MCU_CARDLED_STATUS_AUTO     = 0,
    MCU_CARDLED_STATUS_OFF      = 1,
    MCU_CARDLED_STATUS_ON       = 2,
    MCU_CARDLED_STATUS_ERROR    = 3 /* MCU is treated as ON */
}
MCUCardLedStatus;

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
  Name:         MCU_GetWifiLedStatus

  Description:  get wifi LED status

  Arguments:    None

  Returns:      TRUE is on
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_GetWifiLedStatus( void )
{
    u8 data;
    if ( MCU_ReadRegisters( MCU_REG_WIFILED_ADDR, &data, 1 ) )
    {
        return ( data & MCU_REG_WIFILED_MASK ) >> MCU_REG_WIFILED_SHIFT;
    }
    return FALSE;   // error
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetWifiLedStatus

  Description:  set wifi LED status

  Arguments:    enabled     TRUE if enabled

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetWifiLedStatus( BOOL enabled )
{
    return MCU_SetParams( MCU_REG_WIFILED_ADDR, (u8)( enabled ? MCU_REG_WIFILED_MASK : 0 ), MCU_REG_WIFILED_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetCameraLedPattern

  Description:  get camera LED status.

  Arguments:    None

  Returns:      camera LED pattern
 *---------------------------------------------------------------------------*/
static inline MCUCameraLedPattern MCU_GetCameraLedPattern( void )
{
    u8 data;
    if ( MCU_ReadRegisters( MCU_REG_CAMLED_ADDR, &data, 1 ) )
    {
        return (MCUCameraLedPattern)( ( data & MCU_REG_CAMLED_PATTERN_MASK ) >> MCU_REG_CAMLED_PATTERN_SHIFT );
    }
    return MCU_CAMLED_PATTERN_NONE; // error
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetCameraLedPattern

  Description:  set camera LED status.

  Arguments:    pattern     one of camera LED pattern

  Returns:      TRUE if sucess
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetCameraLedPattern( MCUCameraLedPattern pattern )
{
    return MCU_SetParams( MCU_REG_CAMLED_ADDR, (u8)( pattern << MCU_REG_CAMLED_PATTERN_SHIFT ), MCU_REG_CAMLED_PATTERN_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         MCU_GetCardLedStatus

  Description:  get card LED status.
                NOTE: slot 1/2 does not correspond to MC_IF[A]/[B]

  Arguments:    slot    physical slot number (1 or 2)

  Returns:      card LED status
 *---------------------------------------------------------------------------*/
static inline MCUCardLedStatus MCU_GetCardLedStatus( int slot )
{
    u8 data;
    u8 addr = (u8)(slot == 1 ? MCU_REG_CARDLED_1_ADDR : MCU_REG_CARDLED_2_ADDR);
    SDK_ASSERT(slot == 1 || slot == 2);
    if ( MCU_ReadRegisters( addr, &data, 1 ) )
    {
        return (MCUCardLedStatus)( ( data & MCU_REG_CARDLED_MASK ) >> MCU_REG_CARDLED_SHIFT );
    }
    return MCU_CARDLED_STATUS_ERROR; // error
}

/*---------------------------------------------------------------------------*
  Name:         MCU_SetCardLedStatus

  Description:  set card LED status.
                NOTE: slot 1/2 does not correspond to MC_IF[A]/[B]

  Arguments:    slot    physical slot number (1 or 2)
                status  one of card LED status

  Returns:      TRUE if sucess
 *---------------------------------------------------------------------------*/
static inline BOOL MCU_SetCardLedStatus( int slot, MCUCardLedStatus status )
{
    u8 addr = (u8)(slot == 1 ? MCU_REG_CARDLED_1_ADDR : MCU_REG_CARDLED_2_ADDR);
    SDK_ASSERT(slot == 1 || slot == 2);
    return MCU_SetParams( addr, (u8)( status << MCU_REG_CARDLED_SHIFT ), MCU_REG_CARDLED_MASK );
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

#endif /* TWL_MCU_CONTROL_H_ */
