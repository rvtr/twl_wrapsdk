/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - MIC
  File:     mic.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_MIC_H_
#define TWL_MIC_H_

#include <twl/mic/common/fifo.h>

#ifdef SDK_ARM7

#include <twl/mic/ARM7/twl_mic_api.h>
#include <twl/mic/ARM7/twl_mic_server.h>

#else

#include <twl/mic/ARM9/twl_mic_api.h>

#endif // SDK_ARM7

/* TWL_MIC_H_ */
#endif
