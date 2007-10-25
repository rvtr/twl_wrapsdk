/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CDC - include
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
#ifndef TWL_CDC_CDC_REG_H_
#define TWL_CDC_CDC_REG_H_

#include <twl/misc.h>
#include <twl/types.h>


#ifdef __cplusplus
extern "C" {
#endif


//================================================================
//    DS-mode(only SPI) register parameters
//================================================================
//---------------- address
// page 255  (DS-mode)
#define REG_CDC255_AUD_CTL_ADDR     0     // R/W
#define REG_CDC255_DS_MIC_CTL_ADDR  2     // R/W
#define REG_CDC255_DS_MIC_GAIN_ADDR 3     // R/W
#define REG_CDC255_BKCMPT_MODE_ADDR 5     // R/W
#define REG_CDC255_PAGE_CTL_ADDR    127   // R/W


//---------------- each register spec
//---- CDC255_AUD_CTL
#define CDC255_AUD_CTL_PWR          (1<< 0)

//---- CDC255_DS_MIC_CTL
#define CDC255_DS_MIC_CTL_BIAS_PWR  (1<< 0)

//---- CDC255_DS_MIC_GAIN
#define CDC255_DS_MIC_GAIN_SHIFT    0
#define CDC255_DS_MIC_GAIN_MASK     (3<< CDC255_DS_MIC_GAIN_SHIFT)

//---- CDC255_BKCMPT_MODE
#define CDC255_BKCMPT_MODE_DS_DIS     (1<< 0)  // default


//---- CDC255_DS_MIC_GAIN
typedef enum
{
    CDC255_DS_MIC_GAIN_26 = 0,
    CDC255_DS_MIC_GAIN_32 = 1,  // default
    CDC255_DS_MIC_GAIN_38 = 2,
    CDC255_DS_MIC_GAIN_44 = 3
}
CDCDsMicGain;

//---- CDC255_BKCMPT_MODE
typedef enum
{
    CDC255_BKCMPT_MODE_DS = 0,
    CDC255_BKCMPT_MODE_TWL = CDC255_BKCMPT_MODE_DS_DIS  // default
}
CDCBackCompatibleMode;


//================================================================
//    I2C & SPI common register parameters
//================================================================
//---------------- address
// for all pages except page 255
#define REG_CDC_PAGE_CTL_ADDR       0     // R/W

// page 0 (serialIF and digital control)
#define REG_CDC0_RST_ADDR           1     // R/W
#define REG_CDC0_VEND_ID_ADDR       2     // R
#define REG_CDC0_REV_ID_ADDR        3     // R
#define REG_CDC0_CLK_GEN_ADDR       4     // R/W
#define REG_CDC0_PLL_P_R_ADDR       5     // R/W
#define REG_CDC0_PLL_J_ADDR         6     // R/W
#define REG_CDC0_PLL_D_MSB_ADDR     7     // R/W
#define REG_CDC0_PLL_D_LSB_ADDR     8     // R/W
#define REG_CDC0_NDAC_DIV_ADDR      11    // R/W
#define REG_CDC0_MDAC_DIV_ADDR      12    // R/W
#define REG_CDC0_NADC_DIV_ADDR      18    // R/W
#define REG_CDC0_MADC_DIV_ADDR      19    // R/W
#define REG_CDC0_AUD_IF_ADDR        27    // R/W
#define REG_CDC0_I2C_CTL_ADDR       34    // R/W
#define REG_CDC0_ADC_STAT_ADDR      36    // R
#define REG_CDC0_DAC_STAT1_ADDR     37    // R
#define REG_CDC0_DAC_STAT2_ADDR     38    // R
#define REG_CDC0_OVFL_ADDR          39    // R
#define REG_CDC0_DAC_WARN_ADDR      44    // R
#define REG_CDC0_ADC_WARN_ADDR      45    // R
#define REG_CDC0_PIN_CTL1_ADDR      57    // R/W
#define REG_CDC0_PIN_CTL2_ADDR      58    // R/W
#define REG_CDC0_DAC_OPSET_ADDR     60    // R/W
#define REG_CDC0_ADC_OPSET_ADDR     61    // R/W
#define REG_CDC0_DIG_PATH_ADDR      63    // R/W
#define REG_CDC0_DIG_VOL_M_ADDR     64    // R/W
#define REG_CDC0_DIG_VOL_L_ADDR     65    // R/W
#define REG_CDC0_DIG_VOL_R_ADDR     66    // R/W
#define REG_CDC0_BP_CTL_L_ADDR      71    // R/W
#define REG_CDC0_BP_CTL_R_ADDR      72    // R/W
#define REG_CDC0_BP_LEN_MSB_ADDR    73    // R/W
#define REG_CDC0_BP_LEN_MID_ADDR    74    // R/W
#define REG_CDC0_BP_LEN_LSB_ADDR    75    // R/W
#define REG_CDC0_BP_SIN_MSB_ADDR    76    // R/W
#define REG_CDC0_BP_SIN_LSB_ADDR    77    // R/W
#define REG_CDC0_BP_COS_MSB_ADDR    78    // R/W
#define REG_CDC0_BP_COS_LSB_ADDR    79    // R/W
#define REG_CDC0_ADC_PWR_STEP_ADDR  81    // R/W
#define REG_CDC0_ADC_MUTE_ADDR      82    // R/W
#define REG_CDC0_AGC_CTL1_ADDR      86    // R/W
#define REG_CDC0_AGC_CTL2_ADDR      87    // R/W
#define REG_CDC0_AGC_MAX_GAIN_ADDR  88    // R/W
#define REG_CDC0_AGC_ATTCK_TM_ADDR  89    // R/W
#define REG_CDC0_AGC_DECAY_TM_ADDR  90    // R/W
#define REG_CDC0_AGC_NSE_DBNC_ADDR  91    // R/W
#define REG_CDC0_AGC_SIG_DBNC_ADDR  92    // R/W
#define REG_CDC0_AGC_GAIN_STAT_ADDR 93    // R
#define REG_CDC0_VOLADC_CTL_ADDR    116   // R/W
#define REG_CDC0_VOLADC_STAT_ADDR   117   // R

// page 1 (analog I/O control)
#define REG_CDC1_HP_DRV_ADDR        31    // R/W
#define REG_CDC1_SP_DRV_ADDR        32    // R/W
#define REG_CDC1_HP_DRV_TM_ADDR     33    // R/W
#define REG_CDC1_HPSP_RAMPDWN_ADDR  34    // R/W
#define REG_CDC1_DAC_OUTPUT_ADDR    35    // R/W
#define REG_CDC1_HP_ANGVOL_L_ADDR   36    // R/W
#define REG_CDC1_HP_ANGVOL_R_ADDR   37    // R/W
#define REG_CDC1_SP_ANGVOL_L_ADDR   38    // R/W
#define REG_CDC1_SP_ANGVOL_R_ADDR   39    // R/W
#define REG_CDC1_HP_DRV_L_ADDR      40    // R/W
#define REG_CDC1_HP_DRV_R_ADDR      41    // R/W
#define REG_CDC1_SP_DRV_L_ADDR      42    // R/W
#define REG_CDC1_SP_DRV_R_ADDR      43    // R/W
#define REG_CDC1_MIC_BIAS_ADDR      46    // R/W
#define REG_CDC1_MIC_ADC_PGA_ADDR   47    // R/W
#define REG_CDC1_MIC_PGA_P_ADDR     48    // R/W
#define REG_CDC1_MIC_PGA_M_ADDR     49    // R/W
#define REG_CDC1_MIC_INPUT_ADDR     50    // R/W

// page 3 (touch-panel(TCS) control)
#define REG_CDC3_TP_ADC_CTL_ADDR    2     // R/W
#define REG_CDC3_TP_CONV_MODE_ADDR  3     // R/W
#define REG_CDC3_TP_PRECHG_SNS_ADDR 4     // R/W
#define REG_CDC3_TP_VLT_STB_ADDR    5     // R/W
#define REG_CDC3_TP_STAT1_ADDR      6     // R
#define REG_CDC3_TP_STAT2_ADDR      7     // R
#define REG_CDC3_TP_NBUF_MODE_ADDR  14    // R/W
#define REG_CDC3_TP_DELAY_ADDR      15    // R/W
#define REG_CDC3_TP_DELAY_CLK_ADDR  16    // R/W
#define REG_CDC3_TP_ADC_RST_ADDR    17    // R/W
#define REG_CDC3_TP_DBNC_TM_ADDR    18    // R/W
#define REG_CDC3_TP_AUTO_AUX_ADDR   19    // R/W
#define REG_CDC3_TP_X_MSB_ADDR      42    // R
#define REG_CDC3_TP_X_LSB_ADDR      43    // R
#define REG_CDC3_TP_Y_MSB_ADDR      44    // R
#define REG_CDC3_TP_Y_LSB_ADDR      45    // R
#define REG_CDC3_IN1_MSB_ADDR       54    // R
#define REG_CDC3_IN1_LSB_ADDR       55    // R
#define REG_CDC3_IN2_MSB_ADDR       56    // R
#define REG_CDC3_IN2_LSB_ADDR       57    // R
#define REG_CDC3_IN3_MSB_ADDR       58    // R
#define REG_CDC3_IN3_LSB_ADDR       59    // R

// page 4 (ADC coefficients)
#define REG_CDC4_ADC_COEF_TGL_ADDR  1     // R
#define REG_CDC4_ADC_C1_MSB_ADDR    2     // R/W
#define REG_CDC4_ADC_C1_LSB_ADDR    3     // R/W
#define REG_CDC4_ADC_C2_MSB_ADDR    4     // R/W
#define REG_CDC4_ADC_C2_LSB_ADDR    5     // R/W
#define REG_CDC4_ADC_C3_MSB_ADDR    6     // R/W
#define REG_CDC4_ADC_C3_LSB_ADDR    7     // R/W
#define REG_CDC4_ADC_C4_MSB_ADDR    8     // R/W
#define REG_CDC4_ADC_C4_LSB_ADDR    9     // R/W
#define REG_CDC4_ADC_C5_MSB_ADDR    10    // R/W
#define REG_CDC4_ADC_C5_LSB_ADDR    11    // R/W
#define REG_CDC4_ADC_C6_MSB_ADDR    12    // R/W
#define REG_CDC4_ADC_C6_LSB_ADDR    13    // R/W
#define REG_CDC4_ADC_C7_MSB_ADDR    14    // R/W
#define REG_CDC4_ADC_C7_LSB_ADDR    15    // R/W
#define REG_CDC4_ADC_C8_MSB_ADDR    16    // R/W
#define REG_CDC4_ADC_C8_LSB_ADDR    17    // R/W
#define REG_CDC4_ADC_C9_MSB_ADDR    18    // R/W
#define REG_CDC4_ADC_C9_LSB_ADDR    19    // R/W
#define REG_CDC4_ADC_C10_MSB_ADDR   20    // R/W
#define REG_CDC4_ADC_C10_LSB_ADDR   21    // R/W
#define REG_CDC4_ADC_C11_MSB_ADDR   22    // R/W
#define REG_CDC4_ADC_C11_LSB_ADDR   23    // R/W
#define REG_CDC4_ADC_C12_MSB_ADDR   24    // R/W
#define REG_CDC4_ADC_C12_LSB_ADDR   25    // R/W
#define REG_CDC4_ADC_C13_MSB_ADDR   26    // R/W
#define REG_CDC4_ADC_C13_LSB_ADDR   27    // R/W
#define REG_CDC4_ADC_C14_MSB_ADDR   28    // R/W
#define REG_CDC4_ADC_C14_LSB_ADDR   29    // R/W
#define REG_CDC4_ADC_C15_MSB_ADDR   30    // R/W
#define REG_CDC4_ADC_C15_LSB_ADDR   31    // R/W
#define REG_CDC4_ADC_C16_MSB_ADDR   32    // R/W
#define REG_CDC4_ADC_C16_LSB_ADDR   33    // R/W
#define REG_CDC4_ADC_C17_MSB_ADDR   34    // R/W
#define REG_CDC4_ADC_C17_LSB_ADDR   35    // R/W
#define REG_CDC4_ADC_C18_MSB_ADDR   36    // R/W
#define REG_CDC4_ADC_C18_LSB_ADDR   37    // R/W
#define REG_CDC4_ADC_C19_MSB_ADDR   38    // R/W
#define REG_CDC4_ADC_C19_LSB_ADDR   39    // R/W
#define REG_CDC4_ADC_C20_MSB_ADDR   40    // R/W
#define REG_CDC4_ADC_C20_LSB_ADDR   41    // R/W
#define REG_CDC4_ADC_C21_MSB_ADDR   42    // R/W
#define REG_CDC4_ADC_C21_LSB_ADDR   43    // R/W
#define REG_CDC4_ADC_C22_MSB_ADDR   44    // R/W
#define REG_CDC4_ADC_C22_LSB_ADDR   45    // R/W
#define REG_CDC4_ADC_C23_MSB_ADDR   46    // R/W
#define REG_CDC4_ADC_C23_LSB_ADDR   47    // R/W
#define REG_CDC4_ADC_C24_MSB_ADDR   48    // R/W
#define REG_CDC4_ADC_C24_LSB_ADDR   49    // R/W
#define REG_CDC4_ADC_C25_MSB_ADDR   50    // R/W
#define REG_CDC4_ADC_C25_LSB_ADDR   51    // R/W
#define REG_CDC4_ADC_C26_MSB_ADDR   52    // R/W
#define REG_CDC4_ADC_C26_LSB_ADDR   53    // R/W
#define REG_CDC4_ADC_C27_MSB_ADDR   54    // R/W
#define REG_CDC4_ADC_C27_LSB_ADDR   55    // R/W
#define REG_CDC4_ADC_C28_MSB_ADDR   56    // R/W
#define REG_CDC4_ADC_C28_LSB_ADDR   57    // R/W
#define REG_CDC4_ADC_C29_MSB_ADDR   58    // R/W
#define REG_CDC4_ADC_C29_LSB_ADDR   59    // R/W
#define REG_CDC4_ADC_C30_MSB_ADDR   60    // R/W
#define REG_CDC4_ADC_C30_LSB_ADDR   61    // R/W
#define REG_CDC4_ADC_C31_MSB_ADDR   62    // R/W
#define REG_CDC4_ADC_C31_LSB_ADDR   63    // R/W

// page 8 (DAC coefficients 1)
#define REG_CDC8_DAC_COEF_TGL_ADDR  1     // R
#define REG_CDC8_DAC_C1_MSB_ADDR    2     // R/W
#define REG_CDC8_DAC_C1_LSB_ADDR    3     // R/W
#define REG_CDC8_DAC_C2_MSB_ADDR    4     // R/W
#define REG_CDC8_DAC_C2_LSB_ADDR    5     // R/W
#define REG_CDC8_DAC_C3_MSB_ADDR    6     // R/W
#define REG_CDC8_DAC_C3_LSB_ADDR    7     // R/W
#define REG_CDC8_DAC_C4_MSB_ADDR    8     // R/W
#define REG_CDC8_DAC_C4_LSB_ADDR    9     // R/W
#define REG_CDC8_DAC_C5_MSB_ADDR    10    // R/W
#define REG_CDC8_DAC_C5_LSB_ADDR    11    // R/W
#define REG_CDC8_DAC_C6_MSB_ADDR    12    // R/W
#define REG_CDC8_DAC_C6_LSB_ADDR    13    // R/W
#define REG_CDC8_DAC_C7_MSB_ADDR    14    // R/W
#define REG_CDC8_DAC_C7_LSB_ADDR    15    // R/W
#define REG_CDC8_DAC_C8_MSB_ADDR    16    // R/W
#define REG_CDC8_DAC_C8_LSB_ADDR    17    // R/W
#define REG_CDC8_DAC_C9_MSB_ADDR    18    // R/W
#define REG_CDC8_DAC_C9_LSB_ADDR    19    // R/W
#define REG_CDC8_DAC_C10_MSB_ADDR   20    // R/W
#define REG_CDC8_DAC_C10_LSB_ADDR   21    // R/W
#define REG_CDC8_DAC_C11_MSB_ADDR   22    // R/W
#define REG_CDC8_DAC_C11_LSB_ADDR   23    // R/W
#define REG_CDC8_DAC_C12_MSB_ADDR   24    // R/W
#define REG_CDC8_DAC_C12_LSB_ADDR   25    // R/W
#define REG_CDC8_DAC_C13_MSB_ADDR   26    // R/W
#define REG_CDC8_DAC_C13_LSB_ADDR   27    // R/W
#define REG_CDC8_DAC_C14_MSB_ADDR   28    // R/W
#define REG_CDC8_DAC_C14_LSB_ADDR   29    // R/W
#define REG_CDC8_DAC_C15_MSB_ADDR   30    // R/W
#define REG_CDC8_DAC_C15_LSB_ADDR   31    // R/W
#define REG_CDC8_DAC_C16_MSB_ADDR   32    // R/W
#define REG_CDC8_DAC_C16_LSB_ADDR   33    // R/W
#define REG_CDC8_DAC_C17_MSB_ADDR   34    // R/W
#define REG_CDC8_DAC_C17_LSB_ADDR   35    // R/W
#define REG_CDC8_DAC_C18_MSB_ADDR   36    // R/W
#define REG_CDC8_DAC_C18_LSB_ADDR   37    // R/W
#define REG_CDC8_DAC_C19_MSB_ADDR   38    // R/W
#define REG_CDC8_DAC_C19_LSB_ADDR   39    // R/W
#define REG_CDC8_DAC_C20_MSB_ADDR   40    // R/W
#define REG_CDC8_DAC_C20_LSB_ADDR   41    // R/W
#define REG_CDC8_DAC_C21_MSB_ADDR   42    // R/W
#define REG_CDC8_DAC_C21_LSB_ADDR   43    // R/W
#define REG_CDC8_DAC_C22_MSB_ADDR   44    // R/W
#define REG_CDC8_DAC_C22_LSB_ADDR   45    // R/W
#define REG_CDC8_DAC_C23_MSB_ADDR   46    // R/W
#define REG_CDC8_DAC_C23_LSB_ADDR   47    // R/W
#define REG_CDC8_DAC_C24_MSB_ADDR   48    // R/W
#define REG_CDC8_DAC_C24_LSB_ADDR   49    // R/W
#define REG_CDC8_DAC_C25_MSB_ADDR   50    // R/W
#define REG_CDC8_DAC_C25_LSB_ADDR   51    // R/W
#define REG_CDC8_DAC_C26_MSB_ADDR   52    // R/W
#define REG_CDC8_DAC_C26_LSB_ADDR   53    // R/W
#define REG_CDC8_DAC_C27_MSB_ADDR   54    // R/W
#define REG_CDC8_DAC_C27_LSB_ADDR   55    // R/W
#define REG_CDC8_DAC_C28_MSB_ADDR   56    // R/W
#define REG_CDC8_DAC_C28_LSB_ADDR   57    // R/W
#define REG_CDC8_DAC_C29_MSB_ADDR   58    // R/W
#define REG_CDC8_DAC_C29_LSB_ADDR   59    // R/W
#define REG_CDC8_DAC_C30_MSB_ADDR   60    // R/W
#define REG_CDC8_DAC_C30_LSB_ADDR   61    // R/W
#define REG_CDC8_DAC_ZERO1_ADDR     64    // R/W
#define REG_CDC8_DAC_ZERO2_ADDR     65    // R/W
#define REG_CDC8_DAC_C38_MSB_ADDR   76    // R/W
#define REG_CDC8_DAC_C38_LSB_ADDR   77    // R/W
#define REG_CDC8_DAC_C39_MSB_ADDR   78    // R/W
#define REG_CDC8_DAC_C39_LSB_ADDR   79    // R/W
#define REG_CDC8_DAC_C40_MSB_ADDR   80    // R/W
#define REG_CDC8_DAC_C40_LSB_ADDR   81    // R/W
#define REG_CDC8_DAC_C41_MSB_ADDR   82    // R/W
#define REG_CDC8_DAC_C41_LSB_ADDR   83    // R/W
#define REG_CDC8_DAC_C42_MSB_ADDR   84    // R/W
#define REG_CDC8_DAC_C42_LSB_ADDR   85    // R/W
#define REG_CDC8_DAC_C43_MSB_ADDR   86    // R/W
#define REG_CDC8_DAC_C43_LSB_ADDR   87    // R/W
#define REG_CDC8_DAC_C44_MSB_ADDR   88    // R/W
#define REG_CDC8_DAC_C44_LSB_ADDR   89    // R/W
#define REG_CDC8_DAC_C45_MSB_ADDR   90    // R/W
#define REG_CDC8_DAC_C45_LSB_ADDR   91    // R/W
#define REG_CDC8_DAC_C46_MSB_ADDR   92    // R/W
#define REG_CDC8_DAC_C46_LSB_ADDR   93    // R/W
#define REG_CDC8_DAC_C47_MSB_ADDR   94    // R/W
#define REG_CDC8_DAC_C47_LSB_ADDR   95    // R/W
#define REG_CDC8_DAC_C48_MSB_ADDR   96    // R/W
#define REG_CDC8_DAC_C48_LSB_ADDR   97    // R/W
#define REG_CDC8_DAC_C49_MSB_ADDR   98    // R/W
#define REG_CDC8_DAC_C49_LSB_ADDR   99    // R/W
#define REG_CDC8_DAC_C50_MSB_ADDR   100   // R/W
#define REG_CDC8_DAC_C50_LSB_ADDR   101   // R/W
#define REG_CDC8_DAC_C51_MSB_ADDR   102   // R/W
#define REG_CDC8_DAC_C51_LSB_ADDR   103   // R/W
#define REG_CDC8_DAC_C52_MSB_ADDR   104   // R/W
#define REG_CDC8_DAC_C52_LSB_ADDR   105   // R/W
#define REG_CDC8_DAC_C53_MSB_ADDR   106   // R/W
#define REG_CDC8_DAC_C53_LSB_ADDR   107   // R/W
#define REG_CDC8_DAC_C54_MSB_ADDR   108   // R/W
#define REG_CDC8_DAC_C54_LSB_ADDR   109   // R/W
#define REG_CDC8_DAC_C55_MSB_ADDR   110   // R/W
#define REG_CDC8_DAC_C55_LSB_ADDR   111   // R/W
#define REG_CDC8_DAC_C56_MSB_ADDR   112   // R/W
#define REG_CDC8_DAC_C56_LSB_ADDR   113   // R/W
#define REG_CDC8_DAC_C57_MSB_ADDR   114   // R/W
#define REG_CDC8_DAC_C57_LSB_ADDR   115   // R/W
#define REG_CDC8_DAC_C58_MSB_ADDR   116   // R/W
#define REG_CDC8_DAC_C58_LSB_ADDR   117   // R/W
#define REG_CDC8_DAC_C59_MSB_ADDR   118   // R/W
#define REG_CDC8_DAC_C59_LSB_ADDR   119   // R/W
#define REG_CDC8_DAC_C60_MSB_ADDR   120   // R/W
#define REG_CDC8_DAC_C60_LSB_ADDR   121   // R/W
#define REG_CDC8_DAC_C61_MSB_ADDR   122   // R/W
#define REG_CDC8_DAC_C61_LSB_ADDR   123   // R/W
#define REG_CDC8_DAC_C62_MSB_ADDR   124   // R/W
#define REG_CDC8_DAC_C62_LSB_ADDR   125   // R/W

// page 9 (DAC coefficients 2)
#define REG_CDC9_DAC_C65_MSB_ADDR   2     // R/W
#define REG_CDC9_DAC_C65_LSB_ADDR   3     // R/W
#define REG_CDC9_DAC_C66_MSB_ADDR   4     // R/W
#define REG_CDC9_DAC_C66_LSB_ADDR   5     // R/W
#define REG_CDC9_DAC_C67_MSB_ADDR   6     // R/W
#define REG_CDC9_DAC_C67_LSB_ADDR   7     // R/W
#define REG_CDC9_DAC_C68_MSB_ADDR   8     // R/W
#define REG_CDC9_DAC_C68_LSB_ADDR   9     // R/W
#define REG_CDC9_DAC_C69_MSB_ADDR   10    // R/W
#define REG_CDC9_DAC_C69_LSB_ADDR   11    // R/W
#define REG_CDC9_DAC_C70_MSB_ADDR   12    // R/W
#define REG_CDC9_DAC_C70_LSB_ADDR   13    // R/W
#define REG_CDC9_DAC_C71_MSB_ADDR   14    // R/W
#define REG_CDC9_DAC_C71_LSB_ADDR   15    // R/W
#define REG_CDC9_DAC_C72_MSB_ADDR   16    // R/W
#define REG_CDC9_DAC_C72_LSB_ADDR   17    // R/W
#define REG_CDC9_DAC_C73_MSB_ADDR   18    // R/W
#define REG_CDC9_DAC_C73_LSB_ADDR   19    // R/W

// page 252 (buffer mode)
#define REG_CDC252_TP_BUF_MSB_ADDR  1     // R/W
#define REG_CDC252_TP_BUF_LSB_ADDR  2     // R/W


#define REG_CDC4_ADC_COEF_NUM       (REG_CDC4_ADC_C31_LSB_ADDR - REG_CDC4_ADC_C1_MSB_ADDR + 1)
#define REG_CDC8_DAC_COEF_NUM       (REG_CDC8_DAC_C62_LSB_ADDR - REG_CDC8_DAC_C1_MSB_ADDR + 1)
#define REG_CDC9_DAC_COEF_NUM       (REG_CDC9_DAC_C73_LSB_ADDR - REG_CDC9_DAC_C65_MSB_ADDR + 1)


//---------------- each register spec
// page 0 (serialIF and digital control)
//---- CDC0_RST (reg1)
#define CDC0_RST_E                  (1<< 0)

//---- CDC0_REV_ID (reg3)
#define CDC0_REV_ID_SHIFT           4
#define CDC0_REV_ID_MASK            (0x07<< CDC0_REV_ID_SHIFT)

//---- CDC0_CLK_GEN (reg4)
#define CDC0_CLK_GEN_SHIFT          0
#define CDC0_CLK_GEN_MASK           (0x3<< CDC0_CLK_GEN_SHIFT)

//---- CDC0_PLL_P_R (reg5)
#define CDC0_PLL_P_R_MUL_SHIFT      0
#define CDC0_PLL_P_R_MUL_MASK       (0xf<< CDC0_PLL_P_R_MUL_SHIFT)
#define CDC0_PLL_P_R_DIV_SHIFT      4
#define CDC0_PLL_P_R_DIV_MASK       (0x7<< CDC0_PLL_P_R_DIV_SHIFT)
#define CDC0_PLL_P_R_PWR            (1<< 7)

//---- CDC0_PLL_J (reg6)
#define CDC0_PLL_J_SHIFT            0
#define CDC0_PLL_J_MASK             (0x3f<< CDC0_PLL_J_SHIFT)

//---- CDC0_PLL_D_MSB (reg7)
#define CDC0_PLL_D_MSB_SHIFT        0
#define CDC0_PLL_D_MSB_MASK         (0x3f<< CDC0_PLL_D_MSB_SHIFT)

//---- CDC0_PLL_D_LSB (reg8)
#define CDC0_PLL_D_LSB_SHIFT        0
#define CDC0_PLL_D_LSB_MASK         (0xff<< CDC0_PLL_D_LSB_SHIFT)

//---- CDC0_NDAC_DIV (reg11)
#define CDC0_NDAC_DIV_SHIFT         0
#define CDC0_NDAC_DIV_MASK          (0x7f<< CDC0_NDAC_DIV_SHIFT)
#define CDC0_NDAC_DIV_PWR           (1<< 7)

//---- CDC0_MDAC_DIV (reg12)
#define CDC0_MDAC_DIV_SHIFT         0
#define CDC0_MDAC_DIV_MASK          (0x7f<< CDC0_MDAC_DIV_SHIFT)
#define CDC0_MDAC_DIV_PWR           (1<< 7)

//---- CDC0_NADC_DIV (reg18)
#define CDC0_NADC_DIV_SHIFT         0
#define CDC0_NADC_DIV_MASK          (0x7f<< CDC0_NADC_DIV_SHIFT)
#define CDC0_NADC_DIV_PWR           (1<< 7)

//---- CDC0_MADC_DIV (reg19)
#define CDC0_MADC_DIV_SHIFT         0
#define CDC0_MADC_DIV_MASK          (0x7f<< CDC0_MADC_DIV_SHIFT)
#define CDC0_MADC_DIV_PWR           (1<< 7)

//---- CDC0_AUD_IF (reg27)
#define CDC0_AUD_IF_TRISTATE_E      (1<< 0)
#define CDC0_AUD_IF_BITS_SHIFT      4
#define CDC0_AUD_IF_BITS_MASK       (0x3<< CDC0_AUD_IF_BITS_SHIFT)
#define CDC0_AUD_IF_FMT_SHIFT       6
#define CDC0_AUD_IF_FMT_MASK        (0x3<< CDC0_AUD_IF_FMT_SHIFT)

//---- CDC0_I2C_CTL (reg34)


//---- CDC0_ADC_STAT (reg36)
#define CDC0_ADC_STAT_MAX_GAIN      (1<< 5)
#define CDC0_ADC_STAT_PWR           (1<< 6)
#define CDC0_ADC_STAT_PRG_GAIN      (1<< 7)

//---- CDC0_DAC_STAT1 (reg37)
#define CDC0_DAC_STAT1_SP_PWR_R     (1<< 0)
#define CDC0_DAC_STAT1_HP_PWR_R     (1<< 1)
#define CDC0_DAC_STAT1_DAC_PWR_R    (1<< 3)
#define CDC0_DAC_STAT1_SP_PWR_L     (1<< 4)
#define CDC0_DAC_STAT1_HP_PWR_L     (1<< 5)
#define CDC0_DAC_STAT1_DAC_PWR_L    (1<< 7)

//---- CDC0_DAC_STAT2 (reg38)
#define CDC0_DAC_STAT2_PRG_GAIN_R   (1<< 0)
#define CDC0_DAC_STAT2_PRG_GAIN_L   (1<< 4)

//---- CDC0_OVFL (reg39)
#define CDC0_OVFL_ADC               (1<< 3)
#define CDC0_OVFL_DAC_R             (1<< 6)
#define CDC0_OVFL_DAC_L             (1<< 7)

//---- CDC0_DAC_WARN (reg44)
#define CDC0_DAC_WARN_HP_SHTC_R     (1<< 6)
#define CDC0_DAC_WARN_HP_SHTC_L     (1<< 7)

//---- CDC0_ADC_WARN (reg45)
#define CDC0_ADC_WARN_SIGPWR_LOW    (1<< 6)

//---- CDC0_PIN_CTL1 (reg57)
#define CDC0_PIN_CTL1_VCNT5_SHIFT   5
#define CDC0_PIN_CTL1_VCNT5_MASK    (3 << CDC0_PIN_CTL1_VCNT5_SHIFT)
#define CDC0_PIN_CTL1_VCNT5_E       (3 << CDC0_PIN_CTL1_VCNT5_SHIFT)
#define CDC0_PIN_CTL1_SPHP_SHIFT    1
#define CDC0_PIN_CTL1_SPHP_MASK     (3 << CDC0_PIN_CTL1_SPHP_SHIFT)
#define CDC0_PIN_CTL1_SPHP_E        (3 << CDC0_PIN_CTL1_SPHP_SHIFT)

//---- CDC0_PIN_CTL2 (reg58)
#define CDC0_PIN_CTL2_PMOFF_SHIFT   5
#define CDC0_PIN_CTL2_PMOFF_MASK    (3 << CDC0_PIN_CTL2_PMOFF_SHIFT)
#define CDC0_PIN_CTL2_PMOFF_E       (3 << CDC0_PIN_CTL2_PMOFF_SHIFT)

//---- CDC0_DAC_OPSET (reg60)
#define CDC0_DAC_OPSET_SHIFT        0
#define CDC0_DAC_OPSET_MASK         (0x1f<< CDC0_DAC_OPSET_SHIFT)

//---- CDC0_ADC_OPSET (reg61)
#define CDC0_ADC_OPSET_SHIFT        0
#define CDC0_ADC_OPSET_MASK         (0x1f<< CDC0_ADC_OPSET_SHIFT)

//---- CDC0_DIG_PATH (reg63)
#define CDC0_DIG_PATH_VOLSTEP_SHIFT 0
#define CDC0_DIG_PATH_VOLSTEP_MASK  (0x3<< CDC0_DIG_PATH_VOLSTEP_SHIFT)
#define CDC0_DIG_PATH_R_SHIFT       2
#define CDC0_DIG_PATH_R_MASK        (0x3<< CDC0_DIG_PATH_R_SHIFT)
#define CDC0_DIG_PATH_L_SHIFT       4
#define CDC0_DIG_PATH_L_MASK        (0x3<< CDC0_DIG_PATH_L_SHIFT)
#define CDC0_DIG_PATH_CH_PWR_R      (1<< 6)
#define CDC0_DIG_PATH_CH_PWR_L      (1<< 7)

//---- CDC0_DIG_VOL_M (reg64)
#define CDC0_DIG_VOL_M_E_SHIFT      0
#define CDC0_DIG_VOL_M_E_MASK       (0x3<< CDC0_DIG_VOL_M_E_SHIFT)
#define CDC0_DIG_VOL_M_MUTE_R       (1<< 2)
#define CDC0_DIG_VOL_M_MUTE_L       (1<< 3)

//---- CDC0_DIG_VOL_L
#define CDC0_DIG_VOL_L_SHIFT        0
#define CDC0_DIG_VOL_L_MASK         (0xff<< CDC0_DIG_VOL_L_SHIFT)

//---- CDC0_DIG_VOL_R
#define CDC0_DIG_VOL_R_SHIFT        0
#define CDC0_DIG_VOL_R_MASK         (0xff<< CDC0_DIG_VOL_R_SHIFT)

//---- CDC0_BP_CTL_L
#define CDC0_BP_CTL_L_VOL_SHIFT     0
#define CDC0_BP_CTL_L_VOL_MASK      (0x3f<< CDC0_BP_CTL_L_VOL_SHIFT)
#define CDC0_BP_CTL_L_TP_AUTO_E     (1<< 6)
#define CDC0_BP_CTL_L_E             (1<< 7)

//---- CDC0_BP_CTL_R
#define CDC0_BP_CTL_R_VOL_SHIFT     0
#define CDC0_BP_CTL_R_VOL_MASK      (0x3f<< CDC0_BP_CTL_R_VOL_SHIFT)
#define CDC0_BP_CTL_R_TP_AUTO_E     (1<< 6)
#define CDC0_BP_CTL_R_E             (1<< 7)

//---- CDC0_ADC_PWR_STEP (reg81)
#define CDC0_ADC_PWR_STEP_ST_SHIFT  0
#define CDC0_ADC_PWR_STEP_ST_MASK   (0x3<< CDC0_ADC_PWR_STEP_ST_SHIFT)
#define CDC0_ADC_PWR_STEP_PWRUP     (1<< 7)
#define CDC0_ADC_PWR_STEP_PWRDN     0

//---- CDC0_ADC_MUTE (reg82)
#define CDC0_ADC_MUTE_E             (1<< 7)
#define CDC0_ADC_MUTE_D             0

//---- CDC0_AGC_CTL1 (reg86)
#define CDC0_AGC_CTL1_GAIN_SHIFT    4
#define CDC0_AGC_CTL1_GAIN_MASK     (0x7<< CDC0_AGC_CTL1_GAIN_SHIFT)
#define CDC0_AGC_CTL1_E             (1<< 7)
#define CDC0_AGC_CTL1_D             0
#define CDC0_AGC_CTL1_MINUS_5_5DB   (0<< CDC0_AGC_CTL1_GAIN_SHIFT)
#define CDC0_AGC_CTL1_MINUS_8DB     (1<< CDC0_AGC_CTL1_GAIN_SHIFT)
#define CDC0_AGC_CTL1_MINUS_10DB    (2<< CDC0_AGC_CTL1_GAIN_SHIFT)
#define CDC0_AGC_CTL1_MINUS_12DB    (3<< CDC0_AGC_CTL1_GAIN_SHIFT)
#define CDC0_AGC_CTL1_MINUS_14DB    (4<< CDC0_AGC_CTL1_GAIN_SHIFT)
#define CDC0_AGC_CTL1_MINUS_17DB    (5<< CDC0_AGC_CTL1_GAIN_SHIFT)
#define CDC0_AGC_CTL1_MINUS_20DB    (6<< CDC0_AGC_CTL1_GAIN_SHIFT)
#define CDC0_AGC_CTL1_MINUS_24DB    (7<< CDC0_AGC_CTL1_GAIN_SHIFT)
#define CDC0_AGC_CTL1_DEFAULT_GAIN  CDC0_AGC_CTL1_MINUS_5_5DB

//---- CDC0_AGC_CTL2 (reg87)
#define CDC0_AGC_CTL2_CLIP_ST_E     (1<< 0)
#define CDC0_AGC_CTL2_NSE_THD_SHIFT 1
#define CDC0_AGC_CTL2_NSE_THD_MASK  (0x1f<< CDC0_AGC_CTL2_NSE_THD_SHIFT)
#define CDC0_AGC_CTL2_HYST_SHIFT    6
#define CDC0_AGC_CTL2_HYST_MASK     (0x3<< CDC0_AGC_CTL2_HYST_SHIFT)
#define CDC0_AGC_CTL2_HYST_D        (3<< CDC0_AGC_CTL2_HYST_SHIFT)
#define CDC0_AGC_CTL2_HYST_1DB      (0<< CDC0_AGC_CTL2_HYST_SHIFT)
#define CDC0_AGC_CTL2_HYST_2DB      (1<< CDC0_AGC_CTL2_HYST_SHIFT)
#define CDC0_AGC_CTL2_HYST_4DB      (2<< CDC0_AGC_CTL2_HYST_SHIFT)

//---- CDC0_AGC_MAX_GAIN
#define CDC0_AGC_MAX_GAIN_SHIFT     0
#define CDC0_AGC_MAX_GAIN_MASK      (0x7f<< CDC0_AGC_MAX_GAIN_SHIFT)

//---- CDC0_AGC_ATTCK_TM
#define CDC0_AGC_ATTCK_TM_MUL_SHIFT 0
#define CDC0_AGC_ATTCK_TM_MUL_MASK  (0x7<< CDC0_AGC_ATTCK_TM_MUL_SHIFT)
#define CDC0_AGC_ATTCK_TM_SHIFT     3
#define CDC0_AGC_ATTCK_TM_MASK      (0x1f<< CDC0_AGC_ATTCK_TM_SHIFT)

//---- CDC0_AGC_DECAY_TM
#define CDC0_AGC_DECAY_TM_MUL_SHIFT 0
#define CDC0_AGC_DECAY_TM_MUL_MASK  (0x7<< CDC0_AGC_DECAY_TM_MUL_SHIFT)
#define CDC0_AGC_DECAY_TM_SHIFT     3
#define CDC0_AGC_DECAY_TM_MASK      (0x1f<< CDC0_AGC_DECAY_TM_SHIFT)

//---- CDC0_AGC_NSE_DBNC
#define CDC0_AGC_NSE_DBNC_SHIFT     0
#define CDC0_AGC_NSE_DBNC_MASK      (0x1f<< CDC0_AGC_NSE_DBNC_SHIFT)

//---- CDC0_AGC_SIG_DBNC
#define CDC0_AGC_SIG_DBNC_SHIFT     0
#define CDC0_AGC_SIG_DBNC_MASK      (0xf<< CDC0_AGC_SIG_DBNC_SHIFT)

//---- CDC0_AGC_GAIN_STAT
#define CDC0_AGC_GAIN_STAT_SHIFT    0
#define CDC0_AGC_GAIN_STAT_MASK     (0xff<< CDC0_AGC_GAIN_STAT_SHIFT)

//---- CDC0_VOLADC_CTL
#define CDC0_VOLADC_CTL_RATE_SHIFT  0
#define CDC0_VOLADC_CTL_RATE_MASK   (0x7<< CDC0_VOLADC_CTL_RATE_SHIFT)
#define CDC0_VOLADC_CTL_HYST_SHIFT  4
#define CDC0_VOLADC_CTL_HYST_MASK   (0x3<< CDC0_VOLADC_CTL_HYST_SHIFT)
#define CDC0_VOLADC_CTL_CLK_SRC     (1<< 6)
#define CDC0_VOLADC_CTL_E           (1<< 7)

//---- CDC0_VOLADC_STAT
#define CDC0_VOLADC_STAT_GAIN_SHIFT 0
#define CDC0_VOLADC_STAT_GAIN_MASK  (0x7f<< CDC0_VOLADC_STAT_GAIN_SHIFT)


// page 1 (analog I/O control)
//---- CDC1_HP_DRV (reg31)
#define CDC1_HP_DRV_WARN_SHTC       (1<< 0)  // RO
#define CDC1_HP_DRV_SHTC_PDN_E      (1<< 1)
#define CDC1_HP_DRV_SHTC_PROTECT_E  (1<< 2)  // default
#define CDC1_HP_DRV_OVLT_SHIFT      3
#define CDC1_HP_DRV_OVLT_MASK       (0x3<< CDC1_HP_DRV_OVLT_SHIFT)
#define CDC1_HP_CMN_MODE_VOL_1_35V  (0<< CDC1_HP_DRV_OVLT_SHIFT)
#define CDC1_HP_CMN_MODE_VOL_1_50V  (1<< CDC1_HP_DRV_OVLT_SHIFT)
#define CDC1_HP_CMN_MODE_VOL_1_65V  (2<< CDC1_HP_DRV_OVLT_SHIFT)
#define CDC1_HP_CMN_MODE_VOL_1_80V  (3<< CDC1_HP_DRV_OVLT_SHIFT)
#define CDC1_HP_DRV_PWR_R           (1<< 6)
#define CDC1_HP_DRV_PWR_L           (1<< 7)

//---- CDC1_SP_DRV (reg32)
#define CDC1_SP_DRV_WARN_SHTC       (1<< 0)
#define CDC1_SP_DRV_SHTC_PDN_E      (1<< 1)  // default
#define CDC1_SP_DRV_SHTC_PROTECT_E  (1<< 2)  // default
#define CDC1_SP_DRV_PWR_R           (1<< 6)
#define CDC1_SP_DRV_PWR_L           (1<< 7)

//---- CDC1_HP_DRV_TM (reg33)
#define CDC1_HP_DRV_TM_WEAK_OV_SRC  (1<< 0)
#define CDC1_HP_DRV_TM_RAMPUP_SHIFT 1
#define CDC1_HP_DRV_TM_RAMPUP_MASK  (0x3<< CDC1_HP_DRV_TM_RAMPUP_SHIFT)
#define CDC1_HP_DRV_TM_PWON_SHIFT   3
#define CDC1_HP_DRV_TM_PWON_MASK    (0x7<< CDC1_HP_DRV_TM_PWON_SHIFT)

// power-on time
#define CDC_HP_DRV_PWON_TM_0US      ( 0<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_10US     ( 1<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_100US    ( 2<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_1MS      ( 3<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_10MS     ( 4<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_50MS     ( 5<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_100MS    ( 6<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_200MS    ( 7<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_400MS    ( 8<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_800MS    ( 9<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_2S       (10<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_4S       (11<<CDC1_HP_DRV_TM_PWON_SHIFT)
#define CDC_HP_DRV_PWON_TM_DEFAULT  CDC_HP_DRV_PWON_TM_0US

// ramp-up step time
#define CDC_HP_DRV_RAMPUP_TM_0MS        ( 0<<CDC1_HP_DRV_TM_RAMPUP_SHIFT)
#define CDC_HP_DRV_RAMPUP_TM_1MS        ( 1<<CDC1_HP_DRV_TM_RAMPUP_SHIFT)
#define CDC_HP_DRV_RAMPUP_TM_2MS        ( 2<<CDC1_HP_DRV_TM_RAMPUP_SHIFT)
#define CDC_HP_DRV_RAMPUP_TM_4MS        ( 3<<CDC1_HP_DRV_TM_RAMPUP_SHIFT)
#define CDC_HP_DRV_RAMPUP_TM_DEFAULT    CDC_HP_DRV_RAMPUP_TM_0MS

//---- CDC1_HPSP_RAMPDWN (reg34)
#define CDC1_HPSP_RAMPDWN_TM_SHIFT  0
#define CDC1_HPSP_RAMPDWN_TM_MASK   (0xf<< CDC1_HPSP_RAMPDWN_TM_SHIFT)

// ramp-down step time
#define CDC_HPSP_DRV_RAMPDWN_TM_0_12MS      ( 0<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_0_48MS      ( 1<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_0_952MS     ( 2<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_3_808MS     ( 3<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_6_664MS     ( 4<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_7_616MS     ( 5<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_9_520MS     ( 6<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_11_424MS    ( 7<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_15_232MS    ( 8<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_19_040MS    ( 9<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_22_848MS    (10<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_30_464MS    (11<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_38_080MS    (12<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_45_696MS    (13<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_76_160MS    (14<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_118_048MS   (15<<CDC1_HPSP_RAMPDWN_TM_SHIFT)
#define CDC_HPSP_DRV_RAMPDWN_TM_DEFAULT     CDC_HPSP_DRV_RAMPDWN_TM_3_808MS

//---- CDC1_DAC_OUTPUT (reg35)
#define CDC1_DAC_OUTPUT_E_R         (1<< 2)
#define CDC1_DAC_OUTPUT_E_L         (1<< 6)

//---- CDC1_HP_ANGVOL_L (reg36)
#define CDC1_HP_ANGVOL_L_GAIN_SHIFT 0
#define CDC1_HP_ANGVOL_L_GAIN_MASK  (0x7f<< CDC1_HP_ANGVOL_L_GAIN_SHIFT)
#define CDC1_HP_ANGVOL_L_E          (1<< 7)

//---- CDC1_HP_ANGVOL_R (reg37)
#define CDC1_HP_ANGVOL_R_GAIN_SHIFT 0
#define CDC1_HP_ANGVOL_R_GAIN_MASK  (0x7f<< CDC1_HP_ANGVOL_R_GAIN_SHIFT)
#define CDC1_HP_ANGVOL_R_E          (1<< 7)

//---- CDC1_SP_ANGVOL_L (reg38)
#define CDC1_SP_ANGVOL_L_GAIN_SHIFT 0
#define CDC1_SP_ANGVOL_L_GAIN_MASK  (0x7f<< CDC1_SP_ANGVOL_L_GAIN_SHIFT)
#define CDC1_SP_ANGVOL_L_E          (1<< 7)

//---- CDC1_SP_ANGVOL_R (reg39)
#define CDC1_SP_ANGVOL_R_GAIN_SHIFT 0
#define CDC1_SP_ANGVOL_R_GAIN_MASK  (0x7f<< CDC1_SP_ANGVOL_R_GAIN_SHIFT)
#define CDC1_SP_ANGVOL_R_E          (1<< 7)

// Analog Volume Utility Definition
#define CDC1_ANGVOL_E               0x80
#define CDC1_ANGVOL_D               0
#define CDC1_ANGVOL_GAIN_MUTE       0x7f
#define CDC1_ANGVOL_GAIN_MAX        0x00

//---- CDC1_HP_DRV_L (reg40), CDC1_HP_DRV_R (reg41)
#define CDC1_HP_DRV_APPLIED       (1<< 0)  // RO
#define CDC1_HP_DRV_PDN_TRISTATE  (1<< 1)  // default
#define CDC1_HP_DRV_MUTEN         (1<< 2)
#define CDC1_HP_DRV_GAIN_SHIFT    3
#define CDC1_HP_DRV_GAIN_MASK     (0xf<< CDC1_HP_DRV_GAIN_SHIFT)

//---- CDC1_SP_DRV_L (reg42), CDC1_SP_DRV_R (reg43)
#define CDC1_SP_DRV_GAIN_APPLIED  (1<< 0)  // RO
#define CDC1_SP_DRV_MUTEN         (1<< 2)
#define CDC1_SP_DRV_GAIN_SHIFT    3
#define CDC1_SP_DRV_GAIN_MASK     (0xf<< CDC1_SP_DRV_GAIN_SHIFT)
#define CDC1_SP_DRV_GAIN_0DB      (0<< CDC1_SP_DRV_GAIN_SHIFT)
#define CDC1_SP_DRV_GAIN_6DB      (1<< CDC1_SP_DRV_GAIN_SHIFT)
#define CDC1_SP_DRV_GAIN_12DB     (2<< CDC1_SP_DRV_GAIN_SHIFT)
#define CDC1_SP_DRV_GAIN_18DB     (3<< CDC1_SP_DRV_GAIN_SHIFT)

//---- CDC1_MIC_BIAS (reg46)
#define CDC1_MIC_BIAS_SHIFT         0
#define CDC1_MIC_BIAS_MASK          (0x3<< CDC1_MIC_BIAS_SHIFT)
#define CDC1_MIC_BIAS_PWRDN         0
#define CDC1_MIC_BIAS_2_0V          1
#define CDC1_MIC_BIAS_2_5V          2
#define CDC1_MIC_BIAS_AVDD          3

//---- CDC1_MIC_ADC_PGA
#define CDC1_MIC_ADC_PGA_GAIN_SHIFT 0
#define CDC1_MIC_ADC_PGA_GAIN_MASK  (0x7f<< CDC1_MIC_ADC_PGA_GAIN_SHIFT)
#define CDC1_MIC_ADC_PGA_MUTE       (1<< 7)  // default

//---- CDC1_MIC_PGA_P
#define CDC1_MIC_PGA_P_I_SHIFT      6
#define CDC1_MIC_PGA_P_I_MASK       (0x3<< CDC1_MIC_PGA_P_I_SHIFT)

//---- CDC1_MIC_PGA_M
#define CDC1_MIC_PGA_M_I_SHIFT      6
#define CDC1_MIC_PGA_M_I_MASK       (0x3<< CDC1_MIC_PGA_M_I_SHIFT)

//---- CDC1_MIC_INPUT
#define CDC1_MIC_INPUT_GAIN_APPLIED (1<< 0)  // RO
#define CDC1_MIC_INPUT_CM_CONNECT   (1<< 7)


// page 3 (touch-panel(TCS) control)
//---- CDC3_TP_ADC_CTL
#define CDC3_TP_ADC_CTL_DIV_SHIFT   3
#define CDC3_TP_ADC_CTL_DIV_MASK    (0x3<< CDC3_TP_ADC_CTL_DIV_SHIFT)
#define CDC3_TP_ADC_CTL_BITS_SHIFT  5
#define CDC3_TP_ADC_CTL_BITS_MASK   (0x3<< CDC3_TP_ADC_CTL_BITS_SHIFT)
#define CDC3_TP_ADC_CTL_PDN         (1<< 7)

//---- CDC3_TP_CONV_MODE
#define CDC3_TP_CONV_MODE_IM_SHIFT  0
#define CDC3_TP_CONV_MODE_IM_MASK   (0x3<< CDC3_TP_CONV_MODE_IM_SHIFT)
#define CDC3_TP_CONV_MODE_CM_SHIFT  2
#define CDC3_TP_CONV_MODE_CM_MASK   (0xf<< CDC3_TP_CONV_MODE_CM_SHIFT)
#define CDC3_TP_CONV_MODE_SELF      (1<< 7)

//---- CDC3_TP_PRECHG_SNS
#define CDC3_TP_PRECHG_SNS_TS_SHIFT 0
#define CDC3_TP_PRECHG_SNS_TS_MASK  (0x7<< CDC3_TP_PRECHG_SNS_TS_SHIFT)
#define CDC3_TP_PRECHG_SNS_TP_SHIFT 4
#define CDC3_TP_PRECHG_SNS_TP_MASK  (0x7<< CDC3_TP_PRECHG_SNS_TP_SHIFT)
#define CDC3_TP_PRECHG_SNS_DIS      (1<< 7)

//---- CDC3_TP_VLT_STB
#define CDC3_TP_VLT_STB_TM_SHIFT    0
#define CDC3_TP_VLT_STB_TM_MASK     (0x7<< CDC3_TP_VLT_STB_TM_SHIFT)

//---- CDC3_TP_STAT1
#define CDC3_TP_STAT1_Y_RDY         (1<< 2)
#define CDC3_TP_STAT1_X_RDY         (1<< 3)
#define CDC3_TP_STAT1_DATA_RDY      (1<< 5)
#define CDC3_TP_STAT1_ADC_RDY       (1<< 6)  // default
#define CDC3_TP_STAT1_TOUCH         (1<< 7)

//---- CDC3_TP_STAT2
#define CDC3_TP_STAT2_IN3_RDY       (1<< 5)
#define CDC3_TP_STAT2_IN2_RDY       (1<< 6)
#define CDC3_TP_STAT2_IN1_RDY       (1<< 7)

//---- CDC3_TP_NBUF_MODE
#define CDC3_TP_NBUF_MODE_FLUSHED   (1<< 0)  // RO
#define CDC3_TP_NBUF_MODE_DATA_RDYN (1<< 1)  // RO
#define CDC3_TP_NBUF_MODE_HOLDOFF   (1<< 2)  // default
#define CDC3_TP_NBUF_MODE_LEN_SHIFT 0
#define CDC3_TP_NBUF_MODE_LEN_MASK  (0x7<< CDC3_TP_NBUF_MODE_LEN_SHIFT)
#define CDC3_TP_NBUF_MODE_ONESHOT   (1<< 6)
#define CDC3_TP_NBUF_MODE_E         (1<< 7)

//---- CDC3_TP_DELAY
#define CDC3_TP_DELAY_AUX_TM_SHIFT  0
#define CDC3_TP_DELAY_AUX_TM_MASK   (0x7<< CDC3_TP_DELAY_AUX_TM_SHIFT)
#define CDC3_TP_DELAY_AUX_E         (1<< 3)
#define CDC3_TP_DELAY_TP_TM_SHIFT   0
#define CDC3_TP_DELAY_TP_TM_MASK    (0x7<< CDC3_TP_DELAY_TP_TM_SHIFT)
#define CDC3_TP_DELAY_TP_E          (1<< 7)

//---- CDC3_TP_DELAY_CLK
#define CDC3_TP_DELAY_CLK_DIV_SHIFT 0
#define CDC3_TP_DELAY_CLK_DIV_MASK  (0x7f<< CDC3_TP_DELAY_CLK_DIV_SHIFT)
#define CDC3_TP_DELAY_CLK_SRC       (1<< 7)

//---- CDC3_TP_ADC_RST
#define CDC3_TP_ADC_RST_DIV_SHIFT   0
#define CDC3_TP_ADC_RST_DIV_MASK    (0x7f<< CDC3_TP_ADC_RST_DIV_SHIFT)
#define CDC3_TP_ADC_RST_CLK_SRC     (1<< 7)

//---- CDC3_TP_DBNC_TM
#define CDC3_TP_DBNC_TM_SHIFT       0
#define CDC3_TP_DBNC_TM_MASK        (0x7<< CDC3_TP_DBNC_TM_SHIFT)

//---- CDC3_TP_AUTO_AUX
#define CDC3_TP_AUTO_AUX_IN3_E      (1<< 5)
#define CDC3_TP_AUTO_AUX_IN2_E      (1<< 6)
#define CDC3_TP_AUTO_AUX_IN1_E      (1<< 7)

#if 0
// page 0 (serialIF and digital control)
//---- CDC0_CLK_GEN
typedef enum
{
    CDC0_CLK_GEN_MCLK = 0,
    CDC0_CLK_GEN_PLL  = 3   // default
}
CDCClockGen;

//---- CDC0_PLL_P_R
typedef enum
{
    CDC0_PLL_DIV_MIN = 1,
    CDC0_PLL_DIV_MAX = 8,
    CDC0_PLL_DIV_DEFAULT = 2  // default
}
CDCPllDivider;

typedef enum
{
    CDC0_PLL_MUL_R_MIN = 1,  // default
    CDC0_PLL_MUL_R_MAX = 16,
    CDC0_PLL_MUL_R_DEFAULT= CDC0_PLL_MUL_R_MIN
}
CDCPllMultiplierR;

//---- CDC0_PLL_J
typedef enum
{
    CDC0_PLL_J_MIN = 1,
    CDC0_PLL_J_MAX = 63,
    CDC0_PLL_J_DEFAULT = 15  // default
}
CDCPllMultiplierJ;

//---- CDC0_PLL_D_MSB / CDC0_PLL_D_LSB
typedef enum
{
    CDC0_PLL_D_MIN = 0,  // default
    CDC0_PLL_D_MAX = 9999,
    CDC0_PLL_D_DEFAULT = CDC0_PLL_D_MIN
}
CDCPllFracMultiplierD;

//---- CDC0_NDAC_DIV
typedef enum
{
    CDC0_NDAC_DIV_MIN = 1,
    CDC0_NDAC_DIV_MAX = 128,
    CDC0_NDAC_DIV_DEFAULT = 5  // default
}
CDCNDacValue;

//---- CDC0_MDAC_DIV
typedef enum
{
    CDC0_MDAC_DIV_MIN = 1,
    CDC0_MDAC_DIV_MAX = 128,
    CDC0_MDAC_DIV_DEFAULT = 3  // default
}
CDCMDacValue;

//---- CDC0_NADC_DIV
typedef enum
{
    CDC0_NADC_DIV_MIN = 1,
    CDC0_NADC_DIV_MAX = 128,
    CDC0_NADC_DIV_DEFAULT = 5  // default
}
CDCNAdcValue;

//---- CDC0_MADC_DIV
typedef enum
{
    CDC0_MADC_DIV_MIN = 1,
    CDC0_MADC_DIV_MAX = 128,
    CDC0_MADC_DIV_DEFAULT = 3  // default
}
CDCMAdcValue;

//---- CDC0_AUD_IF
typedef enum
{
    CDC0_AUD_IF_FMT_I2S = 0,  // default
    CDC0_AUD_IF_FMT_RJF = 2,
    CDC0_AUD_IF_FMT_LJF = 3
}
CDCAudioIfFormat;

typedef enum
{
    CDC0_AUD_IF_BITS_16 = 0,  // default
    CDC0_AUD_IF_BITS_20 = 1,
    CDC0_AUD_IF_BITS_24 = 2,
    CDC0_AUD_IF_BITS_32 = 3
}
CDCAudioIfBits;

//---- CDC0_DAC_OPSET
typedef enum
{
    CDC0_DAC_OPSET_MIN = 0,
    CDC0_DAC_OPSET_MAX = 0x1f,
    CDC0_DAC_OPSET_DEFAULT = 25  // default
}
CDCDacOpset;

//---- CDC0_ADC_OPSET
typedef enum
{
    CDC0_ADC_OPSET_MIN = 0,
    CDC0_ADC_OPSET_MAX = 0x1f,
    CDC0_ADC_OPSET_DEFAULT = 5  // default
}
CDCAdcOpset;

//---- CDC0_ADC_WARN
typedef enum
{
    CDC0_ADC_WARN_SIGPWR_GT = 0,  // default
    CDC0_ADC_WARN_SIGPWR_LT = CDC0_ADC_WARN_SIGPWR_LOW
}
CDCAdcWarnSignalPower;

//---- CDC0_DIG_PATH
typedef enum
{
    CDC0_DIG_PATH_OFF = 0,
    CDC0_DIG_PATH_THRU = 1,  // default
    CDC0_DIG_PATH_CROSS = 2,
    CDC0_DIG_PATH_MIX = 3
}
CDCDigitalPath;

typedef enum
{
    CDC0_DIG_PATH_VOL_STEP_F = 0,   // default
    CDC0_DIG_PATH_VOL_STEP_2F = 1,
    CDC0_DIG_PATH_VOL_STEP_DISABLE = 2
}
CDCDigitalPathVolumeStep;

//---- CDC0_DIG_VOL_M
typedef enum
{
    CDC0_DIG_VOL_M_E_LR = 0,   // default
    CDC0_DIG_VOL_M_E_R = 1,
    CDC0_DIG_VOL_M_E_L = 2,
    CDC0_DIG_VOL_M_E_DISABLE = 3
}
CDCDigitalVolumeEnable;

//---- CDC0_DIG_VOL_L / CDC0_DIG_VOL_R
typedef enum
{
    CDC0_DIG_VOL_MAX = 48,   // +24dB
    CDC0_DIG_VOL_MIN = -128, // -64dB
    CDC0_DIG_VOL_DEFAULT = 0  // default 0dB
}
CDCDigitalVolume;

//---- CDC0_BP_CTL_L / CDC0_BP_CTL_R
typedef enum
{
    CDC0_BP_VOL_MIN = 0,  //   0dB
    CDC0_BP_VOL_MAX = 63, // -63dB
    CDC0_BP_VOL_DEFAULT = 0  // default 0dB
}
CDCBeepVolume;

//---- CDC0_BP_LEN_MSB / CDC0_BP_LEN_MID / CDC0_BP_LEN_LSB
typedef enum
{
    CDC0_BP_LEN_MIN = 0,
    CDC0_BP_LEN_MAX = 0xffffff,
    CDC0_BP_LEN_DEFAULT = 0xff  // default
}
CDCBeepLength;

//---- CDC0_BP_SIN_MSB / CDC0_BP_SIN_LSB
typedef enum
{
    CDC0_BP_SIN_MIN = 0,
    CDC0_BP_SIN_MAX = 0x7fff,
    CDC0_BP_SIN_DEFAULT = 16 * 0x100 + 216  // default
}
CDCBeepSin;

//---- CDC0_BP_COS_MSB / CDC0_BP_COS_LSB
typedef enum
{
    CDC0_BP_COS_MIN = 0,
    CDC0_BP_COS_MAX = 0x7fff,
    CDC0_BP_COS_DEFAULT = 126 * 0x100 + 227  // default
}
CDCBeepCos;

//---- CDC0_ADC_PWR_STEP
typedef enum
{
    CDC0_ADC_STEP_F = 0,   // default
    CDC0_ADC_STEP_2F = 1,
    CDC0_ADC_STEP_DISABLE = 2
}
CDCAdcStep;

//---- CDC0_AGC_CTL1
typedef enum
{
    CDC0_AGC_GAIN_N5 = 0,
    CDC0_AGC_GAIN_N8 = 1,
    CDC0_AGC_GAIN_N10 = 2,
    CDC0_AGC_GAIN_N12 = 3,
    CDC0_AGC_GAIN_N14 = 4,
    CDC0_AGC_GAIN_N17 = 5,
    CDC0_AGC_GAIN_N20 = 6,
    CDC0_AGC_GAIN_N24 = 7,
    CDC0_AGC_GAIN_DEFAULT = CDC0_AGC_GAIN_N5  // default
}
CDCAgcGain;

//---- CDC0_AGC_CTL2
typedef enum
{
    CDC0_AGC_NSE_THD_MIN = 1,   // -30dB
    CDC0_AGC_NSE_THD_MAX = 31,  // -90dB
    CDC0_AGC_NSE_THD_DISABLE = 0,
    CDC0_AGC_NSE_THD_DEFAULT = CDC0_AGC_NSE_THD_DISABLE  // default
}
CDCAgcNoiseThreshold;

typedef enum
{
    CDC0_AGC_HYST_1 = 0,  // default
    CDC0_AGC_HYST_2 = 1,
    CDC0_AGC_HYST_4 = 2,
    CDC0_AGC_HYST_DISABLE = 3,
    CDC0_AGC_HYST_DEFAULT = CDC0_AGC_HYST_1
}
CDCAgcHysterysis;

//---- CDC0_AGC_MAX_GAIN
typedef enum
{
    CDC0_AGC_MAX_GAIN_MIN = 0,  // default
    CDC0_AGC_MAX_GAIN_MAX = 116, // +58dB
    CDC0_AGC_MAX_GAIN_DEFAULT = CDC0_AGC_MAX_GAIN_MAX
}
CDCAgcMaxGain;

//---- CDC0_AGC_ATTCK_TM / CDC0_AGC_DECAY_TM
typedef enum
{
    CDC0_AGC_TM_MUL_1 = 0,  // default
    CDC0_AGC_TM_MUL_2 = 1,
    CDC0_AGC_TM_MUL_4 = 2,
    CDC0_AGC_TM_MUL_8 = 3,
    CDC0_AGC_TM_MUL_DEFAULT = CDC0_AGC_TM_MUL_1
}
CDCAgcTimeMultiplier;

typedef enum
{
    CDC0_AGC_TM_MIN = 0,  // default
    CDC0_AGC_TM_MAX = 31,
    CDC0_AGC_TM_DEFAULT = CDC0_AGC_TM_MAX
}
CDCAgcTime;

//---- CDC0_AGC_NSE_DBNC
typedef enum
{
    CDC0_AGC_NSE_DBNC_0 = 0,  // default
    CDC0_AGC_NSE_DBNC_4 = 1,
    CDC0_AGC_NSE_DBNC_8 = 2,
    CDC0_AGC_NSE_DBNC_16 = 3,
    CDC0_AGC_NSE_DBNC_32 = 4,
    CDC0_AGC_NSE_DBNC_64 = 5,
    CDC0_AGC_NSE_DBNC_128 = 6,
    CDC0_AGC_NSE_DBNC_256 = 7,
    CDC0_AGC_NSE_DBNC_512 = 8,
    CDC0_AGC_NSE_DBNC_1K = 9,
    CDC0_AGC_NSE_DBNC_2K = 10,
    CDC0_AGC_NSE_DBNC_4K = 11,
    CDC0_AGC_NSE_DBNC_8K = 12,
    CDC0_AGC_NSE_DBNC_12K = 13,
    CDC0_AGC_NSE_DBNC_16K = 14,
    CDC0_AGC_NSE_DBNC_20K = 15,
    CDC0_AGC_NSE_DBNC_24K = 16,
    CDC0_AGC_NSE_DBNC_28K = 17,
    CDC0_AGC_NSE_DBNC_32K = 18,
    CDC0_AGC_NSE_DBNC_36K = 19,
    CDC0_AGC_NSE_DBNC_40K = 20,
    CDC0_AGC_NSE_DBNC_44K = 21,
    CDC0_AGC_NSE_DBNC_48K = 22,
    CDC0_AGC_NSE_DBNC_52K = 23,
    CDC0_AGC_NSE_DBNC_56K = 24,
    CDC0_AGC_NSE_DBNC_60K = 25,
    CDC0_AGC_NSE_DBNC_64K = 26,
    CDC0_AGC_NSE_DBNC_68K = 27,
    CDC0_AGC_NSE_DBNC_72K = 28,
    CDC0_AGC_NSE_DBNC_76K = 29,
    CDC0_AGC_NSE_DBNC_80K = 30,
    CDC0_AGC_NSE_DBNC_84K = 31,
    CDC0_AGC_NSE_DBNC_DEFAULT = CDC0_AGC_NSE_DBNC_0
}
CDCAgcNoiseDebounce;

//---- CDC0_AGC_SIG_DBNC
typedef enum
{
    CDC0_AGC_SIG_DBNC_0 = 0,  // default
    CDC0_AGC_SIG_DBNC_4 = 1,
    CDC0_AGC_SIG_DBNC_8 = 2,
    CDC0_AGC_SIG_DBNC_16 = 3,
    CDC0_AGC_SIG_DBNC_32 = 4,
    CDC0_AGC_SIG_DBNC_64 = 5,
    CDC0_AGC_SIG_DBNC_128 = 6,
    CDC0_AGC_SIG_DBNC_256 = 7,
    CDC0_AGC_SIG_DBNC_512 = 8,
    CDC0_AGC_SIG_DBNC_1K = 9,
    CDC0_AGC_SIG_DBNC_2K = 10,
    CDC0_AGC_SIG_DBNC_4K = 11,
    CDC0_AGC_SIG_DBNC_6K = 12,
    CDC0_AGC_SIG_DBNC_8K = 13,
    CDC0_AGC_SIG_DBNC_10K = 14,
    CDC0_AGC_SIG_DBNC_12K = 15,
    CDC0_AGC_SIG_DBNC_DEFAULT = CDC0_AGC_SIG_DBNC_0
}
CDCAgcSignalDebounce;

//---- CDC0_VOLADC_CTL
typedef enum
{
    CDC0_VOLADC_RATE_500 = 0,  // default
    CDC0_VOLADC_RATE_750 = 1,
    CDC0_VOLADC_RATE_1K = 2,
    CDC0_VOLADC_RATE_2K = 3,
    CDC0_VOLADC_RATE_3K = 4,
    CDC0_VOLADC_RATE_4K = 5,
    CDC0_VOLADC_RATE_7K = 6,
    CDC0_VOLADC_RATE_10K = 7,
    CDC0_VOLADC_RATE_DEFAULT = CDC0_VOLADC_RATE_500
}
CDCVolAdcRate;

typedef enum
{
    CDC0_VOLADC_HYST_NONE = 0,  // default
    CDC0_VOLADC_HYST_1BIT = 1,
    CDC0_VOLADC_HYST_2BIT = 2,
    CDC0_VOLADC_HYST_DEFAULT = CDC0_VOLADC_HYST_NONE
}
CDCVolAdcHyst;

typedef enum
{
    CDC0_VOLADC_CLK_INTERNAL = 0,  // default
    CDC0_VOLADC_CLK_MCLK = CDC0_VOLADC_CTL_CLK_SRC
}
CDCVolAdcClockSrc;


// page 1 (analog I/O control)
//---- CDC1_HP_DRV
typedef enum
{
    CDC1_HP_OVLT_1_35V = 0,  // default
    CDC1_HP_OVLT_1_50V = 1,
    CDC1_HP_OVLT_1_65V = 2,
    CDC1_HP_OVLT_1_80V = 3
}
CDCHpOutputVolt;

//---- CDC1_HP_DRV_TM
typedef enum
{
    CDC1_HP_RAMPUP_0MS = 0,
    CDC1_HP_RAMPUP_1MS = 1,
    CDC1_HP_RAMPUP_2MS = 2,
    CDC1_HP_RAMPUP_4MS = 3  // default
}
CDCHpRampUpTime;

typedef enum
{
    CDC1_HP_PWON_0S = 0,
    CDC1_HP_PWON_10US = 1,
    CDC1_HP_PWON_100US = 2,
    CDC1_HP_PWON_1MS = 3,
    CDC1_HP_PWON_10MS = 4,
    CDC1_HP_PWON_50MS = 5,
    CDC1_HP_PWON_100MS = 6,
    CDC1_HP_PWON_200MS = 7,  // default
    CDC1_HP_PWON_400MS = 8,
    CDC1_HP_PWON_800MS = 9,
    CDC1_HP_PWON_2S = 10,
    CDC1_HP_PWON_4S = 11
}
CDCHpPowerOnTime;

//---- CDC1_HP_ANGVOL_L / CDC1_HP_ANGVOL_H
typedef enum
{
    CDC1_HP_ANGVOL_MIN = 0,  // default
    CDC1_HP_ANGVOL_MAX = 0x7f,
    CDC1_HP_ANGVOL_DEFAULT = CDC1_HP_ANGVOL_MIN
}
CDCHpAnalogVolume;

//---- CDC1_SP_ANGVOL_L / CDC1_SP_ANGVOL_H
typedef enum
{
    CDC1_SP_ANGVOL_MIN = 0,  // default
    CDC1_SP_ANGVOL_MAX = 0x7f,
    CDC1_SP_ANGVOL_DEFAULT = CDC1_SP_ANGVOL_MIN
}
CDCSpAnalogVolume;

//---- CDC1_HPSP_RAMPDWN
typedef enum
{
    CDC1_HPSP_RAMPDWN_120US = 0,  // default
    CDC1_HPSP_RAMPDWN_480US = 1,
    CDC1_HPSP_RAMPDWN_952US = 2,
    CDC1_HPSP_RAMPDWN_3_81MS = 3,
    CDC1_HPSP_RAMPDWN_6_66MS = 4,
    CDC1_HPSP_RAMPDWN_7_62MS = 5,
    CDC1_HPSP_RAMPDWN_9_52MS = 6,
    CDC1_HPSP_RAMPDWN_11_4MS = 7,
    CDC1_HPSP_RAMPDWN_15_2MS = 8,
    CDC1_HPSP_RAMPDWN_19_0MS = 9,
    CDC1_HPSP_RAMPDWN_22_8MS = 10,
    CDC1_HPSP_RAMPDWN_30_5MS = 11,
    CDC1_HPSP_RAMPDWN_38_1MS = 12,
    CDC1_HPSP_RAMPDWN_45_7MS = 13,
    CDC1_HPSP_RAMPDWN_76_2MS = 14,
    CDC1_HPSP_RAMPDWN_118MS = 15,
    CDC1_HPSP_RAMPDWN_DEFAULT = CDC1_HPSP_RAMPDWN_120US
}
CDCHpSpRampDownTime;

//---- CDC1_HP_DRV_L / CDC1_HP_DRV_R
typedef enum
{
    CDC1_HP_DRV_GAIN_MIN = 0,  // default
    CDC1_HP_DRV_GAIN_MAX = 9,
    CDC1_HP_DRV_GAIN_DEFAULT = CDC1_HP_DRV_GAIN_MIN
}
CDCHpDrvGain;

//---- CDC1_SP_DRV_L / CDC1_SP_DRV_R
typedef enum
{
    CDC1_SP_DRV_GAIN_MIN = 0,  // default
    CDC1_SP_DRV_GAIN_MAX = 15,  // ?
    CDC1_SP_DRV_GAIN_DEFAULT = CDC1_SP_DRV_GAIN_MIN
}
CDCSpDrvGain;

//---- CDC1_MIC_BIAS
typedef enum
{
    CDC1_MIC_BIAS_PDN = 0,  // default
    CDC1_MIC_BIAS_2V = 1,
    CDC1_MIC_BIAS_2_5V = 2,
    CDC1_MIC_BIAS_AVDD = 3
}
CDCMicBias;

//---- CDC1_MIC_ADC_PGA
typedef enum
{
    CDC1_MIC_GAIN_MIN = 0,    // default 0dB
    CDC1_MIC_GAIN_MAX = 119,  // +59.5dB
    CDC1_MIC_GAIN_DEFAULT = CDC1_MIC_GAIN_MIN
}
CDCMicGain;

//---- CDC1_MIC_PGA_P / CDC1_MIC_PGA_M
typedef enum
{
    CDC1_MIC_PGA_NULL = 0,  // default
    CDC1_MIC_PGA_18_8K = 1,
    CDC1_MIC_PGA_37_8K = 2,
    CDC1_MIC_PGA_75_6K = 3
}
CDCMicPgaImpedance;


// page 3 (touch-panel(TCS) control)
//---- CDC3_TP_ADC_CTL
typedef enum
{
    CDC3_TP_ADC_CLK_DIV_1 = 0,  // default
                                // (for 8bit only)
    CDC3_TP_ADC_CLK_DIV_2 = 1,  // (for 8bit/10bit only)
    CDC3_TP_ADC_CLK_DIV_4 = 2,
    CDC3_TP_ADC_CLK_DIV_8 = 3
}
CDCTpAdcClockDivider;

typedef enum
{
    CDC3_TP_ADC_BITS_12 = 0,  // default
    CDC3_TP_ADC_BITS_8  = 1,
    CDC3_TP_ADC_BITS_10 = 2
}
CDCTpAdcBits;

//---- CDC3_TP_CONV_MODE
typedef enum
{
    CDC3_TP_INTR_TOUCH = 0,  // default
    CDC3_TP_INTR_DATA_RDY  = 1,
    CDC3_TP_INTR_TOUCH_DATA_RDY = 2,
    CDC3_TP_INTR_READ_RDY = 3
}
CDCTpIntrMode;

typedef enum
{
    CDC3_TP_CONV_NONE = 0,  // default
    CDC3_TP_CONV_XY = 1,
    CDC3_TP_CONV_X = 3,
    CDC3_TP_CONV_Y = 4,
    CDC3_TP_CONV_IN3 = 6,
    CDC3_TP_CONV_IN2 = 7,
    CDC3_TP_CONV_IN1 = 8,
    CDC3_TP_CONV_AUTO_IN123 = 9,
    CDC3_TP_CONV_PORT_IN123 = 11,
    CDC3_TP_CONV_TURN_X = 13,
    CDC3_TP_CONV_TURN_Y = 14,
    CDC3_TP_CONV_TURN_XY = 15
}
CDCTpConvMode;

//---- CDC3_TP_PRECHG_SNS
typedef enum
{
    CDC3_TP_SENSE_100NS = 0,  // default
    CDC3_TP_SENSE_1US = 1,
    CDC3_TP_SENSE_3US = 2,
    CDC3_TP_SENSE_10US = 3,
    CDC3_TP_SENSE_30US = 4,
    CDC3_TP_SENSE_100US = 5,
    CDC3_TP_SENSE_300US = 6,
    CDC3_TP_SENSE_1MS = 7
}
CDCTpSenseTime;

typedef enum
{
    CDC3_TP_PRECHG_100NS = 0,  // default
    CDC3_TP_PRECHG_1US = 1,
    CDC3_TP_PRECHG_3US = 2,
    CDC3_TP_PRECHG_10US = 3,
    CDC3_TP_PRECHG_30US = 4,
    CDC3_TP_PRECHG_100US = 5,
    CDC3_TP_PRECHG_300US = 6,
    CDC3_TP_PRECHG_1MS = 7
}
CDCTpPrechargeTime;

//---- CDC3_TP_VLT_STB
typedef enum
{
    CDC3_TP_VLT_STB_100NS = 0,  // default
    CDC3_TP_VLT_STB_1US  = 1,
    CDC3_TP_VLT_STB_3US = 2,
    CDC3_TP_VLT_STB_10US = 3,
    CDC3_TP_VLT_STB_30US = 4,
    CDC3_TP_VLT_STB_100US = 5,
    CDC3_TP_VLT_STB_300US = 6,
    CDC3_TP_VLT_STB_1MS = 7
}
CDCTpVoltStabTime;

//---- CDC3_TP_NBUF_MODE
typedef enum
{
    CDC3_TP_NBUF_MODE_LEN_8 = 0,
    CDC3_TP_NBUF_MODE_LEN_1 = 1,  // default
    CDC3_TP_NBUF_MODE_LEN_2 = 2,
    CDC3_TP_NBUF_MODE_LEN_3 = 3,
    CDC3_TP_NBUF_MODE_LEN_4 = 4,
    CDC3_TP_NBUF_MODE_LEN_5 = 5,
    CDC3_TP_NBUF_MODE_LEN_6 = 6,
    CDC3_TP_NBUF_MODE_LEN_7 = 7
}
CDCTpNewBufferLength;

//---- CDC3_TP_DELAY
typedef enum
{
    CDC3_TP_DELAY_8MS = 0,  // default
    CDC3_TP_DELAY_1MS = 1,
    CDC3_TP_DELAY_2MS = 2,
    CDC3_TP_DELAY_3MS = 3,
    CDC3_TP_DELAY_4MS = 4,
    CDC3_TP_DELAY_5MS = 5,
    CDC3_TP_DELAY_6MS = 6,
    CDC3_TP_DELAY_7MS = 7
}
CDCTpDelayTime;

typedef enum
{
    CDC3_TP_IN123_DELAY_1_12S = 0,  // default
    CDC3_TP_IN123_DELAY_3_36S = 1,
    CDC3_TP_IN123_DELAY_5_59S = 2,
    CDC3_TP_IN123_DELAY_7_83S = 3,
    CDC3_TP_IN123_DELAY_10_0S = 4,
    CDC3_TP_IN123_DELAY_12_3S = 5,
    CDC3_TP_IN123_DELAY_14_5S = 6,
    CDC3_TP_IN123_DELAY_16_8S = 7
}
CDCTpIn123DelayTime;

//---- CDC3_TP_DELAY_CLK / CDC3_TP_ADC_RST
typedef enum
{
    CDC3_TP_CLK_SRC_INTERNAL = 0,  // default
    CDC3_TP_CLK_SRC_MCLK = (1<< 7)
}
CDCTpClockSrc;

typedef enum
{
    CDC3_TP_CLK_DIV_MIN = 1,  // default
    CDC3_TP_CLK_DIV_MAX = 128,
    CDC3_TP_CLK_DIV_DEFAULT = CDC3_TP_CLK_DIV_MIN
}
CDCTpClockDivider;

//---- CDC3_TP_DBNC_TM
typedef enum
{
    CDC3_TP_DBNC_NONE = 0,  // default
    CDC3_TP_DBNC_16US = 1,
    CDC3_TP_DBNC_32US = 2,
    CDC3_TP_DBNC_64US = 3,
    CDC3_TP_DBNC_128US = 4,
    CDC3_TP_DBNC_256US = 5,
    CDC3_TP_DBNC_512US = 6,
    CDC3_TP_DBNC_1MS = 7
}
CDCTpDebounceTime;

//---- CDC3_TP_X_MSB / CDC3_TP_X_LSB
typedef enum
{
    CDC3_TP_X_MIN = 0,
    CDC3_TP_X_MAX = 0xffff
}
CDCTpCoordX;

//---- CDC3_TP_Y_MSB / CDC3_TP_Y_LSB
typedef enum
{
    CDC3_TP_Y_MIN = 0,
    CDC3_TP_Y_MAX = 0xffff
}
CDCTpCoordY;


// page 4 (ADC coefficients)
//---- CDC4_ADC_C*_MSB / CDC4_ADC_C*_LSB
typedef enum
{
    CDC4_ADC_COEF_MIN = 0,
    CDC4_ADC_COEF_MAX = 0xffff
}
CDCAdcCoef;


// page 8/9 (DAC coefficients)
//---- CDC8_DAC_C*_MSB / CDC8_DAC_C*_LSB
//---- CDC9_DAC_C*_MSB / CDC9_DAC_C*_LSB
typedef enum
{
    CDC8_DAC_COEF_MIN = 0,
    CDC8_DAC_COEF_MAX = 0xffff
}
CDCDacCoef;


// page 252 (buffer mode)
//---- CDC252_TP_BUF_MSB / CDC252_TP_BUF_LSB
typedef enum
{
    CDC252_TP_BUF_DATA_MIN = 0,
    CDC252_TP_BUF_DATA_MAX = 0xffff
}
CDCTpBufData;

#endif //0

//---------------- conversion macro
// page 0 (serialIF and digital control)
//---- CDC0_DIG_VOL_L / CDC0_DIG_VOL_R
#define CDC0_DIG_VOL_TO_REGVAL(x)  ((x)/2)

//---- CDC0_BP_CTL_L / CDC0_BP_CTL_R
#define CDC0_BP_VOL_TO_REGVAL(x)  (-(x))

//---- CDC0_AGC_CTL2
#define CDC0_AGC_SIG_THD_NSE_TO_REGVAL(x)  (((x)/-2)-14)

//---- CDC0_AGC_MAX_GAIN
#define CDC0_AGC_MAX_GAIN_TO_REGVAL(x)  ((x)/2)

//---- CDC0_AGC_ATTCK_TM
#define CDC0_AGC_ATTCK_TM_TO_REGVAL(x)  ((x)*2+1)

//---- CDC0_AGC_DECAY_TM
#define CDC0_AGC_DECAY_TM_TO_REGVAL(x)  ((x)*2+1)

// page 1 (analog I/O control)
//---- CDC1_MIC_ADC_PGA
#define CDC1_MIC_ADC_PGA_GAIN_TO_REGVAL(x)  ((x)*2)


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_CDC_CDC_REG_H_ */
#endif
