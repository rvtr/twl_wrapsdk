/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraties - cdc
  File:     cdc_filter_coefficient.c

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
#include <twl/cdc.h>


//================================================================================
//        Typical Filter Coefficients (1st order)
//================================================================================
/*---------------------------------------------------------------------------*
  HPF, cut-off(-3dB) point = 0x00021*Fs = 9.26Hz (@Fs = 44.1kHz)
 *---------------------------------------------------------------------------*/
u8 cdc1stCoef_HPF_9_26Hz[6] = { 0x7F, 0xEA, 0x80, 0x16, 0x7F, 0xD5 };


