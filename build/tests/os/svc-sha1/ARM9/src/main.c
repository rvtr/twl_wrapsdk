/*---------------------------------------------------------------------------*
  Project:  NitroSDK - OS - demos - svc-sha1
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

static const u32 gs_data[] ATTRIBUTE_ALIGN(32) = {
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
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567,
    0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567, 0x89abcdef, 0x01234567
};

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

/*
    SHA1 API
*/
static inline void SHA1_Init(SVCSHA1Context *ctx)
{
    if (ctx == NULL)
        return;

//    ctx->sha_block = NULL;
//    MI_CpuClear8(ctx, sizeof(SVCSHA1Context));
    SVC_SHA1Init(ctx);
}

static inline void SHA1_Update(SVCSHA1Context *ctx, const void* data, u32 len)
{
    if (ctx == NULL)
        return;
    if (len > 0 && data == NULL)
        return;
    SVC_SHA1Update(ctx, data, len);
}

static inline void SHA1_GetHash(SVCSHA1Context *ctx, u8* md)
{
    if (ctx == NULL)
        return;
    if (md == NULL)
        return;
    SVC_SHA1GetHash(ctx, md);
}

static inline void SHA1_Calc(u8* md, const void* data, u32 len)
{
    SVC_CalcSHA1(md, data, len);
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

    OS_Printf("ARM9 starts.\n");

    OS_InitTick();
    OS_EnableIrq();

    dump("Source:", gs_data, sizeof(gs_data));

    /*
        TEST1: 一度にSHA1を計算する
    */
    DC_FlushAll();  // 条件をそろえる
    {
        u8 md[20];
        OSTick begin = OS_GetTick();
        SHA1_Calc(md, gs_data, sizeof(gs_data));    // calc sha1 at once
        OS_TPrintf("\nSHA1_Calc was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));
        dump("Test1 Result:", md, sizeof(md));
    }

    /*
        TEST2: 複数回に分けてSHA1を計算する
    */
    DC_FlushAll();  // 条件をそろえる
    {
        u8 md[20];
        SVCSHA1Context ctx;
        OSTick begin = OS_GetTick();
        SHA1_Init(&ctx);
        OS_TPrintf("\nSHA1_Init was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));

        begin = OS_GetTick();
        SHA1_Update(&ctx, gs_data, sizeof(gs_data));
        OS_TPrintf("\n1st SHA1_Update was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));

        begin = OS_GetTick();
        SHA1_Update(&ctx, gs_data, sizeof(gs_data));
        OS_TPrintf("\n2nd SHA1_Update was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));

        begin = OS_GetTick();
        SHA1_GetHash(&ctx, md);
        OS_TPrintf("\nSHA1_GetHash was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));
        dump("Test2 Result:", md, sizeof(md));
    }

    /*
        TEST3: 複数回に分けてSHA1を計算する (DGTライブラリ編)
    */
    DC_FlushAll();  // 条件をそろえる
    {
        u8 md[20];
        DGTHash2Context ctx;
        OSTick begin = OS_GetTick();
        DGT_Hash2Reset(&ctx);
        OS_TPrintf("\nDGT_Hash2Reset was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));

        begin = OS_GetTick();
        DGT_Hash2SetSource(&ctx, (const u8*)gs_data, sizeof(gs_data));
        OS_TPrintf("\n1st DGT_Hash2SetSource was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));

        begin = OS_GetTick();
        DGT_Hash2SetSource(&ctx, (const u8*)gs_data, sizeof(gs_data));
        OS_TPrintf("\n2nd DGT_Hash2SetSource was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));

        begin = OS_GetTick();
        DGT_Hash2GetDigest(&ctx, md);
        OS_TPrintf("\nDGT_Hash2GetDigest was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));
        dump("Test3 Result:", md, sizeof(md));
    }

    /*
        TEST4: SHA1を利用して、偏ったデータを一様にばらけさせる (入出力サイズは任意)
               SHA1を利用しているので出力データの先頭160bitが同じなら以降も同じになる
               うまくラップすれば疑似乱数として使用できるが、直接SVC_SHA1*を利用した方がよいかも
               名前はSVC_EqualizeSHA1の方が良いかな？
    */
    DC_FlushAll();  // 条件をそろえる
    {
        u8 md[20];
        OSTick begin = OS_GetTick();
        SVC_RandomSHA1(md, sizeof(md), gs_data, sizeof(gs_data));   // map gs_data to md
        OS_TPrintf("\nSVC_RandomSHA1 was consumed %d usec\n", (int)OS_TicksToMicroSeconds(OS_GetTick()-begin));
        dump("Test4 Result:", md, sizeof(md));
    }

    // done
    OS_TPrintf("\nARM9 ends.\n");
    OS_Terminate();
}
