/*---------------------------------------------------------------------------*
  Project:  CTR - ELF Loader
  File:     loader_subset.h

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef _LOADER_SUBSET_H_
#define _LOADER_SUBSET_H_

#include "elf.h"
#include "elf_loader.h"


/*------------------------------------------------------
  �x�j�����o�b�t�@�ɃR�s�[����
    start : �Ăяo�����A�h���X
    data : ��ѐ�A�h���X
 -----------------------------------------------------*/
void* i_elCopyVeneerToBuffer( ElDesc* elElfDesc, u32 start, u32 data, s32 threshold);


/*------------------------------------------------------
  �x�j�����o�b�t�@�ɃR�s�[����
    start : �Ăяo����
    data : ��ѐ�
    threshold : ���͈͓̔��Ɋ��Ƀx�j��������Ύg���܂킷
 -----------------------------------------------------*/
void* i_elCopyV4tVeneerToBuffer( ElDesc* elElfDesc, u32 start, u32 data, s32 threshold);


/*------------------------------------------------------
  �Z�O�����g���o�b�t�@�ɃR�s�[����
 -----------------------------------------------------*/
void* i_elCopySegmentToBuffer( ElDesc* elElfDesc, Elf32_Phdr* Phdr);


/*------------------------------------------------------
  �Z�N�V�������o�b�t�@�ɃR�s�[����
 -----------------------------------------------------*/
void* i_elCopySectionToBuffer( ElDesc* elElfDesc, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  �Z�N�V�������o�b�t�@�Ɋm�ۂ���i�R�s�[���Ȃ��j
 -----------------------------------------------------*/
void* i_elAllocSectionToBuffer( ElDesc* elElfDesc, Elf32_Shdr* Shdr);
    

/*------------------------------------------------------
  �w��C���f�b�N�X�̃v���O�����w�b�_���o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void i_elGetPhdr( ElDesc* elElfDesc, u32 index, Elf32_Phdr* Phdr);


/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�����w�b�_���o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void i_elGetShdr( ElDesc* elElfDesc, u32 index, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�����̃G���g�����o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void i_elGetSent( ElDesc* elElfDesc, u32 index, void* entry_buf, u32 offset, u32 size);


/*------------------------------------------------------
  �w��Z�N�V�����w�b�_�̎w��C���f�b�N�X�̃G���g�����o�b�t�@�Ɏ擾����
  �i�G���g���T�C�Y���Œ�̃Z�N�V�����̂݁j
 -----------------------------------------------------*/
void i_elGetEntry( ElDesc* elElfDesc, Elf32_Shdr* Shdr, u32 index, void* entry_buf);


/*------------------------------------------------------
  STR�Z�N�V�����w�b�_�̎w��C���f�b�N�X�̕�������擾����
 -----------------------------------------------------*/
void i_elGetStrAdr( ElDesc* elElfDesc, u32 strsh_index, u32 ent_index, char* str, u32 len);


/*------------------------------------------------------
  �V���{�����Ē�`����
 -----------------------------------------------------*/
void i_elRelocateSym( ElDesc* elElfDesc, u32 relsh_index);

/*------------------------------------------------------
  �O���[�o���V���{�����A�h���X�e�[�u���ɓ����
 -----------------------------------------------------*/
void i_elGoPublicGlobalSym( ElDesc* elElfDesc, u32 symtblsh_index);

/*------------------------------------------------------
  i_elRelocateSym��i_elGoPublicGlobalSym�̒��ō쐬�����V���{�����X�g��
  �J������i�ǂ�����ĂяI�������Ō�ɂ���API���Ă�ł��������j
 -----------------------------------------------------*/
void i_elFreeSymList( ElDesc* elElfDesc);

/*------------------------------------------------------
  �������������ƂɃV���{������������
 -----------------------------------------------------*/
BOOL i_elDoRelocate( ElDesc* elElfDesc, ElUnresolvedEntry* UnresolvedInfo);


/*------------------------------------------------------
  ���X�g����w��C���f�b�N�X��ElSymEx�����o��
 -----------------------------------------------------*/
//ElSymEx* i_elGetSymExfromList( ElSymEx* SymExStart, u32 index);


/*------------------------------------------------------
  ���X�g����w��C���f�b�N�X��ElShdrEx�����o��
 -----------------------------------------------------*/
ElShdrEx* i_elGetShdrExfromList( ElShdrEx* ShdrExStart, u32 index);


/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�������f�o�b�O��񂩂ǂ������肷��
 -----------------------------------------------------*/
BOOL i_elShdrIsDebug( ElDesc* elElfDesc, u32 index);


/*------------------------------------------------------
  elElfDesc��SymEx�e�[�u���𒲂ׁA�w��C���f�b�N�X��
�@�w��I�t�Z�b�g�ɂ���R�[�h��ARM��THUMB���𔻒肷��
 -----------------------------------------------------*/
u32 i_elCodeIsThumb( ElDesc* elElfDesc, u16 sh_index, u32 offset);


/*---------------------------------------------------------
 ���������G���g��������������
 --------------------------------------------------------*/
//void i_elUnresolvedInfoInit( ElUnresolvedEntry* UnresolvedInfo);


/*---------------------------------------------------------
 ���������e�[�u���ɃG���g����ǉ�����
 --------------------------------------------------------*/
//void i_elAddUnresolvedEntry( ElUnresolvedEntry* UnrEnt);


void*     i_elFreeVenTbl( void);


#endif /*_LOADER_SUBSET_H_*/
