
#ifndef	__SEARCH_ARG__
#define __SEARCH_ARG__



typedef struct
{
    char*	opt[7];
    char*	opt_arg[7];
    char*	arg[7];
    int		opt_num;
    int		arg_num;
}SAArgList;


/**/
typedef struct
{
    void* next;
    char* name;
}SANameList;

typedef struct
{
    void*       next;
    char*       opt_name;
    SANameList* NameList;
    int         name_num;
}SAOptList;


typedef struct
{
    SAOptList* OptList;
    int        opt_num;
}SAArgInfo;



/*引数を解析する*/
void SA_searchopt( SAArgInfo* ArgInfo, int argc, char* argv[]);

/*オプション文字列が存在すれば構造体のアドレスを返す*/
SAOptList* SA_IsThereOpt( SAArgInfo* ArgInfo, char* opt_name);

/*引数の解析結果を表示する*/
void SA_printf( SAArgInfo* ArgInfo);



//void SA_searchopt( SAArgList* ArgList, int argc, char* argv[]);
    

#endif /*__SEARCH_ARG__*/
