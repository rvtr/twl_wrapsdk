/*---------------------------------------------------------------------------*
  Project:  NitroSDK - AES - demos - aes-1
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl.h>

#define INPUT_DMA   4
#define OUTPUT_DMA  5

#define ADATA_LENGTH    320
#define PDATA_LENGTH    (sizeof(gs_data) - ADATA_LENGTH)

static const u128 key = {
    0x01234567,
    0x89abcdef,
    0x01234567,
    0x89abcdef
};
static const u128 key2 = {
    0x00112233,
    0x44556677,
    0x8899aabb,
    0xccddeeff
};
static const u96 nonce = {
    0x01234567,
    0x89abcdef,
    0x01234567
};
static const u32 gs_data[] ATTRIBUTE_ALIGN(32) = {
    // ADATA
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef,
    // PDATA
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567
};

static u8 dataA[sizeof(gs_data) + sizeof(u128)] ATTRIBUTE_ALIGN(32);
static u8 dataB[sizeof(gs_data) + sizeof(u128)] ATTRIBUTE_ALIGN(32);

//================================================================================
static void dump(const char *str, void *ptr, u32 length)
{
    u8 *data = (u8*)ptr;
    int i;
    OS_TPrintf("\n[%s] (%d bytes):\n\t", str, length);
    for (i = 0; i < length; i++) {
        OS_TPrintf("%02X", *data++);
        if ((i & 0xF) == 0xF) OS_TPrintf("\n\t");
        else    OS_TPrintf(" ");
    }
    OS_TPrintf("\n");
}

#define TEST0_USE_DMA_INPUT
#define TEST0_USE_DMA_OUTPUT
static void test0(void)
{
    OSTick begin;
    AESResult result;

    OS_TPrintf("\n%s() is starting.\n\n", __func__);

    // とりあえず、リセットする (鍵関係は何も変わらない (KEY_SELレジスタも消えるが意味は無い))
    result = AES_Reset();
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_Reset (%d).\n", __func__, result);
    }

    // 鍵は0番目を使う
    result = AES_SelectKey(0);
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_SelectKey (%d).\n", __func__, result);
    }

#ifdef TEST0_USE_DMA_INPUT
    // 入力DMA設定
    result = AES_StartDmaSend(INPUT_DMA, gs_data, sizeof(gs_data));    // adata + pdata
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_StartDmaSend (%d).\n", __func__, result);
    }
#endif

#ifdef TEST0_USE_DMA_OUTPUT
    // 出力DMA設定
    MI_CpuClear32(dataA, sizeof(dataA));
    DC_FlushRange(dataA, sizeof(dataA));
    result = AES_StartDmaRecv(OUTPUT_DMA, dataA, sizeof(gs_data) + sizeof(u128));   // adata + pdata + mac
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_StartDmaSend (%d).\n", __func__, result);
    }
#endif

    begin = OS_GetTick();

    // AES-CCMエンコードを開始する
    result = AES_StartCcmEnc(&nonce, ADATA_LENGTH, PDATA_LENGTH, TRUE);
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_StartCcmEnc (%d).\n", __func__, result);
    }

#ifdef TEST0_USE_DMA_INPUT
#ifdef TEST0_USE_DMA_OUTPUT
    // AES完了待ち
    result = AES_Wait();
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_Wait (%d).\n", __func__, result);
    }
#else
    // CPUで出力してみる
    result = AES_CpuRecv(dataA, sizeof(gs_data) + sizeof(u128));   // adata + pdata + mac
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_CpuRecv (%d).\n", __func__, result);
    }
#endif
#else
#ifdef TEST0_USE_DMA_OUTPUT
    // CPUで入力してみる
    result = AES_CpuSend(gs_data, sizeof(gs_data));   // adata + pdata
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_CpuSend (%d).\n", __func__, result);
    }
#else
#error "Does not support CPU input and CPU output at same time."
#endif
#endif

    OS_TPrintf("%s: %d usec.\n", __func__, (u32)OS_TicksToMicroSeconds(OS_GetTick()-begin));

    // 出力結果の表示
    dump(__func__, dataA, sizeof(gs_data) + sizeof(u128));
}

static void test1(void)
{
    OSTick begin;
    AESResult result;

    OS_TPrintf("\n%s() is starting.\n\n", __func__);

    // とりあえず、リセットする (鍵関係は何も変わらない (KEY_SELレジスタも消えるが意味は無い))
    result = AES_Reset();
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_Reset (%d).\n", __func__, result);
    }

    // 鍵を設定しない (前回の値を使いまわす)

    // 入力DMA設定
    DC_FlushRange(dataA, sizeof(dataA));
    result = AES_StartDmaSend(INPUT_DMA, dataA, sizeof(gs_data) + sizeof(u128));   // adata + pdata + mac
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_StartDmaSend (%d).\n", __func__, result);
    }

    // 出力バッファのクリア
    MI_CpuClear32(dataB, sizeof(dataB));
    DC_FlushRange(dataB, sizeof(dataB));

    begin = OS_GetTick();

    // AES-CCMデコードを開始する (MACはFIFOから入力する(pdata長に含めないこと))
    result = AES_StartCcmDec(&nonce, NULL, ADATA_LENGTH, PDATA_LENGTH, TRUE);
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_StartCcmDec (%d).\n", __func__, result);
    }

    // CPUで出力してみる
    result = AES_CpuRecv(dataB, sizeof(gs_data));   // adata + pdata
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_CpuRecv (%d).\n", __func__, result);
    }

    OS_TPrintf("%s: %d usec.\n", __func__, (u32)OS_TicksToMicroSeconds(OS_GetTick()-begin));

    // 出力結果の表示
    dump(__func__, dataB, sizeof(gs_data));

    result = AES_IsValid();
    if (AES_RESULT_SUCCESS_TRUE != result && AES_RESULT_SUCCESS_FALSE != result)
    {
        OS_TPrintf("%s: Failed to call AES_IsValid (%d).\n", __func__, result);
    }
    else
    {
        OS_TPrintf("Result: %s\n", AES_RESULT_SUCCESS_TRUE == result ? "Success" : "Failed");
    }
}

static void test2(void)
{
    OSTick begin;
    AESResult result;

    OS_TPrintf("\n%s() is starting.\n\n", __func__);

    // とりあえず、リセットする (鍵関係は何も変わらない (KEY_SELレジスタも消えるが意味は無い))
    result = AES_Reset();
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_Reset (%d).\n", __func__, result);
    }

    // 鍵を設定する
    result = AES_SelectKey(1);
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_SelectKey (%d).\n", __func__, result);
    }

    // 出力DMA設定
    MI_CpuClear32(dataB, sizeof(dataB));
    DC_FlushRange(dataB, sizeof(dataB));
    result = AES_StartDmaRecv(OUTPUT_DMA, dataB, sizeof(gs_data));   // adata + pdata
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_StartDmaSend (%d).\n", __func__, result);
    }

    begin = OS_GetTick();

    // AES-CCMデコードを開始する (MACはFIFOから入力する(pdata長に含めないこと))
    result = AES_StartCcmDec(&nonce, NULL, ADATA_LENGTH, PDATA_LENGTH, TRUE);
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_StartCcmDec (%d).\n", __func__, result);
    }

    // CPUで入力してみる
    AES_CpuSend(dataA, sizeof(gs_data) + sizeof(u128));   // adata + pdata + mac
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_CpuSend (%d).\n", __func__, result);
    }

    OS_TPrintf("%s: %d usec.\n", __func__, (u32)OS_TicksToMicroSeconds(OS_GetTick()-begin));

    // 出力結果の表示
    dump(__func__, dataB, sizeof(gs_data));

    result = AES_IsValid();
    if (AES_RESULT_SUCCESS_TRUE != result && AES_RESULT_SUCCESS_FALSE != result)
    {
        OS_TPrintf("%s: Failed to call AES_IsValid (%d).\n", __func__, result);
    }
    else
    {
        OS_TPrintf("Result: %s\n", AES_RESULT_SUCCESS_TRUE == result ? "Success" : "Failed (OK!)");
    }
}


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    AESResult result;

    OS_Init();

    OS_Printf("ARM9 starts.\n");

    OS_InitTick();

    OS_TPrintf("Debug Info:\n");
    OS_TPrintf("\tdataA = 0x%08X\n", dataA);
    OS_TPrintf("\tdataB = 0x%08X\n", dataB);

    AES_Init();
    OS_TPrintf("AES_Init wad done.\n");

    OS_EnableIrq();

    while (1)
    {
        result = AES_TryLock();
        if (AES_RESULT_SUCCESS_TRUE == result)
        {
            break;
        }
        if (AES_RESULT_SUCCESS_FALSE != result)
        {
            OS_TPrintf("%s: Failed to call AES_TryLock (%d).\n", __func__, result);
        }
        OS_Sleep(1);
    }
    OS_TPrintf("AES_TryLock wad done.\n");

    // 鍵を設定しておく
    result = AES_SetKey(0, &key);
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_SetKey (%d).\n", __func__, result);
    }
    result = AES_SetKey2(1, &key, &key2);
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_SetKey2 (%d).\n", __func__, result);
    }

    test0();
    test1();
    test2();

    result = AES_Unlock();
    if (AES_RESULT_SUCCESS != result)
    {
        OS_TPrintf("%s: Failed to call AES_Unlock (%d).\n", __func__, result);
    }

    // done
    OS_TPrintf("\nARM9 ends.\n");
    OS_Terminate();
}
