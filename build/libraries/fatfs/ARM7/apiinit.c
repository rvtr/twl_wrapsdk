/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS inc, 1996
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/

/******************************************************************************
    PC_ERTFS_INIT() - Configure ERTFS drive letter/device mapping and initialize
    device drivers.

    Returns
    TRUE  If ertfs memory and system resource init functions succeed
    FALSE If ertfs memory or system resource init functions fail

    The user must modify this code to assign parameters and bind device
    driver entry points to ERTFS drive structures. The source code contains
    examples for configuring each of the stock device drivers.

    This routine initializes ERTFS resources and binds device drivers to drive
    letters so it must must be called before any other calls to the ERTFS
    API are made.


    Device driver configuration block.

    The ERTFS drive structure (struct ddrive) contains a section that must be
    initialized by the user.

    The following fields must be initialized -

    register_file_address   - IO address pointer that may be used by the
    device driver to access the controller. For the stock set of drivers
    only the IDE/ATAPI driver use this parameter.
    interrupt_number        - Interrupt vector number that the controller
    will interrupt on.
    drive_flags             - The user sets bits in this field to tell ERTFS
    and the device driver about the drive s properties. The device driver
    uses this field to keep track of the device status.

    These two bits may be set by the user in the main body of the
    the pc_ertfs_init routine.

    DRIVE_FLAGS_PARTITIONED - Set this bit if the device is a partitioned
    device.
    DRIVE_FLAGS_PCMCIA      - Set this bit if the device is a pcmcia device.
    ERTFS does not look at this bit. It is used ony by the device drivers.

    DRIVE_FLAGS_FAILSAFE    - Set this bit if you want pc_ertfs_init()
    to call pro_failsafe_auto_init for this drive to automatically
    enable failsafe operating mode.
    Note: pro_failsafe_auto_init() is implemented inside the file
    prapifs.c. By default the function automatically fails unless
    the prapifs.c is modified and the compile time constant
    named INCLUDE_FAILSAFE_AUTO_INIT is set to one. pro_failsafe_auto_init()
    as implemented supports only one drive at a time. It places the
    device in autorestore, autorecover and autocommit
    mode. To change the default behavior or to support more than one device
    you must modify the source code.

    These bits are set by device driver.

    DRIVE_FLAGS_VALID -  Set by the driver in the WARMSTART IO Control
    call if  initialization succeeded. If this flags is not set then ERTFS
    will not call the driver again.

    DRIVE_FLAGS_FORMAT -  Set by the driver in the WARMSTART IO Control
    call if the device requires formatting before it may be used. If this flags is set
    then ERTFS will format the device during the rtfs_init phase.

    DRIVE_FLAGS_REMOVABLE - Set by the device driver init routine if the
    device is removable. If this bit is set then the driver must implement
    the DEVCTL_CHECKSTATUS ioctrl function.

    DRIVE_FLAGS_INSERTED - This bit is used by the device driver to maintain
    state information. This bit is used by the PCMCIA drivers in the
    following way. The default value is ZERO. If this value is zero when the
    check status IOCTRL call is made then the driver attempts to map the
    device through the pcmcia controller. The pcmcia management interrupt
    routine clears this bit when a card removal occurs.

    partition_number    - If DRIVE_FLAGS_PARTITIONED is set in the
    drive_flags field then this must be initialized with the partition
    number. 0,1,2 etc.
    pcmcia_slot_number  - If DRIVE_FLAGS_PCMCIA is set then this tells which
    pcmcia slot. Must be (0 or 1) for the supplied PCMCIA subsystem.
    pcmcia_cfg_opt_value - This is the value to be place in the card s
    configuration option register by the pcmcia driver after power up.
    controller_number   - This is set by the user in init and used by
    driver to access controller specific information. For example the ide
    device driver supports two controllers. This value is used by the driver
    to store controller specific data and calculate controller specific
    addresses and values.
    logical_unit_number - This is a logical unit number that is used only
    by the device driver. For example the IDE driver uses logical unit
    number 1 to access the slave drive. The floppy disk uses logical
    unit 1 to access the secondary floppy.
    driveno - This is the drive number that the user wishes ERTFS to access
    this device as. 0 = A:, 1 = B: etc.


    dev_table_drive_io  - This is the io function for the device. The
    function must perform the requested read or write operation and
    return the correct status (TRUE for success, FALSE for failure).

    By example here is the ide drive io function.

        BOOLEAN ide_io(driveno, sector, buffer, count, BOOLEAN reading)
        int driveno - 0 to prtfs_cfg->cfg_NDRIVES == A:,B:,.. the driver calls
        pc_drno_to_drive_struct(driveno) to access the drive structure.
        dword sector- Starting sector number to read or write
        void *buffer- Buffer to read to write from.
        word count  - Number of sectors to transfer
        reading     - True for a read request, False for a write request.

    dev_table_perform_device_ioctl - This is the control interface to the
    device driver.

    By example here is the ide drive io control function.

    int ide_perform_device_ioctl(driveno, opcode, pargs)
        int driveno - 0 to prtfs_cfg->cfg_NDRIVES == A:,B:,.. the driver calls
        pc_drno_to_drive_struct(driveno) to access the drive structure.
        int opcode - One of the following:
            DEVCTL_CHECKSTATUS
            DEVCTL_WARMSTART
            DEVCTL_POWER_RESTORE
            DEVCTL_POWER_LOSS
            DEVCTL_GET_GEOMETRY
            DEVCTL_FORMAT
            DEVCTL_REPORT_REMOVE
        void * pargs - void pointer to arguments. See details below.

            Here are detailed descriptions of each OPCODE and its relation
            to pargs.

            DEVCTL_CHECKSTATUS  - Return the status of device. ERTFS calls
            the driver with this opcode before it performs IO.

            Note: For the CHECKSTATUS call. ERTFS will pass a BOOLEAN value
            to the driver to indicate if the volume in question has unwritten
            cached data that will be lost if the driver returns anything but
            DEVTEST_NOCHANGE. If this value is true then the driver may
            make more aggresive efforts to correct media changes. For example
            by asking the user to re-insert the correct diskette or card.

            The driver must return one of the following values:
            DEVTEST_NOCHANGE - The device is available and no media change
            has been detected since the previous CHECKSTATUS call.
            DEVTEST_NOMEDIA  - The device contains no media.
            DEVTEST_UNKMEDIA - The device contains media but the driver
            can not read or write to it.
            DEVTEST_CHANGED  - The device is available to but a media change
            has been detected since the previous CHECKSTATUS call. ERTFS will
            close out the current logical volume and re-mount the device.

            DEVCTL_WARMSTART - The device driver is called with this opcode
            once at startup. For non removable media the device should verify
            that the device is accessable and if so set the DRIVE_FLAGS_VALID
            bit. For removable media the device should verify that the device
            controller is accessable and if so set the DRIVE_FLAGS_VALID and
            DRIVE_FLAGS_REMOVABLE bits. pargs is not used.

            DEVCTL_POWER_RESTORE - Tell the device driver that system power
            has been restored. The driver should attempt to restore the
            device to the "UP" state. ERTFS never calls the driver with this
            opcode. System integrators may call it to inform the driver of
            power restore. pargs is not used.

            DEVCTL_POWER_LOSS - Tell the device driver that system power
            has been lost or will soon be lost. The driver should attempt to
            go into low power mode. ERTFS never calls the driver with this
            opcode. System integrators may call it to inform the driver of
            power loss. pargs is not used.

            DEVCTL_GET_GEOMETRY  - ERTFS calls this routine to get the values
            it will use to partition and format volumes.
            pargs points to a structure of type DEV_GEOMETRY. The driver
            should fill in the values as defined below.
            typedef struct dev_geometry {
            -  Geometry in HCN format
            int dev_geometry_heads;      - Must be < 256
            int dev_geometry_cylinders;  - Must be < 1024
            int dev_geometry_secptrack;  - Must be < 64
            BOOLEAN fmt_parms_valid;   If the device io control call sets
                                       this TRUE then it it telling the
                                       the format parameters are valid
                                       and should be used. This is a way to
                                       format floppy disks exactly as they
                                       are formatted by dos.
            FMTPARMS fmt;
            } DEV_GEOMETRY;

            DEVCTL_FORMAT       - ERTFS calls this routine to physically
            format the device. pargs points to the DEV_GEOMETRY structure
            that was returned from the GET_GEOMETRY call. The device driver
            should physically format the device if needed.

            DEVCTL_REPORT_REMOVE - This can be called by an external
            interrupt to tell the driver that the device has been removed.
            The pcmcia management interrupt calls this and the pcmcia aware
            drivers, PCMSRAM.C and IDE_DRV.C, clear the DRIVE_FLAGS_INSERTED
            bit in the drive structure. They later query this from the
            CHECKSTATUS code. pargs is not used.

*/
#include <twl.h>
#include <rtfs_target_os.h> /* twl modified */
#include <rtfs.h>
#include <portconf.h>   /* For included devices */

/*ctr modified(delete prototype definition of the all driver here.)*/

int auto_format_disk(DDRIVE *pdr, byte *drivename);
void drno_to_string(byte *pname, int drno);
void print_device_names(void);


#if (STORE_DEVICE_NAMES_IN_DRIVE_STRUCT)
/*ctr modified*/
#define STORE_DEVICE_NAME(NAME) rtfs_strcpy(current_pdr->device_name, (byte *)NAME);
#else
#define STORE_DEVICE_NAME(NAME)
#endif


/*------------------ctr modified start------------------*/
//BOOLEAN i_rtfs_begin_attach( void);
BOOLEAN i_rtfs_attach( int driveno, DDRIVE* pdr, char* dev_name);
BOOLEAN i_rtfs_end_attach( void);

extern BOOLEAN rtfs_first_attach;	//attach.cのattach APIがまだ未使用ならTRUE

int	enabled_drivers;
DDRIVE* current_pdr;

BOOLEAN rtfs_init( void)
{
    int j;
    DDRIVE* pdr;
    
    /* Call the user supplied configuration function */
    prtfs_cfg = 0;
    /* pc_ertfs_init: Can't used rtfs_set_errno(p_errno); */
    if (!pc_ertfs_config())	/*apicnfig.c*/
    {
        return(FALSE);
    }

    if (!pc_memory_init())	/*rtkernfn.c*/
    {
        return(FALSE);
    }
    /* Allocate semaphores for all drives here and assign them to the
       drive structures. If an allocation fails, fail to initialize */
    pdr = prtfs_cfg->mem_drives_structures;
    for (j = 0; j < prtfs_cfg->cfg_NDRIVES; j++, pdr++)
    {
        /* make sure this drive has a semaphore associated with it */
           pdr->access_semaphore = rtfs_port_alloc_mutex();
        if (!pdr->access_semaphore)
            return(FALSE);
    }
    /*---------- ctr modified ----------*/
    for( j=0; j<26; j++) {
        prtfs_cfg->drno_to_dr_map[j] = 0;
    }
    /*----------------------------------*/

    enabled_drivers = 0;
    current_pdr = prtfs_cfg->mem_drives_structures;
    
    rtfs_first_attach = FALSE;			//ctr modified

    // RTCの初期化
//    OSAPI_RTCINIT();                          //ctr modified
    
    return( TRUE);
}
/*--- ctr modified ---*/
/*SDK_WEAK_SYMBOL BOOL rtcInit( void)
{
    return( FALSE);
}*/
/*--------------------*/

/*---------------------------------------------------------------------------*
  Name:         rtfs_attach

  Description:  

  Arguments:    driveno : ドライブ番号

  Returns:      
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL BOOLEAN i_rtfs_attach( int driveno, DDRIVE* pdr, char* dev_name)
{
    return( FALSE);
/*    if (++enabled_drivers > prtfs_cfg->cfg_NDRIVES) {
        osTPrintf( "%d / %d\n", enabled_drivers, prtfs_cfg->cfg_NDRIVES);
        return( FALSE);
    }

    current_pdr->driveno						= driveno;
    current_pdr->dev_table_drive_io     		= pdr->dev_table_drive_io;
    current_pdr->dev_table_perform_device_ioctl = pdr->dev_table_perform_device_ioctl;
    STORE_DEVICE_NAME(dev_name)
    current_pdr->register_file_address  = pdr->register_file_address;
    current_pdr->interrupt_number       = pdr->interrupt_number;
    current_pdr->drive_flags            = pdr->drive_flags;
    current_pdr->partition_number       = pdr->partition_number;
    current_pdr->pcmcia_slot_number     = pdr->pcmcia_slot_number;
    current_pdr->controller_number      = pdr->controller_number;
    current_pdr->logical_unit_number    = pdr->logical_unit_number;
    
    current_pdr++;
    return( TRUE);*/
}

/*---------------------------------------------------------------------------*
  Name:         rtfs_detach

  Description:  

  Arguments:    driveno : ドライブ番号

  Returns:      
 *---------------------------------------------------------------------------*/
/*BOOLEAN rtfs_detach( int driveno)
{
    DDRIVE *target_pdr;
    
    target_pdr = prtfs_cfg->drno_to_dr_map[pdr->driveno];
    if( target_pdr->dev_table_drive_io == default_pdr->dev_table_drive_io) {
        return( FALSE);		//既にdetach済み
    }

    target_pdr->driveno						= driveno;
    target_pdr->dev_table_drive_io     		= default_pdr->dev_table_drive_io;
    target_pdr->dev_table_perform_device_ioctl = default_pdr->dev_table_perform_device_ioctl;
    rtfs_strcpy(target_pdr->device_name, (byte *)"DEFAULT\0");    //STORE_DEVICE_NAME(dev_name)
    target_pdr->register_file_address  = default_pdr->register_file_address;
    target_pdr->interrupt_number       = default_pdr->interrupt_number;
    target_pdr->drive_flags            = default_pdr->drive_flags;
    target_pdr->partition_number       = default_pdr->partition_number;
    target_pdr->pcmcia_slot_number     = default_pdr->pcmcia_slot_number;
    target_pdr->controller_number      = default_pdr->controller_number;
    target_pdr->logical_unit_number    = default_pdr->logical_unit_number;
    
    return( TRUE);
}*/


BOOLEAN i_rtfs_end_attach( void)
{
//    current_pdr = prtfs_cfg->mem_drives_structures;
    return( FALSE);
}

/*----------------ctr modified end---------------*/


BOOLEAN pc_ertfs_init(void) /* __apifn__ */
{
int j;
DDRIVE *pdr;
//int drives_used;
byte drname[8]; /* Temp buffer for displaying drive letters as strings */
int default_drive;

    if( rtfs_first_attach == FALSE) { return( TRUE); }	//ctr modified

    pdr = prtfs_cfg->mem_drives_structures;

//    drives_used = 0;

	//ctr modified : erase attach codes of some drivers here.

    /* End User initialization section */

    print_device_names();

    pdr= prtfs_cfg->mem_drives_structures;
    default_drive = 27; /* greater than maximum legal value  */
    for (j = 0; j < prtfs_cfg->cfg_NDRIVES; j++, pdr++)
    {
        if (pdr->dev_table_drive_io)
        {
            prtfs_cfg->drno_to_dr_map[pdr->driveno] = pdr; /* MAPS DRIVE structure to DRIVE: */
            if (pdr->dev_table_perform_device_ioctl(pdr->driveno, DEVCTL_WARMSTART, (void *) 0) != 0)
            {
                prtfs_cfg->drno_to_dr_map[pdr->driveno] = 0; /* It is not there.  */
                                                             /* so forget it */
                continue;
            }
#if (INCLUDE_FAILSAFE_CODE)
            /* Call the the ertfs pro failsafe autoinit function */
            if (pdr->drive_flags&DRIVE_FLAGS_FAILSAFE)
            {
                if (!pro_failsafe_auto_init(pdr))	//マニュアル参照
                {
                    prtfs_cfg->drno_to_dr_map[pdr->driveno] = 0; /* Forget it, not there.  */
                    continue;
                }
            }
#endif
            /* Set the default drive to the lowest assigned drive letter */
            if (pdr->driveno < default_drive)
            {
                default_drive = pdr->driveno;
            }
        }
    }

    /* If there are no valid devices return failure
       otherwise use the lowest valid drive id as the system's default drive*/
    if (default_drive == 27)
        return(FALSE);
    prtfs_cfg->default_drive_id = default_drive;

    RTFS_PRINT_STRING_1(USTRING_RTFSINIT_05,PRFLG_NL); /* "Autoformatting RAM Devices\n" */
    RTFS_PRINT_STRING_1(USTRING_RTFSINIT_06,PRFLG_NL); /* "==========================\n" */

    pdr= prtfs_cfg->mem_drives_structures;
    for (j = 0; j < prtfs_cfg->cfg_NDRIVES; j++, pdr++)
    {
        if ( pdr->drive_flags&DRIVE_FLAGS_VALID && pdr->drive_flags&DRIVE_FLAGS_FORMAT)
        {
            OS_CLAIM_LOGDRIVE(pdr->driveno)  /* Autoformat Register drive in use  */
            drno_to_string(drname, pdr->driveno);
            RTFS_PRINT_STRING_2(USTRING_RTFSINIT_07, drname,0); /* "Autoformatting Drive Id - " */
#if (STORE_DEVICE_NAMES_IN_DRIVE_STRUCT)
            RTFS_PRINT_STRING_2(USTRING_RTFSINIT_08, pdr->device_name,PRFLG_NL); /* " as Device: " */
#endif
            if (auto_format_disk(pdr, drname) != 0)
            {
                RTFS_PRINT_STRING_2(USTRING_RTFSINIT_09, drname,PRFLG_NL); /* "Autoformatting Drive Id - \n" */
            }
            OS_RELEASE_LOGDRIVE(pdr->driveno) /* Autoformat release */
        }
    }
    RTFS_PRINT_STRING_1(USTRING_RTFSINIT_10, PRFLG_NL); /* "             =           " */
    return(TRUE);
need_more_drives:
     RTFS_PRINT_STRING_1(USTRING_RTFSINIT_11, PRFLG_NL); /* "NDRIVES is too small to mount all devices" */
    return(FALSE);
}


int auto_format_disk(DDRIVE *pdr, byte *drivename)
{
DEV_GEOMETRY geometry;

    /* check media and clear change conditions */
    if (!check_drive_number_present(pdr->driveno))
        return(-1);

    /* This must be called before calling the later routines   */
    if (!pc_get_media_parms(drivename, &geometry))
        return(-1);

    /* Call the low level media format. Do not do this if formatting a
       volume that is the second partition on the drive */
    if (!pc_format_media(drivename, &geometry))
           return(-1);

    /* Get media parms again in case the format operation changed the parameters */
    if (!pc_get_media_parms(drivename, &geometry))
        return(-1);

    if (!pc_format_volume(drivename, &geometry))
        return(-1);
    return (0);
}
void drno_to_string(byte *pname, int drno)
{
byte c,*p;
    p = pname;
    c = (byte) ('A' + drno);
    CS_OP_ASSIGN_ASCII(p,c);
    CS_OP_INC_PTR(p);
    CS_OP_ASSIGN_ASCII(p,':');
    CS_OP_INC_PTR(p);
    CS_OP_TERM_STRING(p);
}

void print_device_names(void)
{
int j;
DDRIVE *pdr;
byte drname[8];

    RTFS_PRINT_STRING_1(USTRING_RTFSINIT_01,PRFLG_NL); /* "ERTFS Device List" */
    RTFS_PRINT_STRING_1(USTRING_RTFSINIT_02,PRFLG_NL); /* "=================" */
#if (!STORE_DEVICE_NAMES_IN_DRIVE_STRUCT)
    RTFS_PRINT_STRING_1(USTRING_RTFSINIT_03,PRFLG_NL); /* "Name Logging Disabled" */
#else

    pdr = prtfs_cfg->mem_drives_structures;
    for (j = 0; j < prtfs_cfg->cfg_NDRIVES; j++, pdr++)
    {
        if (pdr->dev_table_drive_io)
        {
            drno_to_string(drname, pdr->driveno);
            RTFS_PRINT_STRING_2(USTRING_RTFSINIT_12, pdr->device_name,0); /* "Device name : " */
            RTFS_PRINT_STRING_2(USTRING_RTFSINIT_13, drname,PRFLG_NL);    /* " Is mounted on "  */
        }
    }
    RTFS_PRINT_STRING_1(USTRING_RTFSINIT_04,PRFLG_NL); /* "       =         " */
#endif
}
