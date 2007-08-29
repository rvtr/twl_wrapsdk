/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - mcu
  File:     mcu_reg.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MCU_ARM7_MCU_REG_H_
#define TWL_MCU_ARM7_MCU_REG_H_

#define MCU_REG_IRQ_ADDR                0x00
#define MCU_REG_MODE_ADDR               0x01
#define MCU_REG_WIFI_ADDR               0x02
#define MCU_REG_CAMERA_ADDR             0x03
#define MCU_REG_VOLUME_ADDR             0x04
#define MCU_REG_INFO_ADDR               0x05

#define MCU_REG_TEMP_ADDR               0x06
#define MCU_REG_TEMP_LAST_ADDR          0x25

/* MCU_REG_IRQ */

#define MCU_REG_IRQ_RESET_SHIFT         0
#define MCU_REG_IRQ_RESET_SIZE          1
#define MCU_REG_IRQ_RESET_MASK          0x01

/* MCU_REG_MODE */

#define MCU_REG_MODE_SYSTEM_SHIFT       0
#define MCU_REG_MODE_SYSTEM_SIZE        2
#define MCU_REG_MODE_SYSTEM_MASK        0x03

/* MCU_REG_WIFI */

#define MCU_REG_WIFI_SHIFT              0
#define MCU_REG_WIFI_SIZE               1
#define MCU_REG_WIFI_MASK               0x01

/* MCU_REG_CAMERA */

#define MCU_REG_CAMERA_PATTERN_SHIFT    4
#define MCU_REG_CAMERA_PATTERN_SIZE     4
#define MCU_REG_CAMERA_PATTERN_MASK     0xf0

#define MCU_REG_CAMERA_PARAM_SHIFT      0
#define MCU_REG_CAMERA_PARAM_SIZE       4
#define MCU_REG_CAMERA_PARAM_MASK       0x0f

/* MCU_REG_VOL */

#define MCU_REG_VOLUME_SHIFT            0
#define MCU_REG_VOLUME_SIZE             5
#define MCU_REG_VOLUME_MASK             0x1f

/* MCU_REG_INFO */

#define MCU_REG_INFO_VERSION_SHIFT      4
#define MCU_REG_INFO_VERSION_SIZE       4
#define MCU_REG_INFO_VERSION_MASK       0xf0

#define MCU_REG_INFO_REVISION_SHIFT     0
#define MCU_REG_INFO_REVISION_SIZE      4
#define MCU_REG_INFO_REVISION_MASK      0x0f

#endif /* TWL_MCU_ARM7_MCU_REG_H_ */
