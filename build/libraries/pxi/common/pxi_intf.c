/*---------------------------------------------------------------------------*
  Project:  TwlFirm - library - pxi
  File:     pxi_intf.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include  <twl/os.h>
#include  <twl/pxi.h>


/*---------------------------------------------------------------------------*
  Name:         PXI_SendByIntf

  Description:  Send 4bit data to the other processor

  Arguments:    id          sending id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_SendByIntf( u32 data )
{
	reg_PXI_INTF = (u16)(data << REG_PXI_INTF_SEND_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         PXI_RecvByIntf

  Description:  Receive 4bit data from the other processor

  Arguments:    None

  Returns:      received id
 *---------------------------------------------------------------------------*/
u32  PXI_RecvByIntf( void )
{
	return (u32)(((reg_PXI_INTF & REG_PXI_INTF_RECV_MASK) >> REG_PXI_INTF_RECV_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         PXI_WaitByIntf

  Description:  Wait 4bit data from the other processor

  Arguments:    id          waiting id

  Returns:      None
 *---------------------------------------------------------------------------*/
void  PXI_WaitByIntf( u32 data )
{
    while (PXI_RecvByIntf() != data)
    {
    }
}

