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
  グローバル変数
 -----------------------------------------------------*/
//#if (TARGET_OS_NITRO == 1)
//#else
ElReadImage i_elReadImage;
ElAlloc     i_elAlloc;
ElFree      i_elFree;
//#endif


/*------------------------------------------------------
  ローカル関数の宣言
 -----------------------------------------------------*/
static u16 elLoadSegments( ElDesc* elElfDesc);
static u16 elLoadSections( ElDesc* elElfDesc);

// ELFオブジェクトまたはそのアーカイブをバッファに再配置する
static u16 i_elLoadLibrary( ElDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf);
// ELFオブジェクトをバッファに再配置するコア関数
static u16 i_elLoadObject( ElDesc* elElfDesc, void* obj_offset, void* buf);
// ELFオブジェクトからデータを読み出すスタブ関数
static void i_elReadFile( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
static void i_elReadMem( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
static void i_elReadUsr( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);


/*---------------------------------------------------------
 ELFオブジェクトのサイズを求める
    
    buf : ELFイメージのアドレス
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
  ダイナミックリンクシステムを初期化する
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
void elInit( void)
{
    void* heap_start;

    /*--- メモリアロケーション関係の設定 ---*/
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
  ElDesc構造体を初期化する
 -----------------------------------------------------*/
BOOL elInitDesc( ElDesc* elElfDesc)
{
    if( elElfDesc == NULL) {    /*NULLチェック*/
        return FALSE;
    }

    /*初期値の設定*/
    elElfDesc->ShdrEx   = NULL;
    elElfDesc->SymEx    = NULL;
    elElfDesc->SymExTbl = NULL;
    elElfDesc->SymExTarget = 0xFFFFFFFF;

    elElfDesc->process = EL_INITIALIZED;    /*フラグの設定*/

    return TRUE;
}

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    ObjFile : OBJファイルまたはアーカイブファイルの構造体
    buf : ロードするバッファ
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
u16 elLoadLibraryfromFile( ElDesc* elElfDesc, FSFile* ObjFile, void* buf)
#else
u16 elLoadLibraryfromFile( ElDesc* elElfDesc, int ObjFile, void* buf)
#endif
{
    u16 result;
    u32 len;

    /*リード関数の設定*/
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
  ELFオブジェクトまたはそのアーカイブをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    readfunc : OBJファイルまたはアーカイブファイルを読み出すユーザ関数
    buf : ロードするバッファ
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
  ELFオブジェクトまたはそのアーカイブをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    obj_image : OBJファイルまたはアーカイブファイルのRAM上イメージアドレス
    buf : ロードするバッファ
 -----------------------------------------------------*/
u16 elLoadLibraryfromMem( ElDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf)
{
    u16 result;
    
    /*リード関数の設定*/
    elElfDesc->i_elReadStub = i_elReadMem;
    elElfDesc->FileStruct = NULL;
    
    result = i_elLoadLibrary( elElfDesc, obj_image, obj_len, buf);

    return result;
}

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    obj_image : OBJファイルまたはアーカイブファイルのRAM上イメージアドレス
    buf : ロードするバッファ
 -----------------------------------------------------*/
static u16 i_elLoadLibrary( ElDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf)
{
    u16        result, all_result;
    u32        image_pointer;
    u32        arch_size;
    u32        elf_num = 0;                /*ELFオブジェクトの数*/
    ArchHdr    ArHdr;
    char       OBJMAG[8];
    char       ELFMAG[4] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

    all_result = EL_RELOCATED;
    elElfDesc->ar_head = obj_image;
    image_pointer = 0;

    elElfDesc->i_elReadStub( OBJMAG, elElfDesc->FileStruct, (u32)obj_image, 0, 8);    /*OBJの文字列を取得*/
    /*--------------- アーカイブファイルの場合 ---------------*/
    if( OSAPI_STRNCMP( OBJMAG, ARMAG, 8) == 0) {
        arch_size = sizeof( ArchHdr);
        image_pointer += 8;                /*最初のエントリへ*/
        
        elElfDesc->buf_current = buf;
        while( image_pointer < obj_len) {
            elElfDesc->i_elReadStub( OBJMAG, elElfDesc->FileStruct, (u32)(obj_image), (image_pointer+arch_size), 4);    /*OBJの文字列を取得*/
            if( OSAPI_STRNCMP( OBJMAG, ELFMAG, 4) == 0) {
                elf_num++;
                result = i_elLoadObject( elElfDesc, (void*)(image_pointer+arch_size), elElfDesc->buf_current);
                if( result < all_result) {        /*悪い結果のときだけall_resultに反映*/
                    all_result = result;
                }
                /*初期値の設定*/
                elElfDesc->ShdrEx   = NULL;
                elElfDesc->SymEx    = NULL;
                elElfDesc->SymExTbl = NULL;
                elElfDesc->SymExTarget = 0xFFFFFFFF;
                elElfDesc->process = EL_INITIALIZED;    /*フラグの設定*/
            }else{
            }
            /*次のエントリへ*/
            elElfDesc->i_elReadStub( &ArHdr, elElfDesc->FileStruct, (u32)(obj_image), image_pointer, arch_size);
            image_pointer += arch_size + AR_GetEntrySize( &ArHdr);
        }
    }else{/*--------------- ELFファイルの場合 ---------------*/
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
  ELFオブジェクトをバッファに再配置する
    
    elElfDesc : ヘッダ構造体
    obj_offset : OBJファイルのRAM上イメージアドレスからのオフセット
    buf : ロードするバッファ(TODO:バッファオーバーフロー対策)
 -----------------------------------------------------*/
static u16 i_elLoadObject( ElDesc* elElfDesc, void* obj_offset, void* buf)
{
    u16 ret_val;
    
    /* ElDescの初期化チェック */
    if( elElfDesc->process != EL_INITIALIZED) {
        return EL_FAILED;
    }
    /* ELFヘッダの取得 */
    elElfDesc->i_elReadStub( &(elElfDesc->CurrentEhdr), elElfDesc->FileStruct,
                             (u32)(elElfDesc->ar_head), (u32)(obj_offset), sizeof( Elf32_Ehdr));

    /* ElDesc構造体の構築 */
    elElfDesc->elf_offset = obj_offset;
    elElfDesc->buf_current = buf;
    elElfDesc->shentsize = elElfDesc->CurrentEhdr.e_shentsize;
    elElfDesc->entry_adr = elElfDesc->CurrentEhdr.e_entry;

    /* ELFファイルタイプ別に処理 */
    switch( elElfDesc->CurrentEhdr.e_type) {
        
      case ET_NONE:
        PRINTDEBUG( "ERROR : Elf type \"ET_NONE\"\n");
        ret_val = EL_FAILED;
        break;
        
      case ET_REL:  /* 実行×、再配置○ */
        PRINTDEBUG( "Elf type \"ET_REL\"\n");
        if( buf == NULL) {        /* バッファのNULLチェック */
            return EL_FAILED;
        }
        ret_val = elLoadSections( elElfDesc);
        break;
        
      case ET_EXEC: /* 実行○、再配置× */
        PRINTDEBUG( "Elf type \"ET_EXEC\"\n");
        ret_val = elLoadSegments( elElfDesc);
        break;
        
      case ET_DYN:  /* 実行○、再配置○ (TODO:未テスト)*/
        PRINTDEBUG( "Elf type \"ET_DYN\"\n");
        if( buf == NULL) { //ロードアドレスが指定されてないときはET_EXEC扱い
            ret_val = elLoadSegments( elElfDesc);
        }else{             //ロードアドレスが指定されていればET_REL扱い
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
  全セグメントを調べてコピーする
    
    elElfDesc : ヘッダ構造体
 -----------------------------------------------------*/
static u16 elLoadSegments( ElDesc* elElfDesc)
{
    u16        i;
    //u32        load_start;
    Elf32_Phdr CurrentPhdr;
    
    for( i=0; i<(elElfDesc->CurrentEhdr.e_phnum); i++) {
        /*プログラムヘッダをコピー*/
        i_elGetPhdr( elElfDesc, i, &CurrentPhdr);
        
        if( CurrentPhdr.p_type == PT_LOAD) {
            /*ロード可能セグメントならメモリにロード*/
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
  全セクションを調べてコピーする
    
    elElfDesc : ヘッダ構造体
 -----------------------------------------------------*/
static u16 elLoadSections( ElDesc* elElfDesc)
{
    u16         i;
    ElShdrEx*   FwdShdrEx;
    ElShdrEx*   CurrentShdrEx;
    ElShdrEx*   InfoShdrEx;      //例えばCurrentShdrExがrel.textの場合.textをさす
    ElShdrEx    DmyShdrEx;
#if (DEBUG_PRINT_ON == 1)
    u16         j;
    u32         num_of_entry;
    char        sym_str[128];    //デバッグプリント用
    u32         offset;          //デバッグプリント用
#endif
    
    /*---------- ElShdrExのリストを作る ----------*/
    CurrentShdrEx = &DmyShdrEx;
    for( i=0; i<(elElfDesc->CurrentEhdr.e_shnum); i++) {
        CurrentShdrEx->next = OSAPI_MALLOC( sizeof(ElShdrEx));
        CurrentShdrEx = (ElShdrEx*)(CurrentShdrEx->next);
        OSAPI_CPUFILL8( CurrentShdrEx, 0, sizeof(ElShdrEx));    //ゼロクリア
        
        /*デバッグ情報かどうかを判別してフラグをセット*/
        if( i_elShdrIsDebug( elElfDesc, i) == TRUE) {    /*デバッグ情報の場合*/
            CurrentShdrEx->debug_flag = 1;
        }else{                                           /*デバッグ情報でない場合*/
            /*セクションヘッダをコピー*/
            i_elGetShdr( elElfDesc, i, &(CurrentShdrEx->Shdr));
            CurrentShdrEx->debug_flag = 0;
        }
    }
    CurrentShdrEx->next = NULL;
    elElfDesc->ShdrEx = DmyShdrEx.next;
    /*--------------------------------------------*/

    /*---------- 全セクションを調べてコピーする ----------*/
    PRINTDEBUG( "\nLoad to RAM:\n");
    for( i=0; i<(elElfDesc->CurrentEhdr.e_shnum); i++) {
        /**/
        CurrentShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx, i);
//        PRINTDEBUG( "section:%d sh_flag=0x%x\n", i, CurrentShdrEx->Shdr.sh_flags);
//        PRINTDEBUG( "section:%d sh_type=0x%x\n", i, CurrentShdrEx->Shdr.sh_type);

        if( CurrentShdrEx->debug_flag == 1) {              /*デバッグ情報の場合*/
            PRINTDEBUG( "skip debug-section %02x\n", i);
        }else{                                             /*デバッグ情報でない場合*/
            /* .text section */
            if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //メモリにコピー
                CurrentShdrEx->loaded_adr = (u32)
                                i_elCopySectionToBuffer( elElfDesc, &(CurrentShdrEx->Shdr));
            }
            /* .data, .data1 section (初期化済みデータ) */
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //メモリにコピー
                CurrentShdrEx->loaded_adr = (u32)
                                i_elCopySectionToBuffer( elElfDesc, &(CurrentShdrEx->Shdr));
            }
            /* .bss section */
            else if( (CurrentShdrEx->Shdr.sh_flags == (SHF_ALLOC | SHF_WRITE))&&
                (CurrentShdrEx->Shdr.sh_type == SHT_NOBITS)) {
                //コピーしない
                CurrentShdrEx->loaded_adr = (u32)
                                i_elAllocSectionToBuffer( elElfDesc, &(CurrentShdrEx->Shdr));
            }
            /* .rodata, .rodata1 section */
            else if( (CurrentShdrEx->Shdr.sh_flags == SHF_ALLOC)&&
                (CurrentShdrEx->Shdr.sh_type == SHT_PROGBITS)) {
                //メモリにコピー
                CurrentShdrEx->loaded_adr = (u32)
                                i_elCopySectionToBuffer( elElfDesc, &(CurrentShdrEx->Shdr));
            }

            PRINTDEBUG( "section %02x relocated at %08x\n",
                        i, CurrentShdrEx->loaded_adr);
        }
    }
    /* コピー終了後 */
    elElfDesc->process = EL_COPIED;
    /*----------------------------------------------------*/

    /*---------- グローバルシンボルの公開とローカルシンボルの再配置 ----------*/
    PRINTDEBUG( "\nRelocate Symbols:\n");
    for( i=0; i<(elElfDesc->CurrentEhdr.e_shnum); i++) {
        /**/
        CurrentShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx, i);
        
        if( CurrentShdrEx->debug_flag == 1) {                /*デバッグ情報の場合*/
        }else{                                               /*デバッグ情報でない場合*/

            if( CurrentShdrEx->Shdr.sh_type == SHT_REL) {
                /*リロケート*/
                InfoShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx,
                                                    CurrentShdrEx->Shdr.sh_info);
                if( InfoShdrEx->loaded_adr != 0) { //対象セクションがロードされていれば内部を再配置
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
                    /*次のエントリへ*/
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
#endif
            }
            else if( CurrentShdrEx->Shdr.sh_type == SHT_RELA) {
                /*リロケート*/
                InfoShdrEx = i_elGetShdrExfromList( elElfDesc->ShdrEx,
                                                    CurrentShdrEx->Shdr.sh_info);
                if( InfoShdrEx->loaded_adr != 0) { //対象セクションがロードされていれば内部を再配置
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
                    /*次のエントリへ*/
                    offset += (u32)(CurrentShdrEx->Shdr.sh_entsize);
                }
#endif
            }
            else if( CurrentShdrEx->Shdr.sh_type == SHT_SYMTAB) {
                /*グローバルシンボルをアドレステーブルに登録*/
                i_elGoPublicGlobalSym( elElfDesc, i);
            }
        }
    }
    /*i_elRelocateSymやi_elGoPublicGlobalSym内で作成&使い回したシンボルリストを開放*/
    i_elFreeSymList( elElfDesc);

    /*------- ElShdrExのリストを解放する -------*/
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

    /*RAM上のDLLが呼ばれる前にキャッシュをフラッシュ*/
#if (TARGET_ARM_V5 == 1)
    OSAPI_FLUSHCACHEALL();
    OSAPI_WAITCACHEBUF();
#endif
    
    return (elElfDesc->process);
}

/*------------------------------------------------------
  未解決のシンボルがあればアドレステーブルを使って解決する
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
        AdrEnt = elGetAdrEntry( UnrEnt->sym_str);        /*アドレステーブルから検索*/
        if( AdrEnt) {                                    /*アドレステーブルから見つかった場合*/
            UnrEnt->S_ = (u32)(AdrEnt->adr);
            UnrEnt->T_ = (u32)(AdrEnt->thumb_flag);
            PRINTDEBUG( "\n symbol found %s : %8x\n", UnrEnt->sym_str, UnrEnt->S_);
            ret_val = i_elDoRelocate( elElfDesc, UnrEnt);           /*シンボル解決*/
            if( ret_val == FALSE) {
                return EL_FAILED; //osPanicの方がいい?
            }else{
                UnrEnt->remove_flag = 1;                 /*解決したというマーキング*/
            }
        }else{                                           /*アドレステーブルから見つからなかった場合*/
            PRINTDEBUG( "\n ERROR! cannot find symbol : %s\n\n", UnrEnt->sym_str);
            return EL_FAILED; //osPanicの方がいい?
        }
        UnrEnt = UnrEnt->next;                           /*次の未解決エントリへ*/
    }

    /*TODO:ここで解決できなかったUnrEntエントリだけ削除するべき*/
    
    /*--- ElUnresolvedEntryのリストを解放する ---*/
    CurrentUnrEnt = ELUnrEntStart;
//    if( CurrentUnrEnt) {
        while( CurrentUnrEnt != NULL) {
            FwdUnrEnt = CurrentUnrEnt;
            CurrentUnrEnt = CurrentUnrEnt->next;
            OSAPI_FREE( FwdUnrEnt->sym_str);    //シンボル名文字列
            OSAPI_FREE( FwdUnrEnt);            //構造体自身
        }
        ELUnrEntStart = NULL;
//    }
    /*-------------------------------------------*/

    /*ベニヤのリンクリストを開放*/
    i_elFreeVenTbl();

    /*RAM上のDLLが呼ばれる前にキャッシュをフラッシュ*/
#if (TARGET_ARM_V5 == 1)
    OSAPI_FLUSHCACHEALL();
    OSAPI_WAITCACHEBUF();
#endif
    
    return EL_RELOCATED;
}


/*------------------------------------------------------
  アドレステーブルからエントリを削除する
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

    /*リンクリストの繋ぎ直し*/
    CurrentAdrEnt->next = AdrEnt->next;
    ELAdrEntStart = DmyAdrEnt.next;

    /*開放*/
    OSAPI_FREE( AdrEnt);
    
     return TRUE;
}

/*------------------------------------------------------
  アドレステーブルにエントリを追加する
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
  アドレステーブルにスタティック側のエントリを追加する
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
  アドレステーブルから指定文字列に該当するエントリを探す
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
  アドレステーブルから指定文字列に該当するアドレスを返す
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
  アドレステーブルを解放する（アプリケーションが登録したエントリまで削除しようとするのでＮＧ）
 -----------------------------------------------------*/
#if 0
void* elFreeAdrTbl( void)
{
    ElAdrEntry*    FwdAdrEnt;
    ElAdrEntry*    CurrentAdrEnt;
    
    /*--- ElAdrEntryのリストを解放する ---*/
    CurrentAdrEnt = ELAdrEntStart;
    if( CurrentAdrEnt) {
        while( CurrentAdrEnt->next != NULL) {
            FwdAdrEnt = CurrentAdrEnt;
            CurrentAdrEnt = CurrentAdrEnt->next;
            OSAPI_FREE( FwdAdrEnt->name);        //シンボル名文字列
            OSAPI_FREE( FwdAdrEnt);              //構造体自身
        }
        ELAdrEntStart = NULL;
    }
    /*------------------------------------*/

    return NULL;
}
#endif

/*------------------------------------------------------
  ELFオブジェクトからデータを読み出すスタブ
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
