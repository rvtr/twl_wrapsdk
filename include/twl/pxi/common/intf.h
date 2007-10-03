/*---------------------------------------------------------------------------*
  Project:  TwlSDK - -include - PXI
  File:     intf.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_PXI_COMMON_INTF_H_
#define TWL_PXI_COMMON_INTF_H_

#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/pxi/common/regname.h>


#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         PXI_SendByIntf

  Description:  send 4bit data to other processor

  Arguments:    id          notifying id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_SendByIntf( u32 data );

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvByIntf

  Description:  receive 4bit data from the other processor

  Arguments:    None

  Returns:      received id
 *---------------------------------------------------------------------------*/
u32  PXI_RecvByIntf( void );

/*---------------------------------------------------------------------------*
  Name:         PXI_WaitByIntf

  Description:  Wait 4bit data from the other processor

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_WaitByIntf( u32 data );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_PXI_COMMON_INTF_H_ */
