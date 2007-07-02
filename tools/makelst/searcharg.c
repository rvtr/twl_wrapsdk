
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "searcharg.h"

/*
char* opt[7];
char* opt_arg[7];
char* arg[7];

int opt_num;
int arg_num;
*/
char opt_name_null[1] = { '\0'};

static void SA_insertOpt( SAArgInfo* ArgInfo, char* opt_name);
static void SA_insertName( SAArgInfo* ArgInfo, char* name);


/*引数を解析する*/
void SA_searchopt( SAArgInfo* ArgInfo, int argc, char* argv[])
{
    int i;

    /* 0で初期化 */
    memset( ArgInfo, 0, sizeof( SAArgInfo));
    
    for( i=1; i<argc; i++) {
        if( strncmp( argv[i], "-", 1) == 0) {
            SA_insertOpt( ArgInfo, argv[i]);
//            printf( "option added(%s)\n", argv[i]);
        }else{
            SA_insertName( ArgInfo, argv[i]);
//            printf( "name added(%s)\n", argv[i]);
        }
    }
}

/*オプション文字列が存在すれば構造体のアドレスを返す*/
SAOptList* SA_IsThereOpt( SAArgInfo* ArgInfo, char* opt_name)
{
    int        i;
    SAOptList* CurrentOptList;

    CurrentOptList = ArgInfo->OptList;
    for( i=0; i<(ArgInfo->opt_num); i++) {
        if( strcmp( CurrentOptList->opt_name, opt_name) == 0) {
            return( CurrentOptList);
        }
        CurrentOptList = CurrentOptList->next;
    }
    return( NULL);
}

/*引数の解析結果を表示する*/
void SA_printf( SAArgInfo* ArgInfo)
{
    int         i, j;
    SAOptList*  CurrentOptList;
    SANameList* CurrentNameList;

    CurrentOptList = (SAOptList*)(ArgInfo->OptList);
    printf( "opt_num = %d\n", ArgInfo->opt_num);

    for( i=0; i<(ArgInfo->opt_num); i++) {
        
        CurrentNameList = (SANameList*)(CurrentOptList->NameList);
        printf( "option[%s] ...", CurrentOptList->opt_name);
        
        for( j=0; j<(CurrentOptList->name_num); j++) {
            printf( " %s", CurrentNameList->name);
            CurrentNameList = CurrentNameList->next;
        }
        printf( "\n");
        
        CurrentOptList = CurrentOptList->next;
    }
}

/**/
static void SA_insertOpt( SAArgInfo* ArgInfo, char* opt_name)
{
    SAOptList  DmyOptList;
    SAOptList* CurrentOptList;

    DmyOptList.next = ArgInfo->OptList;
    CurrentOptList = &DmyOptList;
    while( (CurrentOptList->next) != NULL) {
        CurrentOptList = (SAOptList*)(CurrentOptList->next);
    }
    CurrentOptList->next = (SAOptList*)malloc( sizeof( SAOptList));
    CurrentOptList = CurrentOptList->next;
    /*--- 構築 ---*/
    CurrentOptList->opt_name = opt_name;
    CurrentOptList->name_num = 0;
    CurrentOptList->NameList = NULL;
    CurrentOptList->next     = NULL;
    /*------------*/
    ArgInfo->opt_num++;
    
    if( ArgInfo->OptList == NULL) {
        ArgInfo->OptList = CurrentOptList;
    }
}

/**/
static void SA_insertName( SAArgInfo* ArgInfo, char* name)
{
    SAOptList   DmyOptList;
    SAOptList*  CurrentOptList;
    SANameList  DmyNameList;
    SANameList* CurrentNameList;

    /*単独で見つかった場合はNULL文字のオプションに属させる*/
    if( ArgInfo->OptList == NULL) {
        SA_insertOpt( ArgInfo, opt_name_null);
    }
    
    DmyOptList.next = ArgInfo->OptList;
    CurrentOptList = &DmyOptList;
    while( (CurrentOptList->next) != NULL) {
        CurrentOptList = (SAOptList*)(CurrentOptList->next);
    }

    DmyNameList.next = CurrentOptList->NameList;
    CurrentNameList = &(DmyNameList);
    while( (CurrentNameList->next) != NULL) {
        CurrentNameList = (SANameList*)(CurrentNameList->next);
    }

    CurrentNameList->next = (SANameList*)malloc( sizeof( SANameList));
    CurrentNameList = CurrentNameList->next;
    CurrentNameList->name = name;
    CurrentNameList->next = NULL;
    CurrentOptList->name_num++;

    if( CurrentOptList->NameList == NULL) {
        CurrentOptList->NameList = CurrentNameList;
    }
}


/*
void SA_searchopt( SAArgList* ArgList, int argc, char* argv[])
{
    int i, j, k;

    j = 0;
    k = 0;
    for( i=1; i<argc; i++) {
        if( strncmp( argv[i], "-", 1) == 0) {
            ArgList->opt[j] = malloc( strlen( argv[i])+1);
            strcpy( ArgList->opt[j], argv[i]);
            if( i+1 == argc) {
                j++;
                break;
            }
            if( strncmp( argv[i+1], "-", 1) != 0) {
	            i++;
	            ArgList->opt_arg[j] = malloc( strlen( argv[i])+1);
	            strcpy( ArgList->opt_arg[j], argv[i]);
            }
            j++;
        }else{
            ArgList->arg[k] = malloc( strlen( argv[i])+1);
            strcpy( ArgList->arg[k], argv[i]);
            k++;
        }
    }
    ArgList->opt_num = j;
    ArgList->arg_num = k;

    printf( "argc num : %x\n", argc);
    printf( "opt num : %x, arg num : %x\n", j, k);
    for( i=0; i<j; i++) {
        printf( "%s ... %s\n", ArgList->opt[i], ArgList->opt_arg[i]);
    }
    for( i=0; i<k; i++) {
        printf( "%s\n", ArgList->arg[i]);
    }
}
*/
void portoption( void)
{
//    if( k != 2) {
        printf( "error : too few input files.\n\n");
//    }
}
