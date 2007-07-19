/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CAMERA - include
  File:     camera.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_CAMERA_H_
#define TWL_CAMERA_H_

#include <twl/camera/common/fifo.h>
#include <twl/camera/common/types.h>

#ifdef SDK_ARM7

#include <twl/camera/ARM7/i2c.h>
#include <twl/camera/ARM7/control.h>

#else

#include <twl/camera/ARM9/camera.h>
#include <twl/camera/ARM9/camera_api.h>
#include <twl/camera/ARM9/transfer.h>

#endif

/* TWL_LCDC_H_ */
#endif
