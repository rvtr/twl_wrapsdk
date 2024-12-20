
#include "arch.h"


/*---------------------------------------------------------
 エントリヘッダからエントリのサイズを求める
 --------------------------------------------------------*/
u32 AR_GetEntrySize( ArchHdr* ArHdr)
{
    u16 i;
    u32 digit = 1;
    u32 size = 0;

    /*----- 何桁あるか調べる -----*/
    for( i=0; i<10; i++) {
        if( ArHdr->ar_size[i] == 0x20) {
            break;
        }else{
            digit *= 10;
        }
    }
    digit /= 10;
    /*----------------------------*/

    /*----- サイズを算出する -----*/
    for( i=0; i<10; i++) {
        size += (*(((u8*)(ArHdr->ar_size))+i) - 0x30) * digit;	//charをu8に変換
        if( digit == 1) {
            break;
        }else{
	        digit /= 10;
        }
    }
    /*----------------------------*/
    
    return size;
}


