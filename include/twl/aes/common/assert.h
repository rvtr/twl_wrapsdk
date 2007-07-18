/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - include
  File:     assert.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_AES_ASSERT_H_
#define TWL_AES_ASSERT_H_

/*---------------------------------------------------------------------------*
    ASSERT
 *---------------------------------------------------------------------------*/
#define AES_KEY_MAX_NUM 3

#define AES_ASSERT_KEYNO( keyNo )        SDK_ASSERTMSG( (keyNo) <= AES_KEY_MAX_NUM, "illegal AES Key No." )
#define AES_ASSERT_DATA_LENGTH( len )    SDK_ASSERTMSG( (len) & 0xFF0000FF, "illegal data length." )

#endif /* TWL_AES_ASSERT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
