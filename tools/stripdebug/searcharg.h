
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


void SA_searchopt( SAArgList* ArgList, int argc, char* argv[]);



#endif /*__SEARCH_ARG__*/
