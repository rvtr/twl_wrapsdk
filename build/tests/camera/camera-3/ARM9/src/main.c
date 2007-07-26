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

#define DMA_NO      5           // �g�p����DMA�ԍ�(4-7)
#define WIDTH       256         // �C���[�W�̕�
#define HEIGHT      192         // �C���[�W�̍���

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // ���̓]�����C����
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // �ꃉ�C���̓]���o�C�g��

static void VBlankIntr(void);
static void CameraIntr(void);

static BOOL startRequest = FALSE;

#if 0
    �`������\����FPS���傫���O��ł̃g���v���o�b�t�@�V�[�P���X
#endif

static int wp;  // �J��������f�[�^����荞�ݒ��̃o�b�t�@
static int rp;  // �\�����̃o�b�t�@
static BOOL wp_pending; // ��荞�݂𒆒f���� (�Ăѓ����o�b�t�@�Ɏ�荞��)
static GXDispMode dispMode[] =
{
    GX_DISPMODE_VRAM_A,
    GX_DISPMODE_VRAM_C,
    GX_DISPMODE_VRAM_D
};
static void* dispAddr[] =
{
    (void*)HW_LCDC_VRAM_A,
    (void*)HW_LCDC_VRAM_C,
    (void*)HW_LCDC_VRAM_D
};
static void DebugReport(void)
{
    const char* const str[] =
    {
        "VRAM[A]",
        "VRAM[C]",
        "VRAM[D]"
    };
    OSIntrMode enabled = OS_DisableInterrupts();
    int wp_bak = wp;
    int rp_bak = rp;
    OS_RestoreInterrupts(enabled);
    OS_TPrintf("\nCapture to %s\tDisplay from %s\n", str[wp_bak], str[rp_bak]);
}
static GXDispMode GetNextDispMode(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    GXDispMode mode;
    int next = (rp + 1) % 3;
    if (next != wp) // ���o�b�t�@���������ݒ��łȂ�
    {
        rp = next;
    }
    mode = dispMode[rp];
    OS_RestoreInterrupts(enabled);
    //DebugReport();
    return mode;
}
static void* GetNextCaptureAddr(void)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    void* addr;
    int next = (wp + 1) % 3;
    if (wp_pending)         // �����o�b�t�@�Ɏ�荞�݂�����
    {
        wp_pending = FALSE;
    }
    else if (next != rp)    // ���o�b�t�@���ǂݍ��ݒ��łȂ�
    {
        wp = next;
    }
    else    // ��荞�ނׂ��o�b�t�@���Ȃ� (fps����݂Ă��蓾�Ȃ�)
    {
        OS_TPrintf("ERROR: there is no unused frame to capture.\n");
    }
    addr = dispAddr[wp];
    OS_RestoreInterrupts(enabled);
    //DebugReport();
    return addr;
}
static void PendingCapture(void)
{
    wp_pending = TRUE;
}
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
    GX_SetBankForLCDC(GX_VRAM_LCDC_C);
    GX_SetBankForLCDC(GX_VRAM_LCDC_D);
    MI_CpuClearFast(dispAddr[1], WIDTH*HEIGHT*2);   // first display point (before to capture)
    MI_CpuClearFast(dispAddr[2], WIDTH*HEIGHT*2);   // second display point (before to capture)
    wp = 0;
    rp = 2;
    wp_pending = TRUE;
    GX_SetGraphicsMode(GetNextDispMode(), GX_BGMODE_0, GX_BG0_AS_2D);
    //GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
    OS_WaitVBlankIntr();
    GX_DispOn();

    // �J����������
    CAMERA_Init();      // wakeup camera module

    result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
    if (result != CAMERA_RESULT_SUCCESS_TRUE)
    {
        OS_TPrintf("CAMERA_I2CActivate was failed. (%d)\n", result);
    }
#if 0
    result = CAMERA_I2CResize(CAMERA_SELECT_IN, 320, 240);
    if (result != CAMERA_RESULT_SUCCESS_TRUE)
    {
        OS_TPrintf("CAMERA_I2CResize was failed. (%d)\n", result);
    }
#endif
    CAMERA_SetTrimmingParamsCenter(WIDTH, HEIGHT, 320, 240);    // clipped by camera i/f
    CAMERA_SetTrimming(TRUE);
    CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
    CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));

    // �J�������荞�ݐݒ�
    //CAMERA_SetVsyncIntrrupt(CAMERA_INTR_VSYNC_POSITIVE_EDGE);   // almost end of vblank
    CAMERA_SetVsyncIntrrupt(CAMERA_INTR_VSYNC_NEGATIVE_EDGE);   // almost begin of vblank
    CAMERA_SetBufferErrorIntrrupt(TRUE);
    CAMERA_SetMasterIntrrupt(TRUE);
    OS_SetIrqFunction(OS_IE_CAM, CameraIntr);
    (void)OS_EnableIrqMask(OS_IE_CAM);

    // �J�����X�^�[�g (���N�G�X�g�̂�)
    startRequest = TRUE;
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
            DebugReport();
        }
    }
}

//--------------------------------------------------------------------------------
//    �u�u�����N���荞�ݏ���
//
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
    GX_SetGraphicsMode(GetNextDispMode(), GX_BGMODE_0, GX_BG0_AS_2D);
}

//--------------------------------------------------------------------------------
//    �J�������荞�ݏ��� (�G���[����Vsync���̗����Ŕ���)
//
#define PRINT_RATE  32
void CameraIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_CAM); // checking camera interrupt

    if (CAMERA_GetErrorStatus())    // error?
    {
        OS_TPrintf("Error was occurred.\n");
        // ��~����
        CAMERA_StopCapture();   // �J������~
        CAMERA_ClearBuffer();   // �N���A (�����ɂł���H)
        MIi_StopExDma(DMA_NO);  // DMA��~
        PendingCapture();       // ����������t���[�����g�p����
        startRequest = TRUE;    // �J�����ĊJ�v��
        return;     // waiting next frame (skip current frame)
    }

    // �ȍ~��Vsync���̏���

    if (startRequest)
    {
        CAMERA_ClearBuffer();
        CAMERA_StartCapture();
        startRequest = FALSE;
    }

    if (CAMERA_IsBusy() == FALSE)   // done to execute stop command?
    {
        //OS_TPrintf("while stopping the capture or just finished\n");
    }
    else
    {
        OS_TPrintf(".");
        if (MIi_IsExDmaBusy(DMA_NO))    // NOT done to capture last frame?
        {
            OS_TPrintf("DMA was not done until VBlank.\n");
            return; // waiting next frame (skip current frame)
        }
        // start to capture for next frame
        CAMERA_DmaRecvAsync(DMA_NO, GetNextCaptureAddr(), BYTES_PER_LINE * LINES_AT_ONCE, BYTES_PER_LINE * HEIGHT);
    }

    // debug print
    {
        static OSTick begin = 0;
        static int count = 0;
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
    }
}
