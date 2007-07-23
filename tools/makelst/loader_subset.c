/*---------------------------------------------------------------------------*
  Project:  TWL - ELF Loader
  File:     loader_subset.c

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "loader_subset.h"


extern ELUnresolvedEntry*    ELUnrEntStart;

extern u16        dbg_print_flag;
extern u32        unresolved_table_block_flag;


//ARM7���ǂ����𔻕ʂ���Ƃ��́A
//#ifdef  SDK_ARM7
//#endif    /*SDK_ARM7*/


/*------------------------------------------------------
  �A�h���X���A���C�������g����
 -----------------------------------------------------*/
#define ALIGN( addr, align_size ) (((addr)  & ~((align_size) - 1)) + (align_size))

static u32 ELi_ALIGN( u32 addr, u32 align_size);
u32 ELi_ALIGN( u32 addr, u32 align_size)
{
    u32 aligned_addr;
    
    if( (addr % align_size) == 0) {
        aligned_addr = addr;
    }else{
        aligned_addr = (((addr) & ~((align_size) - 1)) + (align_size));
    }
    
    return aligned_addr;
}


/*------------------------------------------------------
  �Z�N�V�������o�b�t�@�ɃR�s�[����
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32         load_start;
    Elf32_Addr    sh_size;

    /*�A���C�������g���Ƃ�*/
//    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), (Shdr->sh_addralign));
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    /*�T�C�Y�ݒ�*/
    sh_size = Shdr->sh_size;

    /*�R�s�[*/
    ElfHandle->ELi_ReadStub( (void*)load_start,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset)+(u32)(Shdr->sh_offset),
                             sh_size);

    /*�o�b�t�@�|�C���^���ړ�*/
    ElfHandle->buf_current = (void*)(load_start + sh_size);

    /*���[�h�����擪�A�h���X��Ԃ�*/
    return (void*)load_start;
}


/*------------------------------------------------------
  �Z�N�V�������o�b�t�@�Ɋm�ہi�R�s�[���Ȃ��j���A
  ���̗̈��0�Ŗ��߂�
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32            load_start;
    Elf32_Addr    sh_size;

    /*�A���C�������g���Ƃ�*/
//    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), (Shdr->sh_addralign));
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    /*�T�C�Y�ݒ�*/
    sh_size = Shdr->sh_size;

    /*�o�b�t�@�|�C���^���ړ�*/
    ElfHandle->buf_current = (void*)(load_start + sh_size);

    /*0�Ŗ��߂�i.bss�Z�N�V������z�肵�Ă��邽�߁j*/
    memset( (void*)load_start, 0, sh_size);
    
    /*�m�ۂ����擪�A�h���X��Ԃ�*/
    return (void*)load_start;
}


/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�����w�b�_���o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void ELi_GetShdr( ELHandle* ElfHandle, u32 index, Elf32_Shdr* Shdr)
{
    u32 offset;
    
    offset = (ElfHandle->CurrentEhdr.e_shoff) + ((u32)(ElfHandle->shentsize) * index);
    
    ElfHandle->ELi_ReadStub( Shdr,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset) + offset,
                             sizeof( Elf32_Shdr));
}

/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�����̃G���g�����o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void ELi_GetSent( ELHandle* ElfHandle, u32 index, void* entry_buf, u32 offset, u32 size)
{
    Elf32_Shdr    Shdr;
    //u32            entry_adr;

    ELi_GetShdr( ElfHandle, index, &Shdr);
    
    ElfHandle->ELi_ReadStub( entry_buf,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset) + (u32)(Shdr.sh_offset) + offset,
                             size);
}


/*------------------------------------------------------
  �w��Z�N�V�����w�b�_�̎w��C���f�b�N�X�̃G���g�����o�b�t�@�Ɏ擾����
    �iRel,Rela,Sym�ȂǃG���g���T�C�Y���Œ�̃Z�N�V�����̂݁j
    Shdr : �w��Z�N�V�����w�b�_
    index : �G���g���ԍ�(0�`)
 -----------------------------------------------------*/
void ELi_GetEntry( ELHandle* ElfHandle, Elf32_Shdr* Shdr, u32 index, void* entry_buf)
{
    u32 offset;

    offset = (u32)(Shdr->sh_offset) + ((Shdr->sh_entsize) * index);
    
    ElfHandle->ELi_ReadStub( entry_buf,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset) + offset,
                             Shdr->sh_entsize);
}

/*------------------------------------------------------
  STR�Z�N�V�����w�b�_�̎w��C���f�b�N�X�̕�������擾����

    Shdr : �w��Z�N�V�����w�b�_
    index : �G���g���C���f�b�N�X(0�`)
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELHandle* ElfHandle, u32 strsh_index, u32 ent_index, char* str, u32 len)
{
    /*������G���g���̐擪�A�h���X*/
    ELi_GetSent( ElfHandle, strsh_index, str, ent_index, len);
}

/*------------------------------------------------------
  �V���{�����Ē�`����

    <Rel�Z�N�V�����w�b�_>
    RelShdr->sh_link : �V���{���Z�N�V�����ԍ�
    RelShdr->sh_info : �^�[�Q�b�g�Z�N�V�����ԍ�(��Frel.text��.text��\��)
    
    <Rel�G���g��>
    Rel->r_offset : �^�[�Q�b�g�Z�N�V�������̃I�t�Z�b�g�A�h���X
    ELF32_R_SYM( Rel->r_info) : �V���{���G���g���ԍ�
    ELF32_R_TYPE( Rel->r_info) : �����P�[�g�^�C�v

    <Sym�Z�N�V�����w�b�_>
    SymShdr->sh_link : �V���{���̕�����e�[�u���Z�N�V�����ԍ�
    
    <Sym�G���g��>
    Sym->st_name : �V���{���̕�����G���g���ԍ�
    Sym->st_value : �V���{���̏�������Z�N�V�������̃I�t�Z�b�g�A�h���X
    Sym->st_size : �V���{���̏�������Z�N�V�������ł̃T�C�Y
    Sym->st_shndx : �V���{���̏�������Z�N�V�����̔ԍ�
    ELF32_ST_BIND( Sym->st_info) : �o�C���h(LOCAL or GLOBAL)
    ELF32_ST_TYPE( Sym->st_info) : �^�C�v(�V���{�����֘A�t�����Ă���Ώ�)
 -----------------------------------------------------*/
void ELi_RelocateSym( ELHandle* ElfHandle, u32 relsh_index)
{
    u32                    i;
    u32                    num_of_sym;        //�V���{���̑S�̐�
    u32                    num_of_rel;        //�Ē�`���ׂ��V���{���̐�
    Elf32_Shdr             RelOrRelaShdr;    //REL�܂���RELA�Z�N�V�����w�b�_
    Elf32_Rela            CurrentRela;    //REL�܂���RELA�G���g���̃R�s�[��
    Elf32_Shdr*         SymShdr;
    ELSymEx*            CurrentSymEx;
    ELSymEx*            FwdSymEx;
    ELSymEx                DmySymEx;
    ELShdrEx*            TargetShdrEx;
    ELShdrEx*            CurrentShdrEx;
    u32                    relocation_adr;
    char                sym_str[128];
    u32                    copy_size;
    ELAdrEntry*            CurrentAdrEntry;
    u32                    sym_loaded_adr;
    ELAdrEntry*            ExportAdrEntry;
    u32                    thumb_func_flag;
    ELUnresolvedEntry    UnresolvedInfo;
    ELUnresolvedEntry*    UnrEnt;
    u32                    unresolved_num = 0;

    /*REL or RELA�Z�N�V�����w�b�_�擾*/
    ELi_GetShdr( ElfHandle, relsh_index, &RelOrRelaShdr);

    /*REL�Z�N�V������SYM�Z�N�V������1��1�Ή�*/
    ELi_GetShdr( ElfHandle, RelOrRelaShdr.sh_link, &(ElfHandle->SymShdr));
    SymShdr = &(ElfHandle->SymShdr);
    if( dbg_print_flag == 1) {
        printf( "SymShdr->link:%02x, SymShdr->info:%02x\n",
                (int)(SymShdr->sh_link), (int)(SymShdr->sh_info));
    }

    /*�^�[�Q�b�g�Z�N�V������EX�w�b�_*/
    TargetShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, RelOrRelaShdr.sh_info);
    
    num_of_rel = (RelOrRelaShdr.sh_size) / (RelOrRelaShdr.sh_entsize);    //�Ē�`���ׂ��V���{���̐�
    num_of_sym = (SymShdr->sh_size) / (SymShdr->sh_entsize);    //�V���{���̑S�̐�

    /*---------- ELSymEx�̃��X�g����� ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx->next = (void*)(malloc( sizeof(ELSymEx)));
        CurrentSymEx = (ELSymEx*)(CurrentSymEx->next);
        
        /*�V���{���G���g�����R�s�[*/
        ELi_GetEntry( ElfHandle, SymShdr, i, &(CurrentSymEx->Sym));
        
        /*�f�o�b�O���t���O���Z�b�g*/
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
        if( CurrentShdrEx) {
            CurrentSymEx->debug_flag = CurrentShdrEx->debug_flag;
        }else{
            CurrentSymEx->debug_flag = 0;
        }

//        printf( "sym_no: %02x ... st_shndx: %04x\n", i, CurrentSymEx->Sym.st_shndx);
    }
    CurrentSymEx->next = NULL;
    ElfHandle->SymEx = DmySymEx.next;
    /*-------------------------------------------*/

    /*----- ELSymEx��Thumb�t���O���Z�b�g�i�֐��V���{�������K�v�j-----*/
    CurrentSymEx = ElfHandle->SymEx;
    for( i=0; i<num_of_sym; i++) {
        if( ELF32_ST_TYPE( CurrentSymEx->Sym.st_info) == STT_FUNC) {
            CurrentSymEx->thumb_flag = (u16)(ELi_CodeIsThumb( ElfHandle, CurrentSymEx->Sym.st_shndx,
                                                           CurrentSymEx->Sym.st_value));
        }else{
            CurrentSymEx->thumb_flag = 0;
        }
        CurrentSymEx = CurrentSymEx->next;
    }
    /*---------------------------------------------------------------*/

    /*--- �Ē�`���K�v�ȃV���{���̍Ē�` ---*/
    for( i=0; i<num_of_rel; i++) {
        
        /*- Rel�܂���Rela�G���g���擾 -*/
        ELi_GetEntry( ElfHandle, &RelOrRelaShdr, i, &CurrentRela);
        
        if( RelOrRelaShdr.sh_type == SHT_REL) {
            CurrentRela.r_addend = 0;
        }
        /*-----------------------------*/

        /*�V���{��Ex�G���g���擾*/
        CurrentSymEx = ELi_GetSymExfromList( ElfHandle->SymEx,
                                             ELF32_R_SYM( CurrentRela.r_info));

        if( CurrentSymEx->debug_flag == 1) {            /*�f�o�b�O���̏ꍇ*/
        }else{                                            /*�f�o�b�O���łȂ��ꍇ*/
            /**/
            ELi_UnresolvedInfoInit( &UnresolvedInfo);
            /*����������A�h���X�i�d�l���ł����uP�v�j*/
            relocation_adr = (TargetShdrEx->loaded_adr) + (CurrentRela.r_offset);
            UnresolvedInfo.r_type = ELF32_R_TYPE( CurrentRela.r_info);
            UnresolvedInfo.A_ = (CurrentRela.r_addend);
            UnresolvedInfo.P_ = (relocation_adr);
            UnresolvedInfo.sh_type = (RelOrRelaShdr.sh_type);
            
            /*�V���{���̃A�h���X��˂��~�߂�*/
            if( CurrentSymEx->Sym.st_shndx == SHN_UNDEF) {
                /*�A�h���X�e�[�u�����猟��*/
                ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
                CurrentAdrEntry = EL_GetAdrEntry( sym_str);
                if( CurrentAdrEntry) {
                    sym_loaded_adr = (u32)(CurrentAdrEntry->adr);
                    /*THUMB�֐��t���O�i�d�l���ł����uT�v�jTHUMB or ARM �̔���*/
                    thumb_func_flag = CurrentAdrEntry->thumb_flag;
                    if( dbg_print_flag == 1) {
                        printf( "\n symbol found %s : %8x\n", sym_str, (int)(sym_loaded_adr));
                    }
                }else{
                    /*������Ȃ��������̓G���[(S_��T_�������ł��Ȃ�)*/
                    copy_size = (u32)strlen( sym_str) + 1;
                    UnresolvedInfo.sym_str = (char*)(malloc( copy_size));
                    //MI_CpuCopy8( sym_str, UnresolvedInfo.sym_str, copy_size);
                    memcpy( UnresolvedInfo.sym_str, sym_str, copy_size);

                    /*�O���[�o���Ȗ������e�[�u���ɒǉ�*/
                    copy_size = sizeof( ELUnresolvedEntry);
                    UnrEnt = (ELUnresolvedEntry*)(malloc( copy_size));
                    //MI_CpuCopy8( &UnresolvedInfo, UnrEnt, copy_size);
                    memcpy( UnrEnt, &UnresolvedInfo, copy_size);
                    
                    if( unresolved_table_block_flag == 0) {    //�e�[�u���ւ̒ǉ����֎~����Ă��Ȃ����
                        ELi_AddUnresolvedEntry( UnrEnt);
                    }

                    unresolved_num++;    /*�������V���{�������J�E���g*/
                    if( dbg_print_flag == 1) {
                         printf( "WARNING! cannot find symbol : %s\n", sym_str);
                    }
                }
            }else{
                /*�V���{���̏�������Z�N�V������Ex�w�b�_�擾*/
                CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
                sym_loaded_adr = CurrentShdrEx->loaded_adr;
                sym_loaded_adr += CurrentSymEx->Sym.st_value;    //sym_loaded_adr�͎d�l���ł����uS�v
                /*THUMB�֐��t���O�i�d�l���ł����uT�v�jTHUMB or ARM �̔���*/
                thumb_func_flag = CurrentSymEx->thumb_flag;
            }

            if( !UnresolvedInfo.sym_str) {        /*sym_str���Z�b�g����Ă���Ƃ��̓V���{�������s�\*/
                /*�d�l���Ɠ����ϐ����ɂ���*/
                UnresolvedInfo.S_ = (sym_loaded_adr);
                UnresolvedInfo.T_ = (thumb_func_flag);

                /*--------------- �V���{���̉���(�Ĕz�u�̎��s) ---------------*/
//                CurrentSymEx->relocation_val = ELi_DoRelocate( &UnresolvedInfo);
                /*------------------------------------------------------------*/
            }
        }
    }
    /*-----------------------------------*/
    /*--- ���C�u��������GLOBAL�V���{�����A�h���X�e�[�u���Ɍ��J���� ---*/
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx = ELi_GetSymExfromList( ElfHandle->SymEx, i);
        /*GLOBAL�ŁA���֘A����Z�N�V���������C�u�������ɑ��݂���ꍇ*/
        if( ((ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_GLOBAL) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_WEAK) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_MW_SPECIFIC))&&
            (CurrentSymEx->Sym.st_shndx != SHN_UNDEF)) {
            
            ExportAdrEntry = (ELAdrEntry*)(malloc( sizeof(ELAdrEntry)));    /*�������m��*/
            
            ExportAdrEntry->next = NULL;
            
            ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
            copy_size = (u32)strlen( sym_str) + 1;
            ExportAdrEntry->name = (char*)(malloc( copy_size));
            //MI_CpuCopy8( sym_str, ExportAdrEntry->name, copy_size);
            memcpy( ExportAdrEntry->name, sym_str, copy_size);

            CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
            //Sym.st_value�͋���/���ARM/Thumb�𔻕ʂł���悤�ɒ�������Ă���ꍇ������̂ŁA���̒������폜���Đ����̒l���o��
            ExportAdrEntry->adr = (void*)(CurrentShdrEx->loaded_adr + ((CurrentSymEx->Sym.st_value)&0xFFFFFFFE));
            ExportAdrEntry->func_flag = (u16)(ELF32_ST_TYPE( CurrentSymEx->Sym.st_info));
            ExportAdrEntry->thumb_flag = CurrentSymEx->thumb_flag;
            
            if( EL_GetAdrEntry( ExportAdrEntry->name) == NULL) {    //�����ĂȂ�������
                if( dbg_print_flag == 1) {
                    printf( "Add Entry : %s(0x%x), func=%d, thumb=%d\n",
                                ExportAdrEntry->name,
                                (int)(ExportAdrEntry->adr),
                                ExportAdrEntry->func_flag,
                                ExportAdrEntry->thumb_flag);
                }
                EL_AddAdrEntry( ExportAdrEntry);    //�o�^
            }
        }
    }
    /*----------------------------------------------------------------*/

    /*------- ELSymEx�̃��X�g��������� -------*/
    CurrentSymEx = ElfHandle->SymEx;
    if( CurrentSymEx) {
        while( CurrentSymEx->next != NULL) {
            FwdSymEx = CurrentSymEx;
            CurrentSymEx = CurrentSymEx->next;
            free( FwdSymEx);
        }
        ElfHandle->SymEx = NULL;
    }
    /*-----------------------------------------*/

    /* �Ĕz�u������ */
    if( unresolved_num == 0) {
        ElfHandle->process = EL_RELOCATED;
    }
}

/*------------------------------------------------------
    makelst��p�֐�
    �V���{���Z�N�V�����̒�����GLOBAL�Ȃ��̂�
    �A�h���X�e�[�u���ɓo�^����
 -----------------------------------------------------*/
void ELi_DiscriminateGlobalSym( ELHandle* ElfHandle, u32 symsh_index)
{
    u32                    i;
    u32                    num_of_sym;
    Elf32_Shdr            CurrentSymShdr;
    Elf32_Shdr*         SymShdr;        //SYM�Z�N�V�����w�b�_
    ELSymEx*            CurrentSymEx;
    ELSymEx*            FwdSymEx;
    ELSymEx                DmySymEx;
    ELShdrEx*            CurrentShdrEx;
    ELAdrEntry*            ExportAdrEntry;
    char                sym_str[128];
    u32                    copy_size;
    
    /*SYM�Z�N�V�����w�b�_�擾*/
    ELi_GetShdr( ElfHandle, symsh_index, &CurrentSymShdr);
    SymShdr = &CurrentSymShdr;

    num_of_sym = (SymShdr->sh_size) / (SymShdr->sh_entsize);    //�V���{���̑S�̐�

    /*---------- ELSymEx�̃��X�g����� ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx->next = (void*)(malloc( sizeof(ELSymEx)));
        CurrentSymEx = (ELSymEx*)(CurrentSymEx->next);
        
        /*�V���{���G���g�����R�s�[*/
        ELi_GetEntry( ElfHandle, SymShdr, i, &(CurrentSymEx->Sym));
        
        /*�f�o�b�O���t���O���Z�b�g*/
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
        if( CurrentShdrEx) {
            CurrentSymEx->debug_flag = CurrentShdrEx->debug_flag;
        }else{
            CurrentSymEx->debug_flag = 0;
        }

//        printf( "sym_no: %02x ... st_shndx: %04x\n", i, CurrentSymEx->Sym.st_shndx);
    }
    CurrentSymEx->next = NULL;
    ElfHandle->SymEx = DmySymEx.next;
    /*-------------------------------------------*/

    /*----- ELSymEx��Thumb�t���O���Z�b�g�i�֐��V���{�������K�v�j-----*/
    CurrentSymEx = ElfHandle->SymEx;
    for( i=0; i<num_of_sym; i++) {
        if( ELF32_ST_TYPE( CurrentSymEx->Sym.st_info) == STT_FUNC) {
            CurrentSymEx->thumb_flag = (u16)(ELi_CodeIsThumb( ElfHandle, CurrentSymEx->Sym.st_shndx,
                                                           CurrentSymEx->Sym.st_value));
        }else{
            CurrentSymEx->thumb_flag = 0;
        }
        CurrentSymEx = CurrentSymEx->next;
    }
    /*---------------------------------------------------------------*/
    /*--- ���C�u��������GLOBAL�V���{�����A�h���X�e�[�u���Ɍ��J���� ---*/
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx = ELi_GetSymExfromList( ElfHandle->SymEx, i);
        /*GLOBAL�ŁA���֘A����Z�N�V���������C�u�������ɑ��݂���ꍇ*/
        if( ((ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_GLOBAL) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_WEAK) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_MW_SPECIFIC))&&
            (CurrentSymEx->Sym.st_shndx != SHN_UNDEF)) {
            
            ExportAdrEntry = (ELAdrEntry*)(malloc( sizeof(ELAdrEntry)));    /*�������m��*/
            
            ExportAdrEntry->next = NULL;
            
            ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
            copy_size = (u32)strlen( sym_str) + 1;
            ExportAdrEntry->name = (char*)(malloc( copy_size));
            //MI_CpuCopy8( sym_str, ExportAdrEntry->name, copy_size);
            memcpy( ExportAdrEntry->name, sym_str, copy_size);

            if( (CurrentSymEx->Sym.st_shndx) < SHN_LORESERVE) { //�֘A�Z�N�V����������ꍇ
                if( (CurrentSymEx->Sym.st_shndx == SHN_ABS)) {
                    //Sym.st_value�͋���/���ARM/Thumb�𔻕ʂł���悤�ɒ�������Ă���ꍇ������̂ŁA���̒������폜���Đ����̒l���o��
                    ExportAdrEntry->adr = (void*)((CurrentSymEx->Sym.st_value)&0xFFFFFFFE);
                }else{
                    CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
                    //Sym.st_value�͋���/���ARM/Thumb�𔻕ʂł���悤�ɒ�������Ă���ꍇ������̂ŁA���̒������폜���Đ����̒l���o��
                    ExportAdrEntry->adr = (void*)(CurrentShdrEx->loaded_adr + ((CurrentSymEx->Sym.st_value)&0xFFFFFFFE));
                }
            ExportAdrEntry->func_flag = (u16)(ELF32_ST_TYPE( CurrentSymEx->Sym.st_info));
            ExportAdrEntry->thumb_flag = CurrentSymEx->thumb_flag;

            if( EL_GetAdrEntry( ExportAdrEntry->name) == NULL) {    //�����ĂȂ�������
                if( dbg_print_flag == 1) {
                    printf( "Add Entry : %s(0x%x), func=%d, thumb=%d\n",
                                ExportAdrEntry->name,
                                (int)(ExportAdrEntry->adr),
                                ExportAdrEntry->func_flag,
                                ExportAdrEntry->thumb_flag);
                }            
                EL_AddAdrEntry( ExportAdrEntry);    //�o�^
            }
            }
        }
    }
    /*----------------------------------------------------------------*/

    /*------- ELSymEx�̃��X�g��������� -------*/
    CurrentSymEx = ElfHandle->SymEx;
    if( CurrentSymEx) {
        while( CurrentSymEx->next != NULL) {
            FwdSymEx = CurrentSymEx;
            CurrentSymEx = CurrentSymEx->next;
            free( FwdSymEx);
        }
        ElfHandle->SymEx = NULL;
    }
    /*-----------------------------------------*/
}


/*------------------------------------------------------
  �������������ƂɃV���{������������

    r_type,S_,A_,P_,T_���S�ĕ������Ă���K�v������B
 -----------------------------------------------------*/
#define _S_    (UnresolvedInfo->S_)
#define _A_    (UnresolvedInfo->A_)
#define _P_    (UnresolvedInfo->P_)
#define _T_    (UnresolvedInfo->T_)
u32    ELi_DoRelocate( ELUnresolvedEntry* UnresolvedInfo)
{
    s32 signed_val;
    u32 relocation_val = 0;
    u32 relocation_adr;

    relocation_adr = _P_;

    switch( (UnresolvedInfo->r_type)) {
      case R_ARM_PC24:
      case R_ARM_PLT32:
      case R_ARM_CALL:
      case R_ARM_JUMP24:
        if( UnresolvedInfo->sh_type == SHT_REL) {
            _A_ = (((*(vu32*)relocation_adr)|0xFF800000) << 2);    //��ʓI�ɂ�-8�ɂȂ��Ă���͂�
        }
        signed_val = (( (s32)(_S_) + _A_) | (s32)(_T_)) - (s32)(_P_);
        if( _T_) {        /*BLX���߂�ARM����Thumb�ɔ��(v5��������BL���x�j�A��BX�Ƃ����d�g�݂��K�v)*/
            relocation_val = (0xFA000000) | ((signed_val>>2) & 0x00FFFFFF) | (((signed_val>>1) & 0x1)<<24);
        }else{            /*BL���߂�ARM����ARM�ɔ��*/
            signed_val >>= 2;
            relocation_val = (*(vu32*)relocation_adr & 0xFF000000) | (signed_val & 0x00FFFFFF);
        }
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_ABS32:
        relocation_val = (( _S_ + _A_) | _T_);
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_REL32:
        relocation_val = (( _S_ + _A_) | _T_) - _P_;
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_LDR_PC_G0:
        signed_val = ( (s32)(_S_) + _A_) - (s32)(_P_);
        signed_val >>= 2;
        relocation_val = (*(vu32*)relocation_adr & 0xFF000000) | (signed_val & 0x00FFFFFF);
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_ABS16:
      case R_ARM_ABS12:
      case R_ARM_THM_ABS5:
      case R_ARM_ABS8:
        relocation_val = _S_ + _A_;
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_THM_PC22:/*�ʖ��FR_ARM_THM_CALL*/
        if( UnresolvedInfo->sh_type == SHT_REL) {
            _A_ = (((*(vu16*)relocation_adr & 0x07FF)<<11) + ((*((vu16*)(relocation_adr)+1)) & 0x07FF));
            _A_ = (_A_ | 0xFFC00000) << 1;    //��ʓI�ɂ�-4�ɂȂ��Ă���͂�(PC�͌����߃A�h���X+4�Ȃ̂�)
        }
        signed_val = (( (s32)(_S_) + _A_) | (s32)(_T_)) - (s32)(_P_);
        signed_val >>= 1;
        if( _T_) {    /*BL���߂�Thumb����Thumb�ɔ��*/
            relocation_val = ((*(vu16*)relocation_adr & 0xF800) | ((signed_val>>11) & 0x07FF)) +
                                   ((((*((vu16*)(relocation_adr)+1)) & 0xF800) | (signed_val & 0x07FF)) << 16);
        }else{        /*BLX���߂�Thumb����ARM�ɔ��(v5��������BL���x�j�A��BX�Ƃ����d�g�݂��K�v)*/
            if( (signed_val & 0x1)) {    //_P_��4�o�C�g�A���C������Ă��Ȃ��Ƃ����ɗ���
                signed_val += 1;
            }
            relocation_val = ((*(vu16*)relocation_adr & 0xF800) | ((signed_val>>11) & 0x07FF)) +
                                   ((((*((vu16*)(relocation_adr)+1)) & 0xE800) | (signed_val & 0x07FF)) << 16);
        }
        *(vu16*)relocation_adr = (vu16)relocation_val;
        *((vu16*)relocation_adr+1) = (vu16)((u32)(relocation_val) >> 16);
        break;
      case R_ARM_THM_JUMP24:
        break;
      default:
        if( dbg_print_flag == 1) {
            printf( "ERROR! : unsupported relocation type!\n");
        }
        break;
    }
    
    return relocation_val;
}
#undef _S_
#undef _A_
#undef _P_
#undef _T_

/*------------------------------------------------------
  ���X�g����w��C���f�b�N�X��ELSymEx�����o��
 -----------------------------------------------------*/
ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index)
{
    u32         i;
    ELSymEx*    SymEx;

    SymEx = SymExStart;
    for( i=0; i<index; i++) {
        SymEx = (ELSymEx*)(SymEx->next);
        if( SymEx == NULL) {
            break;
        }
    }
    return SymEx;
}

/*------------------------------------------------------
  ���X�g����w��C���f�b�N�X��ELShdrEx�����o��
 -----------------------------------------------------*/
ELShdrEx* ELi_GetShdrExfromList( ELShdrEx* ShdrExStart, u32 index)
{
    u32         i;
    ELShdrEx*    ShdrEx;

    ShdrEx = ShdrExStart;
    for( i=0; i<index; i++) {
        ShdrEx = (ELShdrEx*)(ShdrEx->next);
        if( ShdrEx == NULL) {
            break;
        }
    }
    return ShdrEx;
}



/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�������f�o�b�O��񂩂ǂ������肷��

���f�o�b�O���̒�`��
�E�Z�N�V��������".debug"����n�܂�Z�N�V����

�E.rel.debug�` �Z�N�V�����ȂǁAsh_info ���f�o�b�O���Z�N�V�����ԍ���
�@�����Ă���Z�N�V����
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELHandle* ElfHandle, u32 index)
{
    Elf32_Shdr    TmpShdr;
    char        shstr[6];

    /*-- �Z�N�V�������̕������6�����擾 --*/
    ELi_GetShdr( ElfHandle, index, &TmpShdr);
    ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
                   TmpShdr.sh_name, shstr, 6);
    /*-------------------------------------*/
    
    if( strncmp( shstr, ".debug", 6) == 0) {    /*�f�o�b�O�Z�N�V�����̏ꍇ*/
        return TRUE;
    }else{                        /*�f�o�b�O�Z�N�V�����Ɋւ���Ĕz�u�Z�N�V�����̏ꍇ*/
        if( (TmpShdr.sh_type == SHT_REL) || (TmpShdr.sh_type == SHT_RELA)) {
            if( ELi_ShdrIsDebug( ElfHandle, TmpShdr.sh_info) == TRUE) {
                return TRUE;
            }
        }
    }

    return FALSE;
}



/*------------------------------------------------------
  ElfHandle��SymEx�e�[�u���𒲂ׁA�w��C���f�b�N�X��
�@�w��I�t�Z�b�g�ɂ���R�[�h��ARM��THUMB���𔻒肷��B
  �i�\�� ElfHandle->SymShdr �� ElfHandle->SymEx ��ݒ肵�Ă������Ɓj
    
    sh_index : ���ׂ����Z�N�V�����C���f�b�N�X
    offset : ���ׂ����Z�N�V�������̃I�t�Z�b�g
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELHandle* ElfHandle, u16 sh_index, u32 offset)
{
    u32            i;
    u32            thumb_flag;
    Elf32_Shdr*    SymShdr;
    char        str_adr[3];
    ELSymEx*    CurrentSymEx;

    /*�V���{���̃Z�N�V�����w�b�_��SymEx���X�g�擾*/
    SymShdr = &(ElfHandle->SymShdr);
    CurrentSymEx = ElfHandle->SymEx;

    i = 0;
    thumb_flag = 0;
    while( CurrentSymEx != NULL) {
        
        if( CurrentSymEx->Sym.st_shndx == sh_index) {
            ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, str_adr, 3);
            if( strncmp( str_adr, "$a\0", strlen("$a\0")) == 0) {
                thumb_flag = 0;
            }else if( strncmp( str_adr, "$t\0", strlen("$t\0")) == 0) {
                thumb_flag = 1;
            }
            if( CurrentSymEx->Sym.st_value > offset) {
                break;
            }
        }
        
        CurrentSymEx = CurrentSymEx->next;
        i++;
    }

    return thumb_flag;
}


/*---------------------------------------------------------
 ���������G���g��������������
 --------------------------------------------------------*/
void ELi_UnresolvedInfoInit( ELUnresolvedEntry* UnresolvedInfo)
{
    UnresolvedInfo->sym_str = NULL;
    UnresolvedInfo->r_type = 0;
    UnresolvedInfo->S_ = 0;
    UnresolvedInfo->A_ = 0;
    UnresolvedInfo->P_ = 0;
    UnresolvedInfo->T_ = 0;
    UnresolvedInfo->remove_flag = 0;
}

/*------------------------------------------------------
  ���������e�[�u������G���g�����폜����
 -----------------------------------------------------*/
BOOL ELi_RemoveUnresolvedEntry( ELUnresolvedEntry* UnrEnt)
{
    ELUnresolvedEntry    DmyUnrEnt;
    ELUnresolvedEntry*    CurrentUnrEnt;

    if( UnrEnt == NULL) {
        return FALSE;
    }
    
    DmyUnrEnt.next = ELUnrEntStart;
    CurrentUnrEnt = &DmyUnrEnt;

    while( CurrentUnrEnt->next != UnrEnt) {
        if( CurrentUnrEnt->next == NULL) {
            return FALSE;
        }else{
            CurrentUnrEnt = (ELUnresolvedEntry*)(CurrentUnrEnt->next);
        }
    }

    CurrentUnrEnt->next = UnrEnt->next;
    free( UnrEnt->sym_str);
    free( UnrEnt);
    ELUnrEntStart = DmyUnrEnt.next;
    
    return TRUE;
}

/*---------------------------------------------------------
 ���������e�[�u���ɃG���g����ǉ�����
 --------------------------------------------------------*/
void ELi_AddUnresolvedEntry( ELUnresolvedEntry* UnrEnt)
{
    ELUnresolvedEntry    DmyUnrEnt;
    ELUnresolvedEntry*    CurrentUnrEnt;

    if( !ELUnrEntStart) {
        ELUnrEntStart = UnrEnt;
    }else{
        DmyUnrEnt.next = ELUnrEntStart;
        CurrentUnrEnt = &DmyUnrEnt;

        while( CurrentUnrEnt->next != NULL) {
            CurrentUnrEnt = CurrentUnrEnt->next;
        }
        CurrentUnrEnt->next = (void*)UnrEnt;
    }
    UnrEnt->next = NULL;
}

/*------------------------------------------------------
  ���������e�[�u������w�蕶����ɊY������G���g����T��
 -----------------------------------------------------*/
ELUnresolvedEntry* ELi_GetUnresolvedEntry( char* ent_name)
{
    ELUnresolvedEntry* CurrentUnrEnt;

    CurrentUnrEnt = ELUnrEntStart;
    if( CurrentUnrEnt == NULL) {
        return NULL;
    }
    while( 1) {
        if( (strcmp( CurrentUnrEnt->sym_str, ent_name) == 0)&&
            (CurrentUnrEnt->remove_flag == 0)) {
            break;
        }
        CurrentUnrEnt = (ELUnresolvedEntry*)CurrentUnrEnt->next;
        if( CurrentUnrEnt == NULL) {
            break;
        }
    }
    return CurrentUnrEnt;
}
