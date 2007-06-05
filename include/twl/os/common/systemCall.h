/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     systemCall.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_SYSTEMCALL_H_
#define TWL_OS_SYSTEMCALL_H_


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         SVC_DecryptoSign

  Description:  

  Arguments:    buffer : 
                sgn_ptr : 
                key_ptr : 

  Returns:      None
 *---------------------------------------------------------------------------*/
int SVC_DecryptoSign(
                    void*   buffer,     //  出力領域
                    const void*   sgn_ptr,    //  データへのポインタ
                    const void*   key_ptr     //  キーへのポインタ
                    );


#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_SYSTEMCALL_H_ */
#endif
