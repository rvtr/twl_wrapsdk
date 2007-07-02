/*---------------------------------------------------------------------------*
  Project:  CTR - ELF Loader
  File:     loader_subset.c

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
  #include <string.h>
#endif

#include "loader_subset.h"

extern ElUnresolvedEntry*    ELUnrEntStart;

static ElVeneer*             ELVenEntStart = NULL;
static ElVeneer*             ELV4tVenEntStart = NULL;


//ARM7���ǂ����𔻕ʂ���Ƃ��́A
//#ifdef  SDK_ARM7
//#endif    /*SDK_ARM7*/

/*------------------------------------------------------
  extern�ϐ�
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
#else
extern ElAlloc   i_elAlloc;
extern ElFree    i_elFree;
#endif


/*------------------------------------------------------
  static�֐�
 -----------------------------------------------------*/
static u32 el_veneer[3] = { //ARM�R�[�h
    0xE59FC000,    //(LDR r12,[PC])
    0xE12FFF1C,    //(BX  r12)
    0x00000000,    //(data)
};

static u16 el_veneer_t[10] = { //v4t�pThumb�R�[�h
    0xB580,  //(PUSH {R7,LR})
    0x46FE,  //(NOP)
    0x46FE,  //(MOV LR, PC)
    0x2707,  //(MOV R7, 7)
    0x44BE,  //(ADD LR,R7)
    0x4F01,  //(LDR R7,[PC+4])
    0x4738,  //(BX R7) ---> ARM routine
    0xBD80,  //(POP {R7,PC} <---LR(v4t�ł͖�������Thumb����)
    0x0000,  //(data) ARM routine address
    0x0000,  //(data)
};

static void     i_elBuildSymList( ElDesc* elElfDesc, u32 symsh_index);
static ElSymEx* i_elGetSymExfromList( ElSymEx* SymExStart, u32 index);
static ElSymEx* i_elGetSymExfromTbl( ElSymEx** SymExTbl, u32 index);
static void     i_elUnresolvedInfoInit( ElUnresolvedEntry* UnresolvedInfo);
static void     i_elAddUnresolvedEntry( ElUnresolvedEntry* UnrEnt);

static void      i_elAddVeneerEntry( ElVeneer** start, ElVeneer* VenEnt);
static BOOL      i_elRemoveVenEntry( ElVeneer** start, ElVeneer* VenEnt);
static ElVeneer* i_elGetVenEntry( ElVeneer** start, u32 data);
static BOOL      i_elIsFar( u32 P, u32 S, s32 threshold);


/*------------------------------------------------------
  �A�h���X���A���C�������g����
 -----------------------------------------------------*/
#define ALIGN( addr, align_size ) (((addr)  & ~((align_size) - 1)) + (align_size))

static u32 i_elALIGN( u32 addr, u32 align_size);
u32 i_elALIGN( u32 addr, u32 align_size)
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
  �x�j�����o�b�t�@�ɃR�s�[����
    start : �Ăяo����
    data : ��ѐ�
    threshold : ���͈͓̔��Ɋ��Ƀx�j��������Ύg���܂킷
 -----------------------------------------------------*/
void* i_elCopyVeneerToBuffer( ElDesc* elElfDesc, u32 start, u32 data, s32 threshold)
{
    u32*        el_veneer_dest;
    u32         load_start;
    Elf32_Addr  sh_size;
    ElVeneer*   elVenEnt;

    /*--- ���łɃx�j��������΂�����g�� ---*/
    elVenEnt = i_elGetVenEntry( &ELVenEntStart, data);
    if( elVenEnt != NULL) {
        if( i_elIsFar( start, elVenEnt->adr, threshold) == FALSE) {
            PRINTDEBUG( "Veneer Hit!\n");
            return( (void*)elVenEnt->adr);
        }else{
            /*�x�j�����������Ďg���Ȃ��̂Ń����N���X�g����폜*/
            i_elRemoveVenEntry( &ELVenEntStart, elVenEnt);
        }
    }
    /*--------------------------------------*/

    /*�A���C�������g���Ƃ�*/
    load_start = i_elALIGN( ((u32)(elElfDesc->buf_current)), 4);
    /*�T�C�Y�ݒ�*/
    sh_size = 12;//el_veneer

    /*�R�s�[*/
    OSAPI_CPUCOPY8( el_veneer, (u32*)load_start, sh_size);

    /*�o�b�t�@�|�C���^���ړ�*/
    elElfDesc->buf_current = (void*)(load_start + sh_size);
    el_veneer_dest = (u32*)load_start;
    el_veneer_dest[2] = data;

    /*--- �x�j�����X�g�ɒǉ� ---*/
    elVenEnt = OSAPI_MALLOC( sizeof( ElVeneer));
    elVenEnt->adr = load_start;
    elVenEnt->data = data;
    i_elAddVeneerEntry( &ELVenEntStart, elVenEnt);
    /*--------------------------*/

    /*���[�h�����擪�A�h���X��Ԃ�*/
    return (void*)load_start;
}


/*------------------------------------------------------
  �x�j�����o�b�t�@�ɃR�s�[����
    start : �Ăяo����
    data : ��ѐ�
    threshold : ���͈͓̔��Ɋ��Ƀx�j��������Ύg���܂킷
 -----------------------------------------------------*/
void* i_elCopyV4tVeneerToBuffer( ElDesc* elElfDesc, u32 start, u32 data, s32 threshold)
{
    u32*        el_veneer_dest;
    u32         load_start;
    Elf32_Addr  sh_size;
    ElVeneer*   elVenEnt;

    /*--- ���łɃx�j��������΂�����g�� ---*/
    elVenEnt = i_elGetVenEntry( &ELV4tVenEntStart, data);
    if( elVenEnt != NULL) {
        if( i_elIsFar( start, elVenEnt->adr, threshold) == FALSE) {
            PRINTDEBUG( "Veneer Hit!\n");
            return( (void*)elVenEnt->adr);
        }else{
            /*�x�j�����������Ďg���Ȃ��̂Ń����N���X�g����폜*/
            i_elRemoveVenEntry( &ELV4tVenEntStart, elVenEnt);
        }
    }
    /*--------------------------------------*/

    /*�A���C�������g���Ƃ�*/
    load_start = i_elALIGN( ((u32)(elElfDesc->buf_current)), 4);
    /*�T�C�Y�ݒ�*/
    sh_size = 20;//el_veneer_t

    /*�R�s�[*/
    OSAPI_CPUCOPY8( el_veneer_t, (u32*)load_start, sh_size);

    /*�o�b�t�@�|�C���^���ړ�*/
    elElfDesc->buf_current = (void*)(load_start + sh_size);
    el_veneer_dest = (u32*)load_start;
    el_veneer_dest[4] = data;

    /*--- �x�j�����X�g�ɒǉ� ---*/
    elVenEnt = OSAPI_MALLOC( sizeof( ElVeneer));
    elVenEnt->adr = load_start;
    elVenEnt->data = data;
    i_elAddVeneerEntry( &ELV4tVenEntStart, elVenEnt);
    /*--------------------------*/

    /*���[�h�����擪�A�h���X��Ԃ�*/
    return (void*)load_start;
}


/*------------------------------------------------------
  �Z�O�����g���o�b�t�@�ɃR�s�[����
 -----------------------------------------------------*/
void* i_elCopySegmentToBuffer( ElDesc* elElfDesc, Elf32_Phdr* Phdr)
{
    u32 load_start;
    
    load_start = i_elALIGN( Phdr->p_vaddr, Phdr->p_align);
    
    elElfDesc->i_elReadStub( (void*)load_start,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset)+(u32)(Phdr->p_offset),
                             Phdr->p_filesz);
    
    return( (void*)load_start);
}

/*------------------------------------------------------
  �Z�N�V�������o�b�t�@�ɃR�s�[����
 -----------------------------------------------------*/
void* i_elCopySectionToBuffer( ElDesc* elElfDesc, Elf32_Shdr* Shdr)
{
    u32         load_start;
    Elf32_Addr  sh_size;

    /*�A���C�������g���Ƃ�*/
    load_start = i_elALIGN( ((u32)(elElfDesc->buf_current)), (Shdr->sh_addralign));
    /*�T�C�Y�ݒ�*/
    sh_size = Shdr->sh_size;

    /*�R�s�[*/
    elElfDesc->i_elReadStub( (void*)load_start,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset)+(u32)(Shdr->sh_offset),
                             sh_size);

    /*�o�b�t�@�|�C���^���ړ�*/
    elElfDesc->buf_current = (void*)(load_start + sh_size);

    /*���[�h�����擪�A�h���X��Ԃ�*/
    return (void*)load_start;
}


/*------------------------------------------------------
  �Z�N�V�������o�b�t�@�Ɋm�ہi�R�s�[���Ȃ��j���A
  ���̗̈��0�Ŗ��߂�
 -----------------------------------------------------*/
void* i_elAllocSectionToBuffer( ElDesc* elElfDesc, Elf32_Shdr* Shdr)
{
    u32         load_start;
    Elf32_Addr  sh_size;

    /*�A���C�������g���Ƃ�*/
    load_start = i_elALIGN( ((u32)(elElfDesc->buf_current)), (Shdr->sh_addralign));
    /*�T�C�Y�ݒ�*/
    sh_size = Shdr->sh_size;

    /*�o�b�t�@�|�C���^���ړ�*/
    elElfDesc->buf_current = (void*)(load_start + sh_size);

    /*0�Ŗ��߂�i.bss�Z�N�V������z�肵�Ă��邽�߁j*/
    OSAPI_CPUFILL8( (void*)load_start, 0, sh_size);
    
    /*�m�ۂ����擪�A�h���X��Ԃ�*/
    return (void*)load_start;
}


/*------------------------------------------------------
  �w��C���f�b�N�X�̃v���O�����w�b�_���o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void i_elGetPhdr( ElDesc* elElfDesc, u32 index, Elf32_Phdr* Phdr)
{
    u32 offset;
    
    offset = (elElfDesc->CurrentEhdr.e_phoff) +
             ((u32)(elElfDesc->CurrentEhdr.e_phentsize) * index);
    
    elElfDesc->i_elReadStub( Phdr,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset) + offset,
                             sizeof( Elf32_Shdr));
}

/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�����w�b�_���o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void i_elGetShdr( ElDesc* elElfDesc, u32 index, Elf32_Shdr* Shdr)
{
    u32 offset;
    
    offset = (elElfDesc->CurrentEhdr.e_shoff) + ((u32)(elElfDesc->shentsize) * index);
    
    elElfDesc->i_elReadStub( Shdr,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset) + offset,
                             sizeof( Elf32_Shdr));
}

/*------------------------------------------------------
  �w��C���f�b�N�X�̃Z�N�V�����̃G���g�����o�b�t�@�Ɏ擾����
 -----------------------------------------------------*/
void i_elGetSent( ElDesc* elElfDesc, u32 index, void* entry_buf, u32 offset, u32 size)
{
    Elf32_Shdr  Shdr;

    i_elGetShdr( elElfDesc, index, &Shdr);
    
    elElfDesc->i_elReadStub( entry_buf,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset) + (u32)(Shdr.sh_offset) + offset,
                             size);
}


/*------------------------------------------------------
  �w��Z�N�V�����w�b�_�̎w��C���f�b�N�X�̃G���g�����o�b�t�@�Ɏ擾����
    �iRel,Rela,Sym�ȂǃG���g���T�C�Y���Œ�̃Z�N�V�����̂݁j
    Shdr : �w��Z�N�V�����w�b�_
    index : �G���g���ԍ�(0�`)
 -----------------------------------------------------*/
void i_elGetEntry( ElDesc* elElfDesc, Elf32_Shdr* Shdr, u32 index, void* entry_buf)
{
    u32 offset;

    offset = (u32)(Shdr->sh_offset) + ((Shdr->sh_entsize) * index);
    
    elElfDesc->i_elReadStub( entry_buf,
                             elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head),
                             (u32)(elElfDesc->elf_offset) + offset,
                             Shdr->sh_entsize);
}

/*------------------------------------------------------
  STR�Z�N�V�����w�b�_�̎w��C���f�b�N�X�̕�������擾����

    Shdr : �w��Z�N�V�����w�b�_
    index : �G���g���C���f�b�N�X(0�`)
 -----------------------------------------------------*/
void i_elGetStrAdr( ElDesc* elElfDesc, u32 strsh_index, u32 ent_index, char* str, u32 len)
{
    /*������G���g���̐擪�A�h���X*/
    i_elGetSent( elElfDesc, strsh_index, str, ent_index, len);
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
void i_elRelocateSym( ElDesc* elElfDesc, u32 relsh_index)
{
    u32                 i;
    u32                 num_of_rel;     //�Ē�`���ׂ��V���{���̐�
    Elf32_Shdr          RelOrRelaShdr;  //REL�܂���RELA�Z�N�V�����w�b�_
    Elf32_Rela          CurrentRela;    //REL�܂���RELA�G���g���̃R�s�[��
    Elf32_Shdr*         SymShdr;
    ElSymEx*            CurrentSymEx;
    ElShdrEx*           TargetShdrEx;
    ElShdrEx*           CurrentShdrEx;
    u32                 relocation_adr;
    char                sym_str[128];
    u32                 copy_size;
    ElAdrEntry*         CurrentAdrEntry;
    u32                 sym_loaded_adr = 0;
    u32                 thumb_func_flag = 0;
    ElUnresolvedEntry   UnresolvedInfo;
    ElUnresolvedEntry*  UnrEnt;
    u32                 unresolved_num = 0;

    /*REL or RELA�Z�N�V�����w�b�_�擾*/
    i_elGetShdr( elElfDesc, relsh_index, &RelOrRelaShdr);

    /*�^�[�Q�b�g�Z�N�V������EX�w�b�_*/
    TargetShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx, RelOrRelaShdr.sh_info);
    
    num_of_rel = (RelOrRelaShdr.sh_size) / (RelOrRelaShdr.sh_entsize);    //�Ē�`���ׂ��V���{���̐�

    /*�V���{�����X�g���쐬����*/
    i_elBuildSymList( elElfDesc, RelOrRelaShdr.sh_link);
    /*i_elBuildSymList���ĂԂ�elElfDesc->SymShdr���Z�b�g�����*/
    SymShdr = &(elElfDesc->SymShdr);
    PRINTDEBUG( "SymShdr->link:%02x, SymShdr->info:%02x\n", SymShdr->sh_link, SymShdr->sh_info);


    /*--- �Ē�`���K�v�ȃV���{���̍Ē�` ---*/
    for( i=0; i<num_of_rel; i++) {
        
        /*- Rel�܂���Rela�G���g���擾 -*/
        i_elGetEntry( elElfDesc, &RelOrRelaShdr, i, &CurrentRela);
        
        if( RelOrRelaShdr.sh_type == SHT_REL) {
            CurrentRela.r_addend = 0;
        }
        /*-----------------------------*/

        /*�V���{��Ex�G���g���擾(i_elGetSymExfromTbl�ł��悢)*/
        CurrentSymEx = (ElSymEx*)(elElfDesc->SymExTbl[ELF32_R_SYM( CurrentRela.r_info)]);

//        if( CurrentSymEx->debug_flag == 1) {            /*�f�o�b�O���̏ꍇ*/
        if( CurrentSymEx == NULL) {
        }else{                                            /*�f�o�b�O���łȂ��ꍇ*/
            /**/
            i_elUnresolvedInfoInit( &UnresolvedInfo);
            /*����������A�h���X�i�d�l���ł����uP�v�j*/
            relocation_adr = (TargetShdrEx->loaded_adr) + (CurrentRela.r_offset);
            UnresolvedInfo.r_type = ELF32_R_TYPE( CurrentRela.r_info);
            UnresolvedInfo.A_ = (CurrentRela.r_addend);
            UnresolvedInfo.P_ = (relocation_adr);
            UnresolvedInfo.sh_type = (RelOrRelaShdr.sh_type);
            
            /*�V���{���̃A�h���X��˂��~�߂�*/
            if( CurrentSymEx->Sym.st_shndx == SHN_UNDEF) {
                /*�A�h���X�e�[�u�����猟��*/
                i_elGetStrAdr( elElfDesc, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
                CurrentAdrEntry = elGetAdrEntry( sym_str);
                if( CurrentAdrEntry) {
                    sym_loaded_adr = (u32)(CurrentAdrEntry->adr);
                    /*THUMB�֐��t���O�i�d�l���ł����uT�v�jTHUMB or ARM �̔���*/
                    thumb_func_flag = CurrentAdrEntry->thumb_flag;
                    PRINTDEBUG( "\n symbol found %s : %8x\n", sym_str, sym_loaded_adr);
                }else{
                    /*������Ȃ��������̓G���[(S_��T_�������ł��Ȃ�)*/
                    copy_size = (u32)OSAPI_STRLEN( sym_str) + 1;
                    UnresolvedInfo.sym_str = OSAPI_MALLOC( copy_size);
                    OSAPI_CPUCOPY8( sym_str, UnresolvedInfo.sym_str, copy_size);

                    /*�O���[�o���Ȗ������e�[�u���ɒǉ�*/
                    copy_size = sizeof( ElUnresolvedEntry);
                    UnrEnt = OSAPI_MALLOC( copy_size);
                    OSAPI_CPUCOPY8( &UnresolvedInfo, UnrEnt, copy_size);
                    i_elAddUnresolvedEntry( UnrEnt);

                    unresolved_num++;    /*�������V���{�������J�E���g*/
                    PRINTDEBUG( "\n WARNING! cannot find symbol : %s\n", sym_str);
                }
            }else{
                /*�V���{���̏�������Z�N�V������Ex�w�b�_�擾*/
                CurrentShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx, CurrentSymEx->Sym.st_shndx);
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
                /*CurrentSymEx->relocation_val =*/ i_elDoRelocate( elElfDesc,
                                                                   &UnresolvedInfo);
                /*------------------------------------------------------------*/
            }
        }
    }
    /*-----------------------------------*/


    /*�V���{�����X�g���J������*/
//    i_elFreeSymList( elElfDesc);

    
    /* �Ĕz�u������ */
    if( unresolved_num == 0) {
        elElfDesc->process = EL_RELOCATED;
    }
}


/*------------------------------------------------------
  �O���[�o���V���{�����A�h���X�e�[�u���ɓ����

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
void i_elGoPublicGlobalSym( ElDesc* elElfDesc, u32 symtblsh_index)
{
    Elf32_Shdr   SymShdr;
    ElSymEx*     CurrentSymEx;
    ElShdrEx*    CurrentShdrEx;
    ElAdrEntry*  ExportAdrEntry;
    char         sym_str[128];
    u32          copy_size;
    
    /*SYMTAB�Z�N�V�����w�b�_�擾*/
    i_elGetShdr( elElfDesc, symtblsh_index, &SymShdr);
    
    /*�V���{�����X�g���쐬����*/
    i_elBuildSymList( elElfDesc, symtblsh_index);
    
    /*--- ���C�u��������GLOBAL�V���{�����A�h���X�e�[�u���Ɍ��J���� ---*/
    CurrentSymEx = elElfDesc->SymEx; //Tbl����łȂ��A�������炽�ǂ��debug���͊܂܂�Ȃ�
    while( CurrentSymEx != NULL) {
//        CurrentSymEx = i_elGetSymExfromList( elElfDesc->SymExTbl, i);
//        if( CurrentSymEx != NULL) {
        /*GLOBAL�ŁA���֘A����Z�N�V���������C�u�������ɑ��݂���ꍇ*/
        if( (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_GLOBAL) &&
            (CurrentSymEx->Sym.st_shndx != SHN_UNDEF)) {
            
            ExportAdrEntry = OSAPI_MALLOC( sizeof(ElAdrEntry));    /*�������m��*/
            
            ExportAdrEntry->next = NULL;
            
            i_elGetStrAdr( elElfDesc, SymShdr.sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
            copy_size = (u32)OSAPI_STRLEN( sym_str) + 1;
            ExportAdrEntry->name = OSAPI_MALLOC( copy_size);
            OSAPI_CPUCOPY8( sym_str, ExportAdrEntry->name, copy_size);

            CurrentShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx, CurrentSymEx->Sym.st_shndx);
            //Sym.st_value�͋���/���ARM/Thumb�𔻕ʂł���悤�ɒ�������Ă���ꍇ������̂ŁA���̒������폜���Đ����̒l���o��
            ExportAdrEntry->adr = (void*)(CurrentShdrEx->loaded_adr + ((CurrentSymEx->Sym.st_value)&0xFFFFFFFE));
            ExportAdrEntry->func_flag = (u16)(ELF32_ST_TYPE( CurrentSymEx->Sym.st_info));
            ExportAdrEntry->thumb_flag = CurrentSymEx->thumb_flag;

            if( elGetAdrEntry( ExportAdrEntry->name) == NULL) {    //�����ĂȂ�������
                elAddAdrEntry( ExportAdrEntry);    /*�o�^*/
            }
        }
        CurrentSymEx = CurrentSymEx->next;
//        }
    }
    /*----------------------------------------------------------------*/

    /*�V���{�����X�g���J������*/
//    i_elFreeSymList( elElfDesc);
}


/*------------------------------------------------------
  �V���{�����X�g���쐬����

 -----------------------------------------------------*/
static void i_elBuildSymList( ElDesc* elElfDesc, u32 symsh_index)
{
    u32         i;
    u32         num_of_sym;        //�V���{���̑S�̐�
    u16         debug_flag;
    Elf32_Sym   TestSym;
    ElSymEx*    CurrentSymEx;
    ElShdrEx*   CurrentShdrEx;
    Elf32_Shdr* SymShdr;
    ElSymEx     DmySymEx;

    if( elElfDesc->SymExTarget == symsh_index) {
        PRINTDEBUG( "%s skip.\n", __FUNCTION__);
        return;                              //���Ƀ��X�g�쐬�ς�
    }else{
        i_elFreeSymList( elElfDesc); /*�V���{�����X�g���J������*/
    }
    
    PRINTDEBUG( "%s build\n", __FUNCTION__);
    
    /*SYMTAB�Z�N�V�����w�b�_�擾*/
    i_elGetShdr( elElfDesc, symsh_index, &(elElfDesc->SymShdr));
    SymShdr = &(elElfDesc->SymShdr);
    
    num_of_sym = (SymShdr->sh_size) / (SymShdr->sh_entsize);    //�V���{���̑S�̐�
    elElfDesc->SymExTbl = OSAPI_MALLOC( num_of_sym * 4);
    
    /*---------- ElSymEx�̃��X�g����� ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        
        /*�V���{���G���g�����R�s�[*/
        i_elGetEntry( elElfDesc, (Elf32_Shdr*)SymShdr, i, &TestSym);
        /*-- �f�o�b�O���t���O���Z�b�g --*/
        CurrentShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx, TestSym.st_shndx);
        if( CurrentShdrEx) {
            debug_flag = CurrentShdrEx->debug_flag;
        }else{
            debug_flag = 0;
        }/*-------------------------------*/

        if( debug_flag == 1) {
            elElfDesc->SymExTbl[i] = NULL;
        }else{
            CurrentSymEx->next = OSAPI_MALLOC( sizeof(ElSymEx));
            CurrentSymEx = (ElSymEx*)(CurrentSymEx->next);
            
            OSAPI_CPUCOPY8( &TestSym, &(CurrentSymEx->Sym), sizeof(TestSym));
            
            elElfDesc->SymExTbl[i] = CurrentSymEx;
            
            PRINTDEBUG( "sym_no: %02x ... st_shndx: %04x\n", i, CurrentSymEx->Sym.st_shndx);
        }
    }
    
    CurrentSymEx->next = NULL;
    elElfDesc->SymEx = DmySymEx.next;
    /*-------------------------------------------*/
    

    /*-------- ARM or Thumb ����(i_elCodeIsThumb��SymEx���o���ĂȂ��Ɠ����Ȃ�����) --------*/
    CurrentSymEx = elElfDesc->SymEx;
    while( CurrentSymEx != NULL) { //�������炽�ǂ��debug���͖���
        /*-- ElSymEx��Thumb�t���O���Z�b�g�i�֐��V���{�������K�v�j--*/
        if( ELF32_ST_TYPE( CurrentSymEx->Sym.st_info) == STT_FUNC) {
            CurrentSymEx->thumb_flag = (u16)(i_elCodeIsThumb( elElfDesc,
                                                              CurrentSymEx->Sym.st_shndx,
                                                              CurrentSymEx->Sym.st_value));
        }else{
            CurrentSymEx->thumb_flag = 0;
        }/*--------------------------------------------------------*/
        
        CurrentSymEx = CurrentSymEx->next;
    }
    /*-------------------------------------------------------------------------------------*/


    elElfDesc->SymExTarget = symsh_index;
}


/*------------------------------------------------------
  �V���{�����X�g���J������

 -----------------------------------------------------*/
void i_elFreeSymList( ElDesc* elElfDesc)
{
    ElSymEx*  CurrentSymEx;
    ElSymEx*  FwdSymEx;

    if( elElfDesc->SymExTbl != NULL) {
        OSAPI_FREE( elElfDesc->SymExTbl);
        elElfDesc->SymExTbl = NULL;
    }
    
    /*------- ElSymEx�̃��X�g��������� -------*/
    CurrentSymEx = elElfDesc->SymEx;
    if( CurrentSymEx) {
        while( CurrentSymEx->next != NULL) {
            FwdSymEx = CurrentSymEx;
            CurrentSymEx = CurrentSymEx->next;
            OSAPI_FREE( FwdSymEx);
        }
        elElfDesc->SymEx = NULL;
    }
    /*-----------------------------------------*/


    elElfDesc->SymExTarget = 0xFFFFFFFF;
}


/*------------------------------------------------------
  �������������ƂɃV���{������������

    r_type,S_,A_,P_,T_���S�ĕ������Ă���K�v������B
 -----------------------------------------------------*/
#define _S_    (UnresolvedInfo->S_)
#define _A_    (UnresolvedInfo->A_)
#define _P_    (UnresolvedInfo->P_)
#define _T_    (UnresolvedInfo->T_)
BOOL    i_elDoRelocate( ElDesc* elElfDesc, ElUnresolvedEntry* UnresolvedInfo)
{
    s32    signed_val;
    u32    relocation_val = 0;
    u32    relocation_adr;
    BOOL   ret_val;
    BOOL   veneer_flag = TRUE;

    ret_val = TRUE;
    relocation_adr = _P_;

    switch( (UnresolvedInfo->r_type)) {
      case R_ARM_PC24:
      case R_ARM_PLT32:
      case R_ARM_CALL:
      case R_ARM_JUMP24:
        if( UnresolvedInfo->sh_type == SHT_REL) {
            _A_ = (s32)(((*(vu32*)relocation_adr)|0xFF800000) << 2);    //��ʓI�ɂ�-8�ɂȂ��Ă���͂�
        }
        /*----------------- veneer -----------------*/
        veneer_flag = i_elIsFar( _P_, _S_|_T_, 0x2000000); //+-32MBytes����?
#if (TARGET_ARM_V5 == 1)
        if( veneer_flag == TRUE) {
#else //(TARGET_ARM_V4 == 1)
        //ARM->Thumb�����ARM->ARM�Ń����O�u�����`�̂Ƃ��� veneer �g�p
        if( (_T_) || (veneer_flag == TRUE)) {
#endif
            //_A_��PC���Ε���̂Ƃ��̒����l�Ȃ̂ŁA��Ε���̏ꍇ�͊֌W�Ȃ�
            _S_ = (u32)i_elCopyVeneerToBuffer( elElfDesc, _P_,
                                               (u32)(( (s32)(_S_) /*+ _A_*/) | (s32)(_T_)),
                                               0x2000000);
            _T_ = 0; //veneer��ARM�R�[�h�̂���
        }/*-----------------------------------------*/
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
      case R_ARM_PREL31:
        relocation_val = (( _S_ + _A_) | _T_) - _P_;
        *(vu32*)relocation_adr = relocation_val;
        break;
      case R_ARM_LDR_PC_G0:
        /*----------------- veneer -----------------*/
        veneer_flag = i_elIsFar( _P_, _S_|_T_, 0x2000000); //+-32MBytes����?
#if (TARGET_ARM_V5 == 1)
        if( veneer_flag == TRUE) {
#else //(TARGET_ARM_V4 == 1)
        //ARM->Thumb�����ARM->ARM�Ń����O�u�����`�̂Ƃ��� veneer �g�p
        if( (_T_) || (veneer_flag == TRUE)) {
#endif
            //_A_��PC���Ε���̂Ƃ��̒����l�Ȃ̂ŁA��Ε���̏ꍇ�͊֌W�Ȃ�
            _S_ = (u32)i_elCopyVeneerToBuffer( elElfDesc, _P_,
                                               (u32)(( (s32)(_S_) /*+ _A_*/) | (s32)(_T_)),
                                               0x2000000);
            _T_ = 0; //veneer��ARM�R�[�h�̂���
        }/*-----------------------------------------*/
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
            _A_ = (s32)(((*(vu16*)relocation_adr & 0x07FF)<<11) + ((*((vu16*)(relocation_adr)+1)) & 0x07FF));
            _A_ = (s32)((_A_ | 0xFFC00000) << 1);    //��ʓI�ɂ�-4�ɂȂ��Ă���͂�(PC�͌����߃A�h���X+4�Ȃ̂�)
        }
        /*----------------- veneer -----------------*/
        veneer_flag = i_elIsFar( _P_, _S_|_T_, 0x400000); //+-4MBytes����?
#if (TARGET_ARM_V5 == 1)
        if( veneer_flag == TRUE) {
            //_A_��PC���Ε���̂Ƃ��̒����l�Ȃ̂ŁA��Ε���̏ꍇ�͊֌W�Ȃ�
            _S_ = (u32)i_elCopyVeneerToBuffer( elElfDesc, _P_,
                                               (u32)(( (s32)(_S_) /*+ _A_*/) | (s32)(_T_)),
                                               0x400000);
            _T_ = 0; //veneer��ARM�R�[�h�̂���
        }/*-----------------------------------------*/
#else //(TARGET_ARM_V4 == 1)
        /*----------------- v4t veneer -----------------*/
        //Thumb->ARM�����Thumb->Thumb�Ń����O�u�����`�̂Ƃ��� v4T veneer �g�p
        if( (_T_ == 0) || (veneer_flag == TRUE)) {
            _S_ = (u32)i_elCopyV4tVeneerToBuffer( elElfDesc, _P_,
                                                  (u32)(( (s32)(_S_)) | (s32)(_T_)),
                                                  0x400000);
            _T_ = 1; //v4t veneer��Thumb�R�[�h�̂���
        }
        /*---------------------------------------------*/
#endif
        signed_val = (( (s32)(_S_) + _A_) | (s32)(_T_)) - (s32)(_P_);
        signed_val >>= 1;
        if( _T_) {    /*BL���߂�Thumb����Thumb�ɔ��*/
            relocation_val = (*(vu16*)relocation_adr & 0xF800) | ((signed_val>>11) & 0x07FF) +
                                   ((((*((vu16*)(relocation_adr)+1)) & 0xF800) | (signed_val & 0x07FF)) << 16);
        }else{        /*BLX���߂�Thumb����ARM�ɔ��(v5��������BL���x�j�A��BX�Ƃ����d�g�݂��K�v)*/
            if( (signed_val & 0x1)) {    //_P_��4�o�C�g�A���C������Ă��Ȃ��Ƃ����ɗ���
                signed_val += 1;
            }
            relocation_val = (*(vu16*)relocation_adr & 0xF800) | ((signed_val>>11) & 0x07FF) +
                                   ((((*((vu16*)(relocation_adr)+1)) & 0xE800) | (signed_val & 0x07FF)) << 16);
        }
        *(vu16*)relocation_adr = (vu16)relocation_val;
        *((vu16*)relocation_adr+1) = (vu16)((u32)(relocation_val) >> 16);
        break;
      case R_ARM_NONE:
        /*R_ARM_NONE�̓^�[�Q�b�g�̃V���{����ێ����A�����J�Ƀf�b�h�X�g���b�v
          ����Ȃ��悤�ɂ��܂�*/
        break;
      case R_ARM_THM_JUMP24:
      default:
        ret_val = FALSE;
        PRINTDEBUG( "ERROR! : unsupported relocation type (0x%x)!\n", (UnresolvedInfo->r_type));
        PRINTDEBUG( "S = 0x%x\n", _S_);
        PRINTDEBUG( "A = 0x%x\n", _A_);
        PRINTDEBUG( "P = 0x%x\n", _P_);
        PRINTDEBUG( "T = 0x%x\n", _T_);
        break;
    }
    
    return ret_val;//relocation_val;
}
#undef _S_
#undef _A_
#undef _P_
#undef _T_

/*------------------------------------------------------
  ���X�g����w��C���f�b�N�X��ElSymEx�����o��
 -----------------------------------------------------*/
static ElSymEx* i_elGetSymExfromList( ElSymEx* SymExStart, u32 index)
{
    u32         i;
    ElSymEx*    SymEx;

    SymEx = SymExStart;
    for( i=0; i<index; i++) {
        SymEx = (ElSymEx*)(SymEx->next);
        if( SymEx == NULL) {
            break;
        }
    }
    return SymEx;
}

/*------------------------------------------------------
  �e�[�u������w��C���f�b�N�X��ElSymEx�����o��
 -----------------------------------------------------*/
static ElSymEx* i_elGetSymExfromTbl( ElSymEx** SymExTbl, u32 index)
{
    return( (ElSymEx*)(SymExTbl[index]));
}

/*------------------------------------------------------
  ���X�g����w��C���f�b�N�X��ElShdrEx�����o��
 -----------------------------------------------------*/
ElShdrEx* i_elGetShdrExfromList( ElShdrEx* ShdrExStart, u32 index)
{
    u32         i;
    ElShdrEx*   ShdrEx;

    ShdrEx = ShdrExStart;
    for( i=0; i<index; i++) {
        ShdrEx = (ElShdrEx*)(ShdrEx->next);
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
BOOL i_elShdrIsDebug( ElDesc* elElfDesc, u32 index)
{
    Elf32_Shdr  TmpShdr;
    char        shstr[6];

    /*-- �Z�N�V�������̕������6�����擾 --*/
    i_elGetShdr( elElfDesc, index, &TmpShdr);
    i_elGetStrAdr( elElfDesc, elElfDesc->CurrentEhdr.e_shstrndx,
                   TmpShdr.sh_name, shstr, 6);
    /*-------------------------------------*/
    
    if( OSAPI_STRNCMP( shstr, ".debug", 6) == 0) {    /*�f�o�b�O�Z�N�V�����̏ꍇ*/
        return TRUE;
    }else{                        /*�f�o�b�O�Z�N�V�����Ɋւ���Ĕz�u�Z�N�V�����̏ꍇ*/
        if( (TmpShdr.sh_type == SHT_REL) || (TmpShdr.sh_type == SHT_RELA)) {
            if( i_elShdrIsDebug( elElfDesc, TmpShdr.sh_info) == TRUE) {
                return TRUE;
            }
        }
    }

    return FALSE;
}



/*------------------------------------------------------
  elElfDesc��SymEx�e�[�u���𒲂ׁA�w��C���f�b�N�X��
�@�w��I�t�Z�b�g�ɂ���R�[�h��ARM��THUMB���𔻒肷��B
  �i�\�� elElfDesc->SymShdr �� elElfDesc->SymEx ��ݒ肵�Ă������Ɓj
    
    sh_index : ���ׂ����Z�N�V�����C���f�b�N�X
    offset : ���ׂ����Z�N�V�������̃I�t�Z�b�g
 -----------------------------------------------------*/
u32 i_elCodeIsThumb( ElDesc* elElfDesc, u16 sh_index, u32 offset)
{
    u32            i;
    u32            thumb_flag;
    Elf32_Shdr*    SymShdr;
    char           str_adr[3];
    ElSymEx*       CurrentSymEx;

    /*�V���{���̃Z�N�V�����w�b�_��SymEx���X�g�擾*/
    SymShdr = &(elElfDesc->SymShdr);
    CurrentSymEx = elElfDesc->SymEx;

    i = 0;
    thumb_flag = 0;
    while( CurrentSymEx != NULL) {
        
        if( CurrentSymEx->Sym.st_shndx == sh_index) {
            i_elGetStrAdr( elElfDesc, SymShdr->sh_link, CurrentSymEx->Sym.st_name, str_adr, 3);
            if( OSAPI_STRNCMP( str_adr, "$a\0", OSAPI_STRLEN("$a\0")) == 0) {
                thumb_flag = 0;
            }else if( OSAPI_STRNCMP( str_adr, "$t\0", OSAPI_STRLEN("$t\0")) == 0) {
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
static void i_elUnresolvedInfoInit( ElUnresolvedEntry* UnresolvedInfo)
{
    UnresolvedInfo->sym_str = NULL;
    UnresolvedInfo->r_type = 0;
    UnresolvedInfo->S_ = 0;
    UnresolvedInfo->A_ = 0;
    UnresolvedInfo->P_ = 0;
    UnresolvedInfo->T_ = 0;
    UnresolvedInfo->remove_flag = 0;
}

/*---------------------------------------------------------
 ���������e�[�u���ɃG���g����ǉ�����
 --------------------------------------------------------*/
static void i_elAddUnresolvedEntry( ElUnresolvedEntry* UnrEnt)
{
    ElUnresolvedEntry    DmyUnrEnt;
    ElUnresolvedEntry*   CurrentUnrEnt;

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


/*---------------------------------------------------------
 �x�j���e�[�u���ɃG���g����ǉ�����
 --------------------------------------------------------*/
static void i_elAddVeneerEntry( ElVeneer** start, ElVeneer* VenEnt)
{
    ElVeneer    DmyVenEnt;
    ElVeneer*   CurrentVenEnt;

    if( !(*start)) {
        (*start) = VenEnt;
    }else{
        DmyVenEnt.next = (*start);
        CurrentVenEnt = &DmyVenEnt;

        while( CurrentVenEnt->next != NULL) {
            CurrentVenEnt = CurrentVenEnt->next;
        }
        CurrentVenEnt->next = (void*)VenEnt;
    }
    VenEnt->next = NULL;
}

/*------------------------------------------------------
  �x�j���e�[�u������G���g�����폜����
 -----------------------------------------------------*/
static BOOL i_elRemoveVenEntry( ElVeneer** start, ElVeneer* VenEnt)
{
    ElVeneer  DmyVenEnt;
    ElVeneer* CurrentVenEnt;

    DmyVenEnt.next = (*start);
    CurrentVenEnt = &DmyVenEnt;

    while( CurrentVenEnt->next != VenEnt) {
        if( CurrentVenEnt->next == NULL) {
            return FALSE;
        }else{
            CurrentVenEnt = (ElVeneer*)CurrentVenEnt->next;
        }
    }

    /*�����N���X�g�̌q������*/
    CurrentVenEnt->next = VenEnt->next;
    (*start) = DmyVenEnt.next;

    /*�J��*/
    OSAPI_FREE( VenEnt);
    
     return TRUE;
}

/*------------------------------------------------------
  �x�j���e�[�u������w��f�[�^�ɊY������G���g����T��
 -----------------------------------------------------*/
static ElVeneer* i_elGetVenEntry( ElVeneer** start, u32 data)
{
    ElVeneer* CurrentVenEnt;

    CurrentVenEnt = (*start);
    if( CurrentVenEnt == NULL) {
        return NULL;
    }
    while( CurrentVenEnt->data != data) {
        CurrentVenEnt = (ElVeneer*)CurrentVenEnt->next;
        if( CurrentVenEnt == NULL) {
            break;
        }
    }
    return CurrentVenEnt;
}

/*------------------------------------------------------
  �x�j���e�[�u�����������
 -----------------------------------------------------*/
void* i_elFreeVenTbl( void)
{
    ElVeneer*    FwdVenEnt;
    ElVeneer*    CurrentVenEnt;
    
    /*--- ElVenEntry�̃��X�g��������� ---*/
    CurrentVenEnt = ELVenEntStart;
    while( CurrentVenEnt != NULL) {
        FwdVenEnt = CurrentVenEnt;
        CurrentVenEnt = CurrentVenEnt->next;
        OSAPI_FREE( FwdVenEnt);              //�\���̎��g
    }
    ELVenEntStart = NULL;
    /*------------------------------------*/

    /*--- ElVenEntry�̃��X�g��������� ---*/
    CurrentVenEnt = ELV4tVenEntStart;
    while( CurrentVenEnt != NULL) {
        FwdVenEnt = CurrentVenEnt;
        CurrentVenEnt = CurrentVenEnt->next;
        OSAPI_FREE( FwdVenEnt);              //�\���̎��g
    }
    ELV4tVenEntStart = NULL;
    /*------------------------------------*/
    
    return NULL;
}

/*------------------------------------------------------
  2�̃A�h���X�l�̍��� (+threshold)�`(-threshold)����
  ���܂��Ă��邩�𒲂ׂ�
 -----------------------------------------------------*/
static BOOL i_elIsFar( u32 P, u32 S, s32 threshold)
{
    s32 diff;

    diff = (s32)S - (s32)(P);
    if( (diff < threshold)&&( diff > -threshold)) {
        return( FALSE); //Near
    }else{
        return( TRUE);  //Far
    }
}

