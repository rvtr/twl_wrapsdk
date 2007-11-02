/*---------------------------------------------------------------------------*
  Project:  TwlSDK - mcu - include
  File:     mcu_reg.h

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
#ifndef TWL_MCU_MCU_REG_H_
#define TWL_MCU_MCU_REG_H_

#define MCU_REG_INFO_ADDR               0x00
#define MCU_REG_IRQ_ADDR                0x01
#define MCU_REG_MODE_ADDR               0x02

#define MCU_REG_WIFILED_ADDR            0x10
#define MCU_REG_CAMLED_ADDR             0x11
#define MCU_REG_CARDLED_1_ADDR          0x12
#define MCU_REG_CARDLED_2_ADDR          0x13

#define MCU_REG_TEMP_ADDR               0x18
#define MCU_REG_TEMP_LAST_ADDR          0x1f

#define MCU_REG_VOLUME_ADDR             0x20

/* MCU_REG_INFO */

#define MCU_REG_INFO_VERSION_SHIFT      4
#define MCU_REG_INFO_VERSION_SIZE       4
#define MCU_REG_INFO_VERSION_MASK       0xf0

#define MCU_REG_INFO_REVISION_SHIFT     0
#define MCU_REG_INFO_REVISION_SIZE      4
#define MCU_REG_INFO_REVISION_MASK      0x0f

/* MCU_REG_IRQ */

#define MCU_REG_IRQ_RESET_SHIFT         0
#define MCU_REG_IRQ_RESET_SIZE          1
#define MCU_REG_IRQ_RESET_MASK          0x01

/* MCU_REG_MODE */

#define MCU_REG_MODE_SYSTEM_SHIFT       0
#define MCU_REG_MODE_SYSTEM_SIZE        2
#define MCU_REG_MODE_SYSTEM_MASK        0x03

/* MCU_REG_WIFILED */

#define MCU_REG_WIFILED_SHIFT           0
#define MCU_REG_WIFILED_SIZE            1
#define MCU_REG_WIFILED_MASK            0x01

/* MCU_REG_CAMLED */

#define MCU_REG_CAMLED_PATTERN_SHIFT    4
#define MCU_REG_CAMLED_PATTERN_SIZE     4
#define MCU_REG_CAMLED_PATTERN_MASK     0xf0

#define MCU_REG_CAMLED_PARAM_SHIFT      0
#define MCU_REG_CAMLED_PARAM_SIZE       4
#define MCU_REG_CAMLED_PARAM_MASK       0x0f

/* MCU_REG_CARDLED_1, MCU_REG_CARDLED_2 */

#define MCU_REG_CARDLED_SHIFT           0
#define MCU_REG_CARDLED_SIZE            2
#define MCU_REG_CARDLED_MASK            0x03

#define MCU_REG_CARDLED_1_SHIFT         MCU_REG_CARDLED_SHIFT
#define MCU_REG_CARDLED_1_SIZE          MCU_REG_CARDLED_SIZE
#define MCU_REG_CARDLED_1_MASK          MCU_REG_CARDLED_MASK

#define MCU_REG_CARDLED_2_SHIFT         MCU_REG_CARDLED_SHIFT
#define MCU_REG_CARDLED_2_SIZE          MCU_REG_CARDLED_SIZE
#define MCU_REG_CARDLED_2_MASK          MCU_REG_CARDLED_MASK

/* MCU_REG_VOL */

#define MCU_REG_VOLUME_SHIFT            0
#define MCU_REG_VOLUME_SIZE             5
#define MCU_REG_VOLUME_MASK             0x1f

#endif /* TWL_MCU_MCU_REG_H_ */
