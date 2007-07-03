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
  セクションヘッダ拡張(ロードアドレス等に対応)
 -----------------------------------------------------*/
typedef struct {
  void*			next;
  u16			index;
  u16			debug_flag;		/*0:デバッグ情報でない、1:デバッグ情報*/
  u32			loaded_adr;
  u32			alloc_adr;
  u32			loaded_size;
  Elf32_Shdr	Shdr;
  char*         str;            /*セクション名文字列をコピーしてきたもの*/
  u32*          sym_table;      /*シンボルセクションのとき有効な新旧対応表*/
  void*         str_table;      /*STRセクションのとき有効：新文字列テーブル*/
  u32           str_table_size; /*STRセクションのとき有効：新文字列テーブルのサイズ*/
}ELShdrEx;


/*------------------------------------------------------
  シンボルエントリ拡張(ロードアドレス等に対応)
 -----------------------------------------------------*/
typedef struct {
  void*		next;
  u16		debug_flag;			/*0:デバッグ情報でない、1:デバッグ情報*/
  u16		thumb_flag;
  u32		relocation_val;
  Elf32_Sym	Sym;
}ELSymEx;



/*------------------------------------------------------
  ELFオブジェクトの管理
 -----------------------------------------------------*/
typedef void (*ELi_ReadFunc)( void* buf, void* file_struct, u32 file_base, u32 file_offset, u32 size);
typedef struct {
  void*			ar_head;		/* ARまたはELFファイルの先頭アドレス */
  void*			elf_offset;		/* ELFオブジェクトの先頭へのオフセット */
  void*			buf_current;	/* Loader作業用 */
  u16			shentsize;		/* 1セクションヘッダのサイズ */
  u16			process;		/* 再配置状況 */

  ELShdrEx*		ShdrEx;			/* ShdrExリストの先頭 */

  Elf32_Ehdr	CurrentEhdr;	/* ELFヘッダ */

  Elf32_Rel		Rel;			/* 再配置エントリ */
  Elf32_Rela	Rela;
  Elf32_Sym		Sym;			/* シンボルエントリ */
  Elf32_Shdr	SymShdr;

  ELSymEx*		SymEx;			/* SymExリストの先頭 */
  ELSymEx**     SymExTbl;       /* SymExアドレスのテーブル（全シンボル数ぶん）*/
  u32           SymExTarget;    /* SymExリストを構築したシンボルセクションのセクション番号 */

  ELi_ReadFunc	ELi_ReadStub;	/* リードスタブ関数 */
  void*			FileStruct;		/* ファイル構造体 */

  u32			mem_adr;		/*最初にロードされたセクションのsh_addrが入る(DSのROMヘッダ用パラメータ)*/
  u32           newelf_size;
}ELHandle;



/*------------------------------------------------------
  アドレステーブル
 -----------------------------------------------------*/
typedef struct {
  void*		next;				/*次のアドレスエントリ*/
  char*		name;				/*文字列*/
  void*		adr;				/*アドレス*/
  u16		func_flag;			/*0:データ、1:関数*/
  u16		thumb_flag;			/*0:armコード、1:thumbコード*/
}ELAdrEntry;


/*------------------------------------------------------
  未解決の再配置情報テーブル

  後からアドレステーブルを参照すれば次のように解決する。
  S_ = AdrEntry.adr;
  T_ = (u32)(AdrEntry.thumb_flag);
 -----------------------------------------------------*/
typedef struct {
  void* next;					/*次のエントリ*/
  char*	sym_str;				/*未解決の外部参照シンボル名*/
  u32	r_type;					/*リロケーションタイプ（ELF32_R_TYPE( Rela.r_info)）*/
  u32	S_;						/*未解決の外部参照シンボルアドレス*/
  s32	A_;						/*解決済み*/
  u32	P_;						/*解決済み*/
  u32	T_;						/*未解決の外部参照シンボルのARM/Thumbフラグ*/
  u32	sh_type;				/*SHT_REL or SHT_RELA*/
  u32	remove_flag;			/*解決したときにセットする（消しても良いことを識別する）フラグ*/
  ELAdrEntry*	AdrEnt;			/*アドレステーブルから探し出したエントリの場所*/
}ELUnresolvedEntry;



/* ELHandle の process値 */
#define EL_FAILED			0x00
#define EL_INITIALIZED		0x5A
#define EL_COPIED			0xF0
#define EL_RELOCATED		0xF1



/*---------------------------------------------------------
 ELFオブジェクトのサイズを求める
 --------------------------------------------------------*/
u32 EL_GetElfSize( const void* buf);

/*------------------------------------------------------
  ダイナミックリンクシステムを初期化する
 -----------------------------------------------------*/
void EL_Init( void);

/*------------------------------------------------------
  ELHandle構造体を初期化する
 -----------------------------------------------------*/
BOOL EL_InitHandle( ELHandle* ElfHandle);

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをファイルからバッファに再配置する
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromFile( ELHandle* ElfHandle, FILE* ObjFile, void* buf);

/*------------------------------------------------------
  ELFオブジェクトまたはそのアーカイブをメモリからバッファに再配置する
 -----------------------------------------------------*/
u16 EL_LoadLibraryfromMem( ELHandle* ElfHandle, void* obj_image, u32 obj_len, void* buf);

/*------------------------------------------------------
  アドレステーブルを使って未解決のシンボルを解決する
 -----------------------------------------------------*/
u16 EL_ResolveAllLibrary( void);


/*------------------------------------------------------
  マーキングされたシンボルを公開用ファイルに構造体として書き出す
 -----------------------------------------------------*/
u16 EL_ExtractStaticSym1( void);
/*------------------------------------------------------
  マーキングされたシンボルを公開用ファイルにAPIとして書き出す
 -----------------------------------------------------*/
u16 EL_ExtractStaticSym2( void);


/*------------------------------------------------------
  アドレステーブルからエントリを削除する
 -----------------------------------------------------*/
BOOL EL_RemoveAdrEntry( ELAdrEntry* AdrEnt);

/*------------------------------------------------------
  アドレステーブルにエントリを追加する
 -----------------------------------------------------*/
void EL_AddAdrEntry( ELAdrEntry* AdrEnt);

/*------------------------------------------------------
  アドレステーブルから指定文字列に該当するエントリを探す
 -----------------------------------------------------*/
ELAdrEntry* EL_GetAdrEntry( char* ent_name);

/*------------------------------------------------------
  アドレステーブルから指定文字列に該当するアドレスを返す
 -----------------------------------------------------*/
void* EL_GetGlobalAdr( char* ent_name);





/*他に必要そうな関数*/
//ロードに必要なメモリのバイト数を算出する関数
//EL_FreeLibrary

#endif	/*_ELF_LOADER_H_*/



