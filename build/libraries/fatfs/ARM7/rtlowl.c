/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* RTLOWL.C - Low level functions that don't directly manipulate the fat.

    Routines in this file include:

    pc_i_dskopen        -   Mount a drive if you can
    pc_read_partition_table
                        -  Load a drive structure with partition info
    pc_gblk0            -   Read block zero and set up internal structures.
    pc_clzero           -   Write zeroes to a cluster on disk.
    pc_drno2dr          -   Convert a drive number to a drive structure.
    pc_dskfree          -   Free resources associated with a drive.
    pc_sec2cluster      -   Convert a sector number to a cluster value.
    pc_sec2index        -   Convert a sector number to a cluster offset.
    pc_cl2sector        -   Convert a cluster value to a sector number.
    pc_pfinode_cluster  -   Assign a cluster value to a finode
    pc_finode_cluster   -   Get the cluster value from a finode
*/

#include <rtfs.h>

/******************************************************************************
    PC_I_DSKOPEN -  Open a disk for business.

 Description
    Called by lower level code in chkmedia to open the disk


 Returns
    Returns TRUE if the disk was successfully initialized.
****************************************************************************/

int pc_log_base_2(word n)                                   /*__fn__*/
{
int log;

    log = 0;
    if (n <= 1)
        return(log);

    while(n)
    {
        log += 1;
        n >>= 1;
    }
    return((int)(log-1));
}

/*
* Note: This routine is called with the drive already locked so
*   in several cases there is no need for critical section code handling
*   This is a helper function for pc_i_dskopen()
*/

BOOLEAN pc_init_drv_fat_info(DDRIVE *pdr, struct pcblk0 *pbl0);

BOOLEAN pc_i_dskopen(int driveno)                      /*__fn__*/
{
    DDRIVE *pdr;
    struct pcblk0 bl0;
    BOOLEAN ret_val;
    int partition_status;

    if (!prtfs_cfg)
    {
        /* Failed: pc_meminit() must not have been called   */
        pc_report_error(PCERR_INITCORE);
        return (FALSE);
    }

    /* Check drive number   */
    if (!pc_validate_driveno(driveno))
    {
        rtfs_set_errno(PEINVALIDDRIVEID);
        return(FALSE);
    }

    pdr = pc_drno_to_drive_struct(driveno);

    /* Do not do anything on reopens   */
    if (pdr->mount_valid)
    {
        return(TRUE);
    }
    else
    {
        /* Zero the structure so all of our initial values are right   */
        OS_CLAIM_FSCRITICAL()
        {
            byte *p1, *p2;
            p1 = (byte *) pdr;
            p2 = (byte *) &pdr->begin_user_area;
            while(p1 < p2) *p1++ = 0;
            /* Set driveno now because we overwrote it */
            pdr->driveno = (word)driveno;
        }
        OS_RELEASE_FSCRITICAL()
    }

    /* Set this to true now so check media does not try to mount */
    pdr->mount_valid = TRUE;
    partition_status = 0;
    if (pdr->drive_flags & DRIVE_FLAGS_PARTITIONED)
    {
        partition_status = pc_read_partition_table(driveno, pdr);
        if (partition_status != READ_PARTION_OK)
        {
            /* If we read a block but didn't see a partition table signature
               and this drive structure is for the first partition number
               then fall through and see if there is a BPB at block 0. */
            if (partition_status == READ_PARTION_NO_TABLE && pdr->partition_number == 0)
            {
                rtfs_set_errno(0);       /* Clear errno, we'll set it later */
                pdr->partition_base = 0; /* Is BPB at 0 ? */
            }
            else
            {
                pc_report_error(PCERR_INITDEV);
 return_error:
                pdr->mount_valid = FALSE;
                return(FALSE);
            }
        }
    }

    /* Read block 0   */
    if (!pc_gblk0((word) driveno, &bl0 ))
    {
        /* pc_gblk0 set errno */
        pc_report_error(PCERR_INITREAD);
        goto return_error;
    }

    /* Verify that we have a good dos formatted disk   */
    if ( (bl0.jump != (byte) 0xE9) && (bl0.jump !=(byte) 0xEB) )
    {
        /* If we were checking for an MBR at physical block 0 with a driver
           set for partitioned devices, set PEINVALIDMBR, otherwise
           set PEINVALIDBPB */
        if (pdr->drive_flags & DRIVE_FLAGS_PARTITIONED && partition_status == READ_PARTION_NO_TABLE)
            rtfs_set_errno(PEINVALIDMBR);
        else
            rtfs_set_errno(PEINVALIDBPB);  /* pc_i_dskopen Unkown values in Bios Parameter block */
        pc_report_error(PCERR_INITMEDI);
        goto return_error;
    }

    /* set up the drive structur from block 0   */
    pdr->bytspsector = bl0.bytspsector; /* bytes/sector */
    pdr->secpalloc = bl0.secpalloc; /* sectors / cluster */
    pdr->numroot = bl0.numroot; /* Maximum number of root entries */
    pdr->numsecs = (BLOCKT) bl0.numsecs;    /* Total sectors on the disk */
    pdr->mediadesc =    bl0.mediadesc;  /* Media descriptor byte */
    pdr->secreserved = bl0.secreserved; /* sectors reserved */
    pdr->secptrk    = bl0.secptrk;  /* sectors per track */
    pdr->numhead    = bl0.numhead;  /* number of heads */
    {

    dword ltemp;
        pdr->numhide    =bl0.numhide;   /* # hidden sectors */
        ltemp = bl0.numhide2;
        ltemp <<= 16;
        pdr->numhide    |= ltemp;
    }

    copybuff(pdr->volume_label, &bl0.vollabel[0], 11);
    pdr->volume_label[11] = 0;

    pdr->volume_serialno = bl0.volid;

/* Check if running on a DOS (4.0) huge partition                           */
    /* If traditional total # sectors is zero, use value in extended BPB    */
    if (pdr->numsecs == 0L)
        pdr->numsecs = bl0.numsecs2;                            /* (4.0) */

    /* derive some things   */
    pdr->bytespcluster = (int) (512 * pdr->secpalloc);
    /* bits to mask in to calculate byte offset in cluster from file pointer.
        AND file pointer with this to get byte offset in cluster a shift right
        9 to get block offset in cluster */
    pdr->byte_into_cl_mask = (dword) pdr->bytespcluster;
    pdr->byte_into_cl_mask -= 1L;
    /* save away log of sectors per alloc   */
    pdr->log2_secpalloc = (int)pc_log_base_2((word)pdr->secpalloc);

    /* Get
        pdr->secpfat        pdr->numfats        pdr->fatblock
        pdr->rootblock      pdr->secproot       pdr->firstclblock
        pdr->maxfindex      pdr->infosec        pdr->fasize
        pdr->free_contig_base
        pdr->free_contig_pointer
        pdr->known_free_clusters
    */
    pc_init_drv_fat_info(pdr, &bl0);

    /* Load the fat driver functions and initilialize the fat */
    /* Init_fat will set errno */
    if (pdr->fasize == 8)
        ret_val = init_fat32(pdr);
    else if (pdr->fasize == 4)
        ret_val = init_fat16(pdr);
    else if (pdr->fasize == 3)
        ret_val = init_fat12(pdr);
    else
    {
        rtfs_set_errno(PEINVALIDBPB);  /* pc_i_dskopen Unkown values in Bios Parameter block */
        ret_val = 0;
    }
    if (!ret_val)
    {
        pc_report_error(PCERR_FATREAD);
        goto return_error;
    }
    pdr->mount_valid = TRUE;
    /* Save Unique id for this mount */
    prtfs_cfg->drive_opencounter += 1;
    pdr->drive_opencounter = prtfs_cfg->drive_opencounter;
    return(TRUE);
}

/******************************************************************************
    pc_read_partition_table() -  Load a drive structure with partition info

 Description
    Read the partition table from a disk. If one is found then check the
    entry in the table that is specified by pdr->partition_number. If the
    entry is valid then load the fields
        pdr->partition_base,pdr->partition_size and pdr->partition_type;

 Returns
    The following values.

    READ_PARTION_OK         Partition read succesfully
    READ_PARTION_ERR        Internal error (could not allocate buffers ?)
    READ_PARTION_NO_TABLE   No partition table found
    READ_PARTION_NO_ENTRY   Request entry not found
    READ_PARTION_IOERROR    Device IO error

****************************************************************************/
int pc_read_partition_table(int driveno, DDRIVE *pdr)
{
    PTABLE *ppart;
    BLKBUFF *buf;
    byte *pbuf;
    word i;
    int ret_val;

    /* Grab some working space   */
    buf = pc_scratch_blk();
    if (!buf)
    {
        return(READ_PARTION_ERR);
    }

    /* Read block zero   */
    if (!devio_read(driveno, 0 , buf->data , 1, TRUE))
    {
        /* Failed reading Master boot record */
        rtfs_set_errno(PEIOERRORREADMBR);
        ret_val = READ_PARTION_IOERROR;
        goto done;
    }
    /* Copy the table to a word alligned buffer   */
    pbuf = buf->data;
    pbuf += 0x1be;          /* The info starts at buf[1be] */
    /* Don't use sizeof here since the structure does not pack to exact size */
    copybuff(buf->data, pbuf, 0x42);
    ppart = (PTABLE *) buf->data;

    if (to_WORD((byte *) &ppart->signature)  !=  0xAA55)  /*X*/
    {
        rtfs_set_errno(PEINVALIDMBR);
        ret_val = READ_PARTION_NO_TABLE;
        goto done;
    }
#if (SUPPORT_EXTENDED_PARTITIONS)
{
    /* Special code to support extended partitions */
    /* since most applications don't use these we compile it
       conditionally to save code space */
    dword extended_base, ltemp;
    int j, skip_count;
    for (j = 0; j < 4; j++)
    {
        if (ppart->ents[j].p_typ == 0x5 || ppart->ents[j].p_typ == 0xF)
        {
            if (j <= (int)pdr->partition_number)
            {
                /* the partition is inside an extended partition */
                /* Get the relative start and size   */
                pdr->partition_base = to_DWORD ((byte *) &ppart->ents[j].r_sec);
                extended_base = pdr->partition_base;
                pdr->partition_size = to_DWORD ((byte *) &ppart->ents[j].p_size);
                skip_count = (int)pdr->partition_number - j;
                for (;;)
                {
                    /* Read the partition information in the extended partition */
                    if (!devio_read(driveno, pdr->partition_base , buf->data , 1, TRUE))
                    {
                        /* Failed reading Master boot record */
                        rtfs_set_errno(PEIOERRORREADMBR);
                        ret_val = READ_PARTION_IOERROR;
                        goto done;
                    }
                    /* Copy the table to a word alligned buffer   */
                    pbuf = buf->data;
                    pbuf += 0x1be;          /* The info starts at buf[1be] */
                        /* Don't use sizeof here since the structure does not pack to exact size */
                    copybuff(buf->data, pbuf, 0x42);
                    ppart = (PTABLE *) buf->data;

                    if (to_WORD((byte *) &ppart->signature)  !=  0xAA55)  /*X*/
                    {
                        rtfs_set_errno(PEINVALIDMBR);
                        ret_val = READ_PARTION_NO_TABLE;
                        goto done;
                    }
                    if (skip_count==0)
                    {
                        if (pc_validate_partition_type(ppart->ents[0].p_typ))
                        {
                            pdr->partition_type = ppart->ents[0].p_typ;
                            ltemp = pdr->partition_base;
                            pdr->partition_base = ltemp + to_DWORD ((byte *) &ppart->ents[0].r_sec);
                            pdr->partition_size = to_DWORD ((byte *) &ppart->ents[0].p_size);
                            ret_val = READ_PARTION_OK;
                            goto done;
                        }
                        else
                        {
                            rtfs_set_errno(PEINVALIDMBROFFSET);
                            ret_val = READ_PARTION_NO_TABLE;
                            goto done;
                        }
                    }
                    else
                    {
                        if (ppart->ents[1].p_typ != 0x5 && ppart->ents[1].p_typ != 0xF)
                        {
                            rtfs_set_errno(PEINVALIDMBROFFSET);
                            ret_val = READ_PARTION_NO_TABLE;
                            goto done;
                        }
                        pdr->partition_base = extended_base + to_DWORD ((byte *) &ppart->ents[1].r_sec);
                        pdr->partition_size = to_DWORD ((byte *) &ppart->ents[1].p_size);
                        skip_count -= 1;
                    }
                }
            }
        }
    }
}
    /* Fall through to here if not in an extended partition */
#endif /* (SUPPORT_EXTENDED_PARTITIONS) */
    i = (word)pdr->partition_number;

    if (pc_validate_partition_type(ppart->ents[i].p_typ))
    {
        /* Get the relative start and size   */
        pdr->partition_base = to_DWORD ((byte *) &ppart->ents[i].r_sec);
        pdr->partition_size = to_DWORD ((byte *) &ppart->ents[i].p_size);
        pdr->partition_type = ppart->ents[i].p_typ;
        ret_val = READ_PARTION_OK;
    }
    else
    {
        rtfs_set_errno(PEINVALIDMBROFFSET);
        ret_val = READ_PARTION_NO_TABLE;
    }

done:
    pc_free_scratch_blk(buf);
    return(ret_val);
}


/****************************************************************************
    PC_GBLK0 -  Read block 0 and load values into a a structure

 Description
    Given a valid drive number, read block zero and convert
    its contents from intel to native byte order.

 Returns
    Returns TRUE if all went well.

****************************************************************************/

/* read block zero   */
BOOLEAN pc_gblk0(word driveno, struct pcblk0 *pbl0)                 /*__fn__*/
{
    BLKBUFF *buf;
    byte *b;
    /* Zero fill pbl0 so we do not get any surprises */
    rtfs_memset(pbl0, (byte) 0,sizeof(struct pcblk0));
    /* Grab a buffer to play with   */
    buf = pc_scratch_blk();
    if (!buf)
    {
        rtfs_set_errno(PERESOURCEBLOCK); /* pc_gblk0 couldn't allocate a buffer */
        return(FALSE);
    }
    b = buf->data;      /* Now we do not have to use the stack */

    /* get 1 block starting at 0 from driveno   */
    /* READ                                     */
    if (!devio_read(driveno, 0L ,b,1, FALSE))
    {
        rtfs_set_errno(PEIOERRORREADBPB); /* pc_gblk0 failed reading Bios Parameter block */
        pc_free_scratch_blk(buf);
        return(FALSE);
    }
    /* Now load the structure from the buffer   */
    pbl0->jump = b[0];
    copybuff( &pbl0->oemname[0],b+3,8);
    pbl0->oemname[8] = CS_OP_ASCII('\0');
    pbl0->secpalloc = b[0xd];
    pbl0->numfats = b[0x10];
    pbl0->mediadesc = b[0x15];
    pbl0->physdrv = b[0x24];            /* Physical Drive No. (4.0) */
    pbl0->xtbootsig = b[0x26];      /* Extended signt 29H if 4.0 stuf valid */
    /* BUG FIX 12-1-99 - Add KS_LITTLE_ODD_PTR_OK flag to split between
       big endian and little endian system. The top section works on little
       endian systems that do not require even alligned word accesses like
       the x86 but for example on Little endian ARM systems these assignments
       derefrencing a pointer to word at an odd address which gives bad data .*/
    pbl0->bytspsector = to_WORD(b+0xb); /*X*/
    pbl0->secreserved = to_WORD(b+0xe); /*X*/
    pbl0->numroot   = to_WORD(b+0x11); /*X*/
    pbl0->numsecs   = to_WORD(b+0x13); /*X*/
    pbl0->secpfat   = to_WORD(b+0x16); /*X*/
    pbl0->secptrk   = to_WORD(b+0x18); /*X*/
    pbl0->numhead   = to_WORD(b+0x1a); /*X*/
    pbl0->numhide   = to_WORD(b+0x1c); /*X*/
    pbl0->numhide2  = to_WORD(b+0x1e); /*X*/
    pbl0->numsecs2  = to_DWORD(b+0x20);/*X*/ /* # secs if > 32M (4.0) */
    pbl0->volid     = to_DWORD(b+0x27);/*X*/ /* Unique number per volume (4.0) */
    copybuff( &pbl0->vollabel[0],b+0x2b,11); /* Volume label (4.0) */

    if (pbl0->numroot == 0 && !pc_gblk0_32(driveno, pbl0, b))
    {
        pc_free_scratch_blk(buf);
        rtfs_set_errno(PEIOERRORREADINFO32); /* pc_gblk0_32 failed reading Bios Parameter block */
        return(FALSE);
    }
    pc_free_scratch_blk(buf);
    return(TRUE);
}


#if (RTFS_WRITE)
#if (RTFS_SUBDIRS)
/***************************************************************************
    PC_CLZERO -  Fill a disk cluster with zeroes

 Description
    Write zeroes into the cluster at clusterno on the drive pointed to by
    pdrive. Used to zero out directory and data file clusters to eliminate
    any residual data.

 Returns
    Returns FALSE on a write erro.

****************************************************************************/

/* Write zeros to all blocks in a cluster   */
BOOLEAN pc_clzero(DDRIVE *pdrive, CLUSTERTYPE cluster)                  /*__fn__*/
{
    BLKBUFF *pbuff;
    CLUSTERTYPE i;
    BLOCKT currbl;

    currbl = pc_cl2sector(pdrive , cluster);
    if (!currbl)
        return (FALSE);

    /*Init and write a block for each block in cl. Note: init clears the core  */
    for (i = 0; i < pdrive->secpalloc; i++, currbl++ )
    {
        pbuff = pc_init_blk( pdrive , currbl);
        if (!pbuff)
        {
            return (FALSE);
        }
        if (!pc_write_blk ( pbuff ) )
        {
            pc_discard_buf(pbuff);
            return (FALSE);
        }
        pc_release_buf(pbuff);
    }
    return (TRUE);
}
#endif
#endif

/****************************************************************************
    PC_DRNO2DR -  Convert a drive number to a pointer to DDRIVE

 Description
    Given a drive number look up the DDRIVE structure associated with it.

 Returns
    Returns NULL if driveno is not an open drive.

****************************************************************************/

DDRIVE *pc_drno_to_drive_struct(int driveno)                                    /*__fn__*/
{
DDRIVE  *pdr;

    pdr = 0;
    /* Check drive number   */
    if (pc_validate_driveno(driveno))
    {
        pdr = prtfs_cfg->drno_to_dr_map[driveno];
    }
    return(pdr);
}

DDRIVE  *pc_drno2dr(int driveno)                                    /*__fn__*/
{
DDRIVE  *pdr;
DDRIVE  *pretval;

    pdr = pc_drno_to_drive_struct(driveno);
    pretval = 0;

    OS_CLAIM_FSCRITICAL()
    /* Check drive number   */
    if (pdr)
    {
        if (pdr->mount_valid)
        {
            pretval = pdr;
        }
    }
    OS_RELEASE_FSCRITICAL()
    return(pretval);
}

/***************************************************************************
    PC_DSKFREE -  Deallocate all core associated with a disk structure

 Description
    Given a valid drive number. If the drive open count goes to zero, free the
    file allocation table and the block zero information associated with the
    drive. If unconditional is true, ignore the open count and release the
    drive.
    If open count reaches zero or unconditional, all future accesses to
    driveno will fail until re-opened.

 Returns
    Returns FALSE if driveno is not an open drive.

****************************************************************************/


/* free up all core associated with the drive
    called by close. A drive restart would consist of
    pc_dskfree(driveno, TRUE), pc_dskopen() */
BOOLEAN pc_dskfree(int driveno)                          /*__fn__*/
{
    DDRIVE *pdr;

    /* Note this will fail unless mount_valid is true */
    pdr = pc_drno2dr(driveno);
    if (!pdr)
    {
        return(FALSE);
    }

    if (pdr->mount_valid)
    {
        /* Free the current working directory for this drive for all users   */
        pc_free_all_users(driveno);
        /* Free all files, finodes & blocks associated with the drive   */
        pc_free_all_fil(pdr);
        pc_free_all_i(pdr);
        /* Free all drobj structures that have not yet been accessed */
        pc_free_all_drobj(pdr);
        pc_free_all_blk(pdr);
        /* Clear the fat cache */
        pc_free_all_fat_blocks(&pdr->fatcontext);
    }

    pdr->mount_valid = FALSE;
    pdr->mount_abort = FALSE;
    return (TRUE);
}



/****************************************************************************
    PC_SEC2CLUSTER - Convert a block number to its cluster representation.

 Description
    Convert blockno to its cluster representation if it is in cluster space.

 Returns
    Returns 0 if the block is not in cluster space, else returns the
    cluster number associated with block.

****************************************************************************/

#if (RTFS_SUBDIRS)
/* Cluster<->sector conversion routines       */
/* Convert sector to cluster. 0 == s error    */
CLUSTERTYPE pc_sec2cluster(DDRIVE *pdrive, BLOCKT blockno)              /*__fn__*/
{
    BLOCKT ltemp;
    BLOCKT answer;

    if ((blockno >= pdrive->numsecs) || (pdrive->firstclblock > blockno))
        return (0);
    else
    {
        /*  (2 + (blockno - pdrive->firstclblock)/pdrive->secpalloc)   */
        ltemp = blockno - pdrive->firstclblock;
        answer = ltemp;
        answer = (BLOCKT) answer >> pdrive->log2_secpalloc;
        answer += 2;
        return ((CLUSTERTYPE)answer);
    }
}
#endif

/****************************************************************************
    PC_SEC2INDEX - Calculate the offset into a cluster for a block.

 Description
    Given a block number offset from the beginning of the drive, calculate
    which block number within a cluster it will be. If the block number
    coincides with a cluster boundary, the return value will be zero. If it
    coincides with a cluster boundary + 1 block, the value will be 1, etc.


 Returns
    0,1,2 upto blockspcluster -1.

***************************************************************************/

#if (RTFS_SUBDIRS)
/* Convert sector to index into a cluster . No error detection   */
word pc_sec2index(DDRIVE *pdrive, BLOCKT blockno)               /*__fn__*/
{
    BLOCKT answer;

    /*  ((blockno - pdrive->firstclblock) % pdrive->secpalloc) );   */

    answer = blockno - pdrive->firstclblock;
    answer = answer % pdrive->secpalloc;

    return ( (word) answer);
}

#endif


/***************************************************************************
    PC_CL2SECTOR - Convert a cluster number to block number representation.

 Description
    Convert cluster number to a blocknumber.

 Returns
    Returns 0 if the cluster is out of range. else returns the
    block number of the beginning of the cluster.


****************************************************************************/

/* Convert cluster. to sector   */
BLOCKT pc_cl2sector(DDRIVE *pdrive, CLUSTERTYPE cluster)               /*__fn__*/
{
    BLOCKT blockno;
    dword t;

    if (cluster < 2)
        return (BLOCKEQ0);
    else
    {
        t = cluster - 2;
        t = t << pdrive->log2_secpalloc;
        blockno = pdrive->firstclblock + t;
    }
    if (blockno >= pdrive->numsecs)
        return (BLOCKEQ0);
    else
        return (blockno);
}
/* Round a size up to it's next cluster boundary and return
   the length in clusters */
dword pc_chain_length(DDRIVE *pdrive, dword byte_length)
{
dword ltemp,chain_length;
    ltemp = byte_length + pdrive->byte_into_cl_mask;
    if (ltemp > byte_length)
    {
        chain_length =  ltemp & ~(pdrive->byte_into_cl_mask);
        chain_length >>= (int) (pdrive->log2_secpalloc + 9);
    }
    else
    {
        ltemp = (byte_length - pdrive->bytespcluster)+ pdrive->byte_into_cl_mask;
        chain_length =  ltemp & ~(pdrive->byte_into_cl_mask);
        chain_length >>= (int) (pdrive->log2_secpalloc + 9);
        chain_length += 1;
    }
    return(chain_length);
}
