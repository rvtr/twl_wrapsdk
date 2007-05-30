/*---------------------------------------------------------------------------*
  Project:  CTR - for RTFS
  File:     drdefault.c

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
  Name:         defaultRtfsIo

  Description:  上位層からのセクタリード／ライト要求を受ける

  Arguments:    driveno : ドライブ番号
    			block : 開始ブロック番号
    			buffer : 
    			count : ブロック数
    			reading : リード要求時にTRUE

  Returns:      TRUE/FALSE
 *---------------------------------------------------------------------------*/
BOOL defaultRtfsIo( int driveno, dword block, void* buffer, word count, BOOLEAN reading)
{
    if( reading) {
		PRINTDEBUG( "DEVCTL_IO_READ ... block:%x, count:%x -> buf:%x\n", block, count, buffer);
    }else{
		PRINTDEBUG( "DEVCTL_IO_WRITE ... block:%x, count:%x <- buf:%x\n", block, count, buffer);
    }
    
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         defaultRtfsCtrl

  Description:  上位層からのコントロール要求を受ける

  Arguments:    driveno : ドライブ番号
    			opcode : 要求の種類
    			pargs : 

  Returns:      
 *---------------------------------------------------------------------------*/
int defaultRtfsCtrl( int driveno, int opcode, void* pargs)
{
    DDRIVE			*pdr;
    DEV_GEOMETRY	gc;
    int				heads, secptrack;

    pdr = pc_drno_to_drive_struct( driveno);
    
    switch( opcode) {
      case DEVCTL_GET_GEOMETRY:
        PRINTDEBUG( "DEVCTL_GET_GEOMETRY\n");
        rtfs_memset( &gc, (byte)0, sizeof(gc));
        
        gc.dev_geometry_lbas = 0;
	    gc.dev_geometry_heads 		= 0;
        gc.dev_geometry_cylinders 	= 0;
        gc.dev_geometry_secptrack 	= 0;
        /**/
        gc.fmt_parms_valid 	= FALSE;
        OSAPI_CPUCOPY8( &gc, pargs, sizeof(gc));
        return( -1);	//fix
        
      case DEVCTL_FORMAT:
        PRINTDEBUG( "DEVCTL_FORMAT\n");
        break;

      case DEVCTL_REPORT_REMOVE:
        pdr->drive_flags &= ~DRIVE_FLAGS_INSERTED;
        break;
        
      case DEVCTL_CHECKSTATUS:
        PRINTDEBUG( "DEVCTL_CHECKSTATUS\n");
        if (!(pdr->drive_flags & DRIVE_FLAGS_REMOVABLE)) {	//リムーバブルでない場合
            return(DEVTEST_NOCHANGE);	//fix
        }else{												//リムーバブルの場合
            return(DEVTEST_NOMEDIA);	//fix
        }
        
      case DEVCTL_WARMSTART:
        PRINTDEBUG( "DEVCTL_WARMSTART\n");
        pdr->drive_flags |= (/*DRIVE_FLAGS_VALID |*/ DRIVE_FLAGS_REMOVABLE);
        pdr->partition_number = 0;
        return( 0);	//fix
        
      case DEVCTL_POWER_RESTORE:
        PRINTDEBUG( "DEVCTL_POWER_RESTORE\n");
        break;
        
      case DEVCTL_POWER_LOSS:
        PRINTDEBUG( "DEVCTL_POWER_LOSS\n");
        break;
        
      default:
        PRINTDEBUG( "DEVCTL_unknown\n");
        break;
    }
    return( 0);
}

/*---------------------------------------------------------------------------*
  Name:         defaultRtfsAttach

  Description:  defaultドライバをドライブに割り当てる

  Arguments:    driveno : ドライブ番号

  Returns:      
 *---------------------------------------------------------------------------*/
/* rtfs_detachでdefaultドライバが割り当てられるため、このAPIは必要ない*/
/*
void defaultRtfsAttach( int driveno)
{
    BOOLEAN	result;
    DDRIVE	pdr;

    pdr.dev_table_drive_io     = defaultRtfsIo;
    pdr.dev_table_perform_device_ioctl = defaultRtfsCtrl;
    pdr.register_file_address  = (dword) 0; // Not used
    pdr.interrupt_number       = 0;    		// Not used
    pdr.drive_flags            = 0;			// DRIVE_FLAGS_VALID をセットしない
    pdr.partition_number       = 0;    		// Not used
    pdr.pcmcia_slot_number     = 0;    		// Not used
    pdr.controller_number      = 0;
    pdr.logical_unit_number    = 0;
    

    //REMOVE扱い
    pdr.drive_flags |= DRIVE_FLAGS_REMOVABLE;
    pdr.drive_flags &= ~DRIVE_FLAGS_INSERTED;

    
    result = rtfs_attach( driveno, &pdr, "DEFAULT");	//構造体がFSライブラリ側にコピーされる
    if( !result) {
        PRINTDEBUG( "fsEnableDevice failured\n");
    }
}
*/
