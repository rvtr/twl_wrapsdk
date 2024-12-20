/*---------------------------------------------------------------------------*
  Project:  NitroSDK - include
  File:     dtcm_begin.h

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: dtcm_begin.h,v $
  Revision 1.4  2006/01/18 02:11:19  kitase_hirotake
  do-indent

  Revision 1.3  2005/07/11 05:00:42  yada
  add include nitro/section.h

  Revision 1.2  2005/02/28 05:26:02  yosizaki
  do-indent.

  Revision 1.1  2004/07/10 06:11:24  yasu
  Support to select a section 'ITCM/DTCM/WRAM'

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
//
// include this instead of using C99 pragma extensions for compatibility
//
#include <nitro/section.h>

#if	defined(SDK_CW) || defined(__MWERKS__)
//#pragma section DTCM begin
#elif	defined(SDK_ADS)
TO BE   DEFINED
#elif	defined(SDK_GCC)
TO BE   DEFINED
#endif
