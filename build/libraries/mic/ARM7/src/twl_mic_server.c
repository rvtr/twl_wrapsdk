/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - twlmic
  File:     twl_mic_server.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  TWL Mic�p�ɏ����ARM7���X���b�h�����܂����B
  �s���悯���SPI�X���b�h�Ȃǂɋz�����Ă��������B
 ----------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/mic.h>

/*---------------------------------------------------------------------------*
    �}�N����`
 *---------------------------------------------------------------------------*/

// �A���C�����g�������ăR�s�[����
#define MIC_UNPACK_U16(d, s)    \
    (*(d) = (u16)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8)))
#define MIC_UNPACK_U32(d, s)    \
    (*(d) = (u32)((((u8*)s)[0] << 0) | (((u8*)s)[1] << 8) | (((u8*)s)[2] << 16) | (((u8*)s)[3] << 24)))

/*---------------------------------------------------------------------------*
    �ÓI�ϐ���`
 *---------------------------------------------------------------------------*/

static BOOL micInitialized;          	// �������m�F�t���O
static TWLMICServerWork micWork;      	// ���[�N�ϐ����܂Ƃ߂��\����

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

static void TWL_MICi_PxiCallback(PXIFifoTag tag, u32 data, BOOL err);
static void TWL_MICi_ReturnResult(TWLMICPxiCommand command, TWLMICPxiResult result);
static void TWL_MICi_ReturnResultEx(TWLMICPxiCommand command, TWLMICPxiResult result, u8 size, u8* data);
static void TWL_MICi_Thread(void *arg);
static void TWL_MICi_FullCallback( u8 loop );
static BOOL TWL_MICi_SetEntry(TWLMICPxiCommand command, u16 args, ...);

/*---------------------------------------------------------------------------*
  Name:         TWL_MIC_InitServer

  Description:  ARM7���Ƃ��Ƃ���s�����߂̏������s���܂��B

  Arguments:    priority

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
TWL_MIC_InitServer(u32 priority)
{
    // �������ς݂��m�F
    if (micInitialized)
    {
        return;
    }
    micInitialized = 1;

    // ������ԊǗ��ϐ����N���A
    micWork.status = TWL_MIC_STATUS_READY;

    // PXI�֘A��������
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_TWL_MIC, TWL_MICi_PxiCallback);

    // ���������s���X���b�h���쐬���N��
    OS_InitMessageQueue(&micWork.msgQ, micWork.msgArray, TWL_MIC_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&micWork.thread,
                    TWL_MICi_Thread,
                    0,
                    (void *)(micWork.stack + (TWL_MIC_THREAD_STACK_SIZE / sizeof(u64))),
                    TWL_MIC_THREAD_STACK_SIZE, priority);
    OS_WakeupThreadDirect(&micWork.thread);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_SetEntry

  Description:  TWL MIC �X���b�h�ɑ���v����\�񂷂�B

  Arguments:    command -   �s���ׂ������ID���w��B
                args    -   ����ɔ����p�����[�^�̐����w��B
                ...     -   �e�p�����[�^��u32�^�Ŏw��B

  Returns:      BOOL    -   �\��ɐ��������ꍇ��TRUE���A���s�����ꍇ��FALSE��Ԃ��B
 *---------------------------------------------------------------------------*/
static BOOL 
TWL_MICi_SetEntry(TWLMICPxiCommand command, u16 args, ...)
{
    OSIntrMode e;
    void   *w;
    va_list vlist;
    s32     i;

    // �����̐����`�F�b�N
    if (args > TWL_MIC_MESSAGE_ARGS_MAX)
    {
        return FALSE;
    }

    e = OS_DisableInterrupts();
    micWork.entry[micWork.entryIndex].command = command;

    // �w�萔�̈�����ǉ�
    va_start(vlist, args);
    for (i = 0; i < args; i++)
    {
        micWork.entry[micWork.entryIndex].arg[i] = va_arg(vlist, u32);
    }
    va_end(vlist);

    w = &(micWork.entry[micWork.entryIndex]);
    micWork.entryIndex = (u32)((micWork.entryIndex + 1) % TWL_MIC_MESSAGE_ARRAY_MAX);
    (void)OS_RestoreInterrupts(e);
    return OS_SendMessage(&(micWork.msgQ), w, OS_MESSAGE_NOBLOCK);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_PxiCallback

  Description:  PXI�o�R�Ŏ�M�����f�[�^����͂���B

  Arguments:    tag -  PXI��ʂ������^�O�B
                data - ��M�����f�[�^�B����26bit���L���B
                err -  PXI�ʐM�ɂ�����G���[�t���O�B
                       ARM9���ɂē���ʂ�PXI������������Ă��Ȃ����Ƃ������B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWL_MICi_PxiCallback(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused( tag )

    // PXI�ʐM�G���[���`�F�b�N
    if (err)
    {
        return;
    }
    // �擪�f�[�^
    if (data & TWL_MIC_PXI_START_BIT)
    {
        micWork.total = (u8)((data & TWL_MIC_PXI_DATA_NUMS_MASK) >> TWL_MIC_PXI_DATA_NUMS_SHIFT);
        micWork.current = 0;
        micWork.command = (TWLMICPxiCommand)((data & TWL_MIC_PXI_COMMAND_MASK) >> TWL_MIC_PXI_COMMAND_SHIFT);
        micWork.data[micWork.current++] = (u8)((data & TWL_MIC_PXI_1ST_DATA_MASK) >> TWL_MIC_PXI_1ST_DATA_SHIFT);
    }
    // �㑱�f�[�^
    else
    {
        micWork.data[micWork.current++] = (u8)((data & 0x00FF0000) >> 16);
        micWork.data[micWork.current++] = (u8)((data & 0x0000FF00) >> 8);
        micWork.data[micWork.current++] = (u8)((data & 0x000000FF) >> 0);
    }

    // �p�P�b�g����
    if (micWork.current >= micWork.total)   // �ő��2�o�C�g�]���Ɏ擾����
    {
        // ��M�����R�}���h�����
        switch (micWork.command)
        {
			// �P���T���v�����O
			case TWL_MIC_PXI_COMMAND_ONE_SAMPLING:
	            if (micWork.status != TWL_MIC_STATUS_READY)
	            {
					// �R�}���h�����s�ł����Ԃł͂Ȃ�
	                TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
	            }
				else
				{
					// TWL MIC �X���b�h�ɑ���v��
					if (TWL_MICi_SetEntry( micWork.command, 0 ))
					{
						// ��Ԃ�"�P���T���v�����O�J�n�҂�"��
						micWork.status = TWL_MIC_STATUS_ONE_SAMPLING_START;  
					}
					else
					{
						// �G���g���[���s�i���b�Z�[�W�L���[�ɋ󂫖����j
    	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
					}
				}
				break;

            // �I�[�g�T���v�����O�J�n
			case TWL_MIC_PXI_COMMAND_AUTO_START:
	            if (micWork.status != TWL_MIC_STATUS_READY)
	            {
					// �R�}���h�����s�ł����Ԃł͂Ȃ�
	                TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
	            }
				else
				{
					u32 dmaNo, buffer, size, frequency, loop;
					
					dmaNo = micWork.data[0];
		            MIC_UNPACK_U32(&buffer, &micWork.data[1]);
					MIC_UNPACK_U32(&size, &micWork.data[5]);
					frequency = (u32)(micWork.data[9] << REG_SND_MICCNT_FIFO_SMP_SHIFT);
					loop  = micWork.data[10];

					// TWL MIC �X���b�h�ɑ���v��
					if (TWL_MICi_SetEntry( micWork.command, 5, dmaNo, buffer, size, frequency, loop ))
					{
						// ��Ԃ�"�����T���v�����O�J�n�҂�"��
						micWork.status = TWL_MIC_STATUS_AUTO_START;  
					}
					else
					{
						// �G���g���[���s�i���b�Z�[�W�L���[�ɋ󂫖����j
    	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
					}
				}
				break;

			// �I�[�g�T���v�����O��~
			case TWL_MIC_PXI_COMMAND_AUTO_STOP:
	            if (micWork.status != TWL_MIC_STATUS_AUTO_SAMPLING)
	            {
					// �R�}���h�����s�ł����Ԃł͂Ȃ�
	                TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
	            }
				else
				{
					// TWL MIC �X���b�h�ɑ���v��
					if (TWL_MICi_SetEntry( micWork.command, 0 ))
					{
						// ��Ԃ�"�����T���v�����O��~�҂�"��
						micWork.status = TWL_MIC_STATUS_AUTO_END;  	
					}
					else
					{
						// �G���g���[���s�i���b�Z�[�W�L���[�ɋ󂫖����j
    	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
					}
				}
            	break;

			// �ŐV�T���v�����O�f�[�^�A�h���X�擾
			case TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS:

				// TWL MIC �X���b�h�ɑ���v��
				if (!TWL_MICi_SetEntry( micWork.command, 0 ))
				{
					// �G���g���[���s�i���b�Z�[�W�L���[�ɋ󂫖����j
	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
				}
            	break;

            // �v���O���}�u���A���v�Q�C���ݒ�
			case TWL_MIC_PXI_COMMAND_SET_AMP_GAIN:
				{
					u32 gain = micWork.data[0];

					// TWL MIC �X���b�h�ɑ���v��
					if (!TWL_MICi_SetEntry( micWork.command, 1, gain ))
					{
						// �G���g���[���s�i���b�Z�[�W�L���[�ɋ󂫖����j
    	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
					}
				}
				break;

			// �v���O���}�u���A���v�Q�C���擾
			case TWL_MIC_PXI_COMMAND_GET_AMP_GAIN:

				// TWL MIC �X���b�h�ɑ���v��
				if (!TWL_MICi_SetEntry( micWork.command, 0 ))
				{
					// �G���g���[���s�i���b�Z�[�W�L���[�ɋ󂫖����j
	    			TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_FATAL_ERROR);
				}
            	break;

            // ���m�̃R�}���h
        	default:
            	TWL_MICi_ReturnResult(micWork.command, TWL_MIC_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_ReturnResult

  Description:  PXI�o�R�ŏ������ʂ�ARM9�ɑ��M����B

  Arguments:    command     - �ΏۃR�}���h
                result      - TWLMICPxiResult�̂ЂƂ�

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void 
TWL_MICi_ReturnResult(TWLMICPxiCommand command, TWLMICPxiResult result)
{
    u32 pxiData = (u32)(TWL_MIC_PXI_START_BIT | TWL_MIC_PXI_RESULT_BIT |
            ((command << TWL_MIC_PXI_COMMAND_SHIFT) & TWL_MIC_PXI_COMMAND_MASK) |
            ((1 << TWL_MIC_PXI_DATA_NUMS_SHIFT) & TWL_MIC_PXI_DATA_NUMS_MASK) |
            ((result << TWL_MIC_PXI_1ST_DATA_SHIFT) & TWL_MIC_PXI_1ST_DATA_MASK));
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TWL_MIC, pxiData, 0))
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_ReturnResultEx

  Description:  �w��㑱�f�[�^��PXI�o�R��ARM9�ɑ��M����B

  Arguments:    command     - �ΏۃR�}���h
                result      - TWLMICPxiResult�̂ЂƂ�
                size        - �t���f�[�^�T�C�Y
                data        - �t���f�[�^

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void 
TWL_MICi_ReturnResultEx(TWLMICPxiCommand command, TWLMICPxiResult result, u8 size, u8* data)
{
    u32 pxiData = (u32)(TWL_MIC_PXI_START_BIT | TWL_MIC_PXI_RESULT_BIT |
            ((command << TWL_MIC_PXI_COMMAND_SHIFT) & TWL_MIC_PXI_COMMAND_MASK) |
            (((size+1) << TWL_MIC_PXI_DATA_NUMS_SHIFT) & TWL_MIC_PXI_DATA_NUMS_MASK) |
            ((result << TWL_MIC_PXI_1ST_DATA_SHIFT) & TWL_MIC_PXI_1ST_DATA_MASK));
    int i;
    while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TWL_MIC, pxiData, 0))
    {
    }
    for (i = 0; i < size; i += 3)
    {
        pxiData = (u32)((data[i] << 16) | (data[i+1] << 8) | data[i+2]);
        while (0 > PXI_SendWordByFifo(PXI_FIFO_TAG_TWL_MIC, pxiData, 0))
        {
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_Thread

  Description:  MIC����̎��������s���X���b�h�B

  Arguments:    arg - �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWL_MICi_Thread(void *arg)
{
#pragma unused( arg )

    OSMessage msg;
    TWLMICMessageData* entry;

    while (TRUE)
    {
        // ���b�Z�[�W�����s�����܂ŐQ��
        (void)OS_ReceiveMessage(&(micWork.msgQ), &msg, OS_MESSAGE_BLOCK);
        entry = (TWLMICMessageData *) msg;

        // �R�}���h�ɏ]���Ċe�폈�������s
        switch (entry->command)
        {
		//--- �P���T���v�����O�J�n
        case TWL_MIC_PXI_COMMAND_ONE_SAMPLING:
			if (micWork.status == TWL_MIC_STATUS_ONE_SAMPLING_START)
			{
				u16 temp;
				TWL_MIC_DoSampling( &temp );

				TWL_MICi_ReturnResultEx(entry->command, TWL_MIC_PXI_RESULT_SUCCESS, 
									2, (u8*)&temp );	// ARM9�ɏ����̐�����ʒB

				micWork.status = TWL_MIC_STATUS_READY;     				// ��Ԃ�"�ʏ푀��҂�"��
			}
			else
			{
	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
			}
			break;

		//--- �I�[�g�T���v�����O�J�n
        case TWL_MIC_PXI_COMMAND_AUTO_START:
			if (micWork.status == TWL_MIC_STATUS_AUTO_START)
			{
				TWLMICParam param;

				param.dmaNo     = (u8)entry->arg[0];
	            param.buffer    = (void *)entry->arg[1];
				param.size      = entry->arg[2];
				param.frequency = (MICSampleRate)entry->arg[3];
				param.loop      = (u8)entry->arg[4];
				param.callback = TWL_MICi_FullCallback;
				TWL_MIC_StartAutoSampling( param );

	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_SUCCESS); 	// ARM9�ɏ����̐�����ʒB
				micWork.status = TWL_MIC_STATUS_AUTO_SAMPLING;     				// ��Ԃ�"�����T���v�����O��"��
			}
			else
			{
	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
			}
			break;

		//--- �I�[�g�T���v�����O��~
        case TWL_MIC_PXI_COMMAND_AUTO_STOP:
	        if ((micWork.status == TWL_MIC_STATUS_AUTO_END) || (micWork.status == TWL_MIC_STATUS_END_WAIT))
	        {
				TWL_MIC_StopAutoSampling();

	            // ARM9�ɏ����̐�����ʒB
	            if (micWork.status == TWL_MIC_STATUS_AUTO_END)
	            {
		            TWL_MICi_ReturnResult(TWL_MIC_PXI_COMMAND_AUTO_STOP, TWL_MIC_PXI_RESULT_SUCCESS);
	            }
	            // ������Ԃ��X�V
	            micWork.status = TWL_MIC_STATUS_READY;     // ��Ԃ�"�ʏ푀��҂�"��
			}
	        else
	        {
	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_ILLEGAL_STATUS);
	        }
			break;

		//--- �ŐV�T���v�����O�f�[�^�A�h���X�擾
        case TWL_MIC_PXI_COMMAND_GET_LAST_SAMPLING_ADDRESS:
			{
				void* adress = TWL_MIC_GetLastSamplingAddress();
	
				TWL_MICi_ReturnResultEx(entry->command, TWL_MIC_PXI_RESULT_SUCCESS, 
									4, (u8*)&adress );	// ARM9�ɏ����̐�����ʒB
			}
			break;

		//--- �v���O���}�u���A���v�Q�C���ݒ�
        case TWL_MIC_PXI_COMMAND_SET_AMP_GAIN:
			{
				u8 gain = (u8)entry->arg[0];
				TWL_MIC_SetAmpGain( gain );
	            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_SUCCESS); 	// ARM9�ɏ����̐�����ʒB
			}
			break;
		
		//--- �v���O���}�u���A���v�Q�C���擾
		case TWL_MIC_PXI_COMMAND_GET_AMP_GAIN:
		{
			u8 gain = TWL_MIC_GetAmpGain();
			TWL_MICi_ReturnResultEx(entry->command, TWL_MIC_PXI_RESULT_SUCCESS, 
									1, (u8*)&gain );	// ARM9�ɏ����̐�����ʒB
		}
        //--- �T�|�[�g���Ȃ��R�}���h
        default:
            TWL_MICi_ReturnResult(entry->command, TWL_MIC_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         TWL_MICi_FullCallback

  Description:  

  Arguments:    

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TWL_MICi_FullCallback( u8 loop )
{
	// FULL�ʒm
	TWL_MICi_ReturnResult(TWL_MIC_PXI_COMMAND_MIC_BUFFER_FULL, TWL_MIC_PXI_RESULT_SUCCESS);

	if ( !loop )
	{
		// �����T���v�����O��~
		if (TWL_MICi_SetEntry( TWL_MIC_PXI_COMMAND_AUTO_STOP, 0 ))
		{
			micWork.status = TWL_MIC_STATUS_END_WAIT;	// ��Ԃ�"�����T���v�����O�����҂�"��
		}
		else
		{
			TWL_MICi_ReturnResult(TWL_MIC_PXI_COMMAND_AUTO_STOP, TWL_MIC_PXI_RESULT_FATAL_ERROR);
		}
	}
}


