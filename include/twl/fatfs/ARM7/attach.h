/*---------------------------------------------------------------------------*
  Project:  CTR - for RTFS
  File:     attach.h

  2006 Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef	__ATTACH_H__
#define __ATTACH_H__


#include <twl.h>
#include <rtfs.h>



/*---------------------------------------------------------------------------*
    API
 *---------------------------------------------------------------------------*/
BOOLEAN rtfs_attach( int driveno, DDRIVE* pdr, char* dev_name);
BOOLEAN rtfs_detach( int driveno);



#endif	/*__ATTACH_H__*/
