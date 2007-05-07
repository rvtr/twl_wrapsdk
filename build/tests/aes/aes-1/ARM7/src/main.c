/*---------------------------------------------------------------------------*
  Project:  NitroSDK - AES - demos - _ARM7-aes-1
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

#define ENABLE_INTERRUPT_TEST

#define PRIORITY    5

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

static u32 dataA[(sizeof(gs_data) + sizeof(u128)) / sizeof(u32)] ATTRIBUTE_ALIGN(32);
static u32 dataB[(sizeof(gs_data) + sizeof(u128)) / sizeof(u32)] ATTRIBUTE_ALIGN(32);

//================================================================================
static void dump(const char *str, void *ptr, u32 length)
{
    u8 *data = (u8*)ptr;
    int i;
    OS_TPrintf("\n[%s]:\n\t", str);
    for (i = 0; i < length; i++) {
        OS_TPrintf("%02X", *data++);
        if ((i & 0xF) == 0xF) OS_TPrintf("\n\t");
        else    OS_TPrintf(" ");
    }
    OS_TPrintf("\n");
}

#define TEST0_USE_DMA_INPUT
//#define TEST0_USE_DMA_OUTPUT
static void test0(void)
{
    OSTick begin;

    OS_TPrintf("\n%s() is starting.\n\n", __func__);

    // とりあえず、リセットする (鍵関係は何も変わらない (KEY_SELレジスタも消えるが意味は無い))
    AES_Reset();

    // 鍵は0番目を使う
    AES_SelectKey(0);

#ifdef TEST0_USE_DMA_INPUT
    // 入力DMA設定
    AES_DmaSendAsync(INPUT_DMA, gs_data, sizeof(gs_data));    // adata + pdata
#endif
#ifdef TEST0_USE_DMA_OUTPUT
    // 出力DMA設定
    MI_CpuClear32(dataA, sizeof(dataA));
    AES_DmaRecvAsync(OUTPUT_DMA, dataA, sizeof(gs_data) + sizeof(u128));   // adata + pdata + mac
#endif

    begin = OS_GetTick();

    // AES-CCMエンコードを開始する
    AES_StartCcmEnc(&nonce, ADATA_LENGTH, PDATA_LENGTH, TRUE);

#ifdef TEST0_USE_DMA_INPUT
#ifdef TEST0_USE_DMA_OUTPUT
    // DMA完了待ち (AES完了待ちでも良いでしょう)
    MIi_WaitExDma(OUTPUT_DMA);
#else
    // CPUで出力してみる
    AES_CpuRecv(dataA, sizeof(gs_data) + sizeof(u128));   // adata + pdata + mac
#endif
#else
#ifdef TEST0_USE_DMA_OUTPUT
    // CPUで入力してみる
    AES_CpuSend(gs_data, sizeof(gs_data));   // adata + pdata
#else
#error "Does not support CPU input and CPU output at same time."
#endif
#endif

    OS_TPrintf("%s: %d usec.\n", __func__, (u32)OS_TicksToMicroSeconds(OS_GetTick()-begin));

    // 出力結果の表示
    dump(__func__, dataA, sizeof(gs_data));
}

static void test1(void)
{
    OSTick begin;

    OS_TPrintf("\n%s() is starting.\n\n", __func__);

    // とりあえず、リセットする (鍵関係は何も変わらない (KEY_SELレジスタも消えるが意味は無い))
    AES_Reset();

    // 鍵を設定しない (前回の値を使いまわす)

    // 入力DMA設定
    AES_DmaSendAsync(INPUT_DMA, dataA, sizeof(gs_data) + sizeof(u128));   // adata + pdata + mac

    // 出力バッファのクリア
    MI_CpuClear32(dataB, sizeof(dataB));

    begin = OS_GetTick();

    // AES-CCMデコードを開始する (MACはFIFOから入力する(pdata長に含めないこと))
    AES_StartCcmDec(&nonce, NULL, ADATA_LENGTH, PDATA_LENGTH, TRUE);

    // CPUで出力してみる
    AES_CpuRecv(dataB, sizeof(gs_data));   // adata + pdata

    OS_TPrintf("%s: %d usec.\n", __func__, (u32)OS_TicksToMicroSeconds(OS_GetTick()-begin));

    // 出力結果の表示
    dump(__func__, dataB, sizeof(gs_data));

    OS_TPrintf("Result: %s\n", AES_IsValid() ? "Success" : "Failed");
}

static void test2(void)
{
    OSTick begin;

    OS_TPrintf("\n%s() is starting.\n\n", __func__);

    // とりあえず、リセットする (鍵関係は何も変わらない (KEY_SELレジスタも消えるが意味は無い))
    AES_Reset();

    // 鍵を設定する
    AES_SelectKey(1);

    // 出力DMA設定
    MI_CpuClear32(dataB, sizeof(dataB));
    AES_DmaRecvAsync(OUTPUT_DMA, dataB, sizeof(gs_data));   // adata + pdata

    begin = OS_GetTick();

    // AES-CCMデコードを開始する (MACはFIFOから入力する(pdata長に含めないこと))
    AES_StartCcmDec(&nonce, NULL, ADATA_LENGTH, PDATA_LENGTH, TRUE);

    // CPUで入力してみる
    AES_CpuSend(dataA, sizeof(gs_data) + sizeof(u128));   // adata + pdata + mac

    OS_TPrintf("%s: %d usec.\n", __func__, (u32)OS_TicksToMicroSeconds(OS_GetTick()-begin));

    // 出力結果の表示
    dump(__func__, dataB, sizeof(gs_data));

    OS_TPrintf("Result: %s\n", AES_IsValid() ? "Success" : "Failed");
}

static u32 intrCounter[3];
static u8  aesID;
static u8  inputDmaID;
static u8  outputDmaID;

static void AesIntr(void)
{
    intrCounter[aesID]++;

    //---- check interrupt flag
    OS_SetIrqCheckFlag( OS_IE_AES );
}

static void InputDmaIntr(void)
{
    u32 ofs = INPUT_DMA - MI_EXDMA_CH_MIN;
    OSIrqMask mask = OS_IE_DMA4 << ofs;

    intrCounter[inputDmaID]++;

    //---- check interrupt flag
    OS_SetIrqCheckFlag( mask );
}

static void OutputDmaIntr(void)
{
    u32 ofs = OUTPUT_DMA - MI_EXDMA_CH_MIN;
    OSIrqMask mask = OS_IE_DMA4 << ofs;

    intrCounter[outputDmaID]++;

    //---- check interrupt flag
    OS_SetIrqCheckFlag( mask );
}

static void InitAesDmaIntr(void)
{
    u32 i_ofs = INPUT_DMA - MI_EXDMA_CH_MIN;
    u32 o_ofs = OUTPUT_DMA - MI_EXDMA_CH_MIN;
    OSIrqMask i_mask = OS_IE_DMA4 << i_ofs;
    OSIrqMask o_mask = OS_IE_DMA4 << o_ofs;
    u8 id_alloc = 0;

    BOOL ime = OS_DisableIrq();

    aesID       = id_alloc++;
    inputDmaID  = id_alloc++;
    outputDmaID = id_alloc++;

    (void)OS_DisableIrqMask( OS_IE_AES | i_mask | o_mask );
    (void)OS_ResetRequestIrqMask( OS_IE_AES | i_mask | o_mask );

    (void)OS_SetIrqFunction( OS_IE_AES, AesIntr );
    (void)OS_SetIrqFunction( i_mask, InputDmaIntr );
    (void)OS_SetIrqFunction( o_mask, OutputDmaIntr );
    (void)OS_EnableIrqMask( OS_IE_AES | i_mask | o_mask );

    (void)OS_RestoreIrq( ime );
}

static void PrintIntrCount(void)
{
    OS_TPrintf( "\ninterrupt count: aes = %d, input_dma = %d, output_dma = %d.\n", 
                intrCounter[aesID], intrCounter[inputDmaID], intrCounter[outputDmaID]);
}


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    OS_Init();

    OS_Printf("ARM7 starts.\n");

    OS_InitTick();

#ifdef ENABLE_INTERRUPT_TEST
    InitAesDmaIntr();
#endif

    OS_TPrintf("Debug Info:\n");
    OS_TPrintf("\tdataA = 0x%08X\n", dataA);
    OS_TPrintf("\tdataB = 0x%08X\n", dataB);

    AES_Init(PRIORITY);     // ARM9側からも利用するときのみ必要
    AES_Lock();             // ARM9側からも利用するときのみ必要
    OS_EnableIrq();         // ARM9側からも利用するときのみ必要

    // 鍵を設定しておく
    AES_SetKey(0, &key);
    AES_SetKey2(1, &key, &key2);

    test0();
    test1();
    test2();

    AES_Unlock();           // ARM9側からも利用するときのみ必要

#ifdef ENABLE_INTERRUPT_TEST
    PrintIntrCount();
#endif

    // done
    OS_TPrintf("\nARM7 ends.\n");
    OS_Terminate();
}
