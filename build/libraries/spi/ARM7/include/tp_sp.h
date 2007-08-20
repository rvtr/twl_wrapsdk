/*---------------------------------------------------------------------------*
  Project:  NitroSDK - libraries - spi
  File:     tp_sp.h

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: tp_sp.h,v $
  Revision 1.7  2006/01/18 02:12:27  kitase_hirotake
  do-indent

  Revision 1.6  2005/02/28 05:26:27  yosizaki
  do-indent.

  Revision 1.5  2004/12/29 02:03:59  takano_makoto
  SetStability�֐���retry�p�����[�^��p�~, range�̎��������X�C�b�`��L���ɕύX

  Revision 1.4  2004/12/20 00:40:55  takano_makoto
  �����T���v�����O����VAlarm��delay��ݒ�

  Revision 1.3  2004/12/15 09:12:40  takano_makoto
  range�̎���������ǉ��BSDK_TP_AUTO_ADJUST_RANGE�ɂ���ėL���ɂȂ�B

  Revision 1.2  2004/11/05 05:47:24  terui
  SPI�R�}���h�ɂ��Ă̐��񎖍����R�����g�ɒǉ��B

  Revision 1.1  2004/09/06 12:54:18  terui
  libraries/spi/include����libraries/spi/ARM7/include�Ɉړ��B
  SPI�����\������̎����ɔ����C���B

  Revision 1.9  2004/08/09 13:23:25  takano_makoto
  MIC�����T���v�����O�̍��Ԃ�TP�̂P��T���v�����O�����s�ł���悤�ɏC��

  Revision 1.8  2004/07/31 08:06:45  terui
  �T���v�����O�R�}���h���^�[�Q�b�gHW�ɂ��U�蕪����d�l��ǉ�

  Revision 1.7  2004/06/17 11:07:43  terui
  �ڐG����R�}���h��`��ύX�B0x94 -> 0x84

  Revision 1.6  2004/05/25 00:58:01  terui
  SPI�e�f�o�C�X�p���C�u�����ו����ɔ����C��

  Revision 1.5  2004/05/14 04:50:36  yosiokat
  TEG��TS�̗����ɑΉ�����悤�ύX

  Revision 1.4  2004/04/29 10:25:59  terui
  �֐���`�̈����폜�ɔ����ύX

  Revision 1.3  2004/04/14 06:26:46  terui
  SPI���C�u�����̃\�[�X�����ɔ����X�V

  Revision 1.2  2004/04/05 04:47:05  terui
  Change composition.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_TP_SP_H_
#define LIBRARIES_TP_SP_H_

#include    <nitro/types.h>
#include    "spi_sp.h"
/////////////////////////////// TWL
#include    "tp_reg.h"
///////////////////////////////


#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
/* 2004/11/04 �ƈ�
 *
 * �V�K�ʃf�o�C�X�̂��߂ɁA�ȉ��̃R�}���h�ȊO�͗\��Ƃ��܂��B
 * ARM7��TP�R���g���[���ɂ����ȊO�̃R�}���h�𔭍s����K�v������ꍇ��
 * �\�񂳂�Ă��邱�Ƃ��l�����Ē��ӂ��Ďg�p���ĉ������B
 *
 * 0x0000   �_�~�[����
 * 0x0084   �ڐG����
 * 0x0091   Y�����T���v�����O
 * 0x0093   Y�����T���v�����O(���o�[�W����)
 * 0x00D1   X�����T���v�����O
 * 0x00D3   X�����T���v�����O(���o�[�W����)
 *
 */
#ifdef  SDK_TEG
    // �������t�@�����X���g�p
#define     TP_COMMAND_SAMPLING_X       0x00D3  // X�����T���v�����O����
#define     TP_COMMAND_SAMPLING_Y       0x0093  // Y�����T���v�����O����
#else  // SDK_TS
#if ( SDK_TS_VERSION >= 100 )          // ��Ver.D�ȍ~
        // �O�����t�@�����X���g�p
#define     TP_COMMAND_SAMPLING_X       0x00D1  // X�����T���v�����O����
#define     TP_COMMAND_SAMPLING_Y       0x0091  // Y�����T���v�����O����
#else  // ��Ver.D����
        // �������t�@�����X���g�p
#define     TP_COMMAND_SAMPLING_X       0x00D3  // X�����T���v�����O����
#define     TP_COMMAND_SAMPLING_Y       0x0093  // Y�����T���v�����O����
#endif
#endif

#define     TP_COMMAND_DETECT_TOUCH     0x0084  // �ڐG���薽��

#define     TP_VALID_BIT_MASK           0x7ff8  // �L���f�[�^�r�b�g
#define     TP_VALID_BIT_SHIFT          3       // xooooooo oooooxxx

#define     TP_VALARM_DELAY_MAX         10

/*---------------------------------------------------------------------------*
    �\���̒�`
 *---------------------------------------------------------------------------*/
// �^�b�`�p�l���Ɋւ�������I�ȏ��
typedef enum TPStatus
{
    TP_STATUS_READY = 0,               // �ʏ푀��҂����
    TP_STATUS_AUTO_START,              // �����T���v�����O�J�n�҂����
    TP_STATUS_AUTO_SAMPLING,           // �����T���v�����O��
    TP_STATUS_AUTO_WAIT_END            // �����T���v�����O��~�҂����
}
TPStatus;

// �^�b�`�p�l���p���[�N�\����
typedef struct TPWork
{
    u16     command[SPI_PXI_CONTINUOUS_PACKET_MAX];
    TPStatus status;                   // �^�b�`�p�l��������ԊǗ��ϐ�
    s32     range;                     // ���蔻�莞�̋��e����U�ꕝ�̃f�t�H���g�l
    s32     rangeMin;                  // ���蔻�莞�̋��e����U�ꕝ�̍ŏ��l(range���������Ŏg�p)
    OSVAlarm vAlarm[SPI_TP_SAMPLING_FREQUENCY_MAX];     // �^�b�`�p�l���pV�A���[��
    u16     vCount[SPI_TP_SAMPLING_FREQUENCY_MAX];      // �^�b�`�p�l���pV�J�E���g�ޔ��G���A

}
TPWork;


/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/
void    TP_Init(void);
void    TP_AnalyzeCommand(u32 data);
void    TP_ExecuteProcess(SPIEntry * entry);

//////////////////////////// TWL
void    TWL_TP_Init(void);
//void    TWL_TP_AnalyzeCommand(u32 data);
//void    TWL_TP_ExecuteProcess(SPIEntry * entry);

void    TWL_TP_SetStabilizationTime( TpSetupTime_t time );
void    TWL_TP_SetPrechargeTime( TpSetupTime_t time );
void    TWL_TP_SetSenseTime( TpSetupTime_t time );
void    TWL_TP_SetResolution( TpResolution_t res );
void    TWL_TP_GetResolution( TpResolution_t *res );
void    TWL_TP_SetTouchPanelDataDepth(u8 depth);
void    TWL_TP_SetConvertChannel( TpChannel_t ch );
void    TWL_TP_SetInterval( tpInterval_t interval );
void    TWL_TP_EnableNewBufferMode( void );
void    TWL_TP_DisableNewBufferMode( void );
BOOL    TWL_TP_ReadBuffer( SPITpData *data );
////////////////////////////

#define SDK_TP_AUTO_ADJUST_RANGE       // range���������X�C�b�`

#ifdef SDK_TP_AUTO_ADJUST_RANGE        // range���������X�C�b�`
void    TP_ExecSampling(SPITpData *data, s32 range, u16 *density);
#else
void    TP_ExecSampling(SPITpData *data, s32 range);
#endif

/*---------------------------------------------------------------------------*
    �C�����C���֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         TP_SPIChangeMode

  Description:  SPI�R���g���[�����W�X�^��ҏW���ă^�b�`�p�l���p�R�}���h�]��������
                ������B���̎������ɁASPI�A���N���b�N���U���[�h��؂�ւ���B
                
                CODEC�̐����ɂ��A
                DS���[�h�ł�SPI�N���b�N��2MHz�����x�ƂȂ�̂Œ��ӁB
                TWL���[�h�ł�SPI�N���b�N��4MHz�ƂȂ邽�߁A
                ���̊֐��ł͂Ȃ�CDCi_ChangeSpiMode���g�p����B

  Arguments:    continuous - SPI�A���N���b�N���U�ہB'1'��1byte�ʐM�̓s�xCS���グ
                             �Ȃ����[�h�B�������A���]���̍Ō��1�o�C�g�ł�'0'�ɂ���
                             1byte�]�����[�h�ɂ��Ȃ��Ɖi�v��CS���オ��Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void TP_SPIChangeMode(SPITransMode continuous)
{
    reg_SPI_SPICNT = (u16)((0x0001 << REG_SPI_SPICNT_E_SHIFT) | (0x0000 << REG_SPI_SPICNT_I_SHIFT) | (SPI_COMMPARTNER_TP << REG_SPI_SPICNT_SEL_SHIFT) | // �g���ύX
//      ( SPI_COMMPARTNER_PMIC << REG_SPI_SPICNT_SEL_SHIFT ) |              // �g���ύX�O
                           (continuous << REG_SPI_SPICNT_MODE_SHIFT) |
                           (0x0000 << REG_SPI_SPICNT_BUSY_SHIFT) |
                           (SPI_BAUDRATE_2MHZ << REG_SPI_SPICNT_BAUDRATE_SHIFT));
}



/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBRARIES_TP_SP_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
