/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PM - include - common
  File:     pm_ex_reg.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_PM_PM_EX_REG_H_
#define TWL_PM_PM_EX_REG_H_


#ifdef __cplusplus
extern "C" {
#endif


//================================================================
//    PMIC extention register parameter
//================================================================
//---------------- address
#define REG_PMIC_CTL2_ADDR       0x10     // R/W
#define REG_PMIC_BT_STAT_ADDR    0x11     // R
#define REG_PMIC_SW_FLAGS_ADDR   0x12     // R/W
#define REG_PMIC_AGPIO_CTL_ADDR  0x13     // R/W
#define REG_PMIC_GPIO_ADDR       0x14     // R/W
#define REG_PMIC_OFF_TIME_ADDR   0x15     // R/W
#define REG_PMIC_PFM_PWM_ADDR    0x16     // R/W
#define REG_PMIC_GPU_VLT_ADDR    0x17     // R/W
#define REG_PMIC_BT_CRCT_ADDR    0x18     // R/W
#define REG_PMIC_BT_THL_ADDR     0x19     // R/W
#define REG_PMIC_BT_THH_ADDR     0x1a     // R/W
#define REG_PMIC_BT_VDET_ADDR    0x1b     // R/W
#define REG_PMIC_LED_CTL_ADDR    0x1c     // R/W
#define REG_PMIC_LED12_B4_ADDR   0x1d     // R/W
#define REG_PMIC_LED12_B3_ADDR   0x1e     // R/W
#define REG_PMIC_LED12_B2_ADDR   0x1f     // R/W
#define REG_PMIC_LED12_B15_ADDR  0x20     // R/W
#define REG_PMIC_LED3_BRT_ADDR   0x21     // R/W
#define REG_PMIC_LED12_BLK_ADDR  0x22     // R/W
#define REG_PMIC_LED3_BLK_ADDR   0x23     // R/W
#define REG_PMIC_BL_BRT_ADDR     0x24     // R/W
#define REG_PMIC_DEBUG_ADDR      0x2f     // R/W


//---------------- each register spec
//---- PMIC_CTL2
#define PMIC_CTL2_RST           (1<< 0)
#define PMIC_CTL2_BL_SHIFT       2
#define PMIC_CTL2_BL_ON         (3<< PMIC_CTL2_BL_SHIFT)
#define PMIC_CTL2_BL_OFF        (0<< PMIC_CTL2_BL_SHIFT)
#define PMIC_CTL2_GPU_DPD       (1<< 4)
#define PMIC_CTL2_LCD_PWR       (1<< 5)
#define PMIC_CTL2_PWR_OFF       (1<< 6)

//---- PMIC_BT_STAT
#define PMIC_BT_STAT_VLTLOW     (1<< 0)
#define PMIC_BT_STAT_VLT_SHIFT  1
#define PMIC_BT_STAT_VLT_MASK   (7<< PMIC_BT_STAT_VLT_SHIFT)
#define PMIC_BT_STAT_MKR_SHIFT  5
#define PMIC_BT_STAT_MKR_MASK   (7<< PMIC_BT_STAT_MKR_SHIFT)

//---- PMIC_SW_FLAGS
#define PMIC_SW_FLAGS_WARMBOOT  (1 << 7)

//---- PMIC_AGPIO_CTL
#define PMIC_AGPIO_CTL_O_DACRST (1<< 1)     // maybe include GPURST
#define PMIC_AGPIO_CTL_O_GPIO2  (1<< 2)
#define PMIC_AGPIO_CTL_O_ADPT   (1<< 3)
#define PMIC_AGPIO_CTL_AO_SHIFT 6
#define PMIC_AGPIO_CTL_AO_MASK  (3<< PMIC_AGPIO_CTL_SHIFT)

//---- PMIC_GPIO
#define PMIC_GPIO_O_DACRST      (1<< 1)     // maybe include GPURST
#define PMIC_GPIO_IO_GPIO2      (1<< 2)
#define PMIC_GPIO_I_ADPT        (1<< 3)
#define PMIC_GPIO_MKR_SHIFT     4
#define PMIC_GPIO_MKR_MASK      (3<< PMIC_BGPIO_MKR_SHIFT)
#define PMIC_GPIO_VER_SHIFT     6
#define PMIC_GPIO_VER_MASK      (3<< PMIC_BGPIO_VER_SHIFT)

//---- PMIC_OFF_TIME
#define PMIC_OFF_TIME_SHIFT     0
#define PMIC_OFF_TIME_MASK      (0xf<< PMIC_OFF_TIME_SHIFT)

//---- PMIC_GPU_VLT
#define PMIC_GPU_VLT_V1A_SHIFT  0
#define PMIC_GPU_VLT_V1A_MASK   (0xf<< PMIC_GPU_VLT_V1A_SHIFT)
#define PMIC_GPU_VLT_V18_SHIFT  4
#define PMIC_GPU_VLT_V18_MASK   (0x3<< PMIC_GPU_VLT_V18_SHIFT)

//---- PMIC_BT_CRCT
#define PMIC_BT_CRCT_TEMP_ON    (1<< 0)
#define PMIC_BT_CRCT_AMPR_ON    (1<< 1)
#define PMIC_BT_CRCT_AK_SHIFT   4
#define PMIC_BT_CRCT_AK_MASK    (0x3<< PMIC_BT_CRCT_AK_SHIFT)
#define PMIC_BT_CRCT_TK_SHIFT   6
#define PMIC_BT_CRCT_TK_MASK    (0x3<< PMIC_BT_CRCT_TK_SHIFT)

//---- PMIC_BT_THL
#define PMIC_BT_THL_TH1_SHIFT   0
#define PMIC_BT_THL_TH1_MASK    (7<< PMIC_BT_THL_TH1_SHIFT)
#define PMIC_BT_THL_TH2_SHIFT   4
#define PMIC_BT_THL_TH2_MASK    (7<< PMIC_BT_THL_TH2_SHIFT)

//---- PMIC_BT_THH
#define PMIC_BT_THH_TH3_SHIFT   0
#define PMIC_BT_THH_TH3_MASK    (7<< PMIC_BT_THH_TH3_SHIFT)
#define PMIC_BT_THH_TH4_SHIFT   4
#define PMIC_BT_THH_TH4_MASK    (7<< PMIC_BT_THH_TH4_SHIFT)

//---- PMIC_PFM_PWM
#define PMIC_PFM_PWM_V18_SHIFT  0
#define PMIC_PFM_PWM_V18_MASK   (0x3<< PMIC_PFM_PWM_V18_SHIFT)
#define PMIC_PFM_PWM_V12_SHIFT  2
#define PMIC_PFM_PWM_V12_MASK   (0x1<< PMIC_PFM_PWM_V12_SHIFT)
#define PMIC_PFM_PWM_V33_SHIFT  3
#define PMIC_PFM_PWM_V33_MASK   (0x1<< PMIC_PFM_PWM_V33_SHIFT)

//---- PMIC_BT_VDET
#define PMIC_BT_VDET_FREQ_SHIFT 0
#define PMIC_BT_VDET_FREQ_MASK  (0x3<< PMIC_BT_VDET_FREQ_SHIFT)
#define PMIC_BT_VDET_NUM_SHIFT  2
#define PMIC_BT_VDET_NUM_MASK   (0x3<< PMIC_BT_VDET_NUM_SHIFT)

//---- PMIC_LED_CTL
#define PMIC_LED_CTL_L12_B4_ONLY (1<< 0)
#define PMIC_LED_CTL_L12_BLK    (1<< 1)
#define PMIC_LED_CTL_L3_BLK     (1<< 2)
#define PMIC_LED_CTL_L12_B3_E   (1<< 4)
#define PMIC_LED_CTL_L12_B4_E   (1<< 5)
#define PMIC_LED_CTL_L12_B5_E   (1<< 6)

//---- PMIC_LED12_B4
#define PMIC_LED12_B4_L1_SHIFT  0
#define PMIC_LED12_B4_L1_MASK   (0x7<< PMIC_LED12_B4_L1_SHIFT)
#define PMIC_LED12_B4_L2_SHIFT  4
#define PMIC_LED12_B4_L2_MASK   (0x7<< PMIC_LED12_B4_L2_SHIFT)

//---- PMIC_LED12_B3
#define PMIC_LED12_B3_L1_SHIFT  0
#define PMIC_LED12_B3_L1_MASK   (0x7<< PMIC_LED12_B3_L1_SHIFT)
#define PMIC_LED12_B3_L2_SHIFT  4
#define PMIC_LED12_B3_L2_MASK   (0x7<< PMIC_LED12_B3_L2_SHIFT)

//---- PMIC_LED12_B2
#define PMIC_LED12_B2_L1_SHIFT  0
#define PMIC_LED12_B2_L1_MASK   (0x7<< PMIC_LED12_B2_L1_SHIFT)
#define PMIC_LED12_B2_L2_SHIFT  4
#define PMIC_LED12_B2_L2_MASK   (0x7<< PMIC_LED12_B2_L2_SHIFT)

//---- PMIC_LED12_B15
#define PMIC_LED12_B5_L1_SHIFT  0
#define PMIC_LED12_B5_L1_MASK   (0x7<< PMIC_LED12_B5_L1_SHIFT)
#define PMIC_LED12_B1_L2_SHIFT  4
#define PMIC_LED12_B1_L2_MASK   (0x7<< PMIC_LED12_B1_L2_SHIFT)

//---- PMIC_LED3_BRT
#define PMIC_LED3_BRT_SHIFT     0
#define PMIC_LED3_BRT_MASK      (0x7<< PMIC_LED3_BRT_SHIFT)

//---- PMIC_LED12_BLK
#define PMIC_LED12_BLK_FQ_SHIFT 0
#define PMIC_LED12_BLK_FQ_MASK  (0x7<< PMIC_LED12_BLK_FQ_SHIFT)
#define PMIC_LED12_BLK_DT_SHIFT 4
#define PMIC_LED12_BLK_DT_MASK  (0x3<< PMIC_LED12_BLK_DT_SHIFT)

//---- PMIC_LED3_BLK
#define PMIC_LED3_BLK_FQ_SHIFT  0
#define PMIC_LED3_BLK_FQ_MASK   (0x7<< PMIC_LED3_BLK_FQ_SHIFT)
#define PMIC_LED3_BLK_DT_SHIFT  4
#define PMIC_LED3_BLK_DT_MASK   (0x3<< PMIC_LED3_BLK_DT_SHIFT)

//---- PMIC_BL_BRT
#define PMIC_BL_BRT_SHIFT       0
#define PMIC_BL_BRT_MASK        (0x1f<< PMIC_BL_BRT_SHIFT)


//---- PMIC_BT_STAT
typedef enum
{
    PMIC_BT_STAT_VLT_L1 = (0 << PMIC_BT_STAT_VLT_SHIFT),
    PMIC_BT_STAT_VLT_L2 = (1 << PMIC_BT_STAT_VLT_SHIFT),
    PMIC_BT_STAT_VLT_L3 = (2 << PMIC_BT_STAT_VLT_SHIFT),
    PMIC_BT_STAT_VLT_L4 = (3 << PMIC_BT_STAT_VLT_SHIFT),
    PMIC_BT_STAT_VLT_L5 = (4 << PMIC_BT_STAT_VLT_SHIFT)
}
PMBatteryLevel;

//---- PMIC_AGPIO_CTL
typedef enum
{
    PMIC_AGPIO_CTL_AO_NONE = (0x0 << PMIC_AGPIO_CTL_AO_SHIFT),  // default
    PMIC_AGPIO_CTL_AO_VLT  = (0x1 << PMIC_AGPIO_CTL_AO_SHIFT),
    PMIC_AGPIO_CTL_AO_AMPR = (0x2 << PMIC_AGPIO_CTL_AO_SHIFT),
    PMIC_AGPIO_CTL_AO_TEMP = (0x3 << PMIC_AGPIO_CTL_AO_SHIFT)
}
PMAnalogOut;

//---- PMIC_OFF_TIME
typedef enum
{
    PMIC_OFF_TIME_100MS  = (0x0 << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_200MS  = (0x1 << PMIC_OFF_TIME_SHIFT),  // default
    PMIC_OFF_TIME_300MS  = (0x2 << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_500MS  = (0x3 << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_700MS  = (0x4 << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_900MS  = (0x5 << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_1S     = (0x6 << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_1500MS = (0x7 << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_2S     = (0x8 << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_2500MS = (0x9 << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_3S     = (0xa << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_4S     = (0xb << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_5S     = (0xc << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_7S     = (0xd << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_9S     = (0xe << PMIC_OFF_TIME_SHIFT),
    PMIC_OFF_TIME_10S    = (0xf << PMIC_OFF_TIME_SHIFT)
}
PMOffTime;

//---- PMIC_GPU_VLT
typedef enum
{
    PMIC_GPU_V1A_0875 = (0x0 << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_0900 = (0x1 << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_0925 = (0x2 << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_0950 = (0x3 << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_0975 = (0x4 << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1000 = (0x5 << PMIC_GPU_VLT_V1A_SHIFT),  // default
    PMIC_GPU_V1A_1025 = (0x6 << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1050 = (0x7 << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1075 = (0x8 << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1100 = (0x9 << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1125 = (0xa << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1150 = (0xb << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1175 = (0xc << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1200 = (0xd << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1225 = (0xe << PMIC_GPU_VLT_V1A_SHIFT),
    PMIC_GPU_V1A_1250 = (0xf << PMIC_GPU_VLT_V1A_SHIFT)
}
PMGpuV1A;

typedef enum
{
    PMIC_GPU_V18_1800 = (0x0 << PMIC_GPU_VLT_V18_SHIFT),  // default
    PMIC_GPU_V18_1850 = (0x1 << PMIC_GPU_VLT_V18_SHIFT),
    PMIC_GPU_V18_1900 = (0x2 << PMIC_GPU_VLT_V18_SHIFT)
}
PMGpuV18;

//---- PMIC_BT_CRCT
typedef enum
{
    PMIC_BT_CRCT_AK_30_10 = (0 << PMIC_BT_CRCT_AK_SHIFT),
    PMIC_BT_CRCT_AK_50_10 = (1 << PMIC_BT_CRCT_AK_SHIFT),  // default
    PMIC_BT_CRCT_AK_70_10 = (2 << PMIC_BT_CRCT_AK_SHIFT),
    PMIC_BT_CRCT_AK_90_10 = (3 << PMIC_BT_CRCT_AK_SHIFT)
}
PMAmprCoeff;

typedef enum
{
    PMIC_BT_CRCT_TK_10 = (0 << PMIC_BT_CRCT_TK_SHIFT),
    PMIC_BT_CRCT_TK_15 = (1 << PMIC_BT_CRCT_TK_SHIFT),  // default
    PMIC_BT_CRCT_TK_20 = (2 << PMIC_BT_CRCT_TK_SHIFT),
    PMIC_BT_CRCT_TK_30 = (3 << PMIC_BT_CRCT_TK_SHIFT)
}
PMTempCoeff;

//---- PMIC_BT_THL / PMIC_BT_THH
typedef enum
{
    PMIC_BT_TH_D0   = 0,
    PMIC_BT_TH_D20  = 1,
    PMIC_BT_TH_D40  = 2,
    PMIC_BT_TH_D60  = 3,
    PMIC_BT_TH_D80  = 4,
    PMIC_BT_TH_D100 = 5,
    PMIC_BT_TH_D120 = 6,
    PMIC_BT_TH_D140 = 7
}
PMBatteryThresholdDownCommon;

typedef enum
{
    PMIC_BT_THL_TH1_D0   = (0 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D20  = (1 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D40  = (2 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D60  = (3 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D80  = (4 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D100 = (5 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D120 = (6 << PMIC_BT_THL_TH1_SHIFT),
    PMIC_BT_THL_TH1_D140 = (7 << PMIC_BT_THL_TH1_SHIFT)
}
PMBatteryThreshold1Down;

typedef enum
{
    PMIC_BT_THL_TH2_D0   = (0 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D20  = (1 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D40  = (2 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D60  = (3 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D80  = (4 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D100 = (5 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D120 = (6 << PMIC_BT_THL_TH2_SHIFT),
    PMIC_BT_THL_TH2_D140 = (7 << PMIC_BT_THL_TH2_SHIFT)
}
PMBatteryThreshold2Down;

typedef enum
{
    PMIC_BT_THH_TH3_D0   = (0 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D20  = (1 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D40  = (2 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D60  = (3 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D80  = (4 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D100 = (5 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D120 = (6 << PMIC_BT_THH_TH3_SHIFT),
    PMIC_BT_THH_TH3_D140 = (7 << PMIC_BT_THH_TH3_SHIFT)
}
PMBatteryThreshold3Down;

typedef enum
{
    PMIC_BT_THH_TH4_D0   = (0 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D20  = (1 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D40  = (2 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D60  = (3 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D80  = (4 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D100 = (5 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D120 = (6 << PMIC_BT_THH_TH4_SHIFT),
    PMIC_BT_THH_TH4_D140 = (7 << PMIC_BT_THH_TH4_SHIFT)
}
PMBatteryThreshold4Down;

//---- PMIC_PFM_PWM
typedef enum
{
    PMIC_PFM_PWM_V18_PWM  = (0 << PMIC_PFM_PWM_V18_SHIFT),  // default
    PMIC_PFM_PWM_V18_PFM  = (1 << PMIC_PFM_PWM_V18_SHIFT),
    PMIC_PFM_PWM_V18_AUTO = (2 << PMIC_PFM_PWM_V18_SHIFT)
}
PMPfmPwmV18;

typedef enum
{
    PMIC_PFM_PWM_V12_PWM  = (0 << PMIC_PFM_PWM_V12_SHIFT),  // default
    PMIC_PFM_PWM_V12_PFM  = (1 << PMIC_PFM_PWM_V12_SHIFT)
}
PMPfmPwmV12;

typedef enum
{
    PMIC_PFM_PWM_V33_PWM  = (0 << PMIC_PFM_PWM_V33_SHIFT),  // default
    PMIC_PFM_PWM_V33_PFM  = (1 << PMIC_PFM_PWM_V33_SHIFT)
}
PMPfmPwmV33;

//---- PMIC_BT_VDET
typedef enum
{
    PMIC_BT_VDET_FREQ_10HZ  = (0 << PMIC_BT_VDET_FREQ_SHIFT),
    PMIC_BT_VDET_FREQ_100HZ = (1 << PMIC_BT_VDET_FREQ_SHIFT),
    PMIC_BT_VDET_FREQ_200HZ = (2 << PMIC_BT_VDET_FREQ_SHIFT),  // default
    PMIC_BT_VDET_FREQ_1KHZ  = (3 << PMIC_BT_VDET_FREQ_SHIFT)
}
PMVltDetectFreq;

typedef enum
{
    PMIC_BT_VDET_NUM_16  = (0 << PMIC_BT_VDET_NUM_SHIFT),  // default
    PMIC_BT_VDET_NUM_64  = (1 << PMIC_BT_VDET_NUM_SHIFT),
    PMIC_BT_VDET_NUM_256 = (2 << PMIC_BT_VDET_NUM_SHIFT),
    PMIC_BT_VDET_NUM_512 = (3 << PMIC_BT_VDET_NUM_SHIFT)
}
PMVltDetectNum;

//---- PMIC_LED12_B4 / PMIC_LED12_B3 / PMIC_LED12_B2 / PMIC_LED12_B15 / PMIC_LED3_BRT
typedef enum
{
    PMIC_LED_BRT_OFF = 0,  // default
    PMIC_LED_BRT_14  = 1,
    PMIC_LED_BRT_28  = 2,
    PMIC_LED_BRT_43  = 3,
    PMIC_LED_BRT_57  = 4,
    PMIC_LED_BRT_71  = 5,
    PMIC_LED_BRT_85  = 6,
    PMIC_LED_BRT_100 = 7
}
PMLedBrightCommon;

typedef enum
{
    PMIC_LED12_B4_L1_OFF = (0 << PMIC_LED12_B4_L1_SHIFT),  // default
    PMIC_LED12_B4_L1_14  = (1 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_28  = (2 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_43  = (3 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_57  = (4 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_71  = (5 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_85  = (6 << PMIC_LED12_B4_L1_SHIFT),
    PMIC_LED12_B4_L1_100 = (7 << PMIC_LED12_B4_L1_SHIFT)
}
PMLed1Bright4;

typedef enum
{
    PMIC_LED12_B4_L2_OFF = (0 << PMIC_LED12_B4_L2_SHIFT),  // default
    PMIC_LED12_B4_L2_14  = (1 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_28  = (2 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_43  = (3 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_57  = (4 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_71  = (5 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_85  = (6 << PMIC_LED12_B4_L2_SHIFT),
    PMIC_LED12_B4_L2_100 = (7 << PMIC_LED12_B4_L2_SHIFT)
}
PMLed2Bright4;

typedef enum
{
    PMIC_LED12_B3_L1_OFF = (0 << PMIC_LED12_B3_L1_SHIFT),  // default
    PMIC_LED12_B3_L1_14  = (1 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_28  = (2 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_43  = (3 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_57  = (4 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_71  = (5 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_85  = (6 << PMIC_LED12_B3_L1_SHIFT),
    PMIC_LED12_B3_L1_100 = (7 << PMIC_LED12_B3_L1_SHIFT)
}
PMLed1Bright3;

typedef enum
{
    PMIC_LED12_B3_L2_OFF = (0 << PMIC_LED12_B3_L2_SHIFT),  // default
    PMIC_LED12_B3_L2_14  = (1 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_28  = (2 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_43  = (3 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_57  = (4 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_71  = (5 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_85  = (6 << PMIC_LED12_B3_L2_SHIFT),
    PMIC_LED12_B3_L2_100 = (7 << PMIC_LED12_B3_L2_SHIFT)
}
PMLed2Bright3;

typedef enum
{
    PMIC_LED12_B2_L1_OFF = (0 << PMIC_LED12_B2_L1_SHIFT),  // default
    PMIC_LED12_B2_L1_14  = (1 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_28  = (2 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_43  = (3 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_57  = (4 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_71  = (5 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_85  = (6 << PMIC_LED12_B2_L1_SHIFT),
    PMIC_LED12_B2_L1_100 = (7 << PMIC_LED12_B2_L1_SHIFT)
}
PMLed1Bright2;

typedef enum
{
    PMIC_LED12_B2_L2_OFF = (0 << PMIC_LED12_B2_L2_SHIFT),  // default
    PMIC_LED12_B2_L2_14  = (1 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_28  = (2 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_43  = (3 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_57  = (4 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_71  = (5 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_85  = (6 << PMIC_LED12_B2_L2_SHIFT),
    PMIC_LED12_B2_L2_100 = (7 << PMIC_LED12_B2_L2_SHIFT)
}
PMLed2Bright2;

typedef enum
{
    PMIC_LED12_B5_L1_OFF = (0 << PMIC_LED12_B5_L1_SHIFT),  // default
    PMIC_LED12_B5_L1_14  = (1 << PMIC_LED12_B5_L1_SHIFT),
    PMIC_LED12_B5_L1_28  = (2 << PMIC_LED12_B5_L1_SHIFT),
    PMIC_LED12_B5_L1_43  = (3 << PMIC_LED12_B5_L1_SHIFT),
    PMIC_LED12_B5_L1_57  = (4 << PMIC_LED12_B5_L1_SHIFT),
    PMIC_LED12_B5_L1_71  = (5 << PMIC_LED12_B5_L1_SHIFT),
    PMIC_LED12_B5_L1_85  = (6 << PMIC_LED12_B5_L1_SHIFT),
    PMIC_LED12_B5_L1_100 = (7 << PMIC_LED12_B5_L1_SHIFT)
}
PMLed1Bright5;

typedef enum
{
    PMIC_LED12_B1_L2_OFF = (0 << PMIC_LED12_B1_L2_SHIFT),  // default
    PMIC_LED12_B1_L2_14  = (1 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_28  = (2 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_43  = (3 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_57  = (4 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_71  = (5 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_85  = (6 << PMIC_LED12_B1_L2_SHIFT),
    PMIC_LED12_B1_L2_100 = (7 << PMIC_LED12_B1_L2_SHIFT)
}
PMLed2Bright1;

typedef enum
{
    PMIC_LED3_BRT_OFF = (0 << PMIC_LED3_BRT_SHIFT),  // default
    PMIC_LED3_BRT_14  = (1 << PMIC_LED3_BRT_SHIFT),
    PMIC_LED3_BRT_28  = (2 << PMIC_LED3_BRT_SHIFT),
    PMIC_LED3_BRT_43  = (3 << PMIC_LED3_BRT_SHIFT),
    PMIC_LED3_BRT_57  = (4 << PMIC_LED3_BRT_SHIFT),
    PMIC_LED3_BRT_71  = (5 << PMIC_LED3_BRT_SHIFT),
    PMIC_LED3_BRT_85  = (6 << PMIC_LED3_BRT_SHIFT),
    PMIC_LED3_BRT_100 = (7 << PMIC_LED3_BRT_SHIFT)
}
PMLed3Bright;

//---- PMIC_LED12_BLK / PMIC_LED3_BLK

typedef enum
{
    PMIC_LED_BLK_FREQ_033HZ = 0,  // default
    PMIC_LED_BLK_FREQ_050HZ = 1,
    PMIC_LED_BLK_FREQ_067HZ = 2,
    PMIC_LED_BLK_FREQ_1HZ   = 3,
    PMIC_LED_BLK_FREQ_2HZ   = 4,
    PMIC_LED_BLK_FREQ_4HZ   = 5
}
PMLedBlinkFreqCommon;

typedef enum
{
    PMIC_LED12_BLK_FREQ_033HZ = (0 << PMIC_LED12_BLK_FQ_SHIFT),  // default
    PMIC_LED12_BLK_FREQ_050HZ = (1 << PMIC_LED12_BLK_FQ_SHIFT),
    PMIC_LED12_BLK_FREQ_067HZ = (2 << PMIC_LED12_BLK_FQ_SHIFT),
    PMIC_LED12_BLK_FREQ_1HZ   = (3 << PMIC_LED12_BLK_FQ_SHIFT),
    PMIC_LED12_BLK_FREQ_2HZ   = (4 << PMIC_LED12_BLK_FQ_SHIFT),
    PMIC_LED12_BLK_FREQ_4HZ   = (5 << PMIC_LED12_BLK_FQ_SHIFT)
}
PMLed12BlinkFreq;

typedef enum
{
    PMIC_LED3_BLK_FREQ_033HZ = (0 << PMIC_LED3_BLK_FQ_SHIFT),  // default
    PMIC_LED3_BLK_FREQ_050HZ = (1 << PMIC_LED3_BLK_FQ_SHIFT),
    PMIC_LED3_BLK_FREQ_067HZ = (2 << PMIC_LED3_BLK_FQ_SHIFT),
    PMIC_LED3_BLK_FREQ_1HZ   = (3 << PMIC_LED3_BLK_FQ_SHIFT),
    PMIC_LED3_BLK_FREQ_2HZ   = (4 << PMIC_LED3_BLK_FQ_SHIFT),
    PMIC_LED3_BLK_FREQ_4HZ   = (5 << PMIC_LED3_BLK_FQ_SHIFT)
}
PMLed3BlinkFreq;

typedef enum
{
    PMIC_LED12_BLK_DUTY_10 = (0 << PMIC_LED12_BLK_DT_SHIFT),  // default
    PMIC_LED12_BLK_DUTY_25 = (1 << PMIC_LED12_BLK_DT_SHIFT),
    PMIC_LED12_BLK_DUTY_50 = (2 << PMIC_LED12_BLK_DT_SHIFT),
    PMIC_LED12_BLK_DUTY_75 = (3 << PMIC_LED12_BLK_DT_SHIFT)
}
PMLed12BlinkDuty;

typedef enum
{
    PMIC_LED3_BLK_DUTY_10      = (0 << PMIC_LED3_BLK_DT_SHIFT),  // default
    PMIC_LED3_BLK_DUTY_25      = (1 << PMIC_LED3_BLK_DT_SHIFT),
    PMIC_LED3_BLK_DUTY_FIREFLY = (2 << PMIC_LED3_BLK_DT_SHIFT),
    PMIC_LED3_BLK_DUTY_NTR     = (3 << PMIC_LED3_BLK_DT_SHIFT)
}
PMLed3BlinkDuty;

//---- PMIC_BL_BRT
typedef enum
{
    PMIC_BL_BRT_MIN = 0,  // default
    PMIC_BL_BRT_MAX = 0x1f,
    PMIC_BL_BRT_DEFAULT = 0x08
}
PMBackLightBrightness;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_PM_PM_EX_REG_H_ */
#endif
