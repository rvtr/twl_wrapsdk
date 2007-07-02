/*---------------------------------------------------------------------------*
  Project:  CTR - ELF Loader
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

#include "el_config.h"
#if (TARGET_OS_NITRO == 1)
#include <nitro/fs.h>
#else
#include <ctr.h>
#include <stdio.h>
#endif
#include "elf.h"

#ifdef __cplusplus
extern "C" {
#endif


/*------------------------------------------------------
  �Z�N�V�����w�b�_�g��(���[�h�A�h���X���ɑΉ�)
 -----------------------------------------------------*/
typedef struct {
  void*         next;
  u16           index;
  u16           debug_flag;    /*0:�f�o�b�O���łȂ��A1:�f�o�b�O���*/
  u32           loaded_adr;
  u32           alloc_adr;
  u32           loaded_size;
  Elf32_Shdr    Shdr;
}ElShdrEx;


/*------------------------------------------------------
  �V���{���G���g���g��(���[�h�A�h���X���ɑΉ�)
 -----------------------------------------------------*/
typedef struct {
  void*      next;
  u16        debug_flag;       /*0:�f�o�b�O���łȂ��A1:�f�o�b�O���*/
  u16        thumb_flag;
  u32        relocation_val;
  Elf32_Sym  Sym;
}ElSymEx;


/*------------------------------------------------------
  �x�j���̃����N���X�g(i_elDoRelocate�Ŏg�p)
 -----------------------------------------------------*/
typedef struct {
  void* next;
  u32   adr;     /* �x�j���R�[�h�̐擪�A�h���X */
  u32   data;    /* �x�j���̃��e�����v�[���Ɋi�[����Ă����ѐ�̃A�h���X�l */
}ElVeneer;


/*------------------------------------------------------
  ELF�I�u�W�F�N�g�̊Ǘ�
 -----------------------------------------------------*/
typedef void (*i_elReadFunc)( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
typedef struct {
  void*         ar_head;        /* AR�܂���ELF�t�@�C���̐擪�A�h���X */
  void*         elf_offset;     /* ELF�I�u�W�F�N�g�̐擪�ւ̃I�t�Z�b�g */
  void*         buf_current;    /* Loader��Ɨp */
  u16           shentsize;      /* 1�Z�N�V�����w�b�_�̃T�C�Y */
  u16           process;        /* �Ĕz�u�� */

  ElShdrEx*     ShdrEx;         /* ShdrEx���X�g�̐擪 */

  Elf32_Ehdr    CurrentEhdr;    /* ELF�w�b�_ */

  Elf32_Rel     Rel;            /* �Ĕz�u�G���g�� */
  Elf32_Rela    Rela;
  Elf32_Sym     Sym;            /* �V���{���G���g�� */
  Elf32_Shdr    SymShdr;

  ElSymEx*      SymEx;          /* SymEx���X�g�̐擪�i��f�o�b�O�V���{���̂݌q����j */
  ElSymEx**     SymExTbl;       /* SymEx�A�h���X�̃e�[�u���i�S�V���{�����Ԃ�j*/
  u32           SymExTarget;    /* SymEx���X�g���\�z�����V���{���Z�N�V�����̃Z�N�V�����ԍ� */

  ElVeneer*     VeneerEx;       /* �x�j���̃����N���X�g */
  
  i_elReadFunc  i_elReadStub;   /* ���[�h�X�^�u�֐� */
  void*         FileStruct;     /* �t�@�C���\���� */
    
  u32           entry_adr;      /* �G���g���A�h���X */
}ElDesc;



/*------------------------------------------------------
  �A�h���X�e�[�u��
 -----------------------------------------------------*/
typedef struct {
  void*      next;              /*���̃A�h���X�G���g��*/
  char*      name;              /*������*/
  void*      adr;               /*�A�h���X*/
  u16        func_flag;         /*0:�f�[�^�A1:�֐�*/
  u16        thumb_flag;        /*0:arm�R�[�h�A1:thumb�R�[�h*/
}ElAdrEntry;


/*------------------------------------------------------
  �������̍Ĕz�u���e�[�u��

  �ォ��A�h���X�e�[�u�����Q�Ƃ���Ύ��̂悤�ɉ�������B
  S_ = AdrEntry.adr;
  T_ = (u32)(AdrEntry.thumb_flag);
 -----------------------------------------------------*/
typedef struct {
  void*  next;                  /*���̃G���g��*/
  char*  sym_str;               /*�������̊O���Q�ƃV���{����*/
  u32    r_type;                /*�����P�[�V�����^�C�v�iELF32_R_TYPE( Rela.r_info)�j*/
  u32    S_;                    /*�������̊O���Q�ƃV���{���A�h���X*/
  s32    A_;                    /*�����ς�*/
  u32    P_;                    /*�����ς�*/
  u32    T_;                    /*�������̊O���Q�ƃV���{����ARM/Thumb�t���O*/
  u32    sh_type;               /*SHT_REL or SHT_RELA*/
  u32    remove_flag;           /*���������Ƃ��ɃZ�b�g����i�����Ă��ǂ����Ƃ����ʂ���j�t���O*/
}ElUnresolvedEntry;



/* ElDesc �� process�l */
#define EL_FAILED           0x00
#define EL_INITIALIZED      0x5A
#define EL_COPIED           0xF0
#define EL_RELOCATED        0xF1



/* typedef */
//#if (TARGET_OS_NITRO == 1)
//#else
typedef void *(*ElAlloc)(size_t size);
typedef void (*ElFree)(void* ptr);
typedef u32 (*ElReadImage)( u32 offset, void* buf, u32 size);
//#endif


/*---------------------------------------------------------
 ELF�I�u�W�F�N�g�̃T�C�Y�����߂�
 --------------------------------------------------------*/
u32 elGetElfSize( const void* buf);

/*------------------------------------------------------
  �_�C�i�~�b�N�����N�V�X�e��������������
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
void elInit( void);
#else
void elInit( ElAlloc alloc, ElFree free);
#endif

/*------------------------------------------------------
  ElDesc�\���̂�����������
 -----------------------------------------------------*/
BOOL elInitDesc( ElDesc* elElfDesc);

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���t�@�C������o�b�t�@�ɍĔz�u����
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
u16 elLoadLibraryfromFile( ElDesc* elElfDesc, FSFile* ObjFile, void* buf);
#else
u16 elLoadLibraryfromFile( ElDesc* elElfDesc, int ObjFile, void* buf);
#endif

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u�����[�U�̃��[�hAPI��ʂ��čĔz�u����
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
#else
u16 elLoadLibrary( ElDesc* elElfDesc, ElReadImage readfunc, u32 len, void* buf);
#endif

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u������������o�b�t�@�ɍĔz�u����
 -----------------------------------------------------*/
u16 elLoadLibraryfromMem( ElDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf);

/*------------------------------------------------------
  �A�h���X�e�[�u�����g���Ė������̃V���{������������
 -----------------------------------------------------*/
u16 elResolveAllLibrary( ElDesc* elElfDesc);


/*------------------------------------------------------
  �A�h���X�e�[�u������G���g�����폜����
 -----------------------------------------------------*/
BOOL elRemoveAdrEntry( ElAdrEntry* AdrEnt);

/*------------------------------------------------------
  �A�h���X�e�[�u���ɃG���g����ǉ�����
 -----------------------------------------------------*/
void elAddAdrEntry( ElAdrEntry* AdrEnt);

/*------------------------------------------------------
  �A�h���X�e�[�u���ɃX�^�e�B�b�N���̃G���g����ǉ�����
  �iEL���C�u��������WEAK�V���{���Ƃ��Ē�`����Ă���A
    makelst�����������t�@�C���̒�`�ŏ㏑�������j
 -----------------------------------------------------*/
void elAddStaticSym( void);


/*------------------------------------------------------
  �A�h���X�e�[�u������w�蕶����ɊY������G���g����T��
 -----------------------------------------------------*/
ElAdrEntry* elGetAdrEntry( const char* ent_name);

/*------------------------------------------------------
  �A�h���X�e�[�u������w�蕶����ɊY������A�h���X��Ԃ�
 -----------------------------------------------------*/
void* elGetGlobalAdr( const char* ent_name);



/*���ɕK�v�����Ȋ֐�*/
//���[�h�ɕK�v�ȃ������̃o�C�g�����Z�o����֐�
//elFreeLibrary


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif    /*_ELF_LOADER_H_*/
