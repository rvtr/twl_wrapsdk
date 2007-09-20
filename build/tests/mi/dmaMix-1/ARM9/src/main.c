/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - demos - allDma-1
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <../build/libraries/mi/common/include/mi_dma.h>


#define MY_DMA_WRAM   ((t_TestBuf *)HW_WRAM_1_END)
#define MY_TEST_LOOPS (sizeof(copyfillArg)/sizeof(t_CommonArg))
#define ONE_BUF_SIZE  (0x2000 - 4)

#define MY_DMA_CH_START  1
#define MY_DMA_CH_END    7

typedef struct
{
    u32 prePad __attribute__ ((aligned (32)));
    u16 src[8][ONE_BUF_SIZE/2]  __attribute__ ((aligned (32)));
    u16 dest[8][ONE_BUF_SIZE/2] __attribute__ ((aligned (32)));
    u32 PostPad __attribute__ ((aligned (32)));
}
t_TestBuf;

typedef struct
{
    u16 (*src)[ONE_BUF_SIZE/2];
    u16 (*dest)[ONE_BUF_SIZE/2];
    char *copyStr;
    char *fillStr;
}
t_CommonArg;


t_TestBuf testBuf __attribute__ ((aligned (32)));

t_CommonArg copyfillArg[] =
{
    { testBuf.src,      testBuf.dest,      "DmaCopy success on MAIN_MEM -> MAIN_MEM.\n", NULL, },
    { MY_DMA_WRAM->src, testBuf.dest,      "DmaCopy success on WRAM -> MAIN_MEM.\n",     "DmaFill success on MAIN_MEM.\n", },
    { testBuf.src,      MY_DMA_WRAM->dest, "DmaCopy success on MAIN_MEM -> WRAM.\n",     NULL, },
    { MY_DMA_WRAM->src, MY_DMA_WRAM->dest, "DmaCopy success on WRAM -> WRAM.\n",         "DmaFill success on WRAM.\n", },
};

t_CommonArg stopArg[] =
{
    { testBuf.src,      testBuf.dest,      "Stopping DmaCopy success on MAIN_MEM -> MAIN_MEM.\n", NULL, },
    { MY_DMA_WRAM->src, testBuf.dest,      "Stopping DmaCopy success on WRAM -> MAIN_MEM.\n",     "Stopping DmaFill success on MAIN_MEM.\n", },
    { testBuf.src,      MY_DMA_WRAM->dest, "Stopping DmaCopy success on MAIN_MEM -> WRAM.\n",     NULL, },
    { MY_DMA_WRAM->src, MY_DMA_WRAM->dest, "Stopping DmaCopy success on WRAM -> WRAM.\n",         "Stopping DmaFill success on WRAM.\n", },
};

t_CommonArg copyfillAsyncArg[] =
{
    { testBuf.src,      testBuf.dest,      "DmaCopyAsync success on MAIN_MEM -> MAIN_MEM.\n", NULL, },
    { MY_DMA_WRAM->src, testBuf.dest,      "DmaCopyAsync success on WRAM -> MAIN_MEM.\n",     "DmaFillAsync success on MAIN_MEM.\n", },
    { testBuf.src,      MY_DMA_WRAM->dest, "DmaCopyAsync success on MAIN_MEM -> WRAM.\n",     NULL, },
    { MY_DMA_WRAM->src, MY_DMA_WRAM->dest, "DmaCopyAsync success on WRAM -> WRAM.\n",         "DmaFillAsync success on WRAM.\n", },
};

u32 exDmaIntrCount[MI_EXDMA_CH_NUM];

void InitExDmaIntr(void);
void ClearIntrCount(void);
void PrintIntrCount(void);
void ExDma4Intr(void);
void ExDma5Intr(void);
void ExDma6Intr(void);
void ExDma7Intr(void);

static BOOL CheckDmaCopy( u32 dmaNo, void *src, void *dest, const char *str )
{
    BOOL ercd = TRUE;
    u8 *s = src;
    u8 *d = dest;
    int i;

    for (i=0; i<ONE_BUF_SIZE; i++)
    {
        if ( s[i] != d[i] )
        {
            OS_TPrintf( "error: DmaCopy failed address = 0x%x count = 0x%x dmaNo = %d.\n", &d[i], i/2, dmaNo );
            OS_TPrintf( "       src = 0x%02x dest = 0x%02x.\n", s[i], d[i] );
            ercd = FALSE;
            break;
        }
    }
    if (str)
    {
        OS_TPrintf( str );
    }

    return ercd;
}

static BOOL CheckDmaFill( u32 dmaNo, void *dest, u32 data, char *str )
{
    BOOL ercd = TRUE;
    u32 *d = dest;
    int i;

    for (i=0; i<ONE_BUF_SIZE/4; i++)
    {
        if ( data != d[i] )
        {
            OS_TPrintf( "error: DmaFill failed address = 0x%x count = 0x%x dmaNo = %d.\n", &d[i], i/2, dmaNo );
            OS_TPrintf( "       data = 0x%08x dest = 0x%08x.\n", data, d[i] );
            ercd = FALSE;
            break;
        }
    }
    if (str)
    {
        OS_TPrintf( str );
    }

    return ercd;
}


static BOOL CheckDmaCopyAndFill( t_CommonArg *arg, u32 data )
{
    u16 (*src)[ONE_BUF_SIZE/2] = arg->src;
    u16 (*dest)[ONE_BUF_SIZE/2] = arg->dest;
    char *copyStr = arg->copyStr;
    char *fillStr = arg->fillStr;
    BOOL c_ercd = TRUE, f_ercd = TRUE;
    u32 i, ii;

    for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
    {
        for (ii=0; ii<ONE_BUF_SIZE/2; ii++)
        {
            src[i][ii] = (u16)(ii+i-data);
        }
    }

    if ( copyStr )
    {
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            u16 *s = src[i];
            u16 *d = dest[i];
            char *str = NULL;

            DC_FlushAll();
            IC_InvalidateAll();
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                MIi_ExDmaCopy( ch, s, d, ONE_BUF_SIZE );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                MI_DmaCopy32( ch, s, d, ONE_BUF_SIZE );
            }
            if ( i == MY_DMA_CH_START )
            {
                str = copyStr;
            }
            c_ercd |= CheckDmaCopy( ch, s, d, str );
        }
    }

    if ( fillStr )
    {
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            u16 *d = dest[i];
            char *str = NULL;

            DC_FlushAll();
            IC_InvalidateAll();
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                 MIi_ExDmaFill( ch, d, data+i, ONE_BUF_SIZE );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                 MI_DmaFill32( ch, d, data+i, ONE_BUF_SIZE );
            }
            if ( i == MY_DMA_CH_START )
            {
                str = fillStr;
            }
            f_ercd |= CheckDmaFill( ch, d, data+i, str );
        }
    }

    return c_ercd | f_ercd;
}

static BOOL CheckDmaCopyAndFillAsync( t_CommonArg *arg, u32 data )
{
    u16 (*src)[ONE_BUF_SIZE/2] = arg->src;
    u16 (*dest)[ONE_BUF_SIZE/2] = arg->dest;
    char *copyStr = arg->copyStr;
    char *fillStr = arg->fillStr;
    BOOL c_ercd = TRUE, f_ercd = TRUE;
    u32 i, ii;

    for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
    {
        for (ii=0; ii<ONE_BUF_SIZE/2; ii++)
        {
            src[i][ii] = (u16)(ii+i-data);
        }
    }

    if ( copyStr )
    {
        DC_FlushAll();
        while ( GX_GetVCount() != 190 )
        {
        }
        while ( GX_GetVCount() != 191 )
        {
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            u16 *s = src[i];
            u16 *d = dest[i];

            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                MIi_ExDmaCopyAsync( ch, s, d, ONE_BUF_SIZE );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                MIi_DmaSetParams(ch, (u32)s, (u32)d, MI_CNT_VBCOPY32(ONE_BUF_SIZE) & ~MI_DMA_CONTINUOUS_ON);
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            BOOL bool = FALSE;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                bool = MIi_IsExDmaBusy( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                bool = MI_IsDmaBusy( ch );
            }
            if ( bool == FALSE )
            {
                OS_TPrintf( "warning: DmaCopyAsync isn't busy dmaNo = %d.\n", ch );
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                MIi_WaitExDma( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                MI_WaitDma( ch );
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            u16 *s = src[i];
            u16 *d = dest[i];
            char *str = NULL;

            if ( i == MY_DMA_CH_START )
            {
                str = copyStr;
            }
            c_ercd |= CheckDmaCopy( ch, s, d, str );
        }
    }

    if ( fillStr )
    {
        DC_FlushAll();
        while ( GX_GetVCount() != 190 )
        {
        }
        while ( GX_GetVCount() != 191 )
        {
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            u16 *d = dest[i];

            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                MIi_ExDmaFillAsync( ch, d, data+i, ONE_BUF_SIZE );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                MIi_DmaSetParams_src32(ch, data+i, (u32)d, (MI_CNT_VBCOPY32(ONE_BUF_SIZE) & ~MI_DMA_CONTINUOUS_ON) | MI_DMA_SRC_FIX);
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            BOOL bool = FALSE;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                bool = MIi_IsExDmaBusy( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                bool = MI_IsDmaBusy( ch );
            }
            if ( bool == FALSE )
            {
                OS_TPrintf( "warning: DmaFillAsync isn't busy dmaNo = %d.\n", ch );
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                MIi_WaitExDma( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                MI_WaitDma( ch );
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            u16 *d = dest[i];
            char *str = NULL;

            if ( i == MY_DMA_CH_START )
            {
                str = fillStr;
            }
            f_ercd |= CheckDmaFill( ch, d, data+i, str );
        }
    }

    return c_ercd | f_ercd;
}

static BOOL CheckDmaStop( t_CommonArg *arg )
{
    u16 (*src)[ONE_BUF_SIZE/2] = arg->src;
    u16 (*dest)[ONE_BUF_SIZE/2] = arg->dest;
    char *copyStr = arg->copyStr;
    char *fillStr = arg->fillStr;
    BOOL c_ercd = TRUE, f_ercd = TRUE;
    u32 i;

    if ( copyStr )
    {
        while ( GX_GetVCount() != 189 )
        {
        }
        while ( GX_GetVCount() != 190 )
        {
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            u16 *s = src[i];
            u16 *d = dest[i];

            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                MIi_ExDmaCopyAsync( ch, s, d, ONE_BUF_SIZE );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                MIi_DmaSetParams(ch, (u32)s, (u32)d, MI_CNT_VBCOPY32(ONE_BUF_SIZE) & ~MI_DMA_CONTINUOUS_ON);
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            BOOL bool = FALSE;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                bool = MIi_IsExDmaBusy( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                bool = MI_IsDmaBusy( ch );
            }
            if ( bool == FALSE )
            {
                OS_TPrintf( "warning: DmaCopyAsync isn't busy dmaNo = %d.\n", ch );
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                MIi_StopExDma( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                MI_StopDma( ch );
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            BOOL bool = FALSE;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                bool = MIi_IsExDmaBusy( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                bool = MI_IsDmaBusy( ch );
            }
            if ( bool == TRUE )
            {
                OS_TPrintf( "error: Stopping DmaCopy failed dmaNo = %d.\n", ch );
            }
        }
        if ( c_ercd == TRUE && copyStr )
        {
            OS_TPrintf( copyStr );
        }
    }

    if ( fillStr )
    {
        while ( GX_GetVCount() != 189 )
        {
        }
        while ( GX_GetVCount() != 190 )
        {
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            u16 *d = dest[i];

            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                MIi_ExDmaFillAsync( ch, d, i, ONE_BUF_SIZE );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                MIi_DmaSetParams_src32(ch, i, (u32)d, (MI_CNT_VBCOPY32(ONE_BUF_SIZE) & ~MI_DMA_CONTINUOUS_ON) | MI_DMA_SRC_FIX);
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            BOOL bool = FALSE;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                bool = MIi_IsExDmaBusy( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                bool = MI_IsDmaBusy( ch );
            }
            if ( bool == FALSE )
            {
                OS_TPrintf( "warning: DmaFillAsync isn't busy dmaNo = %d.\n", ch );
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                MIi_StopExDma( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                MI_StopDma( ch );
            }
        }
        for (i=MY_DMA_CH_END; i>=MY_DMA_CH_START; i--)
        {
            u32 ch = i;
            BOOL bool = FALSE;
            if (MI_EXDMA_CH_MIN <= ch && ch <= MI_EXDMA_CH_MAX)
            {
                bool = MIi_IsExDmaBusy( ch );
            }
            else if (ch < MI_EXDMA_CH_MIN)
            {
                bool = MI_IsDmaBusy( ch );
            }
            if ( bool == TRUE )
            {
                OS_TPrintf( "error: Stopping DmaFill failed dmaNo = %d.\n", ch );
            }
        }
        if ( f_ercd == TRUE && fillStr )
        {
            OS_TPrintf( fillStr );
        }
    }

    return c_ercd | f_ercd;
}

static void TestDmaFuncs( void )
{
    u32 i;

    ClearIntrCount();

    // sync copy and fill test
    OS_TPrintf( "\nChecking DmaCopy and DmaFill ....\n" );
    for (i=0; i<MY_TEST_LOOPS; i++)
    {
        (void)CheckDmaCopyAndFill( &copyfillArg[i], i );
    }

    // async copy and fill test
    OS_TPrintf( "\nChecking DmaCopyAsync and DmaFillAsync ....\n" );
    for (i=0; i<MY_TEST_LOOPS; i++)
    {
        (void)CheckDmaCopyAndFillAsync( &copyfillAsyncArg[i], i );
    }

    // stop test
    OS_TPrintf( "\nChecking DmaStop ....\n" );
    for (i=0; i<MY_TEST_LOOPS; i++)
    {
        (void)CheckDmaStop( &stopArg[i] );
    }

    PrintIntrCount();
}


//================================================================================
/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    OS_Init();

    InitExDmaIntr();

    OS_TPrintf("\nARM9 starts.\n");

//    OS_DisableProtectionUnit();

    // priority dma test
    OS_TPrintf( "\nTurn into Priority Mode.\n" );

    MIi_SetExDmaArbitration( MI_EXDMAGBL_ARB_PRIORITY );
    MIi_SetExDmaInterval( 4, 595, MI_EXDMA_PRESCALER_1 );
    MIi_SetExDmaInterval( 5, 580, MI_EXDMA_PRESCALER_1 );
    MIi_SetExDmaInterval( 6, 565, MI_EXDMA_PRESCALER_1 );
    MIi_SetExDmaInterval( 7, 550, MI_EXDMA_PRESCALER_1 );

    TestDmaFuncs();

    // round robin dma test
    OS_TPrintf( "\nTurn into Round Robin Mode.\n" );

    MIi_SetExDmaArbitration( MI_EXDMAGBL_ARB_ROUND_ROBIN );
    MIi_SetExDmaYieldCycles( MI_EXDMAGBL_YLD_CYCLE_DEFAULT );
    MIi_SetExDmaInterval( 4, 115, MI_EXDMA_PRESCALER_1 );
    MIi_SetExDmaInterval( 5, 111, MI_EXDMA_PRESCALER_1 );
    MIi_SetExDmaInterval( 6, 107, MI_EXDMA_PRESCALER_1 );
    MIi_SetExDmaInterval( 7, 104, MI_EXDMA_PRESCALER_1 );

    TestDmaFuncs();

    OS_TPrintf("\nARM9 ends.\n");
    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         InitExDmaIntr

  Description:  initialize extended dma interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void InitExDmaIntr(void)
{
    (void)OS_SetIrqFunction( OS_IE_DMA4, ExDma4Intr );
    (void)OS_SetIrqFunction( OS_IE_DMA5, ExDma5Intr );
    (void)OS_SetIrqFunction( OS_IE_DMA6, ExDma6Intr );
    (void)OS_SetIrqFunction( OS_IE_DMA7, ExDma7Intr );
    (void)OS_EnableIrqMask( OS_IE_DMA4 | OS_IE_DMA5 | OS_IE_DMA6 | OS_IE_DMA7 );
    (void)OS_EnableIrq();
}

/*---------------------------------------------------------------------------*
  Name:         ClearIntrCount

  Description:  clear interrupt counter

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void ClearIntrCount(void)
{
    int i;

    OS_ResetRequestIrqMask( OS_IE_DMA4 | OS_IE_DMA5 | OS_IE_DMA6 | OS_IE_DMA7 );

    for (i=0; i<MI_EXDMA_CH_NUM; i++)
    {
        exDmaIntrCount[i] = 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         PrintIntrCount

  Description:  print interrupt counter

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PrintIntrCount(void)
{
    OS_TPrintf( "\ninterrupt count: dma4 = %d, dma5 = %d, dma6 = %d, dma7 = %d.\n", 
                exDmaIntrCount[0], exDmaIntrCount[1], exDmaIntrCount[2], exDmaIntrCount[3]);
}

/*---------------------------------------------------------------------------*
  Name:         ExDmaIntr

  Description:  extended dma interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void ExDma4Intr(void)
{
    u32 ofs = 4 - MI_EXDMA_CH_MIN;

    exDmaIntrCount[ofs]++;

    //---- check interrupt flag
    OS_SetIrqCheckFlag( OS_IE_DMA4 );
}

void ExDma5Intr(void)
{
    u32 ofs = 5 - MI_EXDMA_CH_MIN;

    exDmaIntrCount[ofs]++;

    //---- check interrupt flag
    OS_SetIrqCheckFlag( OS_IE_DMA5 );
}

void ExDma6Intr(void)
{
    u32 ofs = 6 - MI_EXDMA_CH_MIN;

    exDmaIntrCount[ofs]++;

    //---- check interrupt flag
    OS_SetIrqCheckFlag( OS_IE_DMA6 );
}

void ExDma7Intr(void)
{
    u32 ofs = 7 - MI_EXDMA_CH_MIN;

    exDmaIntrCount[ofs]++;

    //---- check interrupt flag
    OS_SetIrqCheckFlag( OS_IE_DMA7 );
}

/*====== End of main.c ======*/
