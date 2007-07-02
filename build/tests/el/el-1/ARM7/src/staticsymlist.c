/*This file generated automatically by the "makelst".*/

#ifndef __STATIC_SYM_LIST__
#define __STATIC_SYM_LIST__

#include <twl.h>
#include <el/elf_loader.h>

/*--------------------------------
  extern symbol
 --------------------------------*/


/*--------------------------------
  symbol structure
 --------------------------------*/
ElAdrEntry AdrEnt_OS_TPrintf = {
    (void*)NULL,
    (char*)"OS_TPrintf\0", 
    (void*)OS_TPrintf,
    1,
    0,
};
ElAdrEntry AdrEnt_OS_Printf = {
    (void*)NULL,
    (char*)"OS_Printf\0", 
    (void*)OS_Printf,
    1,
    0,
};


/*--------------------------------
  API
 --------------------------------*/
void elAddStaticSym( void)
{
    elAddAdrEntry( &AdrEnt_OS_TPrintf);
    elAddAdrEntry( &AdrEnt_OS_Printf);
}

#endif /*__STATIC_SYM_LIST__*/
