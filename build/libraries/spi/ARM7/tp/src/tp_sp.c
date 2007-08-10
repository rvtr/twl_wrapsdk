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

#include    "tp_sp.h"
#include    <tp_reg.h>				    //////////////
#include    <twl/cdc/ARM7/cdc.h>	    //////////////
#include    <twl/cdc/ARM7/cdc_api.h>	//////////////


#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <nitro/os/common/printf.h>
#define DBG_PRINTF OS_TPrintf
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_CHAR( c )      ((void)0)
#endif


/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/
static TPWork tpw;

///////////////////////////  TWL
tpData_t    tpData;
///////////////////////////

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

// CODEC�̓d���͌���SndInit��ON�ɂȂ��Ă����͂��B
// CDCInit��ON�ɂ���悤�ɂ��邩�H

// ������CODEC�̏�Ԃ�DS���[�h��TWL���[�h���ɂ����
// ���򂳂���Ƃ����l��������B
// �X���b�h��ʂɂ���B
// TP�̃X���b�h�͂���������SPI�X���b�h�Ƃ��Ă܂Ƃ߂��Ă���񂾂����B
// TP_AnalyzeCommandw��ʂɂ��邾���ł����H
// ����TP_ExecuteProcess�ł���B


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
/*
	    tpSetPrechargeTime(     TP_SETUP_TIME_0_1US );
	    tpSetSenseTime(         TP_SETUP_TIME_0_1US );
	    tpSetStabilizationTime( TP_SETUP_TIME_0_1US );				// �ς�
	    
	    tpSetDebounceTime( TP_DEBOUNCE_16US );
	    
	    tpSetResolution( TP_RESOLUTION_12 );						// �ς�
	    tpSetInterval( TP_INTERVAL_4MS );							// �ς�

	    tpSetTouchPanelDataDepth( TP_DATA_SAMPLE_DEPTH_DEFAULT );	// �ς�

	    tpSetConvertChannel( TP_CHANNEL_XY );						// �ς�
	    
	    tpEnableNewBufferMode();									// �ς�
*/

    	TWL_TP_DisableNewBufferMode();							// for TSC2101 mode
	    TWL_TP_SetResolution( TP_RESOLUTION_12 );
		TWL_TP_SetInterval(TP_INTERVAL_NONE);					// for TSC2101 mode	
	    TWL_TP_SetTouchPanelDataDepth( 5 /*TP_DATA_SAMPLE_DEPTH_DEFAULT*/ );
		TWL_TP_SetConvertChannel( TP_CHANNEL_XY );
		TWL_TP_SetStabilizationTime( TP_SETUP_TIME_100US );		// Y�̍��W���S�������ɂȂ邱�Ƃ�����������0.1us->1us
		TWL_TP_SetSenseTime(         TP_SETUP_TIME_3US );		// Y�̍��W���S�������ɂȂ邱�Ƃ�����������0.1us->1us
		TWL_TP_SetPrechargeTime(     TP_SETUP_TIME_3US );		// Y�̍��W���S�������ɂȂ邱�Ƃ�����������0.1us->1us

		// �V���O���V���b�g���[�h�ɐݒ肷��
		//    CDC_ChangePage( 3 );
		//	CDC_SetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_CONVERSION_MODE_SINGLESHOT, TP_CONVERSION_MODE_MASK );

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

// �r����CDC_ReadSpiRegister(s)�����ł�SPI_Lock()->SPIi_GetException�Ŏ�������
/*
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
*/
        // �T���v�����O�����s
        {
            SPITpData temp;

			if (CDC_IsTwlMode())
			{
//    			OSTick tick = OS_GetTick();	///////////////////////
				
				if (TWL_TP_ReadBuffer(&temp))
				{
		            // �V�X�e���̈�ɏ����o��( 2�o�C�g�A�N�Z�X )
		            *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
		            *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];
				}
	
//				DBG_PRINTF("TWL_TP_ReadBuffer = %6d us\n", OS_TicksToMicroSeconds(OS_GetTick() - tick));	/////////
			}
			else
			{
//    			OSTick tick = OS_GetTick();	/////////////////

#ifdef SDK_TP_AUTO_ADJUST_RANGE        // range�̎��������X�C�b�`
            	u16     density;
            	TP_ExecSampling(&temp, tpw.range, &density);
            	TP_AutoAdjustRange(&temp, density);
#else
            	TP_ExecSampling(&temp, tpw.range);
#endif
	            // �V�X�e���̈�ɏ����o��( 2�o�C�g�A�N�Z�X )
	            *((u16 *)(&(OS_GetSystemWork()->touch_panel[0]))) = temp.halfs[0];
	            *((u16 *)(&(OS_GetSystemWork()->touch_panel[2]))) = temp.halfs[1];

//				DBG_PRINTF("TP_ExecSampling = %6d us\n", OS_TicksToMicroSeconds(OS_GetTick() - tick));	/////////
			}
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
        // �r������I��
//      SPIi_ReleaseException(SPI_DEVICE_TYPE_TP);
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
  Name:         TWL_TP_SetTouchPanelDataDepth

  Description:  set touch-panel data depth (1 <= depth <= 8)
  
  Arguments:    int depth : data depth (1<= depth <= 8)

  Returns:      None
 *---------------------------------------------------------------------------*/ //TODO: depth�̐؂�ւ���Page3, Reg14, D7 is "0"�̏�Ԃōs���悤�ɏC������
void TWL_TP_SetTouchPanelDataDepth( u8 depth )
{
    u8 tmp;
    
    SDK_ASSERT( (1 <= depth) && (depth <= 8) );
    
    tmp = (u8)(depth << TP_DATA_DEPTH_SHIFT);
    if (depth == 8) tmp = 0;
    
	CDC_ChangePage( 3 );
    CDC_SetI2cParams( REG_TP_DATA_DEPTH, tmp, TP_DATA_DEPTH_MASK );

    tpData.tpDepth = depth;
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetConvertChannel

  Description:  set ADC target channel
  
  Arguments:    TpChannel_t ch : Convert Channel

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetConvertChannel( TpChannel_t ch )
{
    SDK_ASSERT( (ch == TP_CHANNEL_NONE)   || (ch == TP_CHANNEL_XY)       ||
                (ch == TP_CHANNEL_XYZ)    || (ch == TP_CHANNEL_X)        ||
                (ch == TP_CHANNEL_Y)      || (ch == TP_CHANNEL_Z)        ||
                (ch == TP_CHANNEL_AUX3)   || (ch == TP_CHANNEL_AUX2)     ||
                (ch == TP_CHANNEL_AUX1)   || (ch == TP_CHANNEL_AUTO_AUX) ||
                (ch == TP_CHANNEL_AUX123) || (ch == TP_CHANNEL_XP_XM)    ||
                (ch == TP_CHANNEL_YP_YM)  || (ch == TP_CHANNEL_YP_XM)      );
    
	CDC_ChangePage( 3 );

//  i_tpWriteSpiRegister( REG_TP_CHANNEL, ch );
//	cdcWriteI2cRegister( REG_TP_CHANNEL, ch );
//	CDC_WriteI2cRegister( REG_TP_CHANNEL, ch );	// TODO: �}�X�N���������ɏC������

//	CDC_WriteI2cRegister( REG_TP_CHANNEL, (ch & 0x7f) );	// �����Ƀz�X�g�R���g���[�����[�h

	CDC_WriteI2cRegister( REG_TP_CHANNEL, (u8)(ch & 0xfd) );	// 2101 & self
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetInterval

  Description:  set Touch-Panel / AUX Interval Time
                Either Touch-Panel or AUX can be enabled, the last setting
                is only valid. Normally, Touch-Panel is enabled.
  
  Arguments:    tpInterval_t interval : interval time between sampling

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetInterval( tpInterval_t interval )
{
    SDK_ASSERT( (interval == TP_INTERVAL_NONE) || 
                (interval == TP_INTERVAL_8MS)  || (interval == TP_AUX_INTERVAL_1_12M) ||
                (interval == TP_INTERVAL_1MS)  || (interval == TP_AUX_INTERVAL_3_36M) ||
                (interval == TP_INTERVAL_2MS)  || (interval == TP_AUX_INTERVAL_5_59M) ||
                (interval == TP_INTERVAL_3MS)  || (interval == TP_AUX_INTERVAL_7_83M) ||
                (interval == TP_INTERVAL_4MS)  || (interval == TP_AUX_INTERVAL_10_01M) ||
                (interval == TP_INTERVAL_5MS)  || (interval == TP_AUX_INTERVAL_12_30M) ||
                (interval == TP_INTERVAL_6MS)  || (interval == TP_AUX_INTERVAL_14_54M) ||
                (interval == TP_INTERVAL_7MS)  || (interval == TP_AUX_INTERVAL_16_78M)
              );
    
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );

//  i_tpWriteSpiRegister( REG_TP_INTERVAL, interval );
//  cdcWriteI2cRegister( REG_TP_INTERVAL, interval );
	CDC_WriteI2cRegister( REG_TP_INTERVAL, interval );
}

/*---------------------------------------------------------------------------*
  Name:         tpEnableNewBufferMode

  Description:  enable new buffer mode
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_EnableNewBufferMode( void )
{
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );

//  i_tpSetSpiParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_E, TP_NEW_BUFFER_MODE_MASK );
//  i_tpSetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_E, TP_NEW_BUFFER_MODE_MASK );
    CDC_SetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_E, TP_NEW_BUFFER_MODE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         tpDisableNewBufferMode

  Description:  disable new buffer mode
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_DisableNewBufferMode( void )
{
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );

//  i_tpSetSpiParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_D, TP_NEW_BUFFER_MODE_MASK );
//  i_tpSetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_D, TP_NEW_BUFFER_MODE_MASK );
    CDC_SetI2cParams( REG_TP_NEW_BUFFER_MODE, TP_NEW_BUFFER_MODE_D, TP_NEW_BUFFER_MODE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetResolution

  Description:  set AD Converting Resolution (8, 10, or 12-bit)
  
  Arguments:    TpResolution_t res : Converting Resolution

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetResolution( TpResolution_t res )
{
    SDK_ASSERT( (res == TP_RESOLUTION_12) || 
                (res == TP_RESOLUTION_8)  ||
                (res == TP_RESOLUTION_10) );
    
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );


//  i_tpSetSpiParams( REG_TP_RESOLUTION, res, TP_RESOLUTION_MASK );
//  i_tpSetI2cParams( REG_TP_RESOLUTION, res, TP_RESOLUTION_MASK );
    CDC_SetI2cParams( REG_TP_RESOLUTION, res, TP_RESOLUTION_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_GetResolution

  Description:  get AD Converting Resolution (8, 10, or 12-bit)
  
  Arguments:    TpResolution_t *res : Converting Resolution

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_GetResolution( TpResolution_t *res )
{
//  i_tpChangePage( 3 );
	CDC_ChangePage( 3 );

//  *res = (TpResolution_t)(i_tpReadSpiRegister( REG_TP_RESOLUTION ) & TP_RESOLUTION_MASK);
//  *res = (TpResolution_t)( cdcReadI2cRegister( REG_TP_RESOLUTION ) & TP_RESOLUTION_MASK);
    *res = (TpResolution_t)( CDC_ReadI2cRegister( REG_TP_RESOLUTION ) & TP_RESOLUTION_MASK);
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetStabilizationTime

  Description:  set ADC stabilization time before touch detection
  
  Arguments:    TpSetupTime_t time : stabilization time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetStabilizationTime( TpSetupTime_t time )
{
    SDK_ASSERT( (TP_SETUP_TIME_0_1US <= time) || (time <= TP_SETUP_TIME_1MS) );
    
	CDC_ChangePage( 3 );
	CDC_SetI2cParams( REG_TP_STABILIZATION_TIME, time, TP_STABILIZATION_TIME_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetPrechargeTime

  Description:  set ADC precharge time before touch detection
  
  Arguments:    TpSetupTime_t time : precharge time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetPrechargeTime( TpSetupTime_t time )
{
    SDK_ASSERT( (TP_SETUP_TIME_0_1US <= time) || (time <= TP_SETUP_TIME_1MS) );
    
    CDC_ChangePage( 3 );
    CDC_SetI2cParams( REG_TP_PRECHARGE, (u8)(time << TP_PRECHARGE_SHIFT), TP_PRECHARGE_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_SetSenseTime

  Description:  set ADC sense time before touch detection
  
  Arguments:    TpSetupTime_t time : sense time

  Returns:      None
 *---------------------------------------------------------------------------*/
void TWL_TP_SetSenseTime( TpSetupTime_t time )
{
    SDK_ASSERT( (TP_SETUP_TIME_0_1US <= time) || (time <= TP_SETUP_TIME_1MS) );
    
    CDC_ChangePage( 3 );
    CDC_SetI2cParams( REG_TP_SENSE_TIME, time, TP_SENSE_TIME_MASK );
}

/*---------------------------------------------------------------------------*
  Name:         TWL_TP_IS_TOUCH

  Description:  �^�b�`�p�l���ڐG����
 
  Arguments:    none

  Returns:      BOOL : if touched, return TRUE. otherwise FALSE.
 *---------------------------------------------------------------------------*/
static BOOL TWL_TP_IS_TOUCH( void )
{
	vu8 penup = 0;

	penup = CDC_ReadSpiRegister( 9 );
	if ((penup & 0x80) == 0x00)
	{
		penup = CDC_ReadSpiRegister( 9 );
		if ((penup & 0x80) == 0x00)
		{
			penup = CDC_ReadSpiRegister( 9 );
			if ((penup & 0x80) == 0x00)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


#define ABS(x) ( ( (x) >= 0 ) ? (x) : ( -(x) ) )
/*---------------------------------------------------------------------------*
  Name:         TWL_TP_ReadBuffer

  Description:  read Touch-Panel Buffer
  
  Arguments:    data : �f�[�^�i�[�|�C���^

  Returns:      BOOL : if read success, return TRUE. otherwise FALSE.
 *---------------------------------------------------------------------------*/
BOOL TWL_TP_ReadBuffer( SPITpData *data )
{
    int i;
    int target_index = 0;   // ������f�[�^���i�[����̈�̐擪�C���f�b�N�X
    u8  buf[32];
	u8 not_readready;

//	(void)cdcLock();	// CODEC�f�o�C�X�̑��쌠�����擾

    CDC_ChangePage( 3 );


//  �y���A�b�v����
	if (!TWL_TP_IS_TOUCH())
	{
        data->e.touch    = FALSE;
		data->e.validity = FALSE;
        return TRUE;	// �����̓y���A�b�v�Ƃ��ăV�X�e���̈�ɏ����o��
	}

/*
	//----- Available�`�F�b�N
	not_readready = (u8)(CDC_ReadI2cRegister( 9 ));
	if ((not_readready && 0x0c) == 0x0c)
	{
		(void)cdcUnlock();	// CODEC�f�o�C�X�̑��쌠������� (�Y�ꂸ�Ɂj
		return FALSE;
	}
*/
    
    if (tpData.tpIndex == 0)
        target_index = TP_DATA_SAMPLE_DEPTH_MAX;


	for (i=0;i<tpData.tpDepth;i++)
	{
//		OS_SpinWait(100);		// �T���v�����O���Ԃ��s�����Ă���悤�Ȃ�ǉ��������i�����͖{�����f�B�`�F�b�N����ׂ��j

		// XY�T���v�����O
		CDC_ReadSpiRegisters( 42, &buf[i*4], 4 );

		//  �T���v�����O��̃y���A�b�v���� (�T���v�����O���Ԃ��҂���j
		if (!TWL_TP_IS_TOUCH())
		{
		    return FALSE;	// �V�X�e���̈�ɂ͏����o���Ȃ�
		}
	}


    for (i=0; i<tpData.tpDepth; i++)
    {
        tpData.xBuf[target_index+i]  = (u16)((buf[i*4]     << 8) | buf[i*4 + 1]);
        tpData.yBuf[target_index+i]  = (u16)((buf[i*4 + 2] << 8) | buf[i*4 + 3]);
    }
    
    tpData.tpIndex = target_index;

//	(void)cdcUnlock();	// CODEC�f�o�C�X�̑��쌠�������

	{
	    int i, j, index;
	    int xSum = 0;
		int ySum = 0;
		int same_required = (tpData.tpDepth >> 1) + 1;
		int same_chance   = tpData.tpDepth - same_required + 1;
		int same_count = 0;

	    index  = tpData.tpIndex;

/*
		// �y���A�b�vbit�`�F�b�N
	    for (i=0; i<tpData.tpDepth; i++, index++)
	    {
	        if ((tpData.xBuf[index] & TP_NOT_TOUCH_MASK) || (tpData.yBuf[index] & TP_NOT_TOUCH_MASK))
	        {
	            data->e.touch    = FALSE;
				data->e.validity = FALSE;
	            return TRUE;	// �����̓y���A�b�v�Ƃ��ăV�X�e���̈�ɏ����o��
	        }
//	        xSum += tpData.xBuf[index];
//	        ySum += tpData.yBuf[index];
	    }
*/

	    index  = tpData.tpIndex;

	    // �T���v�����O�������̔����ȏオrange�ȓ��ł����valid�ȃf�[�^�Ƃ���B
		for (i=0; i<same_chance; i++)
		{
			same_count = 0;
			xSum = tpData.xBuf[index + i];
			ySum = tpData.yBuf[index + i];

			for (j=0; j<tpData.tpDepth; j++)
			{
				if (i==j) { continue; }
				if ((ABS( tpData.xBuf[index + i] - tpData.xBuf[index + j] ) < 5) && 
					(ABS( tpData.yBuf[index + i] - tpData.yBuf[index + j] ) < 5))
				{
					same_count++;
					xSum += tpData.xBuf[index + j];
					ySum += tpData.yBuf[index + j];
				}
			}

			if (same_count >= same_required) { break; }
		}

		if (same_count < same_required)
		{
	            return FALSE;	// �V�X�e���̈�ɂ͏����o���Ȃ�
		}

		data->e.x        = xSum / (same_count+1);
		data->e.y        = ySum / (same_count+1);
	    data->e.touch    = TRUE;
	    data->e.validity = TRUE;

//DBG_PRINTF("x : %4d %4d %4d %4d %4d %4d %4d %4d -> %4d\n",   tpData.xBuf[index], tpData.xBuf[index + 1], tpData.xBuf[index + 2], tpData.xBuf[index + 3], tpData.xBuf[index + 4], tpData.xBuf[index + 5], tpData.xBuf[index + 6], tpData.xBuf[index + 7], data->e.x);   
//DBG_PRINTF("y : %4d %4d %4d %4d %4d %4d %4d %4d -> %4d\n\n", tpData.yBuf[index], tpData.yBuf[index + 1], tpData.yBuf[index + 2], tpData.yBuf[index + 3], tpData.yBuf[index + 4], tpData.yBuf[index + 5], tpData.yBuf[index + 6], tpData.yBuf[index + 7], data->e.y);   

	}

    return TRUE;
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
