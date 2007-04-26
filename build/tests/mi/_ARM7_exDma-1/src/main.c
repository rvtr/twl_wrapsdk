/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - demos - exDma-1
  File:     main.c

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <twl.h>


#define ONE_BUF_SIZE  0x2004

typedef struct
{
    u32 prePad __attribute__ ((aligned (32)));
    u16 src[4][ONE_BUF_SIZE/2]  __attribute__ ((aligned (32)));
    u16 dest[4][ONE_BUF_SIZE/2] __attribute__ ((aligned (32)));
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
    { testBuf.src,      testBuf.dest,      "DmaCopy success.\n", "DmaFill success on WRAM.\n", },
};

t_CommonArg stopArg[] =
{
    { testBuf.src,      testBuf.dest,      "Stopping DmaCopy success.\n", "Stopping DmaFill success on WRAM.\n", },
};

t_CommonArg copyfillAsyncArg[] =
{
    { testBuf.src,      testBuf.dest,      "DmaCopyAsync success.\n", "DmaFillAsync success on WRAM.\n", },
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

    for (i=0; i<4; i++)
    {
        for (ii=0; ii<ONE_BUF_SIZE/2; ii++)
        {
            src[i][ii] = (u16)(ii+i-data);
        }
    }
    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        u16 *s = src[i];
        u16 *d = dest[i];
        char *str = NULL;

        MIi_ExDmaCopy( ch, s, d, ONE_BUF_SIZE );
        if ( i == 3 )
        {
            str = copyStr;
        }
        c_ercd |= CheckDmaCopy( ch, s, d, str );
    }

    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        u16 *d = dest[i];
        char *str = NULL;

        MIi_ExDmaFill( ch, d, data+i, ONE_BUF_SIZE );
        if ( i == 3 )
        {
            str = fillStr;
        }
        f_ercd |= CheckDmaFill( ch, d, data+i, str );
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

    for (i=0; i<4; i++)
    {
        for (ii=0; ii<ONE_BUF_SIZE/2; ii++)
        {
            src[i][ii] = (u16)(ii+i-data);
        }
    }
    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        u16 *s = src[i];
        u16 *d = dest[i];

        MIi_ExDmaCopyAsync( ch, s, d, ONE_BUF_SIZE );
    }
    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        if ( MIi_IsExDmaBusy( ch ) == FALSE )
        {
            OS_TPrintf( "warning: DmaCopyAsync isn't busy dmaNo = %d.\n", ch );
        }
    }
    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        MIi_WaitExDma( ch );
    }
    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        u16 *s = src[i];
        u16 *d = dest[i];
        char *str = NULL;

        if ( i == 3 )
        {
            str = copyStr;
        }
        c_ercd |= CheckDmaCopy( ch, s, d, str );
    }

    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        u16 *d = dest[i];

        MIi_ExDmaFillAsync( ch, d, data+i, ONE_BUF_SIZE );
    }
    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        if ( MIi_IsExDmaBusy( ch ) == FALSE )
        {
            OS_TPrintf( "warning: DmaFillAsync isn't busy dmaNo = %d.\n", ch );
        }
    }
    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        MIi_WaitExDma( ch );
    }
    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        u16 *d = dest[i];
        char *str = NULL;

        if ( i == 3 )
        {
            str = fillStr;
        }
        f_ercd |= CheckDmaFill( ch, d, data+i, str );
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

    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        u16 *s = src[i];
        u16 *d = dest[i];

        MIi_ExDmaCopyAsync( ch, s, d, ONE_BUF_SIZE );
        if ( MIi_IsExDmaBusy( ch ) == FALSE )
        {
            OS_TPrintf( "warning: DmaCopyAsync isn't busy dmaNo = %d.\n", ch );
        }
        MIi_StopExDma( ch );

        if ( MIi_IsExDmaBusy( ch ) == TRUE )
        {
            OS_TPrintf( "error: Stopping DmaCopy failed dmaNo = %d.\n", ch );
            c_ercd = FALSE;
//            break;
        }
    }
    if ( c_ercd == TRUE )
    {
        OS_TPrintf( copyStr );
    }

    for (i=0; i<4; i++)
    {
        u32 ch = i + MI_EXDMA_CH_MIN;
        u16 *d = dest[i];

        MIi_ExDmaFillAsync( ch, d, i, ONE_BUF_SIZE );
        if ( MIi_IsExDmaBusy( ch ) == FALSE )
        {
            OS_TPrintf( "warning: DmaFillAsync isn't busy dmaNo = %d.\n", ch );
        }
        MIi_StopExDma( ch );

        if ( MIi_IsExDmaBusy( ch ) == TRUE )
        {
            OS_TPrintf( "error: Stopping DmaFill failed dmaNo = %d.\n", ch );
            f_ercd = FALSE;
//            break;
        }
    }
    if ( f_ercd == TRUE )
    {
        OS_TPrintf( fillStr );
    }

    return c_ercd | f_ercd;
}

static void TestDmaFuncs( void )
{
    u32 i;

    ClearIntrCount();

    // sync copy and fill test
    OS_TPrintf( "\nChecking DmaCopy and DmaFill ....\n" );
    for (i=0; i<1; i++)
    {
        (void)CheckDmaCopyAndFill( &copyfillArg[i], i );
    }

    // async copy and fill test
    OS_TPrintf( "\nChecking DmaCopyAsync and DmaFillAsync ....\n" );
    for (i=0; i<1; i++)
    {
        (void)CheckDmaCopyAndFillAsync( &copyfillAsyncArg[i], i );
    }

    // stop test
    OS_TPrintf( "\nChecking DmaStop ....\n" );
    for (i=0; i<1; i++)
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

    OS_TPrintf("\nARM7 starts.\n");

    // priority dma test
    OS_TPrintf( "\nTurn into Priority Mode.\n" );

    MIi_SetExDmaArbiter( MI_EXDMAGBL_ARB_PRIORITY, MI_EXDMAGBL_YLD_CYCLE_DEFAULT );

    TestDmaFuncs();

    // round robin dma test
    OS_TPrintf( "\nTurn into Round Robin Mode.\n" );

    MIi_SetExDmaArbiter( MI_EXDMAGBL_ARB_ROUND_ROBIN, MI_EXDMAGBL_YLD_CYCLE_DEFAULT );

    TestDmaFuncs();

    OS_TPrintf("\nARM7 ends.\n");
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
