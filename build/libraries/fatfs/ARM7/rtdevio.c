/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* rtdevio.c - Media check functions and device io wrapper functions
*
*
*/
#include <rtfs.h>

static BOOLEAN check_media_entry(int driveno);
static BOOLEAN check_media_io(DDRIVE *pdr, BOOLEAN raw);
static BOOLEAN check_media(DDRIVE *pdr, BOOLEAN ok_to_automount, BOOLEAN raw_access_requested, BOOLEAN call_crit_err);
static int card_failed_handler(DDRIVE *pdr);

/* Check if a drive id is valid and mount the drive. This is an internal
   routine that is called by other api calls 
   If it fails, return -1 with the drive not claimed.
   If it succeeds return the drive number with the drive claimed.
   The caller will call release_drive_mount(driveno) to release it
*/

int check_drive_name_mount(byte *name)  /*__fn__*/
{
int driveno;
    /* Get the drive and make sure it is mounted   */
    if (pc_parsedrive( &driveno, name))
    {
        OS_CLAIM_LOGDRIVE(driveno)  /* check_drive_name_mount Register drive in use */
        if (!check_media_entry(driveno))
        {
            OS_RELEASE_LOGDRIVE(driveno) /* check_drive_name_mount release */
            return(-1);
        }
        return(driveno);
    }
    else
    {
        rtfs_set_errno(PEINVALIDDRIVEID); /* Check drive called with bad drive name */
        return(-1);
    }
}

/* Check if a drive id is valid and mount the drive. This is an internal
   routine that is called by other api calls 
   If it fails, return -1 with the drive not claimed.
   If it succeeds return the drive number with the drive claimed.
   The caller will call release_drive_mount(driveno) to release it
*/

BOOLEAN check_drive_number_mount(int driveno)  /*__fn__*/
{
    if (!pc_validate_driveno(driveno))
    {
        rtfs_set_errno(PEINVALIDDRIVEID); /* Check drive called with bad drive name */
        return(FALSE);
    }
    else
    {
        /* Get the drive and make sure it is mounted   */
        OS_CLAIM_LOGDRIVE(driveno)  /* check_drive_number_mount Register drive in use */
        if (!check_media_entry(driveno))
        {
            OS_RELEASE_LOGDRIVE(driveno) /* check_drive_number_mount release */
            return(FALSE);
         }
         return(TRUE);
    }
}

/* Release a drive that was claimed by a succesful call to 
   check_drive_name_mount, or check_drive_number_mount
   If the the operation queued an abort request then
   free the drive an all it's structures 
*/

void release_drive_mount(int driveno)
{
DDRIVE *pdr;
    pdr = pc_drno_to_drive_struct(driveno);
    if (pdr && pdr->mount_abort)
    {
        pc_dskfree(driveno);
    }
    OS_RELEASE_LOGDRIVE(driveno) /* release_drive_mount release */
}

BOOLEAN release_drive_mount_write(int driveno)
{
DDRIVE *pdr;
BOOLEAN ret_val;

    ret_val = TRUE;
    pdr = pc_drno_to_drive_struct(driveno);
    if (pdr)
    {
        if (pdr->mount_abort)
            pc_dskfree(driveno);
#if (INCLUDE_FAILSAFE_CODE) /* Call failsafe autocommit */
        else
            ret_val = pro_failsafe_autocommit(pdr);
#endif
    }
    OS_RELEASE_LOGDRIVE(driveno) /* release_drive_mount release */
    return(ret_val);
}


/* BOOLEAN check_drive_number_present(int driveno) 
*
*  Called from the apps layer when formatting 
*  Ignore and clear DISKCHANGED status 
*  Return TRUE if the device is UP. 
*
*  Inputs:
*   int     driveno;        Drive number 0 ..  prtfs_cfg->cfg_NDRIVES
*
* Outputs:
*   TRUE if is is okay to proceed with IO
*   FALSE  If we can not proceed with IO
*
*
*/

BOOLEAN check_drive_number_present(int driveno)  /*__fn__*/
{
    int media_status;
    DDRIVE *pdr;

    pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr || !(pdr->drive_flags&DRIVE_FLAGS_VALID) )
    {
        rtfs_set_errno(PEDEVICEINIT); /* device not initialized in pc_ertfs_init().*/
        return(FALSE);
    }
    media_status = DEVTEST_NOCHANGE;
    if (pdr->drive_flags & DRIVE_FLAGS_REMOVABLE)
    {
        media_status = pdr->dev_table_perform_device_ioctl(driveno, DEVCTL_CHECKSTATUS, 0);
        /* Call it again to clear media change */
        if (media_status == DEVTEST_CHANGED)
            media_status = pdr->dev_table_perform_device_ioctl(driveno, DEVCTL_CHECKSTATUS, 0);
    }
    if (media_status == DEVTEST_NOCHANGE)
        return(TRUE);
    else if (media_status == DEVTEST_NOMEDIA)
        rtfs_set_errno(PEDEVICENOMEDIA);
    else if (media_status == DEVTEST_UNKMEDIA)
        rtfs_set_errno(PEDEVICEUNKNOWNMEDIA);
    else
        rtfs_set_errno(PEDEVICEFAILURE);
    /* Queue the drive for disk free and remount */
    if (pdr->mount_valid)
        pdr->mount_abort = TRUE;
    return(FALSE);
}

/* BOOLEAN check_media_entry(int driveno)
*
*  Called from the top of the API
*  Return TRUE if the drive is mounted and it is okay to proceed with IO on
*  a drive
*
*  This routine is called from the top of the io layer. If the drive is
*  Not mounted it may be (re)mounted as a result of this call and the call
*  will return TRUE (it is okay to proceed). A similar call, check_media_io(),
*  is called from a lower layer. It will succeed if raw IO is requested 
*  and the device is ok or if the device is mounted and ok
*
*     Note: The the logical drive must be claimed before this routine is 
*    called and later released by the caller.
* 
*  Inputs:
*   int     driveno;        Drive number 0 ..  prtfs_cfg->cfg_NDRIVES
*
* Outputs:
*   TRUE if is mounted and it is okay to proceed with IO
*   FALSE  if the drive ca not be used and the user did not 
*   replace it with a usable disk
*
*
*/
static BOOLEAN check_media_entry(int driveno)      /*__fn__*/
{
    DDRIVE *pdr;
    /* Call media check
       If dev is OK and mounted proceed
       If dev is OK and not mounted try to mount.
       If dev is changed and mounted but buffers are not dirty then
       unmount and remount.
       If dev is changed and mounted but buffers are dirty then unmount and fail
    */
    pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr || !(pdr->drive_flags&DRIVE_FLAGS_VALID) )
    {
        rtfs_set_errno(PEDEVICEINIT); /* device not initialized in pc_ertfs_init().*/
        return(FALSE);
    }
    return(check_media(pdr, TRUE, FALSE, TRUE));
}


/* BOOLEAN check_media_io(DDRIVE *pdr, BOOLEAN raw)
*
*  Called from the IO layer
*  Return TRUE if the device is UP and either raw IO is requested 
*  or the drive is mounted.
* 
*     Note: The the logical drive must be claimed before this routine is 
*    called and later released by the caller.
*
*  Inputs:
*   int     driveno;        Drive number 0 ..  prtfs_cfg->cfg_NDRIVES
*
* Outputs:
*   TRUE if is is okay to proceed with IO
*   FALSE  If we ca not proceed with IO
*
*
*/

static BOOLEAN check_media_io(DDRIVE *pdr, BOOLEAN raw)    /*__fn__*/
{
    /* Call media check
       If dev is OK and mounted or raw proceed
       If dev is changed and not mounted and raw then proceed
       If dev is changed and mounted then unmount and fail
    */
    if (!pdr || !(pdr->drive_flags&DRIVE_FLAGS_VALID) )
    {
        rtfs_set_errno(PEDEVICEINIT); /* device not initialized in pc_ertfs_init().*/
        return(FALSE);
    }
    return(check_media(pdr, FALSE, raw, TRUE));
}

/* BOOLEAN check_media(DDRIVE *pdr, BOOLEAN ok_to_automount, BOOLEAN raw_access_requested, BOOLEAN call_crit_err)
*
*  Return TRUE is it is okay to proceed with IO on a drive
*
*     Note: The the logical drive must be claimed before this routine is 
*    called and later released by the caller.
** 
*  Inputs:
*   int     driveno;                Drive number 0 ..  prtfs_cfg->cfg_NDRIVES
*   BOOLEAN ok_to_automount         Automount the drive
*   BOOLEAN raw_access_requested    Mount not needed but device must
*                                   be functional
*    BOOLEAN call_crit_err          If true call error handler
*/

static BOOLEAN check_media(DDRIVE *pdr, BOOLEAN ok_to_automount, BOOLEAN raw_access_requested, BOOLEAN call_crit_err) /*__fn__*/
{
    int media_status;
    int crit_err_media_status;
    int drive_is_dirty;

    /* If an abort request is pending, executed it */
    if (ok_to_automount && pdr->mount_valid && pdr->mount_abort)
    {
        pc_dskfree(pdr->driveno);
    }

    /* If the device is removable check status to see if we must recover or remount */
    if (pdr->drive_flags & DRIVE_FLAGS_REMOVABLE)
        media_status = pdr->dev_table_perform_device_ioctl(pdr->driveno, DEVCTL_CHECKSTATUS, 0);
    else
        media_status = 0;

    crit_err_media_status = 0;

    if (media_status==0)            /* Device is up */
    {
        if (pdr->mount_valid)           /* already mounted */
        {
            goto status_is_ok;
        }
        else
        {
            if (raw_access_requested)   /* Mount not required */
                goto status_is_ok;
            else if (ok_to_automount)
            {
                /* Mount the drive if we can -
                   . Read partition table if required
                   . Read block zero
                */
                if (pc_i_dskopen(pdr->driveno))
                {
#if (INCLUDE_FAILSAFE_CODE) /* Call failsafe autorestore and open routines */
                    if (!pro_failsafe_autorestore(pdr) ||
                        !pro_failsafe_dskopen(pdr) )
                    { /* Failsafe set errno */
                        crit_err_media_status = 0;
                        goto status_is_bad;
                    }
#endif
                    goto status_is_ok;
                }
                else
                {
                    /* Tell the user it is unformatted */
                    /* pc_i_dskopen() sets errno */
                    crit_err_media_status = CRERR_BAD_FORMAT;
                    goto status_is_bad;
                }
            }
            else
            {
                /* The drive is not mounted. and ok_to_automount is false */
                rtfs_set_errno(PEDEVICECHANGED);
                goto status_is_bad;
            }
        }
    }
    else if (media_status == DEVTEST_CHANGED)
    {
        drive_is_dirty = (pdr->mount_valid && (pdr->fat_is_dirty || pc_test_all_fil(pdr)));
        /* If the drive is dirty the user must explicitly abort */
        if (drive_is_dirty)
        {
            rtfs_set_errno(PEDEVICECHANGED);
            crit_err_media_status = CRERR_CHANGED_CARD;
            goto status_is_bad;
        }
        else 
        {
            /* If automount is requested then close the existing
               mount and continue. */
            if (ok_to_automount)
            {
                pc_dskfree((int)pdr->driveno);
                /* Mount the drive if we can */
                if (pc_i_dskopen(pdr->driveno))
                {
#if (INCLUDE_FAILSAFE_CODE) /* Call failsafe autorestore and open routines */
                    if (!pro_failsafe_autorestore(pdr) ||
                        !pro_failsafe_dskopen(pdr) )
                    { /* Failsafe set errno */
                        crit_err_media_status = 0;
                        goto status_is_bad;
                    }
#endif
                    goto status_is_ok;
                }
                else
                {
                    /* pc_i_dskopen() sets errno */
                    crit_err_media_status = CRERR_BAD_FORMAT;
                    goto status_is_bad;
                }
            }
            else
            {
                rtfs_set_errno(PEDEVICECHANGED);
                crit_err_media_status = CRERR_CHANGED_CARD;
                goto status_is_bad;
            }
        }
    }   
    else 
    {
        if (media_status == DEVTEST_NOMEDIA)
        {
            rtfs_set_errno(PEDEVICENOMEDIA);
            crit_err_media_status = CRERR_NO_CARD;
        }
        else if (media_status == DEVTEST_UNKMEDIA)
        {
            rtfs_set_errno(PEDEVICEUNKNOWNMEDIA);
            crit_err_media_status = CRERR_BAD_CARD;
        }
        else
        {
            rtfs_set_errno(PEDEVICEFAILURE);
            crit_err_media_status = CRERR_CARD_FAILURE;
        }
        goto status_is_bad;
    }
status_is_ok:
    return(TRUE);
status_is_bad:
    if (crit_err_media_status)
    {
        if (call_crit_err)
            critical_error_handler(pdr->driveno, crit_err_media_status, 0);
    }
    /* Queue the drive for disk free and remount */
    pdr->mount_abort = TRUE;
    return(FALSE);
}



static int card_failed_handler(DDRIVE *pdr)    /* __fn__ */
{
int ret_val;

    ret_val = critical_error_handler(pdr->driveno,CRERR_CARD_FAILURE, 0);
                
    if (ret_val == CRITICAL_ERROR_ABORT)
    {
        /* Queue the drive for disk free and remount */
        if (pdr->mount_valid)
        {
            pdr->mount_abort = TRUE;
        }
    }
    return(ret_val);
}
/*
*     Note: The the logical drive must be claimed before this routine is 
*    called and later released by the caller.
*/
BOOLEAN devio_read(int driveno, dword blockno, byte * buf, word n_to_read, BOOLEAN raw) /* __fn__ */
{
DDRIVE *pdr;

    pdr = pc_drno_to_drive_struct(driveno);

/* Bug Fix. 1-28-02 thanks to Kerry Krouse. This code was inside the 
   loop, which caused retries to fail 
*/
   if (pdr->drive_flags & DRIVE_FLAGS_PARTITIONED)
   {
       if (!raw)
           blockno += pdr->partition_base;
   }

    for(;;)
    {
        /* Check if disk is accessable and has not changed since mounted */
        if (!(pdr->drive_flags & DRIVE_FLAGS_VALID))
            return(FALSE);
        if (!check_media_io(pdr, raw))
            return(FALSE);

        if (pdr->dev_table_drive_io(driveno, blockno, buf, n_to_read, TRUE))
        {
            return(TRUE);
        }
        else
        {
            if (card_failed_handler(pdr) != CRITICAL_ERROR_RETRY)
                break;
        }
    }
    return(FALSE);
}

#if (RTFS_WRITE)

/* This is a special version of devio_write that is used by the format utility
   It is the same as devio_write except that it does not automount the 
   drive or load the partition table. If non-raw IO is requested the 
   caller must first be sure that the partition table is loaded 
*/
/*
*     Note: The the logical drive must be claimed before this routine is 
*    called and later released by the caller.
*/
 
BOOLEAN devio_write_format(int driveno, dword blockno, byte * buf, word n_to_write, BOOLEAN raw) /* __fn__ */
{
DDRIVE *pdr;

    pdr = pc_drno_to_drive_struct(driveno);
    if (pdr->drive_flags & DRIVE_FLAGS_PARTITIONED)
    {
        if (!raw)
           blockno += pdr->partition_base;
    }

    for (;;)
    {
        /* Check if disk is accessable and has not changed since mounted */
        if (!check_media_io(pdr, TRUE))
            return(FALSE);

        if (pdr->dev_table_drive_io(driveno, blockno, buf, n_to_write, FALSE))
        {
            return(TRUE);
        }
        else
        {
            if (card_failed_handler(pdr) != CRITICAL_ERROR_RETRY)
                break;
        }
    }
    rtfs_set_errno(PEIOERRORWRITE);/* devio_write_format: write failed */
    return(FALSE);
}
/*
*     Note: The the logical drive must be claimed before this routine is 
*    called and later released by the caller.
*/

BOOLEAN devio_write(int driveno, dword blockno, byte * buf, word n_to_write, BOOLEAN raw) /* __fn__ */
{
DDRIVE *pdr;

    pdr = pc_drno_to_drive_struct(driveno);
    if (pdr->drive_flags & DRIVE_FLAGS_PARTITIONED)
    {
         if (!raw)
             blockno += pdr->partition_base;
    }

    for (;;)
    {
        /* Check if disk is accessable and has not changed since mounted */
        if (!check_media_io(pdr, raw))
            return(FALSE);

        if (pdr->dev_table_drive_io(driveno, blockno, buf, n_to_write, FALSE))
        {
            return(TRUE);
        }
        else
        {
            if (card_failed_handler(pdr) != CRITICAL_ERROR_RETRY)
                break;
        }
    }
    return(FALSE);
}

#endif

