/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - snd - channel
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

#include    <twl_sp.h>
#include    <twl/cdc.h>     // for DS mode
#include    <twl/snd/ARM7/i2s.h>

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
// ===== �X���b�h�D��x =====

#define THREAD_PRIO_SPI         2
#define THREAD_PRIO_SND         6

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystem(void);
static void VBlankIntr(void);

/*---------------------------------------------------------------------------*
  Name:         TwlSpMain

  Description:  Initialize and do main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlSpMain(void)
{
    OSHeapHandle heapHandle;

    // OS������
    OS_Init();
    OS_InitThread();

    // PXI�������AARM9�Ɠ���
    PXI_Init();

    // �q�[�v�̈�ݒ�
    heapHandle = InitializeAllocateSystem();

    // �����݋���
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // �T�E���h������
    SND_Init(THREAD_PRIO_SND);

#if 0
    // DS mode
    SND_Disable();
    SND_I2SSetSamplingRatio(FALSE); // 32kHz
    CDC_GoDsMode();
    SND_Enable();
#endif

//    reg_CFG_DS_MDFY |= REG_CFG_DS_MDFY_SND_MASK;  // SOUND��H�o�O�C�� (default: off)
//    reg_CFG_DS_MDFY |= REG_CFG_DS_MDFY_SDMA_MASK; // SOUND-DMA�o�O�C�� (default: off)
//    reg_CFG_DS_EX &= ~REG_CFG_DS_EX_SDMA2_MASK;   // SOUND-DMA�V��H (default: on)

    // �{�^�����̓T�[�`������
    (void)PAD_InitXYButton();

    // SPI������
    SPI_Init(THREAD_PRIO_SPI);

    while (TRUE)
    {
        OS_Halt();

        //---- check reset
        if (OS_IsResetOccurred())
        {
            OS_ResetSystem();
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  �����������ăV�X�e��������������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystem(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    OS_TPrintf("OS_GetWramSubPrivArenaLo() = %p\n", OS_GetWramSubPrivArenaLo());
    OS_TPrintf("OS_GetWramSubPrivArenaHi() = %p\n", OS_GetWramSubPrivArenaHi());
    OS_TPrintf("OS_GetWramSubArenaLo() = %p\n", OS_GetWramSubArenaLo());
    OS_TPrintf("OS_GetWramSubArenaHi() = %p\n", OS_GetWramSubArenaHi());
    OS_TPrintf("OS_GetSubPrivArenaLo() = %p\n", OS_GetSubPrivArenaLo());
    OS_TPrintf("OS_GetSubPrivArenaHi() = %p\n", OS_GetSubPrivArenaHi());

    OS_TPrintf("call OS_SetWramSubPrivArenaHi(0x0380f980); to fix arena.\n");
    OS_SetWramSubPrivArenaHi((void*)0x0380f980);

    // �����������ď�����
    tempLo = OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV,
                          OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi(), 1);

    // �A���[�i��0�N���A
    MI_CpuClear8(tempLo, (u32)OS_GetWramSubPrivArenaHi() - (u32)tempLo);

    // �A���[�i���ʃA�h���X��ݒ�
    OS_SetArenaLo(OS_ARENA_WRAM_SUBPRIV, tempLo);

    // �q�[�v�쐬
    hh = OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV,
                       OS_GetWramSubPrivArenaLo(), OS_GetWramSubPrivArenaHi());

    if (hh < 0)
    {
        OS_Panic("ARM7: Fail to create heap.\n");
    }

    // �J�����g�q�[�v�ɐݒ�
    (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);

    return hh;
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef SDK_TEG

extern BOOL PMi_Initialized;
void    PM_SelfBlinkProc(void);

static void VBlankIntr(void)
{
    //---- LED blink system
    if (PMi_Initialized)
    {
        PM_SelfBlinkProc();
    }
}

#else

static void VBlankIntr(void)
{
}

#endif

/*---------------------------------------------------------------------------*
    End of file
 *---------------------------------------------------------------------------*/
