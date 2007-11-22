/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* RTFAT16.C - FAT16 management routines  */

#include <rtfs.h>

CLUSTERTYPE fatxx_clalloc(DDRIVE *pdr, CLUSTERTYPE clhint);
CLUSTERTYPE  fatxx_clgrow(DDRIVE *pdr, CLUSTERTYPE  clno);


CLUSTERTYPE fatxx_clalloc(DDRIVE *pdr, CLUSTERTYPE clhint);
CLUSTERTYPE  fatxx_clgrow(DDRIVE *pdr, CLUSTERTYPE  clno);


BOOLEAN pc_init_drv_fat_info16(DDRIVE *pdr, struct pcblk0 *pbl0)
{
    pdr->secpfat = (CLUSTERTYPE) pbl0->secpfat;   /* sectors / fat */
    pdr->numfats = pbl0->numfats; /* Number of fat copies */
    pdr->fatblock = (BLOCKT) pbl0->secreserved;
    pdr->rootblock = pdr->fatblock + pdr->secpfat * pdr->numfats;
    pdr->secproot =  (int)((pdr->numroot + INOPBLOCK - 1)/INOPBLOCK);
    /* The first block of the cluster area is just past the root   */
    /* Round up if we have to                                      */
    pdr->firstclblock = pdr->rootblock +
                        (pdr->numroot + INOPBLOCK - 1)/INOPBLOCK;
    /*  Calculate the largest index in the file allocation table.
        Total # block in the cluster area)/Blockpercluster == s total
        Number of clusters. Entries 0 & 1 are reserved so the highest
        valid fat index is 1 + total # clusters.
    */
    pdr->maxfindex = (long)(1 + ((pdr->numsecs - pdr->firstclblock)/pdr->secpalloc));
    /* Nibbles/fat entry if < 4087 clusters then 12 bit else 16   */
    pdr->fasize = (int) ((pdr->maxfindex < 4087) ? 3 : 4);
    {
    /* Make sure the calculated index doesn't overflow the fat sectors */
    dword max_index;
    /* For FAT16 Each block of the fat holds 256 entries so the maximum index is
        (pdr->secpfat * 256)-1; */
        max_index = (dword) pdr->secpfat;
        if (pdr->fasize == 3)
        {/* Max clusters in FAT12.. 1024 per 3 blocks plus 341 per residual block */
         /* Modified 8-16-06 to distinguish between FAT12 and FAT16 */
            dword max_index3,div3,ltemp,residual;
            max_index3 = max_index;
            div3 = max_index3/3;
            max_index = div3 * 1024;
            ltemp = div3 * 3;
            residual = max_index3-ltemp;
            max_index += (residual * 341);
        }
        else
        {
            max_index *= 256;
        }
        max_index -= 1;
        if (pdr->maxfindex > max_index)
            pdr->maxfindex = max_index;
    }
    /* if calculated size > fff0 set it to one less. fff0 to ffff are
        reserved values. */
    if (pdr->maxfindex >= 0xfff0 && pdr->maxfindex <= 0xffff)
        pdr->maxfindex = 0xffef;

    /* Create a hint for where we should write file data. We do this
        because directories are allocated in one cluster chunks while
        file may allocate larger chunks. We Try to put directory
        data at the beginning of the disk in a seperate region so we
        do not break the contiguous space further out */

    /* guess that 1/32nd of the disk will store directory info and the
        rest will be data. */
    pdr->free_contig_base = (word) (pdr->maxfindex >> 5);
    if (pdr->free_contig_base < 2)
        pdr->free_contig_base = 2;
     /* set the pointer to where to look for free clusters to the contiguous
        area. On the first call to write this will hunt for the real free
        blocks. */
    pdr->free_contig_pointer = pdr->free_contig_base;

    pdr->infosec = 0;
    return(TRUE);
}
BOOLEAN pc_mkfs16(int driveno, FMTPARMS *pfmt, BOOLEAN use_raw)                         /*__fn__*/
{
    byte *b;
    BLKBUFF *buf;
    dword  ltotsecs;
    word totsecs;
    word nclusters;
    dword  lnclusters;
    dword ldata_area;
    int fausize;
    word i,j;
    BLOCKT blockno;
    BOOLEAN ret_val;

    if (use_raw)    /* check media and clear change conditions */
        check_drive_number_present(driveno);

    ret_val = FALSE;

    buf = pc_scratch_blk();
    if (!buf)
    {
        return(FALSE);
    }
    b = buf->data;

    /* Build up a block 0   */
    rtfs_memset(&b[0], 0, 512);

    b[0] = (byte) 0xe9;    /* Jump vector. Used to id MS-DOS disk */
    b[1] = (byte) 0x00;
    b[2] = (byte) 0x00;
    /* Copy the OEM name   */
    pc_cppad(&b[3], pfmt->oemname, 8);
    /* bytes per sector   */

    fr_WORD ( &(b[11]), 512);   /*X*/
    /* sectors / cluster   */
    b[13] = pfmt->secpalloc;
    /* Number of reserved sectors. (Including block 0)   */
    fr_WORD ( &(b[14]), pfmt->secreserved); /*X*/
    /* number of dirents in root   */
    fr_WORD ( &(b[17]), pfmt->numroot);     /*X*/
    /* total sectors in the volume   */

    /* Set totsecs to 0 if size > 64k. This triggers sensing huge 4.0
       partitions. */

    ltotsecs = pfmt->numcyl;
    ltotsecs *= pfmt->secptrk;
    ltotsecs *= pfmt->numhead;
    ltotsecs -= pfmt->numhide;	//ctr modified

    if (ltotsecs > 0xffffL)
    {
        /* HUGE partition  the 3.xx totsecs field is zeroed   */
        totsecs = 0;
    }
    else
    {
        totsecs = (word) ltotsecs;
    }
    fr_WORD ( &(b[19]), totsecs);   /*X*/

    /* Media descriptor   */
    b[21] = pfmt->mediadesc;
    /* sectors per trak   */
    fr_WORD ( &(b[24]), pfmt->secptrk); /*X*/
    /* number heads   */
    fr_WORD ( &(b[26]), pfmt->numhead); /*X*/
    /* number hidden sectors   */
    fr_DWORD ( &(b[28]), pfmt->numhide);
    /* number of duplicate fats   */
    b[16] = pfmt->numfats;
    fr_WORD ( &(b[22]), (word)pfmt->secpfat); /*X*/

    /* Now fill in 4.0 specific section of the boot block   */
    if (ltotsecs > 0xffffL)
    {
        /* HUGE partition   */
        fr_DWORD ( &(b[32]), ltotsecs); /*X*/
    }
    else
    {
        fr_DWORD ( &(b[32]), 0L);       /*X*/
    }

    b[36] = pfmt->physical_drive_no;
    b[38] = 0x29;                       /* extended boot signature */
    fr_DWORD(&(b[39]) , pfmt->binary_volume_label); /*X*/
    pc_cppad( &(b[43]), pfmt->text_volume_label, 11);
    fr_WORD(&(b[0x01fe]), (word)0xaa55);

    /* Count the size of the area managed by the fat.   */
    ldata_area = ltotsecs;
    ldata_area -= pfmt->numfats * pfmt->secpfat;
    ldata_area -= pfmt->secreserved;

    /* Note: numroot must be an even multiple op INOPBLOCK   */
    ldata_area -= pfmt->numroot/INOPBLOCK;

    /* Nibbles/fat entry if < 4087 clusters then 12 bit else 16   */
    lnclusters =  ldata_area/pfmt->secpalloc;
    if (lnclusters > 0xffffL)
    {
        rtfs_set_errno(PEINVALIDPARMS); /* pc_mkfs16: volume too large */
        goto errex;
    }
    else
    {
        nclusters = (word) lnclusters;
    }

    fausize = (int) ( (nclusters < 4087) ? 3 : 4 );

    /* Check the FAT.
    if ( (nibbles needed) > (nibbles if fatblocks)
            trouble;
    */
    {
    long ltotnibbles;
    long lnibsinfatbls;

        /* Total nibbles = (# clusters * nibbles/cluster)   */
        ltotnibbles = lnclusters;
        ltotnibbles *= fausize;

        /* How many nibbles are available.   */
        lnibsinfatbls = pfmt->secpfat;
        lnibsinfatbls <<= 10;            /* 1024 nibbles/block */

        if (ltotnibbles > lnibsinfatbls)
        {
            rtfs_set_errno(PEINVALIDPARMS); /* pc_mkfs16: volume too large */
            goto errex;
        }
    }

    if (pfmt->numroot % INOPBLOCK)
    {
        rtfs_set_errno(PEINVALIDPARMS); /* pc_mkfs16: numroot incorrect */
        goto errex;
    }


    if (!devio_write_format(driveno, (dword) 0 + pfmt->numhide, &(b[0]), 1, use_raw) )
    {
        goto errex;
    }
    /* Now write the fats out   */
    for (i = 0; i < pfmt->numfats; i++)
    {
        rtfs_memset(&b[0], 0, 512);
        /* The first 3(4) bytes of a fat are MEDIADESC,FF,FF,(FF)   */
        b[0] = pfmt->mediadesc;
        b[1] = (byte) 0xff;
        b[2] = (byte) 0xff;
        if (fausize == 4)
            b[3] = (byte) 0xff;

        blockno = pfmt->numhide + pfmt->secreserved + (i * pfmt->secpfat);	//ctr modified
        for ( j = 0; j < pfmt->secpfat; j++)
        {
            /* WRITE   */
            if (!devio_write_format(driveno, blockno, &(b[0]), 1, use_raw) )
            {
                goto errex;
            }
            blockno += 1;
            rtfs_memset(&b[0], 0, 512);
        }
    }

    /* Now write the root sectors   */
    blockno = pfmt->numhide + pfmt->secreserved + pfmt->numfats * pfmt->secpfat; //ctr modified
    rtfs_memset(&b[0], 0, 512);
    for ( j = 0; j < (pfmt->numroot/INOPBLOCK) ; j++)
    {
        if (!devio_write_format(driveno, blockno, &(b[0]), 1, use_raw) )
        {
            goto errex;
        }
        blockno += 1;
    }
    ret_val = TRUE;

errex:      /* Not only errors return through here. Everything does. */
    pc_free_scratch_blk(buf);
    return(ret_val);
}

#if (!FAT32)

BOOLEAN pc_init_drv_fat_info(DDRIVE *pdr, struct pcblk0 *pbl0)
{
    return (pc_init_drv_fat_info16(pdr, pbl0));
}

CLUSTERTYPE pc_get_parent_cluster(DDRIVE *pdrive, DROBJ *pobj)
{
        return(pc_sec2cluster(pdrive,pobj->blkinfo.my_frstblock));
}
/* Grab a cluster for a new directory entry.
   To minimize fragmentation give a hint where to start looking for new
   clusters based on the position of the parent directory */

CLUSTERTYPE pc_alloc_dir(DDRIVE *pdrive, DROBJ *pmom)
{
CLUSTERTYPE clbase,cluster;
        if (pc_isroot(pmom))
            clbase = 0;
        else
            clbase = pc_finode_cluster(pmom->pdrive,pmom->finode);
        /*Grab a cluster for the new dir */
        cluster = fatxx_clalloc(pdrive, clbase);
        return(cluster);
}

CLUSTERTYPE pc_grow_dir(DDRIVE *pdrive, DROBJ *pobj)
{
CLUSTERTYPE cluster;
    if  (pc_isroot(pobj))
    {
        rtfs_set_errno(PENOSPC);
        cluster = 0;
    }
    else
        cluster = fatxx_clgrow(pdrive, pc_finode_cluster(pdrive,pobj->finode));
    return(cluster);
}
void pc_truncate_dir(DDRIVE *pdrive, DROBJ *pobj, CLUSTERTYPE cluster)
{
     FATOP(pdrive)->fatop_cl_truncate_dir(pdrive, pc_finode_cluster(pdrive,pobj->finode), cluster);
}
BOOLEAN pc_mkfs32(int driveno, FMTPARMS *pfmt, BOOLEAN use_raw)                         /*__fn__*/
{
    RTFS_ARGSUSED_INT(driveno);
    RTFS_ARGSUSED_PVOID((void *) pfmt);
    RTFS_ARGSUSED_INT((int) use_raw);
    return(FALSE);
}
CLUSTERTYPE pc_finode_cluster(DDRIVE *pdr, FINODE *finode)  /* __fn__ */
{
    RTFS_ARGSUSED_PVOID((void *) pdr);
    return ( (CLUSTERTYPE)finode->fcluster );
}

void pc_pfinode_cluster(DDRIVE *pdr, FINODE *finode, CLUSTERTYPE value) /*__fn__ */
{
    finode->fcluster = (word)value;
    RTFS_ARGSUSED_PVOID((void *) pdr);
}
BOOLEAN pc_gblk0_32(word driveno, struct pcblk0 *pbl0, byte *b)                 /*__fn__*/
{
    RTFS_ARGSUSED_INT((int) driveno);
    RTFS_ARGSUSED_PVOID((void *) pbl0);
    RTFS_ARGSUSED_PVOID((void *) b);
    return(FALSE);
}
BOOLEAN pc_validate_partition_type(byte p_type)
{
    if ( (p_type == 0x01) || (p_type == 0x04) || (p_type == 0x06) || (p_type == 0x0E))
         return(TRUE);
    else
         return(FALSE);
}
BOOLEAN fat_flushinfo(DDRIVE *pdr)
{
    RTFS_ARGSUSED_PVOID((void *) pdr);
    return(TRUE);
}
#endif
