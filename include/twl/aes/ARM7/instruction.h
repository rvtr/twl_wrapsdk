/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - aes
  File:     aes_common.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_AES_COMMON_H_
#define TWL_AES_COMMON_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
typedef enum
{
    AES_INPUT_TYPE_WCNT_4   = (0x3UL << REG_AES_AESCNT_IFIFO_DREQ_SHIFT),
    AES_INPUT_TYPE_WCNT_8   = (0x2UL << REG_AES_AESCNT_IFIFO_DREQ_SHIFT),
    AES_INPUT_TYPE_WCNT_12  = (0x1UL << REG_AES_AESCNT_IFIFO_DREQ_SHIFT),
    AES_INPUT_TYPE_WCNT_16  = (0x0UL << REG_AES_AESCNT_IFIFO_DREQ_SHIFT)
}
AESInputType;

typedef enum
{
    AES_OUTPUT_TYPE_WCNT_4  = (0x0UL << REG_AES_AESCNT_OFIFO_DREQ_SHIFT),
    AES_OUTPUT_TYPE_WCNT_8  = (0x1UL << REG_AES_AESCNT_OFIFO_DREQ_SHIFT),
    AES_OUTPUT_TYPE_WCNT_12 = (0x2UL << REG_AES_AESCNT_OFIFO_DREQ_SHIFT),
    AES_OUTPUT_TYPE_WCNT_16 = (0x3UL << REG_AES_AESCNT_OFIFO_DREQ_SHIFT)
}
AESOutputType;

typedef enum
{
    AES_MODE_CCM_DECRYPT    = (0x0UL << REG_AES_AESCNT_MODE_SHIFT),
    AES_MODE_CCM_ENCRYPT    = (0x1UL << REG_AES_AESCNT_MODE_SHIFT),
    AES_MODE_CTR            = (0x2UL << REG_AES_AESCNT_MODE_SHIFT),
    AES_MODE_CTR_DECRYPT    = AES_MODE_CTR,
    AES_MODE_CTR_ENCRYPT    = AES_MODE_CTR
}
AESMode;

typedef enum
{
    AES_MAC_LENGTH_4        = (1 << REG_AES_AESCNT_MAC_LEN_SHIFT),
    AES_MAC_LENGTH_6        = (2 << REG_AES_AESCNT_MAC_LEN_SHIFT),
    AES_MAC_LENGTH_8        = (3 << REG_AES_AESCNT_MAC_LEN_SHIFT),
    AES_MAC_LENGTH_10       = (4 << REG_AES_AESCNT_MAC_LEN_SHIFT),
    AES_MAC_LENGTH_12       = (5 << REG_AES_AESCNT_MAC_LEN_SHIFT),
    AES_MAC_LENGTH_14       = (6 << REG_AES_AESCNT_MAC_LEN_SHIFT),
    AES_MAC_LENGTH_16       = (7 << REG_AES_AESCNT_MAC_LEN_SHIFT)
}
AESMacLength;

/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         AES_Reset

  Description:  stop and reset AES block. but key resisters do not clear.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Reset(void);

/*---------------------------------------------------------------------------*
  Name:         AES_IsBusy

  Description:  check whether AES is busy or not

  Arguments:    None.

  Returns:      TRUE if AES is busy, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL AES_IsBusy(void);

/*---------------------------------------------------------------------------*
  Name:         AES_Wait

  Description:  wait while AES is busy

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Wait(void);

/*---------------------------------------------------------------------------*
  Name:         AES_InputFifoIsFull

  Description:  check whether AES input fifo is full or not

  Arguments:    None.

  Returns:      TRUE if AES is busy, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL AES_InputFifoIsFull(void);

/*---------------------------------------------------------------------------*
  Name:         AES_OutputFifoIsEmpty

  Description:  check whether AES output fifo is empty or not

  Arguments:    None.

  Returns:      TRUE if AES is busy, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL AES_OutputFifoIsEmpty(void);

/*---------------------------------------------------------------------------*
  Name:         AES_WaitInputFifoNotFull

  Description:  wait while AES input fifo is full.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_WaitInputFifoNotFull(void);

/*---------------------------------------------------------------------------*
  Name:         AES_WaitOutputFifoNotEmpty

  Description:  wait while AES output fifo is empty.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_WaitOutputFifoNotEmpty(void);

/*---------------------------------------------------------------------------*
  Name:         AES_IsValid

  Description:  check whether CCM decryption is valid or not.
                it may return TRUE just after CCM decryption has been completed.

  Arguments:    None.

  Returns:      TRUE if CCM decryption was valid, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL AES_IsValid(void);

/*---------------------------------------------------------------------------*
  Name:         AES_SelectKey

  Description:  select key from one of four key registers
                Note: SHOULD be called after AES_Set*() prior to AES_Start*().

  Arguments:    keyNo   - key group number.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SelectKey(u32 keyNo);

/*---------------------------------------------------------------------------*
  Name:         AES_SetKey

  Description:  set key data into key register

  Arguments:    keyNo   - key group number.
                pKey    - pointer to key data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SetKey(u32 keyNo, const u128 *pKey);

/*---------------------------------------------------------------------------*
  Name:         AES_SetId

  Description:  set id data into id register
                Note: never set key register with id and seed

  Arguments:    keyNo   - key group number.
                pId     - pointer to id data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SetId(u32 keyNo, const u128 *pId);

/*---------------------------------------------------------------------------*
  Name:         AES_SetSeed

  Description:  set seed data into seed register
                Note: automatically set associated key register with id and seed

  Arguments:    keyNo   - key group number.
                pSeed   - pointer to seed data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SetSeed(u32 keyNo, const u128 *pSeed);

/*---------------------------------------------------------------------------*
  Name:         AES_SetKey2

  Description:  set seed/id data into seed/id register
                Note: automatically set associated key register with id and seed

  Arguments:    keyNo   - key group number.
                pId      - pointer to id data
                pSeed    - pointer to seed data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SetKey2(u32 keyNo, const u128 *pId, const u128 *pSeed);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmDec

  Description:  start AES engine for AES-CCM decryption.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                mac         - pointer to 128-bit mac data.
                              if NULL, it assumes the mac will be sent from
                              the input FIFO.
                adataLength - length of the associated data.
                pdataLength - length of the payload (encrypted) data.
                              it excludes mac length even if the mac will be
                              sent from the input FIFO.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_StartCcmDec(const u96 *nonce, const u128 *mac, u32 adataLength, u32 pdataLength, BOOL isDistA);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmEnc

  Description:  start AES engine for AES-CCM encryption.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              note that output length will be extended for
                              the mac data.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_StartCcmEnc(const u96 *nonce, u32 adataLength, u32 pdataLength, BOOL isDistA);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCtrDec

  Description:  start AES engine for AES-CTR encryption/decryption.

  Arguments:    iv          - pointer to 128-bit iv data.
                length      - length of the data.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_StartCtrDec(const u128 *iv, u32 length);
/*---------------------------------------------------------------------------*
  Name:         AES_StartCtrEnc

  Description:  start AES engine for AES-CTR encryption/decryption.

  Arguments:    iv          - pointer to 128-bit iv data.
                length      - length of the data.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void AES_StartCtrEnc(const u128 *iv, u32 length)
{
    AES_StartCtrDec(iv, length);
}

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_AES_COMMON_H_ */
