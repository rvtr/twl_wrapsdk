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

#include "el_config.h"

#if (TARGET_OS_NITRO == 1)
#include <twl.h>
#else
#include <ctr.h>
#include <ctr/fs.h>
#include <stdio.h>
#include <string.h>
#endif

#include "elf.h"
#include "elf_loader.h"
#include "arch.h"
#include "loader_subset.h"

//#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
#if (TARGET_OS_NITRO == 1)
OSHeapHandle        EL_Heap;
#endif
ElAdrEntry*         ELAdrEntStart = NULL;
ElUnresolvedEntry*  ELUnrEntStart = NULL;


/*------------------------------------------------------
  �O���[�o���ϐ�
 -----------------------------------------------------*/
//#if (TARGET_OS_NITRO == 1)
//#else
ElReadImage i_elReadImage;
ElAlloc     i_elAlloc;
ElFree      i_elFree;
//#endif


/*------------------------------------------------------
  ���[�J���֐��̐錾
 -----------------------------------------------------*/
static u16 elLoadSegments( ElDesc* elElfDesc);
static u16 elLoadSections( ElDesc* elElfDesc);

// ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���o�b�t�@�ɍĔz�u����
static u16 i_elLoadLibrary( ElDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf);
// ELF�I�u�W�F�N�g���o�b�t�@�ɍĔz�u����R�A�֐�
static u16 i_elLoadObject( ElDesc* elElfDesc, void* obj_offset, void* buf);
// ELF�I�u�W�F�N�g����f�[�^��ǂݏo���X�^�u�֐�
static void i_elReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
static void i_elReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
static void i_elReadUsr( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);


/*---------------------------------------------------------
 ELF�I�u�W�F�N�g�̃T�C�Y�����߂�
    
    buf : ELF�C���[�W�̃A�h���X
 --------------------------------------------------------*/
u32 elGetElfSize( const void* buf)
{
    Elf32_Ehdr  Ehdr;
    u32         size;
    
    if( ELF_LoadELFHeader( buf, &Ehdr) == NULL) {
        return 0;
    }
    size = (u32)(Ehdr.e_shoff + (Ehdr.e_shentsize * Ehdr.e_shnum));
    return size;
}


/*------------------------------------------------------
  �_�C�i�~�b�N�����N�V�X�e��������������
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
void elInit( void)
{
    void* heap_start;

    /*--- �������A���P�[�V�����֌W�̐ݒ� ---*/
    OS_InitArena();
    heap_start = OS_InitAlloc( OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo( heap_start );
    EL_Heap = OS_CreateHeap( OS_ARENA_MAIN, heap_start, (void*)((u32)(OS_GetMainArenaHi())+1));
    OS_SetCurrentHeap( OS_ARENA_MAIN, EL_Heap);
    /*--------------------------------------*/
}
#else
void elInit( ElAlloc alloc, ElFree free)
{
    i_elAlloc = alloc;
    i_elFree = free;
}
#endif

/*------------------------------------------------------
  ElDesc�\���̂�����������
 -----------------------------------------------------*/
BOOL elInitDesc( ElDesc* elElfDesc)
{
    if( elElfDesc == NULL) {    /*NULL�`�F�b�N*/
        return FALSE;
    }

    /*�����l�̐ݒ�*/
    elElfDesc->ShdrEx   = NULL;
    elElfDesc->SymEx    = NULL;
    elElfDesc->SymExTbl = NULL;
    elElfDesc->SymExTarget = 0xFFFFFFFF;

    elElfDesc->process = EL_INITIALIZED;    /*�t���O�̐ݒ�*/

    return TRUE;
}

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���o�b�t�@�ɍĔz�u����
    
    elElfDesc : �w�b�_�\����
    ObjFile : OBJ�t�@�C���܂��̓A�[�J�C�u�t�@�C���̍\����
    buf : ���[�h����o�b�t�@
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
u16 elLoadLibraryfromFile( ElDesc* elElfDesc, FSFile* ObjFile, void* buf)
#else
u16 elLoadLibraryfromFile( ElDesc* elElfDesc, int ObjFile, void* buf)
#endif
{
    u16 result;
    u32 len;

    /*���[�h�֐��̐ݒ�*/
    elElfDesc->i_elReadStub = i_elReadFile;
    elElfDesc->FileStruct = (int*)ObjFile;
    
#if (TARGET_OS_NITRO == 1)
    len = FS_GetLength( ObjFile);
#else
    len = fsLseek( ObjFile, 0, SEEK_END);
    fsLseek( ObjFile, 0, SEEK_SET);
#endif
    
    result = i_elLoadLibrary( elElfDesc, NULL, len, buf);

    return result;
}

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���o�b�t�@�ɍĔz�u����
    
    elElfDesc : �w�b�_�\����
    readfunc : OBJ�t�@�C���܂��̓A�[�J�C�u�t�@�C����ǂݏo�����[�U�֐�
    buf : ���[�h����o�b�t�@
 -----------------------------------------------------*/
//#if (TARGET_OS_NITRO == 1)
//#else
u16 elLoadLibrary( ElDesc* elElfDesc, ElReadImage readfunc, u32 len, void* buf)
{
    i_elReadImage = readfunc;
    elElfDesc->i_elReadStub = i_elReadUsr;

    return( i_elLoadLibrary( elElfDesc, NULL, len, buf));
}
//#endif

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���o�b�t�@�ɍĔz�u����
    
    elElfDesc : �w�b�_�\����
    obj_image : OBJ�t�@�C���܂��̓A�[�J�C�u�t�@�C����RAM��C���[�W�A�h���X
    buf : ���[�h����o�b�t�@
 -----------------------------------------------------*/
u16 elLoadLibraryfromMem( ElDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf)
{
    u16 result;
    
    /*���[�h�֐��̐ݒ�*/
    elElfDesc->i_elReadStub = i_elReadMem;
    elElfDesc->FileStruct = NULL;
    
    result = i_elLoadLibrary( elElfDesc, obj_image, obj_len, buf);

    return result;
}

/*------------------------------------------------------
  ELF�I�u�W�F�N�g�܂��͂��̃A�[�J�C�u���o�b�t�@�ɍĔz�u����
    
    elElfDesc : �w�b�_�\����
    obj_image : OBJ�t�@�C���܂��̓A�[�J�C�u�t�@�C����RAM��C���[�W�A�h���X
    buf : ���[�h����o�b�t�@
 -----------------------------------------------------*/
static u16 i_elLoadLibrary( ElDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf)
{
    u16        result, all_result;
    u32        image_pointer;
    u32        arch_size;
    u32        elf_num = 0;                /*ELF�I�u�W�F�N�g�̐�*/
    ArchHdr    ArHdr;
    char       OBJMAG[8];
    char       ELFMAG[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

    all_result = EL_RELOCATED;
    elElfDesc->ar_head = obj_image;
    image_pointer = 0;

    elElfDesc->i_elReadStub( OBJMAG, elElfDesc->FileStruct, (u32)obj_image, 0, 8);    /*OBJ�̕�������擾*/
    /*--------------- �A�[�J�C�u�t�@�C���̏ꍇ ---------------*/
    if( OSAPI_STRNCMP( OBJMAG, ARMAG, 8) == 0) {
        arch_size = sizeof( ArchHdr);
        image_pointer += 8;                /*�ŏ��̃G���g����*/
        
        elElfDesc->buf_current = buf;
        while( image_pointer < obj_len) {
            elElfDesc->i_elReadStub( OBJMAG, elElfDesc->FileStruct, (u32)(obj_image), (image_pointer+arch_size), 4);    /*OBJ�̕�������擾*/
            if( OSAPI_STRNCMP( OBJMAG, ELFMAG, 4) == 0) {
                elf_num++;
                result = i_elLoadObject( elElfDesc, (void*)(image_pointer+arch_size), elElfDesc->buf_current);
                if( result < all_result) {        /*�������ʂ̂Ƃ�����all_result�ɔ��f*/
                    all_result = result;
                }
                /*�����l�̐ݒ�*/
                elElfDesc->ShdrEx   = NULL;
                elElfDesc->SymEx    = NULL;
                elElfDesc->SymExTbl = NULL;
                elElfDesc->SymExTarget = 0xFFFFFFFF;
                elElfDesc->process = EL_INITIALIZED;    /*�t���O�̐ݒ�*/
            }else{
            }
            /*���̃G���g����*/
            elElfDesc->i_elReadStub( &ArHdr, elElfDesc->FileStruct, (u32)(obj_image), image_pointer, arch_size);
            image_pointer += arch_size + AR_GetEntrySize( &ArHdr);
        }
    }else{/*--------------- ELF�t�@�C���̏ꍇ ---------------*/
        if( OSAPI_STRNCMP( OBJMAG, ELFMAG, 4) == 0) {
            elf_num++;
            all_result = i_elLoadObject( elElfDesc, 0, buf);
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
    
    elElfDesc : �w�b�_�\����
    obj_offset : OBJ�t�@�C����RAM��C���[�W�A�h���X����̃I�t�Z�b�g
    buf : ���[�h����o�b�t�@(TODO:�o�b�t�@�I�[�o�[�t���[�΍�)
 -----------------------------------------------------*/
static u16 i_elLoadObject( ElDesc* elElfDesc, void* obj_offset, void* buf)
{
    u16 ret_val;
    
    /* ElDesc�̏������`�F�b�N */
    if( elElfDesc->process != EL_INITIALIZED) {
        return EL_FAILED;
    }
    /* ELF�w�b�_�̎擾 */
    elElfDesc->i_elReadStub( &(elElfDesc->CurrentEhdr), elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head), (u32)(obj_offset), sizeof( Elf32_Ehdr));

    /* ElDesc�\���̂̍\�z */
    elElfDesc->elf_offset = obj_offset;
    elElfDesc->buf_current = buf;
    elElfDesc->shentsize = elElfDesc->CurrentEhdr.e_shentsize;
    elElfDesc->entry_adr = elElfDesc->CurrentEhdr.e_entry;

    /* ELF�t�@�C���^�C�v�ʂɏ��� */
    switch( elElfDesc->CurrentEhdr.e_type) {
        
      case ET_NONE:
        PRINTDEBUG( "ERROR : Elf type \"ET_NONE\"\n");
        ret_val = EL_FAILED;
        break;
        
      case ET_REL:  /* ���s�~�A�Ĕz�u�� */
        PRINTDEBUG( "Elf type \"ET_REL\"\n");
        if( buf == NULL) {        /* �o�b�t�@��NULL�`�F�b�N */
            return EL_FAILED;
        }
        ret_val = elLoadSections( elElfDesc);
        break;
        
      case ET_EXEC: /* ���s���A�Ĕz�u�~ */
        PRINTDEBUG( "Elf type \"ET_EXEC\"\n");
        ret_val = elLoadSegments( elElfDesc);
        break;
        
      case ET_DYN:  /* ���s���A�Ĕz�u�� (TODO:���e�X�g)*/
        PRINTDEBUG( "Elf type \"ET_DYN\"\n");
        if( buf == NULL) { //���[�h�A�h���X���w�肳��ĂȂ��Ƃ���ET_EXEC����
            ret_val = elLoadSegments( elElfDesc);
        }else{             //���[�h�A�h���X���w�肳��Ă����ET_REL����
            ret_val = elLoadSections( elElfDesc);
        }
        break;
        
      case ET_CORE:
        PRINTDEBUG( "ERROR : Elf type \"ET_CORE\"\n");
        ret_val = EL_FAILED;
        break;
        
      default:
        PRINTDEBUG( "ERROR : Invalid Elf type 0x%x\n",
                    elElfDesc->CurrentEhdr.e_type);
        ret_val = EL_FAILED;
        break;
    }

    return( ret_val);
}


/*------------------------------------------------------
  �S�Z�O�����g�𒲂ׂăR�s�[����
    
    elElfDesc : �w�b�_�\����
 -----------------------------------------------------*/
static u16 elLoadSegments( ElDesc* elElfDesc)
{
    u16        i;
    //u32        load_start;
    Elf32_Phdr CurrentPhdr;
    
    for( i=0; i<(elElfDesc->CurrentEhdr.e_phnum); i++) {
        /*�v���O�����w�b�_���R�s�[*/
        i_elGetPhdr( elElfDesc, i, &CurrentPhdr);
        
        if( CurrentPhdr.p_type == PT_LOAD) {
            /*���[�h�\�Z�O�����g�Ȃ烁�����Ƀ��[�h*/
            i_elCopySegmentToBuffer( elElfDesc, &CurrentPhdr);
        }else{
            PRINTDEBUG( "WARNING : skip segment (type = 0x%x)\n",
                        CurrentPhdr.p_type);
        }
    }
    elElfDesc->process = EL_COPIED;
    return( elElfDesc->process);
}


/*------------------------------------------------------
  �S�Z�N�V�����𒲂ׂăR�s�[����
    
    elElfDesc : �w�b�_�\����
 -----------------------------------------------------*/
static u16 elLoadSections( ElDesc* elElfDesc)
{
    u16         i;
    ElShdrEx*   FwdShdrEx;
    ElShdrEx*   CurrentShdrEx;
    ElShdrEx*   InfoShdrEx;      //�Ⴆ��CurrentShdrEx��rel.text�̏ꍇ.text������
    ElShdrEx    DmyShdrEx;
#if (DEBUG_PRINT_ON == 1)
    u16         j;
    u32         num_of_entry;
    char        sym_str[128];    //�f�o�b�O�v�����g�p
    u32         offset;          //�f�o�b�O�v�����g�p
#endif
    
    /*---------- ElShdrEx�̃��X�g����� ----------*/
    CurrentShdrEx = &DmyShdrEx;
    for( i=0; i<(elElfDesc->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx->next = OSAPI_MALLOC( sizeof(ElShdrEx));
        CurrentShdrEx = (ElShdrEx*)(CurrentShdrEx->next);
        OSAPI_CPUFILL8( CurrentShdrEx, 0, sizeof(ElShdrEx));    //�[���N���A
        
        /*�f�o�b�O��񂩂ǂ����𔻕ʂ��ăt���O���Z�b�g*/
        if( i_elShdrIsDebug( elElfDesc, i) == TRUE) {    /*�f�o�b�O���̏ꍇ*/
            CurrentShdrEx->debug_flag = 1;
        }else{                                           /*�f�o�b�O���łȂ��ꍇ*/
            /*�Z�N�V�����w�b�_���R�s�[*/
            i_elGetShdr( elElfDesc, i, &(CurrentShdrEx->Shdr));
            CurrentShdrEx->debug_flag = 0;
        }
    }
    CurrentShdrEx->next = NULL;
    elElfDesc->ShdrEx = DmyShdrEx.next;
    /*--------------------------------------------*/

    /*---------- �S�Z�N�V�����𒲂ׂăR�s�[���� ----------*/
    PRINTDEBUG( "\nLoad to RAM:\n");
    for( i=0; i<(elElfDesc->CurrentEhdr.e_shnum); i++) {
        /**/
        CurrentShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx, i);
//        PRINTDEBUG( "section:%d sh_flag=0x%x\n", i, CurrentShdrEx->Shdr.sh_flags);
//        PRINTDEBUG( "section:%d sh_type=0x%x\n", i, CurrentShdrEx->Shdr.sh_type);

        if( CurrentShdrEx->debug_flag == 1) {              /*�f�o�b�O���̏ꍇ*/
            PRINTDEBUG( "skip debug-section %02x\n", i);
        }else{                                             /*�f�o�b�O���łȂ��ꍇ*/
            /* .text section */
            if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //�������ɃR�s�[
                CurrentShdrEx->loaded_adr = (u32)
                                i_elCopySectionToBuffer( elElfDesc, &(CurrentShdrEx->Shdr));
            }
            /* .data, .data1 section (�������ς݃f�[�^) */
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //�������ɃR�s�[
                CurrentShdrEx->loaded_adr = (u32)
                                i_elCopySectionToBuffer( elElfDesc, &(CurrentShdrEx->Shdr));
            }
            /* .bss section */
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_NOBITS)) {
                //�R�s�[���Ȃ�
                CurrentShdrEx->loaded_adr = (u32)
                                i_elAllocSectionToBuffer( elElfDesc, &(CurrentShdrEx->Shdr));
            }
            /* .rodata, .rodata1 section */
            else if( (CurrentShdrEx->Shdr.sh_flags == SHF_ALLOC)&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //�������ɃR�s�[
                CurrentShdrEx->loaded_adr = (u32)
                                i_elCopySectionToBuffer( elElfDesc, &(CurrentShdrEx->Shdr));
            }

            PRINTDEBUG( "section %02x relocated at %08x\n",
                        i, CurrentShdrEx->loaded_adr);
        }
    }
    /* �R�s�[�I���� */
    elElfDesc->process = EL_COPIED;
    /*----------------------------------------------------*/

    /*---------- �O���[�o���V���{���̌��J�ƃ��[�J���V���{���̍Ĕz�u ----------*/
    PRINTDEBUG( "\nRelocate Symbols:\n");
    for( i=0; i<(elElfDesc->CurrentEhdr.e_shnum); i++) {
        /**/
        CurrentShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx, i);
        
        if( CurrentShdrEx->debug_flag == 1) {                /*�f�o�b�O���̏ꍇ*/
        }else{                                               /*�f�o�b�O���łȂ��ꍇ*/

            if( CurrentShdrEx->Shdr.sh_type == SHT_REL) {
                /*�����P�[�g*/
                InfoShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx,
                                                    CurrentShdrEx->Shdr.sh_info);
                if( InfoShdrEx->loaded_adr != 0) { //�ΏۃZ�N�V���������[�h����Ă���Γ������Ĕz�u
                    i_elRelocateSym( elElfDesc, i);
                }
#if (DEBUG_PRINT_ON == 1)
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);

                PRINTDEBUG( "num of REL = %x\n", num_of_entry);
                PRINTDEBUG( "Section Header Info.\n");
                PRINTDEBUG( "link   : %x\n", CurrentShdrEx->Shdr.sh_link);
                PRINTDEBUG( "info   : %x\n", CurrentShdrEx->Shdr.sh_info);
                PRINTDEBUG( " Offset     Info    Type            Sym.Value  Sym.Name\n");
                offset = 0;
                for( j=0; j<num_of_entry; j++) {
                    i_elGetSent( elElfDesc, i, &(elElfDesc->Rel), offset, sizeof(Elf32_Rel));
                    i_elGetShdr( elElfDesc, CurrentShdrEx->Shdr.sh_link, &(elElfDesc->SymShdr));
                    i_elGetSent( elElfDesc, CurrentShdrEx->Shdr.sh_link, &(elElfDesc->Sym),
                                 (u32)(elElfDesc->SymShdr.sh_entsize * ELF32_R_SYM( elElfDesc->Rel.r_info)), sizeof(Elf32_Sym));
                    i_elGetStrAdr( elElfDesc, elElfDesc->SymShdr.sh_link, elElfDesc->Sym.st_name, sym_str, 128);
                
                    PRINTDEBUG( "%08x  ", elElfDesc->Rel.r_offset);
                    PRINTDEBUG( "%08x ", elElfDesc->Rel.r_info);
                    PRINTDEBUG( "                  ");
                    PRINTDEBUG( "%08x ", elElfDesc->Sym.st_value);
                    PRINTDEBUG( sym_str);
                    PRINTDEBUG( "\n");
                    /*���̃G���g����*/
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
#endif
            }
            else if( CurrentShdrEx->Shdr.sh_type == SHT_RELA) {
                /*�����P�[�g*/
                InfoShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx,
                                                    CurrentShdrEx->Shdr.sh_info);
                if( InfoShdrEx->loaded_adr != 0) { //�ΏۃZ�N�V���������[�h����Ă���Γ������Ĕz�u
                    i_elRelocateSym( elElfDesc, i);
                }
                
#if (DEBUG_PRINT_ON == 1)
                num_of_entry = (CurrentShdrEx->Shdr.sh_size) /
                                (CurrentShdrEx->Shdr.sh_entsize);
                PRINTDEBUG( "num of RELA = %x\n", num_of_entry);
                PRINTDEBUG( "Section Header Info.\n");
                PRINTDEBUG( "link   : %x\n", CurrentShdrEx->Shdr.sh_link);
                PRINTDEBUG( "info   : %x\n", CurrentShdrEx->Shdr.sh_info);
                PRINTDEBUG( " Offset     Info    Type            Sym.Value  Sym.Name\n");
                offset = 0;
                for( j=0; j<num_of_entry; j++) {
                    i_elGetSent( elElfDesc, i, &(elElfDesc->Rela), offset, sizeof(Elf32_Rel));
                    i_elGetShdr( elElfDesc, CurrentShdrEx->Shdr.sh_link, &(elElfDesc->SymShdr));
                    i_elGetSent( elElfDesc, CurrentShdrEx->Shdr.sh_link, &(elElfDesc->Sym),
                                 (u32)(elElfDesc->SymShdr.sh_entsize * ELF32_R_SYM( elElfDesc->Rela.r_info)), sizeof(Elf32_Sym));
                    i_elGetStrAdr( elElfDesc, elElfDesc->SymShdr.sh_link, elElfDesc->Sym.st_name, sym_str, 128);
                
                    PRINTDEBUG( "%08x  ", elElfDesc->Rela.r_offset);
                    PRINTDEBUG( "%08x ", elElfDesc->Rela.r_info);
                    PRINTDEBUG( "                  ");
                    PRINTDEBUG( "%08x ", elElfDesc->Sym.st_value);
                    PRINTDEBUG( sym_str);
                    PRINTDEBUG( "\n");
                    /*���̃G���g����*/
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
#endif
            }
            else if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                /*�O���[�o���V���{�����A�h���X�e�[�u���ɓo�^*/
                i_elGoPublicGlobalSym( elElfDesc, i);
            }
        }
    }
    /*i_elRelocateSym��i_elGoPublicGlobalSym���ō쐬&�g���񂵂��V���{�����X�g���J��*/
    i_elFreeSymList( elElfDesc);

    /*------- ElShdrEx�̃��X�g��������� -------*/
    CurrentShdrEx = elElfDesc->ShdrEx;
    if( CurrentShdrEx) {
        while( CurrentShdrEx->next != NULL) {
            FwdShdrEx = CurrentShdrEx;
            CurrentShdrEx = CurrentShdrEx->next;
            OSAPI_FREE( FwdShdrEx);
        }
        elElfDesc->ShdrEx = NULL;
    }
    /*-----------------------------------------*/

    /*RAM���DLL���Ă΂��O�ɃL���b�V�����t���b�V��*/
#if (TARGET_ARM_V5 == 1)
    OSAPI_FLUSHCACHEALL();
    OSAPI_WAITCACHEBUF();
#endif
    
    return (elElfDesc->process);
}

/*------------------------------------------------------
  �������̃V���{��������΃A�h���X�e�[�u�����g���ĉ�������
 -----------------------------------------------------*/
u16 elResolveAllLibrary( ElDesc* elElfDesc)
{
    ElAdrEntry*           AdrEnt;
    ElUnresolvedEntry*    UnrEnt;
    ElUnresolvedEntry*    CurrentUnrEnt;
    ElUnresolvedEntry*    FwdUnrEnt;
    BOOL                  ret_val;

    UnrEnt = ELUnrEntStart;
    PRINTDEBUG( "\nResolve all symbols:\n");
    while( UnrEnt != NULL) {
        AdrEnt = elGetAdrEntry( UnrEnt->sym_str);        /*�A�h���X�e�[�u�����猟��*/
        if( AdrEnt) {                                    /*�A�h���X�e�[�u�����猩�������ꍇ*/
            UnrEnt->S_ = (u32)(AdrEnt->adr);
            UnrEnt->T_ = (u32)(AdrEnt->thumb_flag);
            PRINTDEBUG( "\n symbol found %s : %8x\n", UnrEnt->sym_str, UnrEnt->S_);
            ret_val = i_elDoRelocate( elElfDesc, UnrEnt);           /*�V���{������*/
            if( ret_val == FALSE) {
                return EL_FAILED; //osPanic�̕�������?
            }else{
                UnrEnt->remove_flag = 1;                 /*���������Ƃ����}�[�L���O*/
            }
        }else{                                           /*�A�h���X�e�[�u�����猩����Ȃ������ꍇ*/
            PRINTDEBUG( "\n ERROR! cannot find symbol : %s\n\n", UnrEnt->sym_str);
            return EL_FAILED; //osPanic�̕�������?
        }
        UnrEnt = UnrEnt->next;                           /*���̖������G���g����*/
    }

    /*TODO:�����ŉ����ł��Ȃ�����UnrEnt�G���g�������폜����ׂ�*/
    
    /*--- ElUnresolvedEntry�̃��X�g��������� ---*/
    CurrentUnrEnt = ELUnrEntStart;
//    if( CurrentUnrEnt) {
        while( CurrentUnrEnt != NULL) {
            FwdUnrEnt = CurrentUnrEnt;
            CurrentUnrEnt = CurrentUnrEnt->next;
            OSAPI_FREE( FwdUnrEnt->sym_str);    //�V���{����������
            OSAPI_FREE( FwdUnrEnt);            //�\���̎��g
        }
        ELUnrEntStart = NULL;
//    }
    /*-------------------------------------------*/

    /*�x�j���̃����N���X�g���J��*/
    i_elFreeVenTbl();

    /*RAM���DLL���Ă΂��O�ɃL���b�V�����t���b�V��*/
#if (TARGET_ARM_V5 == 1)
    OSAPI_FLUSHCACHEALL();
    OSAPI_WAITCACHEBUF();
#endif
    
    return EL_RELOCATED;
}


/*------------------------------------------------------
  �A�h���X�e�[�u������G���g�����폜����
 -----------------------------------------------------*/
BOOL elRemoveAdrEntry( ElAdrEntry* AdrEnt)
{
    ElAdrEntry  DmyAdrEnt;
    ElAdrEntry* CurrentAdrEnt;

    DmyAdrEnt.next = ELAdrEntStart;
    CurrentAdrEnt = &DmyAdrEnt;

    while( CurrentAdrEnt->next != AdrEnt) {
        if( CurrentAdrEnt->next == NULL) {
            return FALSE;
        }else{
            CurrentAdrEnt = (ElAdrEntry*)CurrentAdrEnt->next;
        }
    }

    /*�����N���X�g�̌q������*/
    CurrentAdrEnt->next = AdrEnt->next;
    ELAdrEntStart = DmyAdrEnt.next;

    /*�J��*/
    OSAPI_FREE( AdrEnt);
    
     return TRUE;
}

/*------------------------------------------------------
  �A�h���X�e�[�u���ɃG���g����ǉ�����
 -----------------------------------------------------*/
void elAddAdrEntry( ElAdrEntry* AdrEnt)
{
    ElAdrEntry  DmyAdrEnt;
    ElAdrEntry* CurrentAdrEnt;

    if( !ELAdrEntStart) {
        ELAdrEntStart = AdrEnt;
    }else{
        DmyAdrEnt.next = ELAdrEntStart;
        CurrentAdrEnt = &DmyAdrEnt;

        while( CurrentAdrEnt->next != NULL) {
            CurrentAdrEnt = (ElAdrEntry*)CurrentAdrEnt->next;
        }
        CurrentAdrEnt->next = (void*)AdrEnt;
    }
    AdrEnt->next = NULL;
}

/*------------------------------------------------------
  �A�h���X�e�[�u���ɃX�^�e�B�b�N���̃G���g����ǉ�����
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
__declspec(weak) void elAddStaticSym( void)
#else
SDK_WEAK_SYMBOL void elAddStaticSym( void)
#endif
{
    PRINTDEBUG( "please link file which is generated by \"makelst\".\n");
    while( 1) {};
}

/*------------------------------------------------------
  �A�h���X�e�[�u������w�蕶����ɊY������G���g����T��
 -----------------------------------------------------*/
ElAdrEntry* elGetAdrEntry( const char* ent_name)
{
    ElAdrEntry* CurrentAdrEnt;

    CurrentAdrEnt = ELAdrEntStart;
    if( CurrentAdrEnt == NULL) {
        return NULL;
    }
    while( OSAPI_STRCMP( CurrentAdrEnt->name, ent_name) != 0) {
        CurrentAdrEnt = (ElAdrEntry*)CurrentAdrEnt->next;
        if( CurrentAdrEnt == NULL) {
            break;
        }
    }
    return CurrentAdrEnt;
}

/*------------------------------------------------------
  �A�h���X�e�[�u������w�蕶����ɊY������A�h���X��Ԃ�
 -----------------------------------------------------*/
void* elGetGlobalAdr( const char* ent_name)
{
    u32         adr;
    ElAdrEntry* CurrentAdrEnt;

    CurrentAdrEnt = elGetAdrEntry( ent_name);

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


/*------------------------------------------------------
  �A�h���X�e�[�u�����������i�A�v���P�[�V�������o�^�����G���g���܂ō폜���悤�Ƃ���̂łm�f�j
 -----------------------------------------------------*/
#if 0
void* elFreeAdrTbl( void)
{
    ElAdrEntry*    FwdAdrEnt;
    ElAdrEntry*    CurrentAdrEnt;
    
    /*--- ElAdrEntry�̃��X�g��������� ---*/
    CurrentAdrEnt = ELAdrEntStart;
    if( CurrentAdrEnt) {
        while( CurrentAdrEnt->next != NULL) {
            FwdAdrEnt = CurrentAdrEnt;
            CurrentAdrEnt = CurrentAdrEnt->next;
            OSAPI_FREE( FwdAdrEnt->name);        //�V���{����������
            OSAPI_FREE( FwdAdrEnt);              //�\���̎��g
        }
        ELAdrEntStart = NULL;
    }
    /*------------------------------------*/

    return NULL;
}
#endif

/*------------------------------------------------------
  ELF�I�u�W�F�N�g����f�[�^��ǂݏo���X�^�u
 -----------------------------------------------------*/
static void i_elReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
#pragma unused( file_base)

#if (TARGET_OS_NITRO == 1)
    FS_SeekFile( file_struct, (s32)(file_offset), FS_SEEK_SET);
    FS_ReadFile( file_struct, buf, (s32)(size));
#else
    fsLseek( (int)file_struct, (s32)(file_offset), SEEK_SET);
    fsRead( (int)file_struct, buf, (s32)(size));
#endif
}

static void i_elReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
#pragma unused( file_struct)
  
    OSAPI_CPUCOPY8( (void*)(file_base + file_offset),
                    buf,
                    size);
}

//#if (TARGET_OS_NITRO == 1)
//#else
static void i_elReadUsr( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size)
{
#pragma unused( file_struct, file_base)
  
    i_elReadImage( file_offset, buf, size);
}
//#endif
