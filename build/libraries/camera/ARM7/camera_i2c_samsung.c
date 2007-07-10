/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - camera
  File:     camera_i2c.c

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
#include <twl/camera.h>

//#define USE_MULTIPLE_IO   // use [Read|Write]Registers();

// for samsung 1/10

extern BOOL CAMERAi_I2CPreset_A3AFX_EVT2_20fps_16M_PLLoff_hVGA( CameraSelect camera );
extern BOOL CAMERAi_I2CPreset_A3AFX_EVT2_30fps_16M_QVGA( CameraSelect camera );
extern BOOL CAMERAi_I2CPreset_A3AFX_EVT2_20fps_16M( CameraSelect camera );
extern BOOL CAMERAi_I2CPreset_320x240_noPLL_20fps( CameraSelect camera );
extern BOOL CAMERAi_I2CPreset_320x240_PLL_30fps( CameraSelect camera );
extern BOOL CAMERAi_I2CPreset_A3AFX_EVT3_30fps_Scaledown_16_7M_50Hz( CameraSelect camera );
extern BOOL CAMERAi_I2CPreset_A3AFX_EVT3_30fps_Subsampling_16_7M_50Hz( CameraSelect camera );

typedef BOOL (*CameraI2CPresetFunc)( CameraSelect camera );

static CameraI2CPresetFunc gs_preset[] = {
//    CAMERAi_I2CPreset_A3AFX_EVT2_20fps_16M_PLLoff_hVGA,
//    CAMERAi_I2CPreset_A3AFX_EVT2_30fps_16M_QVGA,
    CAMERAi_I2CPreset_A3AFX_EVT2_20fps_16M,
//    CAMERAi_I2CPreset_320x240_noPLL_20fps,
//    CAMERAi_I2CPreset_320x240_PLL_30fps,
    CAMERAi_I2CPreset_A3AFX_EVT3_30fps_Scaledown_16_7M_50Hz,
    CAMERAi_I2CPreset_A3AFX_EVT3_30fps_Subsampling_16_7M_50Hz,
};

BOOL CAMERA_I2CPreset(CameraSelect camera, CameraPreset preset)
{
    BOOL result = FALSE;
    if (preset >= CAMERA_PRESET_MAX) {
        return result;
    }
    if (gs_preset[preset] == NULL) {
        return result;
    }
    (void)I2C_Lock();
    result = gs_preset[preset](camera);
    (void)I2C_Unlock();
    return result;
}


#define PAGE_ADDR   0xef

#define MIRROR_MODE 0x02    //[6:7] only?
#define WRP_DOWN    0x04
#define WCP_DOWN    0x06

BOOL CAMERA_I2CSetFlipMode(CameraSelect camera, CameraFlipMode mode)
{

    (void)I2C_Lock();

    switch (mode)
    {
    case CAMERA_FLIPMODE_NONE:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x02) == FALSE ||
            CAMERAi_WriteRegister(camera, MIRROR_MODE, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, WRP_DOWN, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, WCP_DOWN, 0x00) == FALSE) {
            goto error;
        }
        break;
    case CAMERA_FLIPMODE_HORIZONTAL:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x02) == FALSE ||
            CAMERAi_WriteRegister(camera, MIRROR_MODE, 0x40) == FALSE ||
            CAMERAi_WriteRegister(camera, WRP_DOWN, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, WCP_DOWN, 0x01) == FALSE) {
            goto error;
        }
        break;
    case CAMERA_FLIPMODE_VERTICAL:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x02) == FALSE ||
            CAMERAi_WriteRegister(camera, MIRROR_MODE, 0x80) == FALSE ||
            CAMERAi_WriteRegister(camera, WRP_DOWN, 0x01) == FALSE ||
            CAMERAi_WriteRegister(camera, WCP_DOWN, 0x00) == FALSE) {
            goto error;
        }
        break;
    case CAMERA_FLIPMODE_HORIZONTAL_VERTICAL:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x02) == FALSE ||
            CAMERAi_WriteRegister(camera, MIRROR_MODE, 0xC0) == FALSE ||
            CAMERAi_WriteRegister(camera, WRP_DOWN, 0x01) == FALSE ||
            CAMERAi_WriteRegister(camera, WCP_DOWN, 0x01) == FALSE) {
            goto error;
        }
        break;
    default:
        goto error;
    }
    (void)I2C_Unlock();
    return TRUE;
error:
    (void)I2C_Unlock();
    return FALSE;
}

#define NEVAGIVE_EFFECT_MODE    0xD3
#define SEPIA_EFFECT_MODE       0xD4
#define SEPIA_EFFECT_CB         0xD5
#define SEPIA_EFFECT_CR         0xD6

BOOL CAMERA_I2CSetSpecialMode(CameraSelect camera, CameraSpecialMode mode)
{

    (void)I2C_Lock();

    switch (mode)
    {
    case CAMERA_SPECIALMODE_NONE:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, NEVAGIVE_EFFECT_MODE, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_MODE, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CB, 0x2C) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CR, 0x81) == FALSE) {
            goto error;
        }
        break;
    case CAMERA_SPECIALMODE_NEVATIVE:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, NEVAGIVE_EFFECT_MODE, 0x01) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_MODE, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CB, 0x2C) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CR, 0x81) == FALSE) {
            goto error;
        }
        break;
    case CAMERA_SPECIALMODE_SEPIA:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, NEVAGIVE_EFFECT_MODE, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_MODE, 0x03) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CB, 0x2C) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CR, 0x81) == FALSE) {
            goto error;
        }
        break;
    //case CAMERA_SPECIALMODE_AQUA:
    case CAMERA_SPECIALMODE_BLUISH:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, NEVAGIVE_EFFECT_MODE, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_MODE, 0x03) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CB, 0xAC) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CR, 0x81) == FALSE) {
            goto error;
        }
        break;
    case CAMERA_SPECIALMODE_REDDISH:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, NEVAGIVE_EFFECT_MODE, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_MODE, 0x03) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CB, 0xAC) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CR, 0xC1) == FALSE) {
            goto error;
        }
        break;
    case CAMERA_SPECIALMODE_GREENISH:
        if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, NEVAGIVE_EFFECT_MODE, 0x00) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_MODE, 0x03) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CB, 0xAC) == FALSE ||
            CAMERAi_WriteRegister(camera, SEPIA_EFFECT_CR, 0x41) == FALSE) {
            goto error;
        }
        break;
    default:
            goto error;
    }
    (void)I2C_Unlock();
    return TRUE;
error:
    (void)I2C_Unlock();
    return FALSE;
}

#define DEST_ADDR   0x7F
#define CAPT_ADDR   0xC4
BOOL CAMERA_I2CSetCroppingParams(CameraSelect camera, u16 width, u16 height)
{
#ifdef USE_MULTIPLE_IO
    u8 data[4];
#endif
    if (width > 640 || height > 480) {
        return FALSE;
    }
    (void)I2C_Lock();
    if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x00) == FALSE) {
        (void)I2C_Unlock();
        return FALSE;
    }
    // set destination registers
#ifdef USE_MULTIPLE_IO
    data[0] = (height + 0x10) >> 8;
    data[1] = (height + 0x10) & 0xFF;
    data[2] = (width + 0x10) >> 8;
    data[3] = (width + 0x10) & 0xFF;
    if (CAMERAi_WriteRegisters(camera, DEST_ADDR, data, 4) == FALSE) {
#else
    if (CAMERAi_WriteRegister(camera, DEST_ADDR, (u8)((height + 0x10) >> 8)) == FALSE ||
        CAMERAi_WriteRegister(camera, DEST_ADDR+1, (u8)((height + 0x10) & 0xFF)) ==FALSE ||
        CAMERAi_WriteRegister(camera, DEST_ADDR+2, (u8)((width + 0x10) >> 8)) == FALSE ||
        CAMERAi_WriteRegister(camera, DEST_ADDR+3, (u8)((width + 0x10) & 0xFF)) == FALSE) {
#endif
        (void)I2C_Unlock();
        return FALSE;
    }
    // set capture registers
#ifdef USE_MULTIPLE_IO
    data[0] = height >> 8;
    data[1] = height & 0xFF;
    data[2] = width >> 8;
    data[3] = width & 0xFF;
    if (CAMERAi_WriteRegisters(camera, CAPT_ADDR, data, 4) == FALSE) {
#else
    if (CAMERAi_WriteRegister(camera, CAPT_ADDR, (u8)(height >> 8)) == FALSE ||
        CAMERAi_WriteRegister(camera, CAPT_ADDR+1, (u8)(height & 0xFF)) ==FALSE ||
        CAMERAi_WriteRegister(camera, CAPT_ADDR+2, (u8)(width >> 8)) == FALSE ||
        CAMERAi_WriteRegister(camera, CAPT_ADDR+3, (u8)(width & 0xFF)) == FALSE) {
#endif
        (void)I2C_Unlock();
        return FALSE;
    }
    (void)I2C_Unlock();
    return TRUE;
}

BOOL CAMERA_I2CGetCroppingParams(CameraSelect camera, u16 *pWidth, u16 *pHeight)
{
    u8 data[4];
    (void)I2C_Lock();
    if (CAMERAi_WriteRegister(camera, PAGE_ADDR, 0x00) == FALSE) {
        (void)I2C_Unlock();
        return FALSE;
    }
    data[0] = CAMERAi_ReadRegister(camera, CAPT_ADDR);
    data[1] = CAMERAi_ReadRegister(camera, CAPT_ADDR+1);
    data[2] = CAMERAi_ReadRegister(camera, CAPT_ADDR+2);
    data[3] = CAMERAi_ReadRegister(camera, CAPT_ADDR+3);

    (void)I2C_Unlock();

    if (pWidth) {
        *pWidth = (u16)(data[3] | (data[2] << 8));
    }
    if (pHeight) {
        *pHeight = (u16)(data[1] | (data[0] << 8));
    }
    return TRUE;
}
