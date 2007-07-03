
#include "arch.h"


/*---------------------------------------------------------
 �G���g���w�b�_����G���g���̃T�C�Y�����߂�
 --------------------------------------------------------*/
u32 AR_GetEntrySize( ArchHdr* ArHdr)
{
    u16 i;
    u32 digit = 1;
    u32 size = 0;

    /*----- �������邩���ׂ� -----*/
    for( i=0; i<10; i++) {
        if( ArHdr->ar_size[i] == 0x20) {
            break;
        }else{
            digit *= 10;
        }
    }
    digit /= 10;
    /*----------------------------*/

    /*----- �T�C�Y���Z�o���� -----*/
    for( i=0; i<10; i++) {
        size += (*(((u8*)(ArHdr->ar_size))+i) - 0x30) * digit;	//char��u8�ɕϊ�
        if( digit == 1) {
            break;
        }else{
	        digit /= 10;
        }
    }
    /*----------------------------*/
    
    return size;
}


