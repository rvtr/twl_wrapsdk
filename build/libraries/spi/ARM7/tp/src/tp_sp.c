/*---------------------------------------------------------------------------*
  Project:  NitroSDK - libraries - spi
  File:     tp_sp.c

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: tp_sp.c,v $
  Revision 1.15  2006/01/18 02:12:29  kitase_hirotake
  do-indent

  Revision 1.14  2005/02/28 05:26:32  yosizaki
  do-indent.

  Revision 1.13  2004/12/29 02:04:13  takano_makoto
  SetStability�֐���retry�p�����[�^��p�~

  Revision 1.12  2004/12/20 00:41:05  takano_makoto
  �����T���v�����O����VAlarm��delay��ݒ�

  Revision 1.11  2004/12/15 12:02:51  takano_makoto
  �R�[�h�̏ȃT�C�Y��

  Revision 1.10  2004/12/15 09:12:19  takano_makoto
  range�̎���������ǉ��Btp_sp.h��SDK_TP_AUTO_ADJUST_RANGE�ɂ���ėL���ɂȂ�B

  Revision 1.9  2004/10/21 04:02:57  terui
  LCD�̃��C������`����ύX�B

  Revision 1.8  2004/10/20 06:34:45  terui
  LCD�̃��C������`����ύX

  Revision 1.7  2004/09/06 13:08:42  terui
  SPI�����\������̎����ɔ����C���B

  Revision 1.6  2004/08/10 05:07:44  takano_makoto
  �������t�@�����X�ŃT���v�����O�̏ꍇ�ATPi_ExecSamplingInSpaceTime()���s���
  �}�C�N�f�o�C�X�̋N�������ǉ�

  Revision 1.5  2004/08/09 13:19:32  takano_makoto
  MIC�����T���v�����O����TP�̂P��T���v�����O�����s�ł���悤�ɏC��

  Revision 1.4  2004/07/31 02:29:20  terui
  IC��reset�������ꕔ�ύX

  Revision 1.3  2004/07/29 13:09:13  takano_makoto
  TP_Init��IC�̃��Z�b�g�����ǉ�

  Revision 1.2  2004/06/03 11:12:10  terui
  SPI�f�o�C�X���ɃX���b�h�D��x�𒲐���������B

  Revision 1.1  2004/05/25 01:05:48  terui
  TP���C�u������SPI���C�u�������番��

  Revision 1.2  2004/04/29 10:20:17  terui
  �r����������ʊ֐��ōs���悤�ύX

  Revision 1.1  2004/04/14 06:27:50  terui
  SPI���C�u�����̃\�[�X�����ɔ����X�V

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

/*
	TP�֘A��CODEC��DS���[�h��TWL���[�h�̂�����ł����Ă�ARM9���瓯���֐���
�@�@�g����悤�ɂ��Ă���܂��B�Ƃ��������ATWL���[�h������Ƃ����ē��ɗǂ�
�@�@���Ƃ͑����ĂȂ��Ƃ����������������ł��B���S�݊��̕����悩�����ł��ˁB

	�����_�ł� CDC_IsTwlMode() �̌��ʂɂ����
	DS�݊����[�h��TWL���[�h�𕪊򂳂��Ă��܂��B
	�����R�ɗ������Ă��������B
*/

#include    "tp_sp.h"
#include    <twl/cdc/ARM7/cdc_api.h>

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <nitro/os/common/printf.h>
#define DBG_PRINTF OS_TPrintf
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_modeAR( c )      ((void)0)
#endif

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/
static TPWork tpw;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void TpVAlarmHandler(void *arg);
static void SetStability(u16 range);

/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TP_Init

  Description:  �^�b�`�p�l���Ɋւ�������Ǘ��ϐ�������������B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

void TP_Init(void)
{
    s32     i;

    // ������ԊǗ��ϐ����N���A
    tpw.status = TP_STATUS_READY;
    tpw.range = SPI_TP_DEFAULT_STABILITY_RANGE;
    tpw.rangeMin = SPI_TP_DEFAULT_STABILITY_RANGE;

    // PXI�R�}���h�ޔ�p�̔z����N���A
    for (i = 0; i < SPI_PXI_CONTINUOUS_PACKET_MAX; i++)
    {
        tpw.command[i] = 0x0000;
    }

    // V�J�E���g�A���[����������
    if (!OS_IsVAlarmAvailable())
    {
        OS_InitVAlarm();
    }
    for (i = 0; i < SPI_TP_SAMPLING_FREQUENCY_MAX; i++)
    {
        OS_CreateVAlarm(&(tpw.vAlarm[i]));
        OS_SetVAlarmTag(&(tpw.vAlarm[i]), SPI_TP_VALARM_TAG);
    }

	if (CDC_IsTwlMode())
	{
    	TWL_TP_DisableNewBufferMode();							// NewBufferMode�͎g�p����TSC2101���[�h���g�p����
	    TWL_TP_SetResolution( TP_RESOLUTION_12 );				// �^�b�`�p�l���f�[�^�̐��x��12bit
		TWL_TP_SetInterval(TP_INTERVAL_NONE);					// TSC2101���[�h�ł͖���	
		TWL_TP_SetConversionMode( 
				TP_CONVERSION_CONTROL_SELF, 
				TP_CONVERSION_MODE_XY,
				TP_CONVERSION_PIN_DATA_AVAILABLE	);			// �ϊ����[�h��XY���W
		TWL_TP_SetStabilizationTime( TP_SETUP_TIME_100US );		// �傫���ƍ��W�l�̈��萫�����܂����ɃT���v�����O���Ԃ�����
		TWL_TP_SetSenseTime(         TP_SETUP_TIME_0_1US );		// sense time ���� Pen Touch �� Low �Ƀ}�X�N�����̂Œ���
		TWL_TP_SetPrechargeTime(     TP_SETUP_TIME_0_1US );		// precharge time ���� Pen Touch �� Low �Ƀ}�X�N�����̂Œ���
		TWL_TP_SetDebounceTime( TP_DEBOUNCE_0US );				// 
	}
	else
	{
	    // TP����IC�̏�����
	    /*  8�N���b�N���M��ɂ���CS���グ�Ă��������v�Ƃ͎v�����A
	       ����TP�R���g���[�����R�X�g�_�E���i�ɍ��ւ�邱�Ƃ��l������
	       24�T�C�N����CS���グ��ۏ؂��ꂽ�ʐM���@���Ƃ��Ă����B */
	    SPI_Wait();
	    TP_SPIChangeMode(SPI_TRANSMODE_CONTINUOUS);
	    SPI_SendWait(TP_COMMAND_DETECT_TOUCH);
	    SPI_DummyWait();
	    TP_SPIChangeMode(SPI_TRANSMODE_1BYTE);
	    SPI_DummyWait();
	}
}

/*---------------------------------------------------------------------------*
  Name:         TP_AnalyzeCommand

  Description:  �^�b�`�p�l���֘APXI�R�}���h����͂��A�����̏���������B
                �����ł̓X���b�h�ɏ����v����\�񂵁A���ۂ�SPI����̓X���b�h��
                �ōs����B

  Arguments:    data  - PXI�o�R�Ŏ󂯎����ARM9����̗v���R�}���h�B

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_AnalyzeCommand(u32 data)
{
    // �A���p�P�b�g�J�n���`�F�b�N
    if (data & SPI_PXI_START_BIT)
    {
        s32     i;

        // �A���p�P�b�g�J�n�Ȃ�R�}���h�ޔ�z����N���A
        for (i = 0; i < SPI_PXI_CONTINUOUS_PACKET_MAX; i++)
        {
            tpw.command[i] = 0x0000;
        }
    }
    // ��M�f�[�^���R�}���h�ޔ�z��ɑޔ�
    tpw.command[(data & SPI_PXI_INDEX_MASK) >> SPI_PXI_INDEX_SHIFT] = (u16)((data &
                                                                             SPI_PXI_DATA_MASK) >>
                                                                            SPI_PXI_DATA_SHIFT);

    if (data & SPI_PXI_END_BIT)
    {
        u16     command;
        u16     wu16[2];

        // ��M�f�[�^����R�}���h�𒊏o
        command = (u16)((tpw.command[0] & 0xff00) >> 8);

        // �R�}���h�����
        switch (command)
        {
            // �T���v�����O���蔻��p�����[�^�ύX
        case SPI_PXI_COMMAND_TP_SETUP_STABILITY:
            wu16[0] = (u16)(tpw.command[0] & 0x00FF);
            SetStability(wu16[0]);
            break;

            // �P�̃T���v�����O
        case SPI_PXI_COMMAND_TP_SAMPLING:
            // TP�T���v�����O������X���b�h�ɗ\��
            if (!SPIi_SetEntry(SPI_DEVICE_TYPE_TP, (u32)command, 0))
            {
                SPIi_ReturnResult(command, SPI_PXI_RESULT_EXCLUSIVE);
            }
            break;

            // �����T���v�����O�J�n
        case SPI_PXI_COMMAND_TP_AUTO_ON:
            // ������Ԃ��`�F�b�N
            if (tpw.status != TP_STATUS_READY)
            {
                // ���Ɏ����T���v�����O���̏ꍇ�͕s���ȏ�Ԃƌ��Ȃ�
                SPIi_ReturnResult(command, SPI_PXI_RESULT_ILLEGAL_STATUS);
                return;
            }
            // "frequency"�p�����[�^���`�F�b�N
            wu16[0] = (u16)(tpw.command[0] & 0x00ff);
            if ((wu16[0] == 0) || (wu16[0] > SPI_TP_SAMPLING_FREQUENCY_MAX))
            {
                SPIi_ReturnResult(command, SPI_PXI_RESULT_INVALID_PARAMETER);
                return;
            }
            // "vCount"�p�����[�^���`�F�b�N
            wu16[1] = tpw.command[1];
            if (wu16[1] >= HW_LCD_LINES)
            {
                SPIi_ReturnResult(command, SPI_PXI_RESULT_INVALID_PARAMETER);
                return;
            }
            // �����T���v�����O�J�n������X���b�h�ɗ\��
            if (!SPIi_SetEntry(SPI_DEVICE_TYPE_TP, (u32)command, 2, (u32)wu16[0], (u32)wu16[1]))
            {                          // �X���b�h�ւ̏����\��Ɏ��s
                SPIi_ReturnResult(command, SPI_PXI_RESULT_EXCLUSIVE);
                return;
            }
            // ������Ԃ��X�V
            tpw.status = TP_STATUS_AUTO_START;  // ��Ԃ�"�����T���v�����O�J�n�҂�"��
            break;

            // �����T���v�����O��~
        case SPI_PXI_COMMAND_TP_AUTO_OFF:
            // ������Ԃ��`�F�b�N
            if (tpw.status != TP_STATUS_AUTO_SAMPLING)
            {
                SPIi_ReturnResult(command, SPI_PXI_RESULT_ILLEGAL_STATUS);
                return;
            }
            // �����T���v�����O��~������X���b�h�ɗ\��
            if (!SPIi_SetEntry(SPI_DEVICE_TYPE_TP, (u32)command, 0))
            {                          // �X���b�h�ւ̏����\��Ɏ��s
                SPIi_ReturnResult(command, SPI_PXI_RESULT_EXCLUSIVE);
                return;
            }
            // ������Ԃ��X�V
            tpw.status = TP_STATUS_AUTO_WAIT_END;       // ��Ԃ�"�����T���v�����O��~�҂�"��
            break;

            // �s���ȃR�}���h
        default:
            SPIi_ReturnResult(command, SPI_PXI_RESULT_INVALID_COMMAND);
        }
    }
}

#ifdef SDK_TP_AUTO_ADJUST_RANGE        // range�̎��������X�C�b�`
/*---------------------------------------------------------------------------*
  Name:         TP_AutoAdjustRange

  Description:  �^�b�`�p�l���̃`���^�����O�΍�p�����[�^�������������܂��B

  Arguments:    tpdata  �T���v�����O����TP�f�[�^
                density �T���v�����O���̍��W���x

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TP_AutoAdjustRange(SPITpData *tpdata, u16 density)
{
    static u8 invalid_cnt = 0;
    static u8 valid_cnt = 0;
#define RANGE_MAX       35
#define RANGE_INC_CNT   4
#define RANGE_DEC_CNT   4
#define RANGE_DEC_CONDITION( d, r ) ( (d) < ( (r) >> 1 ) )

    if (!tpdata->e.touch)
    {
        // �^�b�`����Ă��Ȃ��ꍇ�ɂ́A�J�E���^�����Z�b�g
        invalid_cnt = 0;
        valid_cnt = 0;
        return;
    }

    // �^�b�`����Ă���ꍇ�ɂ�range�̎������������܂��B

    if (tpdata->e.validity)
        // INVALID�̏ꍇ�ɂ� invalid_cnt���J�E���g�A�b�v���܂��B
    {
        valid_cnt = 0;
        if (++invalid_cnt >= RANGE_INC_CNT)     // ���񐔘A������INVALID���擾�����ꍇ�ɂ�range�𒲐�
        {
            invalid_cnt = 0;
            if (tpw.range < RANGE_MAX)
            {
                tpw.range += 1;
            }
        }
    }
    else
    {
        // �T���v�����O�������W�l����蕝�ȏ�Ɏ������Ă�����valid_cnt���J�E���g�A�b�v���܂��B
        invalid_cnt = 0;
        if (!RANGE_DEC_CONDITION(density, tpw.range))
        {
            valid_cnt = 0;
            return;
        }

        if (++valid_cnt >= RANGE_DEC_CNT)
        {
            valid_cnt = 0;
            if (tpw.range > tpw.rangeMin)       // ���񐔘A�����ė]���Ɏ������Ă����ꍇ�ɂ�range�𒲐�
            {
                tpw.range -= 1;
                // range�����炵�Ă݂đʖڂ������ꍇ�ɂ͂����Ɍ��̏�Ԃɖ߂��悤�ɂ��Ă����B
                invalid_cnt = RANGE_INC_CNT - 1;
            }
        }
    }
}
#endif

/*---------------------------------------------------------------------------*
  Name:         TP_ExecuteProcess

  Description:  �^�b�`�p�l���Ɋւ�����ۂ̏������s���B
                ���̊֐���SPI���ꌳ�Ǘ�����X���b�h����Ăяo�����B

  Arguments:    entry   -   �G���g���[�\���̂ւ̃|�C���^�B

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TP_ExecuteProcess(SPIEntry * entry)
{
    switch (entry->process)
    {
        // �����T���v�����O( VAlarm����̃G���g���[ )
    case SPI_PXI_COMMAND_TP_AUTO_SAMPLING:
        if (tpw.status != TP_STATUS_AUTO_SAMPLING)
        {
            // �����T���v�����O���łȂ��ꍇ�͉��������ɏI��
            return;
        }
        // �P�̃T���v�����O
    case SPI_PXI_COMMAND_TP_SAMPLING:

        // �T���v�����O�����s
        {
            SPITpData temp;
#ifdef SDK_TP_AUTO_ADJUST_RANGE        // range�̎��������X�C�b�`
            u16     density;
#endif
			// TWL���[�h
			if (CDC_IsTwlMode())
			{
#ifdef SDK_TP_AUTO_ADJUST_RANGE        // range�̎��������X�C�b�`
				if (TWL_TP_ReadBuffer(&temp, tpw.range, &density))
            	TP_AutoAdjustRange(&temp, density);
#else
				if (TWL_TP_ReadBuffer(&temp, tpw.range))
#endif
				{
		            // �V�X�e���̈�ɏ����o��( 2�o�C�g�A�N�Z�X )
		            *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
		            *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];
				}	
			}
			// DS���[�h
			else
			{
		        // �r������J�n
		        {
		            OSIntrMode e;

		            e = OS_DisableInterrupts();
		            if (!SPIi_CheckException(SPI_DEVICE_TYPE_TP))
		            {
		                (void)OS_RestoreInterrupts(e);
		                // �O���X���b�h�����SPI�r����
		                SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_EXCLUSIVE);
		                return;
		            }
		            SPIi_GetException(SPI_DEVICE_TYPE_TP);
		            (void)OS_RestoreInterrupts(e);
		        }

#ifdef SDK_TP_AUTO_ADJUST_RANGE        // range�̎��������X�C�b�`
            	TP_ExecSampling(&temp, tpw.range, &density);
            	TP_AutoAdjustRange(&temp, density);
#else
            	TP_ExecSampling(&temp, tpw.range);
#endif
	            // �V�X�e���̈�ɏ����o��( 2�o�C�g�A�N�Z�X )
	            *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
	            *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];
			}

	        // �r������I��
	        SPIi_ReleaseException(SPI_DEVICE_TYPE_TP);
        }
        // ARM9�ɏ����̐�����ʒB
        if (entry->process == SPI_PXI_COMMAND_TP_SAMPLING)
        {
            // �P�̃T���v�����O�̉���
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_SUCCESS);
        }
        else
        {
            // �����T���v�����O�̃C���W�P�[�g
            SPIi_ReturnResult((u16)(entry->process), (u16)(entry->arg[0] & 0x00ff));
        }
        break;

        // �����T���v�����O�J�n
    case SPI_PXI_COMMAND_TP_AUTO_ON:
        if (tpw.status == TP_STATUS_AUTO_START)
        {
            s32     i;

            // V�A���[�����N��
            for (i = 0; i < entry->arg[0]; i++)
            {
                // �����C�������T���v�����O�p�x�ŕ������A�eV�J�E���g���v�Z
                tpw.vCount[i] = (u16)((entry->arg[1] +
                                       ((i * HW_LCD_LINES) / entry->arg[0])) % HW_LCD_LINES);
                // V�J�E���g�A���[�����J�n(�����݃n���h���O�łȂ��Ƃ����Ȃ�)
                OS_SetPeriodicVAlarm(&(tpw.vAlarm[i]),
                                     (s16)(tpw.vCount[i]),
                                     TP_VALARM_DELAY_MAX, TpVAlarmHandler, (void *)i);
            }
            // ARM9�ɏ����̐�����ʒB
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_SUCCESS);
            // ������Ԃ��X�V
            tpw.status = TP_STATUS_AUTO_SAMPLING;       // ��Ԃ�"�����T���v�����O��"��
        }
        else
        {
            // ARM9�ɏ����̎��s��ʒB
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_ILLEGAL_STATUS);
        }
        break;

        // �����T���v�����O��~
    case SPI_PXI_COMMAND_TP_AUTO_OFF:
        if (tpw.status == TP_STATUS_AUTO_WAIT_END)
        {
            // V�A���[�����~�߂�
            OS_CancelVAlarms(SPI_TP_VALARM_TAG);
            // ARM9�ɏ����̐�����ʒB
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_SUCCESS);
            // ������Ԃ��X�V
            tpw.status = TP_STATUS_READY;       // ��Ԃ�"�ʏ푀��҂�"��
        }
        else
        {
            // ARM9�ɏ����̎��s��ʒB
            SPIi_ReturnResult((u16)(entry->process), SPI_PXI_RESULT_ILLEGAL_STATUS);
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         TpVAlarmHandler

  Description:  V�J�E���g�A���[���̃n���h���B
                ���T���v�����O����x�Ɏ~�܂�X���b�h���ĊJ������B

  Arguments:    arg - ��������V�J�E���g�A���[����ID

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void TpVAlarmHandler(void *arg)
{
    // TP�T���v�����O������X���b�h�ɗ\��
    if (!SPIi_SetEntry(SPI_DEVICE_TYPE_TP, SPI_PXI_COMMAND_TP_AUTO_SAMPLING, 1, (u32)arg))
    {                                  // �X���b�h�ւ̃T���v�����O�����\��Ɏ��s
        SPITpData temp;

        // �T���v�����O�f�[�^���U��
        temp.e.validity = SPI_TP_VALIDITY_INVALID_XY;
        // �V�X�e���̈�ɏ����o��( 2�o�C�g�A�N�Z�X )
        *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
        *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];
        // �����T���v�����O�̃C���W�P�[�g
        SPIi_ReturnResult(SPI_PXI_COMMAND_TP_AUTO_SAMPLING, (u16)((u32)arg & 0x00ff));
    }
}

/*---------------------------------------------------------------------------*
  Name:         SetStability

  Description:  �^�b�`�p�l���̈��蔻��p�����[�^��ύX�B

  Arguments:    range �l�̌덷��臒l.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetStability(u16 range)
{
    // range�p�����[�^���`�F�b�N
    if (range == 0)
    {
        SPIi_ReturnResult(SPI_PXI_COMMAND_TP_SETUP_STABILITY, SPI_PXI_RESULT_INVALID_PARAMETER);
        return;
    }
    // ���蔻��p�����[�^(�����Ǘ�)��ύX
    tpw.range = (s32)range;
    tpw.rangeMin = (s32)range;

    // ARM9�ɏ����̐�����ʒB
    SPIi_ReturnResult(SPI_PXI_COMMAND_TP_SETUP_STABILITY, SPI_PXI_RESULT_SUCCESS);
    return;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
