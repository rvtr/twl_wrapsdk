/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* RTFATXX.C - Low level File allocation table management functions. 

    Routines in this file include:

    fatxx_alloc_chain      -   Allocate a chain from the FAT.
    fatxx_find_free_cluster-   Find the first free cluster in a given range.
    fatxx_clalloc          -   Allocate a single cluster in the fragmented region.
    fatxx_clgrow           -   Grow a directory chain in the fragmented region.
    fatxx_clnext           -   Get the next cluster in a chain.
    fatxx_clrelease        -   Return a cluster to the free list.
    fatxx_faxx             -   Get a value from the FAT.
    fatxx_flushfat         -   Make sure the FAT is up to date on disk.
    fatxx_freechain        -   Release a chain to the free list.
    fatxx_cl_truncate_dir      -   Truncate a cluster chain.
    fatxx_get_chain        -   Return contiguous clusters in a chain.
    fatxx_pfaxx            -   Put a value to the FAT.
    fatxx_pfswap           -   Swap a block of the FAT into the cache.
    fatxx_fword            -   Get or put a value from the swap cache.
*/      



#include <rtfs.h>

CLUSTERTYPE fatxx_alloc_chain(DDRIVE *pdr, CLUSTERTYPE *pstart_cluster, CLUSTERTYPE n_clusters, BOOLEAN dolink);
CLUSTERTYPE fatxx_find_free_cluster(DDRIVE *pdr, CLUSTERTYPE startpt, CLUSTERTYPE endpt, int *is_error);
CLUSTERTYPE fatxx_clnext(DDRIVE *pdr, CLUSTERTYPE  clno);
BOOLEAN fatxx_clrelease_dir(DDRIVE    *pdr, CLUSTERTYPE  clno);
BOOLEAN fatxx_faxx(DDRIVE *pdr, CLUSTERTYPE clno, CLUSTERTYPE *pvalue);
BOOLEAN fatxx_flushfat(int driveno);
BOOLEAN fatxx_freechain(DDRIVE *pdr, CLUSTERTYPE cluster, dword min_clusters_to_free, dword max_clusters_to_free);
CLUSTERTYPE  fatxx_cl_truncate_dir(DDRIVE *pdr, CLUSTERTYPE cluster, CLUSTERTYPE l_cluster);
CLUSTERTYPE fatxx_get_chain(DDRIVE *pdr, CLUSTERTYPE start_cluster, CLUSTERTYPE *pnext_cluster, CLUSTERTYPE n_clusters, int *end_of_chain);
BOOLEAN fatxx_pfaxxterm(DDRIVE   *pdr, CLUSTERTYPE  clno);
BOOLEAN fatxx_pfaxx(DDRIVE *pdr, CLUSTERTYPE  clno, CLUSTERTYPE  value);
BOOLEAN fatxx_fword(DDRIVE *pdr, CLUSTERTYPE index, word *pvalue, BOOLEAN putting);
BOOLEAN fatxx_pfpdword(DDRIVE *pdr, dword index, dword *pvalue);
BOOLEAN fatxx_pfgdword(DDRIVE *pdr, dword index, dword *value);

/******************************************************************************
    PC_ALLOC_CHAIN  -  Allocate as many contiguous clusters as possible.

 Description
        Reserve up to n_clusters contiguous clusters from the FAT and
        return the number of contiguous clusters reserved.
        If pstart_cluster points to a valid cluster and dolink is true
        then link the new chain to it.
    
 Returns
    Returns the number of contiguous clusters found. Or zero on an error.
    pstart_cluster contains the address of the start of the chain on
    return.

*****************************************************************************/
#if (RTFS_WRITE)

CLUSTERTYPE fatxx_alloc_chain(DDRIVE *pdr, CLUSTERTYPE *pstart_cluster, CLUSTERTYPE n_clusters, BOOLEAN dolink) /*__fatfn__*/
{
    CLUSTERTYPE start_cluster;
    CLUSTERTYPE first_new_cluster;
    CLUSTERTYPE clno;
    CLUSTERTYPE n_contig;
    CLUSTERTYPE value;
    CLUSTERTYPE last_cluster;   /* NEW for 5/18/97 bug fix */
    int is_error;

    is_error = 0;
    start_cluster = *pstart_cluster;
    if (start_cluster && 
        ( (start_cluster < 2) || (start_cluster > pdr->maxfindex) ) )
    {
        rtfs_set_errno(PEINVALIDCLUSTER);   /* fatxx_alloc_chain: bad cluster value internal error */
        return (0);
    }

    /* If the user provided a cluster we find the next cluster beyond that
        one. Otherwise we look at the disk structure and find the next 
        free cluster in the free cluster region after the current best guess
        of the region. If that fails we look to the beginning of the region
        and if that fails we look in the non-contiguous region. */

/* Begin changes for the 5/18/97 bug fix   */
    clno = 0;
/* NEW   */
    if (start_cluster)
    {
        /* search from the start_cluster hint to the end of the fat   */
        /* use: pdr->maxfindex+1 because find_free_cluster uses < end not <= */
        clno = fatxx_find_free_cluster(pdr, start_cluster, pdr->maxfindex+1, &is_error);
        if (is_error)   /* Error reading fat */
            return(0);
        /* If we search again search only to the start_cluster   */
        last_cluster = start_cluster;
    }
    else
        /* When we search again search to the end   */
        /* use: pdr->maxfindex+1 because find_free_cluster uses < end not <= */
        last_cluster = pdr->maxfindex + 1;
    /* Check the most likely place to find contiguous space   */
    if (!clno)
    {
/* NEW   */
        if (!start_cluster || start_cluster >= pdr->free_contig_pointer) 
        {
        /* search from free_contig_pointer to start_cluster or maxfindex whichever 
           is less */
            clno = fatxx_find_free_cluster(pdr, pdr->free_contig_pointer, last_cluster, &is_error);
            if (is_error)   /* Error reading fat */
                return(0);
        /* If we search again search only to the free_contig_pointer   */
            last_cluster = pdr->free_contig_pointer;
        }
    }
    /* Check the area of the disk beyond where we typically write fragments   */
    if (!clno)
    {
    /* NEW   */
        if (!start_cluster || start_cluster > pdr->free_contig_base)
        /* search from free_contig_base to start_cluster or free_contig_pointer whichever 
           is less */
            clno = fatxx_find_free_cluster(pdr, pdr->free_contig_base, last_cluster, &is_error);
            if (is_error)   /* Error reading fat */
                return(0);
    }
    /* Check the beginning of the  the disk where we typically write fragments   */
    if (!clno)
    {
        clno = fatxx_find_free_cluster(pdr, 2, pdr->free_contig_base, &is_error);
        if (is_error)   /* Error reading fat */
            return(0);
    }
    /* We did not find any clusters. Scan the whole fat again this should
       never work but we did have a bug in this area once before ... */
     if (!clno)
     {
        clno = fatxx_find_free_cluster(pdr, 2, pdr->maxfindex, &is_error);
        if (is_error)   /* Error reading fat */
            return(0);
     }
    if (!clno)
    {
        rtfs_set_errno(PENOSPC);    /* fatxx_alloc_chain: No free clusters */
        return(0);
    }
/* End changes for the 5/18/97 bug fix   */

    first_new_cluster = clno;
    value = 0;
    n_contig = 1;

    /* look up the FAT. If the next cluster is free we link to it
        and up the contig count. */
    while ( (n_contig < n_clusters) && (clno < pdr->maxfindex) )
    {
        if (!fatxx_faxx(pdr,(CLUSTERTYPE)(clno+1), &value))
            return(0);

        /* If the next cluster is in-use we are done.   */
        if (value)
            break;

        /* Link the current cluster to the next one   */
        if (!fatxx_pfaxx(pdr, clno, (CLUSTERTYPE)(clno+1)))
            return (0);
        n_contig += (CLUSTERTYPE)1; /* Yep.. we got another */
        clno += (CLUSTERTYPE)1;     /* Up the FAT table */
    }
    /* Terminate the list we just made          */
    if (!fatxx_pfaxxterm(pdr, clno))
        return (0);

    /* Update the hint of most likeley place to find a free cluster   */
    if ((clno < pdr->maxfindex) && (clno >= pdr->free_contig_pointer))
        pdr->free_contig_pointer = (CLUSTERTYPE)(clno+1);

    /* If we were handed a starting cluster we have to stitch our new
        chain after it. */
    if (dolink && start_cluster)
    {
        if (!fatxx_pfaxx(pdr, start_cluster, first_new_cluster))
            return (0);
    }

    *pstart_cluster = first_new_cluster;

    pdr->known_free_clusters = (long)(pdr->known_free_clusters - n_contig);

    return(n_contig);
}

/* Find the first free cluster in a range                        */
/* Note: The caller locks the fat before calling this routine    */
CLUSTERTYPE fatxx_find_free_cluster(DDRIVE *pdr, CLUSTERTYPE startpt, CLUSTERTYPE endpt, int *is_error) /*__fatfn__*/
{
CLUSTERTYPE i;
CLUSTERTYPE value;

    *is_error = 0;
    for (i = startpt; i < endpt; i++)
    {
        if ( !fatxx_faxx(pdr, i, &value) ) 
        {
            *is_error = 1;
            return(0);
        }
        if (value == 0)
        {
            return(i);
        }
    }
    return(0);
}

#if (RTFS_SUBDIRS)
/***************************************************************************
    PC_CLALLOC - Reserve and return the next free cluster on a drive

 Description
    Given a DDRIVE, mark the next available cluster in the file allocation 
    table as used and return the associated cluster number. Clhint provides
    a means of selecting clusters that are near eachother. This should 
    reduce fragmentation.

    NOTE: This routine is used to allocate single cluster chunks for
            maintaining directories. We artificially break the disks into
            two regions. The first region is where single clusters chunks
            used in directory files come from. These are allocated by this
            routine only. Data file clusters are allocated by fatxx_alloc_chain.
            
            THE DISK IS NOT REALLY PARTITIONED. If this routine runs out of
            space in the first region it grabs a cluster from the second 
            region.
 Returns
    Return a new cluster number or 0 if the disk is full.

****************************************************************************/
/* Note: The caller locks the fat before calling this routine   */
CLUSTERTYPE fatxx_clalloc(DDRIVE *pdr, CLUSTERTYPE clhint)                     /*__fatfn__*/
{
    CLUSTERTYPE clno;
    int is_error;

    if (clhint < 2)
        clhint = 2;
    if (clhint >= pdr->free_contig_base)
        clhint = 2;

    /* Look in the fragmentable region first from clhint up   */
    clno = fatxx_find_free_cluster(pdr, clhint, pdr->free_contig_base, &is_error);
    if (is_error)   /* Error reading fat */
        return(0);

    /* Look in the  fragmentable region up to clhint   */
    if (!clno)
    {
        clno = fatxx_find_free_cluster(pdr, 2, clhint, &is_error);
        if (is_error)   /* Error reading fat */
            return(0);
    }

     /* Look in the contiguos region if the fragmentable region is full   */
    if (!clno)
    {
        /* use: pdr->maxfindex+1 because find_free_cluster uses < end not <= */
        clno = fatxx_find_free_cluster(pdr, pdr->free_contig_base, pdr->maxfindex+1, &is_error);
        if (is_error)   /* Error reading fat */
            return(0);
    }

    if (!clno)
        rtfs_set_errno(PENOSPC);    /* fatxx_clalloc: No free clusters */
    else
    {
        /* Mark the cluster in use   */
        if (!fatxx_pfaxxterm(pdr, clno))
            return (0);
        pdr->known_free_clusters = pdr->known_free_clusters - 1;
    }
    return(clno);
}

/****************************************************************************
    PC_CLGROW - Extend a cluster chain and return the next free cluster

 Description
    Given a DDRIVE and a cluster, extend the chain containing the cluster
    by allocating a new cluster and linking clno to it. If clno is zero
    assume it is the start of a new file and allocate a new cluster.

    Note: The chain is traversed to the end before linking in the new 
            cluster. The new cluster terminates the chain.
 Returns
    Return a new cluster number or 0 if the disk is full.

****************************************************************************/

/* Note: The caller locks the fat before calling this routine   */
CLUSTERTYPE  fatxx_clgrow(DDRIVE *pdr, CLUSTERTYPE  clno)                      /*__fatfn__*/
{
    CLUSTERTYPE nxt;
    CLUSTERTYPE nextcluster;
    long range_check;

    /* Check the incoming argument. Should be a valid cluster */
    if ((clno < 2) || (clno > pdr->maxfindex) )
    {
        rtfs_set_errno(PEINVALIDCLUSTER);
        return (0);
    }
    /* Make sure we are at the end of chain   */
    range_check = 0;
    nextcluster = fatxx_clnext(pdr , clno);
    while (nextcluster != 0xffffffff && ++range_check < MAX_CLUSTERS_PER_DIR)
    {
        if (!nextcluster) /* fatxx_clnext - set errno */ 
            return (0);
        clno = nextcluster;
        nextcluster = fatxx_clnext(pdr , clno);
    }
    if (get_errno())
        return (0);
    if (range_check == MAX_CLUSTERS_PER_DIR)
    {
        rtfs_set_errno(PEINVALIDCLUSTER);
        return (0);
    }
    /* Get a cluster, clno provides a hint for more efficient cluster
        allocation */
    nxt = fatxx_clalloc(pdr,clno);
    if (!nxt)
        return((CLUSTERTYPE) 0);
    /* Attach it to the current cluster if not at the begining of the chain   */
    if (!fatxx_pfaxx(pdr, clno, nxt))
        return((CLUSTERTYPE) 0);
    return(nxt);
}

/****************************************************************************
    PC_CLRELEASE - Return a cluster to the pool of free space on a disk

 Description
    Given a DDRIVE and a cluster, mark the cluster in the file allocation
    table as free. It will be used again by calls to fatxx_clalloc().

 Returns
    Nothing



***************************************************************************/
 

/* Return a cluster to the free list                             */
/* Note: The caller locks the fat before calling this routine    */
BOOLEAN fatxx_clrelease_dir(DDRIVE    *pdr, CLUSTERTYPE  clno)                        /*__fatfn__*/
{
    int current_errno;
    /* This is a cleanup routine, an earlier event is the interesting errno 
       to the application, so we restore errno if we fail */ 
    current_errno = get_errno();
    /* No need to check clno value, pfaxx will catch it */
    /* Do not catch any lower level errors here. You will catch them soon enough   */
    if (fatxx_pfaxx(pdr, clno, (CLUSTERTYPE) 0x0))    /* Mark it as free */
    {
        /* If freeing in the contiguous region reset the hint if we
            free space earlier than it. */
        if (clno >= pdr->free_contig_base && clno <= pdr->free_contig_pointer)
            pdr->free_contig_pointer = clno;
        pdr->known_free_clusters = pdr->known_free_clusters + 1;
        return(TRUE);
    }
    else
    {
        rtfs_set_errno(current_errno);
        return(FALSE);
    }
}

/****************************************************************************
    PC_FLUSHFAT -  Write any dirty FAT blocks to disk

 Description
    Given a valid drive number. Write any fat blocks to disk that
    have been modified. Updates all copies of the fat.

 Returns
    Returns FALSE if driveno is not an open drive. Or a write failed.

****************************************************************************/

/* Consult the dirty fat block list and write any. write all copies
    of the fat */
/* Note: The caller locks the fat before calling this routine   */
BOOLEAN fatxx_flushfat(int driveno)                                        /*__fatfn__*/
{
    DDRIVE *pdr;

    pdr = pc_drno2dr(driveno);

    if (!pdr)
    {
        return(FALSE);
    }

    if (!pdr->fat_is_dirty)
        return(TRUE);

    /* write the alloc hints into the info block (no-op on non-FAT32 systems) */
    if (!fat_flushinfo(pdr))
        return(FALSE);

    if (pc_flush_fat_blocks(pdr))
    {
        pdr->fat_is_dirty = FALSE;
        return(TRUE);
    }
    else
        return(FALSE);
}

/****************************************************************************
    PC_FREECHAIN - Free a cluster chain associated with an inode.

 Description
    Trace the cluster chain starting at cluster and return all the clusters to
    the free state for re-use. The FAT is not flushed.

 Returns
    Nothing.

****************************************************************************/
/* Note: The caller locks the fat before calling this routine   */

BOOLEAN fatxx_freechain(DDRIVE *pdr, CLUSTERTYPE cluster, dword min_clusters_to_free, dword max_clusters_to_free)                     /*__fatfn__*/
{
    CLUSTERTYPE nextcluster;
    dword clusters_freed;

    if (max_clusters_to_free==0)
        return(TRUE);

    if ((cluster < 2) || (cluster > pdr->maxfindex) )
    {
        rtfs_set_errno(PEINVALIDCLUSTER);   /* fatxx_pfaxx: bad cluster value internal error */
        return (FALSE);
    }
    clusters_freed = 0;

    nextcluster = fatxx_clnext(pdr , cluster);
    while (cluster != 0xffffffff)
    {
        if (!nextcluster)
            return (FALSE); /* Clnext set errno */
        /* check if we are about to free more than we should */
        if (clusters_freed >= max_clusters_to_free)
        {
            rtfs_set_errno(PEINVALIDCLUSTER);
            return (FALSE);
        }
        if (fatxx_pfaxx(pdr, cluster, (CLUSTERTYPE) 0x0))    /* Mark it as free */
        {
            /* If freeing in the contiguous region reset the hint if we
                free space earlier than it. */
            if (cluster >= pdr->free_contig_base && cluster <= pdr->free_contig_pointer)
                pdr->free_contig_pointer = cluster;
            pdr->known_free_clusters = pdr->known_free_clusters + 1;
        }
        else
            return(FALSE); /* pfaxx set errno */
        clusters_freed += 1;
        cluster = nextcluster;
        if (nextcluster != 0xffffffff)
            nextcluster = fatxx_clnext(pdr , nextcluster);
    }
    if (clusters_freed < min_clusters_to_free)
    {
        rtfs_set_errno(PEINVALIDCLUSTER);
        return (FALSE);
    }
    return(TRUE);
}


/****************************************************************************
    PC_CL_TRUNCATE - Truncate a cluster chain.

 Description
    Trace the cluster chain starting at cluster until l_cluster is reached.
    Then terminate the chain and free from l_cluster on.
    The FAT is not flushed.

    If cluster == l_cluster does nothing. This condition should be handled
    higher up by updating dirents appropriately and then calling fatxx_freechain.

 Returns
    The last cluster in the chain after truncation.
****************************************************************************/
/* Note: The caller locks the fat before calling this routine   */
CLUSTERTYPE  fatxx_cl_truncate_dir(DDRIVE *pdr, CLUSTERTYPE cluster, CLUSTERTYPE l_cluster)/*__fatfn__*/
{
    CLUSTERTYPE nextcluster;
    long range_check;
    int current_errno;
    if ((cluster < 2) || (cluster > pdr->maxfindex) )
    { /* Don't set errno in this function, it is a cleanup */
        return (0);
    }
    /* This is a cleanup routine, an earlier event is the interesting errno 
       to the application, so we restore errno if we fail */ 
    current_errno = get_errno();

    nextcluster = fatxx_clnext(pdr , cluster);

    range_check = 0;
    while (nextcluster != 0xffffffff && ++range_check < MAX_CLUSTERS_PER_DIR)
    {
        if (!nextcluster) /* fatxx_clnext detected error */
        { /* Don't set errno in this function, it is a cleanup */
            rtfs_set_errno(current_errno);
            return (0);
        }
        if (nextcluster == l_cluster)
        {
            if (!fatxx_pfaxxterm(pdr, cluster))    /* Terminate the chain */
                break;                              /* Error break to ret 0 */
            /* We don't know the size so free between 0 and max */
            if (!fatxx_freechain(pdr, l_cluster, 0, MAX_CLUSTERS_PER_DIR))
            {
                rtfs_set_errno(current_errno);
                return(0);
            }
            return(cluster);
        }
        else
        {
            cluster = nextcluster;
            nextcluster = fatxx_clnext(pdr , nextcluster);
        }
    }
    rtfs_set_errno(current_errno);
    return(0);
}

#endif /* (RTFS_SUBDIRS) */

/******************************************************************************
    PC_PFAXXTERM - Write a terminating value to the FAT at clno.
  
 Description
    Given a DDRIVE,cluster number and value. Write the value 0xffffffff or 
    0xffff in the fat at clusterno. Handle 32, 16 and 12 bit fats correctly.

 Returns
    FALSE if an io error occurred during fat swapping, else TRUE

*****************************************************************************/
 
/* Given a clno & fatval Put the value in the table at the index (clno)    */
/* Note: The caller locks the fat before calling this routine              */
BOOLEAN fatxx_pfaxxterm(DDRIVE   *pdr, CLUSTERTYPE  clno)             /*__fatfn__*/
{
#if (FAT32)
            if (pdr->fasize == 8)
                return(fatxx_pfaxx(pdr, clno, 0x0ffffffful));
            else
#endif
                return(fatxx_pfaxx(pdr, clno, 0xffff));
            
}
/******************************************************************************
    PC_PFAXX - Write a value to the FAT at clno.
  
 Description
    Given a DDRIVE,cluster number and value. Write the value in the fat
    at clusterno. Handle 16 and 12 bit fats correctly.

 Returns
    No if an io error occurred during fat swapping, else TRUE.

*****************************************************************************/
 
/* Given a clno & fatval Put the value in the table at the index (clno)    */
/* Note: The caller locks the fat before calling this routine              */
BOOLEAN fatxx_pfaxx(DDRIVE *pdr, CLUSTERTYPE  clno, CLUSTERTYPE  value)            /*__fatfn__*/
{
    union align1 {
        byte    wrdbuf[4];          /* Temp storage area */
        word  fill[2];
    } u;
    CLUSTERTYPE nibble,index,offset, t;

    if ((clno < 2) || (clno > pdr->maxfindex) )
    {
        rtfs_set_errno(PEINVALIDCLUSTER);   /* fatxx_pfaxx: bad cluster value internal error */
        return (FALSE);
    }
    pdr->fat_is_dirty = TRUE;
    if (pdr->fasize == 3)       /* 3 nibble ? */
    {
        nibble = (CLUSTERTYPE)(clno * 3);
        index  = (CLUSTERTYPE)(nibble >> 2);
        offset = (CLUSTERTYPE)(clno & 0x03);
        /* Read the first word   */
        if (!fatxx_fword( pdr, index, (word *) &u.wrdbuf[0], FALSE ))
            return(FALSE);
/*
        |   W0      |   W1      |   W2  |
        A1 A0 B0 A2 B2 B1 C1 C0 D0 C2 D2 D1
        xx xx   xx
*/
        if (offset == 0) /* (A2 << 8) | A1 A2 */
        {
            /* Low nibble of b[1] is hi nibble of value   */
            u.wrdbuf[1] &= 0xf0;
            t = (CLUSTERTYPE)((value >> 8) & 0x0f);
            u.wrdbuf[1] |= (byte) t;
            /* b[0] is lo byte of value   */
            t = (CLUSTERTYPE)(value & 0x00ff);
            u.wrdbuf[0] = (byte) t;
            if (!fatxx_fword( pdr, index, (word *) &u.wrdbuf[0], TRUE ))
                return (FALSE);
        }
/*
        |   W0      |   W1      |   W2  |
        A1 A0 B0 A2 B2 B1 C1 C0 D0 C2 D2 D1
                xx  xx xx 
*/
    
        else if (offset == 1) /* (B1 B2 << 4) | B0 */
        {
            /* Hi nibble of b[1] is lo nibble of value   */
            u.wrdbuf[1] &= 0x0f;
            t = (CLUSTERTYPE)((value << 4) & 0x00f0);
            u.wrdbuf[1] |= (byte)t;
            if (!fatxx_fword( pdr, index, (word *) &u.wrdbuf[0], TRUE ))
                return (FALSE);
            /*  b[0] is hi byte of value   */
            if (!fatxx_fword( pdr, (word)(index+1), (word *) &u.wrdbuf[0], FALSE ))
                return(FALSE);
            t = (CLUSTERTYPE)((value >> 4) & 0x00ff);
            u.wrdbuf[0] = (byte) t;
            if (!fatxx_fword( pdr, (word)(index+1), (word *) &u.wrdbuf[0], TRUE ))
                return (FALSE);
        }
/*
        |   W0      |   W1  |   W2  |
        A1 A0 B0 A2 B2 B1 C1 C0 D0 C2 D2 D1
                            xx xx   xx 
*/
    
        else if (offset == 2) /*(C2 << 8) | C1 C2 */
        {
            /* b[1] = low byte of value   */
            t = (CLUSTERTYPE)(value & 0x00ff);
            u.wrdbuf[1] = (byte) t;
            if (!fatxx_fword( pdr, index, (word *) &u.wrdbuf[0], TRUE ))
                return (FALSE);
            /* lo nibble of b[0] == hi nibble of value   */
            if (!fatxx_fword( pdr, (word)(index+1), (word *) &u.wrdbuf[0], FALSE ))
                return(FALSE);
            u.wrdbuf[0] &= 0xf0;
            t = (CLUSTERTYPE)((value >> 8) & 0x0f);
            u.wrdbuf[0] |= (byte) t;
            if (!fatxx_fword( pdr, (word)(index+1), (word *) &u.wrdbuf[0], TRUE ))
                return (FALSE);
        }
/*
        |   W0      |   W1  |   W2  |
        A1 A0 B0 A2 B2 B1 C1 C0 D0 C2 D2 D1
                                xx  xx xx
*/
    
        else if (offset == 3) /* (D2 D1) << 4 | D0 */   
        {
            /* Hi nibble b[0] == low nible of value    */
            u.wrdbuf[0] &= 0x0f;
            t = (CLUSTERTYPE)((value << 4) & 0x00f0);
            u.wrdbuf[0] |= (byte) t;
            t = (CLUSTERTYPE)((value >> 4) & 0x00ff);
            u.wrdbuf[1] = (byte) t;
            if (!fatxx_fword( pdr, index, (word *) &u.wrdbuf[0], TRUE ))
                return (FALSE);
        }
    }
#if (FAT32) /* FAT32 only supported if FAT swapping is enabled */
    else if (pdr->fasize == 8)
    {
        fr_DWORD(&u.wrdbuf[0],value);          /*X*/
     /* Now put the values back into the FAT   */
        if (!fatxx_pfpdword( pdr, clno, (dword *) &u.wrdbuf[0] ))
        {
            return (FALSE);
        }
    }
#endif /* FAT32 */
    else        /* 16 BIT entries */
    {
        fr_WORD((byte *) &u.wrdbuf[0],(word)value);         /*X*/
        /* Now put the values back into the FAT   */
        if (!fatxx_fword( pdr, clno, (word *) &u.wrdbuf[0], TRUE ))
        {
            return (FALSE);
        }
    }
    return (TRUE);
}
#endif  /* (RTFS_WRITE)  */

/***************************************************************************
    PC_CLNEXT - Return the next cluster in a cluster chain
                    

 Description
    Given a DDRIVE and a cluster number, return the next cluster in the 
    chain containing clno. Return 0 on end of chain.

 Returns
    Return a new cluster number or 0xffffffff on end of chain.

****************************************************************************/

/* Return the next cluster in a chain or 0xffffffff  */
CLUSTERTYPE fatxx_clnext(DDRIVE *pdr, CLUSTERTYPE  clno)                        /*__fatfn__ */
{
    CLUSTERTYPE nxt;
#if (FAT32)
    dword _Oxffffffful;
#endif
    /* Get the value at clno. return 0 on any io errors   */
    if (! fatxx_faxx(pdr,clno,&nxt) )
        return (0);

    if (pdr->fasize == 3)       /* 3 nibble ? */
    {
        if ( (0xff7 < nxt) && (nxt <= 0xfff) )
            nxt = 0xffffffff;                            /* end of chain */
    }
    else
#if (FAT32)
    if (pdr->fasize == 8)
    {
        _Oxffffffful = 0x0ffffffful;
        nxt &= _Oxffffffful;
        if ( nxt == 0x0ffffffful )
            nxt = 0xffffffff;                            /* end of chain */
    }
    else
#endif
    {
#if (FAT32)
        if ( (nxt >= (CLUSTERTYPE)0xfff7) && (nxt <= (CLUSTERTYPE)0xffff) )
#else
    /* If fat32 is not defined the nxt is always <= 0xffff. picky compilers
       notice this and emit a warning */
        if (nxt >= (CLUSTERTYPE)0xfff7)
#endif
            nxt = 0xffffffff;                            /* end of chain */
    }
    if (nxt != 0xffffffff && (nxt < 2 || nxt > pdr->maxfindex) )
    {
        rtfs_set_errno(PEINVALIDCLUSTER);
        return (0);
    }
    return(nxt);
}

/***************************************************************************
    PC_FAXX - Get the value store in the FAT at clno.

 Description
    Given a DDRIVE and a cluster number. Get the value in the fat at clusterno
    (the next cluster in a chain.) Handle 16 and 12 bit fats correctly.

 Returns
    Returns the the value at clno. In pvalue. 
    If any error occured while FAT swapping return FALSE else return TRUE.
***************************************************************************/
 
/* Retrieve a value from the fat   */
BOOLEAN fatxx_faxx(DDRIVE *pdr, CLUSTERTYPE clno, CLUSTERTYPE *pvalue)             /*__fatfn__*/
{
    CLUSTERTYPE  nibble,index, offset, result;
    byte    c;
    union align1 {
        byte    wrdbuf[4];          /* Temp storage area */
        word  fill[2];
    } u;
    union align2 {
    byte    wrdbuf2[4];         /* Temp storage area */
    word  fill[2];
    } u2;
    if ((clno < 2) || (clno > pdr->maxfindex) )
    {
        rtfs_set_errno(PEINVALIDCLUSTER);   /* fatxx_faxx: bad cluster value internal error */
        return (FALSE);
    }
    result = 0;
    if (pdr->fasize == 3)       /* 3 nibble ? */
    {
        nibble = (word)(clno * 3);
        index  = (word)(nibble >> 2);
        offset = (word)(clno & 0x03);
        /* Read the first word   */
        if (!fatxx_fword( pdr, index, (word *) &u.wrdbuf[0], FALSE ))
            return(FALSE);
/*
        |   W0      |   W1      |   W2  |
        A1 A0 B0 A2 B2 B1 C1 C0 D0 C2 D2 D1
        xx xx   xx
*/
        if (offset == 0) /* (A2 << 8) | A1 A2 */
        {
            /* BYTE 0 == s Low byte, byte 1 low nibble == s high byte   */
            u.wrdbuf[1] &= 0x0f;
            result = u.wrdbuf[1];
            result <<= 8;
            result |= u.wrdbuf[0];
        }
/*
        |   W0      |   W1      |   W2  |
        A1 A0 B0 A2 B2 B1 C1 C0 D0 C2 D2 D1
                xx  xx xx 
*/
    
        else if (offset == 1) /* (B1 B2 << 4) | B0 */
        {
            /* BYTE 2 == High byte Byte 1 high nibb == low nib   */
            if (!fatxx_fword( pdr, (word)(index+1), (word *) &u2.wrdbuf2[0], FALSE ))
                return(FALSE);
            c = (byte) (u.wrdbuf[1] >> 4);
            result = u2.wrdbuf2[0];
            result <<= 4;
            result |= c;
        }
/*
        |   W0      |   W1      |   W2  |
        A1 A0 B0 A2 B2 B1 C1 C0 D0 C2 D2 D1
                            xx xx   xx 
*/
        else if (offset == 2) /*(C2 << 8) | C1 C2 */
        {
            if (!fatxx_fword( pdr, (word)(index+1), (word *) &u2.wrdbuf2[0], FALSE ))
                return(FALSE);
            /* BYTE 1 == s Low byte, byte 2 low nibble == s high byte   */
            result = (word) (u2.wrdbuf2[0] & 0x0f);
            result <<= 8;
            result |= u.wrdbuf[1];
        }
/*
        |   W0      |   W1      |   W2  |
        A1 A0 B0 A2 B2 B1 C1 C0 D0 C2 D2 D1
                                xx  xx xx
*/
    
        else if (offset == 3) /* (D2 D1) << 4 | D0 */   
        {
            result = u.wrdbuf[1];
            result <<= 4;
            c = u.wrdbuf[0];
            c >>= 4;
            result |= c;
        }
    }
    else if (pdr->fasize == 8) /* 32 BIT fat. ret the value at 4 * clno */
    {
#if (FAT32)
#if KS_LITTLE_ENDIAN
            if (!fatxx_pfgdword( pdr, clno, (dword *) &result ))
                return (FALSE);
#else
            if ( fatxx_pfgdword( pdr, clno, (dword *) &u.wrdbuf[0] ))
                result = (CLUSTERTYPE) to_DWORD(&u.wrdbuf[0]); 
            else
                return (FALSE);
#endif
#else
        return (FALSE);
#endif
    }
    else    /* 16 BIT fat. ret the value at 2 * clno */
    {
            if ( fatxx_fword( pdr, clno, (word *) &u.wrdbuf[0], FALSE ))
                result = (CLUSTERTYPE) to_WORD(&u.wrdbuf[0]); /*X*/ /* And use the product as index */
            else
                return (FALSE);
    }
    *pvalue = result;
    return (TRUE);
}

/******************************************************************************
    PC_GET_CHAIN  -  Return as many contiguous clusters as possible.


 Description
        Starting at start_cluster return the number of contiguous clusters
        allocated in the chain containing start_cluster or n_clusters,
        whichever is less.
    
 Returns
    Returns the number of contiguous clusters found. Or zero on an error.
    This function should always return at least one. (start_cluster). Unless
    an error occurs.

    The word at *pnext_cluster is filled with on of the following:
        . If we went beyond a contiguous section it contains
        the first cluster in the next segment of the chain.
        . If we are still in a section it contains
        the next cluster in the current segment of the chain.
        . If we are at the end of the chain it contains the last cluster 
        in the chain.

****************************************************************************/
/* Note: The caller locks the fat before calling this routine   */

CLUSTERTYPE fatxx_get_chain(DDRIVE *pdr, CLUSTERTYPE start_cluster, CLUSTERTYPE *pnext_cluster, CLUSTERTYPE n_clusters, int *end_of_chain)
{
    CLUSTERTYPE clno, next_cluster;
    CLUSTERTYPE n_contig;
    CLUSTERTYPE value;

    value = 0;
    clno = start_cluster;
    n_contig = 1;
    *pnext_cluster = 0;     
 
    /* Get each FAT entry. If its value points to the next contiguous entry
        continue. Otherwise we have reached the end of the contiguous chain.
        At which point we return the number of contig s found and by reference
        the address of the FAT entry beginning the next chain segment.
    */
    *end_of_chain = 0;
    for (;;)
    {       
        next_cluster = fatxx_clnext(pdr, clno);
        if (!next_cluster) /* clnext detected an error */
            return(0);
        /* check for end markers set next cluster to the last 
           cluster in the chain if we are at the end */
        if (next_cluster == 0xffffffff) /* clnext detected end */
        {
            *end_of_chain = 1;
            value = clno;
            break;
        }
        else if (next_cluster == ++clno)
        {
            value = next_cluster;
            if (n_contig >= n_clusters)
                break;
            n_contig++;
        }
        else /* (next_cluster != ++clno) and we know it is not an error condition */
        {
            value = next_cluster;
            break;
        }
    }
    *pnext_cluster = value;
    return (n_contig);
}

/***************************************************************************
    PC_FATSW - Map in a page of the FAT 
****************************************************************************/
    
/* Swap in the page containing index                           */
/* Note: The caller locks the fat before calling this routine  */
byte * fatxx_pfswap(DDRIVE *pdr, CLUSTERTYPE index, BOOLEAN for_write)         /*__fatfn__*/
{
    dword  block_offset_in_fat;
    dword flags;
    
    if (pdr->fasize == 8)
        block_offset_in_fat = (dword)(index >> 7);
    else
        block_offset_in_fat = (word) (index >> 8);

    if (block_offset_in_fat >= pdr->secpfat) /* Check range */
        return (0);
    if (for_write)
        flags = 0x80000000ul;
    else
        flags = 0x00000000ul;
    return (pc_map_fat_block(pdr,pdr->fatblock+block_offset_in_fat, flags));
}

/* Put or get a WORD value into the fat at index                */
BOOLEAN fatxx_fword(DDRIVE *pdr, CLUSTERTYPE index, word *pvalue, BOOLEAN putting)         /*__fatfn__*/
{
    word *ppage;
    word offset;    
    /* Make sure we have access to the page. Mark it for writing (if a put)   */
    ppage = (word *)fatxx_pfswap(pdr,index,putting);

    if (!ppage)
        return(FALSE);
    else
    {
        /* there are 256 entries per page   */
        offset = (word) (index & 0xff);
        if (putting)
            ppage[offset] = *pvalue;
        else
            *pvalue = ppage[offset];
    }
    return(TRUE);
}

FAT_DRIVER fatxx_d;

static BOOLEAN init_fat(DDRIVE *pdr)
{
    FAT_DRIVER *pfd;
/*    int driveno;
    driveno = pdr->driveno;*/
    pfd = &fatxx_d;
    pfd->fatop_alloc_chain = fatxx_alloc_chain;
    pfd->fatop_clnext = fatxx_clnext;
    pfd->fatop_clrelease_dir = fatxx_clrelease_dir;
    pfd->fatop_faxx = fatxx_faxx;
    pfd->fatop_flushfat = fatxx_flushfat;
    pfd->fatop_freechain = fatxx_freechain;
    pfd->fatop_cl_truncate_dir = fatxx_cl_truncate_dir;
    pfd->fatop_get_chain = fatxx_get_chain;
    pfd->fatop_pfaxxterm = fatxx_pfaxxterm;
    pfd->fatop_pfaxx = fatxx_pfaxx;
    pdr->fad = (void *) pfd;
    return(TRUE);
}

BOOLEAN faxx_check_free_space(DDRIVE  *pdr)
{
    CLUSTERTYPE i;
    CLUSTERTYPE nxt;
    long freecount = 0;

    for (i = 2 ; i <= pdr->maxfindex; i++)
    {
        if (!FATOP(pdr)->fatop_faxx(pdr, i, &nxt)) /* Any (ifree) */
            return(FALSE);
        if (nxt == 0)
            freecount++;
    }
    pdr->known_free_clusters = freecount;
    return(TRUE);
}

BOOLEAN init_fat32(DDRIVE *pdr)
{
    return(init_fat(pdr) && faxx_check_free_space(pdr));
}
BOOLEAN init_fat16(DDRIVE *pdr)
{
    return(init_fat(pdr) && faxx_check_free_space(pdr));
}
BOOLEAN init_fat12(DDRIVE *pdr)
{
    return(init_fat(pdr) && faxx_check_free_space(pdr));
}



