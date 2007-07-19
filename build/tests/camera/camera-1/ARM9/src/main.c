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
#define NUMS        1           // �o�b�t�@��

#define LINES_AT_ONCE   CAMERA_GET_MAX_LINES(WIDTH)     // ���̓]�����C����
#define BYTES_PER_LINE  CAMERA_GET_LINE_BYTES(WIDTH)    // �ꃉ�C���̓]���o�C�g��

static void VBlankIntr(void);
static void CameraIntr(void);

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
    CAMERA_Init();      // create camera thread
    CAMERA_PowerOn();   // wakeup camera module

    result = CAMERA_I2CInit(CAMERA_SELECT_IN);
    if (result != CAMERA_RESULT_SUCCESS_TRUE)
    {
        OS_TPrintf("CAMERA_I2CInit was failed. (%d)\n", result);
        CAMERA_PowerOff();
        OS_Terminate();
    }
    OS_TPrintf("CAMERA_I2CInit was done successfully.\n");

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

    // �J�����X�^�[�g (DMA�͂����ŊJ�n���Ă����Ȃ��Ă��ǂ�)
    CAMERA_ClearBuffer();
    CAMERA_DmaRecvAsync(DMA_NO, (void *)HW_LCDC_VRAM_A, BYTES_PER_LINE * LINES_AT_ONCE, BYTES_PER_LINE * HEIGHT);
    CAMERA_Start();
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
            OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_IN)... ");
            result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
        }
        if (trg & PAD_BUTTON_B)
        {
            OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_OUT)... ");
            result = CAMERA_I2CActivate(CAMERA_SELECT_OUT);
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
        }
        if (trg & PAD_BUTTON_R)
        {
            OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_NONE)... ");
            result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
            OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
        }
        if (trg & PAD_BUTTON_START) // start/stop to capture
        {
            if (OS_GetIrqMask() & OS_IE_CAM)
            {
                // clear the handler
                (void)OS_DisableIrqMask(OS_IE_CAM);
                OS_TPrintf("call CAMERA_Stop()... ");
                CAMERA_Stop();
                while (CAMERA_IsBusy())
                {
                }
                OS_TPrintf("Camera was stopped.\n");
            }
            else
            {
                // set the handler
                OS_SetIrqFunction(OS_IE_CAM, CameraIntr);
                (void)OS_EnableIrqMask(OS_IE_CAM);

                CAMERA_ClearBuffer();
                CAMERA_Start();
                OS_TPrintf("Camera is shooting a movie...\n");
            }
        }
        if (trg & PAD_BUTTON_SELECT)    // power on/off
        {
            if (reg_CFG_CLK & REG_CFG_CLK_CAM_CKI_MASK)
            {
                OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_NONE)... ");
                result = CAMERA_I2CActivate(CAMERA_SELECT_NONE);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
                OS_TPrintf("call CAMERA_PowerOff()... ");
                CAMERA_PowerOff();
                OS_TPrintf("Done.\n");
            }
            else
            {
                OS_TPrintf("call CAMERA_PowerOn()... ");
                CAMERA_PowerOn();
                OS_TPrintf("Done.\n");
                OS_TPrintf("call CAMERA_I2CInit(CAMERA_SELECT_IN)... ");
                result = CAMERA_I2CInit(CAMERA_SELECT_IN);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
                // resize
                OS_TPrintf("call CAMERA_I2CResize(CAMERA_SELECT_IN)... ");
                result = CAMERA_I2CResize(CAMERA_SELECT_IN, 320, 240);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
                // activate inside camera first
                OS_TPrintf("call CAMERA_I2CActivate(CAMERA_SELECT_IN)... ");
                result = CAMERA_I2CActivate(CAMERA_SELECT_IN);
                OS_TPrintf("%s\n", result == CAMERA_RESULT_SUCCESS_TRUE ? "SUCCESS" : "FAILED");
                // set misc registers
                CAMERA_SetTrimmingParamsCenter(WIDTH, HEIGHT, 320, 240);    // clipped by camera i/f
                CAMERA_SetTrimming(TRUE);
                CAMERA_SetOutputFormat(CAMERA_OUTPUT_RGB);
                CAMERA_SetTransferLines(CAMERA_GET_MAX_LINES(WIDTH));
                CAMERA_SetVsyncIntrrupt(CAMERA_INTR_VSYNC_POSITIVE_EDGE);   // almost end of vblank
                CAMERA_SetBufferErrorIntrrupt(TRUE);
                CAMERA_SetMasterIntrrupt(TRUE);
            }
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
//    �J�������荞�ݏ��� (�G���[����Vsync���̗����Ŕ���)
//
#if 0
    Start����DMA���J�n���Ȃ��ꍇ�AStart�̃^�C�~���O�ɂ���ẮA�ŏ��ɃG���[���m����n�܂�B
    Start����DMA���J�n���Ă���ꍇ�AStart�̃^�C�~���O�ɂ���ẮADMA����~���Ă��Ȃ�����n�܂�B
    DMA����̊Ǘ����J������Vsync����DMA���荞�݂ɕς��Ă��ǂ������B
#endif
#define PRINT_RATE  32
void CameraIntr(void)
{
    OS_TPrintf(".");
    OS_SetIrqCheckFlag(OS_IE_CAM); // checking camera interrupt

    if (CAMERA_GetErrorStatus())    // error?
    {
        OS_TPrintf("Error was occurred.\n");
        // ��~����
        CAMERA_Stop();          // �J������~
        MIi_StopExDma(DMA_NO);  // DMA��~
        while (CAMERA_IsBusy()) //        �҂�
        {
        }
        // �J�����ĊJ
        CAMERA_ClearBuffer();
        CAMERA_Start();
        return; // waiting next frame (skip current frame)
    }

    // �ȍ~��Vsync���̏���

    if (MIi_IsExDmaBusy(DMA_NO))    // NOT done to capture last frame?
    {
        OS_TPrintf("DMA was not done until VBlank.\n");
        return; // waiting next frame (skip current frame)
    }
    if (CAMERA_IsBusy() == FALSE)   // done to execute stop command?
    {
        OS_TPrintf("Finished receiving final frame.\n");
        // disable this handler?
    }
    else
    {
        // start to capture for next frame
        CAMERA_DmaRecvAsync(DMA_NO, (void *)HW_LCDC_VRAM_A, BYTES_PER_LINE * LINES_AT_ONCE, BYTES_PER_LINE * HEIGHT);
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
            OS_TPrintf("%2d.%03d fps\n", (int)(1000000LL / uspf), (int)(1000000000LL / uspf) % 1000);
            count = 0;
            begin = OS_GetTick();
        }
    }
}
