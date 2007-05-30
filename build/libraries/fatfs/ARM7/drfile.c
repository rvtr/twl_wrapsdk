/*---------------------------------------------------------------------------*
  Project:  CTR - for RTFS
  File:     drfile.c

  2006 Nintendo.
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <rtfs_target_os.h>
#include <rtfs.h>
#include <rtfsconf.h>

#if (RTFS_DEBUG_PRINT_ON == 1)
	#if (CTR_DEF_ENVIRONMENT_DSEMU == 1)
		#define PRINTDEBUG	osTPrintf
	#else
		#include <ctr/vlink.h>
		#define PRINTDEBUG	vlink_dos_printf
	#endif
#else
	#define PRINTDEBUG	i_no_print
	static void i_no_print( const char *fmt, ... );
	static void i_no_print( const char *fmt, ... ){ return; }
#endif


/*---------------------------------------------------------------------------*
    global変数
 *---------------------------------------------------------------------------*/
PCFD	fileDescList[26] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

u32	file_capacity;
u32	file_adjusted_capacity;
u16	file_heads;
u16	file_secptrack;
u16	file_cylinders;


/*---------------------------------------------------------------------------*
  extern変数
 *---------------------------------------------------------------------------*/
extern int		rtfs_first_stat_flag[26];


/*---------------------------------------------------------------------------*
    static関数
 *---------------------------------------------------------------------------*/
static void file_get_CHS_params( u32 file_sector_num);


/*---------------------------------------------------------------------------*
  Name:         fileRtfsIo

  Description:  上位層からのセクタリード／ライト要求を受ける

  Arguments:    driveno : ドライブ番号
    			block : 開始ブロック番号
    			buffer : 
    			count : ブロック数
    			reading : リード要求時にTRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
BOOL fileRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading)
{
    int result;
    unsigned long dmy;
    
    po_ulseek( fileDescList[driveno], block*512, &dmy, PSEEK_SET);
    if( reading) {
		PRINTDEBUG( "DEVCTL_IO_READ ... block:%x, count:%x -> buf:%x\n", block, count, buffer);
        result = po_read( fileDescList[driveno], buffer, 512*count);
    }else{
		PRINTDEBUG( "DEVCTL_IO_WRITE ... block:%x, count:%x <- buf:%x\n", block, count, buffer);
        result = po_write( fileDescList[driveno], buffer, 512*count);
    }

    if( result < 0) {
	    return FALSE;
    }else{
        return TRUE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         fileRtfsCtrl

  Description:  上位層からのコントロール要求を受ける

  Arguments:    driveno : ドライブ番号
    			opcode : 要求の種類
    			pargs : 

  Returns:      
 *---------------------------------------------------------------------------*/
int fileRtfsCtrl( int driveno, int opcode, void* pargs)
{
    DDRIVE			*pdr;
    DEV_GEOMETRY	gc;
    int				heads, secptrack;
    unsigned long	file_size, dmy;

    pdr = pc_drno_to_drive_struct( driveno);
    
    switch( opcode) {
      case DEVCTL_GET_GEOMETRY:
        PRINTDEBUG( "%s : DEVCTL_GET_GEOMETRY\n", __FUNCTION__);
        rtfs_memset( &gc, (byte)0, sizeof(gc));

        po_ulseek( fileDescList[driveno], 0, &dmy, PSEEK_SET);
        po_ulseek( fileDescList[driveno], 0, &file_size, PSEEK_END);
        po_ulseek( fileDescList[driveno], 0, &dmy, PSEEK_SET);

        file_capacity = file_size / 512;	//セクタ単位にする

        file_get_CHS_params( file_capacity);

        PRINTDEBUG( "capacity  : 0x%x\n", file_adjusted_capacity);
        PRINTDEBUG( "heads     : 0x%x\n", file_heads);
        PRINTDEBUG( "cylinders : 0x%x\n", file_cylinders);
        PRINTDEBUG( "secptrack : 0x%x\n", file_secptrack);
        
        gc.dev_geometry_lbas = file_adjusted_capacity;
	    gc.dev_geometry_heads 		= file_heads;
        gc.dev_geometry_cylinders 	= file_cylinders;
        gc.dev_geometry_secptrack 	= file_secptrack;
        /**/
        gc.fmt_parms_valid 	= FALSE;
        OSAPI_CPUCOPY8( &gc, pargs, sizeof(gc));
        return( 0);
        
      case DEVCTL_FORMAT:
        {
/*            u32 filebuf[512/4];

            miCpuFill8( filebuf, 0, 512);
            po_write( fileDescList[driveno], filebuf, 512);*/
	        PRINTDEBUG( "%s : DEVCTL_FORMAT\n", __FUNCTION__);
	        return( 0);
        }

      case DEVCTL_REPORT_REMOVE:
        pdr->drive_flags &= ~DRIVE_FLAGS_INSERTED;
        return( 0);
        
      case DEVCTL_CHECKSTATUS:
        PRINTDEBUG( "%s : DEVCTL_CHECKSTATUS\n", __FUNCTION__);
        if (!(pdr->drive_flags & DRIVE_FLAGS_REMOVABLE)) {	//リムーバブルでない場合
            return(DEVTEST_NOCHANGE);	//fix
        }else{												//リムーバブルの場合
 	    	if( rtfs_first_stat_flag[driveno]) {
            	rtfs_first_stat_flag[driveno] = 0;
            	PRINTDEBUG( "CHANGED!\n");
            	pdr->drive_flags |= DRIVE_FLAGS_INSERTED;
            	return(DEVTEST_CHANGED);
	        }
	        pdr->drive_flags |= DRIVE_FLAGS_INSERTED;
	        return( DEVTEST_NOCHANGE);
        }
        
      case DEVCTL_WARMSTART:
        PRINTDEBUG( "%s : DEVCTL_WARMSTART\n", __FUNCTION__);
        pdr->drive_flags |= (DRIVE_FLAGS_VALID | DRIVE_FLAGS_REMOVABLE);
        return( 0);
        
      case DEVCTL_POWER_RESTORE:
        PRINTDEBUG( "%s : DEVCTL_POWER_RESTORE\n", __FUNCTION__);
        break;
        
      case DEVCTL_POWER_LOSS:
        PRINTDEBUG( "%s : DEVCTL_POWER_LOSS\n", __FUNCTION__);
        break;
        
      default:
        PRINTDEBUG( "%s : DEVCTL_unknown\n", __FUNCTION__);
        break;
    }
    return( 0);
}


/*SD File System Specification(仕様書)に基づいた値を出す*/
static void file_get_CHS_params( u32 file_sector_num)
{
    int mbytes;

//    mbytes = (sdmc_current_spec.card_capacity / (1024 * 1024)) * 512;
    mbytes = (file_sector_num >> 11);

    while( 1) {
	    if( mbytes <= 2) {
	        file_heads 	= 2;
	        file_secptrack = 16;
	        break;
	    }
	    if( mbytes <= 16) {
	        file_heads 	= 2;
	        file_secptrack = 32;
	        break;
	    }
	    if( mbytes <= 32) {
	        file_heads 	= 4;
	        file_secptrack = 32;
	        break;
	    }
	    if( mbytes <= 128) {
	        file_heads 	= 8;
	        file_secptrack = 32;
	        break;
	    }
	    if( mbytes <= 256) {
	        file_heads 	= 16;
	        file_secptrack = 32;
	        break;
	    }
	    if( mbytes <= 504) {
	        file_heads 	= 16;
	        file_secptrack = 63;
	        break;
	    }
	    if( mbytes <= 1008) {
	        file_heads 	= 32;
	        file_secptrack = 63;
	        break;
	    }
	    if( mbytes <= 2016) {
	        file_heads 	= 64;
	        file_secptrack = 63;
	        break;
	    }
	    if( mbytes <= 2048) {
	        file_heads 	= 128;
	        file_secptrack = 63;
	        break;
	    }
	    if( mbytes <= 4032) {
	        file_heads 	= 128;
	        file_secptrack = 63;
	        break;
	    }
	    if( mbytes <= 32768) {
	        file_heads 	= 255;
	        file_secptrack = 63;
	        break;
	    }
    }

    /*シリンダ数を計算*/
    file_cylinders = (file_sector_num /	(file_heads * file_secptrack));
    
    /*memory_capacityを再計算してadjusted_memory_capacityに格納*/
    file_adjusted_capacity = file_cylinders *
				        		(file_heads * file_secptrack);
}


/*---------------------------------------------------------------------------*
  Name:         fileRtfsAttach

  Description:  fileドライバをドライブに割り当てる

  Arguments:    driveno : ドライブ番号

  Returns:      
 *---------------------------------------------------------------------------*/
BOOL fileRtfsAttach( PCFD fileDesc, int driveno)
{
    BOOLEAN	result;
    DDRIVE	pdr;

    pdr.dev_table_drive_io     = fileRtfsIo;
    pdr.dev_table_perform_device_ioctl = fileRtfsCtrl;
    pdr.register_file_address  = (dword) 0; /* Not used  */
    pdr.interrupt_number       = 0;    		/* Not used */
    pdr.drive_flags            = DRIVE_FLAGS_VALID;
    pdr.partition_number       = 0;    		/* Not used */
    pdr.pcmcia_slot_number     = 0;    		/* Not used */
    pdr.controller_number      = 0;
    pdr.logical_unit_number    = 0;

    fileDescList[driveno] = fileDesc;				/* file descripter セット */
    
    result = rtfs_attach( driveno, &pdr, "FILE");	//構造体がFSライブラリ側にコピーされる
    /**/
    if( !result) {
        PRINTDEBUG( "fsEnableDevice failured\n");
    }

    return( result);
}


