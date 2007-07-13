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

u32 ELi_ALIGN( u32 addr, u32 align_size);
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
  シンボル文字列の中身をバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopySymStrToBuffer( ELHandle* ElfHandle, ELShdrEx* SymStrShdrEx)
{
    u32 load_start;

    /*アラインメントをとる*/
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);

    memcpy( (void*)load_start, SymStrShdrEx->str_table, SymStrShdrEx->str_table_size);

    /*セクションヘッダのサイズ修正*/
    SymStrShdrEx->Shdr.sh_size = SymStrShdrEx->str_table_size;

    /*バッファポインタを移動*/
    ElfHandle->buf_current = (void*)(load_start + SymStrShdrEx->str_table_size);

    return( void*)load_start;
}

/*------------------------------------------------------
  セクション文字列の中身をバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopyShStrToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32       load_start, i;
    u32       total_size = 0;
    ELShdrEx* CurrentShdrEx;

    /*アラインメントをとる*/
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
//    load_adr = load_start;

    for( i=0; i<(ElfHandle->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx = ELi_GetShdrExfromList( ElfHandle->ShdrEx, i);

        if( CurrentShdrEx->debug_flag == 1) {
        }else{
            CurrentShdrEx->Shdr.sh_name = total_size;
            strcpy( (void*)(load_start+total_size), CurrentShdrEx->str);
            total_size += (strlen( CurrentShdrEx->str) + 1);
        }
    }

    /*セクションヘッダのサイズ修正*/
    Shdr->sh_size = total_size;

    /*バッファポインタを移動*/
    ElfHandle->buf_current = (void*)(load_start + total_size);

    return( void*)load_start;
}

/*------------------------------------------------------
  シンボルエントリをまとめてバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopySymToBuffer( ELHandle* ElfHandle)
{
    u32      load_start, load_adr;
    u32      total_size = 0;
    ELSymEx* CurrentSymEx;

    /*アラインメントをとる*/
    load_start = ELi_ALIGN( ((u32)(ElfHandle->buf_current)), 4);
    load_adr = load_start;

    CurrentSymEx = ElfHandle->SymEx;
    while( CurrentSymEx != NULL) {
        /*コピー*/
        memcpy( (u8*)load_adr, &(CurrentSymEx->Sym),
                sizeof( Elf32_Sym));
        CurrentSymEx = CurrentSymEx->next;
        load_adr += sizeof( Elf32_Sym);
        total_size += sizeof( Elf32_Sym);
//                        printf( "symbol found\n");
    }
    
    /*バッファポインタを移動*/
    ElfHandle->buf_current = (void*)(load_start + total_size);

    return( void*)load_start;
}

/*------------------------------------------------------
  セクションをバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr)
{
    u32         load_start;
    Elf32_Addr  sh_size;

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
    u32           load_start;
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
  シンボルリストを作成する

 -----------------------------------------------------*/
void ELi_BuildSymList( ELHandle* elElfDesc, u32 symsh_index, u32** sym_table)
{
    u32         i;
    u32         num_of_sym;        //シンボルの全体数
    u16         debug_flag;
    Elf32_Sym   TestSym;
    ELSymEx*    CurrentSymEx;
    ELShdrEx*   CurrentShdrEx;
    Elf32_Shdr  SymShdr;
    ELSymEx     DmySymEx;
    u32         sym_num = 0;

    if( elElfDesc->SymExTarget == symsh_index) {
//        printf( "%s skip.\n", __FUNCTION__);
        return;                              //既にリスト作成済み
    }else{
        ELi_FreeSymList( elElfDesc, sym_table); /*シンボルリストを開放する*/
    }
    
//    printf( "%s build\n", __FUNCTION__);

    /*SYMTABセクションヘッダ取得*/
    ELi_GetShdr( elElfDesc, symsh_index, &SymShdr);
    
    /*シンボル新旧対応テーブル構築*/
    *sym_table = (u32*)malloc( 4 * (SymShdr.sh_size / SymShdr.sh_entsize));

    num_of_sym = (SymShdr.sh_size) / (SymShdr.sh_entsize);    //シンボルの全体数
    elElfDesc->SymExTbl = malloc( num_of_sym * 4);
    
    /*---------- ElSymExのリストを作る ----------*/
    CurrentSymEx = &DmySymEx;
    for( i=0; i<num_of_sym; i++) {
        
        /*シンボルエントリをコピー*/
        ELi_GetEntry( elElfDesc, &SymShdr, i, &TestSym);

        /*-- デバッグ情報フラグをセット --*/
        CurrentShdrEx = ELi_GetShdrExfromList( elElfDesc->ShdrEx, TestSym.st_shndx);
        if( CurrentShdrEx) {
            debug_flag = CurrentShdrEx->debug_flag;
        }else{
            debug_flag = 0;
        }/*-------------------------------*/

        if( debug_flag == 1) {
            elElfDesc->SymExTbl[i] = NULL;
            (*sym_table)[i] = 0xFFFFFFFF;
        }else{
            CurrentSymEx->next = malloc( sizeof(ELSymEx));
            CurrentSymEx = (ELSymEx*)(CurrentSymEx->next);
            
            memcpy( &(CurrentSymEx->Sym), &TestSym, sizeof(TestSym));
            
            elElfDesc->SymExTbl[i] = CurrentSymEx;

            (*sym_table)[i] = sym_num;

            sym_num++;
            
//            printf( "sym_no: %02x ... st_shndx: %04x\n", i, CurrentSymEx->Sym.st_shndx);
            
            /*-- ElSymExのThumbフラグをセット（関数シンボルだけ必要）--*/
            if( ELF32_ST_TYPE( CurrentSymEx->Sym.st_info) == STT_FUNC) {
                CurrentSymEx->thumb_flag = (u16)(ELi_CodeIsThumb( elElfDesc, CurrentSymEx->Sym.st_shndx,
                                                               CurrentSymEx->Sym.st_value));
            }else{
                CurrentSymEx->thumb_flag = 0;
            }/*--------------------------------------------------------*/
        }
    }
    
    CurrentSymEx->next = NULL;
    elElfDesc->SymEx = DmySymEx.next;
    /*-------------------------------------------*/


    elElfDesc->SymExTarget = symsh_index;
}


/*------------------------------------------------------
  シンボルリストを開放する

 -----------------------------------------------------*/
void ELi_FreeSymList( ELHandle* elElfDesc, u32** sym_table)
{
    ELSymEx*  CurrentSymEx;
    ELSymEx*  FwdSymEx;

    if( elElfDesc->SymExTbl != NULL) {
        free( elElfDesc->SymExTbl);
        elElfDesc->SymExTbl = NULL;
    }
    
    /*------- ElSymExのリストを解放する -------*/
    CurrentSymEx = elElfDesc->SymEx;
    if( CurrentSymEx) {
        while( CurrentSymEx->next != NULL) {
            FwdSymEx = CurrentSymEx;
            CurrentSymEx = CurrentSymEx->next;
            free( FwdSymEx);
        }
        elElfDesc->SymEx = NULL;
    }
    /*-----------------------------------------*/

/*    if( *sym_table) { //ここはアプリケーションで開放させるのでコメントアウト
        free( *sym_table);
    }*/


    elElfDesc->SymExTarget = 0xFFFFFFFF;
}


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
    ELShdrEx*   ShdrEx;

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
    Elf32_Shdr  TmpShdr;
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
    u32         i;
    u32         thumb_flag;
    Elf32_Shdr* SymShdr;
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
    ELUnresolvedEntry*   CurrentUnrEnt;

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
    ELUnresolvedEntry*   CurrentUnrEnt;

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
