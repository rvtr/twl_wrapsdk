/*---------------------------------------------------------------------------*
  Project:  TWL - ELF Loader
  File:     elf_loader.c

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include "types.h"
#include "elf.h"
#include "elf_loader.h"
#include "arch.h"
#include "loader_subset.h"
#include <string.h>
#include <stdlib.h>


//OSHeapHandle        EL_Heap;
ELAdrEntry*           ELAdrEntStart = NULL;
ELUnresolvedEntry*    ELUnrEntStart = NULL;

extern u16 dbg_print_flag;
extern u32 unresolved_table_block_flag;
extern u32 ELi_ALIGN( u32 addr, u32 align_size);

#define MAKELST_DS_API        "    elAddAdrEntry"    //DS��ŃA�h���X�e�[�u���ɒǉ�����API�֐���

extern char    c_source_line_str[256];
extern FILE*   NewElfFilep;


/*------------------------------------------------------
  ���[�J���֐��̐錾
 -----------------------------------------------------*/
// ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���o�b�t�@�ɍĔz�u����
u16 ELi_LoadLibrary( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf);
// ELF�I�u�W�F�N�g���o�b�t�@�ɍĔz�u����R�A�֐�
u16 ELi_LoadObject( ELHandle* ElfHandle, void* obj_offset, void* buf);
// ELF�I�u�W�F�N�g����f�[�^��ǂݏo���X�^�u�֐�
void ELi_ReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
void ELi_ReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);



/*hex�l��10�i�̕�����ŕ\������*/
void ELi_SetDecSize( char* dec, u32 hex)
{
    u16 i;
    u32 tmp = 1000000000;
    u16 ptr = 0;
  
    memset( dec, 0x20, 10);
    for( i=0; i<10; i++) {
        if( ((hex / tmp) != 0) || (ptr != 0)) {
            dec[ptr++] = 0x30 + (hex/tmp);
            hex -= (hex/tmp) * tmp;
        }
        tmp /= 10;
    }
}



/*---------------------------------------------------------
 ELF�I�u�W�F�N�g�̃T�C�Y�����߂�
    
    buf : ELF�C���[�W�̃A�h���X
 --------------------------------------------------------*/
u32 EL_GetElfSize( const void* buf)
{
    Elf32_Ehdr    Ehdr;
    u32           size;
    
    if( ELF_LoadELFHeader( buf, &Ehdr) == NULL) {
        return 0;
    }
    size = (u32)(Ehdr.e_shoff + (Ehdr.e_shentsize * Ehdr.e_shnum));
    return size;
}


/*------------------------------------------------------
  �_�C�i�~�b�N�����N�V�X�e��������������
 -----------------------------------------------------*/
void EL_Init( void)
{
//    void* heap_start;
    printf( "\n");
    /*--- �������A���P�[�V�����֌W�̐ݒ� ---*/
/*    OS_InitArena();
    heap_start = OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo( heap_start );
    EL_Heap = OS_CreateHeap( OS_ARENA_MAIN, heap_start, (void*)((u32)(OS_GetMainArenaHi())+1));
    OS_SetCurrentHeap( OS_ARENA_MAIN, EL_Heap);*/
    /*--------------------------------------*/
}

/*------------------------------------------------------
  ELHandle�\���̂�����������
 -----------------------------------------------------*/
BOOL EL_InitHandle( ELHandle* ElfHandle)
{
    if( ElfHandle == NULL) {    /*NULL�`�F�b�N*/
        return FALSE;
    }

    /*�����l�̐ݒ�*/
    ElfHandle->ShdrEx = NULL;
    ElfHandle->SymEx = NULL;

    ElfHandle->process = EL_INITIALIZED;    /*�t���O�̐ݒ�*/

    return TRUE;
}

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���o�b�t�@�ɍĔz�u����
    
    ElfHandle : �w�b�_�\����
    ObjFile : OBJ�t�@�C���܂��̓A�[�J�C�u�t�@�C���̍\����
    buf : ���[�h����o�b�t�@
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromFile( ELHandle* ElfHandle, FILE* ObjFile, void* buf)
{
    u16 result;
    u32 len;

    /*���[�h�֐��̐ݒ�*/
    ElfHandle->ELi_ReadStub = ELi_ReadFile;
    ElfHandle->FileStruct = ObjFile;

    fseek( ObjFile, 0, SEEK_END);
    len = ftell( ObjFile);
    fseek( ObjFile, 0, SEEK_SET);
    result = ELi_LoadLibrary( ElfHandle, NULL, len, buf);

    return result;
}

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���o�b�t�@�ɍĔz�u����
    
    ElfHandle : �w�b�_�\����
    obj_image : OBJ�t�@�C���܂��̓A�[�J�C�u�t�@�C����RAM��C���[�W�A�h���X
    buf : ���[�h����o�b�t�@
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromMem( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf)
{
    u16 result;
    
    /*���[�h�֐��̐ݒ�*/
    ElfHandle->ELi_ReadStub = ELi_ReadMem;
    ElfHandle->FileStruct = NULL;
    
    result = ELi_LoadLibrary( ElfHandle, obj_image, obj_len, buf);

    return result;
}

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���o�b�t�@�ɍĔz�u����
    
    ElfHandle : �w�b�_�\����
    obj_image : OBJ�t�@�C���܂��̓A�[�J�C�u�t�@�C����RAM��C���[�W�A�h���X
    buf : ���[�h����o�b�t�@
 -----------------------------------------------------*/
u16 ELi_LoadLibrary( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf)
{
    u16     result, all_result;
    u32     image_pointer;
    u32     new_image_pointer;
    u32     arch_size;
    u32     elf_num = 0;                /*ELF�I�u�W�F�N�g�̐�*/
    u32     newarch_size = 0;
    ArchHdr ArHdr;
    char    OBJMAG[8];
    char    ELFMAG[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

    all_result = EL_RELOCATED;
    ElfHandle->ar_head = obj_image;
    image_pointer = 0;

    
    ElfHandle->ELi_ReadStub( OBJMAG, ElfHandle->FileStruct, (u32)obj_image, 0, 8);    /*OBJ�̕�������擾*/
    /*--------------- �A�[�J�C�u�t�@�C���̏ꍇ ---------------*/
    if( strncmp( OBJMAG, ARMAG, 8) == 0) {
        arch_size = sizeof( ArchHdr);
        image_pointer += 8;                /*�ŏ��̃G���g����*/


        new_image_pointer = (u32)buf;
        memcpy( buf, OBJMAG, 8); //stripped elf�ɃR�s�[
        new_image_pointer += 8;
        newarch_size += 8;
        

        ElfHandle->buf_current = (void*)new_image_pointer;
        while( image_pointer < obj_len) {
            ElfHandle->ELi_ReadStub( &ArHdr, ElfHandle->FileStruct, (u32)(obj_image), image_pointer, arch_size);
            ElfHandle->ELi_ReadStub( OBJMAG, ElfHandle->FileStruct, (u32)(obj_image), (image_pointer+arch_size), 4);    /*OBJ�̕�������擾*/
            if( strncmp( OBJMAG, ELFMAG, 4) == 0) {
                elf_num++;

                memcpy( (void*)new_image_pointer, (const void*)&ArHdr, arch_size); //ar�w�b�_��stripped elf�ɃR�s�[

                result = ELi_LoadObject( ElfHandle, (void*)(image_pointer+arch_size),
                                         (void*)(new_image_pointer + arch_size));
//                                         (ElfHandle->buf_current + arch_size));

                ELi_SetDecSize( (char*)(((ArchHdr*)new_image_pointer)->ar_size), ElfHandle->newelf_size); //arch�w�b�_�̃T�C�Y�X�V
                new_image_pointer += (ElfHandle->newelf_size + arch_size);
                newarch_size += (ElfHandle->newelf_size + arch_size);
                
                if( result < all_result) {        /*�������ʂ̂Ƃ�����all_result�ɔ��f*/
                    all_result = result;
                }
                /*�����l�̐ݒ�*/
                ElfHandle->ShdrEx = NULL;
                ElfHandle->SymEx = NULL;
                ElfHandle->process = EL_INITIALIZED;    /*�t���O�̐ݒ�*/
            }else{
                memcpy( (void*)new_image_pointer, (const void*)&ArHdr, arch_size); //ar�w�b�_��stripped elf�ɃR�s�[

                /*ar�w�b�_�̎��i�G���g���{�́j��stripped elf�ɃR�s�[*/
                ElfHandle->ELi_ReadStub( (void*)(new_image_pointer+arch_size), ElfHandle->FileStruct,
                                         (u32)(obj_image),
                                         (image_pointer+arch_size), AR_GetEntrySize( &ArHdr));

                new_image_pointer += (AR_GetEntrySize( &ArHdr) + arch_size);
                newarch_size += (AR_GetEntrySize( &ArHdr) + arch_size);
            }
            /*���̃G���g����*/
            image_pointer += arch_size + AR_GetEntrySize( &ArHdr);
        }
        ElfHandle->newelf_size = newarch_size;
    }else{/*--------------- ELF�t�@�C���̏ꍇ ---------------*/
        if( strncmp( OBJMAG, ELFMAG, 4) == 0) {
            elf_num++;
            all_result = ELi_LoadObject( ElfHandle, 0, buf);
        }
    }
    /*-------------------------------------------------------*/

    if( elf_num) {
        return all_result;
    }else{
        return EL_FAILED;
    }
}

/*------------------------------------------------------
  ELF�I�u�W�F�N�g���o�b�t�@�ɍĔz�u����
    
    ElfHandle : �w�b�_�\����
    obj_offset : OBJ�t�@�C����RAM��C���[�W�A�h���X����̃I�t�Z�b�g
    buf : ���[�h����o�b�t�@
 -----------------------------------------------------*/
u16 ELi_LoadObject( ELHandle* ElfHandle, void* obj_offset, void* buf)
{
    u16         i, j;
//    u32         num_of_entry;
    ELShdrEx*   FwdShdrEx;
    ELShdrEx*   CurrentShdrEx;
    ELShdrEx    DmyShdrEx;
//    char        sym_str[128];     //�f�o�b�O�v�����g�p
//    u32         offset;           //�f�o�b�O�v�����g�p
    u32         newelf_shoff = 0; //stripped elf�C���[�W�ւ̏������݃|�C���^
    u32         buf_shdr;
    u32         section_num = 0;
//    u32         newelf_size;
    u32         tmp_buf;
    u32         *shdr_table;      //�Z�N�V�����w�b�_�V���ԍ��Ή��e�[�u��
//    u32         *sym_table;       //�V���{���G���g���V���ԍ��Ή��e�[�u��
    /* ELHandle�̏������`�F�b�N */
    if( ElfHandle->process != EL_INITIALIZED) {
        return EL_FAILED;
    }
    /* �o�b�t�@��NULL�`�F�b�N */
    if( buf == NULL) {
        return EL_FAILED;
    }
    /* ELF�w�b�_�̎擾 */
    ElfHandle->ELi_ReadStub( &(ElfHandle->CurrentEhdr), ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head), (u32)(obj_offset), sizeof( Elf32_Ehdr));

    /* �Z�N�V�����n���h���\�z */
    ElfHandle->elf_offset = obj_offset;
    ElfHandle->buf_current = (void*)((u32)buf + sizeof( Elf32_Ehdr));
    ElfHandle->shentsize = ElfHandle->CurrentEhdr.e_shentsize;

    /*�Z�N�V�����w�b�_�e�[�u���\�z*/
    shdr_table = (u32*)malloc( 4 * ElfHandle->CurrentEhdr.e_shnum);
  
    /*---------- ELShdrEx�̃��X�g��shdr_table����� ----------*/
    CurrentShdrEx = &DmyShdrEx;
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx->next = (void*)(malloc( sizeof(ELShdrEx)));
        CurrentShdrEx = (ELShdrEx*)(CurrentShdrEx->next);
        memset( CurrentShdrEx, 0, sizeof(ELShdrEx));     //�[���N���A
        
        /*�f�o�b�O��񂩂ǂ����𔻕ʂ��ăt���O���Z�b�g*/
        if( ELi_ShdrIsDebug( ElfHandle, i) == TRUE) {    /*�f�o�b�O���̏ꍇ*/
            CurrentShdrEx->debug_flag = 1;
            shdr_table[i] = 0xFFFFFFFF;
        }else{                                           /*�f�o�b�O���łȂ��ꍇ*/
            /*�Z�N�V�����w�b�_���R�s�[*/
            ELi_GetShdr( ElfHandle, i, &(CurrentShdrEx->Shdr));
            CurrentShdrEx->debug_flag = 0;
            shdr_table[i] = section_num;                 /*�Z�N�V�����V���e�[�u���쐬*/
            //printf( "shdr_table[0x%x] = 0x%x\n", i, section_num);
            section_num++;
            /*�Z�N�V������������擾���Ă���*/
            CurrentShdrEx->str = (char*)malloc( 128);    //128�����o�b�t�@�擾
            ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
                           CurrentShdrEx->Shdr.sh_name,
                           CurrentShdrEx->str, 128);
        }
    }
    CurrentShdrEx->next = NULL;
    ElfHandle->ShdrEx = DmyShdrEx.next;
    /*--------------------------------------------------------*/

   
    /*---------- �S�Z�N�V�����𒲂ׂăR�s�[���� ----------*/
//    printf( "\nLoad to RAM:\n");
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);

        if( CurrentShdrEx->debug_flag == 1) {                 //�f�o�b�O���̏ꍇ
//            printf( "skip debug-section %02x\n", i);
        }else{                                                //�f�o�b�O���łȂ��ꍇ
            // .text section
            if( /*(CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))&&*/
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //�������ɃR�s�[
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }
            // .data, .data1 section (�������ς݃f�[�^)
/*            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //�������ɃR�s�[
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }*/
            // .bss section
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_NOBITS)) {
                //�R�s�[���Ȃ�
                CurrentShdrEx->loaded_adr = ELi_ALIGN( (u32)(ElfHandle->buf_current), 4);
/*                CurrentShdrEx->loaded_adr = (u32)
                                ELi_AllocSectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));*/
            }
            // .rodata, .rodata1 section
/*            else if( (CurrentShdrEx->Shdr.sh_flags == SHF_ALLOC)&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //�������ɃR�s�[
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
            }*/
            //�V���{���e�[�u���Z�N�V����
            else if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                ELi_BuildSymList( ElfHandle, i, &(CurrentShdrEx->sym_table)); //�V���{�����X�g�쐬
                {
                    ELSymEx* CurrentSymEx;
                    ELShdrEx* StrShEx;    //������Z�N�V����
                    char     symstr[128];
                    u32      symbol_num = 0;

                    /*--- �V���{���G���g���̃f�[�^���C�� ---*/
                    CurrentSymEx = ElfHandle->SymEx;
                    while( CurrentSymEx != NULL) {  //SymEx�����ǂ�΃f�o�b�O�V���{���͊܂܂�Ȃ�
                        /*������Z�N�V�����擾*/
                        StrShEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx,
                                                         CurrentShdrEx->Shdr.sh_link);
                        /*--- ������Z�N�V�����̒��g�\�z ---*/
                        ELi_GetStrAdr( ElfHandle, CurrentShdrEx->Shdr.sh_link, //�V���{��������擾
                                       CurrentSymEx->Sym.st_name,
                                       symstr, 128);

                        StrShEx->str_table = realloc( StrShEx->str_table,     //�V���{��������ǉ�
                                                      (StrShEx->str_table_size) +
                                                      strlen( symstr) + 1);
                        strcpy( (u8*)((u32)StrShEx->str_table + StrShEx->str_table_size),
                                symstr);

                        CurrentSymEx->Sym.st_name = StrShEx->str_table_size; //�V���{���G���g���̃f�[�^���C��

                        StrShEx->str_table_size += ( strlen( symstr) + 1);
                        /*---------------------------------*/
                        symbol_num++;

                        if( (CurrentSymEx->Sym.st_shndx != SHN_UNDEF) &&
                            (CurrentSymEx->Sym.st_shndx < SHN_LORESERVE)) {
                            CurrentSymEx->Sym.st_shndx = shdr_table[CurrentSymEx->Sym.st_shndx]; //�V���{���G���g���̑ΏۃZ�N�V�����ԍ��X�V
                        }
                        CurrentSymEx = CurrentSymEx->next;
                    }/*-------------------------------------*/
                    
                    /*--- �V���{���e�[�u���Z�N�V�����w�b�_�̍X�V ---*/
                    CurrentShdrEx->loaded_adr = (u32)(ELi_CopySymToBuffer( ElfHandle));
                    if( (CurrentShdrEx->Shdr.sh_link != SHN_UNDEF) &&
                        (CurrentShdrEx->Shdr.sh_link < SHN_LORESERVE)) {
                            CurrentShdrEx->Shdr.sh_link = shdr_table[CurrentShdrEx->Shdr.sh_link]; //������Z�N�V�����ԍ��X�V
                    }
                    CurrentShdrEx->Shdr.sh_size = symbol_num * sizeof( Elf32_Sym);
                    /*----------------------------------------------*/
                }
                ELi_FreeSymList( ElfHandle, (u32**)(CurrentShdrEx->sym_table));    //�V���{�����X�g�J��
            }

/*            printf( "section %02x relocated at %08x\n",
                        i, CurrentShdrEx->loaded_adr);*/
        }
    }

    /*---------- REL, RELA, STRTAB�Z�N�V�����̏��� ----------*/
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);

        if( CurrentShdrEx->debug_flag == 1) {                //�f�o�b�O���̏ꍇ
        }else{                                               //�f�o�b�O���łȂ��ꍇ
            //REL�Z�N�V����
            if( CurrentShdrEx->Shdr.sh_type == SHT_REL) {
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
                {
                    Elf32_Rel*  CurrentRel;
                    ELShdrEx*   SymShdrEx;
                    u32         new_sym_num;
                    /*�V���{���Z�N�V�����擾*/
                    SymShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx,
                                                       CurrentShdrEx->Shdr.sh_link);
                    /*�R�s�[���Rel�Z�N�V�������C��*/
                    for( j=0; j<(CurrentShdrEx->Shdr.sh_size/CurrentShdrEx->Shdr.sh_entsize); j++) {
                        CurrentRel = (Elf32_Rel*)(CurrentShdrEx->loaded_adr + (j * sizeof( Elf32_Rel)));
                        new_sym_num = SymShdrEx->sym_table[ELF32_R_SYM(CurrentRel->r_info)];
                        CurrentRel->r_info = ELF32_R_INFO( new_sym_num,
                                                            ELF32_R_TYPE(CurrentRel->r_info));
                        }
                }
            }
            //RELA�Z�N�V����
            else if( CurrentShdrEx->Shdr.sh_type == SHT_RELA) {
                CurrentShdrEx->loaded_adr = (u32)
                                ELi_CopySectionToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
                {
                    Elf32_Rela* CurrentRela;
                    ELShdrEx*   SymShdrEx;
                    u32         new_sym_num;
                    /*�V���{���Z�N�V�����擾*/
                    SymShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx,
                                                       CurrentShdrEx->Shdr.sh_link);
                    /*�R�s�[���Rela�Z�N�V�������C��*/
                    for( j=0; j<(CurrentShdrEx->Shdr.sh_size/CurrentShdrEx->Shdr.sh_entsize); j++) {
                        CurrentRela = (Elf32_Rela*)(CurrentShdrEx->loaded_adr + (j * sizeof( Elf32_Rela)));
//                        printf( "symnum: 0x%x\n", ELF32_R_SYM(CurrentRela->r_info));
//                        printf( "sym_table:0x%x", (u32)(SymShdrEx->sym_table));
                        new_sym_num = SymShdrEx->sym_table[ELF32_R_SYM(CurrentRela->r_info)];
                        CurrentRela->r_info = ELF32_R_INFO( new_sym_num,
                                                            ELF32_R_TYPE(CurrentRela->r_info));
                        }
                }
            }
            //������e�[�u���Z�N�V����
            else if( CurrentShdrEx->Shdr.sh_type == SHT_STRTAB) {
                if( i == ElfHandle->CurrentEhdr.e_shstrndx) { //�Z�N�V������������e�[�u���Z�N�V����
                    CurrentShdrEx->loaded_adr = (u32)
                        ELi_CopyShStrToBuffer( ElfHandle, &(CurrentShdrEx->Shdr));
                }else{
                    CurrentShdrEx->loaded_adr = (u32)
                        ELi_CopySymStrToBuffer( ElfHandle, CurrentShdrEx);
//                    printf( "sym str section:0x%x, size:0x%x\n", i, CurrentShdrEx->str_table_size);
                }
            }
        }
    }
    /*-------------------------------------------------------*/

    
    /*�����܂łŃZ�N�V�����̒��g������stripped elf�ɃR�s�[���ꂽ*/

  
    /*---------- �Z�N�V�����w�b�_�� stripped elf�ɃR�s�[ ----------*/
    buf_shdr = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    ElfHandle->buf_current = (void*)buf_shdr;
//    printf( "buf_shdr = 0x%x\n", buf_shdr);
//    printf( "buf = 0x%x\n", (u32)buf);
    
    newelf_shoff = buf_shdr - ((u32)(buf));
//    printf( "newelf_shoff = 0x%x\n", newelf_shoff);

    section_num = 0;
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        tmp_buf = buf_shdr + ( section_num * sizeof( Elf32_Shdr));
        
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
        if( CurrentShdrEx->debug_flag == 1) {                //�f�o�b�O���̏ꍇ��NULL�Z�N�V�����ɕϊ�
//            memset( (u8*)tmp_buf, '\0', sizeof( Elf32_Shdr));
        }else{
            /*�I�t�Z�b�g�X�V*/
            if( CurrentShdrEx->loaded_adr != 0) {
                CurrentShdrEx->Shdr.sh_offset = (CurrentShdrEx->loaded_adr - (u32)buf);
            }
            /*�Z�N�V�����w�b�_��stripped elf�C���[�W�փR�s�[*/
            CurrentShdrEx->Shdr.sh_link = shdr_table[CurrentShdrEx->Shdr.sh_link]; //�Z�N�V�����ԍ��X�V
            if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                /*�V���{���e�[�u����sh_type�̓V���{���G���g���̃C���f�b�N�X��\��*/
            }else{
                CurrentShdrEx->Shdr.sh_info = shdr_table[CurrentShdrEx->Shdr.sh_info]; //�Z�N�V�����ԍ��X�V
            }
            memcpy( (u8*)tmp_buf, &(CurrentShdrEx->Shdr), 
                    sizeof( Elf32_Shdr));
            section_num++;                        /*�Z�N�V�������X�V*/
        }
    }
    // �R�s�[�I����
    ElfHandle->process = EL_COPIED;
    /*------------------------------------------------------------*/

    ElfHandle->newelf_size = (buf_shdr - (u32)buf) + (section_num*sizeof( Elf32_Shdr));
//    printf( "newelf_size = 0x%x\n", ElfHandle->newelf_size);

    /*---------- ELF�w�b�_�X�V ----------*/
    ElfHandle->CurrentEhdr.e_shnum = section_num; /*�������Z�N�V��������ELF�w�b�_�ɔ��f*/
    ElfHandle->CurrentEhdr.e_shstrndx = shdr_table[ElfHandle->CurrentEhdr.e_shstrndx];
    //�Z�N�V�����w�b�_�I�t�Z�b�g�X�V
    ElfHandle->CurrentEhdr.e_shoff = newelf_shoff;
    memcpy( (u8*)buf, &(ElfHandle->CurrentEhdr), sizeof( Elf32_Ehdr)); /*ELF�w�b�_��stripped elf�C���[�W�ɃR�s�[*/
    /*-----------------------------------*/


    /*---------- �Ĕz�u ----------*/
/*    if( unresolved_table_block_flag == 0) {
        if( dbg_print_flag == 1) {
            printf( "\nRelocate Symbols:\n");
        }
    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        //
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
        
        if( CurrentShdrEx->debug_flag == 1) {                //�f�o�b�O���̏ꍇ
        }else{                                                //�f�o�b�O���łȂ��ꍇ

            if( CurrentShdrEx->Shdr.sh_type == SHT_REL) {
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);
                if( dbg_print_flag == 1) {
                    printf( "num of REL = %x\n", num_of_entry);
                    printf( "Section Header Info.\n");
                    printf( "link   : %x\n", CurrentShdrEx->Shdr.sh_link);
                    printf( "info   : %x\n", CurrentShdrEx->Shdr.sh_info);
                    printf( " Offset     Info    Type            Sym.Value  Sym.Name\n");
                }
                offset = 0;
                for( j=0; j<num_of_entry; j++) {
                    ELi_GetSent( ElfHandle, i, &(ElfHandle->Rel), offset, sizeof(Elf32_Rel));
                    ELi_GetShdr( ElfHandle, CurrentShdrEx->Shdr.sh_link, &(ElfHandle->SymShdr));
                    ELi_GetSent( ElfHandle, CurrentShdrEx->Shdr.sh_link, &(ElfHandle->Sym),
                                 (u32)(ElfHandle->SymShdr.sh_entsize * ELF32_R_SYM( ElfHandle->Rel.r_info)), sizeof(Elf32_Sym));
                    ELi_GetStrAdr( ElfHandle, ElfHandle->SymShdr.sh_link, ElfHandle->Sym.st_name, sym_str, 128);

                    if( dbg_print_flag == 1) {
                        printf( "%08x  ", ElfHandle->Rel.r_offset);
                        printf( "%08x ", ElfHandle->Rel.r_info);
                        printf( "                  ");
                        printf( "%08x ", ElfHandle->Sym.st_value);
                        printf( sym_str);
                        printf( "\n");
                    }
                    //���̃G���g����
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
                if( dbg_print_flag == 1) {                
                    printf( "\n");
                }
                //�����P�[�g
                ELi_RelocateSym( ElfHandle, i);
                if( dbg_print_flag == 1) {
                    printf( "\n");
                }
            }
            else if( CurrentShdrEx->Shdr.sh_type == SHT_RELA) {
                
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);
                if( dbg_print_flag == 1) {
                    printf( "num of RELA = %x\n", num_of_entry);
                    printf( "Section Header Info.\n");
                    printf( "link   : %x\n", CurrentShdrEx->Shdr.sh_link);
                    printf( "info   : %x\n", CurrentShdrEx->Shdr.sh_info);
                    printf( " Offset     Info    Type            Sym.Value  Sym.Name\n");
                }
                offset = 0;
                for( j=0; j<num_of_entry; j++) {
                    ELi_GetSent( ElfHandle, i, &(ElfHandle->Rela), offset, sizeof(Elf32_Rel));
                    ELi_GetShdr( ElfHandle, CurrentShdrEx->Shdr.sh_link, &(ElfHandle->SymShdr));
                    ELi_GetSent( ElfHandle, CurrentShdrEx->Shdr.sh_link, &(ElfHandle->Sym),
                                 (u32)(ElfHandle->SymShdr.sh_entsize * ELF32_R_SYM( ElfHandle->Rela.r_info)), sizeof(Elf32_Sym));
                    ELi_GetStrAdr( ElfHandle, ElfHandle->SymShdr.sh_link, ElfHandle->Sym.st_name, sym_str, 128);

                    if( dbg_print_flag == 1) {
                        printf( "%08x  ", ElfHandle->Rela.r_offset);
                        printf( "%08x ", ElfHandle->Rela.r_info);
                        printf( "                  ");
                        printf( "%08x ", ElfHandle->Sym.st_value);
                        printf( sym_str);
                        printf( "\n");
                    }
                    //���̃G���g����
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
                if( dbg_print_flag == 1) {                
                    printf( "\n");
                }
                //�����P�[�g
                ELi_RelocateSym( ElfHandle, i);
                if( dbg_print_flag == 1) {
                    printf( "\n");
                }
            }
        }
    }
    }else{    //dll�łȂ���static���W���[���̏ꍇ
        for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
            CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);
            if( CurrentShdrEx->debug_flag == 1) {                //�f�o�b�O���̏ꍇ
            }else{
                if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                    ELi_DiscriminateGlobalSym( ElfHandle, i);
                }
            }
        }
    }*/

    /*------- ELShdrEx�̃��X�g��������� -------*/
    CurrentShdrEx = ElfHandle->ShdrEx;
    if( CurrentShdrEx) {
        while( CurrentShdrEx->next != NULL) {
            FwdShdrEx = CurrentShdrEx;
            CurrentShdrEx = CurrentShdrEx->next;
            free( FwdShdrEx);
        }
        ElfHandle->ShdrEx = NULL;
    }
    /*-----------------------------------------*/

    /*RAM���DLL���Ă΂��O�ɃL���b�V�����t���b�V��*/
//    DC_FlushAll();
//    DC_WaitWriteBufferEmpty();
    
    return (ElfHandle->process);
}

/*------------------------------------------------------
  �������̃V���{��������΃A�h���X�e�[�u�����g���ĉ�������
 -----------------------------------------------------*/
u16 EL_ResolveAllLibrary( void)
{
    ELAdrEntry*           AdrEnt;
    ELUnresolvedEntry*    RemoveUnrEnt;
    ELUnresolvedEntry*    UnrEnt;
//    ELUnresolvedEntry*    CurrentUnrEnt;
//    ELUnresolvedEntry*    FwdUnrEnt;
//    u32                   relocation_val;
//    ELAdrEntry            AddAdrEnt;
    char                  sym_str[128];

    UnrEnt = ELUnrEntStart;
    if( dbg_print_flag == 1) {
        printf( "\nResolve all symbols:\n");
    }
    while( UnrEnt != NULL) {
        if( UnrEnt->remove_flag == 0) {
            AdrEnt = EL_GetAdrEntry( UnrEnt->sym_str);        /*�A�h���X�e�[�u�����猟��*/
            if( AdrEnt) {                                    /*�A�h���X�e�[�u�����猩�������ꍇ*/
                UnrEnt->S_ = (u32)(AdrEnt->adr);
                UnrEnt->T_ = (u32)(AdrEnt->thumb_flag);
                if( unresolved_table_block_flag == 0) {
                    if( dbg_print_flag == 1) {
//                        printf( "\n symbol found %s : %8x\n", UnrEnt->sym_str, UnrEnt->S_);
                    }
                    UnrEnt->remove_flag = 1;
    //                ELi_RemoveUnresolvedEntry( UnrEnt);    //���������̂Ŗ��������X�g����폜
                }else{
                    if( dbg_print_flag == 1) {
                        printf( "\n static symbol found %s : %8x\n", UnrEnt->sym_str, (int)(UnrEnt->S_));
                    }
                    UnrEnt->AdrEnt = AdrEnt;            //�������A�h���X�G���g�����Z�b�g
                    UnrEnt->remove_flag = 2;            //�}�[�L���O�imakelst�����Ŏg�p������ʂȒl�j
                    strcpy( sym_str, UnrEnt->sym_str);
                    while( 1) {
                        RemoveUnrEnt = ELi_GetUnresolvedEntry( sym_str);
                        if( RemoveUnrEnt == NULL) {
                            break;
                        }else{
                            RemoveUnrEnt->remove_flag = 1;
                        }
                    }
                }
    /*            relocation_val = ELi_DoRelocate( UnrEnt);    //�V���{������
                if( !relocation_val) {
                    return EL_FAILED;
                }*/
            
            }else{                                            /*�A�h���X�e�[�u�����猩����Ȃ������ꍇ*/
                if( unresolved_table_block_flag == 0) {
                    if( dbg_print_flag == 1) {
                        printf( "ERROR! cannot find symbol : %s\n", UnrEnt->sym_str);
                    }
                }else{
                    if( dbg_print_flag == 1) {
                        printf( "ERROR! cannot find static symbol : %s\n", UnrEnt->sym_str);
                    }
                }
    /*            AddAdrEnt.next = NULL;
                AddAdrEnt.name = UnrEnt->sym_str;
                AddAdrEnt.
                EL_AddAdrEntry( */
    //            return EL_FAILED;
            }
        }
        UnrEnt = (ELUnresolvedEntry*)(UnrEnt->next);                            /*���̖������G���g����*/
    }
    
    /*--- ELUnresolvedEntry�̃��X�g��������� ---*/
/*    CurrentUnrEnt = ELUnrEntStart;
    if( CurrentUnrEnt) {
        while( CurrentUnrEnt->next != NULL) {
            FwdUnrEnt = CurrentUnrEnt;
            CurrentUnrEnt = CurrentUnrEnt->next;
            free( FwdUnrEnt->sym_str);    //�V���{����������
            free( FwdUnrEnt);            //�\���̎��g
        }
        ELUnrEntStart = NULL;
    }*/
    /*-------------------------------------------*/

    /*RAM���DLL���Ă΂��O�ɃL���b�V�����t���b�V��*/
//    DC_FlushAll();
//    DC_WaitWriteBufferEmpty();
    
    return EL_RELOCATED;
}





/*------------------------------------------------------
  �A�h���X�e�[�u������G���g�����폜����
 -----------------------------------------------------*/
BOOL EL_RemoveAdrEntry( ELAdrEntry* AdrEnt)
{
    ELAdrEntry  DmyAdrEnt;
    ELAdrEntry* CurrentAdrEnt;

    DmyAdrEnt.next = ELAdrEntStart;
    CurrentAdrEnt  = &DmyAdrEnt;

    while( CurrentAdrEnt->next != AdrEnt) {
        if( CurrentAdrEnt->next == NULL) {
            return FALSE;
        }else{
            CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        }
    }

    CurrentAdrEnt->next = AdrEnt->next;
    ELAdrEntStart = DmyAdrEnt.next;
    
    return TRUE;
}

/*------------------------------------------------------
  �A�h���X�e�[�u���ɃG���g����ǉ�����
 -----------------------------------------------------*/
void EL_AddAdrEntry( ELAdrEntry* AdrEnt)
{
    ELAdrEntry  DmyAdrEnt;
    ELAdrEntry* CurrentAdrEnt;

    if( !ELAdrEntStart) {
        ELAdrEntStart = AdrEnt;
    }else{
        DmyAdrEnt.next = ELAdrEntStart;
        CurrentAdrEnt = &DmyAdrEnt;

        while( CurrentAdrEnt->next != NULL) {
            CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        }
        CurrentAdrEnt->next = (void*)AdrEnt;
    }
    AdrEnt->next = NULL;
}

/*------------------------------------------------------
  �A�h���X�e�[�u������w�蕶����ɊY������G���g����T��
 -----------------------------------------------------*/
ELAdrEntry* EL_GetAdrEntry( char* ent_name)
{
    ELAdrEntry* CurrentAdrEnt;

    CurrentAdrEnt = ELAdrEntStart;
    if( CurrentAdrEnt == NULL) {
        return NULL;
    }
    while( strcmp( CurrentAdrEnt->name, ent_name) != 0) {
        CurrentAdrEnt = (ELAdrEntry*)CurrentAdrEnt->next;
        if( CurrentAdrEnt == NULL) {
            break;
        }
    }
    return CurrentAdrEnt;
}

/*------------------------------------------------------
  �A�h���X�e�[�u������w�蕶����ɊY������A�h���X��Ԃ�
 -----------------------------------------------------*/
void* EL_GetGlobalAdr( char* ent_name)
{
    u32         adr;
    ELAdrEntry* CurrentAdrEnt;

    CurrentAdrEnt = EL_GetAdrEntry( ent_name);

    if( CurrentAdrEnt) {
        if( CurrentAdrEnt->thumb_flag) {
            adr = (u32)(CurrentAdrEnt->adr) + 1;
        }else{
            adr = (u32)(CurrentAdrEnt->adr);
        }
    }else{
        adr = 0;
    }

    return (void*)(adr);
}

#if 0
/*------------------------------------------------------
  �A�h���X�e�[�u�����������i�A�v���P�[�V�������o�^�����G���g���܂ō폜���悤�Ƃ���̂łm�f�j
 -----------------------------------------------------*/
void* EL_FreeAdrTbl( void)
{
    ELAdrEntry*    FwdAdrEnt;
    ELAdrEntry*    CurrentAdrEnt;
    
    /*--- ELAdrEntry�̃��X�g��������� ---*/
    CurrentAdrEnt = ELAdrEntStart;
    if( CurrentAdrEnt) {
        while( CurrentAdrEnt->next != NULL) {
            FwdAdrEnt = CurrentAdrEnt;
            CurrentAdrEnt = CurrentAdrEnt->next;
            free( FwdAdrEnt->name);        //�V���{����������
            free( FwdAdrEnt);            //�\���̎��g
        }
        ELAdrEntStart = NULL;
    }
    /*------------------------------------*/
}
#endif

/*------------------------------------------------------
  ELF�I�u�W�F�N�g����f�[�^��ǂݏo���X�^�u
 -----------------------------------------------------*/
void ELi_ReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
//  printf( "0x%x, 0x%x\n", file_offset, size);
    fseek( file_struct, file_offset, SEEK_SET);
    fread( buf, 1, size, file_struct);
    
/*    FS_SeekFile( file_struct, (s32)(file_offset), FS_SEEK_SET);
    FS_ReadFile( file_struct, buf, (s32)(size));*/
}

void ELi_ReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
/*    MI_CpuCopy8( (void*)(file_base + file_offset),
                buf,
                size);*/
    memcpy( buf,
            (void*)(file_base + file_offset),
            size);
}
