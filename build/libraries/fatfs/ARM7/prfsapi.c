/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* PRFSAPI.C - Contains user api level source code for ERTFS-Pro
   failsafe functions.

    The following routines are included:
    pro_failsafe_init - Initiate a failsafe session
    pro_failsafe_commit - Commit failsafe buffers to disk
    pro_failsafe_restore - Check or restore from the failsafe journal  
    pro_failsafe_auto_init - Automatically enable failsafe for a device at boot
    pro_failsafe_shutdown - Close or abort FailSafe mode
*/

#include <rtfs.h>

#if (INCLUDE_FAILSAFE_CODE)

#define INCLUDE_FAILSAFE_AUTO_INIT 1

/* pro_failsafe_init - Initiate a failsafe session
*
*   Summary:
*    BOOLEAN pro_failsafe_init(byte *drive_name,
*                              FAILSAFECONTEXT *pfscntxt,
*                              int configuration_flags)
* 
* Description:
* 
* This routine checks the user supplied parameters, initializes the 
* failsafe context structure and places the drive in failsafe mode.
* Once the device is in failsafe mode directory block and FAT block
* writes will be held in a journal file pro_failsafe_commit() is called. 
* 
* Inputs:
* drive_name - Null terminated drive designator, for example "A:"
* pfscntxt - The address of a block of data that will be used as a
* context block for failsafe. This block must remain valid for the whole
* session and may not be deallocated.
* journal_size - If this is non-zero a journal file is created containing
* this many blocks. If this value is zero the journal file will be created 
* using default size calculations. The default size is calculated to be the 
* size of the File Allocation table plus 
* value in blocks
* configuration_flags - You can optionally enable these features by oring 
* these values into the configuration_flags field.
*  FS_MODE_AUTORESTORE- If removable media is reinserted and the 
*                        journal file indicates that restore is needed,
*                        automatically restore and continue.
*                        If this flag is not set when this condition occurs
*                        the mount of the volume fails and the errno is set
*                        to PEFSRESTORENEEDED, the application must then
*                        handle this errno setting and call the pro_failsafe_restore
*  FS_MODE_AUTORECOVER- If AUTORESTORE is set and the restore fails, ignore
*                       ignore the error and continue. Attempt to create 
*                       the journal file. If AUTORECOVER is not set the 
*                       disk mount fails, with errno set to PEFSRESTOREERROR.
*  FS_MODE_AUTOCOMMIT-  If AUTOCOMMIT is enabled the FailSafe commit operation 
*                       will be performed automatically be ERTFS at the
*                       completion of each API call. With AUTOCOMMIT enabled 
*                       the FailSafe operation is transparent to the user 
*                       and it is not necessary to call pro_failsafe_commit().
*
*           
* Additional inputs - Several fields in the FailSafe context block may be 
* initialized before pro_failsafe_init() is called. user_journal_size may
* be set under certain special circumstances. blockmap_freelist and
* blockmap_size should be initialized under most circumstance.
*
*  user_journal_size - This field in the context block may be assigned with
*  a value that prescribes the size of the FailSafe file in blocks. If this 
*  field is left at zero the FailSafe file will be sized according to the
*  algorithm described in the compile time configuration section.  
* 
*  Blockmaps - FailSafe will utilize an optional block map cache if you 
*  provide it with the necessary ram resources. Even though FailSafe will 
*  execute properly if no block mapping is provided its performance will be
*  significantly diminished. It is therefore highly recommended that you 
*  enable block mapping. If the number of blocks written to the FailSafe 
*  file exceeds the number of block map elements FailSafe performance will 
*  be reduced drastically. Each caching structure requires approximately 
*  32 bytes. In the examples provided with ERTFS we provide 128 mapping 
*  structures. 
*
*  blockmap_freelist - This field in the context block may be assigned
*  a pointer to an array of structures of type FSBLOCKMAP. for example:
*  context.blockmap_freelist = &blockmap_array[0];
*
*  blockmap_size - This field may be filled in with the number of elements
*  contained in the  array assigned to blockmap_freelist.
* 
* Returns:
* TRUE          - Success
* FALSE         - Error 
*
* If it returns FALSE errno will be set to one of the following:
*
* PEFSREINIT       - Failsafe already initialized
* PEINVALIDPARMS   - Invalid parameters either no context block was passed
*                    the journal size parameter is too small
* PEINVALIDDRIVEID - Invalid drive
* An ERTFS system error 
* 
*/

BOOLEAN pro_failsafe_init(byte *drivename, FAILSAFECONTEXT *pfscntxt)
{
DDRIVE *pdrive;
int  driveno;
BOOLEAN ret_val;

    if (!pc_parsedrive( &driveno, drivename))
        return (FALSE);

    OS_CLAIM_LOGDRIVE(driveno)  /* pro_failsafe_init register drive in use */
    pdrive = pc_drno_to_drive_struct(driveno);
    if (pdrive->pfscntxt)
    {
        rtfs_set_errno(PEFSREINIT);
        OS_RELEASE_LOGDRIVE(driveno)
        return(FALSE);
    }

    /* If the drive is open now, flush all buffers and close the disk.
       When the drive is reopened it will re-open in failsafe mode */
    if (pdrive->mount_valid)
    {
        /* Flush any FAT buffers */
        if (!FATOP(pdrive)->fatop_flushfat(driveno))
        {
            OS_RELEASE_LOGDRIVE(driveno)
            return(FALSE);
        }
        /* Flush any directory buffers */
        /*HEREHERE - Need an API */
        pc_dskfree(driveno);
    }
    ret_val = pro_failsafe_init_internal(pdrive, pfscntxt);

    OS_RELEASE_LOGDRIVE(driveno)
    return(ret_val);
}

/* pro_failsafe_commit - Commit failsafe buffers to disk
*
*   Summary:
*       int pro_failsafe_commit(FAILSAFECONTEXT *pfscntxt)
* 
* Description:
* 
* This routine updates the disk from the in-memory fat and block buffers.
* If FS_MODE_JOURNALING is enabled the journal file is completed before the 
* buffer flush operation begins. 
*
* When pro_failsafe_commit completes succesfully, the changes made to 
* the volume structure since the previous succesful call to 
* pro_failsafe_commit are guaranteed to be committed to disk. If 
* the disk volume was correct when failsafe was started it will 
* still be correct.
* 
* If pro_failsafe_commit is unsuccessful, those changes made to 
* the volume structure since the last call to pro_failsafe_commit
* may be partially committed to disk. This causes inconsistencies such as
* lost cluster chains and incorrect file lengtths. If JOURNALING is 
* enabled, pro_failsafe_restore may be called to restore the disk volume 
* back to it's state after the last successful call to pro_failsafe_init.
*
*
* Returns:
* TRUE         - Success
* FALSE        - Failure
* If FALSE is returned errno will be set to one of the following.
*
*   PEINVALIDDRIVEID        - Drive argument invalid
*   PENOINIT                - pro_failsafe_init must be called first
*   PEJOURNALOPENFAIL       - journal file or vram init call failed
*   PEIOERRORREAD           - Error reading FAT or buffer area
*   PEIOERRORWRITEJOURNAL   - Error writing journal file or NVRAM section
*   PEIOERRORREADJOURNAL   - Error writing journal file or NVRAM section
*   PEIOERRORWRITEFAT       - Error writing fat area
*   PEIOERRORWRITEBLOCK     - Error writing directory area
*   PEINTERNAL              - internal error, only occurs if ERTFS is not configured.
*   PEIOERRORREAD           - Error reading FAT or buffer area
*   PEIOERRORWRITEJOURNAL   - Error writing journal file or NVRAM section
*   PEIOERRORWRITEFAT       - Error writing fat area
*   PEIOERRORWRITEBLOCK     - Error writing directory area
*   An ERTFS system error 
*/

BOOLEAN pro_failsafe_commit(byte *path) /* __apifn__ */
{
FAILSAFECONTEXT *pfscntxt;
int driveno;
BOOLEAN ret_val;
DDRIVE *pdr;

    driveno = check_drive_name_mount(path);
    if (driveno < 0)
        return(FALSE); /* Check_drive set errno */
    pdr = pc_drno_to_drive_struct(driveno);
    rtfs_set_errno(0);
    pfscntxt = (FAILSAFECONTEXT *) pdr->pfscntxt;
    if (!pfscntxt ||  !(pfscntxt->configuration_flags & FS_MODE_JOURNALING))
    {
        release_drive_mount(driveno);/* Release lock, unmount if aborted */
        rtfs_set_errno(PENOINIT);
        return (FALSE);
    }
   /* Flush any directory buffers */
    if (pro_failsafe_commit_internal(pfscntxt) == 0)
       ret_val = TRUE;
    else
       ret_val = FALSE;
    release_drive_mount(driveno);/* Release lock, unmount if aborted */
    return (ret_val);
}

/* pro_failsafe_restore - Check or restore from the failsafe journal  
*
*   Summary:
*     int pro_failsafe_restore(byte *drive_name, 
*                              FAILSAFECONTEXT *fscntxt, 
*                              BOOLEAN dorestore,
*                              BOOLEAN doclear)
* 
* Description:
* 
* This routine tests the status of the failsafe journal file and returns the 
* status of the drive. If dorestore is requested and restore is required 
* the disk volume is restored from the journal file.
* If doclear Failsafe will clear any errors in the failsafe file 
*
* Note: pro_failsafe_restore() requires a FAILSAFE CONTEXT structure to 
* operate. If Failsafe is already initialized it will already have
* this context block, otherwise 
*
* Returns:
* FS_STATUS_OK           - No restore required
* FS_STATUS_NO_JOURNAL   - No journal file present
* FS_STATUS_BAD_JOURNAL  - Journal present but has invalid fields
* FS_STATUS_BAD_CHECKSUM - Journal data doesn't match stored checksum.
* FS_STATUS_IO_ERROR     - IO error during restore
* FS_STATUS_RESTORED     - Restore was required and completed
* FS_STATUS_MUST_RESTORE - Disk must be restored from the journal file
*                           because flushing of FAT and disk blocks was
*                           interrupted
* FS_STATUS_NO_INIT      - Failsafe init must be called or a context 
*                          structure must be provided
* 
* Example:
*  int s;
*  s = pro_failsafe_restore("A:", 0, FALSE);
*  if (s==FS_STATUS_OK) printf("No restore required\n");
*  else if (s==FS_STATUS_OK) printf("No restore needed\n");
*  else if (s==FS_STATUS_NO_JOURNAL) printf("No journal file present\n");
*  else if (s==FS_STATUS_BAD_JOURNAL) printf("Journal contains bad data\n");
*  else if (s==FS_STATUS_BAD_CHECKSUM) printf("Journal contains bad data\n");
*  else if (s==FS_STATUS_OUT_OF_DATE) printf("Journal out of sync\n");
*  else if (s==FS_STATUS_NO_INIT) printf("Failsafe not initialized\n");
*  else if (s==FS_STATUS_IO_ERROR) printf("IO error accessing journal\n");
*  else if (s==FS_STATUS_MUST_RESTORE) 
*  {
*   printf("Restoring disk from the journal file\n");
*   s =  pro_failsafe_restore("A:", 0, TRUE);
*   if (s==FS_STATUS_RESTORED) 
*       printf("Restore was completed sucesfully\n");
*    else if (s==FS_STATUS_BAD_CHECKSUM)
*       printf("Journal file corrupted\n");
*    else if (s==FS_STATUS_IO_ERROR) 
*       printf("IO error during restore\n");
*   }
*  }
* 
*/

int pro_failsafe_restore(byte *drive_name, FAILSAFECONTEXT *fscntxt, BOOLEAN dorestore, BOOLEAN doclear)
{
int ret_val,driveno;
FAILSAFECONTEXT *savedcontext;
DDRIVE *pdr;

    savedcontext = 0;
    /* Get the drive and make sure it has a context block */
    if (!pc_parsedrive( &driveno, drive_name))
        return(FS_STATUS_NO_INIT);

    pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr)
        return(FS_STATUS_NO_INIT);

    if (!pdr->pfscntxt && !fscntxt)
        return(FS_STATUS_NO_INIT);

    OS_CLAIM_LOGDRIVE(driveno)  /* check_drive_name_mount Register drive in use */
    pc_dskfree(driveno);        /* make sure buffers are free */
    savedcontext = (FAILSAFECONTEXT *)pdr->pfscntxt;  /* Kill failsafe */
    pdr->pfscntxt = 0;
    OS_RELEASE_LOGDRIVE(driveno) /* check_drive_name_mount release */

    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(drive_name);
    if (driveno < 0)
    {
        ret_val = -1; /* check_drive set errno */
    }
    else
    {
        if (fscntxt)
        {
            rtfs_memset((void *) fscntxt, 0, sizeof(*fscntxt));
            fscntxt->pdrive = pdr;
            pdr->pfscntxt = (void *) fscntxt;
        }
        else
            pdr->pfscntxt = (void *) savedcontext;
        ret_val = failsafe_restore_internal((FAILSAFECONTEXT *) pdr->pfscntxt, dorestore, doclear);
        if (dorestore || doclear)
            pc_dskfree(driveno);
    }
    pdr->pfscntxt = (void *) savedcontext;
    OS_RELEASE_LOGDRIVE(driveno)  /* pro+failsafe_restore, release claim by check_drive_name_mount() */
    return(ret_val);
}

/* pro_failsafe_auto_init - Automatically enable failsafe for a device at boot
*  time
*
*  The user must modify this code if he wishes to auto configure failsafe
*  features for a given device.
*
* 
*  Summary:
*      BOOLEAN pro_failsafe_auto_init(DDRIVE *pdrive)
* 
* Description:
* 
*   If in pc_ertfs_init() the DRIVE_FLAGS_FAILSAFE bit is set in
*   pdr->drive_flags then this routine is called 
*   to automatically enable Failsafe mode for that drive.
*   If failsafe is initialized this way there is no need to call 
*   pro_failsafe_init() from the API layer.
*
*    Note: pro_failsafe_auto_init() is implemented inside the file
*    prapifs.c.
*    To use this feature, INCLUDE_FAILSAFE_AUTO_INIT must be set to 
*    1 in the source file, the routine should be modified to fit 
*    your needs and then be recompiled. By default the 
*    INCLUDE_FAILSAFE_AUTO_INIT is set to zero.
*
*
* 
* Inputs:
*  pdrive - A pointer to the drive structure for the device.
*
* Returns:
*  TRUE    - (default) If the user returns TRUE, ERTFS will allow mounts 
*  to occur on the drive.
*  FALSE  - If the user returns FALSE, ERTFS will not allow mounts 
*  to occur on the drive.
*
*/

#if (INCLUDE_FAILSAFE_AUTO_INIT)
FAILSAFECONTEXT auto_fscontext;
#define AUTO_BLOCKMAPSIZE 128
FSBLOCKMAP auto_failsafe_blockmap_array[AUTO_BLOCKMAPSIZE];
#endif 

BOOLEAN pro_failsafe_auto_init(DDRIVE *pdrive)
{
#if (!INCLUDE_FAILSAFE_AUTO_INIT)
        return(FALSE);
#else
    /* In this example we set up failsafe mode for one device
       This code may be modified to enable failsafe on multiple 
       devices and to modify FailSafe settings according to the 
       documentation provided for pro_failsafe_init(). 
    */
    if (auto_fscontext.pdrive) /* Only one device supported int this */
        return (FALSE);        /* Example. Add more contexts */

    rtfs_memset((void *) &auto_fscontext, 0, sizeof(auto_fscontext));
    /* Run fully automated */
    auto_fscontext.configuration_flags = 
//     (FS_MODE_AUTORESTORE|FS_MODE_AUTORECOVER/*|FS_MODE_AUTOCOMMIT*/);
     (FS_MODE_AUTORESTORE|FS_MODE_AUTORECOVER|FS_MODE_AUTOCOMMIT); // ctr modified
    auto_fscontext.blockmap_size = AUTO_BLOCKMAPSIZE;
    auto_fscontext.blockmap_freelist = &auto_failsafe_blockmap_array[0];
    /* Our parameters are correct so we know that the routine will not
       fail but to demonstrate the principle we return FALSE if the 
       call does fail. */
    if (!pro_failsafe_init_internal(pdrive, &auto_fscontext))
        return(FALSE);
    return(TRUE);
#endif /* (INCLUDE_FAILSAFE_AUTO_INIT) */
}

/* pro_failsafe_shutdown - Close or abort FailSafe mode
*
*   Summary:
*       BOOLEAN pro_failsafe_shutdown(byte *drive_name, BOOLEAN abort)
* 
* Description:
* 
* This routine allows the applications layer to disable Failsafe.
* FailSafe. The opcode field determines what operation to perform.
*
*
* If the parameter abort is TRUE then FailSafe is unilaterally disabled
* for the specified drive.
*
* If the parameter abort is FALSE then a FailSafe commit is performed and
* if it is succesful then FailSafe is disabled. If the commit is unsucessful 
* FailSafe is not disabled and errno will contain the reason for the failure.

*
* Returns:
*  TRUE  - If Success
*  FALSE - Failure
*
* If FALSE is returned errno will be set to one of the following.
*
*   PEINVALIDDRIVEID        - Drive argument invalid
*   PEJOURNALOPENFAIL       - journal file or vram init call failed
*   PEINTERNAL              - internal error, only occurs if ERTFS is not configured.
*   PEIOERRORREAD           - Error reading FAT or buffer area
*   PEIOERRORWRITEJOURNAL   - Error writing journal file or NVRAM section
*   PEIOERRORREADJOURNAL   - Error writing journal file or NVRAM section
*   PEIOERRORWRITEFAT       - Error writing fat area
*   PEIOERRORWRITEBLOCK     - Error writing directory area
*   An ERTFS system error 
* 
*/

BOOLEAN pro_failsafe_shutdown(byte *drive_name, BOOLEAN abort)
{
BOOLEAN ret_val;
int driveno;
DDRIVE *pdr;
FAILSAFECONTEXT *pfscntxt;

    if (abort) /* unilaterally suspend */
    {
        if (!pc_parsedrive( &driveno, drive_name))
            return (FALSE);
        pdr = pc_drno_to_drive_struct(driveno);
        if (!pdr)
            return (FALSE);
        OS_CLAIM_LOGDRIVE(driveno)  /* check_drive_name_mount Register drive in use */
        pdr->pfscntxt = 0; /* Shutdown just unlinks it */
        if (pdr->mount_valid)           /* already mounted */
           pc_dskfree(pdr->driveno);
        OS_RELEASE_LOGDRIVE(driveno)
        return(TRUE);
    }
    else
    {
        /* Get the drive and make sure it is mounted   */
        driveno = check_drive_name_mount(drive_name);
        if (driveno < 0)
            return(FALSE); /* check_drive set errno */
        rtfs_set_errno(0);
        pdr = pc_drno_to_drive_struct(driveno);
        ret_val = TRUE;
        pfscntxt = (FAILSAFECONTEXT *) pdr->pfscntxt;
        if (pfscntxt)
        {
            if (pro_failsafe_commit_internal(pfscntxt) != 0)
                ret_val = FALSE;
            else
                pdr->pfscntxt = 0; /* Shutdown just unlinks it */
        }
        release_drive_mount(driveno);
        return(ret_val);
    }
}

#endif /* (INCLUDE_FAILSAFE_CODE) */

