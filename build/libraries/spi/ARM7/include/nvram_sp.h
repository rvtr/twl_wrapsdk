/*---------------------------------------------------------------------------*
  Project:  NitroSDK - libraries - spi
  File:     nvram_sp.h

  Copyright 2003-2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: nvram_sp.h,v $
  Revision 1.6  2006/01/18 02:11:30  kitase_hirotake
  do-indent

  Revision 1.5  2005/02/28 05:26:27  yosizaki
  do-indent.

  Revision 1.4  2004/10/07 06:49:34  terui
  NVRAM�ɂ��ĕʃ��[�J�[���̏ꍇ�̃R�}���h�ǉ��ɔ����C���B

  Revision 1.3  2004/09/16 04:56:40  terui
  NVRAM�̃X�e�[�^�X���W�X�^�Ɋւ����`��ǉ��B

  Revision 1.2  2004/09/07 00:34:27  takano_makoto
  SDK_SMALL_BUILD��`����SDK_NVRAM_USE_READ_HIGHER_SPEED�𖢒�`�ɂ���悤�ύX�B

  Revision 1.1  2004/09/06 12:54:18  terui
  libraries/spi/include����libraries/spi/ARM7/include�Ɉړ��B
  SPI�����\������̎����ɔ����C���B

  Revision 1.5  2004/05/25 00:58:01  terui
  SPI�e�f�o�C�X�p���C�u�����ו����ɔ����C��

  Revision 1.4  2004/05/12 10:51:44  terui
  ���C���������ւ̃A�N�Z�X��MI�֐��ɂčs���R���p�C���X�C�b�`��ǉ�
  ReadHigherSpeed�C���X�g���N�V�����̂ݐ؂藣���R���p�C���X�C�b�`��ǉ�

  Revision 1.3  2004/04/29 10:26:09  terui
  �֐���`�̈����폜�ɔ����ύX

  Revision 1.2  2004/04/14 06:26:46  terui
  SPI���C�u�����̃\�[�X�����ɔ����X�V

  Revision 1.1  2004/04/05 04:46:37  terui
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_NVRAM_SP_H_
#define LIBRARIES_NVRAM_SP_H_

#include    <nitro/types.h>
#include    "spi_sp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
// �����ǂݏo���R�}���h�ɂ��Ή�����ꍇ��define����
#ifndef SDK_SMALL_BUILD
#define     SDK_NVRAM_USE_READ_HIGHER_SPEED
#endif
// LE25FW203T�ɓ��L��instruction���K�v�ȏꍇ��define����
#define     SDK_NVRAM_ANOTHER_MAKER

// ���C���������ւ̃A�N�Z�X��MI�̃o�C�g�A�N�Z�X�֐����g���ďȃR�[�h����ꍇ��define
#define     SDK_SPI_LOW_SPEED_LOW_CODE_SIZE

// NVRAM����R�}���h��`
#define     NVRAM_INSTRUCTION_DUMMY         0x00        // �����擾�p�̃_�~�[����
#define     NVRAM_INSTRUCTION_WREN          0x06        // �������݋���
#define     NVRAM_INSTRUCTION_WRDI          0x04        // �������݋֎~
#define     NVRAM_INSTRUCTION_RDSR          0x05        // �X�e�[�^�X���W�X�^�ǂݏo��
#define     NVRAM_INSTRUCTION_READ          0x03        // �ǂݏo��
#define     NVRAM_INSTRUCTION_FAST_READ     0x0b        // �����ǂݏo��
#define     NVRAM_INSTRUCTION_PW            0x0a        // �y�[�W��������
#define     NVRAM_INSTRUCTION_PP            0x02        // �y�[�W��������(�����t)
#define     NVRAM_INSTRUCTION_PE            0xdb        // �y�[�W����
#define     NVRAM_INSTRUCTION_SE            0xd8        // �Z�N�^����
#define     NVRAM_INSTRUCTION_DP            0xb9        // �ȓd��
#define     NVRAM_INSTRUCTION_RDP           0xab        // �ȓd�͂��畜�A
#ifdef  SDK_NVRAM_ANOTHER_MAKER
#define     NVRAM_INSTRUCTION_CE            0xc7        // �`�b�v�C���[�X
#define     NVRAM_INSTRUCTION_RSI           0x9f        // �V���R��ID�ǂݏo��
#define     NVRAM_INSTRUCTION_SR            0xff        // �\�t�g�E�F�A���Z�b�g
#endif

// NVRAM�X�e�[�^�X���W�X�^���r�b�g��`
#define     NVRAM_STATUS_REGISTER_WIP       0x01
#define     NVRAM_STATUS_REGISTER_WEL       0x02
#ifdef  SDK_NVRAM_ANOTHER_MAKER
#define     NVRAM_STATUS_REGISTER_ERSER     0x20
#endif

/*---------------------------------------------------------------------------*
    �\���̒�`
 *---------------------------------------------------------------------------*/

// NVRAM�p���[�N�\����
typedef struct NVRAMWork
{
    u16     command[SPI_PXI_CONTINUOUS_PACKET_MAX];

}
NVRAMWork;


/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/
void    NVRAM_Init();
void    NVRAM_AnalyzeCommand(u32 data);
void    NVRAM_ExecuteProcess(SPIEntry * entry);

void    NVRAM_WriteEnable(void);
void    NVRAM_WriteDisable(void);
void    NVRAM_ReadStatusRegister(u8 *buf);
void    NVRAM_ReadDataBytes(u32 address, u32 size, u8 *buf);
#ifdef  SDK_NVRAM_USE_READ_HIGHER_SPEED
void    NVRAM_ReadDataBytesAtHigherSpeed(u32 address, u32 size, u8 *buf);
#endif
void    NVRAM_PageWrite(u32 address, u16 size, const u8 *buf);
void    NVRAM_PageProgram(u32 address, u16 size, const u8 *buf);
void    NVRAM_PageErase(u32 address);
void    NVRAM_SectorErase(u32 address);
void    NVRAM_DeepPowerDown(void);
void    NVRAM_ReleaseFromDeepPowerDown(void);
#ifdef  SDK_NVRAM_ANOTHER_MAKER
void    NVRAM_ChipErase(void);
void    NVRAM_ReadSiliconId(u8 *buf);
void    NVRAM_SoftwareReset(void);
#endif


/*---------------------------------------------------------------------------*
    �C�����C���֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NVRAM_SPIChangeMode

  Description:  SPI�R���g���[�����W�X�^��ҏW����NVRAM�p�ɃR�}���h�]�������𐮂���B
                ���̎������ɁASPI�A���N���b�N���U���[�h��؂�ւ���B

  Arguments:    continuous - SPI�A���N���b�N���U�ہB'1'��1byte�ʐM�̓s�xCS���グ
                             �Ȃ����[�h�B�������A���]���̍Ō��1�o�C�g�ł�'0'�ɂ���
                             1byte�]�����[�h�ɂ��Ȃ��Ɖi�v��CS���オ��Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void NVRAM_SPIChangeMode(SPITransMode continuous)
{
    reg_SPI_SPICNT = (u16)((0x0001 << REG_SPI_SPICNT_E_SHIFT) |
                           (0x0000 << REG_SPI_SPICNT_I_SHIFT) |
                           (SPI_COMMPARTNER_EEPROM << REG_SPI_SPICNT_SEL_SHIFT) |
                           (continuous << REG_SPI_SPICNT_MODE_SHIFT) |
                           (0x0000 << REG_SPI_SPICNT_BUSY_SHIFT) |
                           (SPI_BAUDRATE_4MHZ << REG_SPI_SPICNT_BAUDRATE_SHIFT));
}


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBRARIES_NVRAM_SP_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
