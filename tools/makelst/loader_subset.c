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


//ARM7かどうかを判別するときは、
//#ifdef  SDK_ARM7
//#endif    /*SDK_ARM7*/


/*------------------------------------------------------
  アドレスをアラインメントする
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
  セクションをバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32         load_start;
    Elf32_Addr    sh_size;

    /*アラインメントをとる*/
//    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), (Shdr->sh_addralign));
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    /*サイズ設定*/
    sh_size = Shdr->sh_size;

    /*コピー*/
    ElfHandle->ELi_ReadStub( (void*)load_start,
                             ElfHandle->FileStruct,
                             (u32)(ElfHandle->ar_head),
                             (u32)(ElfHandle->elf_offset)+(u32)(Shdr->sh_offset),
                             sh_size);

    /*バッファポインタを移動*/
    ElfHandle->buf_current = (void*)(load_start + sh_size);

    /*ロードした先頭アドレスを返す*/
    return (void*)load_start;
}


/*------------------------------------------------------
  セクションをバッファに確保（コピーしない）し、
  その領域を0で埋める
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32            load_start;
    Elf32_Addr    sh_size;

    /*アラインメントをとる*/
//    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), (Shdr->sh_addralign));
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    /*サイズ設定*/
    sh_size = Shdr->sh_size;

    /*バッファポインタを移動*/
    ElfHandle->buf_current = (void*)(load_start + sh_size);

    /*0で埋める（.bssセクションを想定しているため）*/
    memset( (void*)load_start, 0, sh_size);
    
    /*確保した先頭アドレスを返す*/
    return (void*)load_start;
}


/*------------------------------------------------------
  指定インデックスのセクションヘッダをバッファに取得する
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
  指定インデックスのセクションのエントリをバッファに取得する
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
  指定セクションヘッダの指定インデックスのエントリをバッファに取得する
    （Rel,Rela,Symなどエントリサイズが固定のセクションのみ）
    Shdr : 指定セクションヘッダ
    index : エントリ番号(0～)
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
  STRセクションヘッダの指定インデックスの文字列を取得する

    Shdr : 指定セクションヘッダ
    index : エントリインデックス(0～)
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELHandle* ElfHandle, u32 strsh_index, u32 ent_index, char* str, u32 len)
{
    /*文字列エントリの先頭アドレス*/
    ELi_GetSent( ElfHandle, strsh_index, str, ent_index, len);
}

/*------------------------------------------------------
  シンボルを再定義する

    <Relセクションヘッダ>
    RelShdr->sh_link : シンボルセクション番号
    RelShdr->sh_info : ターゲットセクション番号(例：rel.textの.textを表す)
    
    <Relエントリ>
    Rel->r_offset : ターゲットセクション内のオフセットアドレス
    ELF32_R_SYM( Rel->r_info) : シンボルエントリ番号
    ELF32_R_TYPE( Rel->r_info) : リロケートタイプ

    <Symセクションヘッダ>
    SymShdr->sh_link : シンボルの文字列テーブルセクション番号
    
    <Symエントリ>
    Sym->st_name : シンボルの文字列エントリ番号
    Sym->st_value : シンボルの所属するセクション内のオフセットアドレス
    Sym->st_size : シンボルの所属するセクション内でのサイズ
    Sym->st_shndx : シンボルの所属するセクションの番号
    ELF32_ST_BIND( Sym->st_info) : バインド(LOCAL or GLOBAL)
    ELF32_ST_TYPE( Sym->st_info) : タイプ(シンボルが関連付けられている対象)
 -----------------------------------------------------*/
void ELi_RelocateSym( ELHandle* ElfHandle, u32 relsh_index)
{
    u32                    i;
    u32                    num_of_sym;        //シンボルの全体数
    u32                    num_of_rel;        //再定義すべきシンボルの数
    Elf32_Shdr             RelOrRelaShdr;    //RELまたはRELAセクションヘッダ
    Elf32_Rela            CurrentRela;    //RELまたはRELAエントリのコピー先
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

    /*REL or RELAセクションヘッダ取得*/
    ELi_GetShdr( ElfHandle, relsh_index, &RelOrRelaShdr);

    /*RELセクションとSYMセクションは1対1対応*/
    ELi_GetShdr( ElfHandle, RelOrRelaShdr.sh_link, &(ElfHandle->SymShdr));
    SymShdr = &(ElfHandle->SymShdr);
    if( dbg_print_flag == 1) {
        printf( "SymShdr->link:%02x, SymShdr->info:%02x\n",
                (int)(SymShdr->sh_link), (int)(SymShdr->sh_info));
    }

    /*ターゲットセクションのEXヘッダ*/
    TargetShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, RelOrRelaShdr.sh_info);
    
    num_of_rel = (RelOrRelaShdr.sh_size) / (RelOrRelaShdr.sh_entsize);    //再定義すべきシンボルの数
    num_of_sym = (SymShdr->sh_size) / (SymShdr->sh_entsize);    //シンボルの全体数

    /*---------- ELSymExのリストを作る ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx->next = (void*)(malloc( sizeof(ELSymEx)));
        CurrentSymEx = (ELSymEx*)(CurrentSymEx->next);
        
        /*シンボルエントリをコピー*/
        ELi_GetEntry( ElfHandle, SymShdr, i, &(CurrentSymEx->Sym));
        
        /*デバッグ情報フラグをセット*/
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

    /*----- ELSymExのThumbフラグをセット（関数シンボルだけ必要）-----*/
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

    /*--- 再定義が必要なシンボルの再定義 ---*/
    for( i=0; i<num_of_rel; i++) {
        
        /*- RelまたはRelaエントリ取得 -*/
        ELi_GetEntry( ElfHandle, &RelOrRelaShdr, i, &CurrentRela);
        
        if( RelOrRelaShdr.sh_type == SHT_REL) {
            CurrentRela.r_addend = 0;
        }
        /*-----------------------------*/

        /*シンボルExエントリ取得*/
        CurrentSymEx = ELi_GetSymExfromList( ElfHandle->SymEx,
                                             ELF32_R_SYM( CurrentRela.r_info));

        if( CurrentSymEx->debug_flag == 1) {            /*デバッグ情報の場合*/
        }else{                                            /*デバッグ情報でない場合*/
            /**/
            ELi_UnresolvedInfoInit( &UnresolvedInfo);
            /*書き換えるアドレス（仕様書でいう「P」）*/
            relocation_adr = (TargetShdrEx->loaded_adr) + (CurrentRela.r_offset);
            UnresolvedInfo.r_type = ELF32_R_TYPE( CurrentRela.r_info);
            UnresolvedInfo.A_ = (CurrentRela.r_addend);
            UnresolvedInfo.P_ = (relocation_adr);
            UnresolvedInfo.sh_type = (RelOrRelaShdr.sh_type);
            
            /*シンボルのアドレスを突き止める*/
            if( CurrentSymEx->Sym.st_shndx == SHN_UNDEF) {
                /*アドレステーブルから検索*/
                ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
                CurrentAdrEntry = EL_GetAdrEntry( sym_str);
                if( CurrentAdrEntry) {
                    sym_loaded_adr = (u32)(CurrentAdrEntry->adr);
                    /*THUMB関数フラグ（仕様書でいう「T」）THUMB or ARM の判別*/
                    thumb_func_flag = CurrentAdrEntry->thumb_flag;
                    if( dbg_print_flag == 1) {
                        printf( "\n symbol found %s : %8x\n", sym_str, (int)(sym_loaded_adr));
                    }
                }else{
                    /*見つからなかった時はエラー(S_とT_が解決できない)*/
                    copy_size = (u32)strlen( sym_str) + 1;
                    UnresolvedInfo.sym_str = (char*)(malloc( copy_size));
                    //MI_CpuCopy8( sym_str, UnresolvedInfo.sym_str, copy_size);
                    memcpy( UnresolvedInfo.sym_str, sym_str, copy_size);

                    /*グローバルな未解決テーブルに追加*/
                    copy_size = sizeof( ELUnresolvedEntry);
                    UnrEnt = (ELUnresolvedEntry*)(malloc( copy_size));
                    //MI_CpuCopy8( &UnresolvedInfo, UnrEnt, copy_size);
                    memcpy( UnrEnt, &UnresolvedInfo, copy_size);
                    
                    if( unresolved_table_block_flag == 0) {    //テーブルへの追加が禁止されていなければ
                        ELi_AddUnresolvedEntry( UnrEnt);
                    }

                    unresolved_num++;    /*未解決シンボル数をカウント*/
                    if( dbg_print_flag == 1) {
                         printf( "WARNING! cannot find symbol : %s\n", sym_str);
                    }
                }
            }else{
                /*シンボルの所属するセクションのExヘッダ取得*/
                CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
                sym_loaded_adr = CurrentShdrEx->loaded_adr;
                sym_loaded_adr += CurrentSymEx->Sym.st_value;    //sym_loaded_adrは仕様書でいう「S」
                /*THUMB関数フラグ（仕様書でいう「T」）THUMB or ARM の判別*/
                thumb_func_flag = CurrentSymEx->thumb_flag;
            }

            if( !UnresolvedInfo.sym_str) {        /*sym_strがセットされているときはシンボル解決不可能*/
                /*仕様書と同じ変数名にする*/
                UnresolvedInfo.S_ = (sym_loaded_adr);
                UnresolvedInfo.T_ = (thumb_func_flag);

                /*--------------- シンボルの解決(再配置の実行) ---------------*/
//                CurrentSymEx->relocation_val = ELi_DoRelocate( &UnresolvedInfo);
                /*------------------------------------------------------------*/
            }
        }
    }
    /*-----------------------------------*/
    /*--- ライブラリ内のGLOBALシンボルをアドレステーブルに公開する ---*/
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx = ELi_GetSymExfromList( ElfHandle->SymEx, i);
        /*GLOBALで、かつ関連するセクションがライブラリ内に存在する場合*/
        if( ((ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_GLOBAL) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_WEAK) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_MW_SPECIFIC))&&
            (CurrentSymEx->Sym.st_shndx != SHN_UNDEF)) {
            
            ExportAdrEntry = (ELAdrEntry*)(malloc( sizeof(ELAdrEntry)));    /*メモリ確保*/
            
            ExportAdrEntry->next = NULL;
            
            ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
            copy_size = (u32)strlen( sym_str) + 1;
            ExportAdrEntry->name = (char*)(malloc( copy_size));
            //MI_CpuCopy8( sym_str, ExportAdrEntry->name, copy_size);
            memcpy( ExportAdrEntry->name, sym_str, copy_size);

            CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
            //Sym.st_valueは偶数/奇数でARM/Thumbを判別できるように調整されている場合があるので、その調整を削除して正味の値を出す
            ExportAdrEntry->adr = (void*)(CurrentShdrEx->loaded_adr + ((CurrentSymEx->Sym.st_value)&0xFFFFFFFE));
            ExportAdrEntry->func_flag = (u16)(ELF32_ST_TYPE( CurrentSymEx->Sym.st_info));
            ExportAdrEntry->thumb_flag = CurrentSymEx->thumb_flag;
            
            if( EL_GetAdrEntry( ExportAdrEntry->name) == NULL) {    //入ってなかったら
                if( dbg_print_flag == 1) {
                    printf( "Add Entry : %s(0x%x), func=%d, thumb=%d\n",
                                ExportAdrEntry->name,
                                (int)(ExportAdrEntry->adr),
                                ExportAdrEntry->func_flag,
                                ExportAdrEntry->thumb_flag);
                }
                EL_AddAdrEntry( ExportAdrEntry);    //登録
            }
        }
    }
    /*----------------------------------------------------------------*/

    /*------- ELSymExのリストを解放する -------*/
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

    /* 再配置完了後 */
    if( unresolved_num == 0) {
        ElfHandle->process = EL_RELOCATED;
    }
}

/*------------------------------------------------------
    makelst専用関数
    シンボルセクションの中からGLOBALなものを
    アドレステーブルに登録する
 -----------------------------------------------------*/
void ELi_DiscriminateGlobalSym( ELHandle* ElfHandle, u32 symsh_index)
{
    u32                    i;
    u32                    num_of_sym;
    Elf32_Shdr            CurrentSymShdr;
    Elf32_Shdr*         SymShdr;        //SYMセクションヘッダ
    ELSymEx*            CurrentSymEx;
    ELSymEx*            FwdSymEx;
    ELSymEx                DmySymEx;
    ELShdrEx*            CurrentShdrEx;
    ELAdrEntry*            ExportAdrEntry;
    char                sym_str[128];
    u32                    copy_size;
    
    /*SYMセクションヘッダ取得*/
    ELi_GetShdr( ElfHandle, symsh_index, &CurrentSymShdr);
    SymShdr = &CurrentSymShdr;

    num_of_sym = (SymShdr->sh_size) / (SymShdr->sh_entsize);    //シンボルの全体数

    /*---------- ELSymExのリストを作る ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx->next = (void*)(malloc( sizeof(ELSymEx)));
        CurrentSymEx = (ELSymEx*)(CurrentSymEx->next);
        
        /*シンボルエントリをコピー*/
        ELi_GetEntry( ElfHandle, SymShdr, i, &(CurrentSymEx->Sym));
        
        /*デバッグ情報フラグをセット*/
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

    /*----- ELSymExのThumbフラグをセット（関数シンボルだけ必要）-----*/
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
    /*--- ライブラリ内のGLOBALシンボルをアドレステーブルに公開する ---*/
    for( i=0; i<num_of_sym; i++) {
        CurrentSymEx = ELi_GetSymExfromList( ElfHandle->SymEx, i);
        /*GLOBALで、かつ関連するセクションがライブラリ内に存在する場合*/
        if( ((ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_GLOBAL) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_WEAK) ||
            (ELF32_ST_BIND( CurrentSymEx->Sym.st_info) == STB_MW_SPECIFIC))&&
            (CurrentSymEx->Sym.st_shndx != SHN_UNDEF)) {
            
            ExportAdrEntry = (ELAdrEntry*)(malloc( sizeof(ELAdrEntry)));    /*メモリ確保*/
            
            ExportAdrEntry->next = NULL;
            
            ELi_GetStrAdr( ElfHandle, SymShdr->sh_link, CurrentSymEx->Sym.st_name, sym_str, 128);
            copy_size = (u32)strlen( sym_str) + 1;
            ExportAdrEntry->name = (char*)(malloc( copy_size));
            //MI_CpuCopy8( sym_str, ExportAdrEntry->name, copy_size);
            memcpy( ExportAdrEntry->name, sym_str, copy_size);

            if( (CurrentSymEx->Sym.st_shndx) < SHN_LORESERVE) { //関連セクションがある場合
                if( (CurrentSymEx->Sym.st_shndx == SHN_ABS)) {
                    //Sym.st_valueは偶数/奇数でARM/Thumbを判別できるように調整されている場合があるので、その調整を削除して正味の値を出す
                    ExportAdrEntry->adr = (void*)((CurrentSymEx->Sym.st_value)&0xFFFFFFFE);
                }else{
                    CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, CurrentSymEx->Sym.st_shndx);
                    //Sym.st_valueは偶数/奇数でARM/Thumbを判別できるように調整されている場合があるので、その調整を削除して正味の値を出す
                    ExportAdrEntry->adr = (void*)(CurrentShdrEx->loaded_adr + ((CurrentSymEx->Sym.st_value)&0xFFFFFFFE));
                }
            ExportAdrEntry->func_flag = (u16)(ELF32_ST_TYPE( CurrentSymEx->Sym.st_info));
            ExportAdrEntry->thumb_flag = CurrentSymEx->thumb_flag;

            if( EL_GetAdrEntry( ExportAdrEntry->name) == NULL) {    //入ってなかったら
                if( dbg_print_flag == 1) {
                    printf( "Add Entry : %s(0x%x), func=%d, thumb=%d\n",
                                ExportAdrEntry->name,
                                (int)(ExportAdrEntry->adr),
                                ExportAdrEntry->func_flag,
                                ExportAdrEntry->thumb_flag);
                }            
                EL_AddAdrEntry( ExportAdrEntry);    //登録
            }
            }
        }
    }
    /*----------------------------------------------------------------*/

    /*------- ELSymExのリストを解放する -------*/
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
  未解決情報をもとにシンボルを解決する

    r_type,S_,A_,P_,T_が全て分かっている必要がある。
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
            _A_ = (((*(vu32*)relocation_adr)|0xFF800000) << 2);    //一般的には-8になっているはず
        }
        signed_val = (( (s32)(_S_) + _A_) | (s32)(_T_)) - (s32)(_P_);
        if( _T_) {        /*BLX命令でARMからThumbに飛ぶ(v5未満だとBL→ベニアでBXという仕組みが必要)*/
            relocation_val = (0xFA000000) | ((signed_val>>2) & 0x00FFFFFF) | (((signed_val>>1) & 0x1)<<24);
        }else{            /*BL命令でARMからARMに飛ぶ*/
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
      case R_ARM_THM_PC22:/*別名：R_ARM_THM_CALL*/
        if( UnresolvedInfo->sh_type == SHT_REL) {
            _A_ = (((*(vu16*)relocation_adr & 0x07FF)<<11) + ((*((vu16*)(relocation_adr)+1)) & 0x07FF));
            _A_ = (_A_ | 0xFFC00000) << 1;    //一般的には-4になっているはず(PCは現命令アドレス+4なので)
        }
        signed_val = (( (s32)(_S_) + _A_) | (s32)(_T_)) - (s32)(_P_);
        signed_val >>= 1;
        if( _T_) {    /*BL命令でThumbからThumbに飛ぶ*/
            relocation_val = ((*(vu16*)relocation_adr & 0xF800) | ((signed_val>>11) & 0x07FF)) +
                                   ((((*((vu16*)(relocation_adr)+1)) & 0xF800) | (signed_val & 0x07FF)) << 16);
        }else{        /*BLX命令でThumbからARMに飛ぶ(v5未満だとBL→ベニアでBXという仕組みが必要)*/
            if( (signed_val & 0x1)) {    //_P_が4バイトアラインされていないとここに来る
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
  リストから指定インデックスのELSymExを取り出す
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
  リストから指定インデックスのELShdrExを取り出す
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
  指定インデックスのセクションがデバッグ情報かどうか判定する

＜デバッグ情報の定義＞
・セクション名が".debug"から始まるセクション

・.rel.debug～ セクションなど、sh_info がデバッグ情報セクション番号を
　示しているセクション
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELHandle* ElfHandle, u32 index)
{
    Elf32_Shdr    TmpShdr;
    char        shstr[6];

    /*-- セクション名の文字列を6文字取得 --*/
    ELi_GetShdr( ElfHandle, index, &TmpShdr);
    ELi_GetStrAdr( ElfHandle, ElfHandle->CurrentEhdr.e_shstrndx,
                   TmpShdr.sh_name, shstr, 6);
    /*-------------------------------------*/
    
    if( strncmp( shstr, ".debug", 6) == 0) {    /*デバッグセクションの場合*/
        return TRUE;
    }else{                        /*デバッグセクションに関する再配置セクションの場合*/
        if( (TmpShdr.sh_type == SHT_REL) || (TmpShdr.sh_type == SHT_RELA)) {
            if( ELi_ShdrIsDebug( ElfHandle, TmpShdr.sh_info) == TRUE) {
                return TRUE;
            }
        }
    }

    return FALSE;
}



/*------------------------------------------------------
  ElfHandleのSymExテーブルを調べ、指定インデックスの
　指定オフセットにあるコードがARMかTHUMBかを判定する。
  （予め ElfHandle->SymShdr と ElfHandle->SymEx を設定しておくこと）
    
    sh_index : 調べたいセクションインデックス
    offset : 調べたいセクション内のオフセット
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELHandle* ElfHandle, u16 sh_index, u32 offset)
{
    u32            i;
    u32            thumb_flag;
    Elf32_Shdr*    SymShdr;
    char        str_adr[3];
    ELSymEx*    CurrentSymEx;

    /*シンボルのセクションヘッダとSymExリスト取得*/
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
 未解決情報エントリを初期化する
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
  未解決情報テーブルからエントリを削除する
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
 未解決情報テーブルにエントリを追加する
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
  未解決情報テーブルから指定文字列に該当するエントリを探す
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
