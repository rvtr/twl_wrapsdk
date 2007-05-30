/*---------------------------------------------------------------------------*
  Project:  CTR - for RTFS
  File:     drfile.h

  2006 Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef __DR_FILE_H__
#define __DR_FILE_H__


#include <twl.h>
#include <rtfs.h>


/*---------------------------------------------------------------------------*
    構造体（未使用）
 *---------------------------------------------------------------------------*/
/*RTFS用 FATパラメータ*/
typedef struct {
    u32		capacity;	//data areaのサイズ(512Byte単位)

    u32		adjusted_capacity;	//memory_capacityをシリンダ(heads*secptrack)の倍数に調整したサイズ(cylinders*heads*secptrackになる)
    
    u16		heads;
    u16		secptrack;
    u16		cylinders;
} FileSpec;



/*---------------------------------------------------------------------------*
    API
 *---------------------------------------------------------------------------*/
BOOL fileRtfsAttach( PCFD fileDesc, int driveno);




#endif	/*__DR_FILE_H__*/
