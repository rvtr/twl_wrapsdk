/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APIWRITE.C - Contains user api level file IO source code.

    The following routines are included:
    po_write        - Write Bytes to a file.
    po_flush        - Flush an open file
    po_trunc        - Truncate an open file
    pc_set_attributes - Set File Attributes
    pc_diskflush    -  Flush the FAT and all files on a disk
*/

#include <rtfs.h>

/***************************************************************************
    PO_WRITE    -  Write to a file.

 Description
    Attempt to write count bytes from buf to the current file pointer of file
    at fd. The file pointer is updated.

 Returns
    Returns the number of bytes written or -1 on error.

    errno is set to one of the following
    0               - No error
    PEBADF          - Invalid file descriptor
    PEACCES         - File is read only
    PEIOERRORWRITE  - Error performing write
    PEIOERRORREAD   - Error reading block for merge and write
    PENOSPC         - Disk full
    An ERTFS system error
****************************************************************************/

int po_write(PCFD fd, byte *in_buff, int count)                   /*__apifn__*/
{
    PC_FILE *pfile;
    DDRIVE *pdrive;
    dword block_in_cluster;
    dword byte_offset_in_block;
    dword n_blocks_left;
    dword n_to_write, n_w_to_write;
    CLUSTERTYPE  next_cluster;
    CLUSTERTYPE  n_clusters;
    dword  ltemp;
    int n_bytes;
    int n_written;
    int n_left;
    dword  alloced_size;
    dword  block_to_write;
    int end_of_chain;
    BOOLEAN extending_file;
    int ret_val;
    int p_errno;

    CHECK_MEM(int, -1) /* Make sure memory is initted */

        /* Return 0 (none written) on bad args   */
    if (!count)
        return(0);

    p_errno = 0;
    rtfs_set_errno(0);  /* po_write: clear error status */
    /* Get the FILE. must be open for write   */
    /* Get the file structure and semaphore lock the drive */
    pfile = pc_fd2file(fd, PO_WRONLY|PO_RDWR);
    if (!pfile)
    { /* fd2file set errno */
        ret_val = -1;
        goto return_unlocked;
    }
    pdrive = pfile->pobj->pdrive;
    /* Only one process may write at a time   */
    /* if the  file is zero sized make sure the current cluster pointer
        is invalid */
    if (!pfile->pobj->finode->fsize)
        pfile->fptr_cluster = 0;

    /* Round the file size up to its cluster size by adding in clustersize-1
        and masking off the low bits */
    alloced_size =  (pfile->pobj->finode->fsize + pdrive->byte_into_cl_mask) &
                    ~(pdrive->byte_into_cl_mask);
    if (alloced_size < pfile->pobj->finode->fsize)
        alloced_size = 0xffffffff;

    /* Set the cluster and block file pointers if not already set   */
    if (!_synch_file_ptrs(pfile))
    { /* _synch_file_ptrs set errno */
        ret_val = -1;
        goto return_locked;
    }
    /* Seek to the end if append mode   */
    if (pfile->flag & PO_APPEND)
    {
        if (pfile->pobj->finode->fsize)
        {
            if (!_po_ulseek(pfile, 0L, &ltemp, PSEEK_END))
            { /* po_ulseek will set error status */
                ret_val = -1;
                goto return_locked;
            }
            if (!_synch_file_ptrs(pfile))
            { /* _synch_file_ptrs set errno */
                ret_val = -1;
                goto return_locked;
            }
        }
    }
    /* Check if this write will wrap past 4 Gigabytes
       if so truncate the count to 4 Gig minus 1*/
    ltemp = pfile->fptr + count;
    if (ltemp < pfile->fptr)
    {
#if (RTFS_TRUNCATE_WRITE_TO_MAX)
        ltemp = 0xffffffff;
        count = ltemp - pfile->fptr;
#else
        p_errno = PETOOLARGE;
        ret_val = -1;
        goto return_locked;
#endif
    }
    if (ltemp > RTFS_MAX_FILE_SIZE)
    {
#if (RTFS_TRUNCATE_WRITE_TO_MAX)
        ltemp = RTFS_MAX_FILE_SIZE;
        count = ltemp - pfile->fptr;
#else
        p_errno = PETOOLARGE;
        ret_val = -1;
        goto return_locked;
#endif
    }

    /* calculate initial values   */
    n_left = count;
    n_written = 0;

    while (n_left)
    {
        end_of_chain = 0;
        block_in_cluster = (dword) (pfile->fptr & pdrive->byte_into_cl_mask);
        block_in_cluster >>= 9;

        if (pfile->fptr >= alloced_size)
        {
            /* Extending the file   */
            extending_file = TRUE;
            ltemp = (dword) n_left;
            n_blocks_left = (dword) ((ltemp + 511) >> 9);
            /* how many clusters are left-
            *  assume 1 for the current cluster.
            *   subtract out the blocks in the current
            *   round up by adding secpalloc-1 and then
            *   divide by sectors per cluster

            |  n_clusters = 1 +
            |   (n_blocks_left-
            |       (pdrive->secpalloc-block_in_cluster)
            |       + pdrive->secpalloc-1) >> pdrive->log2_secpalloc;
                ==>
            */
            n_clusters = ( CLUSTERTYPE ) (1 +
            ((n_blocks_left + block_in_cluster -1) >> pdrive->log2_secpalloc));

            /* Call pc_alloc_chain to build a chain up to n_cluster clusters
                long. Return the first cluster in pfile->fptr_cluster and
                return the # of clusters in the chain. If pfile->fptr_cluster
                is non zero link the current cluster to the new one */
            n_clusters = FATOP(pdrive)->fatop_alloc_chain(pdrive, &(pfile->fptr_cluster), n_clusters, TRUE);
            if (!n_clusters)
            { /* Allocchain will set errno to PENOSPC or an IO or internal error */
                /* Handle PENOSPC as a short write - otherwise it is an error that returns -1*/
                if (get_errno() == PENOSPC)
                    break;
                else
                {
                    ret_val = (int) -1;
                    goto return_locked;
                }
            }

            /* Calculate the last cluster in this chain.   */
            next_cluster = (CLUSTERTYPE) (pfile->fptr_cluster + n_clusters -1);
            /* link the chain to the directory object if just starting   */
            if (!pc_finode_cluster(pfile->pobj->pdrive,pfile->pobj->finode))
                pc_pfinode_cluster(pfile->pobj->pdrive,pfile->pobj->finode,pfile->fptr_cluster);

            /* calculate the new block pointer   */
            pfile->fptr_block = pc_cl2sector(pdrive, pfile->fptr_cluster);

            /* calculate amount of space used by the file   */
            ltemp = n_clusters << pdrive->log2_secpalloc; ltemp <<= 9;
            alloced_size += ltemp;
            if (alloced_size < ltemp)
                alloced_size = 0xffffffff;

        }
        else        /* Not extending the file. (writing inside the file) */
        {
            extending_file = FALSE;
            ltemp = (dword) n_left;
            n_blocks_left = (dword) ((ltemp + 511) >> 9);
            /* how many clusters are left-
            *  assume 1 for the current cluster.
            *   subtract out the blocks in the current
            *   round up by adding secpalloc-1 and then
            *   divide by sectors per cluster

            |  n_clusters = 1 +
            |   (n_blocks_left-
            |       (pdrive->secpalloc-block_in_cluster)
            |       + pdrive->secpalloc-1) >> pdrive->log2_secpalloc;
                ==>
            */
           n_clusters = (CLUSTERTYPE) (1 +
               ((n_blocks_left + block_in_cluster -1) >> pdrive->log2_secpalloc));

            /* how many contiguous clusters can we get ? <= n_clusters   */
            end_of_chain = 0;
            n_clusters = FATOP(pdrive)->fatop_get_chain(pdrive, pfile->fptr_cluster,
                                        &next_cluster, n_clusters, &end_of_chain);
            if (!n_clusters)
            {
                /* get_chain already set errno */
                 ret_val = (int) -1;
                 goto return_locked;
            }
        }

        /* Are we inside a block   */
        if ( (pfile->fptr & 0x1ffL) || (n_left < 512) )
        {
            block_in_cluster = (dword) (pfile->fptr & pdrive->byte_into_cl_mask);
            block_in_cluster >>= 9;
            block_to_write = pfile->fptr_block + block_in_cluster;

            byte_offset_in_block = (dword) (pfile->fptr & 0x1ffL);

            /* Copy source data to the local buffer   */
            n_bytes = (int) (512 - byte_offset_in_block);
            if (n_bytes > n_left)
                n_bytes = n_left;
            /* READ - Use the block buffer pool to read for us    */
            if (!pc_load_file_buffer(pfile, block_to_write))
                break;  /* load_file_block set errno */
            /* Merge the data and mark it dirty */
            if (in_buff)
                copybuff(&(pfile->pobj->finode->pfile_buffer->data[byte_offset_in_block]), in_buff, n_bytes);
            pfile->pobj->finode->file_buffer_dirty = 1;
            if (!(pfile->pobj->finode->openflags & OF_BUFFERED))
            {
                /* Write the buffer. and discard it   */
                if (!pc_load_file_buffer(pfile, 0))
                    break;  /* load_file_block set errno */
            }
            if (in_buff)
                in_buff += n_bytes;

            n_left = (int) (n_left - n_bytes);
            pfile->fptr += n_bytes;
            n_written += n_bytes;

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
                        We are about to advance fptr_cluster by
                        making next_cluster the current cluster.
                        If the file pointer is less than the current file
                        size, but we are at the end of chain we know
                        that there is no next_cluster and the chain is
                        corrupted. It shorter than the file size indicates.
                        Reset the byte pointer to match the current
                        block and cluster pointers, set errno to
                        PEINVALIDCLUSTER, return -1
                        We check against alloced_size here because next
                        time through the write routine will check and if
                        fptr >= alloced_size then it will link a new cluster
                        to the file.
                    */
                    if (!extending_file &&
                        pfile->fptr < alloced_size &&
                        end_of_chain)
                    {
                        pfile->fptr -= n_bytes;
                        p_errno = PEINVALIDCLUSTER;
                        ret_val = -1;
                        goto return_locked;
                    }
                    else
                    {
    /* NOTE:            Put the next cluster into the pointer. If we had
                        alloced a chain this value is the last cluster in
                        the chain and does not concur with the byte file pointer.
                        This is not a problem since the cluster pointer is known
                        to be off at this point any (fptr>=alloced_size) */
                        pfile->fptr_cluster = next_cluster;
                        pfile->fptr_block = pc_cl2sector(pdrive, next_cluster);
                    }
                }   /* if (--nclusters) {} else {}; */
            }       /* if (!(pfile->fptr & byte_into_cl_mask)) */
        }           /* if ( (pfile->fptr & 0x1ff) || (n_left < 512) ) */

        if (n_clusters && (n_left>511))
        {
            /* If we get here we need to write contiguous blocks   */
            block_in_cluster = (dword) (pfile->fptr & pdrive->byte_into_cl_mask);
            block_in_cluster >>= 9;
            block_to_write = pfile->fptr_block + block_in_cluster;
            /* how many do we write ?   */
            ltemp = (dword) n_left;
            n_blocks_left = (dword) (ltemp >> 9);
            n_to_write = (dword) ((n_clusters << pdrive->log2_secpalloc) - block_in_cluster);

            if (n_to_write > n_blocks_left)
            {
                n_to_write = n_blocks_left;

            /* If we are not writing to the end of the chain we may not
                advance the cluster pointer to the beginning of the next
                chain. We add in block_in_cluster so we account for the
                partial cluster we have already seen */
                next_cluster = (CLUSTERTYPE) (pfile->fptr_cluster +
                                ((n_to_write+block_in_cluster) >>  pdrive->log2_secpalloc));
            }
            /* Devio takes words for blocks so split it up */
            ltemp = n_to_write;
            while (ltemp)
            {
            	if (ltemp > 0xffff)
                    n_w_to_write = 0xffff;
                else
                    n_w_to_write = ltemp & 0xffff;
                /* Set pdrive->drive_flags to tell the device driver we are perfroming
                   file data transfer */
                pdrive->drive_flags |= DRIVE_FLAGS_FILEIO;
                if (in_buff && !devio_write(pdrive->driveno, block_to_write, in_buff, (word) n_w_to_write, FALSE))
                {
                   pdrive->drive_flags &= ~DRIVE_FLAGS_FILEIO;
                   /* set errno to IO error unless devio set PEDEVICE */
                   if (!get_errno())
                        p_errno = PEIOERRORWRITE;
                    ret_val = n_written;
                    goto return_locked;
                }
                pdrive->drive_flags &= ~DRIVE_FLAGS_FILEIO;
                /* Purge the file block buffer if it was in our range, since we will overwrite */
                pc_sync_file_buffer(pfile, block_to_write, n_w_to_write, FALSE);
                n_bytes = (int) n_w_to_write * 512;
                if (in_buff)
                    in_buff = in_buff + n_bytes;
                block_to_write += n_w_to_write;
                ltemp = ltemp - n_w_to_write;
            }
/* See note above   */
            n_bytes = (int) n_to_write * 512;
            n_left = n_left - n_bytes;
            pfile->fptr += n_bytes;
            n_written += n_bytes;
            pfile->fptr_cluster = next_cluster;
            pfile->fptr_block   = pc_cl2sector(pdrive, next_cluster);
        }
    }       /* while n_left */
    ret_val = n_written;
    /* If we wrote anything change the last modified time and date */
    if (pfile->pobj && ret_val > 0)
    {
    DATESTR crdate;
        pc_getsysdate(&crdate);
        pfile->pobj->finode->fattribute |= ARCHIVE;
        pfile->pobj->finode->ftime = crdate.time;
        pfile->pobj->finode->fdate = crdate.date;
        pfile->needs_flush = TRUE;
    }

return_locked:
    if (pfile->pobj && (pfile->fptr > pfile->pobj->finode->fsize))
    {
        pfile->needs_flush = TRUE;
        pfile->pobj->finode->fsize = pfile->fptr;
        pfile->pobj->finode->fattribute |= ARCHIVE;
    }
    /* If the file pointer is beyond the space allocated to the file note it.
        since we may need to adjust this files cluster and block pointers
        later if someone else extends the file behind our back */
    if (pfile->fptr >= alloced_size)
        pfile->at_eof = TRUE;
    else
        pfile->at_eof = FALSE;
    /* If the file is open in auto flush mode flush directory entry changes to disk */
    if (pfile->flag & PO_AFLUSH)
        if (!_po_flush(pfile))
            ret_val = -1;
    if (!release_drive_mount_write(pdrive->driveno))/* Release lock, unmount if aborted */
        return(-1);
return_unlocked:
    if (p_errno)
        rtfs_set_errno(p_errno);
    return(ret_val);
}

/**************************************************************************
    PO_TRUNCATE -  Truncate an open file.

 Description
    Move the file pointer offset bytes from the beginning of the file
    and truncate the file beyond that point by adjusting the file size
    and freeing the cluster chain past the file pointer.

 Returns
    Returns TRUE if successful otherwise FALSE

    errno is set to one of the following
    0               - No error
    PEBADF          - Invalid file descriptor
    PEACCES         - File is read only or opened more than once
    PEINVALIDPARMS  - Invalid or inconsistent arguments
    An ERTFS system error
*****************************************************************************/


BOOLEAN po_truncate(PCFD fd, dword offset)                               /*__apifn__*/
{
    PC_FILE *pfile;
    DDRIVE *pdrive;
    BOOLEAN ret_val;
    CLUSTERTYPE first_cluster_to_release;
    CLUSTERTYPE last_cluster_in_chain;
    CLUSTERTYPE clno;
    int p_errno;
    dword clusters_to_release;
    dword old_chain_len;
    dword new_chain_len;
    dword range_check;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    p_errno = 0;
    rtfs_set_errno(0);  /* po_truncate: clear error status */

    /* Assume failure */
     ret_val = FALSE;

    /* Get the FILE. must be open for write   */
    /* Get the file structure and semaphore lock the drive */
    pfile = pc_fd2file(fd, PO_WRONLY|PO_RDWR);
    /* Make sure we have write privilages   */
    if (!pfile)
        goto return_error;

    pdrive = pfile->pobj->pdrive;

    /* Can only truncate a file that you hold exclusively   */
    if (pfile->pobj->finode->opencount > 1)
    {
        p_errno = PEACCES;
        goto errex;
    }

    /* Set the cluster and block file pointers if not already set   */
    if (!_synch_file_ptrs(pfile))
    { /* _synch_file_ptrs set errno */
        goto errex;
    }

    /* Make sure the file buffer is clear */
    pc_load_file_buffer(pfile, 0);

    if (offset > pfile->pobj->finode->fsize)
    {
        p_errno = PEINVALIDPARMS;
        goto errex;
    }

    /* Call the internal seek routine that we share with po_lseek. Seek to
        offset from the origin of zero. */
    if (!_po_ulseek(pfile, offset, &offset, PSEEK_SET))
        goto errex; /* _po_lseek() set errno */
    else if (offset == pfile->pobj->finode->fsize)
    {
        ret_val = TRUE;
    }
    else
    {
        pfile->needs_flush = TRUE;

        /* calculate maximum number of clusters we will need to release */
        /* Round the file size up to its cluster size by adding in clustersize-1
        and masking off the low bits   */
        old_chain_len =  pc_chain_length(pdrive, pfile->pobj->finode->fsize);
        new_chain_len =  pc_chain_length(pdrive, offset);

        clusters_to_release = old_chain_len - new_chain_len;

        /* Are we on a cluster boundary ?   */
        if (!(offset & pdrive->byte_into_cl_mask))
        {

        /* Free the current cluster and beyond since we are on a cluster boundary.   */
            first_cluster_to_release = pfile->fptr_cluster;
            /* Find the previous cluster so we can terminate the chain   */
            clno = pc_finode_cluster(pdrive,pfile->pobj->finode);
            last_cluster_in_chain = clno;
            range_check = 0;
            while (clno != first_cluster_to_release)
            {
                if ((clno < 2) || (clno > pdrive->maxfindex) || (++range_check > old_chain_len) )
                {
                    rtfs_set_errno(PEINVALIDCLUSTER);
                    goto errex;
                }
                last_cluster_in_chain = clno;
                clno = FATOP(pdrive)->fatop_clnext(pdrive , clno); /* File */
                if (!clno)
                { /* FATOP will set errno */
                    goto errex;
                }
            }
            /* Set ptr_cluster to last in chain so read&write will work right   */
            pfile->fptr_cluster = last_cluster_in_chain;
            if (last_cluster_in_chain)
            {
                if ((clno < 2) || (clno > pdrive->maxfindex))
                {
                    rtfs_set_errno(PEINVALIDCLUSTER);
                    goto errex;
                }

                pfile->fptr_block = pc_cl2sector(pdrive, last_cluster_in_chain);
            }
            else
                pfile->fptr_block = 0;
            pfile->at_eof = TRUE;
        }
        else        /* Simple case. we are not on a cluster boundary. Just free*/
        {           /* The chain beyond us and terminate the list */
            last_cluster_in_chain = pfile->fptr_cluster;
            first_cluster_to_release = FATOP(pdrive)->fatop_clnext(pdrive, pfile->fptr_cluster); /* File */
            if (!first_cluster_to_release) /* if 0 and not end of chain it's an error */
            { /* clnext set errno */
                goto errex;
            }
            /* Clear cluster number if clnext returned eof */
            if (first_cluster_to_release == 0xffffffff)
                first_cluster_to_release = 0;
            pfile->at_eof = TRUE;
        }

        /*  Now update the directory entry.   */
        pfile->pobj->finode->fsize = offset;
        if (!offset)    /* If the file goes to zero size unlink the chain */
        {
            pc_pfinode_cluster(pdrive,pfile->pobj->finode,0);
            pfile->fptr_cluster = 0;
            pfile->fptr_block = 0;
            pfile->fptr = 0;
            pfile->at_eof = FALSE;
            /* We are freeing the whole chain so we do not mark last_cluster in chain   */
            last_cluster_in_chain = 0;
        }
        ret_val = TRUE; /* If it doesn't get changed to false, it worked */
        /* Convert to native. Overwrite the existing inode.Set archive/date  */
        if (!pc_update_inode(pfile->pobj, TRUE, TRUE))
            ret_val = FALSE;
        /* Terminate the chain and free the lost chain part   */
        /* Free the rest of the chain   */
        if (ret_val && clusters_to_release && first_cluster_to_release)
        {
            /* Release the chain FATOP will set erno if needed - set min and max
               the same so it must delete exactly this many clusters */
            if (!FATOP(pdrive)->fatop_freechain(pdrive, first_cluster_to_release, clusters_to_release, clusters_to_release))
                ret_val = FALSE;
        }
        /* Null terminate the chain   */
        if (ret_val && last_cluster_in_chain)
        {
            if (!FATOP(pdrive)->fatop_pfaxxterm(pdrive, last_cluster_in_chain)) /* File */
                ret_val = FALSE;
        }
        if (ret_val && !FATOP(pdrive)->fatop_flushfat(pdrive->driveno))
            ret_val = FALSE;
    }

errex:
    if (!release_drive_mount_write(pdrive->driveno))/* Release lock, unmount if aborted */
        return(FALSE);
return_error:   /* No only errors return through here. Everything does. */
    if (p_errno)
        rtfs_set_errno(p_errno);
    return(ret_val);
}

/*
* Note: when this routine is caled the files finode is LOCKED so the code
* need not be reentrant relative to the finode.
*/
/* Internal version of po_flush() called by po_flush and po_close   */
BOOLEAN _po_flush(PC_FILE *pfile)                                       /*__fn__*/
{
    /*---------- ctr modified ----------*/
    int		driveno;
    DDRIVE	*pdr;
    /*----------------------------------*/
    BOOLEAN ret_val;
    
   /* Flush the file buffer if it is in use */
    if (!pc_flush_file_buffer(pfile))
        return(FALSE);
    ret_val = TRUE;
    /* Convert to native. Overwrite the existing inode.Set archive
       set date. */
    if (pfile->needs_flush)
    {
        if (pc_update_inode(pfile->pobj, TRUE, TRUE))
        { /* pc_update_inode and FATOP both set errno */
            pfile->needs_flush = FALSE;
            /* Flush the file allocation table   */
            if (!FATOP(pfile->pobj->pdrive)->fatop_flushfat(pfile->pobj->pdrive->driveno))
                ret_val = FALSE;
        }
        else
            ret_val = FALSE;
    }
    
    /*---------- ctr modified ----------*/
    driveno = pfile->pobj->pdrive->driveno;
    pdr = pc_drno_to_drive_struct( driveno);
    if( pdr) {
        if( pdr->dev_table_perform_device_ioctl) {
	        if( pdr->dev_table_perform_device_ioctl( driveno,
                       			                     DEVCTL_FLUSH,
                                                     NULL) != 0) {
                ret_val = FALSE;
            }
        }
    }
    /*----------------------------------*/
    return(ret_val);
}

/****************************************************************************
    PO_FLUSH  -  Flush a file.

 Description
    Flush the file updating the disk.

 Returns
    Returns TRUE if all went well otherwise it returns FALSE.

    errno is set to one of the following
    0               - No error
    PEBADF          - Invalid file descriptor
    PEACCES         - File is read only
    An ERTFS system error
****************************************************************************/

BOOLEAN po_flush(PCFD fd)                                           /*__apifn__*/
{
    PC_FILE *pfile;
    BOOLEAN ret_val;
    int driveno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    rtfs_set_errno(0);  /* po_flush: clear error status */

    /* Get the FILE. must be open for write   */
    /* Get the file structure and semaphore lock the drive */
    pfile = pc_fd2file(fd, PO_WRONLY|PO_RDWR);
    if (pfile)
    {
        driveno = pfile->pobj->pdrive->driveno;
        ret_val = _po_flush(pfile); /* _po_flush() will set errno */
        if (!release_drive_mount_write(driveno))/* Release lock, unmount if aborted */
            ret_val = -1;
        return(ret_val);
    }
    else
    {  /* fd2file set errno */
        return(FALSE);
    }
}
/*****************************************************************************
    pc_set_attributes - Set File Attributes

 Description
    Given a file or directory name return set directory entry attributes
    associated with the entry.

    The following values may be set:

    BIT Nemonic
    0       ARDONLY
    1       AHIDDEN
    2       ASYSTEM
    5       ARCHIVE

    Note: bits 3 & 4 (AVOLUME,ADIRENT) may not be changed.


 Returns
    Returns TRUE if successful otherwise it returns FALSE.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PEINVALIDPATH   - Path specified badly formed.
    PENOENT         - Path not found
    PEACCESS        - Object is read only
    PEINVALIDPARMS  - attribute argument is invalid
    An ERTFS system error
****************************************************************************/

BOOLEAN pc_set_attributes(byte *path, byte attributes)          /*__apifn__*/
{
    DROBJ  *pobj;
    BOOLEAN ret_val;
    int driveno;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    rtfs_set_errno(0);  /* pc_set_attributes: clear error status */
    if ((attributes&(0x40|0x80)) !=0 ) /* Illegal */
    {
        rtfs_set_errno(PEINVALIDPARMS);
        return(FALSE);
    }
    driveno = check_drive_name_mount(path);
    if (driveno < 0)
    {  /* errno was set by check_drive */
        return (FALSE);
    }
    ret_val = FALSE;
    /* pc_fndnode will set errno */
    pobj = pc_fndnode(path);
    if (pobj)
    {
        /* Change the attributes if legal   */
        if (
              (attributes & (AVOLUME|ADIRENT)) ==   /* Still same type */
              (pobj->finode->fattribute & (AVOLUME|ADIRENT)))
        {
            pobj->finode->fattribute = attributes;
            /* Overwrite the existing inode. Do not set archive/date  */
            /* pc_update_inode() will set errno */
            ret_val = pc_update_inode(pobj, FALSE, FALSE);
        }
        else
            rtfs_set_errno(PEACCES);

        pc_freeobj(pobj);
    }
    if (!release_drive_mount_write(driveno))/* Release lock, unmount if aborted */
        ret_val = FALSE;
    return (ret_val);
}

/****************************************************************************
    PC_DISKFLUSH -  Flush the FAT and all files on a disk

 Description

    If an application may call this functions to force all files
    to be flushed and the fat to be flushed. After this call returns
    the disk image is synchronized with RTFSs internal view of the
    voulme.

Returns
    TRUE if the disk flushed else no

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    An ERTFS system error
****************************************************************************/

BOOLEAN pc_diskflush(byte *path)                                        /*__apifn__*/
{
    int driveno;
    DDRIVE *pdrive;
    BOOLEAN ret_val;
    CHECK_MEM(BOOLEAN, 0)   /* Make sure memory is initted */

    rtfs_set_errno(0);  /* pc_diskflush: clear error status */
    ret_val = FALSE;
    driveno = check_drive_name_mount(path);
    /*  if error, errno was set by check_drive */
    if (driveno >= 0)
    {
        /* Find the drive   */
        pdrive = pc_drno2dr(driveno);
        if (pdrive)
        {
            /*---------- ctr modified ----------*/
            if (pc_flush_all_fil(pdrive)) {
                if (FATOP(pdrive)->fatop_flushfat(driveno)) {
                    ret_val = TRUE;
                }
            }
            if( pdrive->dev_table_perform_device_ioctl) {
                if( pdrive->dev_table_perform_device_ioctl( driveno,
                                                            DEVCTL_FLUSH,
                                                            NULL) == 0) {
                    ret_val = TRUE;
                }
            }/*----------------------------------*/
        }

        if (!release_drive_mount_write(driveno))/* Release lock, unmount if aborted */
           ret_val = FALSE;
    }
    return(ret_val);
}

/*ctr modified
  本来、pc_diskflushはRTFSのバッファを吐き出す処理を行うが、
  nandはドライバ側でもバッファを持っているためデバイスに反映されない。
  ドライバ側のバッファも同期してデバイスに吐き出すようにするための変更。
  ファイルであれば_po_flushにDEVCTL_FLUSHを引っ掛けていることにより対策。
  mkdir, rmdirなどは、ファイルでないため_po_flushを素通りしてしまうので、
  pc_diskflushにもDEVCTL_FLUSHを引っ掛けることにより対策。
  これだと上位層でpc_diskflushを呼ばないといけないのでmkdir,rmdirなどに
  も引っ掛けている。（pc_diskflushでDEVCTL_FLUSHは削除してよいかも）
 */
