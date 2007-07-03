/*---------------------------------------------------------------------------*
  Project:  TWL - ELF Loader
  File:     elf_loader.h

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#ifndef _ELF_LOADER_H_
#define _ELF_LOADER_H_

#include <stdio.h>
#include "types.h"
#include "elf.h"


/*------------------------------------------------------
  �Z�N�V�����w�b�_�g��(���[�h�A�h���X���ɑΉ�)
 -----------------------------------------------------*/
typedef struct {
  void*			next;
  u16			index;
  u16			debug_flag;		/*0:�f�o�b�O���łȂ��A1:�f�o�b�O���*/
  u32			loaded_adr;
  u32			alloc_adr;
  u32			loaded_size;
  Elf32_Shdr	Shdr;
  char*         str;            /*�Z�N�V��������������R�s�[���Ă�������*/
  u32*          sym_table;      /*�V���{���Z�N�V�����̂Ƃ��L���ȐV���Ή��\*/
  void*         str_table;      /*STR�Z�N�V�����̂Ƃ��L���F�V������e�[�u��*/
  u32           str_table_size; /*STR�Z�N�V�����̂Ƃ��L���F�V������e�[�u���̃T�C�Y*/
}ELShdrEx;


/*------------------------------------------------------
  �V���{���G���g���g��(���[�h�A�h���X���ɑΉ�)
 -----------------------------------------------------*/
typedef struct {
  void*		next;
  u16		debug_flag;			/*0:�f�o�b�O���łȂ��A1:�f�o�b�O���*/
  u16		thumb_flag;
  u32		relocation_val;
  Elf32_Sym	Sym;
}ELSymEx;



/*------------------------------------------------------
  ELF�I�u�W�F�N�g�̊Ǘ�
 -----------------------------------------------------*/
typedef void (*ELi_ReadFunc)( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
typedef struct {
  void*			ar_head;		/* AR�܂���ELF�t�@�C���̐擪�A�h���X */
  void*			elf_offset;		/* ELF�I�u�W�F�N�g�̐擪�ւ̃I�t�Z�b�g */
  void*			buf_current;	/* Loader��Ɨp */
  u16			shentsize;		/* 1�Z�N�V�����w�b�_�̃T�C�Y */
  u16			process;		/* �Ĕz�u�� */

  ELShdrEx*		ShdrEx;			/* ShdrEx���X�g�̐擪 */

  Elf32_Ehdr	CurrentEhdr;	/* ELF�w�b�_ */

  Elf32_Rel		Rel;			/* �Ĕz�u�G���g�� */
  Elf32_Rela	Rela;
  Elf32_Sym		Sym;			/* �V���{���G���g�� */
  Elf32_Shdr	SymShdr;

  ELSymEx*		SymEx;			/* SymEx���X�g�̐擪 */
  ELSymEx**     SymExTbl;       /* SymEx�A�h���X�̃e�[�u���i�S�V���{�����Ԃ�j*/
  u32           SymExTarget;    /* SymEx���X�g���\�z�����V���{���Z�N�V�����̃Z�N�V�����ԍ� */

  ELi_ReadFunc	ELi_ReadStub;	/* ���[�h�X�^�u�֐� */
  void*			FileStruct;		/* �t�@�C���\���� */

  u32			mem_adr;		/*�ŏ��Ƀ��[�h���ꂽ�Z�N�V������sh_addr������(DS��ROM�w�b�_�p�p�����[�^)*/
  u32           newelf_size;
}ELHandle;



/*------------------------------------------------------
  �A�h���X�e�[�u��
 -----------------------------------------------------*/
typedef struct {
  void*		next;				/*���̃A�h���X�G���g��*/
  char*		name;				/*������*/
  void*		adr;				/*�A�h���X*/
  u16		func_flag;			/*0:�f�[�^�A1:�֐�*/
  u16		thumb_flag;			/*0:arm�R�[�h�A1:thumb�R�[�h*/
}ELAdrEntry;


/*------------------------------------------------------
  �������̍Ĕz�u���e�[�u��

  �ォ��A�h���X�e�[�u�����Q�Ƃ���Ύ��̂悤�ɉ�������B
  S_ = AdrEntry.adr;
  T_ = (u32)(AdrEntry.thumb_flag);
 -----------------------------------------------------*/
typedef struct {
  void* next;					/*���̃G���g��*/
  char*	sym_str;				/*�������̊O���Q�ƃV���{����*/
  u32	r_type;					/*�����P�[�V�����^�C�v�iELF32_R_TYPE( Rela.r_info)�j*/
  u32	S_;						/*�������̊O���Q�ƃV���{���A�h���X*/
  s32	A_;						/*�����ς�*/
  u32	P_;						/*�����ς�*/
  u32	T_;						/*�������̊O���Q�ƃV���{����ARM/Thumb�t���O*/
  u32	sh_type;				/*SHT_REL or SHT_RELA*/
  u32	remove_flag;			/*���������Ƃ��ɃZ�b�g����i�����Ă��ǂ����Ƃ����ʂ���j�t���O*/
  ELAdrEntry*	AdrEnt;			/*�A�h���X�e�[�u������T���o�����G���g���̏ꏊ*/
}ELUnresolvedEntry;



/* ELHandle �� process�l */
#define EL_FAILED			0x00
#define EL_INITIALIZED		0x5A
#define EL_COPIED			0xF0
#define EL_RELOCATED		0xF1



/*---------------------------------------------------------
 ELF�I�u�W�F�N�g�̃T�C�Y�����߂�
 --------------------------------------------------------*/
u32 EL_GetElfSize( const void* buf);

/*------------------------------------------------------
  �_�C�i�~�b�N�����N�V�X�e��������������
 -----------------------------------------------------*/
void EL_Init( void);

/*------------------------------------------------------
  ELHandle�\���̂�����������
 -----------------------------------------------------*/
BOOL EL_InitHandle( ELHandle* ElfHandle);

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���t�@�C������o�b�t�@�ɍĔz�u����
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromFile( ELHandle* ElfHandle, FILE* ObjFile, void* buf);

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u������������o�b�t�@�ɍĔz�u����
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromMem( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf);

/*------------------------------------------------------
  �A�h���X�e�[�u�����g���Ė������̃V���{������������
 -----------------------------------------------------*/
u16 EL_ResolveAllLibrary( void);


/*------------------------------------------------------
  �}�[�L���O���ꂽ�V���{�������J�p�t�@�C���ɍ\���̂Ƃ��ď����o��
 -----------------------------------------------------*/
u16 EL_ExtractStaticSym1( void);
/*------------------------------------------------------
  �}�[�L���O���ꂽ�V���{�������J�p�t�@�C����API�Ƃ��ď����o��
 -----------------------------------------------------*/
u16 EL_ExtractStaticSym2( void);


/*------------------------------------------------------
  �A�h���X�e�[�u������G���g�����폜����
 -----------------------------------------------------*/
BOOL EL_RemoveAdrEntry( ELAdrEntry* AdrEnt);

/*------------------------------------------------------
  �A�h���X�e�[�u���ɃG���g����ǉ�����
 -----------------------------------------------------*/
void EL_AddAdrEntry( ELAdrEntry* AdrEnt);

/*------------------------------------------------------
  �A�h���X�e�[�u������w�蕶����ɊY������G���g����T��
 -----------------------------------------------------*/
ELAdrEntry* EL_GetAdrEntry( char* ent_name);

/*------------------------------------------------------
  �A�h���X�e�[�u������w�蕶����ɊY������A�h���X��Ԃ�
 -----------------------------------------------------*/
void* EL_GetGlobalAdr( char* ent_name);





/*���ɕK�v�����Ȋ֐�*/
//���[�h�ɕK�v�ȃ������̃o�C�g�����Z�o����֐�
//EL_FreeLibrary

#endif	/*_ELF_LOADER_H_*/



