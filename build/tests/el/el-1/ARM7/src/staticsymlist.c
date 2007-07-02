/*This file generated automatically by the "makelst".*/

#ifndef __STATIC_SYM_LIST__
#define __STATIC_SYM_LIST__

#include <twl_sp.h>
#include <el/elf_loader.h>

/*--------------------------------
  extern symbol
 --------------------------------*/
extern int no_data;
extern void __aeabi_unwind_cpp_pr0( void);
//extern void Lib$$Request$$armlib( void);

/*--------------------------------
  symbol structure
 --------------------------------*/
ElAdrEntry AdrEnt_no_data = {
    (void*)NULL,
    (char*)"no_data\0", 
    (void*)&no_data,
    0,
    0,
};
/*
ElAdrEntry AdrEnt_Lib$$Request$$armlib = {
    (void*)NULL,
    (char*)"Lib$$Request$$armlib\0",
    (void*)Lib$$Request$$armlib,
    1,
    0,
};*/

/*--------------------------------
  API
 --------------------------------*/
void elAddStaticSym( void)
{
    elAddAdrEntry( &AdrEnt_no_data);
//    elAddAdrEntry( &AdrEnt_Lib$$Request$$armlib);
}

#endif /*__STATIC_SYM_LIST__*/
