/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* APIREALT.C - Contains user level real time and special user level
   file and FAT manipuation code.

    The following routines are included:

    pc_custer_size  - Return the number of bytes per cluster for a drive
    po_extend_file  - Extend a file by with contiguous clusters.
    po_chsize      -  Truncate or extend an open file.
    pc_get_file_extents - Get the list of block segments that make up a file
    pc_raw_read    - Read blocks directly from a disk
    pc_raw_write   - Write blocks directly to a disk
    pc_get_free_list - Get a list free cluster segments on the drive

*/

#include <rtfs.h>

/*  PC_CLUSTER_SIZE  - Return the number of bytes per cluster for a drive

 Description
        This function will return the cluster size mounted device
        named in the argument.


 Returns
    The cluster size or zero if the device is not mounted.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive name is invalid
*****************************************************************************/
int pc_cluster_size(byte *drive)                                /*__apifn__*/
{
int driveno;
int ret_val;
DDRIVE *pdrive;
CHECK_MEM(int, 0)   /* Make sure memory is initted */

    rtfs_set_errno(0);  /* pc_cluster_size: clear error status */
    /* Get the drive and make sure it is mounted   */
    driveno = check_drive_name_mount(drive);
    if (driveno < 0)
    {
        /* errno was set by check_drive */
        return(0);
    }
    pdrive = pc_drno2dr(driveno);
    ret_val = pdrive->bytespcluster;
    release_drive_mount(driveno);/* Release lock, unmount if aborted */
    return(ret_val);
}

/****************************************************************************

Name:

    po_extend_file  - Extend a file by with contiguous clusters.

Summary:

    BOOLEAN po_extend_file(PCFD fd, dword_bytes, dword_*new_bytes, dword start_cluster,
                        int method)

 Description
    Given a file descriptor, n_bytes bytes  and method, extend the file and
    update the file size.
    If the file can be extended by n_bytes contiguous bytes it will be done.
    Otherwise if there is no contiguos region left on the disk that can
    contain n_bytes the routine DOES NOT extend the file but does return
    the length of the next largest contiguous region that is avaliable.
    With this scheme an application can request to extend the file by a
    given contiguous amount. If that is not possible the routine will return
    the largest contiguous region available, the application can then decide
    if it wishes to use this region. Please read the notes below for an
    important discussion about the limitations of this routine.

    The special method PC_FIXED_FIT may be used to extend the file beginning
    at a specific cluster. With this method, start_cluster must be provided.
    It is used as the starting point for the allocation. With this method it
    is possible to precisely assign locations on the disk to file section.
    This may be used for example to create interleaved files where several
    files share a disk segment with interleaving clusters.


    Method may be one of the following:
       PC_FIRST_FIT  - The first chain  in which the extension will fit
       PC_BEST_FIT   - The smallest chain in which the extension will fit
       PC_WORST_FIT  - The largest chain in which the extension will fit
       PC_FIXED_FIT  - Extend n_clusters from start cluster

    Please note the following issues and limitations.

        . PC_FIRST_FIT is significantly faster than the others
        . If the current end of file is not on a cluster boundary
          then the region to be tested will start at the cluster
          immediatley following the last cluster in the file and the
          routine will allocate from the segment that starts with
          that cluster or it will return the number of contiguous
          bytes available starting at that cluster.
        . If possible you should allocate space in contiguouos regions
          that are a multiple of the drive s cluster size.
        . If the PC_FIXED_FIT option is selected a start cluster must
        must be supplied, n_bytes must be an even multiple of cluster
        size and the file to extend must be either zero
        sized or the end of file must be on a cluster boundary.

 Returns
   FALSE if an error occured.
   TRUE if an error did not occur.

   Returns n_bytes in *new_bytes if the file was extended. Otherwise it
   returns the largest contiguous chain of bytes available in *new_bytes.
   If it n_bytes is not returned the  files was not extended.

   errno is set to one of these values
    0               - No error
    PEBADF          - Invalid file descriptor
    PEACCES         - File is read only
    PEINVALIDPARMS  - Invalid or inconsistent arguments
    An ERTFS system error
*****************************************************************************/

BOOLEAN po_extend_file(PCFD fd, dword n_bytes, dword *new_bytes, dword start_cluster, int method) /* __apifn__ */
{
    BOOLEAN ret_val;
    CLUSTERTYPE clno;
    dword n_alloced;
    dword ltemp;
    CLUSTERTYPE n_clusters;
    CLUSTERTYPE largest_chain;
    CLUSTERTYPE first_cluster;
    CLUSTERTYPE last_cluster_in_chain;
    CLUSTERTYPE i;
    dword alloced_size;
    dword new_alloced_size;
    dword new_file_size;
    dword range_check;
    dword clusters_in_chain;

    PC_FILE *pfile;
    DDRIVE *pdr;
    CHECK_MEM(BOOLEAN, 0)    /* Make sure memory is initted */

    if (!n_bytes)
    {
        ret_val = TRUE;
        *new_bytes = 0;
        goto return_error;
    }

    rtfs_set_errno(0); /* po_extend_file: clear error status */

    /* Assume error to start   */
    ret_val = FALSE;
    /* Get the file structure and semaphore lock the drive */
    pfile = pc_fd2file(fd, PO_WRONLY|PO_RDWR);
    /* Make sure we have write privileges. Make sure we got a  count   */
    if (!pfile)
    {   /* fd2file set errno */
        goto return_error;
    }
    first_cluster = 0;
    /* From here on we exit through alloc_done so we will unlock these resources   */
    pdr = pfile->pobj->pdrive;
    /* Make sure our file pointer is ok   */
    if (!_synch_file_ptrs(pfile))
    { /* synch file pointer set errno */
        goto alloc_done;
    }
    new_file_size = pfile->pobj->finode->fsize + n_bytes;
    /* Fail if it exceeds MAX_FILE_SIZE or it wraps 32 bit offset max */
    if (new_file_size > RTFS_MAX_FILE_SIZE || new_file_size < pfile->pobj->finode->fsize)
    {
        rtfs_set_errno(PETOOLARGE); /* po_extend_file: new size too large */
        goto alloc_done;
    }
    new_alloced_size = (new_file_size + pdr->byte_into_cl_mask) &
                    ~(pdr->byte_into_cl_mask);
    if (new_alloced_size < new_file_size)
        new_alloced_size = 0xffffffff;

    /* Round the file size up to its cluster size by adding in clustersize-1
        and masking off the low bits */
    alloced_size = (pfile->pobj->finode->fsize + pdr->byte_into_cl_mask) &
                    ~(pdr->byte_into_cl_mask);
    if (alloced_size < pfile->pobj->finode->fsize)
        alloced_size = 0xffffffff;

    if (method == PC_FIXED_FIT)
    {
        /* Check for errors. If the old size was not on a cluster
           boundary or the new size is not on a cluster boundary
           or now start cluster was provided then its an error
           for the FIXED_FIT option
        */
        if (    ( new_alloced_size != new_file_size) ||
                (alloced_size != pfile->pobj->finode->fsize) ||
                !start_cluster)
        {
            /* If using fixed fit and not on a cluster boundary
           it is an error */
            rtfs_set_errno(PEINVALIDPARMS); /* po_extend_file: fixed fit and not on a cluster boundary */
            goto alloc_done;
        }
    }
    n_clusters = pc_chain_length(pdr, new_file_size) -
                 pc_chain_length(pdr, pfile->pobj->finode->fsize);

    if (n_clusters)
    {    /* We need space so try to allocate */
         /* Find the end of the files chain   */
        last_cluster_in_chain = pfile->fptr_cluster;
        clno = pfile->fptr_cluster;
        {
            dword ltemp,fptr_mod_cluster;
            /* Round fptr down to cluster boundary */
            fptr_mod_cluster =  pfile->fptr & ~(pdr->byte_into_cl_mask);
            /* Subtract from the rounded up alloced_size value */
            ltemp = alloced_size-fptr_mod_cluster;
            clusters_in_chain = ltemp >> (int) (pfile->pobj->pdrive->log2_secpalloc+9);
        }
        if (clusters_in_chain)
        {
            if ((clno < 2) || (clno > pdr->maxfindex) )
            {
                rtfs_set_errno(PEINVALIDCLUSTER);
                goto alloc_done;
            }
            range_check = 0;
            while (clno != 0xffffffff)
            {
                if (++range_check > clusters_in_chain)
                    break;
                last_cluster_in_chain = clno;
                clno = FATOP(pdr)->fatop_clnext(pdr , clno); /* File */
                if (!clno)  /* clnext set errno */
                    goto alloc_done;
            }
            if (range_check != clusters_in_chain)
            {
                rtfs_set_errno(PEINVALIDCLUSTER);
                goto alloc_done;
            }
            if (get_errno())
                goto alloc_done; /* If FATOP set errno we have trouble */
        }
        n_alloced   = 0;
        largest_chain = 0;
        if ((method == PC_FIXED_FIT) || (alloced_size != pfile->pobj->finode->fsize))
        {
            /* If the end of file is not on a cluster boundary
               see how many are avalailable that are contiguous
               with the last cluster in our file */
            /* Also have special code here since FIXED_FIT Works this way */
            if (method == PC_FIXED_FIT)
                clno = (CLUSTERTYPE) start_cluster;
            else
                clno = (CLUSTERTYPE) (last_cluster_in_chain + 1);

            ltemp = n_clusters;
            /* Walk up the FAT linearly. try for n_clusters */
            while (ltemp)
            {
                if (!FATOP(pdr)->fatop_faxx(pdr, clno, &i) || i != 0) /* File */
                    break;
                else
                {
                   largest_chain++;
                   ltemp--;
                   clno++;
                }
            }
            if (get_errno())
                goto alloc_done; /* If FATOP set errno we have trouble */
            if (!ltemp)
            {
            /* We got a contiguous region */
                n_alloced       = n_clusters;
                largest_chain   = 0;
                /* That starts here */
                if (method == PC_FIXED_FIT)
                    first_cluster = (CLUSTERTYPE) start_cluster;
                else
                    first_cluster = (CLUSTERTYPE) last_cluster_in_chain + 1;
            }
        }
        else
        {
            /* Now allocate clusters. To find the free space we look in three
            regions until we find space:
           1 we look from the last cluster in the file to the end of the fat
            (skip 1 if there is no chain)
           2 we look from the beginning of the data area to the end of the fat
           3 we look from the beginning of the fat area to the end of the fat
            */

            clno = last_cluster_in_chain;
            if (!clno)
                clno = pdr->free_contig_base;
            while (clno)
            {
                n_alloced =  pc_find_contig_clusters(pdr, clno, &first_cluster, n_clusters, method);
                if (n_alloced == 0xfffffffful)
                {
                    /* Fat operations have set errno */
                    ret_val = FALSE;
                    goto alloc_done;
                }
                else if ((CLUSTERTYPE)n_alloced >= n_clusters)
                    break;                  /* We got our chain */
                else
                {
                    /* We did not get enough space. keep track of the biggest chain.
                        Do not need to store first_cluster since we will not alocate chains
                        smaller than what we need */
                    if (largest_chain < (CLUSTERTYPE)n_alloced)
                        largest_chain = (CLUSTERTYPE)n_alloced;
                }
                /* If we were searching between from the end of the  file and end of fat
                    look from the beginning of the file data area */
                if (clno == last_cluster_in_chain)
                {
                    if (last_cluster_in_chain == pdr->free_contig_base)
                        clno = 2;
                    else
                        clno = pdr->free_contig_base;
                }
                /* If we were searching between the beginning of the file data area
                    and end of fat  look from the fat */
                else if (clno == pdr->free_contig_base)
                {
                    if (pdr->free_contig_base == 2)
                        break;
                    else
                        clno = 2;
                }
                else  /* We have looked everywhere. No luck */
                    break;
            }
        }

        if ((CLUSTERTYPE)n_alloced < n_clusters)
        {
            /* We did not get what we asked for so we return the biggest free
                contiguous chain available in bytes plus whatever is left
                in the last cluster */
            *new_bytes = (largest_chain << (pdr->log2_secpalloc+9));
            if (alloced_size > pfile->pobj->finode->fsize) /* Avoid wrap */
                *new_bytes += (alloced_size - pfile->pobj->finode->fsize);
            goto alloc_done;
        }
        /* else   */

        /* We found a large enough contiguos group of clusters   */
        /* Turn them into a chain                                */
        clno = first_cluster;
        for (i = 0; i < (n_clusters-1); i++, clno++)
        {
            /* Link the current cluster to the next one   */
            /* If it fails FATOP will set errno */
            if (!FATOP(pdr)->fatop_pfaxx(pdr, clno, (CLUSTERTYPE) (clno+1) )) /* File */
                goto alloc_done;
            /* 2-19-99 - Bug fix. was not decrementing free cluster count */
            pdr->known_free_clusters = (long)(pdr->known_free_clusters - 1);
        }

        /* Terminate the list           */
        if (!FATOP(pdr)->fatop_pfaxxterm(pdr, clno)) /* File */
        {   /* If it fails FATOP will set errno */
            goto alloc_done;
        }
        else
        {
            /* 2-19-99 - Bug fix. was not decrementing free cluster count */
            pdr->known_free_clusters = (long)(pdr->known_free_clusters - 1);
        }
        if (last_cluster_in_chain)
        {
            /* The file already has clusters in it. Append our new chain   */
            if (!FATOP(pdr)->fatop_pfaxx(pdr, last_cluster_in_chain, first_cluster)) /* File */
            {   /* If it fails FATOP will set errno */
                goto alloc_done;
            }
        }
        else
        {
            /* Put our chain into the directory entry   */
            pc_pfinode_cluster(pfile->pobj->pdrive,pfile->pobj->finode,first_cluster);
            /* Use synch_pointers to set our file pointers up   */
            pfile->fptr_cluster = 0;        /* This is already true but... */
            pfile->fptr_block = 0;
            pfile->fptr = 0;
        }
        /* Flush the fat   */
        if (!FATOP(pdr)->fatop_flushfat(pdr->driveno))
        {   /* If it fails FATOP will set errno */
            goto alloc_done;
        }
    }

    /* If we get here it works and we should update the file size */
    *new_bytes = n_bytes;
    ret_val = TRUE;

    pfile->pobj->finode->fsize = new_file_size;
    /* Write the directory entry. Set archive & date   */
    if (!pc_update_inode(pfile->pobj, TRUE, TRUE) )
    { /* Update inode set errno */
        ret_val = FALSE;
        goto alloc_done;
    }
    /* call synch to take care of both the eof condition and the case where
        we just alloced the beginning of the chain */
    if (!_synch_file_ptrs(pfile))
    { /* synch file pointer set errno */
        ret_val = FALSE;
        goto alloc_done;
    }
    /* All code exits through here. ret_val determines if the function was
        successful. If -1 it is an error. */
alloc_done:
    if (!release_drive_mount_write(pdr->driveno))/* Release lock, unmount if aborted */
        ret_val = FALSE;
return_error:
    return(ret_val);
}

/******************************************************************************
    PC_FIND_CONTIG_CLUSTERS  - Find at least MIN_CLUSTER clusters.


 Description
        Using the provided method, search the FAT from start_pt to the
        end for a free contiguous chain of at least MIN_CLUSTERS. If less
        than MIN_CLUSTERS are found the largest free chain in the region is
        returned.

        There are three possible methods:
            PC_FIRST_FIT  - The first free chain >= MIN_CLUSTERS is returned
            PC_BEST_FIT - The smallest chain    >= MIN_CLUSTERS is returned
            PC_WORST_FIT  - The largest chain   >= MIN_CLUSTERS is returned

        Choose the method that will work best for you.

        Note: PC_FIRST_FIT is significantly faster faster than the others

        NOTE: The chain is not created. The caller must convert the
        clusters to an allocated chain.

 Returns
    Returns the number of contiguous clusters found up to MIN_CLUSTERS.
    *pchain contains the cluster number at the beginning of the chain.
    On error return 0xffff
    Example:
        Get the largest free chain on the disk:
        large = pc_find_contig_clusters(pdr, 2, &chain, 0xffff, PC_FIRST_FIT);

*****************************************************************************/
/* Note: the FAT is locked before this call is made   */

dword pc_find_contig_clusters(DDRIVE *pdr, CLUSTERTYPE startpt, CLUSTERTYPE  *pchain, CLUSTERTYPE min_clusters, int method) /* __apifn__ */
{
CLUSTERTYPE i;
CLUSTERTYPE value;
CLUSTERTYPE best_chain;
CLUSTERTYPE best_size;
CLUSTERTYPE chain_start;
CLUSTERTYPE chain_size;
CLUSTERTYPE largest_size;
CLUSTERTYPE largest_chain;
CLUSTERTYPE endpt;

    best_chain = 0;
    best_size = 0;
    chain_start = 0;
    chain_size = 0;
    largest_size  = 0;
    largest_chain = 0;
    endpt = pdr->maxfindex;

    for (i = startpt; i <= endpt; i++)
    {
        if (!FATOP(pdr)->fatop_faxx(pdr, i, &value) ) /* File */
            return(0xfffffffful);         /* IO error .. oops */
        if (value == 0)
        {
            /* Cluster is free. Run some tests on it.   */
            if (chain_start)
            {
                /* We are in a contiguous region already. Bump the count   */
                chain_size++;
            }
            else
            {
                /* Just starting a contiguous region   */
                chain_size = 1;
                chain_start = i;
            }
            /* If using first fit see if we crossed the threshold   */
            if (method == PC_FIRST_FIT)
            {
                if (chain_size >= min_clusters)
                {
                    best_chain = chain_start;
                    best_size = chain_size;
                    break;
                }
            }
        }       /* if value == 0*/
        /* Did we just finish scanning a contiguous chain ??   */
        if (chain_size && ((value != 0) || (i == endpt)) )
        {
            /* Remember the largest chain   */
            if (chain_size > largest_size)
            {
                largest_size  = chain_size;
                largest_chain = chain_start;
            }
            if (method == PC_BEST_FIT)
            {
                if (chain_size == min_clusters)
                {
                    /* The chain is exactly the size we need take it.   */
                    best_chain = chain_start;
                    best_size = chain_size;
                    break;
                }
                if (chain_size > min_clusters)
                {
                    if (!best_chain || (chain_size < best_size))
                    {
                        /* Chain is closest to what we need so far note it.   */
                        best_size = chain_size;
                        best_chain = chain_start;
                    }
                }
            }   /* if BEST_FIT */
            else if (method == PC_WORST_FIT)
            {
                if (chain_size >= min_clusters)
                {
                    if (!best_chain || chain_size > best_size)
                    {
                        best_size = chain_size;
                        best_chain = chain_start;
                    }
                }
            }   /* if WORST_FIT */
/*
*           else if (method == PC_BEST_FIT)
*               ;
*/
            chain_size = 0;
            chain_start = 0;
        } /* if (chain_size && ((value != 0) || (i == endpt)) ) */
    }   /*  for (i = startpt; i <= endpt; i++) */

    /* If we have a best chain return it here. Else return the largest chain   */
    if (best_chain)
    {
        *pchain = best_chain;
        return((dword)best_size);
    }
    else
    {
        *pchain = largest_chain;
        return((dword)largest_size);
    }

}

/**************************************************************************
    PO_CHSIZE -  Truncate or extend an open file.

 Description
    Given a file handle and a new file size either extend the file or
    truncate it. If the current file pointer is still within the range
    of the file it is unmoved, otherwise it is moved to the end of file.

 Note:
    This is not an ATOMIC file system operation. It uses other API calls
    po_lseek, po_truncate and po_write to size, truncate and extend the
    file.

 Returns
    Returns 0 on suucess -1 on error

    errno is set with one of these values
     0               - No error
     PEBADF          - Invalid file descriptor
     PEACCES         - File is read only
     PEINVALIDPARMS  - Invalid or inconsistent arguments
     An ERTFS system error
*****************************************************************************/

int po_chsize(PCFD fd, dword offset)                               /*__apifn__*/
{
    dword orig_fp;
    dword eof_fp;
    dword n_to_extend;
    dword n_to_try;
    dword ltemp;
    int ret_val;

    rtfs_set_errno(0); /* po_chsize: clear error status. routines that we call will set errno on errors */

    if (!po_ulseek(fd, 0, &orig_fp, PSEEK_CUR))
        return(-1);
    if (!po_ulseek(fd, 0,&eof_fp, PSEEK_END))
        return(-1);

    /* If size is unchanged just return */
    if (eof_fp == offset)
    {
        ret_val = 0;
    }
    /* If offset < eof we truncate */
    else if (eof_fp > offset)
    {
        if (!po_truncate(fd, offset))
            return(-1);
        /* Restore the file pointer if we can */
        if (orig_fp < offset)
            po_ulseek(fd, orig_fp,&ltemp, PSEEK_SET);
        return(0);
    }
    else
    {
        if (offset > RTFS_MAX_FILE_SIZE)
        {
            rtfs_set_errno(PETOOLARGE); /* po_extend_file: new size too large */
            return(-1);
        }

        /* Have to extend the file */
        n_to_extend = offset - eof_fp;
        ret_val= 0;             /* Assume it worked at first */
        while (n_to_extend)
        {
            /* Try to extend the file. po_extend_file will either extend the
                file and return the value we sent in or it will not extend the
                file and return a hint of what should work */
            n_to_try = n_to_extend;
            while (n_to_try)
            {
                if (!po_extend_file(fd, n_to_try, &ltemp, 0, PC_FIRST_FIT))
                {
                    ret_val = -1;
                    n_to_extend = 0;    /* Force break from the outer loop */
                    break;
                }
                else
                {
                    if (ltemp == n_to_try)
                    {
                        n_to_extend -= ltemp;/* We extended by n_to_try */
                        n_to_try = 0;       /* Break from inner loop */
                    }
                    else
                        n_to_try = ltemp;   /* We could not extend but ltemp
                                               should to work so try again*/
                }
            }
        }
    }
    if (!ret_val)
    {
        /* restore seek pointer and return */
        if (!po_ulseek(fd, orig_fp, &ltemp, PSEEK_SET) || ltemp != orig_fp)
            return(-1);
    }
    return(ret_val);
}


/***************************************************************************
Name:

    pc_get_file_extents - Get the list of block segments that make up a file


Summary:

    #include <rtfs.h>

    int pc_get_file_extents(PCFD fd, int infolistsize,
                            FILESEGINFO *plist, BOOLEAN raw)

    Where FILESEGINFO is a structure defined as:

    typedef struct fileseginfo {
        long    block;          Block number of the current extent
        long    nblocks;        Number of blocks in the extent
        } FILESEGINFO;

    And infolistsize is the number of elements in the storage pointed to
    by plist.

    If raw is TRUE the blocks are reported as block offsets from the physical
    base of the drive. Otherwise the block offset origin is the begining of
    the partition. Set raw to TRUE if you will be using the resultant list
    to set up DMA transfers to or from the disk.

 Description:
    This routines traverse the cluster chain of the open file fd and logs
    into the list at plist the block location and length in blocks of each
    segment of the file.
    The block numbers and block length information can then be used to
    read and write the file directly using pc_raw_read() and pc_raw_write()
    or the information may be used to set up DMA transfers to or from the
    raw block locations.
    If the file contains more extents than will fit in plist as indicated
    by infolistsize then the list is not updated beyond infolistsize elements
    but the count is updated and returned so the list size may be adjusted
    and the routine may be called again.

 Returns
    Returns the number of extents in the file or -1 on error.

    errno is set to one of the following

    0               - No error
    PEBADF          - Invalid file descriptor
    An ERTFS system error
****************************************************************************/

int pc_get_file_extents(PCFD fd, int infolistsize, FILESEGINFO *plist, BOOLEAN raw) /* __apifn__ */
{
    int p_errno;
    int end_of_chain;
    int ret_val;
    int n_segments;
    CLUSTERTYPE  first_cluster;
    CLUSTERTYPE  next_cluster;
    CLUSTERTYPE  n_clusters_to_seek;
    CLUSTERTYPE  n_clusters;
    long         partition_start;
    PC_FILE *pfile;
    DDRIVE *pdrive;
    CHECK_MEM(int, -1) /* Make sure memory is initted */

    p_errno = 0;
    rtfs_set_errno(0); /* pc_get_file_extents: clear error status. */

    /* Get the file structure and semaphore lock the drive */
    pfile = pc_fd2file(fd, 0);
    if (!pfile)
    { /* pc_fd2file set errno */
        ret_val = -1;
        goto return_error;
    }
    pdrive = pfile->pobj->pdrive;

    partition_start = pdrive->partition_base;

    ret_val = 0;                    /* Assume Zero segments to start */
    if (pfile->pobj->finode->fsize)
    {
        n_segments = 0;
        /* How many clusters in the file */
        n_clusters_to_seek = (CLUSTERTYPE)
            ((pfile->pobj->finode->fsize+(pdrive->secpalloc<<9)-1) >> (pdrive->log2_secpalloc+9));
        first_cluster = pc_finode_cluster(pdrive,pfile->pobj->finode);

        /* Traverse the file one chain at a time. Log start point (in blocks)
           and the length in blocks of each segment */
        while (n_clusters_to_seek)
        {
            /* Get the chain length and the start of the next chain */
            end_of_chain = 0;
            n_clusters = FATOP(pdrive)->fatop_get_chain(pdrive, first_cluster,
                            &next_cluster, n_clusters_to_seek, &end_of_chain);
            if (!n_clusters)
            {
                if (!get_errno()) /* If errno set already use that */
                    p_errno = PEINVALIDCLUSTER;
                n_segments = -1;
                break;
            }

            n_segments += 1;
            if (n_segments <= infolistsize)
            {
                plist->block = pc_cl2sector(pdrive,first_cluster);
                if (raw)
                    plist->block += partition_start;
                plist->nblocks = n_clusters << pdrive->log2_secpalloc;
                plist++;
            }
            n_clusters_to_seek = (CLUSTERTYPE) (n_clusters_to_seek - n_clusters);
            /* Check for corrupted file
                If there are more to seek but the cluster pointer
                is at the end this means that the file size is
                longer than the actual chain length so set errno
                to PEINVALIDCLUSTER and force error return */
            if (n_clusters_to_seek && end_of_chain)
            {
                p_errno = PEINVALIDCLUSTER;
                n_segments = -1;
                break;
            }
            else
                first_cluster = next_cluster;
        }
        ret_val = n_segments;
    }
    release_drive_mount(pdrive->driveno);/* Release lock, unmount if aborted */
return_error:
    if (p_errno)
        rtfs_set_errno(p_errno);
    return(ret_val);
}

/****************************************************************************
Name:
    pc_raw_read - Read blocks directly from a disk

Summary:

    #include <rtfs.h>

    int pc_raw_read(int driveno,  byte *buf, long blockno, int nblocks, BOOLEAN raw_io)

Description:
    Attempt to read nblocks blocks starting at blockno. If raw_io is TRUE
    then blockno is the offset from the beginning of the disk itself. If
    raw_io is FALSE then blockno is the offset from the beginning of the
    partition.
    This routine may be used in conjunction with pc_get_file_extents()
    to find and read blocks without the additional overhead incurred
    when calling po_read().

    The maximum allowable value for nblocks is 128.

    Note: It is possible to read any range of blocks in the disk.

 Returns:

    Returns 0 if the read succeeded or -1 on error.

    errno is set to one of the following
    0                - No error
    PEINVALIDDRIVEID - Driveno is incorrect
    PEINVALIDPARMS   - Invalid or inconsistent arguments
    PEIOERRORREAD    - The read operation failed
    An ERTFS system error
*****************************************************************************/

int pc_raw_read(int driveno,  byte *buf, long blockno, int nblocks, BOOLEAN raw_io)  /*__apifn__*/
{
    CHECK_MEM(int, -1) /* Make sure memory is initted */

    rtfs_set_errno(0); /* pc_raw_read: clear error status. */

    /* return -1 if bad arguments   */
    if (!nblocks || !buf || (nblocks > 128))
    {
        rtfs_set_errno(PEINVALIDPARMS);
        return(-1);
    }
    if (!check_drive_number_mount(driveno)) /* Check set errno */
        return(-1);
    /* READ   */
    if (!devio_read(driveno, (dword) blockno, buf, (word)nblocks, raw_io))
    {
        rtfs_set_errno(PEIOERRORREAD);
        release_drive_mount(driveno);/* Release lock, unmount if aborted */
        return(-1);
    }
    release_drive_mount(driveno);/* Release lock, unmount if aborted */
    return(0);
}


/****************************************************************************
Name:
    pc_raw_write - Write blocks directly to a disk

Summary:

    #include <rtfs.h>

    int pc_raw_write(int driveno,  byte *buf, long blockno, int nblocks, BOOLEAN raw_io)

Description:
    Attempt to write nblocks blocks starting at blockno. If raw_io is TRUE
    then blockno is the offset from the beginning of the disk itself. If
    raw_io is FALSE then blockno is the offset from the beginning of the
    partition.
    This routine may be used in conjunction with pc_get_file_extents()
    to find and read blocks without the additional overhead incurred
    when calling po_read().

    The maximum allowable value for nblocks is 128.

    Note: It is possible to write any range of blocks in the disk.


 Returns
    Returns 0 if the write succeeded or -1 on error.

    errno is set to one of the following
    0                - No error
    PEINVALIDDRIVEID - Driveno is incorrect
    PEINVALIDPARMS   - Invalid or inconsistent arguments
    PEIOERRORWRITE   - The read operation failed
    An ERTFS system error
*****************************************************************************/

int pc_raw_write(int driveno,  byte *buf, long blockno, int nblocks, BOOLEAN raw_io)          /*__apifn__*/
{
    CHECK_MEM(int, -1) /* Make sure memory is initted */

    rtfs_set_errno(0); /* pc_raw_write: clear error status. */

    /* return -1 if bad arguments   */
    if (!nblocks || !buf || (nblocks > 128))
    {
        rtfs_set_errno(PEINVALIDPARMS);
        return(-1);
    }
    /* we will use release_drive_mount() not release_drive_mount_write()
       since we did not change file system structures */

    if (!check_drive_number_mount(driveno)) /* Check set errno */
        return(-1);
    if (!devio_write(driveno, (dword) blockno, buf, (word)nblocks, raw_io))
    {
        rtfs_set_errno(PEIOERRORWRITE);
        release_drive_mount(driveno);/* Release lock, unmount if aborted */
        return(-1);
    }
    release_drive_mount(driveno);/* Release lock, unmount if aborted */
    return(0);
}

/***************************************************************************
Name:

    pc_get_free_list - Get a list free cluster segments on the drive


Summary:

    #include <rtfs.h>

    int pc_get_free_list(byte *drivename, int listsize, FREELISTINFO *plist, long threshhold)

    Where

    drivename is a valid drive specifier for example C:. An empty string
    denotes the current workin drive.


    FREELISTINFO is a structure defined as:

    typedef struct freelistinfo {
        CLUSTERTYPE cluster;        Cluster where the free region starts
        long        nclusters;      Number of free clusters the free segment
        } FREELISTINFO;

    listsize is the number of elements in the storage pointed to
    by plist.


    threshhold is the smallest contiguous free region to report.
    This is provided to allow the caller to exclude free chains
    that are too snall to be interesting. Setting this to a higher
    value also reduces the number of entries in plist that will
    be used up. The value of threshold must be at least 1. If it
    is one then every free cluster segment is reported.

 Description:
    This routines traverses the file allocation table of the drive.
    It places in the results structure the starting point and size
    of each free segment

    The free list information may then be used by po_extend_file() to
    allocate specific clusters for specific files.
    If the FAT contains more free extents than will fit in plist as indicated
    by listsize then the list is not updated beyond listsize elements
    but the count is updated and returned so the list size may be adjusted
    and the routine may be called again.

 Returns
    Returns the number of free segments or -1 on error.

    errno is set to one of the following
    0                - No error
    PEINVALIDDRIVEID - Driveno is incorrect
    PEINVALIDPARMS   - Invalid or inconsistent arguments
    An ERTFS system error
****************************************************************************/


int pc_get_free_list(int driveno, int listsize, FREELISTINFO *plist, long threshhold) /* __apifn__ */
{
    CLUSTERTYPE  clno;
    CLUSTERTYPE  value;
    int          index;
    long         region_size;
    DDRIVE       *pdr;
    int ret_val;
    int p_errno;
    CHECK_MEM(PCFD, -1) /* Make sure memory is initted */

    ret_val = 0;
    p_errno = 0;
    index = 0;
    region_size = 0;

    rtfs_set_errno(0); /* pc_get_free_list: clear error status. */

    if (threshhold < 1)
    {
        rtfs_set_errno(PEINVALIDPARMS);
        return(-1);
    }

    /* Get the drive and make sure it is mounted   */
    if (!check_drive_number_mount(driveno)) /* Check set errno */
        return(-1);

    pdr = pc_drno2dr(driveno);
    for (clno = 2; clno <= pdr->maxfindex; clno++)
    {
        if (!FATOP(pdr)->fatop_faxx(pdr, clno, &value) )  /* Fat */
        {
            /* Fatop will set errno */
            region_size = 0;
            ret_val = -1;
            break;
        }
        if (value == 0)
        {
            /* The cluster is free */
            if (region_size)
            {
                /* We are inside a free extent so update counter */
                region_size++;
            }
            else
            {
                /* Start tracking a new one */
                region_size = 1;
                if (index < listsize)
                {
                    plist->cluster = clno;
                }
            }
        }
        else
        {
            /* Cluster is non-zero see if we need to terminate a region */
            if (region_size >= threshhold && index < listsize)
            {
                plist->nclusters = region_size;
                plist++;
            }
            if (region_size >= threshhold)
                index++;
            region_size = 0;
        }
    }
    if (!ret_val)
    {
        /* If we left the for loop while still in a region note it */
        if (region_size >= threshhold)
        {
            if (index < listsize)
            {
                plist->nclusters = region_size;
            }
            index++;
        }
        ret_val = index;
    }
    release_drive_mount(driveno);/* Release lock, unmount if aborted */
    if (p_errno)
        rtfs_set_errno(p_errno);
    return(ret_val);
}
