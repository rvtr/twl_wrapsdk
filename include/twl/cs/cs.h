
#ifndef __CS_H__
#define __CS_H__

#include <twl.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         CS_Sjis2Unicode

  Description:  

  Arguments:    

  Returns:      < 0 : success( return string length)
                > 0 : error code
 *---------------------------------------------------------------------------*/
int CS_Sjis2Unicode( void* uni_str, void* sjis_str);


#ifdef __cplusplus
}    /* extern "C" */
#endif


#endif /*__CS_H__*/
