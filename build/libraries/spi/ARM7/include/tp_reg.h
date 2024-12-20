/*---------------------------------------------------------------------------*
  Project:  CtrSDK - TP - include - tp
  File:     cdc_reg.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef CTR_TP_TP_REG_H_
#define CTR_TP_TP_REG_H_

//#include <ctr/misc.h>
//#include <ctr/types.h>


#ifdef __cplusplus
extern "C" {
#endif


//================================================================================
//        CODEC Page Register
//================================================================================
#define REG_TP_PAGE_CTL_ADDR        0
#define REG_TP255_PAGE_CTL_ADDR     127

//================================================================================
//        CODEC Version ..etc
//================================================================================
#define REG_TP0_VENDOR_ID_ADDR      2
#define REG_TP0_REV_ID_ADDR         3
#define TP0_REV_ID_MASK             0x70
#define TP0_REV_ID_SHIFT            4

//================================================================================
//        Resolution / ADC Power
//================================================================================
#define REG_TP_RESOLUTION   2

typedef enum
{
    TP_RESOLUTION_12 = 0x10,
    TP_RESOLUTION_8  = 0x48,
    TP_RESOLUTION_10 = 0x20

} TpResolution_t;

#define TP_RESOLUTION_MASK  0x78
#define TP_ADC_PWR_MASK     0x80
#define TP_ADC_PWRUP        0x00
#define TP_ADC_PWRDN        0x80

//================================================================================
//        Sampling Conversion Mode
//================================================================================
#define REG_TP_CONVERSION_MODE   3
#define TP_CONVERSION_CONTROL_MASK 0x80 
#define TP_CONVERSION_MODE_MASK    0x3c
#define TP_CONVERSION_PIN_MASK     0x03

typedef enum
{
    TP_CONVERSION_CONTROL_HOST     = 0x00,
    TP_CONVERSION_CONTROL_SELF     = 0x80
} TpCnversionControl_t;

typedef enum
{
    TP_CONVERSION_MODE_NONE     = 0x00,
    TP_CONVERSION_MODE_XY       = 0x04,     // Self, /READREADY = ReadReady
    TP_CONVERSION_MODE_XYZ      = 0x08,
    TP_CONVERSION_MODE_X        = 0x0c,
    TP_CONVERSION_MODE_Y        = 0x10,
    TP_CONVERSION_MODE_Z        = 0x14,
    TP_CONVERSION_MODE_AUX3     = 0x18,
    TP_CONVERSION_MODE_AUX2     = 0x1c,
    TP_CONVERSION_MODE_AUX1     = 0x20,
    TP_CONVERSION_MODE_AUTO_AUX = 0x24,
    TP_CONVERSION_MODE_AUX123   = 0x2c,
    TP_CONVERSION_MODE_XP_XM    = 0x34,
    TP_CONVERSION_MODE_YP_YM    = 0x38,
    TP_CONVERSION_MODE_YP_XM    = 0x3c
    
} TpConversionMode_t;

typedef enum
{
    TP_CONVERSION_PIN_INTERRUPT                = 0x00,
    TP_CONVERSION_PIN_DATA_AVAILABLE           = 0x01,
    TP_CONVERSION_PIN_INTERRUPT_DATA_AVAILABLE = 0x02,
    TP_CONVERSION_PIN_NBM                      = 0x03    
} TpCnversionPin_t;

//================================================================================
//        Precharge / Sense / Stability time
//================================================================================
#define REG_TP_PRECHARGE   4

typedef enum
{
    TP_SETUP_TIME_0_1US = 0,
    TP_SETUP_TIME_1US,
    TP_SETUP_TIME_3US,
    TP_SETUP_TIME_10US,
    TP_SETUP_TIME_30US,
    TP_SETUP_TIME_100US,
    TP_SETUP_TIME_300US,
    TP_SETUP_TIME_1MS

} TpSetupTime_t;

#define TP_PRECHARGE_SHIFT  4
#define TP_PRECHARGE_MASK   0x70


#define REG_TP_SENSE_TIME    4
#define TP_SENSE_TIME_SHIFT  0
#define TP_SENSE_TIME_MASK   0x07


#define REG_TP_STABILIZATION_TIME    5
#define TP_STABILIZATION_TIME_SHIFT  0
#define TP_STABILIZATION_TIME_MASK   0x07

//================================================================================
//        IC Status
//================================================================================
/*
#define REG_TP_STATUS1  9
#define REG_TP_STATUS2  10
#define REG_TP_STATUS3  14

#define REG_TP_STATUS3_MASK     0x03

typedef enum
{
    // reg 9
    TP_CONVERTER_STATUS_TOUCH_DETECTED      = 0x8000,
    TP_CONVERTER_STATUS_ADC_NOT_BUSY        = 0x4000,
    TP_CONVERTER_STATUS_DATA_AVAILABLE      = 0x2000,
    TP_CONVERTER_STATUS_X_AVAILABLE         = 0x0800,
    TP_CONVERTER_STATUS_Y_AVAILABLE         = 0x0400,
    // reg 10
    TP_CONVERTER_STATUS_AUX1_AVAILABLE      = 0x0080,
    TP_CONVERTER_STATUS_AUX2_AVAILABLE      = 0x0040,
    TP_CONVERTER_STATUS_AUX3_AVAILABLE      = 0x0020,
    // reg14
    TP_CONVERTER_STATUS_TP_DATA_NOT_READY   = 0x0002,
    TP_CONVERTER_STATUS_FIFO_FLUSH_DONE     = 0x0001
    
} tpConverterStatus_t;
*/
//================================================================================
//        New Buffer Mode
//================================================================================
#define REG_TP_NEW_BUFFER_MODE      14
#define TP_NEW_BUFFER_MODE_E        0x80
#define TP_NEW_BUFFER_MODE_D        0x00
#define TP_NEW_BUFFER_MODE_MASK     0x80

#define TP_NEW_BUFFER_CONVERSION_MODE_CONTINUOUS 0x00 
#define TP_NEW_BUFFER_CONVERSION_MODE_SINGLESHOT 0x40
#define TP_NEW_BUFFER_CONVERSION_MODE_MASK       0x40

#define TP_HOLDOFF_ENABLE   0x04
#define TP_HOLDOFF_DISABLE  0x00
#define TP_HOLDOFF_MASK     0x04

#define REG_TP_DATA_DEPTH           14
#define TP_DATA_DEPTH_SHIFT         3
#define TP_DATA_DEPTH_MASK          0x38

//================================================================================
//        Interval
//================================================================================
#define REG_TP_INTERVAL     15

typedef enum
{
    TP_INTERVAL_NONE        = 0,
    TP_AUX_INTERVAL_1_12M   = 0x08,
    TP_AUX_INTERVAL_3_36M,
    TP_AUX_INTERVAL_5_59M,
    TP_AUX_INTERVAL_7_83M,
    TP_AUX_INTERVAL_10_01M,
    TP_AUX_INTERVAL_12_30M,
    TP_AUX_INTERVAL_14_54M,
    TP_AUX_INTERVAL_16_78M,
    TP_INTERVAL_8MS         = 0x80,
    TP_INTERVAL_1MS         = 0x90,
    TP_INTERVAL_2MS         = 0xa0,
    TP_INTERVAL_3MS         = 0xb0,
    TP_INTERVAL_4MS         = 0xc0,
    TP_INTERVAL_5MS         = 0xd0,
    TP_INTERVAL_6MS         = 0xe0,
    TP_INTERVAL_7MS         = 0xf0
    
} tpInterval_t;

//================================================================================
//        Clock Source
//================================================================================
#define REG_TP_INTERVAL_TIMER_SOURCE        16
#define TP_INTERVAL_TIMER_SOURCE_EXTERNAL   0x81
#define TP_INTERVAL_TIMER_SOURCE_INTERNAL   0x00

#define REG_TP_ADC_TIMER_SOURCE        17
#define TP_ADC_TIMER_SOURCE_EXTERNAL   0x81
#define TP_ADC_TIMER_SOURCE_INTERNAL   0x00

//================================================================================
//        Debounce
//================================================================================
#define REG_TP_DEBOUNCE_TIME     18
#define TP_DEBOUNCE_TIME_SHIFT  0
#define TP_DEBOUNCE_TIME_MASK   0x07

typedef enum
{
    TP_DEBOUNCE_0US     = 0,
    TP_DEBOUNCE_16US,   // ADC Timer Source が External なら、8/12.19倍の期間になる
    TP_DEBOUNCE_32US,
    TP_DEBOUNCE_64US,
    TP_DEBOUNCE_128US,
    TP_DEBOUNCE_256US,
    TP_DEBOUNCE_512US,
    TP_DEBOUNCE_1024US

} tpDebounce_t;

//================================================================================
//        AUX Channel
//================================================================================
#define REG_TP_ENABLED_AUX_CHANNEL      19
#define TP_ENABLED_AUX_CHANNEL_SHIFT    5
#define TP_ENABLED_AUX_CHANNEL_MASK     0xe0

typedef enum
{
    TP_ENABLED_AUX_CHANNEL_NONE     = 0,
    TP_ENABLED_AUX_CHANNEL_IN3,
    TP_ENABLED_AUX_CHANNEL_IN2,
    TP_ENABLED_AUX_CHANNEL_IN2_IN3,
    TP_ENABLED_AUX_CHANNEL_IN1,
    TP_ENABLED_AUX_CHANNEL_IN1_IN3,
    TP_ENABLED_AUX_CHANNEL_IN1_IN2,
    TP_ENABLED_AUX_CHANNEL_IN1_IN2_IN3

} tpEnabledAuxChannel_t;

//================================================================================
//        Sampling Data Read
//================================================================================
#define REG_TP_SAMPLING_DATA_X      42
#define REG_TP_SAMPLING_DATA_Y      44
#define REG_TP_SAMPLING_DATA_IN1    54
#define REG_TP_SAMPLING_DATA_IN2    56
#define REG_TP_SAMPLING_DATA_IN3    58

typedef enum
{
    TP_SAMPLING_DATA_X      = 42,
    TP_SAMPLING_DATA_Y      = 44,
    TP_SAMPLING_DATA_IN1    = 54,
    TP_SAMPLING_DATA_IN2    = 56,
    TP_SAMPLING_DATA_IN3    = 58
    
} tpSamplingDataChannel_t;

#define REG_TP_BUFFER           1
#define TP_READ_BUFFER_TIMEOUT  16

//================================================================================
//        Sampling Data Format
//================================================================================
#define TP_NOT_TOUCH_MASK   0xf000


#ifdef __cplusplus
} /* extern "C" */
#endif

/* CTR_TP_TP_REG_H_ */
#endif
