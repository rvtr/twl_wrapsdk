/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera
  File:     i2c_common.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CAMERA_I2C_COMMON_H_
#define TWL_CAMERA_I2C_COMMON_H_

#include <twl/types.h>
#include <twl/i2c/ARM7/i2c.h>
#include <twl/camera/common/types.h>
#include <twl/camera/ARM7/i2c_micron.h>
#include <twl/camera/ARM7/i2c_sharp.h>

#ifdef _cplusplus
extern "C" {
#endif

#if 0
//================================================================================
//        I2C_ ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERA_WriteRegister

  Description:  set value to decive register through I2C_.

  Arguments:    camera  : one of CameraSelect
                reg     : decive register
                data    : value to be written

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_WriteRegister( CameraSelect camera, u16 reg, u16 data )
{
    return cameraType ? CAMERAi_S_WriteRegister(camera, reg, data)
                      : CAMERAi_M_WriteRegister(camera, reg, data);
}
static inline BOOL CAMERA_WriteRegister( CameraSelect camera, u16 reg, u16 data )
{
    return cameraType ? CAMERA_S_WriteRegister(camera, reg, data)
                      : CAMERA_M_WriteRegister(camera, reg, data);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ReadRegister

  Description:  get value from decive register through I2C_.

  Arguments:    camera  : one of CameraSelect w/o BOTH
                reg      : decive register

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline u16 CAMERAi_ReadRegister( CameraSelect camera, u16 reg )
{
    return cameraType ? CAMERAi_S_ReadRegister(camera, reg)
                      : CAMERAi_M_ReadRegister(camera, reg);
}
static inline u16 CAMERA_ReadRegister( CameraSelect camera, u16 reg )
{
    return cameraType ? CAMERA_S_ReadRegister(camera, reg)
                      : CAMERA_M_ReadRegister(camera, reg);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_WriteRegisters

  Description:  set value to decive registers through I2C_.

  Arguments:    camera  : one of CameraSelect
                reg      : decive register
                bufp     : data array to be written
                size     : data size

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_WriteRegisters( CameraSelect camera, u16 reg, const u16 *bufp, size_t size )
{
    return cameraType ? CAMERAi_S_WriteRegisters(camera, reg, bufp, size)
                      : CAMERAi_M_WriteRegisters(camera, reg, bufp, size);
}
static inline BOOL CAMERA_WriteRegisters( CameraSelect camera, u16 reg, const u16 *bufp, size_t size )
{
    return cameraType ? CAMERA_S_WriteRegisters(camera, reg, bufp, size)
                      : CAMERA_M_WriteRegisters(camera, reg, bufp, size);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ReadRegisters

  Description:  get value from decive registers through I2C_.

  Arguments:    camera  : one of CameraSelect w/o BOTH
                reg      : decive register
                bufp     : data array to be read
                size     : data size

  Returns:      value which is read from specified decive register
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_ReadRegisters( CameraSelect camera, u16 reg, u16 *bufp, size_t size )
{
    return cameraType ? CAMERAi_S_ReadRegisters(camera, reg, bufp, size)
                      : CAMERAi_M_ReadRegisters(camera, reg, bufp, size);
}
static inline BOOL CAMERA_ReadRegisters( CameraSelect camera, u16 reg, u16 *bufp, size_t size )
{
    return cameraType ? CAMERA_S_ReadRegisters(camera, reg, bufp, size)
                      : CAMERA_M_ReadRegisters(camera, reg, bufp, size);
}

//================================================================================
//        I2C_ BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetParamsM

  Description:  set control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_SetParams( CameraSelect camera, u16 reg, u16 setBits, u16 maskBits )
{
    return cameraType ? CAMERAi_S_SetParams(camera, reg, setBits, maskBits)
                      : CAMERAi_M_SetParams(camera, reg, setBits, maskBits);
}
static inline BOOL CAMERA_SetParams( CameraSelect camera, u16 reg, u16 setBits, u16 maskBits )
{
    return cameraType ? CAMERA_S_SetParams(camera, reg, setBits, maskBits)
                      : CAMERA_M_SetParams(camera, reg, setBits, maskBits);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_SetFlagsM

  Description:  set control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_SetFlags( CameraSelect camera, u16 reg, u16 setBits )
{
    return cameraType ? CAMERAi_S_SetFlags(camera, reg, setBits)
                      : CAMERAi_M_SetFlags(camera, reg, setBits);
}
static inline BOOL CAMERA_SetFlags( CameraSelect camera, u16 reg, u16 setBits )
{
    return cameraType ? CAMERA_S_SetFlags(camera, reg, setBits)
                      : CAMERA_M_SetFlags(camera, reg, setBits);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_ClearFlagsM

  Description:  clear control bit to device register

  Arguments:    camera  : one of CameraSelect
                reg      : device register
                clrBits  : bits to clear

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline BOOL CAMERAi_ClearFlags( CameraSelect camera, u16 reg, u16 clrBits )
{
    return cameraType ? CAMERAi_S_ClearFlags(camera, reg, clrBits)
                      : CAMERAi_M_ClearFlags(camera, reg, clrBits);
}

static inline BOOL CAMERA_ClearFlags( CameraSelect camera, u16 reg, u16 clrBits )
{
    return cameraType ? CAMERA_S_ClearFlags(camera, reg, clrBits)
                      : CAMERA_M_ClearFlags(camera, reg, clrBits);
}
#endif

//================================================================================
//        I2C_ API
//================================================================================

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize CAMERA

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CInit(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CStandby

  Description:  standby or resume CAMERA

  Arguments:    camera  : one of CameraSelect
                standby : TRUE if goto standby mode

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CStandby(CameraSelect camera, BOOL standby);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResize

  Description:  resize CAMERA

  Arguments:    camera  : one of CameraSelect
                width   : width of output image
                height  : height of output image

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CResize(CameraSelect camera, u16 width, u16 height);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPreSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CPreSleep(CameraSelect camera);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CPostSleep

  Description:  preset CAMERA registers

  Arguments:    camera  : one of CameraSelect

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL CAMERA_I2CPostSleep(CameraSelect camera);


#ifdef _cplusplus
} /* extern "C" */
#endif

/* TWL_CAMERA_I2C_COMMON_H_ */
#endif
