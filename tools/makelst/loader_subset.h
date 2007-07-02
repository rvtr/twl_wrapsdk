/*---------------------------------------------------------------------------*
  Project:  CTR - ELF Loader
  File:     loader_subset.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef _LOADER_SUBSET_H_
#define _LOADER_SUBSET_H_

#include "types.h"
#include "elf.h"
#include "elf_loader.h"


/*------------------------------------------------------
  セクションをバッファにコピーする
 -----------------------------------------------------*/
void* ELi_CopySectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  セクションをバッファに確保する（コピーしない）
 -----------------------------------------------------*/
void* ELi_AllocSectionToBuffer( ELHandle* ElfHandle, Elf32_Shdr* Shdr);
    

/*------------------------------------------------------
  指定インデックスのセクションヘッダをバッファに取得する
 -----------------------------------------------------*/
void ELi_GetShdr( ELHandle* ElfHandle, u32 index, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  指定インデックスのセクションのエントリをバッファに取得する
 -----------------------------------------------------*/
void ELi_GetSent( ELHandle* ElfHandle, u32 index, void* entry_buf, u32 offset, u32 size);


/*------------------------------------------------------
  指定セクションヘッダの指定インデックスのエントリをバッファに取得する
	（エントリサイズが固定のセクションのみ）
 -----------------------------------------------------*/
void ELi_GetEntry( ELHandle* ElfHandle, Elf32_Shdr* Shdr, u32 index, void* entry_buf);


/*------------------------------------------------------
  STRセクションヘッダの指定インデックスの文字列を取得する
 -----------------------------------------------------*/
void ELi_GetStrAdr( ELHandle* ElfHandle, u32 strsh_index, u32 ent_index, char* str, u32 len);


/*------------------------------------------------------
  シンボルを再定義する
 -----------------------------------------------------*/
void ELi_RelocateSym( ELHandle* ElfHandle, u32 relsh_index);


/*------------------------------------------------------
    makelst専用関数
    シンボルセクションの中からGLOBALなものを
    アドレステーブルに登録する
 -----------------------------------------------------*/
void ELi_DiscriminateGlobalSym( ELHandle* ElfHandle, u32 symsh_index);


/*------------------------------------------------------
  未解決情報をもとにシンボルを解決する
 -----------------------------------------------------*/
u32	ELi_DoRelocate( ELUnresolvedEntry* UnresolvedInfo);


/*------------------------------------------------------
  リストから指定インデックスのELSymExを取り出す
 -----------------------------------------------------*/
ELSymEx* ELi_GetSymExfromList( ELSymEx* SymExStart, u32 index);


/*------------------------------------------------------
  リストから指定インデックスのELShdrExを取り出す
 -----------------------------------------------------*/
ELShdrEx* ELi_GetShdrExfromList( ELShdrEx* ShdrExStart, u32 index);


/*------------------------------------------------------
  指定インデックスのセクションがデバッグ情報かどうか判定する
 -----------------------------------------------------*/
BOOL ELi_ShdrIsDebug( ELHandle* ElfHandle, u32 index);


/*------------------------------------------------------
  ElfHandleのSymExテーブルを調べ、指定インデックスの
　指定オフセットにあるコードがARMかTHUMBかを判定する
 -----------------------------------------------------*/
u32 ELi_CodeIsThumb( ELHandle* ElfHandle, u16 sh_index, u32 offset);


/*---------------------------------------------------------
 未解決情報エントリを初期化する
 --------------------------------------------------------*/
void ELi_UnresolvedInfoInit( ELUnresolvedEntry* UnresolvedInfo);


/*------------------------------------------------------
  未解決情報テーブルからエントリを削除する
 -----------------------------------------------------*/
BOOL ELi_RemoveUnresolvedEntry( ELUnresolvedEntry* UnrEnt);


/*---------------------------------------------------------
 未解決情報テーブルにエントリを追加する
 --------------------------------------------------------*/
void ELi_AddUnresolvedEntry( ELUnresolvedEntry* UnrEnt);


/*------------------------------------------------------
  未解決情報テーブルから指定文字列に該当するエントリを探す
 -----------------------------------------------------*/
ELUnresolvedEntry* ELi_GetUnresolvedEntry( char* ent_name);


#endif	/*_LOADER_SUBSET_H_*/
