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

#define MCU_REG_VER_INFO_ADDR           0x00
#define MCU_REG_PMIC_INFO_ADDR          0x01
#define MCU_REG_BATT_INFO_ADDR          0x02

#define MCU_REG_IRQ_ADDR                0x10
#define MCU_REG_COMMAND_ADDR            0x11
#define MCU_REG_MODE_ADDR               0x12

#define MCU_REG_POWER_INFO_ADDR         0x20
#define MCU_REG_POWER_MODE_ADDR         0x21

#define MCU_REG_WIFI_ADDR               0x30
#define MCU_REG_CAMERA_ADDR             0x31
#define MCU_REG_GPIO_DIR_ADDR           0x32
#define MCU_REG_GPIO_DATA_ADDR          0x33

#define MCU_REG_VOLUME_ADDR             0x40
#define MCU_REG_BL_ADDR                 0x41

#define MCU_REG_BATT_CALIB_ADDR         0x60
#define MCU_REG_CALIB_STATUS_ADDR       0x61

#define MCU_REG_TEMP_ADDR               0x70
#define MCU_REG_TEMP_LAST_ADDR          0x77

#define MCU_REG_RESET_TIME_ADDR         0x80    // temporary
#define MCU_REG_PWOFF_TIME_ADDR         0x81    // temporary

/* MCU_REG_VER_INFO */

#define MCU_REG_VER_INFO_VERSION_SHIFT  4
#define MCU_REG_VER_INFO_VERSION_SIZE   4
#define MCU_REG_VER_INFO_VERSION_MASK   0xf0

#define MCU_REG_VER_INFO_REVISION_SHIFT 0
#define MCU_REG_VER_INFO_REVISION_SIZE  4
#define MCU_REG_VER_INFO_REVISION_MASK  0x0f

/* MCU_REG_PMIC_INFO */

#define MCU_REG_PMIC_INFO_MAKER_SHIFT   4
#define MCU_REG_PMIC_INFO_MAKER_SIZE    3
#define MCU_REG_PMIC_INFO_MAKER_MASK    0x70

#define MCU_REG_PMIC_INFO_VERSION_SHIFT 0
#define MCU_REG_PMIC_INFO_VERSION_SIZE  2
#define MCU_REG_PMIC_INFO_VERSION_MASK  0x03

/* MCU_REG_BATT_INFO */

#define MCU_REG_BATT_INFO_MAKER_SHIFT   4
#define MCU_REG_BATT_INFO_MAKER_SIZE    3
#define MCU_REG_BATT_INFO_MAKER_MASK    0x70

/* MCU_REG_IRQ */

#define MCU_REG_IRQ_EXTDC_SHIFT         7       // not worked
#define MCU_REG_IRQ_EXTDC_SIZE          1       // not worked
#define MCU_REG_IRQ_EXTDC_MASK          0x80    // not worked

#define MCU_REG_IRQ_BATTLOW_SHIFT       5
#define MCU_REG_IRQ_BATTLOW_SIZE        1
#define MCU_REG_IRQ_BATTLOW_MASK        0x20

#define MCU_REG_IRQ_BATTEMP_SHIFT       4
#define MCU_REG_IRQ_BATTEMP_SIZE        1
#define MCU_REG_IRQ_BATTEMP_MASK        0x10

#define MCU_REG_IRQ_PWSW_SHIFT          3
#define MCU_REG_IRQ_PWSW_SIZE           1
#define MCU_REG_IRQ_PWSW_MASK           0x08

#define MCU_REG_IRQ_PWOFF_SHIFT         1
#define MCU_REG_IRQ_PWOFF_SIZE          1
#define MCU_REG_IRQ_PWOFF_MASK          0x02

#define MCU_REG_IRQ_RESET_SHIFT         0
#define MCU_REG_IRQ_RESET_SIZE          1
#define MCU_REG_IRQ_RESET_MASK          0x01

/* MCU_REG_COMMAND */

#define MCU_REG_COMMAND_PWOFF_SHIFT     1       // not worked but using to notify to MCU
#define MCU_REG_COMMAND_PWOFF_SIZE      1       // not worked but using to notify to MCU
#define MCU_REG_COMMAND_PWOFF_MASK      0x02    // not worked but using to notify to MCU

#define MCU_REG_COMMAND_RESET_SHIFT     0
#define MCU_REG_COMMAND_RESET_SIZE      1
#define MCU_REG_COMMAND_RESET_MASK      0x01

/* MCU_REG_MODE */

#define MCU_REG_MODE_SYSTEM_SHIFT       0
#define MCU_REG_MODE_SYSTEM_SIZE        2
#define MCU_REG_MODE_SYSTEM_MASK        0x03

/* MCU_REG_POWER_INFO */

#define MCU_REG_POWER_INFO_EXTDC_SHIFT  7
#define MCU_REG_POWER_INFO_EXTDC_SIZE   1
#define MCU_REG_POWER_INFO_EXTDC_MASK   0x80

#define MCU_REG_POWER_INFO_LEVEL_SHIFT  0
#define MCU_REG_POWER_INFO_LEVEL_SIZE   4
#define MCU_REG_POWER_INFO_LEVEL_MASK   0x0f

/* MCU_REG_POWER_MODE */

#define MCU_REG_POWER_MODE_EXT_SHIFT    0
#define MCU_REG_POWER_MODE_EXT_SIZE     1
#define MCU_REG_POWER_MODE_EXT_MASK     0x01

/* MCU_REG_WIFI */

#define MCU_REG_WIFI_NRESET_SHIFT       4
#define MCU_REG_WIFI_NRESET_SIZE        1
#define MCU_REG_WIFI_NRESET_MASK        0x10

#define MCU_REG_WIFI_LED_SHIFT          0
#define MCU_REG_WIFI_LED_SIZE           1
#define MCU_REG_WIFI_LED_MASK           0x01

/* MCU_REG_CAMERA */

#define MCU_REG_CAMERA_LED_SHIFT        0
#define MCU_REG_CAMERA_LED_SIZE         2
#define MCU_REG_CAMERA_LED_MASK         0x03

/* MCU_REG_GPIO_DIR */

#define MCU_REG_GPIO_DIR_PIN0_SHIFT     0
#define MCU_REG_GPIO_DIR_PIN0_SIZE      1
#define MCU_REG_GPIO_DIR_PIN0_MASK      0x01

/* MCU_REG_GPIO_DATA */

#define MCU_REG_GPIO_DATA_PORT0_SHIFT   0
#define MCU_REG_GPIO_DATA_PORT0_SIZE    1
#define MCU_REG_GPIO_DATA_PORT0_MASK    0x01

/* MCU_REG_VOLUME */

#define MCU_REG_VOLUME_VOLUME_SHIFT     0
#define MCU_REG_VOLUME_VOLUME_SIZE      5
#define MCU_REG_VOLUME_VOLUME_MASK      0x1f

/* MCU_REG_BL */

#define MCU_REG_BL_BRIGHTNESS_SHIFT     0
#define MCU_REG_BL_BRIGHTNESS_SIZE      3
#define MCU_REG_BL_BRIGHTNESS_MASK      0x07

/* MCU_REG_BATT_CALIB */

#define MCU_REG_BATT_CALIB_MODE_SHIFT   0
#define MCU_REG_BATT_CALIB_MODE_SIZE    1
#define MCU_REG_BATT_CALIB_MODE_MASK    0x01

/* MCU_REG_CALIB_STATUS */

#define MCU_REG_CALIB_STATUS_STATUS_SHIFT   0
#define MCU_REG_CALIB_STATUS_STATUS_SIZE    2
#define MCU_REG_CALIB_STATUS_STATUS_MASK    0x03

/* MCU_REG_TEMP */

/* MCU_REG_RESET_TIME */

#define MCU_REG_RESET_TIME_VALUE_SHIFT  0
#define MCU_REG_RESET_TIME_VALUE_SIZE   8
#define MCU_REG_RESET_TIME_VALUE_MASK   0xff

/* MCU_REG_PWOFF_TIME */

#define MCU_REG_PWOFF_TIME_VALUE_SHIFT  0
#define MCU_REG_PWOFF_TIME_VALUE_SIZE   8
#define MCU_REG_PWOFF_TIME_VALUE_MASK   0xff

#endif /* TWL_MCU_MCU_REG_H_ */
