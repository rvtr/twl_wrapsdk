/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - aes
  File:     aes_common.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl/aes/ARM7/instruction.h>
#include <twl/aes/common/assert.h>
#include <nitro/os/common/interrupt.h>

#ifdef AES_DOES_NOT_SUPPORT_MULTIPLE_KEYS
#define REG_AES_KEY_OFFSET                                 0x4410
#define REG_AES_KEY_ADDR                                   (HW_REG_BASE + REG_AES_KEY_OFFSET)
#define reg_AES_AES_KEY                                    (*( REGType128v *) REG_AES_KEY_ADDR)
#define REG_AES_ID_OFFSET                                  0x4440
#define REG_AES_ID_ADDR                                    (HW_REG_BASE + REG_AES_ID_OFFSET)
#define reg_AES_AES_ID                                     (*( REGType128v *) REG_AES_ID_ADDR)
#define REG_AES_SEED_OFFSET                                0x4450
#define REG_AES_SEED_ADDR                                  (HW_REG_BASE + REG_AES_SEED_OFFSET)
#define reg_AES_AES_SEED                                   (*( REGType128v *) REG_AES_SEED_ADDR)
#endif

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define reg_AES_NONCE_CTR   (*( REGType128v *) REG_AES_NNC_CTR_ADDR)
#define reg_AES_NONCE_CCM   (*( REGType96v *) REG_AES_NNC_CTR_ADDR)

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef struct AESKey
{
    u128    key;
    u128    id;
    u128    seed;
}
AESKey;

/*---------------------------------------------------------------------------*
    静的変数定義
 *---------------------------------------------------------------------------*/
static volatile AESKey *const aesKeyArray = (AESKey*)REG_AES_KEY0_ADDR;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void AES_KeySetup(u32 keyNo);


/*---------------------------------------------------------------------------*
  Name:         AES_Reset

  Description:  stop and reset AES block. but key resisters do not clear.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Reset(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    reg_AES_AESCNT = REG_AES_AESCNT_OFIFO_CLR_MASK | REG_AES_AESCNT_IFIFO_CLR_MASK;
    reg_AES_AESCNT = REG_AES_AESCNT_OFIFO_CLR_MASK | REG_AES_AESCNT_IFIFO_CLR_MASK;
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         AES_IsBusy

  Description:  check whether AES is busy or not

  Arguments:    None.

  Returns:      TRUE if AES is busy, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL AES_IsBusy(void)
{
    return (BOOL)((reg_AES_AESCNT & REG_AES_AESCNT_E_MASK) >> REG_AES_AESCNT_E_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         AES_Wait

  Description:  wait while AES is busy

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Wait(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    while (reg_AES_AESCNT & REG_AES_AESCNT_E_MASK)
    {
    }
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         AES_InputFifoIsFull

  Description:  check whether AES input fifo is full or not

  Arguments:    None.

  Returns:      TRUE if AES is busy, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL AES_InputFifoIsFull(void)
{
    return (((reg_AES_AESCNT & REG_AES_AESCNT_IFIFO_COUNT_MASK) >> REG_AES_AESCNT_IFIFO_COUNT_SHIFT) == 16);
}

/*---------------------------------------------------------------------------*
  Name:         AES_OutputFifoIsEmpty

  Description:  check whether AES output fifo is empty or not

  Arguments:    None.

  Returns:      TRUE if AES is busy, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL AES_OutputFifoIsEmpty(void)
{
    return (((reg_AES_AESCNT & REG_AES_AESCNT_OFIFO_COUNT_MASK) >> REG_AES_AESCNT_OFIFO_COUNT_SHIFT) == 0);
}

/*---------------------------------------------------------------------------*
  Name:         AES_WaitInputFifoNotFull

  Description:  wait while AES input fifo is full.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_WaitInputFifoNotFull(void)
{
    while (((reg_AES_AESCNT & REG_AES_AESCNT_IFIFO_COUNT_MASK) >> REG_AES_AESCNT_IFIFO_COUNT_SHIFT) == 16)
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         AES_WaitOutputFifoNotEmpty

  Description:  wait while AES output fifo is empty.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_WaitOutputFifoNotEmpty(void)
{
    while (((reg_AES_AESCNT & REG_AES_AESCNT_OFIFO_COUNT_MASK) >> REG_AES_AESCNT_OFIFO_COUNT_SHIFT) == 0)
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         AES_IsValid

  Description:  check whether CCM decryption is valid or not.
                it may return TRUE just after CCM decryption has been completed.

  Arguments:    None.

  Returns:      TRUE if CCM decryption was valid, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL AES_IsValid(void)
{
    return (BOOL)((reg_AES_AESCNT & REG_AES_AESCNT_MAC_RSLT_MASK) >> REG_AES_AESCNT_MAC_RSLT_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         AES_SelectKey

  Description:  select key from one of four key registers
                Note: SHOULD be called after AES_Set*() prior to AES_Start*().

  Arguments:    keyNo   - key group number.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SelectKey(u32 keyNo)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    AES_ASSERT_KEYNO(keyNo);
    while (reg_AES_AESCNT & REG_AES_AESCNT_KEY_BUSY_MASK)
    {
    }
#ifdef AES_DOES_NOT_SUPPORT_MULTIPLE_KEYS
    reg_AES_AESCNT |= REG_AES_AESCNT_KEY_SET_MASK;
#else
    reg_AES_AESCNT = (reg_AES_AESCNT & ~REG_AES_AESCNT_KEY_SEL_MASK) |
                  (keyNo << REG_AES_AESCNT_KEY_SEL_SHIFT) |
                  REG_AES_AESCNT_KEY_SET_MASK;
#endif
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         AES_SetKey

  Description:  set key data into key register

  Arguments:    keyNo   - key group number.
                pKey    - pointer to key data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SetKey(u32 keyNo, const u128 *pKey)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    vu128 *p = &aesKeyArray[keyNo].key;
    AES_ASSERT_KEYNO(keyNo);
    SDK_NULL_ASSERT(pKey);
#ifdef AES_DOES_NOT_SUPPORT_MULTIPLE_KEYS
    reg_AES_AES_KEY = *pKey;
#else
    *p = *pKey;
#endif
    (void)OS_RestoreInterrupts(enabled);
}
/*---------------------------------------------------------------------------*
  Name:         AES_SetId

  Description:  set id data into id register
                Note: never set key register with id and seed

  Arguments:    keyNo   - key group number.
                pId     - pointer to key data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SetId(u32 keyNo, const u128 *pId)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    vu128 *p = &aesKeyArray[keyNo].id;
    AES_ASSERT_KEYNO(keyNo);
    SDK_NULL_ASSERT(pId);
#ifdef AES_DOES_NOT_SUPPORT_MULTIPLE_KEYS
    reg_AES_AES_ID = *pId;
#else
    *p = *pId;
#endif
    (void)OS_RestoreInterrupts(enabled);
}
/*---------------------------------------------------------------------------*
  Name:         AES_SetSeed

  Description:  set seed data into seed register
                Note: automatically set associated key register with id and seed

  Arguments:    keyNo   - key group number.
                pSeed   - pointer to seed data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SetSeed(u32 keyNo, const u128 *pSeed)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    vu128 *p = &aesKeyArray[keyNo].seed;
    AES_ASSERT_KEYNO(keyNo);
    SDK_NULL_ASSERT(pSeed);
#ifdef AES_DOES_NOT_SUPPORT_MULTIPLE_KEYS
    reg_AES_AES_SEED = *pSeed;
#else
    *p = *pSeed;
#endif
    (void)OS_RestoreInterrupts(enabled);
}
/*---------------------------------------------------------------------------*
  Name:         AES_SetKey2

  Description:  set seed/id data into seed/id register
                Note: automatically set associated key register with id and seed

  Arguments:    keyNo   - key group number.
                pId      - pointer to id data
                pSeed    - pointer to seed data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_SetKey2(u32 keyNo, const u128 *pId, const u128 *pSeed)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    vu128 *pI = &aesKeyArray[keyNo].id;
    vu128 *pS = &aesKeyArray[keyNo].seed;
    AES_ASSERT_KEYNO(keyNo);
    SDK_NULL_ASSERT(pId);
    SDK_NULL_ASSERT(pSeed);
#ifdef AES_DOES_NOT_SUPPORT_MULTIPLE_KEYS
    reg_AES_AES_ID   = *pId;
    reg_AES_AES_SEED = *pSeed;
#else
    *pI = *pId;
    *pS = *pSeed;
#endif
    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmDec

  Description:  start AES engine for AES-CCM decryption.

  Arguments:    nonce       - pointer to 96-bit nonce data.
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
void AES_StartCcmDec(const u96 *nonce, const u128 *mac, u32 adataLength, u32 pdataLength, BOOL isDistA)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    AES_ASSERT_DATA_LENGTH(adataLength);
    AES_ASSERT_DATA_LENGTH(pdataLength);
    SDK_NULL_ASSERT(nonce);

    reg_AES_NONCE_CCM = *nonce;
    reg_AES_AES_ASO_LEN = (u16)(adataLength >> 4);
    reg_AES_AES_PLD_LEN = (u16)(pdataLength >> 4);

    if (mac)
    {
        reg_AES_AES_MAC = *mac;
    }
    reg_AES_AESCNT = AES_INPUT_TYPE_WCNT_4 | AES_OUTPUT_TYPE_WCNT_4 | AES_MAC_LENGTH_16 |
                  (isDistA ? REG_AES_AESCNT_ADATA_OE_MASK : 0) |
                  (mac ? REG_AES_AESCNT_MAC_ISEL_MASK : 0) |
                  REG_AES_AESCNT_I_MASK | REG_AES_AESCNT_E_MASK | AES_MODE_CCM_DECRYPT;

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmEnc

  Description:  start AES engine for AES-CCM encryption.

  Arguments:    nonce       - pointer to 96-bit nonce data.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              note that output length will be extended for
                              the mac data.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_StartCcmEnc(const u96 *nonce, u32 adataLength, u32 pdataLength, BOOL isDistA)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    AES_ASSERT_DATA_LENGTH(adataLength);
    AES_ASSERT_DATA_LENGTH(pdataLength);
    SDK_NULL_ASSERT(nonce);

    reg_AES_NONCE_CCM = *nonce;
    reg_AES_AES_ASO_LEN = (u16)(adataLength >> 4);
    reg_AES_AES_PLD_LEN = (u16)(pdataLength >> 4);

    reg_AES_AESCNT = AES_INPUT_TYPE_WCNT_4 | AES_OUTPUT_TYPE_WCNT_4 | AES_MAC_LENGTH_16 |
                  (isDistA ? REG_AES_AESCNT_ADATA_OE_MASK : 0) |
                  REG_AES_AESCNT_I_MASK | REG_AES_AESCNT_E_MASK | AES_MODE_CCM_ENCRYPT;

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartCtrDec

  Description:  start AES engine for AES-CTR encryption/decryption.

  Arguments:    iv          - pointer to 128-bit iv data.
                length      - length of the data.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_StartCtrDec(const u128 *iv, u32 length)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    AES_ASSERT_DATA_LENGTH(length);
    SDK_NULL_ASSERT(iv);

    reg_AES_NONCE_CTR = *iv;
    reg_AES_AES_PLD_LEN = (u16)(length >> 4);

    reg_AES_AESCNT = AES_INPUT_TYPE_WCNT_4 | AES_OUTPUT_TYPE_WCNT_4 |
                  REG_AES_AESCNT_I_MASK | REG_AES_AESCNT_E_MASK | AES_MODE_CTR;

    (void)OS_RestoreInterrupts(enabled);
}
