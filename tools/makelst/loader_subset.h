/*---------------------------------------------------------------------------*
  Project:  CTR - ELF Loader
  File:     loader_subset.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef _LOADER_SUBSET_H_
#define _LOADER_SUBSET_H_

#include "types.h"
#include "elf.h"
#include "elf_loader.h"


/*------------------------------------------------------
  �Z�N�V�������o�b�t�@�ɃR�s�[����
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  �Z�N�V�������o�b�t�@�Ɋm�ۂ���i�R�s�[���Ȃ��j
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr);
    

/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�����w�b�_���o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void ELi_GetShdr( ELHandle* ElfHandle, u32 index, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�����̃G���g�����o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void ELi_GetSent( ELHandle* ElfHandle, u32 index, void* entry_buf, u32 offset, u32 size);


/*------------------------------------------------------
  �w��Z�N�V�����w�b�_�̎w��C���f�b�N�X�̃G���g�����o�b�t�@�Ɏ擾����
	�i�G���g���T�C�Y���Œ�̃Z�N�V�����̂݁j
 -----------------------------------------------------*/
void ELi_GetEntry( ELHandle* ElfHandle, Elf32_Shdr* Shdr, u32 index, void* entry_buf);


/*------------------------------------------------------
  STR�Z�N�V�����w�b�_�̎w��C���f�b�N�X�̕�������擾����
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELHandle* ElfHandle, u32 strsh_index, u32 ent_index, char* str, u32 len);


/*------------------------------------------------------
  �V���{�����Ē�`����
 -----------------------------------------------------*/
void ELi_RelocateSym( ELHandle* ElfHandle, u32 relsh_index);


/*------------------------------------------------------
    makelst��p�֐�
    �V���{���Z�N�V�����̒�����GLOBAL�Ȃ��̂�
    �A�h���X�e�[�u���ɓo�^����
 -----------------------------------------------------*/
void ELi_DiscriminateGlobalSym( ELHandle* ElfHandle, u32 symsh_index);


/*------------------------------------------------------
  �������������ƂɃV���{������������
 -----------------------------------------------------*/
u32	ELi_DoRelocate( ELUnresolvedEntry* UnresolvedInfo);


/*------------------------------------------------------
  ���X�g����w��C���f�b�N�X��ELSymEx�����o��
 -----------------------------------------------------*/
ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index);


/*------------------------------------------------------
  ���X�g����w��C���f�b�N�X��ELShdrEx�����o��
 -----------------------------------------------------*/
ELShdrEx* ELi_GetShdrExfromList( ELShdrEx* ShdrExStart, u32 index);


/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�������f�o�b�O��񂩂ǂ������肷��
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELHandle* ElfHandle, u32 index);


/*------------------------------------------------------
  ElfHandle��SymEx�e�[�u���𒲂ׁA�w��C���f�b�N�X��
�@�w��I�t�Z�b�g�ɂ���R�[�h��ARM��THUMB���𔻒肷��
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELHandle* ElfHandle, u16 sh_index, u32 offset);


/*---------------------------------------------------------
 ���������G���g��������������
 --------------------------------------------------------*/
void ELi_UnresolvedInfoInit( ELUnresolvedEntry* UnresolvedInfo);


/*------------------------------------------------------
  ���������e�[�u������G���g�����폜����
 -----------------------------------------------------*/
BOOL ELi_RemoveUnresolvedEntry( ELUnresolvedEntry* UnrEnt);


/*---------------------------------------------------------
 ���������e�[�u���ɃG���g����ǉ�����
 --------------------------------------------------------*/
void ELi_AddUnresolvedEntry( ELUnresolvedEntry* UnrEnt);


/*------------------------------------------------------
  ���������e�[�u������w�蕶����ɊY������G���g����T��
 -----------------------------------------------------*/
ELUnresolvedEntry* ELi_GetUnresolvedEntry( char* ent_name);


#endif	/*_LOADER_SUBSET_H_*/
