/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - camera
  File:     camera.c

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
// �l�߂ăR�s�[����
#define CAMERA_PACK_U16(d, s)                   \
    ((d)[0] = (u8)((*((u16*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u16*)s) >>  8) & 0xFF))

#define CAMERA_PACK_U32(d, s)                   \
    ((d)[0] = (u8)((*((u32*)s) >>  0) & 0xFF),  \
     (d)[1] = (u8)((*((u32*)s) >>  8) & 0xFF),  \
     (d)[2] = (u8)((*((u32*)s) >> 16) & 0xFF),  \
     (d)[3] = (u8)((*((u32*)s) >> 24) & 0xFF))

// �f�[�^����ݒ肷��
#define CAMERA_SUBSEQUENT_NUMS(a)  (((a) << CAMERA_PXI_SUBSEQUENT_NUMS_SHIFT) & CAMERA_PXI_SUBSEQUENT_NUMS_MASK)

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/
typedef struct CAMERAWork
{
    BOOL            lock;
    CAMERACallback  callback;
    CAMERAResult    result;             // �擪�f�[�^�����ʘg
    void            *callbackArg;
    CAMERAPxiCommand    command;        // �R�}���h���
    CAMERAPxiResult     pxiResult;      // �擪�f�[�^�����ʘg
    u8      current;                    // ��M�ς݃f�[�^�� (�o�C�g�P��) (�擪������!!)
    u8      total;                      // �ŏI�f�[�^�� (1 + �㑱�R�}���h*3)
    u8      *data;                      // save API arg if any
    size_t  size;                       // save API arg if any
}
CAMERAWork;

/*---------------------------------------------------------------------------*
    �ÓI�ϐ���`
 *---------------------------------------------------------------------------*/
static BOOL cameraInitialized;
static CAMERAWork cameraWork;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static BOOL CameraSendPxiCommand(CAMERAPxiCommand command, u8 size, u8 data);
static void CameraSendPxiData(u8 *pData);
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void CameraSyncCallback(CAMERAResult result, void *arg);
static void CameraCallCallbackAndUnlock(CAMERAResult result);
static void CameraWaitBusy(void);

/*---------------------------------------------------------------------------*
  Name:         CAMERA_Init

  Description:  CAMERA���C�u����������������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CAMERA_Init(void)
{
    // �������ς݂��m�F
    if (cameraInitialized)
    {
        return;
    }
    cameraInitialized = 1;

    // �ϐ�������
    cameraWork.lock = FALSE;
    cameraWork.callback = NULL;

    // PXI�֘A��������
    PXI_Init();
    while (!PXI_IsCallbackReady(PXI_FIFO_TAG_CAMERA, PXI_PROC_ARM7))
    {
    }
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_CAMERA, CameraPxiCallback);
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInitAsync

  Description:  initialize camera registers via I2C
                async version.

  Arguments:    camera      - one of CameraSelect
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CInitAsync(CameraSelect camera, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_INIT;
    const u8                size    = CAMERA_PXI_SIZE_INIT;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // �R�[���o�b�N�ݒ�
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    return CameraSendPxiCommand(command, size, (u8)camera) ? CAMERA_RESULT_SUCCESS : CAMERA_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CInit

  Description:  initialize camera registers via I2C
                sync version.

  Arguments:    camera      - one of CameraSelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CInit(CameraSelect camera)
{
    cameraWork.result = CAMERA_I2CInitAsync(camera, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivateAsync

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                async version

  Arguments:    camera      - one of CameraSelect
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CActivateAsync(CameraSelect camera, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_ACTIVATE;
    const u8                size    = CAMERA_PXI_SIZE_ACTIVATE;
    OSIntrMode enabled;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_BOTH == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // �R�[���o�b�N�ݒ�
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    return CameraSendPxiCommand(command, size, (u8)camera) ? CAMERA_RESULT_SUCCESS : CAMERA_RESULT_SEND_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CActivate

  Description:  activate specified CAMERA (goto standby if NONE is specified)
                sync version.

  Arguments:    camera      - one of CameraSelect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CActivate(CameraSelect camera)
{
    cameraWork.result = CAMERA_I2CActivateAsync(camera, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResizeAsync

  Description:  resize CAMERA output image
                async version

  Arguments:    camera      - one of CameraResize
                width       - width of the image
                height      - height of the image
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CResizeAsync(CameraSelect camera, u16 width, u16 height, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_RESIZE;
    const u8                size    = CAMERA_PXI_SIZE_RESIZE;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // �R�[���o�b�N�ݒ�
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // �f�[�^�쐬
    data[0] = (u8)camera;
    CAMERA_PACK_U16(&data[1], &width);
    CAMERA_PACK_U16(&data[3], &height);

    // �R�}���h���M
    if (CameraSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CResize

  Description:  resize CAMERA output image
                sync version.

  Arguments:    camera      - one of CameraResize
                width       - width of the image
                height      - height of the image

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CResize(CameraSelect camera, u16 width, u16 height)
{
    cameraWork.result = CAMERA_I2CResizeAsync(camera, width, height, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRateAsync

  Description:  set CAMERA frame rate (0 means automatic)
                async version

  Arguments:    camera      - one of CameraSelect
                rate        - frame rate (fps)
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFrameRateAsync(CameraSelect camera, int rate, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_FRAME_RATE;
    const u8                size    = CAMERA_PXI_SIZE_FRAME_RATE;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }
    if (rate < 0 || rate > 30)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // �R�[���o�b�N�ݒ�
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // �f�[�^�쐬
    data[0] = (u8)camera;
    data[1] = (u8)rate;

    // �R�}���h���M
    if (CameraSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFrameRate

  Description:  set CAMERA frame rate (0 means automatic)
                sync version.

  Arguments:    camera      - one of CameraSelect
                rate        - frame rate (fps)

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFrameRate(CameraSelect camera, int rate)
{
    cameraWork.result = CAMERA_I2CFrameRateAsync(camera, rate, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffectAsync

  Description:  set CAMERA frame rate (0 means automatic)
                async version

  Arguments:    camera      - one of CameraSelect
                effect      - one of CameraEffect
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CEffectAsync(CameraSelect camera, CameraEffect effect, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_EFFECT;
    const u8                size    = CAMERA_PXI_SIZE_EFFECT;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // �R�[���o�b�N�ݒ�
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // �f�[�^�쐬
    data[0] = (u8)camera;
    data[1] = (u8)effect;

    // �R�}���h���M
    if (CameraSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CEffect

  Description:  set CAMERA frame rate (0 means automatic)
                sync version.

  Arguments:    camera      - one of CameraSelect
                effect      - one of CameraEffect

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CEffect(CameraSelect camera, CameraEffect effect)
{
    cameraWork.result = CAMERA_I2CEffectAsync(camera, effect, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlipAsync

  Description:  set CAMERA frame rate (0 means automatic)
                async version

  Arguments:    camera      - one of CameraSelect
                flip        - one of CameraFlip
                callback    - �񓯊����������������ĂɌĂяo���֐����w��
                arg         - �R�[���o�b�N�֐��̌Ăяo�����̈������w��B

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFlipAsync(CameraSelect camera, CameraFlip flip, CAMERACallback callback, void *arg)
{
    const CAMERAPxiCommand  command = CAMERA_PXI_COMMAND_FLIP;
    const u8                size    = CAMERA_PXI_SIZE_FLIP;
    OSIntrMode enabled;
    u8  data[size];
    int i;

    SDK_NULL_ASSERT(callback);

    if (CAMERA_SELECT_NONE == camera)
    {
        return CAMERA_RESULT_ILLEGAL_PARAMETER;
    }

    enabled = OS_DisableInterrupts();
    if (cameraWork.lock)
    {
        (void)OS_RestoreInterrupts(enabled);
        return CAMERA_RESULT_BUSY;
    }
    cameraWork.lock = TRUE;
    (void)OS_RestoreInterrupts(enabled);
    // �R�[���o�b�N�ݒ�
    cameraWork.callback = callback;
    cameraWork.callbackArg = arg;

    // �f�[�^�쐬
    data[0] = (u8)camera;
    data[1] = (u8)flip;

    // �R�}���h���M
    if (CameraSendPxiCommand(command, size, data[0]) == FALSE)
    {
        return CAMERA_RESULT_SEND_ERROR;
    }
    for (i = 1; i < size; i+=3) {
        CameraSendPxiData(&data[i]);
    }

    return CAMERA_RESULT_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         CAMERA_I2CFlip

  Description:  set CAMERA frame rate (0 means automatic)
                sync version.

  Arguments:    camera      - one of CameraSelect
                flip        - one of CameraFlip

  Returns:      CAMERAResult
 *---------------------------------------------------------------------------*/
CAMERAResult CAMERA_I2CFlip(CameraSelect camera, CameraFlip flip)
{
    cameraWork.result = CAMERA_I2CFlipAsync(camera, flip, CameraSyncCallback, 0);
    if (cameraWork.result == CAMERA_RESULT_SUCCESS)
    {
        CameraWaitBusy();
    }
    return cameraWork.result;
}


/*---------------------------------------------------------------------------*
  Name:         CameraSendPxiCommand

  Description:  �w��擪�R�}���h��PXI�o�R��ARM7�ɑ��M����B

  Arguments:    command     - �ΏۃR�}���h
                size        - ���M�f�[�^�T�C�Y (�o�C�g�P��)
                data        - �擪�f�[�^ (1�o�C�g�̂�)

  Returns:      BOOL     - PXI�ɑ΂��đ��M�����������ꍇTRUE���A
                           PXI�ɂ�鑗�M�Ɏ��s�����ꍇFALSE��Ԃ��B
 *---------------------------------------------------------------------------*/
static BOOL CameraSendPxiCommand(CAMERAPxiCommand command, u8 size, u8 data)
{
    u32 pxiData = (u32)(CAMERA_PXI_START_BIT |
            ((command << CAMERA_PXI_COMMAND_SHIFT) & CAMERA_PXI_COMMAND_MASK) |
            ((size << CAMERA_PXI_DATA_NUMS_SHIFT) & CAMERA_PXI_DATA_NUMS_MASK) |
            ((data << CAMERA_PXI_1ST_DATA_SHIFT) & CAMERA_PXI_1ST_DATA_MASK));
    if (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CameraSendPxiData

  Description:  �w��㑱�f�[�^��PXI�o�R��ARM7�ɑ��M����B

  Arguments:    pData   - 3�o�C�g�f�[�^�̐擪�ւ̃|�C���^

  Returns:      None
 *---------------------------------------------------------------------------*/
static void CameraSendPxiData(u8 *pData)
{
    u32 pxiData = (u32)((pData[0] << 16) | (pData[1] << 8) | pData[2]);
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_CAMERA, pxiData, 0))
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraPxiCallback

  Description:  �񓯊�RTC�֐��p�̋��ʃR�[���o�b�N�֐��B

  Arguments:    tag -  PXI tag which show message type.
                data - message from ARM7.
                err -  PXI transfer error flag.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraPxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )
    CAMERAResult   result;

    // PXI�ʐM�G���[���m�F
    if (err)
    {
        // �V�[�P���X�������I��
        CameraCallCallbackAndUnlock(CAMERA_RESULT_FATAL_ERROR);
        return;
    }
    // �擪�f�[�^
    if (data & CAMERA_PXI_START_BIT)
    {
        // ��M�f�[�^�����
        SDK_ASSERT((data & CAMERA_PXI_RESULT_BIT) == CAMERA_PXI_RESULT_BIT);
        cameraWork.total = (u8)((data & CAMERA_PXI_DATA_NUMS_MASK) >> CAMERA_PXI_DATA_NUMS_SHIFT);
        cameraWork.current = 0;
        cameraWork.command = (CAMERAPxiCommand)((data & CAMERA_PXI_COMMAND_MASK) >> CAMERA_PXI_COMMAND_SHIFT);
        cameraWork.pxiResult = (CAMERAPxiResult)((data & CAMERA_PXI_1ST_DATA_MASK) >> CAMERA_PXI_1ST_DATA_SHIFT);
    }
    // �㑱�f�[�^
    else
    {
        if (cameraWork.data == NULL)
        {
            // �V�[�P���X�������I��
            CameraCallCallbackAndUnlock(CAMERA_RESULT_FATAL_ERROR);
            return;
        }
        if (cameraWork.current < cameraWork.size)
        {
            cameraWork.data[cameraWork.current++] = (u8)((data & 0xFF0000) >> 16);
        }
        if (cameraWork.current < cameraWork.size)
        {
            cameraWork.data[cameraWork.current++] = (u8)((data & 0x00FF00) >> 8);
        }
        if (cameraWork.current < cameraWork.size)
        {
            cameraWork.data[cameraWork.current++] = (u8)((data & 0x0000FF) >> 0);
        }
    }

    if (cameraWork.current >= cameraWork.total-1)   // > �͖����͂�
    {
        // �������ʂ��m�F
        switch (cameraWork.pxiResult)
        {
        case CAMERA_PXI_RESULT_SUCCESS:
            result = CAMERA_RESULT_SUCCESS;
            break;
        case CAMERA_PXI_RESULT_SUCCESS_TRUE:
            result = CAMERA_RESULT_SUCCESS_TRUE;
            break;
        case CAMERA_PXI_RESULT_SUCCESS_FALSE:
            result = CAMERA_RESULT_SUCCESS_FALSE;
            break;
        case CAMERA_PXI_RESULT_INVALID_COMMAND:
            result = CAMERA_RESULT_INVALID_COMMAND;
            break;
        case CAMERA_PXI_RESULT_INVALID_PARAMETER:
            result = CAMERA_RESULT_ILLEGAL_PARAMETER;
            break;
        case CAMERA_PXI_RESULT_ILLEGAL_STATUS:
            result = CAMERA_RESULT_ILLEGAL_STATUS;
            break;
        case CAMERA_PXI_RESULT_BUSY:
            result = CAMERA_RESULT_BUSY;
            break;
        default:
            result = CAMERA_RESULT_FATAL_ERROR;
        }

        // �R�[���o�b�N�̌Ăяo��
        CameraCallCallbackAndUnlock(result);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraSyncCallback

  Description:  ����API�p�̃R�[���o�b�N

  Arguments:    result  - ARM7���瑗��ꂽ����
                arg     - ���g�p

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraSyncCallback(CAMERAResult result, void *arg)
{
#pragma unused(arg)
    cameraWork.result = result;
}

/*---------------------------------------------------------------------------*
  Name:         CameraCallCallbackAndUnlock

  Description:  �R�[���o�b�N�̌Ăяo���ƃ��b�N�̉������s��

  Arguments:    result  - ARM7���瑗��ꂽ����

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CameraCallCallbackAndUnlock(CAMERAResult result)
{
    CAMERACallback cb;

    if (cameraWork.lock)
    {
        cameraWork.lock = FALSE;
    }
    if (cameraWork.callback)
    {
        cb = cameraWork.callback;
        cameraWork.callback = NULL;
        cb(result, cameraWork.callbackArg);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CameraWaitBusy

  Description:  CAMERA�̔񓯊����������b�N����Ă���ԑ҂B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if 0
#include    <nitro/code32.h>
static asm void CameraWaitBusy(void)
{
    ldr     r12,    =cameraWork.lock
loop:
    ldr     r0,     [ r12,  #0 ]
    cmp     r0,     #TRUE
    beq     loop
    bx      lr
}
#include    <nitro/codereset.h>
#else
extern void PXIi_HandlerRecvFifoNotEmpty(void);
static void CameraWaitBusy(void)
{
    volatile BOOL *p = &cameraWork.lock;

    while (*p)
    {
        if (OS_GetCpsrIrq() == OS_INTRMODE_IRQ_DISABLE)
        {
            PXIi_HandlerRecvFifoNotEmpty();
        }
    }
}
#endif
