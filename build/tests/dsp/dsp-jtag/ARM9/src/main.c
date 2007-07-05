/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - tests - channel
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
#include <twl/dsp.h>

void    VBlankIntr(void);


#define WRAM_BNK_PACK( master, ofs, enable ) \
( \
    (((enable) != FALSE) * REG_MI_WRAM_A0_E_MASK) \
  | (ofs) \
  | (master) \
)

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    u16 Cont = 0xFFFF;

    // 初期化
    OS_Init();
    GX_Init();

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // USAGE
    OS_TPrintf("\n");
    OS_TPrintf("===================================\n");
    OS_TPrintf("START: DSP_ResetOn and DSP_ResetOff\n\n");
    OS_TPrintf("    A: DSP_ResetOff\n");
    OS_TPrintf("    B: DSP_ResetOn\n");
    OS_TPrintf("    X: DSP_ResetInterface\n");
    OS_TPrintf("===================================\n");

#if 0
    // DSP初期コード(JTAG接続待ち)の書き込み
    /* DSP_Iの先頭4Bを 0x5e47, 0x0000 (br ##0, ture)  にするとか？ */
    /* brr でも良いのか？ */
#endif

    // WRAMメモリマップ変更
    {
        reg_MI_WRAM_B0 = WRAM_BNK_PACK(MI_WRAM_B_DSP_I, MI_WRAM_B_OFS_0KB,   TRUE);
        reg_MI_WRAM_B1 = WRAM_BNK_PACK(MI_WRAM_B_DSP_I, MI_WRAM_B_OFS_32KB,  TRUE);
        reg_MI_WRAM_B2 = WRAM_BNK_PACK(MI_WRAM_B_DSP_I, MI_WRAM_B_OFS_64KB,  TRUE);
        reg_MI_WRAM_B3 = WRAM_BNK_PACK(MI_WRAM_B_DSP_I, MI_WRAM_B_OFS_96KB,  TRUE);
        reg_MI_WRAM_B4 = WRAM_BNK_PACK(MI_WRAM_B_DSP_I, MI_WRAM_B_OFS_128KB, TRUE);
        reg_MI_WRAM_B5 = WRAM_BNK_PACK(MI_WRAM_B_DSP_I, MI_WRAM_B_OFS_160KB, TRUE);
        reg_MI_WRAM_B6 = WRAM_BNK_PACK(MI_WRAM_B_DSP_I, MI_WRAM_B_OFS_192KB, TRUE);
        reg_MI_WRAM_B7 = WRAM_BNK_PACK(MI_WRAM_B_DSP_I, MI_WRAM_B_OFS_224KB, TRUE);
    }
    {
        reg_MI_WRAM_C0 = WRAM_BNK_PACK(MI_WRAM_C_DSP_D, MI_WRAM_C_OFS_0KB,   TRUE);
        reg_MI_WRAM_C1 = WRAM_BNK_PACK(MI_WRAM_C_DSP_D, MI_WRAM_C_OFS_32KB,  TRUE);
        reg_MI_WRAM_C2 = WRAM_BNK_PACK(MI_WRAM_C_DSP_D, MI_WRAM_C_OFS_64KB,  TRUE);
        reg_MI_WRAM_C3 = WRAM_BNK_PACK(MI_WRAM_C_DSP_D, MI_WRAM_C_OFS_96KB,  TRUE);
        reg_MI_WRAM_C4 = WRAM_BNK_PACK(MI_WRAM_C_DSP_D, MI_WRAM_C_OFS_128KB, TRUE);
        reg_MI_WRAM_C5 = WRAM_BNK_PACK(MI_WRAM_C_DSP_D, MI_WRAM_C_OFS_160KB, TRUE);
        reg_MI_WRAM_C6 = WRAM_BNK_PACK(MI_WRAM_C_DSP_D, MI_WRAM_C_OFS_192KB, TRUE);
        reg_MI_WRAM_C7 = WRAM_BNK_PACK(MI_WRAM_C_DSP_D, MI_WRAM_C_OFS_224KB, TRUE);
    }

    // DSP初期化 & Go
    OS_TPrintf("DSP_PowerOn...");
    DSP_PowerOn();
    OS_TPrintf("Done.\n");
    OS_TPrintf("DSP_ResetOff...");
    DSP_ResetOff();
    OS_TPrintf("Done.\n");

    while (1)
    {
        u16     ReadData;
        u16     Trg;

        OS_WaitVBlankIntr();

        ReadData = PAD_Read();
        Trg = (u16)(ReadData & ~Cont);
        Cont = ReadData;

        if (Trg & PAD_BUTTON_A)
        {
            OS_TPrintf("DSP_ResetOff...");
            DSP_ResetOff();
            OS_TPrintf("Done.\n");
        }
        if (Trg & PAD_BUTTON_B)
        {
            OS_TPrintf("DSP_ResetOn...");
            DSP_ResetOn();
            OS_TPrintf("Done.\n");
        }
        if (Trg & PAD_BUTTON_X)
        {
            OS_TPrintf("DSP_ResetInterface...");
            DSP_ResetInterface();
            OS_TPrintf("Done.\n");
        }

        if (Trg & PAD_BUTTON_START)
        {
            OS_TPrintf("DSP_ResetOn/DSP_ResetOff...");
            DSP_ResetOn();
#if 0
            // DSP初期コード(JTAG接続待ち)の再書き込み
            /* ここでやりたければ、一時的にWRAMメモリマップを変更する必要がある */
#endif
            DSP_ResetOff();
            OS_TPrintf("Done.\n");
        }
    }
}

//--------------------------------------------------------------------------------
//    Ｖブランク割り込み処理
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}
