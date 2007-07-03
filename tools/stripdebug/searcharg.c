
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
/*
    printf( "argc num : %x\n", argc);
    printf( "opt num : %x, arg num : %x\n", j, k);
    for( i=0; i<j; i++) {
        printf( "%s ... %s\n", ArgList->opt[i], ArgList->opt_arg[i]);
    }
    for( i=0; i<k; i++) {
        printf( "%s\n", ArgList->arg[i]);
    }*/
}

void portoption( void)
{
//    if( k != 2) {
        printf( "error : too few input files.\n\n");
//    }
}
