/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera_sp.c

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
#include <twl/camera.h>

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define CAMERA_PXI_SIZE_CHECK(nums)                                                     \
    if (cameraWork.total != (nums)) {                                                   \
        CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_INVALID_PARAMETER);    \
        break;                                                                          \
    }

// �A���C�����g�������ăR�s�[����
#define CAMERA_UNPACK_U16(d, s)    \
    (*(d) = (u16)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8)))
#define CAMERA_UNPACK_U32(d, s)    \
    (*(d) = (u32)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8) | (((u8*)s)[2] << 16) | (((u8*)s)[3] << 24)))

#define CAMERA_SET_GPIO(r)      ((r) |= CAMERA_STBYN_MASK)
#define CAMERA_CLEAR_GPIO(r)    ((r) &= ~CAMERA_STBYN_MASK)

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �ÓI�ϐ���`
 *---------------------------------------------------------------------------*/
static BOOL cameraInitialized;          // �������m�F�t���O
static CAMERAWork cameraWork;           // ���[�N�ϐ����܂Ƃ߂��\����

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void CameraReturnResult(CAMERAPxiCommand command, CAMERAPxiResult result);
static void CameraReturnResultEx(CAMERAPxiCommand command, CAMERAPxiResult result, u8 size, u8* data);
static void CameraThread(void *arg);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Init

  Description:  CAMERA���C�u����������������B

  Arguments:    priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_Init(u32 priority)
{
    // �������ς݂��m�F
    if (cameraInitialized)
    {
        return;
    }
    cameraInitialized = 1;

    // ���[�N������
    cameraWork.camera = CAMERA_SELECT_NONE;

    // PXI�֘A��������
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CAMERA, CameraPxiCallback);

    // ���������s���X���b�h���쐬
    OS_InitMessageQueue(&cameraWork.msgQ, cameraWork.msgArray, CAMERA_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&cameraWork.thread,
                    CameraThread,
                    0,
                    (void *)(cameraWork.stack + (CAMERA_THREAD_STACK_SIZE / sizeof(u64))),
                    CAMERA_THREAD_STACK_SIZE, priority);
    OS_WakeupThreadDirect(&cameraWork.thread);
}

/*---------------------------------------------------------------------------*
  Name:         CameraPxiCallback

  Description:  PXI�o�R�Ŏ�M�����f�[�^����͂���B

  Arguments:    tag -  PXI��ʂ������^�O�B
                data - ��M�����f�[�^�B����26bit���L���B
                err -  PXI�ʐM�ɂ�����G���[�t���O�B
                       ARM9���ɂē���ʂ�PXI������������Ă��Ȃ����Ƃ������B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    // PXI�ʐM�G���[���`�F�b�N
    if (err)
    {
        return;
    }
    // �擪�f�[�^
    if (data & CAMERA_PXI_START_BIT)
    {
        cameraWork.total = (u8)((data & CAMERA_PXI_DATA_NUMS_MASK) >> CAMERA_PXI_DATA_NUMS_SHIFT);
        cameraWork.current = 0;
        cameraWork.command = (CAMERAPxiCommand)((data & CAMERA_PXI_COMMAND_MASK) >> CAMERA_PXI_COMMAND_SHIFT);
        cameraWork.data[cameraWork.current++] = (u8)((data & CAMERA_PXI_1ST_DATA_MASK) >> CAMERA_PXI_1ST_DATA_SHIFT);
//OS_TPrintf("START_BIT (total=%d, command=%X).\n", cameraWork.total, cameraWork.command);
    }
    // �㑱�f�[�^
    else
    {
        cameraWork.data[cameraWork.current++] = (u8)((data & 0xFF0000) >> 16);
        cameraWork.data[cameraWork.current++] = (u8)((data & 0x00FF00) >> 8);
        cameraWork.data[cameraWork.current++] = (u8)((data & 0x0000FF) >> 0);
    }

    // �p�P�b�g����
    if (cameraWork.current >= cameraWork.total)   // �ő��2�]���Ɏ擾����
    {
        // ��M�����R�}���h�����
        switch (cameraWork.command)
        {
            // ���m�̃R�}���h�Q
        case CAMERA_PXI_COMMAND_INIT:
        case CAMERA_PXI_COMMAND_ACTIVATE:
        case CAMERA_PXI_COMMAND_RESIZE:
        case CAMERA_PXI_COMMAND_FRAME_RATE:
        case CAMERA_PXI_COMMAND_EFFECT:
        case CAMERA_PXI_COMMAND_FLIP:
            // �X���b�h���ĊJ
            if (!OS_SendMessage(&cameraWork.msgQ, NULL, OS_MESSAGE_NOBLOCK))
            {
                CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_FATAL_ERROR);
            }
            break;

            // ���m�̃R�}���h
        default:
            CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraReturnResult

  Description:  PXI�o�R�ŏ������ʂ�ARM9�ɑ��M����B

  Arguments:    command     - �ΏۃR�}���h
                result      - CAMERAPxiResult�̂ЂƂ�

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraReturnResult(CAMERAPxiCommand command, CAMERAPxiResult result)
{
    u32 pxiData = (u32)(CAMERA_PXI_START_BIT | CAMERA_PXI_RESULT_BIT |
            ((command << CAMERA_PXI_COMMAND_SHIFT) & CAMERA_PXI_COMMAND_MASK) |
            ((1 << CAMERA_PXI_DATA_NUMS_SHIFT) & CAMERA_PXI_DATA_NUMS_MASK) |
            ((result << CAMERA_PXI_1ST_DATA_SHIFT) & CAMERA_PXI_1ST_DATA_MASK));
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraReturnResultEx

  Description:  �w��㑱�f�[�^��PXI�o�R��ARM7�ɑ��M����B

  Arguments:    command     - �ΏۃR�}���h
                result      - CAMERAPxiResult�̂ЂƂ�
                size        - �t���f�[�^�T�C�Y
                data        - �t���f�[�^

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraReturnResultEx(CAMERAPxiCommand command, CAMERAPxiResult result, u8 size, u8* data)
{
    u32 pxiData = (u32)(CAMERA_PXI_START_BIT | CAMERA_PXI_RESULT_BIT |
            ((command << CAMERA_PXI_COMMAND_SHIFT) & CAMERA_PXI_COMMAND_MASK) |
            (((size+1) << CAMERA_PXI_DATA_NUMS_SHIFT) & CAMERA_PXI_DATA_NUMS_MASK) |
            ((result << CAMERA_PXI_1ST_DATA_SHIFT) & CAMERA_PXI_1ST_DATA_MASK));
    int i;
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
    }
    for (i = 0; i < size; i += 3)
    {
        pxiData = (u32)((data[i] << 16) | (data[i+1] << 8) | data[i+2]);
        while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
        {
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraThread

  Description:  CAMERA����̎��������s���X���b�h�B

  Arguments:    arg - �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraThread(void *arg)
{
#pragma unused( arg )

    OSMessage msg;
    BOOL result;
    u16 data16a;
    u16 data16b;

    while (TRUE)
    {
        // ���b�Z�[�W�����s�����܂ŐQ��
        (void)OS_ReceiveMessage(&(cameraWork.msgQ), &msg, OS_MESSAGE_BLOCK);

        // �R�}���h�ɏ]���Ċe�폈�������s
        switch (cameraWork.command)
        {
        case CAMERA_PXI_COMMAND_INIT:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_INIT);
            result = CAMERA_I2CInit((CameraSelect)cameraWork.data[0]);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9�ɏ����̐�����ʒB
            break;

        case CAMERA_PXI_COMMAND_ACTIVATE:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_ACTIVATE);
            if (cameraWork.camera != cameraWork.data[0])
            {
                if (cameraWork.camera != CAMERA_SELECT_NONE)
                {
                    if (FALSE == CAMERA_I2CStandby(cameraWork.camera, TRUE))
                    {
                        CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_SUCCESS_FALSE);    // ARM9�ɏ����̎��s��ʒB
                    }
                }
                cameraWork.camera = (CameraSelect)cameraWork.data[0];
                if (cameraWork.camera != CAMERA_SELECT_NONE)
                {
                    if (FALSE == CAMERA_I2CStandby(cameraWork.camera, FALSE))
                    {
                        CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_SUCCESS_FALSE);    // ARM9�ɏ����̎��s��ʒB
                    }
                }
            }
            CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_SUCCESS_TRUE);     // ARM9�ɏ����̐�����ʒB
            break;

        case CAMERA_PXI_COMMAND_RESIZE:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_RESIZE);
            CAMERA_UNPACK_U16(&data16a, &cameraWork.data[1]);
            CAMERA_UNPACK_U16(&data16b, &cameraWork.data[3]);
            result = CAMERA_I2CResize((CameraSelect)cameraWork.data[0], data16a, data16b);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9�ɏ����̐�����ʒB
            break;

        case CAMERA_PXI_COMMAND_FRAME_RATE:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_FRAME_RATE);
            result = CAMERA_I2CFrameRate((CameraSelect)cameraWork.data[0], cameraWork.data[1]);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9�ɏ����̐�����ʒB
            break;

        case CAMERA_PXI_COMMAND_EFFECT:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_EFFECT);
            result = CAMERA_I2CEffect((CameraSelect)cameraWork.data[0], (CameraEffect)cameraWork.data[1]);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9�ɏ����̐�����ʒB
            break;

        case CAMERA_PXI_COMMAND_FLIP:
            CAMERA_PXI_SIZE_CHECK(CAMERA_PXI_SIZE_FLIP);
            result = CAMERA_I2CFlip((CameraSelect)cameraWork.data[0], (CameraFlip)cameraWork.data[1]);
            CameraReturnResult(cameraWork.command, result ? CAMERA_PXI_RESULT_SUCCESS_TRUE : CAMERA_PXI_RESULT_SUCCESS_FALSE);  // ARM9�ɏ����̐�����ʒB
            break;

            // �T�|�[�g���Ȃ��R�}���h
        default:
            CameraReturnResult(cameraWork.command, CAMERA_PXI_RESULT_INVALID_COMMAND);
        }
    }
}
