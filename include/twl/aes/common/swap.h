/*---------------------------------------------------------------------------*
  Project:  TwlSDK - aes - include
  File:     swap.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_AES_SWAP_H_
#define TWL_AES_SWAP_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
  Name:         AES_SwapWord128

  Description:  swap 32-bit array to 128-bit little endian.
                for example, 0x00112233, 0x44556677, 0x8899aabb, 0xccddeeff
                are copied to 0x00112233445566778899aabbccddeeff.

  Arguments:    dest    - destination address
                src     - source address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SwapWord128(u128 *dest, const u32 *src);

/*---------------------------------------------------------------------------*
  Name:         AES_SwapWord96

  Description:  swap 96-bit array to 128-bit little endian.
                for example, 0x00112233, 0x44556677, 0x8899aabb
                are copied to 0x00112233445566778899aabb.

  Arguments:    dest    - destination address
                src     - source address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SwapWord96(u96 *dest, const u32 *src);

/*---------------------------------------------------------------------------*
  Name:         AES_SwapByte128

  Description:  swap 8-bit array to 128-bit little endian.
                for example, 0x00, 0x11..., 0xff are copied to
                0x00112233445566778899aabbccddeeff.

  Arguments:    dest    - destination address
                src     - source address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SwapByte128(u128 *dest, const u8 *src);

/*---------------------------------------------------------------------------*
  Name:         AES_SwapByte96

  Description:  swap 8-bit array to 96-bit little endian.
                for example, 0x00, 0x11..., 0xbb are copied to
                0x00112233445566778899aabb.

  Arguments:    dest    - destination address
                src     - source address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SwapByte96(u96 *dest, const u8 *src);

/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_AES_SWAP_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
