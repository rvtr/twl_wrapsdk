/*---------------------------------------------------------------------------*
  Project:  CTR - ELF Loader
  File:     loader_subset.h

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef _LOADER_SUBSET_H_
#define _LOADER_SUBSET_H_

#include "elf.h"
#include "elf_loader.h"


/*------------------------------------------------------
  ベニヤをバッファにコピーする
    start : 呼び出し元アドレス
    data : 飛び先アドレス
 -----------------------------------------------------*/
void* i_elCopyVeneerToBuffer( ElDesc* elElfDesc, u32 start, u32 data, s32 threshold);


/*------------------------------------------------------
  ベニヤをバッファにコピーする
    start : 呼び出し元
    data : 飛び先
    threshold : この範囲内に既にベニヤがあれば使いまわす
 -----------------------------------------------------*/
void* i_elCopyV4tVeneerToBuffer( ElDesc* elElfDesc, u32 start, u32 data, s32 threshold);


/*------------------------------------------------------
  セグメントをバッファにコピーする
 -----------------------------------------------------*/
void* i_elCopySegmentToBuffer( ElDesc* elElfDesc, Elf32_Phdr* Phdr);


/*------------------------------------------------------
  セクションをバッファにコピーする
 -----------------------------------------------------*/
void* i_elCopySectionToBuffer( ElDesc* elElfDesc, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  セクションをバッファに確保する（コピーしない）
 -----------------------------------------------------*/
void* i_elAllocSectionToBuffer( ElDesc* elElfDesc, Elf32_Shdr* Shdr);
    

/*------------------------------------------------------
  指定インデックスのプログラムヘッダをバッファに取得する
 -----------------------------------------------------*/
void i_elGetPhdr( ElDesc* elElfDesc, u32 index, Elf32_Phdr* Phdr);


/*------------------------------------------------------
  指定インデックスのセクションヘッダをバッファに取得する
 -----------------------------------------------------*/
void i_elGetShdr( ElDesc* elElfDesc, u32 index, Elf32_Shdr* Shdr);


/*------------------------------------------------------
  指定インデックスのセクションのエントリをバッファに取得する
 -----------------------------------------------------*/
void i_elGetSent( ElDesc* elElfDesc, u32 index, void* entry_buf, u32 offset, u32 size);


/*------------------------------------------------------
  指定セクションヘッダの指定インデックスのエントリをバッファに取得する
  （エントリサイズが固定のセクションのみ）
 -----------------------------------------------------*/
void i_elGetEntry( ElDesc* elElfDesc, Elf32_Shdr* Shdr, u32 index, void* entry_buf);


/*------------------------------------------------------
  STRセクションヘッダの指定インデックスの文字列を取得する
 -----------------------------------------------------*/
void i_elGetStrAdr( ElDesc* elElfDesc, u32 strsh_index, u32 ent_index, char* str, u32 len);


/*------------------------------------------------------
  シンボルを再定義する
 -----------------------------------------------------*/
void i_elRelocateSym( ElDesc* elElfDesc, u32 relsh_index);

/*------------------------------------------------------
  グローバルシンボルをアドレステーブルに入れる
 -----------------------------------------------------*/
void i_elGoPublicGlobalSym( ElDesc* elElfDesc, u32 symtblsh_index);

/*------------------------------------------------------
  i_elRelocateSymやi_elGoPublicGlobalSymの中で作成したシンボルリストを
  開放する（どちらも呼び終わったら最後にこのAPIを呼んでください）
 -----------------------------------------------------*/
void i_elFreeSymList( ElDesc* elElfDesc);

/*------------------------------------------------------
  未解決情報をもとにシンボルを解決する
 -----------------------------------------------------*/
BOOL i_elDoRelocate( ElDesc* elElfDesc, ElUnresolvedEntry* UnresolvedInfo);


/*------------------------------------------------------
  リストから指定インデックスのElSymExを取り出す
 -----------------------------------------------------*/
//ElSymEx* i_elGetSymExfromList( ElSymEx* SymExStart, u32 index);


/*------------------------------------------------------
  リストから指定インデックスのElShdrExを取り出す
 -----------------------------------------------------*/
ElShdrEx* i_elGetShdrExfromList( ElShdrEx* ShdrExStart, u32 index);


/*------------------------------------------------------
  指定インデックスのセクションがデバッグ情報かどうか判定する
 -----------------------------------------------------*/
BOOL i_elShdrIsDebug( ElDesc* elElfDesc, u32 index);


/*------------------------------------------------------
  elElfDescのSymExテーブルを調べ、指定インデックスの
　指定オフセットにあるコードがARMかTHUMBかを判定する
 -----------------------------------------------------*/
u32 i_elCodeIsThumb( ElDesc* elElfDesc, u16 sh_index, u32 offset);


/*---------------------------------------------------------
 未解決情報エントリを初期化する
 --------------------------------------------------------*/
//void i_elUnresolvedInfoInit( ElUnresolvedEntry* UnresolvedInfo);


/*---------------------------------------------------------
 未解決情報テーブルにエントリを追加する
 --------------------------------------------------------*/
//void i_elAddUnresolvedEntry( ElUnresolvedEntry* UnrEnt);


void*     i_elFreeVenTbl( void);


#endif /*_LOADER_SUBSET_H_*/
