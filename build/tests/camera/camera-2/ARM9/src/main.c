/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - camera
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
#include <twl/camera.h>

#define DMA_NO  5           // �g�p����DMA�ԍ�(4-7)
#define DMA_IE  OS_IE_DMA5  // �Ή����銄�荞��
#define WIDTH   256         // �C���[�W�̕�
#define HEIGHT  192         // �C���[�W�̍���

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // ���̓]�����C����
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // �ꃉ�C���̓]���o�C�g��

#if 0
    �J�������瑗���Ă����f�[�^���A�t���[���P�ʂ�菬�����P�ʂŏ����������ꍇ�A
    CAMERA_DmaRecvInfinity() �����p�ł���B�Ƃ��������p���邵���Ȃ��B
    (DMA�Ȃ��ň��S�ɃJ��������̃f�[�^���擾�����i���Ȃ����Ƃɒ��ӁI�I�I)
#endif

static void VBlankIntr(void);
static void CameraDmaIntr(void);
static void CameraErrIntr(void);

static int lineNumber = 0;
static BOOL effectFlag = FALSE;
static u16 pipeBuffer[BYTES_PER_LINE * LINES_AT_ONCE / sizeof(u16)] ATTRIBUTE_ALIGN(32);
/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain()
{
    CAMERAResult result;

    // ������
    OS_Init();
    OS_InitThread();
    GX_Init();
    OS_InitTick();

    // V�u�����N���荞�ݐݒ�
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableInterrupts();

    // VRAM�\�����[�h
    GX_SetBankForLCDC(GX_VRAM_LCDC_A);
    MI_CpuClearFast((void *)HW_LCDC_VRAM_A, WIDTH*HEIGHT*2);
    GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, GX_BGMODE_0, GX_BG0_AS_2D);
    //GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
    OS_WaitVBlankIntr();
    GX_DispOn();

    // �J����������
    CAMERA_Init();      // wakeup camera module

    result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CActivate was failed. (%d)\n", result);
    }
#if 0
    result = CAMERA_I2CResize(CAMERA_SELECT_IN, 320, 240);
    if (result != CAMERA_RESULT_SUCCESS)
    {
        OS_TPrintf("CAMERA_I2CResize was failed. (%d)\n", result);
    }
#endif
    CAMERA_SetTrimmingParamsCenter(WIDTH, HEIGHT, 320, 240);    // clipped by camera i/f
    CAMERA_SetTrimming(TRUE);
    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // DMA���荞�ݐݒ�
    OS_SetIrqFunction(DMA_IE, CameraDmaIntr);
    (void)OS_EnableIrqMask(DMA_IE);

    // DMA�X�^�[�g (�����I�Ɏ~�߂Ȃ�����i���ɗL��)
    MIi_StopExDma(DMA_NO);
    CAMERA_DmaPipeInfinity(DMA_NO, pipeBuffer, BYTES_PER_LINE * LINES_AT_ONCE);

    // �J�������荞�ݐݒ�
    CAMERA_SetBufferErrorIntrrupt(TRUE);
    CAMERA_SetMasterIntrrupt(TRUE);
    OS_SetIrqFunction(OS_IE_CAM, CameraErrIntr);
    (void)OS_EnableIrqMask(OS_IE_CAM);

    // �J�����X�^�[�g
    lineNumber = 0;
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
    OS_TPrintf("Camera is shooting a movie...\n");

    while (1)
    {
        u16 pad;
        u16 trg;
        static u16 old = 0xffff;    // ignore the trigger by first data

        OS_WaitVBlankIntr();

        pad = PAD_Read();
        trg = (u16)(pad & ~old);
        old = pad;

        if (trg & PAD_BUTTON_A)
        {
            effectFlag ^= 1;
            OS_TPrintf("Effect %s\n", effectFlag ? "ON" : "OFF");
        }
    }
}

//--------------------------------------------------------------------------------
//    �u�u�����N���荞�ݏ���
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

//--------------------------------------------------------------------------------
//    DMA���荞�ݏ��� (�d�����H)
//
#define PRINT_RATE  32
void CameraDmaIntr(void)
{
    static BOOL effect = FALSE;
    OS_SetIrqCheckFlag(DMA_IE); // checking dma interrupt

    // �K�v�ȏ��������ăt���[���o�b�t�@�ɃR�s�[����
    if (effect)
    {
        int i;
        u32 *src = (u32*)pipeBuffer;
        u32 *dest = (u32*)HW_LCDC_VRAM_A + lineNumber * WIDTH / 2;
        for (i = 0; i < WIDTH * LINES_AT_ONCE / 2; i++) // pack 2 pixel data
        {
            dest[i] = ~src[i];  // reverse (if using RGBA format, should bitORed with 0x10001000)
        }
    }
    else
    {
        MI_CpuCopy8(pipeBuffer, (u16*)HW_LCDC_VRAM_A + lineNumber * WIDTH, sizeof(pipeBuffer));
    }
    DC_InvalidateRange(pipeBuffer, sizeof(pipeBuffer));

    lineNumber += LINES_AT_ONCE;
    if (lineNumber >= HEIGHT)
    {
        static OSTick begin = 0;
        static int count = 0;
        // �K�v�Ȃ�t���[���o�b�t�@�̃X���b�v�Ȃ�

        effect = effectFlag;    // ���̃^�C�~���O�Ŕ��f

        // debug print
        OS_TPrintf(".");
        if (begin == 0) // first time
        {
            begin = OS_GetTick();
        }
        else if (++count == PRINT_RATE)
        {
            OSTick uspf = OS_TicksToMicroSeconds(OS_GetTick() - begin) / count;
            int mfps = (int)(1000000000LL / uspf);
            OS_TPrintf("%2d.%03d fps\n", mfps / 1000, mfps % 1000);
            count = 0;
            begin = OS_GetTick();
        }

        lineNumber = 0;
    }
}

//--------------------------------------------------------------------------------
//    �J�������荞�ݏ��� (�G���[�������������Ȃ��͂�)
//
#if 0
    DMA�͕��u�v���C�ŗǂ��H
#endif
void CameraErrIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_CAM);  // checking camera interrupt

    OS_TPrintf("Error was occurred.\n");

    // �J������~
    CAMERA_StopCapture();
    lineNumber = 0;

    // �J�����ĊJ
    CAMERA_ClearBuffer();
    CAMERA_StartCapture();
}
