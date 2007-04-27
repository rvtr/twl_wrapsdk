/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - aes
  File:     aes.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef TWL_AES_AES_H_
#define TWL_AES_AES_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// 処理結果定義
typedef enum AESResult
{
    AES_RESULT_SUCCESS = 0,
    AES_RESULT_SUCCESS_TRUE,
    AES_RESULT_SUCCESS_FALSE,
    AES_RESULT_BUSY,
    AES_RESULT_ILLEGAL_PARAMETER,
    AES_RESULT_SEND_ERROR,
    AES_RESULT_INVALID_COMMAND,
    AES_RESULT_ILLEGAL_STATUS,
    AES_RESULT_FATAL_ERROR,
    AES_RESULT_MAX
}
AESResult;

// コールバック
typedef void (*AESCallback)(AESResult result, void *arg);


/*---------------------------------------------------------------------------*
  Name:         AES_Init

  Description:  AESライブラリを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AES_Init(void);

/*---------------------------------------------------------------------------*
  Name:         AES_ResetAsync

  Description:  stop and reset AES block. but key resisters do not clear.
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_ResetAsync(AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_Reset

  Description:  stop and reset AES block. but key resisters do not clear.
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_Reset(void);

/*---------------------------------------------------------------------------*
  Name:         AES_IsBusyAsync

  Description:  check whether AES is busy or not
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_IsBusyAsync(AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_IsBusy

  Description:  check whether AES is busy or not
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_IsBusy(void);

/*---------------------------------------------------------------------------*
  Name:         AES_WaitAsync

  Description:  wait while AES is busy
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitAsync(AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_Wait

  Description:  wait while AES is busy
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_Wait(void);

/*---------------------------------------------------------------------------*
  Name:         AES_InputFifoIsFullAsync

  Description:  check whether AES input fifo is full or not
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_InputFifoIsFullAsync(AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_InputFifoIsFull

  Description:  check whether AES input fifo is full or not
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_InputFifoIsFull(void);

/*---------------------------------------------------------------------------*
  Name:         AES_OutputFifoIsEmptyAsync

  Description:  check whether AES output fifo is empty or not
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_OutputFifoIsEmptyAsync(AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_OutputFifoIsEmpty

  Description:  check whether AES output fifo is empty or not
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_OutputFifoIsEmpty(void);

/*---------------------------------------------------------------------------*
  Name:         AES_WaitInputFifoNotFullAsync

  Description:  wait while AES input fifo is full.
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitInputFifoNotFullAsync(AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_WaitInputFifoNotFull

  Description:  wait while AES input fifo is full.
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitInputFifoNotFull(void);

/*---------------------------------------------------------------------------*
  Name:         AES_WaitOutputFifoNotEmptyAsync

  Description:  wait while AES output fifo is empty.
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitOutputFifoNotEmptyAsync(AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_WaitOutputFifoNotEmpty

  Description:  wait while AES output fifo is empty.
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_WaitOutputFifoNotEmpty(void);

/*---------------------------------------------------------------------------*
  Name:         AES_IsValidAsync

  Description:  check whether CCM decryption is valid or not.
                it may return TRUE just after CCM decryption has been completed.
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_IsValidAsync(AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_IsValid

  Description:  check whether CCM decryption is valid or not.
                it may return TRUE just after CCM decryption has been completed.
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_IsValid(void);

/*---------------------------------------------------------------------------*
  Name:         AES_SelectKeyAsync

  Description:  select key from one of four key registers
                Note: SHOULD be called after AES_Set*() prior to AES_Start*().
                async version

  Arguments:    keyNo       - key group number.
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SelectKeyAsync(u32 keyNo, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_SelectKey

  Description:  select key from one of four key registers
                Note: SHOULD be called after AES_Set*() prior to AES_Start*().
                sync version.

  Arguments:    keyNo   - key group number.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SelectKey(u32 keyNo);

/*---------------------------------------------------------------------------*
  Name:         AES_SetKeyAsync

  Description:  set key data into key register
                async version.

  Arguments:    keyNo   - key group number.
                pKey    - pointer to key data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetKeyAsync(u32 keyNo, const u128 *pKey, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_SetKey

  Description:  set key data into key register
                sync version.

  Arguments:    keyNo   - key group number.
                pKey    - pointer to key data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetKey(u32 keyNo, const u128 *pKey);

/*---------------------------------------------------------------------------*
  Name:         AES_SetIdAsync

  Description:  set id data into id register
                Note: never set key register with id and seed
                async version.

  Arguments:    keyNo       - key group number.
                pId         - pointer to id data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetIdAsync(u32 keyNo, const u128 *pId, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_SetId

  Description:  set id data into id register
                Note: never set key register with id and seed
                sync version.

  Arguments:    keyNo   - key group number.
                pId     - pointer to id data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetId(u32 keyNo, const u128 *pId);

/*---------------------------------------------------------------------------*
  Name:         AES_SetSeedAsync

  Description:  set id data into id register
                Note: automatically set associated key register with id and seed
                async version.

  Arguments:    keyNo       - key group number.
                pSeed       - pointer to seed data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetSeedAsync(u32 keyNo, const u128 *pSeed, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_SetSeed

  Description:  set id data into id register
                Note: automatically set associated key register with id and seed
                sync version.

  Arguments:    keyNo   - key group number.
                pSeed   - pointer to seed data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetSeed(u32 keyNo, const u128 *pSeed);

/*---------------------------------------------------------------------------*
  Name:         AES_SetKey2Async

  Description:  set seed/id data into seed/id register
                Note: automatically set associated key register with id and seed
                async version.

  Arguments:    keyNo       - key group number.
                pId         - pointer to id data
                pSeed       - pointer to seed data
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetKey2Async(u32 keyNo, const u128 *pId, const u128 *pSeed, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_SetKey2

  Description:  set seed/id data into seed/id register
                Note: automatically set associated key register with id and seed
                sync version.

  Arguments:    keyNo   - key group number.
                pId      - pointer to id data
                pSeed    - pointer to seed data

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_SetKey2(u32 keyNo, const u128 *pId, const u128 *pSeed);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmDecAsync

  Description:  start AES engine for AES-CCM decryption.
                async version.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                mac         - pointer to 128-bit mac data.
                              if NULL, it assumes the mac will be sent from
                              the input FIFO.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              it excludes mac length even if the mac will be
                              sent from the input FIFO.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCcmDecAsync(const u96 *nonce, const u128 *mac, u32 adataLength, u32 pdataLength, BOOL isDistA, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmDec

  Description:  start AES engine for AES-CCM decryption.
                sync version.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                mac         - pointer to 128-bit mac data.
                              if NULL, it assumes the mac will be sent from
                              the input FIFO.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              it excludes mac length even if the mac will be
                              sent from the input FIFO.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCcmDec(const u96 *nonce, const u128 *mac, u32 adataLength, u32 pdataLength, BOOL isDistA);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmEncAsync

  Description:  start AES engine for AES-CCM encryption.
                async version.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              note that output length will be extended for
                              the mac data.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCcmEncAsync(const u96 *nonce, u32 adataLength, u32 pdataLength, BOOL isDistA, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCcmEnc

  Description:  start AES engine for AES-CCM encryption.
                sync version.

  Arguments:    nonce       - pointer to 128-bit nonce data.
                adataLength - length of the associated data.
                pdataLength - length of the payload data.
                              note that output length will be extended for
                              the mac data.
                isDistA     - whether associated data will be output from the
                              output FIFO or not.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCcmEnc(const u96 *nonce, u32 adataLength, u32 pdataLength, BOOL isDistA);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCtrDecAsync

  Description:  start AES engine for AES-CTR encryption/decryption.
                async version.

  Arguments:    iv          - pointer to 128-bit iv data.
                length      - length of the data.
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCtrDecAsync(const u128 *iv, u32 length, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCtrDec

  Description:  start AES engine for AES-CTR encryption/decryption.
                sync version.

  Arguments:    iv          - pointer to 128-bit iv data.
                length      - length of the data.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartCtrDec(const u128 *iv, u32 length);

/*---------------------------------------------------------------------------*
  Name:         AES_StartCtrEncAsync

  Description:  start AES engine for AES-CTR encryption/decryption.
                async version.

  Arguments:    iv          - pointer to 128-bit iv data.
                length      - length of the data.
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
static inline AESResult AES_StartCtrEncAsync(const u128 *iv, u32 length, AESCallback callback, void *arg)
{
    return AES_StartCtrDecAsync(iv, length, callback, arg);
}

/*---------------------------------------------------------------------------*
  Name:         AES_StartEncDec

  Description:  start AES engine for AES-CTR encryption/decryption.
                sync version.

  Arguments:    iv          - pointer to 128-bit iv data.
                length      - length of the data.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
static inline AESResult AES_StartEncDec(const u128 *iv, u32 length)
{
    return AES_StartCtrDec(iv, length);
}

/*---------------------------------------------------------------------------*
  Name:         AES_TryLockAsync

  Description:  AESライブラリをARM7から使われないように試みる。
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_TryLockAsync(AESCallback callback, void *arg);

/*
    転送コマンド
*/
/*---------------------------------------------------------------------------*
  Name:         AES_StartDmaSendAsync

  Description:  AES DMA send by AES input FIFO timing.
                Should call prior to the AES_Start*().
                async version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                src     : source address
                size    : transfer size (byte) (multiple of 16 bytes)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartDmaSendAsync(u32 dmaNo, const void *src, u32 size, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_StartDmaSend

  Description:  AES DMA send by AES input FIFO timing.
                Should call prior to the AES_Start*().
                sync version.
                NOTE: never wait to done to transfer!

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                src     : source address
                size    : transfer size (byte) (multiple of 16 bytes)

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartDmaSend(u32 dmaNo, const void *src, u32 size);

/*---------------------------------------------------------------------------*
  Name:         AES_StartDmaRecvAsync

  Description:  AES DMA receive by AES output FIFO timing.
                Should call prior to the AES_Start*().
                async version.

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                dest     : destination address
                size    : transfer size (byte) (multiple of 16 bytes)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartDmaRecvAsync(u32 dmaNo, const void *dest, u32 size, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_StartDmaRecv

  Description:  AES DMA receive by AES output FIFO timing.
                Should call prior to the AES_Start*().
                sync version.
                NOTE: never wait to done to transfer!

  Arguments:    dmaNo   : DMA channel No. (4 - 7)
                dest    : destination address
                size    : transfer size (byte) (multiple of 16 bytes)

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_StartDmaRecv(u32 dmaNo, const void *dest, u32 size);

/*---------------------------------------------------------------------------*
  Name:         AES_CpuSendAsync

  Description:  send data to AES input fifo by CPU.
                Should call prior to the AES_Start*().
                async version.
                NOTE: callback will be called after to done to transfer!
                CAUTION: Cannot use AES_CPUSend*() and AES_CPURecv*() at same time.

  Arguments:    src     : source address
                length  : transfer size (byte) (multiple of 16 bytes)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_CpuSendAsync(const void *src, u32 length, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_CpuSend

  Description:  send data to AES input fifo by CPU.
                Should call prior to the AES_Start*().
                sync version.
                CAUTION: Cannot use AES_CPUSend*() and AES_CPURecv*() at same time.

  Arguments:    src     : source address
                length  : transfer size (byte) (multiple of 16 bytes)

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_CpuSend(const void *src, u32 length);

/*---------------------------------------------------------------------------*
  Name:         AES_CpuRecvAsync

  Description:  receive data from AES input fifo by CPU.
                Should call prior to the AES_Start*().
                async version.
                NOTE: callback will be called after to done to transfer!
                CAUTION: Cannot use AES_CPUSend*() and AES_CPURecv*() at same time.

  Arguments:    dest    : destination address
                length  : transfer size (byte) (multiple of 16 bytes)
                callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_CpuRecvAsync(const void *dest, u32 length, AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_CpuRecv

  Description:  receive data from AES input fifo by CPU.
                Should call prior to the AES_Start*().
                sync version.
                CAUTION: Cannot use AES_CPUSend*() and AES_CPURecv*() at same time.

  Arguments:    dest    : destination address
                length  : transfer size (byte) (multiple of 16 bytes)

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_CpuRecv(const void *dest, u32 length);

/*
    特殊コマンド
*/
/*---------------------------------------------------------------------------*
  Name:         AES_TryLock

  Description:  AESライブラリをARM7から使われないように試みる。
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_TryLock(void);

/*---------------------------------------------------------------------------*
  Name:         AES_UnlockAsync

  Description:  AESライブラリのARM9側のロックを解除する
                async version.

  Arguments:    callback    - 非同期処理が完了した再に呼び出す関数を指定
                arg         - コールバック関数の呼び出し時の引数を指定。

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_UnlockAsync(AESCallback callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         AES_Unlock

  Description:  AESライブラリをARM7から使われないように試みる。
                sync version.

  Arguments:    None.

  Returns:      AESResult
 *---------------------------------------------------------------------------*/
AESResult AES_Unlock(void);

/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_AES_AES_H_ */
