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
    �\���́i���g�p�j
 *---------------------------------------------------------------------------*/
/*RTFS�p FAT�p�����[�^*/
typedef struct {
    u32		capacity;	//data area�̃T�C�Y(512Byte�P��)

    u32		adjusted_capacity;	//memory_capacity���V�����_(heads*secptrack)�̔{���ɒ��������T�C�Y(cylinders*heads*secptrack�ɂȂ�)
    
    u16		heads;
    u16		secptrack;
    u16		cylinders;
} FileSpec;



/*---------------------------------------------------------------------------*
    API
 *---------------------------------------------------------------------------*/
BOOL fileRtfsAttach( PCFD fileDesc, int driveno);




#endif	/*__DR_FILE_H__*/
