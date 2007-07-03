/*---------------------------------------------------------------------------*
  Project:  tools - makedsrom
  File:     stripdebug.c

  Copyright 2006,2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.


  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include    <stdio.h>
#include    <stdlib.h>              // atoi()
#include    <ctype.h>
#include    <getopt.h>              // getopt()
#include    <string.h>
#include    "version.h"
#include    "types.h"
#include    "elf.h"
#include    "elf_loader.h"
#include    "searcharg.h"



#define    DS_ROM_HEADER_SIZE    0x4000

char       c_source_line_str[256];

#define    STRIPPED_ELF_FILENAME    "stripped-"
FILE*      NewElfFilep;

/*---------------------------------------------------------------------------*
 *  
 *---------------------------------------------------------------------------*/
u32 adr_ALIGN( u32 addr, u32 align_size);
void file_write( char* c_str, FILE* Fp);


/*---------------------------------------------------------------------------*
 *  
 *---------------------------------------------------------------------------*/
u16        dbg_print_flag;
u16        unresolved_table_block_flag = 0;

/*---------------------------------------------------------------------------*
 *  MAIN
 *---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int         i, j, k;
    int         n;
    int         narg;
    int         t;
    FILE        *FHp;
    u32         *elfbuf;
    u32*        newelfbuf;
    u32         elfsize;
    u32         mainp_malloc_size, subp_malloc_size;
    size_t      filesize;
    ELHandle    ElfH;
    u32         loadstart, loadend, loadsize, ramadr;
    u32         entry_address, ram_address;
    u32         sub_loadstart, sub_loadend, sub_loadsize, sub_ramadr;
    u32         sub_entry_address, sub_ram_address;;
    u32         header_buf[DS_ROM_HEADER_SIZE/4];
    char*       elf_filename;
    u32         elf_namesize;
    char*       slash_pointer;
    SAArgList   ArgList;
    u16         result;
    

//    printf( "binbuf : %x\n", binbuf);
/*    
    for( i=0; i<argc; i++) {
        fprintf(stdout, "%s\n", argv[i]);
    }
*/

    /*-----------------------------------------------------*/
    dbg_print_flag = 0;
    SA_searchopt( &ArgList, argc, argv);

    /*オプションが少ないとき*/
    if( argc == 1) {
        printf( "Development Tool - stripdebug - strip debug-section from armelf.\n");
        printf( "Build %s\n\n", __DATE__);
        printf( "Usage: stripdebug [-o output-file] dll-file\n\n");
        exit( 1);
    }

    /*-d オプションがあればデバッグ表示フラグをセット*/
    for( i=0; i<ArgList.opt_num; i++) {
        if( strcmp( ArgList.opt[i], "-d") == 0) {
            if( ArgList.opt_arg[i] == NULL) {
                dbg_print_flag = 1;
                break;
            }else{
                printf( "Illegal argument \"%s\"\n", ArgList.opt_arg[i]);
                exit( 1);
            }
        }
    }


    EL_Init();
    unresolved_table_block_flag = 0;    //Unresolvedテーブルへの追加禁止を解除

    /* dllファイルを調べる */
    FHp = fopen( ArgList.arg[0], "rb");
    if( FHp == NULL) {
        printf( "cannot open file \"%s\".\n", ArgList.arg[0]);
        exit( 1);
    }
    fseek( FHp, 0, SEEK_END);
    elfsize = ftell( FHp);
    fseek( FHp, 0, SEEK_SET);
    newelfbuf = (u32*)malloc( elfsize);
    
    printf( "input elf size    = 0x%x\n", elfsize);
    
    EL_InitHandle( &ElfH);
    result = EL_LoadLibraryfromFile( &ElfH,  FHp, newelfbuf);
    fclose( FHp);
//    EL_ResolveAllLibrary();
    /*---------------------------------------------*/

    /*仮のelfファイル名生成*/
    elf_namesize = strlen( STRIPPED_ELF_FILENAME) + strlen( ArgList.arg[0]) + 1;//+1は最後のnull文字
    elf_filename = malloc( elf_namesize);
    memset( elf_filename, 0, elf_namesize);
    
    slash_pointer = strrchr( ArgList.arg[0], '/');
    if( slash_pointer == NULL) {                      //スラッシュがない場合
        strcpy( elf_filename, STRIPPED_ELF_FILENAME); //接頭語
        strcat( elf_filename, ArgList.arg[0]);        //ファイル名
    }else{                                            //スラッシュがある場合
        memcpy( elf_filename, ArgList.arg[0], (slash_pointer - ArgList.arg[0])+1);
        strcat( elf_filename, STRIPPED_ELF_FILENAME);
        strcat( elf_filename, slash_pointer+1);
    }
    /*-o オプションがあれば指定ファイル名に変更*/
    for( i=0; i<ArgList.opt_num; i++) {
        if( strcmp( ArgList.opt[i], "-o") == 0) {
            if( ArgList.opt_arg[i] != NULL) {
                elf_filename = ArgList.opt_arg[i];
                break;
            }else{
                printf( "error : no filename after \"-o\" option.\n\n");
                exit( 1);
            }
        }
    }
    
    NewElfFilep = fopen( elf_filename, "wb");
    if( !NewElfFilep) {
        printf( "error : cannot create file \"%s\".\n\n", elf_filename);
        exit( 1);
    }

    printf( "stripped elf size = 0x%x\n", ElfH.newelf_size);
    fwrite( newelfbuf, 1, ElfH.newelf_size, NewElfFilep);
    
    fclose( NewElfFilep);
    /*---------------------------------------------*/

    printf( "stripped elf file \"%s\" is generated.\n\n", elf_filename);
    exit( 0);

    /*-----------------------------------------------------*/
    
    return 0;
}

/*---------------------------------------------------------------------------*
 *  アドレスのアラインメント
 *---------------------------------------------------------------------------*/
u32 adr_ALIGN( u32 addr, u32 align_size)
{
    u32 aligned_addr;
    
    if( (addr % align_size) == 0) {
        aligned_addr = addr;
    }else{
        aligned_addr = (((addr) & ~((align_size) - 1)) + (align_size));
    }
    
    return aligned_addr;
}

/*---------------------------------------------------------------------------*
 *  ファイルに文字列を書き込み
 *---------------------------------------------------------------------------*/
void file_write( char* c_str, FILE* Fp)
{
    fwrite( c_str, 1, strlen( c_str), Fp);
}
