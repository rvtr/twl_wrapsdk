/*---------------------------------------------------------------------------*
  Project:  tools - makedsrom
  File:     makesdrom.c

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
#include    <string.h>
#include    <ctype.h>
#include    <getopt.h>              // getopt()
#include    "version.h"
#include    "types.h"
#include    "elf.h"
#include    "elf_loader.h"
#include    "searcharg.h"



#define  DS_ROM_HEADER_SIZE    0x4000

char     c_source_line_str[256];

#define  C_SOURCE_FILENAME    "staticsymlist.c"
FILE*    CSourceFilep;

/*---------------------------------------------------------------------------*
 *  
 *---------------------------------------------------------------------------*/
u32  adr_ALIGN( u32 addr, u32 align_size);
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
    int             i;
    FILE            *FHp;
    u32             binbuf[4];
    ELHandle        ElfH;
    char*           c_filename;
    SAArgInfo       ArgInfo;
    u16             result;

//    printf( "binbuf : %x\n", binbuf);
/*    
    for( i=0; i<argc; i++) {
        fprintf(stdout, "%s\n", argv[i]);
    }
*/

    /*-----------------------------------------------------*/
    dbg_print_flag = 0;
    SA_searchopt( &ArgInfo, argc, argv);
//    SA_printf( &ArgInfo);

    /*オプションが少ないとき*/
    if( argc == 1) {
        printf( "Development Tool - makelst - Make \"C\" source file\n");
        printf( "Build %s\n\n", __DATE__);
        printf( "Usage: makelst [-o output-file] [-static static-files ...] [-dll dll-files ...]\n\n");
        exit( 1);
    }

    /*-d オプションがあればデバッグ表示フラグをセット*/
/*    for( i=0; i<ArgList.opt_num; i++) {
        if( strcmp( ArgList.opt[i], "-d") == 0) {
            if( ArgList.opt_arg[i] == NULL) {
                dbg_print_flag = 1;
                break;
            }else{
                printf( "Illegal argument \"%s\"\n", ArgList.opt_arg[i]);
                exit( 1);
            }
        }
    }*/


    /*---------- 不正な入力を警告 ----------*/
    {
        /*オプションなしの引数チェック*/
        SAOptList*  CurrentOptList;
        CurrentOptList = SA_IsThereOpt( &ArgInfo, "\0");
        if( CurrentOptList != NULL) {
            printf( "invalid argument (%s)\n", CurrentOptList->NameList->name);
            exit( 0);
        }

        /*不正オプションチェック*/
        CurrentOptList = ArgInfo.OptList;
        for( i=0; i<(ArgInfo.opt_num); i++) {
            if( ( strcmp(CurrentOptList->opt_name, "\0"       ) != 0) &&
                ( strcmp(CurrentOptList->opt_name, "-d\0"     ) != 0) &&
                ( strcmp(CurrentOptList->opt_name, "-o\0"     ) != 0) &&
                ( strcmp(CurrentOptList->opt_name, "-dll\0"   ) != 0) &&
                ( strcmp(CurrentOptList->opt_name, "-static\0") != 0)) {
                    printf( "invalid option (%s)\n", CurrentOptList->opt_name);
                    exit( 0);
            }
            CurrentOptList = CurrentOptList->next;
        }
    }
    /*--------------------------------------*/

    EL_Init();
    unresolved_table_block_flag = 0;    //Unresolvedテーブルへの追加禁止を解除
    /*----------- -dllオプション解析 ----------------*/
    {
        SAOptList*  CurrentOptList;
        SANameList* CurrentNameList;
        CurrentOptList = SA_IsThereOpt( &ArgInfo, "-dll\0");

        if( CurrentOptList) {
            
            if( CurrentOptList->name_num == 0) {
                printf( "no input dll file(s).\n");
                exit( 0);
            }
            
            CurrentNameList = CurrentOptList->NameList;
            for( i=0; i<(CurrentOptList->name_num); i++) {
                FHp = fopen( CurrentNameList->name, "rb");
                if( FHp == NULL) {
                    printf( "cannot open file \"%s\".\n", CurrentNameList->name);
                    exit( 1);
                }
                EL_InitHandle( &ElfH);
                result = EL_LoadLibraryfromFile( &ElfH,  FHp, binbuf);
                fclose( FHp);
                
                CurrentNameList = CurrentNameList->next;
            }
        }else{
            printf( "no input dll file(s).\n");
            exit( 0);
        }
    }
    /*-----------------------------------------------*/
    
    EL_ResolveAllLibrary();


    /*------------- -sオプション解析 --------------*/
    unresolved_table_block_flag = 1;    //Unresolvedテーブルに追加しないようにする
    {
        SAOptList*  CurrentOptList;
        SANameList* CurrentNameList;
        CurrentOptList = SA_IsThereOpt( &ArgInfo, "-static\0");

        if( CurrentOptList) {
            
            if( CurrentOptList->name_num == 0) {
                printf( "no input static file(s).\n");
                exit( 0);
            }
            
            CurrentNameList = CurrentOptList->NameList;
            for( i=0; i<(CurrentOptList->name_num); i++) {
                FHp = fopen( CurrentNameList->name, "rb");
                if( FHp == NULL) {
                    printf( "cannot open file \"%s\".\n", CurrentNameList->name);
                    exit( 1);
                }
                EL_InitHandle( &ElfH);
                result = EL_LoadLibraryfromFile( &ElfH,  FHp, binbuf);
                fclose( FHp);
                
                CurrentNameList = CurrentNameList->next;
            }
        }else{
            printf( "no input static file(s).\n");
            exit( 0);
        }
    }
    /*---------------------------------------------*/

    
    /*仮のCソースファイル名生成*/
    c_filename = malloc( strlen( C_SOURCE_FILENAME));
    strcpy( c_filename, C_SOURCE_FILENAME);
    /*-o オプションがあれば指定ファイル名に変更*/
    {
        SAOptList*  CurrentOptList;
        SANameList* CurrentNameList;
        CurrentOptList = SA_IsThereOpt( &ArgInfo, "-o\0");

        if( CurrentOptList) {
            if( CurrentOptList->name_num != 0) {
                CurrentNameList = CurrentOptList->NameList;
                c_filename = CurrentNameList->name;
            }else{
                printf( "error : no filename after \"-o\" option.\n\n");
                exit( 1);
            }
        }
    }
    
    CSourceFilep = fopen( c_filename, "w");
    if( !CSourceFilep) {
        printf( "error : cannot create file \"%s\".\n\n", c_filename);
        exit( 1);
    }
    file_write( "/*This file generated automatically by the \"makelst\".*/\n", CSourceFilep);
    file_write( "\n", CSourceFilep);
    file_write( "#ifndef __STATIC_SYM_LIST__\n", CSourceFilep);
    file_write( "#define __STATIC_SYM_LIST__\n", CSourceFilep);
    file_write( "\n", CSourceFilep);
    file_write( "#include <ctr.h>\n", CSourceFilep);
    file_write( "#include <elf_loader.h>\n", CSourceFilep);
    file_write( "\n", CSourceFilep);
    
    EL_ResolveAllLibrary();                //抽出シンボルのマーキング
    EL_ExtractStaticSym1();                //構造体設定部分
    file_write( "\n\n", CSourceFilep);
    
    file_write( "/*--------------------------------\n", CSourceFilep);
    file_write( "  API\n", CSourceFilep);
    file_write( " --------------------------------*/\n", CSourceFilep);
    file_write( "void elAddStaticSym( void)\n", CSourceFilep);
    file_write( "{\n", CSourceFilep);
    EL_ExtractStaticSym2();                //API呼び出し部分
    file_write( "}\n", CSourceFilep);
    
    unresolved_table_block_flag = 0;    //Unresolvedテーブルへの追加禁止を解除
    
    file_write( "\n", CSourceFilep);
    file_write( "#endif /*__STATIC_SYM_LIST__*/\n", CSourceFilep);
    fclose( CSourceFilep);
    /*---------------------------------------------*/

    printf( "\"C\" source file \"%s\" is generated.\n\n", c_filename);
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
