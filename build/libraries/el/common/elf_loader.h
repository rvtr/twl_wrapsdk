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
  セクションヘッダ拡張(ロードアドレス等に対応)
 -----------------------------------------------------*/
typedef struct {
  void*         next;
  u16           index;
  u16           debug_flag;    /*0:デバッグ情報でない、1:デバッグ情報*/
  u32           loaded_adr;
  u32           alloc_adr;
  u32           loaded_size;
  Elf32_Shdr    Shdr;
}ElShdrEx;


/*------------------------------------------------------
  シンボルエントリ拡張(ロードアドレス等に対応)
 -----------------------------------------------------*/
typedef struct {
  void*      next;
  u16        debug_flag;       /*0:デバッグ情報でない、1:デバッグ情報*/
  u16        thumb_flag;
  u32        relocation_val;
  Elf32_Sym  Sym;
}ElSymEx;


/*------------------------------------------------------
  ベニヤのリンクリスト(i_elDoRelocateで使用)
 -----------------------------------------------------*/
typedef struct {
  void* next;
  u32   adr;     /* ベニヤコードの先頭アドレス */
  u32   data;    /* ベニヤのリテラルプールに格納されている飛び先のアドレス値 */
}ElVeneer;


/*------------------------------------------------------
  ELFオブジェクトの管理
 -----------------------------------------------------*/
typedef void (*i_elReadFunc)( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
typedef struct {
  void*         ar_head;        /* ARまたはELFファイルの先頭アドレス */
  void*         elf_offset;     /* ELFオブジェクトの先頭へのオフセット */
  void*         buf_current;    /* Loader作業用 */
  u16           shentsize;      /* 1セクションヘッダのサイズ */
  u16           process;        /* 再配置状況 */

  ElShdrEx*     ShdrEx;         /* ShdrExリストの先頭 */

  Elf32_Ehdr    CurrentEhdr;    /* ELFヘッダ */

  Elf32_Rel     Rel;            /* 再配置エントリ */
  Elf32_Rela    Rela;
  Elf32_Sym     Sym;            /* シンボルエントリ */
  Elf32_Shdr    SymShdr;

  ElSymEx*      SymEx;          /* SymExリストの先頭（非デバッグシンボルのみ繋がる） */
  ElSymEx**     SymExTbl;       /* SymExアドレスのテーブル（全シンボル数ぶん）*/
  u32           SymExTarget;    /* SymExリストを構築したシンボルセクションのセクション番号 */

  ElVeneer*     VeneerEx;       /* ベニヤのリンクリスト */
  
  i_elReadFunc  i_elReadStub;   /* リードスタブ関数 */
  void*         FileStruct;     /* ファイル構造体 */
    
  u32           entry_adr;      /* エントリアドレス */
}ElDesc;



/*------------------------------------------------------
  アドレステーブル
 -----------------------------------------------------*/
typedef struct {
  void*      next;              /*次のアドレスエントリ*/
  char*      name;              /*文字列*/
  void*      adr;               /*アドレス*/
  u16        func_flag;         /*0:データ、1:関数*/
  u16        thumb_flag;        /*0:armコード、1:thumbコード*/
}ElAdrEntry;


/*------------------------------------------------------
  未解決の再配置情報テーブル

  後からアドレステーブルを参照すれば次のように解決する。
  S_ = AdrEntry.adr;
  T_ = (u32)(AdrEntry.thumb_flag);
 -----------------------------------------------------*/
typedef struct {
  void*  next;                  /*次のエントリ*/
  char*  sym_str;               /*未解決の外部参照シンボル名*/
  u32    r_type;                /*リロケーションタイプ（ELF32_R_TYPE( Rela.r_info)）*/
  u32    S_;                    /*未解決の外部参照シンボルアドレス*/
  s32    A_;                    /*解決済み*/
  u32    P_;                    /*解決済み*/
  u32    T_;                    /*未解決の外部参照シンボルのARM/Thumbフラグ*/
  u32    sh_type;               /*SHT_REL or SHT_RELA*/
  u32    remove_flag;           /*解決したときにセットする（消しても良いことを識別する）フラグ*/
}ElUnresolvedEntry;



/* ElDesc の process値 */
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
 ELFオブジェクトのサイズを求める
 --------------------------------------------------------*/
u32 elGetElfSize( const void* buf);

/*------------------------------------------------------
  ダイナミックリンクシステムを初期化する
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
void elInit( void);
#else
void elInit( ElAlloc alloc, ElFree free);
#endif

/*------------------------------------------------------
  ElDesc構造体を初期化する
 -----------------------------------------------------*/
BOOL elInitDesc( ElDesc* elElfDesc);

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをファイルからバッファに再配置する
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
u16 elLoadLibraryfromFile( ElDesc* elElfDesc, FSFile* ObjFile, void* buf);
#else
u16 elLoadLibraryfromFile( ElDesc* elElfDesc, int ObjFile, void* buf);
#endif

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをユーザのリードAPIを通して再配置する
 -----------------------------------------------------*/
#if (TARGET_OS_NITRO == 1)
#else
u16 elLoadLibrary( ElDesc* elElfDesc, ElReadImage readfunc, u32 len, void* buf);
#endif

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをメモリからバッファに再配置する
 -----------------------------------------------------*/
u16 elLoadLibraryfromMem( ElDesc* elElfDesc, void* obj_image, u32 obj_len, void* buf);

/*------------------------------------------------------
  アドレステーブルを使って未解決のシンボルを解決する
 -----------------------------------------------------*/
u16 elResolveAllLibrary( ElDesc* elElfDesc);


/*------------------------------------------------------
  アドレステーブルからエントリを削除する
 -----------------------------------------------------*/
BOOL elRemoveAdrEntry( ElAdrEntry* AdrEnt);

/*------------------------------------------------------
  アドレステーブルにエントリを追加する
 -----------------------------------------------------*/
void elAddAdrEntry( ElAdrEntry* AdrEnt);

/*------------------------------------------------------
  アドレステーブルにスタティック側のエントリを追加する
  （ELライブラリ内でWEAKシンボルとして定義されており、
    makelstが生成したファイルの定義で上書きされる）
 -----------------------------------------------------*/
void elAddStaticSym( void);


/*------------------------------------------------------
  アドレステーブルから指定文字列に該当するエントリを探す
 -----------------------------------------------------*/
ElAdrEntry* elGetAdrEntry( const char* ent_name);

/*------------------------------------------------------
  アドレステーブルから指定文字列に該当するアドレスを返す
 -----------------------------------------------------*/
void* elGetGlobalAdr( const char* ent_name);



/*他に必要そうな関数*/
//ロードに必要なメモリのバイト数を算出する関数
//elFreeLibrary


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif    /*_ELF_LOADER_H_*/
