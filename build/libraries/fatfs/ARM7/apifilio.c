/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APIFILEIO.C - Contains user api level file IO source code.

  The following routines are included:

    po_open         - Open a file.
    po_read         - Read bytes from a file.
    po_write        - Write Bytes to a file.
    po_lseek        - Move the file pointer.
    po_close        - Close a file and flush the file allocation table.
    pc_fd2file      -   Map a file descriptor to a file structure.
    pc_allocfile    -   Allocate a file structure.
    pc_freefile     -   Release a file structure.
    pc_free_all_fil -   Release all file structures for a drive
    _synch_file_ptrs -  make sure file pointers are synchronyzed
*/

#include <rtfs.h>

static void pc_freefile(PC_FILE *pfile);

/****************************************************************************
PO_OPEN -  Open a file.

  Description
  Open the file for access as specified in flag. If creating use mode to
  set the access permissions on the file.

    Flag values are

      PO_BINARY   - Ignored. All file access is binary
      PO_TEXT     - Ignored
      PO_RDONLY   - Open for read only
      PO_RDWR     - Read/write access allowed.
      PO_WRONLY   - Open for write only
      PO_CREAT    - Create the file if it does not exist. Use mode to
      specify the permission on the file.
      PO_EXCL     - If flag contains (PO_CREAT | PO_EXCL) and the file already
      exists fail and set xn_getlasterror() to EEXIST
      PO_TRUNC    - Truncate the file if it already exists
      PO_NOSHAREANY   - Fail if the file is already open. If the open succeeds
      no other opens will succeed until it is closed.
      PO_NOSHAREWRITE-  Fail if the file is already open for write. If the open
      succeeds no other opens for write will succeed until it
      is closed.

      Mode values are

      PS_IWRITE   - Write permitted
      PS_IREAD    - Read permitted. (Always true anyway)

      Returns
      Returns a non-negative integer to be used as a file descriptor for
      calling read/write/seek/close otherwise it returns -1.

      errno is set to one of the following
      0                 - No error
      PENOENT           - Not creating a file and file not found
      PEMFILE           - Out of file descriptors
      PEINVALIDPATH    - Invalid pathname
      PENOSPC          - No space left on disk to create the file
      PEACCES           - Is a directory or opening a read only file for write
      PESHARE          - Sharing violation on file opened in exclusive mode
      PEEXIST           - Opening for exclusive create but file already exists
      PEEXIST           - Opening for exclusive create but file already exists
      An ERTFS system error
****************************************************************************/

PCFD po_open(byte *name, word flag, word mode)  /*__apifn__*/
{
    PCFD fd;
    PC_FILE *pfile;
    CLUSTERTYPE  cluster;
    DROBJ *parent_obj;
    DROBJ *pobj;
    byte  *path;
    byte  *filename;
    byte  fileext[4];
    int driveno;
    BOOLEAN open_for_write;
    dword clusters_to_release;
#if (RTFS_SHARE)
    BOOLEAN sharing_error;
#endif
    dword ltemp;
    int p_errno;
    DDRIVE *pdrive;
    CHECK_MEM(PCFD, -1) /* Make sure memory is initted */

    rtfs_set_errno(0);  /* po_open: clear error status */

#if (RTFS_SHARE)
    sharing_error = FALSE;
#endif
    parent_obj = 0;

    open_for_write = FALSE;
    p_errno = 0;

#if (RTFS_WRITE)
    /* We will need to know this in a few places.   */
    if(flag & (PO_WRONLY|PO_RDWR))
        open_for_write = TRUE;
#endif
    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(name);
    if (driveno < 0)
    {
        /* errno was set by check_drive */
        return(-1);
    }
    if ( (fd = pc_allocfile()) < 0 )    /* Grab a file */
    {
        release_drive_mount(driveno); /* Release lock, unmount if aborted */
        rtfs_set_errno(PEMFILE);
        return(-1);
    }
    /* Get the FILE. This will never fail   */
    pfile = prtfs_cfg->mem_file_pool+fd;
    pdrive = pc_drno2dr(driveno);

    /* Use the buffers in the DRIVE structure. Access is locked via semaphore */
    path = (byte *)&(pdrive->pathname_buffer[0]);
    filename = (byte *)&(pdrive->filename_buffer[0]);

    /* Get out the filename and d:parent   */
    if (!pc_parsepath(path,filename,fileext,name))
    {
        p_errno = PEINVALIDPATH;
        goto errex;
    }

    /* Find the parent   */
    /* pc_fndnode will set errno */
    parent_obj = pc_fndnode(path);
    if (!parent_obj)
        goto errex;

    if (!pc_isadir(parent_obj) ||  pc_isavol(parent_obj))
    {
        p_errno = PENOENT;      /* Path is not a directory */
        goto errex;
    }

    pobj =  pc_get_inode(0, parent_obj,filename,(byte*)fileext, GET_INODE_MATCH);
    if (pobj)
    {
        /* If we goto exit: we want them linked so we can clean up   */
        pfile->pobj = pobj;         /* Link the file to the object */
#if (RTFS_SHARE)
        /* check the sharing conditions   */
        sharing_error = FALSE;
        if (pobj->finode->opencount != 1)
        {
        /* The file is already open by someone. Lets see if we are
            compatible */
            /* 1. We do not want to share with anyone   */
            if (flag & PO_NOSHAREANY)
                sharing_error = TRUE;
            /* 2. Someone else does not want to share   */
            if (pobj->finode->openflags & OF_EXCLUSIVE)
                sharing_error = TRUE;
            /* 3. We want exclusive write but already open for write   */
            if ( open_for_write && (flag & PO_NOSHAREWRITE) &&
                (pobj->finode->openflags & OF_WRITE))
                sharing_error = TRUE;
                /* 4. We want open for write but it is already open for
            exclusive */
            if ( (open_for_write) &&
                (pobj->finode->openflags & OF_WRITEEXCLUSIVE))
                sharing_error = TRUE;
            /* 5. Open for trunc when already open   */
            if (flag & PO_TRUNC)
                sharing_error = TRUE;
        }
        if (sharing_error)
        {
            p_errno = PESHARE;
            goto errex;
        }
#endif /* RTFS_SHARE */
        if( pc_isadir(pobj) || pc_isavol(pobj) )
        {
            p_errno = PEACCES;      /* is a directory */
            goto errex;
        }
#if (RTFS_WRITE)
        if ( (flag & (PO_EXCL|PO_CREAT)) == (PO_EXCL|PO_CREAT) )
        {
            p_errno = PEEXIST;      /* Exclusive fail */
            goto errex;
        }
        if(open_for_write && (pobj->finode->fattribute & ARDONLY) )
        {
            p_errno = PEACCES;      /* read only file */
            goto errex;
        }
        if (flag & PO_TRUNC)
        {
            cluster = pc_finode_cluster(pobj->pdrive,pobj->finode);
            ltemp   = pobj->finode->fsize;
            /* calculate clusters to release.
               add (pobj->pdrive->bytespcluster-1) to include if we are
               not on a cluster boundary */
            clusters_to_release = ltemp +
                                  (dword) (pobj->pdrive->bytespcluster-1);
            clusters_to_release = clusters_to_release >> (int) (pobj->pdrive->log2_secpalloc+9);

            pc_pfinode_cluster(pobj->pdrive,pobj->finode,0);
            pobj->finode->fsize = 0L;

            /* Convert to native. Overwrite the existing inode.Set archive/date  */
            if (!pc_update_inode(pobj, TRUE, TRUE))
            {
                /* pc_update_inode has set errno */
                pc_pfinode_cluster(pobj->pdrive,pobj->finode,cluster);
                pobj->finode->fsize = ltemp;
                goto errex;
            }

        /* Release the chain.  FATOP will set erno if needed
         - set min and max the same so it must delete exactly
         this many clusters
         Set min to 0 and max to 0xffffffff to eliminate range checking on the
         cluster chain and force removal of all clusters
        If freechain fails and it is not because of an invalid cluster, we
        let this pass and continue. Any other error is either internal or
        an IO error. */
           if (!FATOP(pobj->pdrive)->fatop_freechain(pobj->pdrive,cluster,clusters_to_release, clusters_to_release)
            && get_errno() != PEINVALIDCLUSTER)
           {
                goto errex;
           }
           /* Flush the FAT, bail out on any IO errors */
           else if (! FATOP(pobj->pdrive)->fatop_flushfat(pobj->pdrive->driveno) )
               goto errex;
        }
#endif
    }
    else    /* File not found */
    {
        if (get_errno() != PENOENT)
            goto errex;
#if (RTFS_WRITE)
        if (!(flag & PO_CREAT))
        {
            /* pc_get_inode() has set errno to PENOENT or to an internal or IO error status */
            goto errex; /* File does not exist */
        }
        rtfs_set_errno(0);      /* Clear PENOENT */
        /* Do not allow create if write bits not set   */
        if(!open_for_write)
        {
            p_errno = PEACCES;      /* read only file */
            goto errex;
        }
        /* Create for read only if write perm not allowed               */
        pobj = pc_mknode( parent_obj, filename, fileext, (byte) ((mode == PS_IREAD) ? ARDONLY : 0), 0);
        if (!pobj)
        {
            /* pc_mknode has set errno  */
            goto errex;
        }

        pfile->pobj = pobj;         /* Link the file to the object */
#else   /* Write not built in. Get out errno already set */
        goto errex;
#endif
    }
    /* Set the file sharing flags in the shared finode structure   */
    /* clear flags if we just opened it .                          */
    if (pobj->finode->opencount == 1)
        pobj->finode->openflags = 0;

    if (flag & PO_BUFFERED)
        pobj->finode->openflags |= OF_BUFFERED;

#if (RTFS_WRITE)
    if (open_for_write)
    {
        pobj->finode->openflags |= OF_WRITE;
        if (flag & PO_NOSHAREWRITE)
            pobj->finode->openflags |= OF_WRITEEXCLUSIVE;
    }
    if (flag & PO_NOSHAREANY)
        pobj->finode->openflags |= OF_EXCLUSIVE;

#endif
    pfile->flag = flag;         /* Access flags */
    pfile->fptr = 0L;           /* File pointer */

        /* Set the cluster and block file pointers   */
    if (!_synch_file_ptrs(pfile))
        goto errex;

    p_errno = 0;
    if (parent_obj)
        pc_freeobj(parent_obj);

    if (!release_drive_mount_write(driveno))/* Release lock, unmount if aborted */
        return(-1);
    return(fd);
errex:
    pc_freefile(pfile);
    if (parent_obj)
        pc_freeobj(parent_obj);
    if (p_errno)
        rtfs_set_errno(p_errno);
    release_drive_mount_write(driveno);/* Release lock, unmount if aborted */
    return(-1);
}


/****************************************************************************
PO_READ -  Read from a file.

  Description
  Attempt to read count bytes from the current file pointer of file at fd
  and put them in buf. The file pointer is updated.

Returns
Returns the number of bytes read or -1 on error.

errno is set to one of the following
0               - No error
PEBADF          - Invalid file descriptor
PEIOERRORREAD   - Read error
An ERTFS system error
*****************************************************************************/


int po_read(PCFD fd,    byte *in_buff, int count)    /*__apifn__*/
{
    PC_FILE *pfile;
    DDRIVE *pdrive;
    word block_in_cluster;
    word byte_offset_in_block;
    dword n_bytes, block_to_read, n_read,ltemp, n_left, n_to_read, n_w_to_read;
    CLUSTERTYPE  next_cluster, n_clusters;
    int p_errno, ret_val;
    int end_of_chain;
    CHECK_MEM(int, -1) /* Make sure memory is initted */


    ret_val = p_errno = 0;
    rtfs_set_errno(0);    /* po_read: clear errno */

    /* return 0 bytes read if bad arguments   */
    if (!count)
        goto return_unlocked;
    /* Get the file structure and semaphore lock the drive */
    if ( (pfile = pc_fd2file(fd, 0)) == 0)
    { /* fd2file set errno */
        ret_val = -1;
        goto return_unlocked;
    }
    pdrive = pfile->pobj->pdrive;

    if (pfile->fptr >= pfile->pobj->finode->fsize)    /* Dont read if done */
    {
        ret_val = 0;
        goto return_locked;
    }
    /* Set the cluster and block file pointers if not already set   */
    if (!_synch_file_ptrs(pfile))
    { /* _synch_file_ptrs set errno */
        ret_val = -1;
        goto return_locked;
    }

    /* Check if this write will wrap past 4 Gigabytes
       if so truncate the count (size limitted to 4 gig - 1 */
    ltemp = pfile->fptr + count;
    if (ltemp < pfile->fptr)
    {
        ltemp = 0xffffffff;
        count = ltemp - pfile->fptr;
    }
    ltemp = (dword) count;
    /* Truncate the read count if we need to   */
    if ( (pfile->fptr + ltemp) >= pfile->pobj->finode->fsize)
        ltemp = pfile->pobj->finode->fsize - pfile->fptr;

    n_left = ltemp;
    n_read = 0;

    while (n_left)
    {
        block_in_cluster = (word) (pfile->fptr & pdrive->byte_into_cl_mask);
        block_in_cluster >>= 9;
        block_to_read = pfile->fptr_block + block_in_cluster;

        /* how many clusters are left   */
        n_to_read = (n_left + 511) >> 9;
        n_clusters = (CLUSTERTYPE) ((n_to_read+block_in_cluster+pdrive->secpalloc-1) >> pdrive->log2_secpalloc); // AICP modified

        /* how many contiguous clusters can we get ? <= n_clusters   */
        end_of_chain = 0;
        n_clusters = FATOP(pdrive)->fatop_get_chain(pdrive, pfile->fptr_cluster,
            &next_cluster, n_clusters, &end_of_chain);

            /* Get_chain sets errno to PEDEVICE on fat access error PERESOURCE
        for bad cluster values */
        if (!n_clusters)
        {
            /* get_chain already set IO error */
             ret_val = (int) -1;
             goto return_locked;
        }

        /* Are we inside a block   */
        if ( (pfile->fptr & 0x1ffL) || (n_left < 512) )
        {
            /* pc_load_file_buffer will write the file buffer if it's dirty */
            if (!pc_load_file_buffer(pfile, block_to_read))
                break; /* pc_load_file_buffer has set errno */
            byte_offset_in_block = (word) (pfile->fptr & 0x1ffL);
            /* Copy source data to the local buffer   */
            n_bytes = (512 - byte_offset_in_block);
            if (n_bytes > n_left)
                n_bytes = n_left;
            if (in_buff)
                copybuff(in_buff, &(pfile->pobj->finode->pfile_buffer->data[byte_offset_in_block]), (int)n_bytes);
            /* If file is not buffered across calls, kill the buffer */
            if (!(pfile->pobj->finode->openflags & OF_BUFFERED))
                pc_load_file_buffer(pfile, 0);

            if (in_buff)
                in_buff += n_bytes;
            n_left = n_left - n_bytes;
            pfile->fptr += n_bytes;
            n_read += n_bytes;

            /* Are we on a cluster boundary  ?   */
            if (!(pfile->fptr & pdrive->byte_into_cl_mask))
            {
                if (--n_clusters)           /* If contiguous */
                {
                    pfile->fptr_block += pdrive->secpalloc;
                    pfile->fptr_cluster += (CLUSTERTYPE)1;
                }
                else
                {
                    /* Check for corrupted file
                        We are about to advancing fptr_cluster by
                        making next_cluster the current cluster.
                        If the file pointer is less than the current file
                        size, but we are at the end of chain we know
                        that there is no next_cluster and the chain is
                        corrupted. It shorter than the file size indicates.
                        Reset the byte pointer to match the current
                        block and cluster pointers, set errno to
                        PEINVALIDCLUSTER, return -1 */
                    if (pfile->fptr < pfile->pobj->finode->fsize && end_of_chain)
                    {
                        pfile->fptr -= n_bytes;
                        p_errno = PEINVALIDCLUSTER;
                        ret_val = -1;
                        goto return_locked;
                    }
                    else
                    {
                        pfile->fptr_cluster = next_cluster;
                        pfile->fptr_block = pc_cl2sector(pdrive, next_cluster);
                    }
                }   /* if (--nclusters) {} else {}; */
            }       /* if (!(pfile->fptr & pdrive->byte_into_cl_mask)) */
        }           /* if ( (pfile->fptr & 0x1ff) || (n_left < 512) ) */
        else
        {
            /* Read as many blocks as possible             */
            /* How many blocks in the current chain    */
            n_to_read = n_clusters << pdrive->log2_secpalloc;
            /* subtract out any leading blocks   */
            n_to_read = n_to_read - block_in_cluster;
            /* How many blocks yet to read   */
            ltemp = n_left >> 9;
            /* take the smallest of the two   */
            if (n_to_read > ltemp)
                n_to_read = ltemp;

            if (n_to_read)
            {
                /* If we get here we need to read contiguous blocks   */
                block_to_read = pfile->fptr_block + block_in_cluster;
                ltemp = n_to_read;
                while (ltemp)
                {
                    if (ltemp > 0xffff)
                        n_w_to_read = 0xffff;
                    else
                        n_w_to_read = ltemp & 0xffff;
                    /* Flush the file block buffer if it was in our range */
                    /* And then read the data */
                    pdrive->drive_flags |= DRIVE_FLAGS_FILEIO;
                    if (!pc_sync_file_buffer(pfile, block_to_read, n_w_to_read, TRUE) ||
                        (in_buff && !devio_read(pdrive->driveno, block_to_read, in_buff, (word)n_w_to_read, FALSE)))
                    {
                        pdrive->drive_flags &= ~DRIVE_FLAGS_FILEIO;
                        /* set errno to IO error unless devio set PEDEVICE */
                        if (!get_errno())
                            p_errno = PEIOERRORREAD;
                        ret_val = (int) n_read;
                        goto return_locked;
                    }
                    pdrive->drive_flags &= ~DRIVE_FLAGS_FILEIO;
                    n_bytes = n_w_to_read << 9;
                    if (in_buff)
                        in_buff += n_bytes;
                    block_to_read += n_w_to_read;
                    ltemp -= n_w_to_read;
                }
                n_bytes = n_to_read << 9;
                n_left= n_left - n_bytes;
                pfile->fptr += n_bytes;
                n_read += n_bytes;

                /* if we advanced to a cluster boundary advance the
                cluster pointer */
                /* ltemp ==s how many clusters we read   */
                ltemp = (n_to_read+block_in_cluster) >> pdrive->log2_secpalloc;

                if (ltemp == n_clusters)
                {
                    /* Check for corrupted file
                        We are about to advance fptr_cluster by
                        making next_cluster the current cluster.
                        If the file pointer is less than the current file
                        size, but we are at the end of chain we know
                        that there is no next_cluster and the chain is
                        corrupted. It shorter than the file size indicates.
                        Reset the byte pointer to match the current
                        block and cluster pointers, set errno to
                        PEINVALIDCLUSTER, return -1 */
                    if (pfile->fptr < pfile->pobj->finode->fsize && end_of_chain)
                    {
                        pfile->fptr -= n_bytes;
                        p_errno = PEINVALIDCLUSTER;
                        ret_val = -1;
                        goto return_locked;
                    }
                    else
					{
						/* Changed Oct 25, 2005. Set eof flag if
						   the file pointer is at the ens of file */
						if (pfile->fptr == pfile->pobj->finode->fsize)
						{
						   pfile->at_eof = TRUE;
						}
                        pfile->fptr_cluster = next_cluster;
					}
                }
                else
                {
                    /* advance the pointer as many as we read   */
                    pfile->fptr_cluster =  (CLUSTERTYPE) (pfile->fptr_cluster + ltemp);
                }
                pfile->fptr_block = pc_cl2sector(pdrive, pfile->fptr_cluster);
            }
        }
    }       /* while n_left */
    ret_val = (int) n_read;

return_locked:
    release_drive_mount(pdrive->driveno);/* Release lock, unmount if aborted */
return_unlocked:
    if (p_errno)
       rtfs_set_errno(p_errno);
    return(ret_val);
}

/**************************************************************************
PO_LSEEK    -  Move file pointer

  Description
  Move the file pointer offset bytes from the origin described by
  origin. The file pointer is set according to the following rules.

    Origin              Rule
    PSEEK_SET           offset from begining of file
    PSEEK_CUR           offset from current file pointer
    PSEEK_END           offset from end of file

      Attempting to seek beyond end of file puts the file pointer one
      byte past eof.

       Returns
       Returns the new offset or -1 on error.

        errno is set to one of the following
        0               - No error
        PEBADF          - Invalid file descriptor
        PEINVALIDPARMS  - Attempt to seek past EOF or to a negative offset
        PEINVALIDCLUSTER- Files contains a bad cluster chain
        An ERTFS system error
*****************************************************************************/

long po_lseek(PCFD fd, long offset, int origin)       /*__apifn__*/
{
    PC_FILE *pfile;
    DDRIVE *pdrive;
    long    ret_val;
    CHECK_MEM(long, -1) /* Make sure memory is initted */

    rtfs_set_errno(0);    /* po_lseek: clear error status */
    /* Get the file structure and semaphore lock the drive */
    pfile = pc_fd2file(fd, 0);
    if (!pfile)
    { /* fd2file set errno */
        return(-1L);
    }

    pdrive = pfile->pobj->pdrive;
    /* Grab exclusive access to the drobj   */
    /*
    * Note the files finode is LOCKED below so the code need not be
    * reentrant relative to the finode.
    */
    /* Set the cluster and block file pointers if not already set   */
    if (_synch_file_ptrs(pfile))
    { /* _synch_file_ptrs set errno */
        /* Call the internal seek routine that we share with po_trunc   */
        ret_val =  _po_lseek(pfile, offset, origin);
    }
    else
        ret_val = -1;

    release_drive_mount(pdrive->driveno);/* Release lock, unmount if aborted */
    return(ret_val);
}

/**************************************************************************
PO_ULSEEK    -  Move file pointer (unsigned)

    BOOLEAN po_ulseek(PCFD fd, dword offset, dword *pnew_offset, int origin)

  Description
  Move the file pointer offset bytes from the origin described by
  origin. The file pointer is set according to the following rules.

    Origin              Rule
    PSEEK_SET           Positive unsigned 32 bit offset from begining of file
    PSEEK_CUR           Positive unsigned 32 bit offset from current file pointer
    PSEEK_CUR_NEG       Unsigned 32 bit offset subtracted from current file pointer
    PSEEK_END           Unsigned 32 bit offset subtracted from current file end


    The new file pointer is returned in *pnew_offset

   Returns
       TRUE - The seek was succesful and the new offset is in *pnew_offset.
       FALSE - An error occured

       If FALSE is returned, errno is set to one of the following

        0               - No error
        PEBADF          - Invalid file descriptor
        PEINVALIDPARMS  - Attempt to seek past EOF or to a negative offset
        PEINVALIDCLUSTER- Files contains a bad cluster chain
        An ERTFS system error
*****************************************************************************/

BOOLEAN po_ulseek(PCFD fd, dword offset, dword *pnew_offset, int origin)       /*__apifn__*/
{
    PC_FILE *pfile;
    DDRIVE *pdrive;
    BOOLEAN    ret_val;
    CHECK_MEM(BOOLEAN, FALSE) /* Make sure memory is initted */

    rtfs_set_errno(0);    /* po_lseek: clear error status */
    /* Get the file structure and semaphore lock the drive */
    pfile = pc_fd2file(fd, 0);
    if (!pfile)
    { /* fd2file set errno */
        return(FALSE);
    }

    pdrive = pfile->pobj->pdrive;
    /* Grab exclusive access to the drobj   */
    /*
    * Note the files finode is LOCKED below so the code need not be
    * reentrant relative to the finode.
    */
    /* Set the cluster and block file pointers if not already set   */
    if (_synch_file_ptrs(pfile))
    { /* _synch_file_ptrs set errno */
        /* Call the internal seek routine that we share with po_trunc   */
        ret_val = _po_ulseek(pfile, offset, pnew_offset, origin);
    }
    else
        ret_val = FALSE;

    release_drive_mount(pdrive->driveno);/* Release lock, unmount if aborted */
    return(ret_val);
}

/**************************************************************************
_PO_ULSEEK   -  Move file pointer (internal)

Description
Behaves as po_lseek but takes a file instead of a file descriptor.

    Attempting to seek beyond end of file puts the file pointer one
    byte past eof.

    All setting up such as drive_enter and drobj_enter should have been done
    before calling here.

        Called By:

        po_lseek and po_truncate.

            Returns
            Returns the new offset or -1 on error.

            If the return value is -1 xn_getlasterror() will be set with one of the following:

                PENBADF     - File descriptor invalid
                PEINVAL     - Seek to negative file pointer attempted.

*****************************************************************************/
/*
* Note: when this routine is caled the files finode is LOCKED so the code
* need not be reentrant relative to the finode.
*/




BOOLEAN _po_ulseek(PC_FILE *pfile, dword offset, dword *new_offset, int origin)       /*__fn__*/
{
    dword file_pointer;
    DDRIVE *pdrive;
    BOOLEAN l_at_eof;
    int  log2_bytespcluster;
    dword  ltemp;
    dword  ltemp2;
    CLUSTERTYPE  n_clusters_to_seek;
    CLUSTERTYPE  n_clusters;
    CLUSTERTYPE  first_cluster, next_cluster;
    dword    ret_val;
    dword  alloced_size;
    int end_of_chain;
    int p_errno;

    p_errno = 0;

    pdrive = pfile->pobj->pdrive;
    /* If file is zero sized. we are there   */
    if (!(pfile->pobj->finode->fsize))
    {
        *new_offset = 0;
        ret_val = TRUE;
        goto errex;
    }
    *new_offset = pfile->fptr;
    ret_val = FALSE;

    if (origin == PSEEK_SET)        /*  offset from begining of file */
        file_pointer = offset;
    else if (origin == PSEEK_CUR)   /* offset from current file pointer */
    {
        file_pointer =  pfile->fptr;
        file_pointer += offset;
        /* See if it wraps past 4 Gig.. if so stop it at 4 gig */
        if (file_pointer < pfile->fptr)
            file_pointer = 0xffffffff;
    }
    else if (origin == PSEEK_CUR_NEG)   /* offset from current file pointer */
    {
        file_pointer =  pfile->fptr;
        if (file_pointer > offset)
            file_pointer -= offset;
        else
            file_pointer = 0;

    }
    else if (origin == PSEEK_END)   /*  offset from end of file */
    {
        file_pointer =  pfile->pobj->finode->fsize;
        if (file_pointer > offset)
            file_pointer -= offset;
        else
            file_pointer = 0;
    }
    else    /* Illegal origin */
    {
        p_errno = PEINVALIDPARMS;
        goto errex;
    }

    if (file_pointer >= pfile->pobj->finode->fsize)
    {
        file_pointer = pfile->pobj->finode->fsize;

        /* If seeking to the end of file see if we are beyond the allocated size of
        the file. If we are we set the at_eof flag so we know to try to move the
        cluster pointer in case another file instance extends the file
        Round the file size up to its cluster size by adding in clustersize-1
        and masking off the low bits */
        alloced_size =  (pfile->pobj->finode->fsize + pdrive->byte_into_cl_mask) &
            ~(pdrive->byte_into_cl_mask);
        if (alloced_size < pfile->pobj->finode->fsize)
            alloced_size = 0xffffffff;

        /* If the file pointer is beyond the space allocated to the file note it
        since we may need to adjust this files cluster and block pointers
        later if someone else extends the file behind our back */
        if (file_pointer >= alloced_size)
            l_at_eof = TRUE;
        else
            l_at_eof = FALSE;
    }
    else
    {
        l_at_eof = FALSE;
    }


    log2_bytespcluster = (int) (pdrive->log2_secpalloc + 9);

    /* How many clusters do we need to seek                       */
    /* use the current cluster as the starting point if we can    */
    if (file_pointer >= pfile->fptr)
    {
        first_cluster = pfile->fptr_cluster;

        ltemp =  file_pointer;

        ltemp >>= log2_bytespcluster;
        ltemp2 = pfile->fptr  >> log2_bytespcluster;
        /* If this algorithm was run twice n_clusters_to_seek would
        end up -1. Which would still work but spin 64K times.
        Thanks top Morgan Woodson of EMU systems for the patch */
        if (ltemp >= ltemp2)
            n_clusters_to_seek = (CLUSTERTYPE) (ltemp - ltemp2);
        else
            n_clusters_to_seek = (word) 0;
    }
    else
    {
        /* seek from the beginning    */
        first_cluster = pc_finode_cluster(pdrive,pfile->pobj->finode);
        ltemp = file_pointer >> log2_bytespcluster;
        n_clusters_to_seek = (CLUSTERTYPE) ltemp;
    }
    next_cluster = first_cluster; /* Set next cluster to first cluster so if n_clusters_to_seek is zero it's harmless. */
    while (n_clusters_to_seek)
    {
        end_of_chain = 0;
        n_clusters = FATOP(pdrive)->fatop_get_chain(pdrive, first_cluster,
            &next_cluster, n_clusters_to_seek, &end_of_chain);
        if (!n_clusters) /* get_chain stops when it reaches n_clusters_to_seek */
        {
            /* get_chain already set errno */
            ret_val = FALSE;
            goto errex;
        }
        /* Check for corrupted file
            If first cluster == next cluster it means that we started at
            the last cluster in the chain and tried to seek foreward. This
            indicates that the file length in the directory entry is longer
            than the actual cluster chain. Check is not valid if the
            fointer is at the EOF mark. */
        if (!l_at_eof && first_cluster == next_cluster)
        {
            p_errno = PEINVALIDCLUSTER;
            ret_val = FALSE;
            goto errex;
        }

        /* How many left to seek */
        n_clusters_to_seek = (CLUSTERTYPE) (n_clusters_to_seek - n_clusters);
        /* Check for corrupted file
            If there are more clusters to seek but the cluster pointer
            is at the end this means that the file size is
            longer than the actual chain length so return
            erro leave the byte block and cluster pointers alone
            and set errno to PEINVALIDCLUSTER */
        if (n_clusters_to_seek && end_of_chain)
        {
            p_errno = PEINVALIDCLUSTER;
            ret_val = FALSE;
            goto errex;
        }
        else
            first_cluster = next_cluster;
    }

    pfile->fptr_cluster = next_cluster;
    pfile->fptr_block = pc_cl2sector(pdrive, next_cluster);
    pfile->fptr= file_pointer;
    pfile->at_eof = l_at_eof;

    *new_offset = pfile->fptr;
    ret_val = TRUE;
errex:
    /* No only errors return through here. Everything does.   */
    if (p_errno)
        rtfs_set_errno(p_errno);
    return(ret_val);
}
/**************************************************************************
_PO_LSEEK   -  Move file pointer (internal)

Description
Behaves as po_lseek but takes a file instead of a file descriptor.

    Attempting to seek beyond end of file puts the file pointer one
    byte past eof.

    All setting up such as drive_enter and drobj_enter should have been done
    before calling here.

        Called By:

        po_lseek and po_truncate.

            Returns
            Returns the new offset or -1 on error.

            If the return value is -1 xn_getlasterror() will be set with one of the following:

                PENBADF     - File descriptor invalid
                PEINVAL     - Seek to negative file pointer attempted.

*****************************************************************************/
/*
* Note: when this routine is caled the files finode is LOCKED so the code
* need not be reentrant relative to the finode.
*/


long _po_lseek(PC_FILE *pfile, long offset, int origin)       /*__fn__*/
{
    long    ret_val;
    int u_origin;
    dword new_offset/*, u_offset*/;

    /*u_offset = offset;*/
    u_origin = origin;

    if (origin == PSEEK_CUR)   /* offset from current file pointer */
    {
        if (offset < 0)
        {
            offset = -offset;
            /*u_offset = (dword) offset;*/
            u_origin = PSEEK_CUR_NEG;
        }
    }
    else if (origin == PSEEK_END)   /*  offset from end of file */
    {
        if (offset < 0)
        {
            offset = -offset;
            /*u_offset = (dword) offset;*/
        }
    }
    if (!_po_ulseek(pfile, offset, &new_offset, u_origin))
        ret_val = -1L;
    else
        ret_val = (long) new_offset;
    return(ret_val);
}

/****************************************************************************
PO_CLOSE  -  Close a file.

  Description
  Close the file updating the disk and freeing all core associated with FD.

Returns
Returns 0 if all went well otherwise it returns -1.

errno is set to one of the following
0               - No error
PEBADF          - Invalid file descriptor
An ERTFS system error
****************************************************************************/

int po_close(PCFD fd)  /*__apifn__*/
{
    PC_FILE *pfile;
    int ret_val;
    int driveno;
    int  p_errno;
    CHECK_MEM(int, -1)  /* Make sure memory is initted */

    p_errno = 0;
    ret_val = 0;
    rtfs_set_errno(0);    /* po_close() - clear errno */

    /* Get the file structure and semaphore lock the drive */
    if ( (pfile = pc_fd2file(fd, 0)) == 0)
    {
        /* Add a check here to see if fd2file failed because it was
           closed by pc_dskfree. If so we mark the file free here and
           return success */
        if (get_errno() == PECLOSED)
        {
            OS_CLAIM_FSCRITICAL()
            pfile = prtfs_cfg->mem_file_pool+fd;
            pfile->is_free = TRUE;
            OS_RELEASE_FSCRITICAL()
            rtfs_set_errno(0);    /* clear errno */
            return (0);
        }
        /* all other errors. fd2file set errno */
        return(-1);
    }
    else
    {
        driveno = pfile->pobj->pdrive->driveno;

#if (RTFS_WRITE)
        if (pfile->flag & ( PO_RDWR | PO_WRONLY ) )
        {
            if (!_po_flush(pfile))
            {
                /* _po_flush has set errno */
                ret_val = -1;
            }
        }
#endif
        /* Release the FD and its core   */
        pc_freefile(pfile);
        if (!release_drive_mount_write(driveno))/* Release lock, unmount if aborted */
            return (-1);
    }
    if (p_errno)
        rtfs_set_errno(p_errno);
    return(ret_val);
}

/****************************************************************************
Miscelaneous File and file descriptor management functions

These functions are private functions used by the po_ file io routines.

    pc_fd2file -
    Map a file descriptor to a file structure. Return null if the file is
    not open. If an error has occured on the file return NULL unless
    allow_err is true.

    pc_allocfile -
    Allocate a file structure an return its handle. Return -1 if no more
    handles are available.

        pc_freefile -
        Free all core associated with a file descriptor and make the descriptor
        available for future calls to allocfile.

        pc_free_all_fil -
*****************************************************************************/

/*  Map a file descriptor to a file structure. Return null if the file is
not open or the flags do not match (test for write access if needed).
Get the file structure and semaphore lock the drive
*/

PC_FILE *pc_fd2file(PCFD fd,int flags)                      /*__fn__*/
{
    PC_FILE *pfile;
    DDRIVE *pdrive;

    /* Get the file and associated drive structure with the crit sem locked */
    if (0 <= fd && fd < pc_nuserfiles())
    {
        pfile = prtfs_cfg->mem_file_pool+fd;
        OS_CLAIM_FSCRITICAL()
        if (!pfile->is_free)
        {
            if (!pfile->pobj)
            {
                /* An event (probably card removal or failure)
                   closed the file. Set errno and return. The
                   user must call po_close to clear this condition */
                rtfs_set_errno(PECLOSED);
            }
        /* If flags == 0. Any access allowed. Otherwise at least one
            bit in the file open flags must match the flags sent in */
            else if (!flags || (pfile->flag&flags))
            {
                /* dereference pobj while critical semaphore is still locked */
                pdrive = pfile->pobj->pdrive;
                OS_RELEASE_FSCRITICAL()
                /* Claim the drive, double check that the file is still
                    good (was not closed out by a pc_dskfree() call) */
                OS_CLAIM_LOGDRIVE(pdrive->driveno)  /* pc_fd2file - Register Drive in use */
                if (pfile->pobj)
                    return(pfile);
                else
                {
                     /* An event (probably card removal or failure)
                        closed the file. Set errno and return. The
                        user must call po_close to clear this condition */
                        OS_RELEASE_LOGDRIVE(pdrive->driveno)  /* pc_fd2file - clear Drive on error */
                        rtfs_set_errno(PECLOSED);
                        return(0);
                }
            }
            else
                rtfs_set_errno(PEACCES);
        }
        else
            rtfs_set_errno(PEBADF);
        OS_RELEASE_FSCRITICAL()
    }
    else
        rtfs_set_errno(PEBADF);
    return(0);
}

/* Assign zeroed out file structure to an FD and return the handle. Return
-1 on error. */
PCFD pc_allocfile(void)                                         /*__fn__*/
{
    PC_FILE *pfile;
    PCFD i;

    OS_CLAIM_FSCRITICAL()
    pfile = prtfs_cfg->mem_file_pool;
    for (i=0;i<pc_nuserfiles();i++, pfile++)
    {
        if (pfile->is_free)
        {
            rtfs_memset(pfile, 0, sizeof(PC_FILE));
            OS_RELEASE_FSCRITICAL()
            return(i);
        }
    }
    OS_RELEASE_FSCRITICAL()
    return (-1);
}

/* Free core associated with a file descriptor. Release the FD for later use   */
static void pc_freefile(PC_FILE *pfile)
{
DROBJ *pobj;
    OS_CLAIM_FSCRITICAL()
    pobj = pfile->pobj;
    pfile->is_free = TRUE;
    OS_RELEASE_FSCRITICAL()
    if (pobj)
       pc_freeobj(pobj);
}


#define ENUM_FLUSH 1
#define ENUM_TEST  2
#define ENUM_FREE  3

/* Release all file descriptors associated with a drive and free up all core
associated with the files called by pc_free_all_fil, pc_flush_all_fil,
pc_test_all_fil. The drive semaphore must be locked before this call is
entered.
*/
int pc_enum_file(DDRIVE *pdrive, int chore)             /*__fn__*/
{
    PC_FILE *pfile;
    DROBJ *pobj;
    PCFD i;
    int dirty_count;

    dirty_count = 0;
    for (i=0; i < pc_nuserfiles(); i++)
    {
        OS_CLAIM_FSCRITICAL()
        pfile = prtfs_cfg->mem_file_pool+i;
        if (pfile && !pfile->is_free && pfile->pobj && pfile->pobj->pdrive == pdrive)
        {
            OS_RELEASE_FSCRITICAL()
#if (RTFS_WRITE)
            if (chore == ENUM_FLUSH)
            {
                if (!_po_flush(pfile))
                    return(-1);
            }
#endif
            if (chore == ENUM_TEST)
            {
                if (pfile->needs_flush)
                    dirty_count += 1;
            }
            if (chore == ENUM_FREE)
            {
                /* Mark the file closed here. po_close must release it */
                OS_CLAIM_FSCRITICAL()
                pobj = pfile->pobj;
                pfile->pobj = 0;
                OS_RELEASE_FSCRITICAL()
                if (pobj)
                    pc_freeobj(pobj);
            }
        }
        else
        {
            OS_RELEASE_FSCRITICAL()
        }
    }
    return(dirty_count);
}

/* Release all file descriptors associated with a drive and free up all core
associated with the files called by dsk_close */
void pc_free_all_fil(DDRIVE *pdrive)                                /*__fn__*/
{
    pc_enum_file(pdrive, ENUM_FREE);
}

/* Flush all files on a drive   */
BOOLEAN pc_flush_all_fil(DDRIVE *pdrive)                                /*__fn__*/
{
    if (pc_enum_file(pdrive, ENUM_FLUSH) == 0)
        return(TRUE);
    else
        return(FALSE);
}

/* Test the dirty flag for all files   */
int pc_test_all_fil(DDRIVE *pdrive)                             /*__fn__*/
{
    return(pc_enum_file(pdrive, ENUM_TEST));
}



/* Synchronize file pointers. Read write Seek and close all call here.
This fixes the following BUGS:
1. If a file is created and left open and then opened again with a new
file handle before any writing takes place. Neither file will get
its fptr_cluster set correctly initially. The first one to write
would get set up correctly but the other would not. Thus if fptr_cluster
is zero we see if we can set it.
2. If one file seeked to the end of the file or has written to the end of
the file its file pointer will point beyond the last cluster in the
chain, the next call to write will notice the fptr is beyond the
file size and extend the file by allocating a new cluster to the
chain. During this time the cluster/block and byte offsets are
out of synch. If another instance extends the file during this time
the next call to write will miss this condition since fptr is not
>= fsize any more. To fix this we note in the file when this
condition is true AND, afterwards each time we work with the file
we see if the file has grown and adjust the cluster pointer and block
pointer if needed.
*/
/*
* Note: The finode owned by the file is always locked when this routine is
*  called so the routine does not need to be reentrant with respect to
*  the finode. Note too that pfile is not a shared structure so the
*  routine does not have to be reentrant with respect to it either.
*/

BOOLEAN _synch_file_ptrs(PC_FILE *pfile)      /*__fn__*/
{
    CLUSTERTYPE clno;

    if (!pfile->fptr_cluster)
    {
        pfile->fptr_cluster = pc_finode_cluster(pfile->pobj->pdrive,pfile->pobj->finode);

        if (pfile->fptr_cluster)
            pfile->fptr_block = pc_cl2sector(pfile->pobj->pdrive, pfile->fptr_cluster);
        else
            pfile->fptr_block = 0;
    }
    if (pfile->at_eof)
    {
        if (pfile->fptr_cluster)
        {
            clno = FATOP(pfile->pobj->pdrive)->fatop_clnext(pfile->pobj->pdrive, pfile->fptr_cluster);
            if (clno == 0)  /* clnext detected an error */
                return(FALSE);
            else if (clno != 0xffffffff)
            {
                pfile->fptr_cluster = clno;
                pfile->fptr_block = pc_cl2sector(pfile->pobj->pdrive, pfile->fptr_cluster);
                pfile->at_eof = FALSE;
            }
        }
    }
    return(TRUE);
}

/*  pc_flush_file_buffer(PC_FILE *pfile) -
* If the finode structure that the file points to contains a file buffer
* that has been modified, but not written to disk, write the data to disk.
*/

BOOLEAN pc_flush_file_buffer(PC_FILE *pfile)
{
BLKBUFF *pfile_buffer;
    pfile_buffer = pfile->pobj->finode->pfile_buffer;
    /* write it if it changed */
    if (pfile_buffer && pfile->pobj->finode->file_buffer_dirty)
    {
    dword save_drive_filio;
        save_drive_filio = pfile_buffer->pdrive->drive_flags & DRIVE_FLAGS_FILEIO;
        pfile_buffer->pdrive->drive_flags |= DRIVE_FLAGS_FILEIO;
        if (!(devio_write(pfile_buffer->pdrive->driveno,pfile_buffer->blockno,
                pfile_buffer->data, (int) 1, FALSE) ))
        {
            if (!save_drive_filio)
                pfile_buffer->pdrive->drive_flags &= ~DRIVE_FLAGS_FILEIO;
            /* set errno to IO error unless devio set PEDEVICE */
            if (!get_errno())
                rtfs_set_errno(PEIOERRORWRITEBLOCK); /* device write error */
            return(FALSE);
        }
        if (!save_drive_filio)
             pfile_buffer->pdrive->drive_flags &= ~DRIVE_FLAGS_FILEIO;
        /* Clear dirty condition */
        pfile->pobj->finode->file_buffer_dirty = 0;
    }
    return(TRUE);
}

/* pc_load_file_buffer(PC_FILE *pfile, dword new_blockno)
*
* If new_blockno is 0, and the finode structure that the file points to
* contains a file buffer, then flush and then discard the file_buffer.
* If new_blockno is not 0, and the finode structure that the file points to
* contains a file buffer, then flush and then load the file_buffer with the
* contents at new_blockno.
*
*/


BOOLEAN pc_load_file_buffer(PC_FILE *pfile, dword new_blockno)
{
BLKBUFF *pfile_buffer;

    pfile_buffer = pfile->pobj->finode->pfile_buffer;
    if (pfile_buffer)
    {
        /* See if we already have it */
        if (pfile_buffer->blockno == new_blockno)
            return(TRUE);
        /* If not, write it if it changed */
        if (!pc_flush_file_buffer(pfile))
        {
            pfile->pobj->finode->pfile_buffer = 0;
            pc_free_scratch_blk(pfile_buffer);
            return(FALSE);
        }
        pfile->pobj->finode->pfile_buffer = 0;
        pc_free_scratch_blk(pfile_buffer);
    }
    if (new_blockno)
    {
    dword save_drive_filio;
        pfile_buffer = pc_scratch_blk();
        if (!pfile_buffer)
             return(FALSE); /* pc_scratch_blk set errno */
        pfile_buffer->blockno = new_blockno;
        pfile_buffer->pdrive = pfile->pobj->pdrive;
        save_drive_filio = pfile_buffer->pdrive->drive_flags & DRIVE_FLAGS_FILEIO;
        pfile_buffer->pdrive->drive_flags |= DRIVE_FLAGS_FILEIO;
        if (!(devio_read(pfile_buffer->pdrive->driveno,pfile_buffer->blockno,
                pfile_buffer->data, (int) 1, FALSE) ))
        {
            if (!save_drive_filio)
                pfile_buffer->pdrive->drive_flags &= ~DRIVE_FLAGS_FILEIO;
            /* set errno to IO error unless devio set PEDEVICE */
            if (!get_errno())
                rtfs_set_errno(PEIOERRORREADBLOCK); /* device read error */
            pfile->pobj->finode->pfile_buffer = 0;
            pc_free_scratch_blk(pfile_buffer);
            return(FALSE);
        }
        if (!save_drive_filio)
            pfile_buffer->pdrive->drive_flags &= ~DRIVE_FLAGS_FILEIO;
        pfile->pobj->finode->pfile_buffer = pfile_buffer;
    }
    return(TRUE);
}

/* pc_sync_file_buffer(PC_FILE *pfile,
*     dword start_block, dword nblocks, BOOLEAN doflush)
* If the finode structure that the file points to contains a file buffer
* that is in the range of start_block to start_block+nblocks,
* Then:
*  If doflush is true and the buffer has been modified, write the data
*  to disk.
*  Discard the buffer
*/
BOOLEAN pc_sync_file_buffer(PC_FILE *pfile, dword start_block, dword nblocks, BOOLEAN doflush)
{
BLKBUFF *pfile_buffer;
    pfile_buffer = pfile->pobj->finode->pfile_buffer;
    if (pfile_buffer)
    {
        if (pfile_buffer->blockno >= start_block && pfile_buffer->blockno < (start_block+nblocks))
        {
            /* If not flushing make sure dirty status is clear so we don't write */
            if (!doflush)
                pfile->pobj->finode->file_buffer_dirty = 0;
            /* discard buffer, pc_load_file_buffer will write it if it's dirty */
            return(pc_load_file_buffer(pfile, 0));
        }
    }
    return(TRUE);
}
