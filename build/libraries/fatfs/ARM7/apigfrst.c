/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APIGFRST.C - Contains user api level source code.

    The following routines are included:

    pc_gfirst       - Get stats on the first file to match a pattern.
    pc_gnext        - Get stats on the next file to match a pattern.
    pc_gdone        - Free resources used by pc_gfirst/pc_gnext.
    pc_upstat       -   Copy directory entry info to a user s stat buffer

*/

#include <rtfs.h>


/***************************************************************************
    PC_GFIRST - Get first entry in a directory to match a pattern.

 Description
    Given a pattern which contains both a path specifier and a search pattern
    fill in the structure at statobj with information about the file and set
    up internal parts of statobj to supply appropriate information for calls
    to pc_gnext.

    Examples of patterns are:
        D:\USR\RELEASE\NETWORK\*.C
        D:\USR\BIN\UU*.*
        D:MEMO_?.*
        D:*.*

 Returns
    Returns TRUE if a match was found otherwise FALSE. (see also the pcls.c
    utility.)

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PEINVALIDPATH   - Path specified badly formed.
    PENOENT         - Not found, no match
    An ERTFS system error

****************************************************************************/


void pc_upstat(DSTAT *statobj);

BOOLEAN pc_gfirst(DSTAT *statobj, byte *name)      /*__apifn__*/
{
    byte  *mompath;
    byte  *filename;
    byte  fileext[4];
    int driveno;
    DDRIVE *pdrive;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    rtfs_set_errno(0);  /* po_gfirst: clear error status */
    rtfs_memset((byte *) statobj,0,sizeof(*statobj));
/*    statobj->pobj = 0; */
/*    statobj->pmom = 0; */

    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(name);
    if (driveno < 0)
    {
        /* errno was set by check_drive */
        return(FALSE);
    }
    pdrive = pc_drno2dr(driveno);
    /* Use the buffers in the DRIVE structure. Access is locked via semaphore */
    mompath = (byte *)&(pdrive->pathname_buffer[0]);
    filename = (byte *)&(pdrive->filename_buffer[0]);

    /* Get out the filename and d:parent   */
    if (!pc_parsepath(mompath,filename,fileext,name))
    {
        release_drive_mount(driveno);/* Release lock, unmount if aborted */
        rtfs_set_errno(PEINVALIDPATH); /* pc_gfirst: Bad path name */
        return(FALSE);
    }

    /* Save the pattern. we will need it in pc_gnext   */
    copybuff(statobj->pname, filename, FILENAMESIZE_BYTES);
    copybuff(statobj->pext, fileext, 4);
    /* Copy over the path. we will need it later   */
    copybuff(statobj->path, mompath, EMAXPATH_BYTES);
    /* Find the file and init the structure   */
    statobj->pmom = (void *) pc_fndnode(mompath);
    /* pc_fndnode will set errno */
    if (statobj->pmom)
        /* Found it. Check access permissions   */
    {
        if(pc_isadir((DROBJ *)(statobj->pmom)))
        {
            /* Now find pattern in the directory   */
            statobj->pobj = (void *) pc_get_inode(0, (DROBJ *)(statobj->pmom), filename, (byte*) fileext, GET_INODE_WILD);
            if (statobj->pobj)
           {
                /* And update the stat structure   */
                pc_upstat(statobj);

                /* remember the drive number. used by gnext et al.   */
                statobj->driveno = driveno;
/* 9-20-94 release the FINODE and allocate a dummy. This will keep everyone
    who expects the drobj to own a finode happy but will not leave the
    finode open which locks out unlink et al */
                pc_freei(((DROBJ *)(statobj->pobj))->finode); /* Release the current */
/* 3-07-07 - Change: Remove additional call to pc_alloci(). Was not needed and caused a leak
   on a hot swap event when a gfirst is outstanding */
                ((DROBJ *)(statobj->pobj))->finode = 0;
/* END 9-20-94   */
                /* Remember the unique number associated with the drive
                   mount. If the drive is closed before we call gnext or
                   gdone we'll know about it because this number won't match */
                statobj->drive_opencounter = pdrive->drive_opencounter;
                release_drive_mount(driveno);/* Release lock, unmount if aborted */
                return(TRUE);
            }
            else
            {
            /* pc_gfirst: if statobj->pobj is 0 pc_get_inode() has set errno to PENOENT or
               to an internal or IO error status
               if PENOENT set we will clear errno */
                if (get_errno() == PENOENT)
                    rtfs_set_errno(0); /* pc_gfirst: file not found in directory
                                      set errno to zero and return FALSE */
            }
        }
        else
            rtfs_set_errno(PEINVALIDDIR); /* pc_gfirst: Path not a directory, report not found */
    }
    /* If it gets here we had a problem   */
    if (statobj->pmom)
        pc_freeobj((DROBJ *)statobj->pmom);
    rtfs_memset((byte *) statobj,0,sizeof(*statobj));
    release_drive_mount(driveno);/* Release lock, unmount if aborted */

    return(FALSE);
}

/****************************************************************************
    PC_GNEXT - Get next entry in a directory that matches a pattern.

 Description
    Given a pointer to a DSTAT structure that has been set up by a call to
    pc_gfirst(), search for the next match of the original pattern in the
    original path. Return TRUE if found and update statobj for subsequent
    calls to pc_gnext.

 Returns
    Returns TRUE if a match was found otherwise FALSE.

    errno is set to one of the following
    0               - No error
    PEINVALIDPARMS - statobj argument is not valid
    PENOENT        - Not found, no match (normal termination of scan)
    An ERTFS system error
****************************************************************************/

BOOLEAN pc_gnext(DSTAT *statobj)     /*__apifn__*/
{
    DROBJ *nextobj;
    DDRIVE *pdrive;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */
    /* see if the drive is still mounted. Do not use pmom et al. since they
        may be purged */
    if (!statobj || !statobj->pmom)
    {
        rtfs_set_errno(PEINVALIDPARMS); /* pc_gnext: statobj is not valid */
        return(FALSE);
    }
    if (!check_drive_number_mount(statobj->driveno))
        return(FALSE);
    pdrive = pc_drno2dr(statobj->driveno);
    if (statobj->drive_opencounter != pdrive->drive_opencounter)
    { /* Card was removed and re-inserted since pc_gfirst() */
        rtfs_set_errno(PEINVALIDPARMS); /* pc_gnext: statobj is not valid */
        release_drive_mount(statobj->driveno);/* Release lock, unmount if aborted */
        return(FALSE);
    }

    rtfs_set_errno(0);  /* po_gnext: clear error status */

    /* Now find the next instance of pattern in the directory   */
    nextobj = pc_get_inode((DROBJ *)(statobj->pobj), (DROBJ *)(statobj->pmom),
    statobj->pname, statobj->pext, GET_INODE_WILD);
    if (nextobj)
    {
        statobj->pobj = (void *)nextobj;
        /* And update the stat structure   */
        pc_upstat(statobj);
/* 9-20-94 release the FINODE and allocate a dummy. This will keep everyone
    who expects the drobj to own a finode happy but will not leave the
    finode open which locks out unlink et al */
         pc_freei(((DROBJ *)(statobj->pobj))->finode); /* Release the current */
/* 3-07-07 - Change: Remove additional call to pc_alloci(). Was not needed and caused a leak
   on a hot swap event when a gfirst is outstanding */
         ((DROBJ *)(statobj->pobj))->finode = 0;
/* END 9-20-94   */
        release_drive_mount(statobj->driveno);/* Release lock, unmount if aborted */
        return(TRUE);
    }
    else
    {
        if (get_errno() == PENOENT)
            rtfs_set_errno(0); /* get_inode: file not found in directory
                                  set errno to zero and return FALSE */
       /* pc_gnext: nextobj is 0 pc_get_inode() has set errno to PENOENT or to an internal or IO error status */
        release_drive_mount(statobj->driveno);/* Release lock, unmount if aborted */
        return(FALSE);
    }
}


/***************************************************************************
    PC_GDONE - Free internal resources used by pc_gnext and pc_gfirst.

 Description
    Given a pointer to a DSTAT structure that has been set up by a call to
    pc_gfirst() free internal elements used by the statobj.

    NOTE: You MUST call this function when done searching through a
    directory.

 Returns
    Nothing

    errno is set to one of the following
    0               - No error
    PEINVALIDPARMS - statobj argument is not valid
****************************************************************************/


void pc_gdone(DSTAT *statobj)                                   /*__apifn__*/
{
    DDRIVE *pdrive;
    VOID_CHECK_MEM()    /* Make sure memory is initted */
    /* see if the drive is still mounted. Do not use pmom et al. since they
        may be purged */
    /* see if the drive is still mounted. Do not use pmom et al. since they
        may be purged */
    if (!statobj || !statobj->pmom)
    {
        return;
    }
    if (!check_drive_number_mount(statobj->driveno))
        return;
    pdrive = pc_drno2dr(statobj->driveno);
    if (statobj->drive_opencounter != pdrive->drive_opencounter)
    { /* Card was removed and re-inserted since pc_gfirst() */
        release_drive_mount(statobj->driveno);/* Release lock, unmount if aborted */
        return;
    }
    if (statobj->pobj)
    {
        pc_freeobj((DROBJ *)statobj->pobj);
    }
    if (statobj->pmom)
        pc_freeobj((DROBJ *)statobj->pmom);
    release_drive_mount(statobj->driveno);/* Release lock, unmount if aborted */
    rtfs_memset((byte *) statobj,0,sizeof(*statobj));
}
/****************************************************************************
    PC_UPSTAT - Copy private information to public fields for a DSTAT struc.

 Description
    Given a pointer to a DSTAT structure that contains a pointer to an
    initialized DROBJ structure, load the public elements of DSTAT with
    name filesize, date of modification et al. (Called by pc_gfirst &
    pc_gnext)

 Returns
    Nothing


****************************************************************************/

/* Copy internal stuf so the outside world can see it   */
void pc_upstat(DSTAT *statobj)                                  /*__fn__*/
{
    DROBJ *pobj;
    FINODE *pi;
    pobj = (DROBJ *)(statobj->pobj);

    pi = pobj->finode;

    copybuff( statobj->fname, pi->fname, 8);
    statobj->fname[8] = CS_OP_ASCII('\0');
    copybuff( statobj->fext, pi->fext, 3);
    statobj->fext[3] = CS_OP_ASCII('\0');
    /* put null termed file.ext into statobj   */
    pc_ascii_mfile((byte *)statobj->filename, (byte *)statobj->fname,
             (byte *)statobj->fext);

    statobj->fattribute = pi->fattribute;
    statobj->ftime = pi->ftime;
    statobj->fdate = pi->fdate;
    statobj->fsize = pi->fsize;
    /* Get the lfn value for this object. If none available make
       sure it is a NULL string in ASCII or UNICODE */
    if (!pc_get_lfn_filename(pobj, (byte *)statobj->lfname))
    {
        statobj->lfname[0] = statobj->lfname[1] = 0;
        pc_cs_mfile((byte *)statobj->lfname, (byte *)statobj->fname,
             (byte *)statobj->fext);
    }
}
