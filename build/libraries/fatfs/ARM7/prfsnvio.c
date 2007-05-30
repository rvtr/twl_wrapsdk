/*               EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* PRFSNVIO.C - ERTFS-PRO FailSafe journal file routines */
/* Contains source code to implement journal file accesses read, write,
   open and create.
    The following routines are exported:

    failsafe_reopen_nv_buffer()
    failsafe_create_nv_buffer()
    failsafe_write_nv_buffer()
    failsafe_read_nv_buffer()

    Note: These routines create and maintains a journal file on the
    disk. If you have a closed system and you prefer to use system non
    volatile ram instead please modify these for functions to use that
    resource instead of a disk based file.
*/

#include <rtfs.h>

#if (INCLUDE_FAILSAFE_CODE)

static BOOLEAN open_failsafe_file(FAILSAFECONTEXT *pfscntxt, BOOLEAN create);

/*
* failsafe_reopen_nv_buffer - Open the failsafe buffer.
*
* Summary:
*   BOOLEAN failsafe_reopen_nv_buffer(FAILSAFECONTEXT *pfscntxt)
*
* Description:
*
* This routine must check for the existence of a failsafe buffer on the
* current disk or in system non volatile ram and return TRUE if one exists,
* or FALSE if one does not.
* It may use the field nv_buffer_handle in the structure pointed to
* by pfscntxt to store a handle for later access by failsafe_read_nv_buffer()
* and failsafe_write_nv_buffer().
*
*/

BOOLEAN failsafe_reopen_nv_buffer(FAILSAFECONTEXT *pfscntxt)
{
    return(open_failsafe_file(pfscntxt, FALSE));
}

/*
* failsafe_create_nv_buffer - Create the failsafe buffer.
*
* Summary:
*   BOOLEAN failsafe_create_nv_buffer(FAILSAFECONTEXT *pfscntxt)
*
* Description:
*
* This routine must create a failsafe buffer on the current disk or
* in system NV ram and return TRUE if successful, FALSE if it is
* unsuccessful.
* It may use the field nv_buffer_handle in the sructure pointed to
* by pfscntxt to store a handle for later access by failsafe_read_nv_buffer()
* and failsafe_write_nv_buffer().
*
* The failsafe buffer must contain space for:
*   (pfscntxt->num_remap_blocks + pfscntxt->num_index_blocks)
*   512 byte blocks.
*
* The source code in prfailsf.c implements the failsafe buffer in a hidden
* file named \FAILSAFE on the disk.
* The reference implementation is convenient and should be adequate for
* most applications. If it is more desirable to implement the failsafe
* buffer in flash or NVRAM then these functions should be modified to access
* that media instead.
*
*/

BOOLEAN failsafe_create_nv_buffer(FAILSAFECONTEXT *pfscntxt)
{
     /* Set the journal file size to zero to force the open routine to
       open the file. If the first open succeeds and the size is as big
       as pfscntxt->correct_journal_size then we won't have to reopen it
       in subsequant calls to restore and create */
    pfscntxt->journal_file_size = 0;
    return(open_failsafe_file(pfscntxt, TRUE));
}

/*
* failsafe_read_nv_buffer - Write a block to the failsafe buffer.
*
* Summary:
*   BOOLEAN failsafe_write_nv_buffer(
*           FAILSAFECONTEXT *pfscntxt,
*           dword block_no,
*           byte *pblock)
*
* Description:
*
* This routine must write one block to the block at offset block_no
* in the failsafe buffer on the current disk or in system NV ram and
* return TRUE if successful, FALSE if it is unsuccessful.
*
*/

BOOLEAN  failsafe_write_nv_buffer(FAILSAFECONTEXT *pfscntxt, dword block_no, byte *pblock)
{
    if (!pfscntxt->nv_buffer_handle || (pfscntxt->journal_file_size <= block_no))
    {
        rtfs_set_errno(PEIOERRORWRITEJOURNAL);
        return(FALSE);
    }
    return (devio_write(pfscntxt->pdrive->driveno,
            pfscntxt->nv_buffer_handle+block_no, pblock,
            (word)1, TRUE));
}

/*
* failsafe_read_nv_buffer - Read a block from the failsafe buffer.
*
* Summary:
*   BOOLEAN failsafe_read_nv_buffer(
*           FAILSAFECONTEXT *pfscntxt,
*           dword block_no,
*           byte *pblocks)
*
* Description:
*
* This routine must read one block from the block at offset block_no
* in the failsafe buffer on the current disk or in system NV ram and
* return TRUE if successful, FALSE if it is unsuccessful.
*
*/

BOOLEAN failsafe_read_nv_buffer(FAILSAFECONTEXT *pfscntxt, dword block_no, byte *pblock)
{
    if (!pfscntxt->nv_buffer_handle || (pfscntxt->journal_file_size <= block_no))
    {
        rtfs_set_errno(PEIOERRORREADJOURNAL);
        return(FALSE);
    }
    return(devio_read(pfscntxt->pdrive->driveno,
            pfscntxt->nv_buffer_handle+block_no, pblock,
            (word)1, TRUE));
}

/*
* open_failsafe_file - low level open/create of failsafe journal file
*
* Summary:
*  BOOLEAN open_failsafe_file(FAILSAFECONTEXT *pfscntxt, BOOLEAN create)
*
* Description:
*
* This opens the failsafe file and check if it contains enough contiguous
* blocks to hold a worst case journal file. If create is not requested and
* it can't find the file or the file is too small then it fails.
* If create is requested then it creates and extends the file as necessary.
*
* This routine is called when from ERTFS with the drive already locked, it
* uses low level access routines to create or open a contiguous file
*
* returns TRUE if successful, FALSE if it is unsuccessful.
*
*/

static BOOLEAN validate_failsafe_blocks(FAILSAFECONTEXT *pfscntxt, BOOLEAN fix_errors);

static BOOLEAN open_failsafe_file(FAILSAFECONTEXT *pfscntxt, BOOLEAN create)
{
    DROBJ *proot, *pobj;
    byte  path[32];     /* leave room for "FAILSAFE" 32 is plenty */
    byte  filename[32];
    byte  fileext[4];
    dword first_cluster, contig_clusters, n_clusters,next_cluster,required_journal_file_size,first_hit;
    int end_of_chain;
    DDRIVE *pdrive;
    FAILSAFECONTEXT *saved_pfscntxt;
    BOOLEAN doupdate = FALSE;
    BOOLEAN doflush = FALSE;


    pfscntxt->nv_buffer_handle = 0;
    pdrive = pfscntxt->pdrive;
    if (!pdrive)
        return(FALSE);
    saved_pfscntxt = (FAILSAFECONTEXT *) pdrive->pfscntxt;
    pdrive->pfscntxt = (void *) 0;

    proot = pc_get_root(pdrive);
    if (!proot ||
        !pc_parsepath((byte *)&path[0],(byte *)&filename[0],fileext,
            rtfs_strtab_user_string(USTRING_SYS_FSFILENAME)))
    {
return_error:
        pdrive->pfscntxt = (void *) saved_pfscntxt;
        return(FALSE);
    }
    /* See if it exists */
    pobj = pc_get_inode( 0, proot, filename, fileext, GET_INODE_MATCH);
    if (!pobj)
    {
        if (!create)
            goto return_error;
        else
        {
            /* Create a file entry in the root, hidden, readonly */
            pobj = pc_mknode( proot, (byte *)filename, fileext, ARDONLY|AHIDDEN, 0);
            if (!pobj)
            {

                goto return_error;
            }
            doupdate = TRUE;
        }
    }
    first_cluster = pc_finode_cluster(pobj->pdrive,pobj->finode);
    if (first_cluster)
        n_clusters = FATOP(pdrive)->fatop_get_chain(pdrive, first_cluster,&next_cluster, 0xffffffff, &end_of_chain);
    else
        n_clusters = 0;

    pfscntxt->journal_file_size = n_clusters * pdrive->secpalloc;
    if (create)
    { /* Creating or extend or truncate to the correct size */
       n_clusters = (CLUSTERTYPE)
               ((pfscntxt->num_remap_blocks +
                 pfscntxt->num_index_blocks +
                 pdrive->secpalloc-1) >> pdrive->log2_secpalloc);
        required_journal_file_size = n_clusters * pdrive->secpalloc;
        if (pfscntxt->journal_file_size != required_journal_file_size)
        {
            doupdate = TRUE;
            doflush = TRUE;
            /* Truncate if necessary */
            if (first_cluster)
            {
                FATOP(pdrive)->fatop_freechain(pdrive, first_cluster, 0, 0xffffffff);
                pc_pfinode_cluster(pdrive,pobj->finode,0);
                pobj->finode->fsize = 0;
            }
            /* Allocate */
            first_cluster = 0;
#if (defined(AF_CONTIGUOUS_MODE_FORCE)) /* Should be true for version 5.0 and up */
            contig_clusters = FATOP(pdrive)->fatop_alloc_chain(pdrive, &first_cluster, n_clusters, AF_CONTIGUOUS_MODE_FORCE);
#else
            contig_clusters = 0;
            first_hit = 0;
            /* Bug fix Nan 9, 2006
               fatop_alloc_chain - allocates clusters starting from
               pdrive->free_contig_pointer. or from first_cluster, if
               first_cluster is not zero.
               The problem is that if first_cluster is not zero
               we link it to the newly allocated cluster chain, which is
               not what we want. So the code has been modified to advance
               pdrive->free_contig_pointer and always have first_cluster == 0 */
            pdrive->free_contig_pointer = pdrive->free_contig_base;

            while (contig_clusters < n_clusters)
            { /* alloc chain can't guarantee n_clusters contiguous so
                 loop and free until we get contig_clusters == n_clusters */
                first_cluster = 0; /*  must be zero or it will be linked. */
                contig_clusters = FATOP(pobj->pdrive)->fatop_alloc_chain(pobj->pdrive, &first_cluster, n_clusters, FALSE);
                if (!first_hit)
                    first_hit = first_cluster;
                else if (first_hit == first_cluster)
                {
                    FATOP(pdrive)->fatop_freechain(pdrive, first_cluster, 0, 0xffffffff);
                    break;
                }
                if (contig_clusters == 0)
                    break;
                if (contig_clusters < n_clusters)
                {
                    FATOP(pdrive)->fatop_freechain(pdrive, first_cluster, 0, 0xffffffff);
                    pdrive->free_contig_pointer = first_cluster+contig_clusters;
                }
            }
            pdrive->free_contig_pointer = pdrive->free_contig_base; /* restore pointer */
#endif
            if (contig_clusters < n_clusters)
            {
                pfscntxt->journal_file_size = 0;
                pobj->finode->fsize = 0;
                pc_pfinode_cluster(pdrive,pobj->finode,0);
                pc_rmnode(pobj);
                doupdate = FALSE;
            }
            else
            {
                pfscntxt->journal_file_size = contig_clusters * pdrive->secpalloc;
                pc_pfinode_cluster(pdrive,pobj->finode,first_cluster);
                pobj->finode->fsize = pfscntxt->journal_file_size * 512;
            }
        }
        if ((doflush &&  !FATOP(pdrive)->fatop_flushfat(pdrive->driveno)) ||
            (doupdate && !pc_update_inode(pobj, TRUE, TRUE)) )
        {
              pfscntxt->journal_file_size = 0;
        }
    }

    if (pfscntxt->journal_file_size == 0)
    {
        pfscntxt->nv_buffer_handle = 0;
        pc_freeobj(pobj);
        pc_free_all_blk(pdrive);
        pc_free_all_fat_blocks(&pdrive->fatcontext);
        goto return_error;
    }
    else
    {
        pfscntxt->nv_buffer_handle =
            pobj->pdrive->partition_base +
                pc_cl2sector(pobj->pdrive, first_cluster);
        pc_freeobj(pobj);
        /* Clear buffer pool and fat buffer pool */
        pc_free_all_blk(pdrive);
        pc_free_all_fat_blocks(&pdrive->fatcontext);
#if (CFG_VALIDATE_JOURNAL)
        if (!validate_failsafe_blocks(pfscntxt, TRUE))
        {
            pfscntxt->journal_file_size = 0;
            pfscntxt->nv_buffer_handle = 0;
            /* Tell the upper levels that an IO error occured.
               This is unrecoverable */
            pfscntxt->open_status = FS_STATUS_IO_ERROR;
            pdrive->pfscntxt = (void *) saved_pfscntxt;
            return(FALSE);
        }
#endif
        pdrive->pfscntxt = (void *) saved_pfscntxt;
        return(TRUE);
    }
}

#if (CFG_VALIDATE_JOURNAL)

static byte static_validate_buffer[CFG_VALIDATE_BUFFER_SIZE*512];
byte *get_validate_buffer(word *pblocks_per_buffer)
{
    *pblocks_per_buffer = CFG_VALIDATE_BUFFER_SIZE;
    return(&static_validate_buffer[0]);
}

static BOOLEAN validate_failsafe_blocks(FAILSAFECONTEXT *pfscntxt, BOOLEAN fix_errors)
{
dword block_no,n_blocks_left;
word  blocks_per_buffer, blocks_this_access;
byte *pbuffer;
int driveno;

    driveno = pfscntxt->pdrive->driveno;
    pbuffer = get_validate_buffer(&blocks_per_buffer);
    if (blocks_per_buffer > 128)
        blocks_per_buffer = 128;
    /* Read the journal file */
    block_no = pfscntxt->nv_buffer_handle;
    n_blocks_left = pfscntxt->journal_file_size;
    while (n_blocks_left)
    {
        if (n_blocks_left > (dword) blocks_per_buffer)
            blocks_this_access = blocks_per_buffer;
        else
            blocks_this_access = (word) n_blocks_left;
        if (!devio_read(driveno, block_no, pbuffer,blocks_this_access,TRUE))
            break;
        block_no += blocks_this_access;
        n_blocks_left -= blocks_this_access;
    }
    if (n_blocks_left == 0)
        return(TRUE); /* Read all blocks without failure */
    if (!fix_errors)
        return(FALSE);
    /* We got here because a read error occured
       Write all zeroes to the file to see if we can  correct it. */
    /* Write the journal file */
    block_no = pfscntxt->nv_buffer_handle;
    n_blocks_left = pfscntxt->journal_file_size;
    while (n_blocks_left)
    {
        if (n_blocks_left > (dword) blocks_per_buffer)
            blocks_this_access = blocks_per_buffer;
        else
            blocks_this_access = (word) n_blocks_left;
        /* Zero the buffer every time we use it. This takes a little
           extra time but in the unlikely event other threads are using it
           for reads we will zero it before we write each time */
        rtfs_memset((void *)pbuffer, 0, (int)(blocks_this_access*512));
        if (!devio_write(driveno, block_no, pbuffer,blocks_this_access,TRUE))
            break;
        block_no += blocks_this_access;
        n_blocks_left -= blocks_this_access;
    }
    if (n_blocks_left != 0)
        return(FALSE);          /* Fail because We could not write the file.. */
    else
    {
                           /* We overwrote, now revalidate */
        return(validate_failsafe_blocks(pfscntxt, FALSE));
    }
}
#endif /* (CFG_VALIDATE_JOURNAL) */

#endif /* (INCLUDE_FAILSAFE_CODE) */
