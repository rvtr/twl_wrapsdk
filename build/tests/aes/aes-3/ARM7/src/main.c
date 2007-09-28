/*---------------------------------------------------------------------------*
  Project:  NitroSDK - AES - demos - aes-3
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

static const u128 key = {
    0x01234567,
    0x89abcdef,
    0x01234567,
    0x89abcdef
};
static const u128 iv = {
    0x89abcdef,
    0x01234567,
    0x89abcdef,
    0x01234567
};
static const u32 gs_data[] ATTRIBUTE_ALIGN(32) = {
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 0
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 2
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 4
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 6
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 8
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 10
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 12
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 14
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 16
    0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, // 18
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, // 20
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, // 22
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, // 24
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, // 26
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, // 28
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, // 30
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, // 32
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567  // 34
};  // 36 blocks

static u8 dataA[sizeof(gs_data)] ATTRIBUTE_ALIGN(32);
static u8 dataB[sizeof(gs_data)] ATTRIBUTE_ALIGN(32);

//================================================================================
static void dump(const char *str, const void *ptr, u32 length)
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

    OS_TPrintf("\n%s() is starting.\n\n", __func__);

    // とりあえず、リセットする (鍵関係は何も変わらない (KEY_SELレジスタも消えるが意味は無い))
    AES_Reset();

    // 鍵は0番目を使う
    AES_SelectKey(0);

#ifdef TEST0_USE_DMA_INPUT
    // 入力DMA設定
    AES_DmaSendAsync(INPUT_DMA, gs_data, sizeof(gs_data));
#endif
#ifdef TEST0_USE_DMA_OUTPUT
    // 出力DMA設定
    MI_CpuClear32(dataA, sizeof(dataA));
    AES_DmaRecvAsync(OUTPUT_DMA, dataA, sizeof(gs_data));
#endif

    begin = OS_GetTick();

    // AES-CTRエンコードを開始する
    AES_StartCtrEnc(&iv, sizeof(gs_data));

#ifdef TEST0_USE_DMA_INPUT
#ifdef TEST0_USE_DMA_OUTPUT
    // DMA完了待ち (AES完了待ちでも良いでしょう)
    MIi_WaitExDma(OUTPUT_DMA);
#else
    // CPUで出力してみる
    AES_CpuRecv(dataA, sizeof(gs_data));
#endif
#else
#ifdef TEST0_USE_DMA_OUTPUT
    // CPUで入力してみる
    AES_CpuSend(gs_data, sizeof(gs_data));
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
    AES_DmaSendAsync(INPUT_DMA, dataA, sizeof(gs_data));

    // 出力バッファのクリア
    MI_CpuClear32(dataB, sizeof(dataB));

    begin = OS_GetTick();

    // AES-CTRデコードを開始する
    AES_StartCtrDec(&iv, sizeof(gs_data));

    // CPUで出力してみる
    AES_CpuRecv(dataB, sizeof(gs_data));

    OS_TPrintf("%s: %d usec.\n", __func__, (u32)OS_TicksToMicroSeconds(OS_GetTick()-begin));

    // 出力結果の表示
    dump(__func__, dataB, sizeof(gs_data));
}
static void FATFSi_AddCounter(u128* pCounter, u32 nums)
{
    u32 data = 0;
    int i;
    u8* ptr = (u8*)pCounter;
    for (i = 0; i < 16; i++)
    {
        data += ptr[i] + (nums & 0xFF);
        ptr[i] = (u8)(data & 0xFF);
        data >>= 8;
        nums >>= 8;
        if ( !data && !nums )
        {
            break;
        }
    }
}
#define reg_AES_NONCE_CTR   (*( REGType128v *) REG_AES_NNC_CTR_ADDR)

static void test2(void)
{
    OSTick begin;
    u128 counter;

    OS_TPrintf("\n%s() is starting.\n\n", __func__);

    // とりあえず、リセットする (鍵関係は何も変わらない (KEY_SELレジスタも消えるが意味は無い))
    AES_Reset();

    // 鍵を設定しない (前回の値を使いまわす)

    // 出力バッファのクリア
    MI_CpuClear32(dataB, sizeof(dataB));

    // 入力DMA設定 (前半)
    AES_DmaSendAsync(INPUT_DMA, dataA, sizeof(gs_data)/2);

    begin = OS_GetTick();

    // AES-CTRデコードを開始する (前半)
    AES_StartCtrDec(&iv, sizeof(gs_data)/2);

    // CPUで出力してみる (前半)
    AES_CpuRecv(dataB, sizeof(gs_data)/2);

    OS_TPrintf("%s: %d usec.\n", __func__, (u32)OS_TicksToMicroSeconds(OS_GetTick()-begin));

    AES_Reset();

    // 入力DMA設定 (後半)
    AES_DmaSendAsync(INPUT_DMA, dataA + sizeof(gs_data)/2, sizeof(gs_data)/2);

    begin = OS_GetTick();

    // AES-CTRデコードを開始する (後半)
    MI_CpuCopy8(&iv, &counter, sizeof(u128));
    FATFSi_AddCounter(&counter, (sizeof(gs_data)/2) >> 4);
    reg_AES_NONCE_CTR = counter;

    //reg_AES_AES_PLD_LEN = (u16)((sizeof(gs_data)/2) >> 4);
    reg_AES_AESCNT = AES_INPUT_TYPE_WCNT_4 | AES_OUTPUT_TYPE_WCNT_4 |
                  REG_AES_AESCNT_I_MASK | REG_AES_AESCNT_E_MASK | AES_MODE_CTR;

    // CPUで出力してみる (前半)
    AES_CpuRecv(dataB + sizeof(gs_data)/2, sizeof(gs_data)/2);

    OS_TPrintf("%s: %d usec.\n", __func__, (u32)OS_TicksToMicroSeconds(OS_GetTick()-begin));

    // 出力結果の表示
    dump(__func__, dataB, sizeof(gs_data));
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
    AES_SetSeed(1, &key);

    test0();
    test1();
    test2();

    AES_Unlock();           // ARM9側からも利用するときのみ必要

#ifdef ENABLE_INTERRUPT_TEST
    PrintIntrCount();
#endif

    // done
    OS_TPrintf("\nARM7 ends.\n");
    //OS_Terminate();
}
