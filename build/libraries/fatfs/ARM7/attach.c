/*---------------------------------------------------------------------------*
  Project:  CTR - for RTFS
  File:     attach.c

  2006 Nintendo.
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <rtfs.h>
#include <portconf.h>   /* For included devices */

#include "drdefault.h"	/* default driver */


/*---------------------------------------------------------------------------*
    global変数
 *---------------------------------------------------------------------------*/
BOOLEAN rtfs_first_attach = TRUE;	//attach APIがまだ未使用ならTRUE

//attachされてからDEVCTL_CHECKSTATUSが呼ばれてなければ1
int		rtfs_first_stat_flag[26] ={
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};


/*---------------------------------------------------------------------------*
    extern変数
 *---------------------------------------------------------------------------*/
extern int		enabled_drivers;
extern DDRIVE*	current_pdr;
extern int		auto_format_disk(DDRIVE *pdr, byte *drivename);
extern void		drno_to_string(byte *pname, int drno);
extern DDRIVE*	pc_drno_to_drive_struct(int driveno);
extern void		print_device_names(void);


/*---------------------------------------------------------------------------*
    extern関数
 *---------------------------------------------------------------------------*/
/*-------------- ctr modified(new function) -------------*/
//extern BOOLEAN i_rtfs_begin_attach( void);
extern BOOLEAN i_rtfs_attach( int driveno, DDRIVE* pdr, char* dev_name);
extern BOOLEAN i_rtfs_end_attach( void);
/*-------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    static関数
 *---------------------------------------------------------------------------*/
static void i_rtfsInit( DDRIVE* pdr);
static int i_rtfs_close_disk( int driveno);



/*---------------------------------------------------------------------------*
  Name:         rtfs_attach

  Description:  

  Arguments:    driveno : ドライブ番号

  Returns:      
 *---------------------------------------------------------------------------*/
BOOLEAN rtfs_attach( int driveno, DDRIVE* pdr, char* dev_name)
{
    DDRIVE *target_pdr;
    
//    osTPrintf( "original attach\n");
    
    if( rtfs_first_attach) {			//最初のattachだった場合
        rtfs_init();//i_rtfs_begin_attach();
        rtfs_first_attach = FALSE;
    }

    /*アタッチする構造体を選択*/
    target_pdr = pc_drno_to_drive_struct( driveno);
    if( target_pdr == 0) {				/*未登録なら新しい構造体へ*/
//        osTPrintf( "target_pdr = current_pdr\n");
        target_pdr = current_pdr;
    }else{
/*        if( target_pdr->dev_table_drive_io == pdr->dev_table_drive_io) {
            return( TRUE);				//自分自身ならスルー
        }*/
        if(( target_pdr->dev_table_drive_io != defaultRtfsIo)&&
           ( target_pdr->dev_table_drive_io != NULL)){
            return( FALSE);				/*default以外のドライバがattach済みならエラー*/
        }
    }

    if (++enabled_drivers > prtfs_cfg->cfg_NDRIVES) {
//        osTPrintf( "%s : too many drives attached - ");
//        osTPrintf( "%d / %d\n", enabled_drivers, prtfs_cfg->cfg_NDRIVES);
        return( FALSE);
    }

    target_pdr->driveno						= driveno;
    target_pdr->dev_table_drive_io     		= pdr->dev_table_drive_io;
    target_pdr->dev_table_perform_device_ioctl = pdr->dev_table_perform_device_ioctl;
    rtfs_strcpy(target_pdr->device_name, (byte *)dev_name);
    target_pdr->register_file_address  = pdr->register_file_address;
    target_pdr->interrupt_number       = pdr->interrupt_number;
    target_pdr->drive_flags            = pdr->drive_flags | DRIVE_FLAGS_REMOVABLE | DRIVE_FLAGS_INSERTED;
    target_pdr->partition_number       = pdr->partition_number;
    target_pdr->pcmcia_slot_number     = pdr->pcmcia_slot_number;
    target_pdr->controller_number      = pdr->controller_number;
    target_pdr->logical_unit_number    = pdr->logical_unit_number;

    
    rtfs_first_stat_flag[driveno] = 1;	/* 初回DEVCTL_CHECKSTATUSフラグON */
    
    i_rtfsInit( target_pdr);

    /**/
    if( target_pdr == current_pdr) {
	    current_pdr++;
    }
    
    return( TRUE);
}


/**/
static void i_rtfsInit( DDRIVE* pdr)
{
	byte drname[8]; /* Temp buffer for displaying drive letters as strings */

    /* WARMSTART */
    if( pdr->dev_table_drive_io) {
        prtfs_cfg->drno_to_dr_map[pdr->driveno] = pdr; /* MAPS DRIVE structure to DRIVE: */
        if (pdr->dev_table_perform_device_ioctl(pdr->driveno, DEVCTL_WARMSTART, (void *) 0) != 0)
        {
            prtfs_cfg->drno_to_dr_map[pdr->driveno] = 0; /* It is not there.  */
                                                         /* so forget it */
        }
        
#if (INCLUDE_FAILSAFE_CODE)
        /* Call the the ertfs pro failsafe autoinit function */
        if (pdr->drive_flags&DRIVE_FLAGS_FAILSAFE)
        {
            if (!pro_failsafe_auto_init(pdr))	//マニュアル参照
            {
                prtfs_cfg->drno_to_dr_map[pdr->driveno] = 0; /* Forget it, not there.  */
            }
        }
#endif
        
        /* Set the default drive to the lowest assigned drive letter */
        prtfs_cfg->default_drive_id = pdr->driveno;
    }


    /* autoformatting ram devices */
    if ( pdr->drive_flags&DRIVE_FLAGS_VALID && pdr->drive_flags&DRIVE_FLAGS_FORMAT)
    {
        OS_CLAIM_LOGDRIVE(pdr->driveno)  /* Autoformat Register drive in use  */
        drno_to_string(drname, pdr->driveno);
//        RTFS_PRINT_STRING_2(USTRING_RTFSINIT_07, drname,0); /* "Autoformatting Drive Id - " */
#if (STORE_DEVICE_NAMES_IN_DRIVE_STRUCT)
//        RTFS_PRINT_STRING_2(USTRING_RTFSINIT_08, pdr->device_name,PRFLG_NL); /* " as Device: " */
#endif
        if (auto_format_disk(pdr, drname) != 0) {
//               RTFS_PRINT_STRING_2(USTRING_RTFSINIT_09, drname,PRFLG_NL); /* "Autoformatting Drive Id - \n" */
        }
        OS_RELEASE_LOGDRIVE(pdr->driveno) /* Autoformat release */
    }
}


/*---------------------------------------------------------------------------*
  Name:         rtfs_detach

  Description:  

  Arguments:    driveno : ドライブ番号

  Returns:      
 *---------------------------------------------------------------------------*/
BOOLEAN rtfs_detach( int driveno)
{
    DDRIVE *target_pdr;
    
//    osTPrintf( "original detach\n");
    
    target_pdr = pc_drno_to_drive_struct( driveno);
    if( target_pdr == 0) {
        return( FALSE);		//未割り当てなのでdetachの必要なし
    }else{
	    if( (target_pdr->dev_table_drive_io == defaultRtfsIo) ||
	        (target_pdr->dev_table_drive_io == NULL)) {
	        return( FALSE);		//既にdetach済み
        }
    }

    /*open中のファイルを全てcloseする*/
    i_rtfs_close_disk( driveno);
    
    --enabled_drivers;

    target_pdr->driveno						= driveno;
    target_pdr->dev_table_drive_io     		= defaultRtfsIo;
    target_pdr->dev_table_perform_device_ioctl = defaultRtfsCtrl;
    rtfs_strcpy(target_pdr->device_name, (byte *)"DEFAULT\0");    //STORE_DEVICE_NAME(dev_name)
    target_pdr->register_file_address  = 0;
    target_pdr->interrupt_number       = 0;
    target_pdr->drive_flags            = 0;
    target_pdr->partition_number       = 0;
    target_pdr->pcmcia_slot_number     = 0;
    target_pdr->controller_number      = 0;
    target_pdr->logical_unit_number    = 0;

    /*REMOVE扱い*/
    target_pdr->drive_flags |= DRIVE_FLAGS_REMOVABLE;
    target_pdr->drive_flags &= ~DRIVE_FLAGS_INSERTED;
    
    return( TRUE);
}


/*---------------------------------------------------------------------------*
  Name:         i_rtfs_close_disk

  Description:  指定ドライブでopen中のファイルを全てcloseする

  Arguments:    driveno : ドライブ番号

  Returns:      
 *---------------------------------------------------------------------------*/
static int i_rtfs_close_disk( int driveno)
{
    PC_FILE *pfile;
    PCFD    i;
    DROBJ   *pobj;
    int     ret_val = 0;
    
    rtfs_set_errno(0);    /* clear errno */
    
    pfile = prtfs_cfg->mem_file_pool;
    for (i=0;i<pc_nuserfiles();i++, pfile++)
    {
        if( (pfile->is_free) == FALSE) //open中か？
        {
            if( !pfile->pobj) {//PECLOSEDエラーに相当(pc_fd2file参照)
                OS_CLAIM_FSCRITICAL()
                pfile->is_free = TRUE;
                OS_RELEASE_FSCRITICAL()
            }else{             //正常にopen中
                if( (pfile->pobj->pdrive->driveno) == driveno) {
#if (RTFS_WRITE)
                    if (pfile->flag & ( PO_RDWR | PO_WRONLY ) )
                    {
                        if (!_po_flush(pfile))
                        {   /* _po_flush has set errno */
                            ret_val = -1;
                        }
                    }
#endif
                    /* Release the FD and its core   */
                    //pc_freefile( pfile); apifilio.c内のstatic関数なので以下に展開
                    OS_CLAIM_FSCRITICAL()
                    pobj = pfile->pobj;
                    pfile->is_free = TRUE;
                    OS_RELEASE_FSCRITICAL()
                    pc_freeobj(pobj);
                }
            }
        }
    }
    if (!release_drive_mount_write(driveno)) {/* Release lock, unmount if aborted */
        return (-1);
    }
    return( ret_val);
}
