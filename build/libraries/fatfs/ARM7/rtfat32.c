/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 1987-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
/* RTFAT32.C - FAT32 specific managment routines  */

#include <rtfs.h>

CLUSTERTYPE fatxx_clalloc(DDRIVE *pdr, CLUSTERTYPE clhint);
CLUSTERTYPE  fatxx_clgrow(DDRIVE *pdr, CLUSTERTYPE  clno);

#if (FAT32)

struct fat32_info {
        dword   fs_sig;             /* Signature of FAT32 (0x61417272) */
#define FSINFOSIG 0x61417272ul
        dword   free_alloc;         /* Free clusters on drive (-1 if unknown) */
        dword   next_alloc;         /* Most recently allocated cluster */
        dword   reserved;           /* Reserved - ignore */
        };

BOOLEAN pc_init_drv_fat_info(DDRIVE *pdr, struct pcblk0 *pbl0)
{
    if (pdr->numroot!=0) /* Drive is not FAT32 */
        return (pc_init_drv_fat_info16(pdr, pbl0));
    pdr->secpfat = (CLUSTERTYPE) pbl0->secpfat;   /* sectors / fat */
    pdr->numfats = pbl0->numfats; /* Number of fat copies */
    if (pdr->secpfat == 0L)
        pdr->secpfat = pbl0->secpfat2;

    if (pbl0->flags & NOFATMIRROR)
    {
        pdr->fatblock = (BLOCKT) pbl0->secreserved +
                        ((pbl0->flags & ACTIVEFAT) * pdr->secpfat);
        pdr->numfats = 1;
    }
    else
        pdr->fatblock = (BLOCKT) pbl0->secreserved;
        /* The first block of the root is just past the fat copies   */
     pdr->firstclblock = pdr->fatblock + pdr->secpfat * pdr->numfats;
/* DM: 7-6-99: BUG FIX: */
     pdr->rootblock = (pbl0->rootbegin-2) * pdr->secpalloc + pdr->firstclblock;
/* WRONG: pdr->rootblock = pbl0->rootbegin-2 + pdr->firstclblock; */

    /*  Calculate the largest index in the file allocation table.
        Total # block in the cluster area)/Blockpercluster == s total
        Number of clusters. Entries 0 & 1 are reserved so the highest
        valid fat index is 1 + total # clusters.
    */
    pdr->maxfindex = (long) (1 + ((pdr->numsecs - pdr->firstclblock)/pdr->secpalloc));
	{
		/* Make sure the calculated index doesn't overflow the fat sectors */
		dword max_index;
		/* For FAT32 Each block of the fat holds 128 entries so the maximum index is
		   (pdr->secpfat * 128)-1; */
		max_index = (dword) pdr->secpfat;
		max_index *= 128;
		max_index -= 1;
		if (pdr->maxfindex > max_index)
			pdr->maxfindex = max_index;
	}

    /* Create a hint for where we should write file data. We do this
        because directories are allocated in one cluster chunks while
        file may allocate larger chunks. We Try to put directory
        data at the beginning of the disk in a seperate region so we
        do not break the contiguous space further out */

    pdr->known_free_clusters = pbl0->free_alloc;
    pdr->free_contig_base = pbl0->next_alloc;
   /* 2-10-2007 - Added defensive code to check for a valid start hint. If it is out of range set it to
      the first cluster in the FAT */
    if (pdr->free_contig_base < 2 || pdr->free_contig_base >= pdr->maxfindex)
        pdr->free_contig_base = 2;

    pdr->free_contig_pointer = pdr->free_contig_base;
    pdr->infosec = pbl0->infosec;
    pdr->fasize = 8;
    return(TRUE);
}
CLUSTERTYPE pc_get_parent_cluster(DDRIVE *pdrive, DROBJ *pobj) /* __fatfn__ */
{
        if ((pdrive->fasize == 8) &&
            (pobj->blkinfo.my_frstblock == pdrive->rootblock))
            return(0);
        else
            return(pc_sec2cluster(pdrive,pobj->blkinfo.my_frstblock));
}

/* Grab a cluster for a new directory entry.
   To minimize fragmentation give a hint where to start looking for new
   clusters based on the position of the parent directory */

CLUSTERTYPE pc_alloc_dir(DDRIVE *pdrive, DROBJ *pmom)  /* __fatfn__ */
{
CLUSTERTYPE clbase,cluster;
        if ( pdrive->fasize != 8 && pc_isroot(pmom))
            clbase = 0;
        else
            clbase = pc_finode_cluster(pmom->pdrive,pmom->finode);
        /*Grab a cluster for the new dir */
        cluster = fatxx_clalloc(pdrive, clbase);
        return(cluster);
}

CLUSTERTYPE pc_grow_dir(DDRIVE *pdrive, DROBJ *pobj) /* __fatfn__ */
{
CLUSTERTYPE tmpcl, cluster;
    if ( pdrive->fasize != 8 && pc_isroot(pobj))
    {
        rtfs_set_errno(PENOSPC);
        cluster = 0;
    }
    else
    {
        tmpcl = pc_finode_cluster(pdrive,pobj->finode);
        if ( (pdrive->fasize == 8) && (tmpcl == 0) )
           tmpcl = pc_sec2cluster(pdrive, pdrive->rootblock);
        cluster = fatxx_clgrow(pdrive, tmpcl);
    }
    return(cluster);
}

void pc_truncate_dir(DDRIVE *pdrive, DROBJ *pobj, CLUSTERTYPE cluster) /* __fatfn__ */
{
    if (pdrive->fasize == 8)
    {
        dword tmpcl = pc_finode_cluster(pdrive,pobj->finode);
        if (tmpcl == 0)
            tmpcl = pc_sec2cluster(pdrive, pdrive->rootblock);
        FATOP(pdrive)->fatop_cl_truncate_dir(pdrive, tmpcl, cluster);
    }
    else
        FATOP(pdrive)->fatop_cl_truncate_dir(pdrive, pc_finode_cluster(pdrive,pobj->finode), cluster);
}

BOOLEAN pc_mkfs32(int driveno, FMTPARMS *pfmt, BOOLEAN use_raw)                         /*__fn__*/
{
    byte *b;
    BLKBUFF *buf;
    dword  ltotsecs;
    word totsecs;
    dword  lnclusters;
    dword ldata_area;
    int fausize;
    word i,j,k;
    BLOCKT blockno;
    BOOLEAN ret_val;

    if (use_raw)    /* check media and clear change conditions */
        check_drive_number_present(driveno);

    ret_val = FALSE;

    buf = pc_scratch_blk();
    if (!buf)
        return(FALSE);
    b = buf->data;

    /* Build up a block 0   */
    rtfs_memset(&b[0], 0, 512);
    for (i=0;i<pfmt->secreserved;i++)
    {
        /* WRITE   */
        if (!devio_write_format(driveno, (dword) i, &(b[0]), 1, use_raw) )
        {
            goto errex;
        }
    }
#if (INCLUDE_FAT32_BOOT_CODE)
    copybuff(&b[0],&FAT32_BOOT_CODE[512],512);
    if (!devio_write_format(driveno, (dword) 8, &(b[0]), 1, use_raw) )
    {
        goto errex;
    }
    if (!devio_write_format(driveno, (dword) 2, &(b[0]), 1, use_raw) )
    {
        goto errex;
    }
    copybuff(&b[0],&FAT32_BOOT_CODE[0],512);
#else
    rtfs_memset(&b[0], 0, 512);
#endif
    b[0] = (byte) 0xe9;    /* Jump vector. Used to id MS-DOS disk */
    b[1] = (byte) 0x00;
    b[2] = (byte) 0x00;
    b[0x40] = 0x80;
    b[0x41] = 0x00;
    b[0x42] = 0x29;
#if (INCLUDE_FAT32_BOOT_CODE)
    b[0] = (byte) 0xeb;    /* Jump vector. Used to id MS-DOS disk */
    b[1] = (byte) 0x58;
    b[2] = (byte) 0x90;
#endif
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

    /* if secpfat was not provided calculate it here   */
    /* TONY - Bug fix 6/23/98 */
    if (!pfmt->secpfat)
    {
       pfmt->secpfat = (ltotsecs + 128*pfmt->secpalloc)
                            /(128*pfmt->secpalloc + 1);
    }
    /* Media descriptor   */
    b[21] = pfmt->mediadesc;
    /* sectors per trak   */
    fr_WORD ( &(b[24]), pfmt->secptrk);
    /* number heads   */
    fr_WORD ( &(b[26]), pfmt->numhead);
    /* number hidden sectors   */
    fr_DWORD ( &(b[28]), pfmt->numhide);
    /* number of duplicate fats   */
    b[16] = pfmt->numfats;
    fr_WORD ( &(b[22]), (word)0);
    fr_DWORD ( &(b[36]), (dword)pfmt->secpfat);
    fr_DWORD ( &(b[40]), (dword)0); /* flags and version */
    fr_DWORD ( &(b[44]), (dword)2); /* root dir starting cluster */
    fr_WORD ( &(b[48]), (word)1);   /* info sector */
    fr_WORD ( &(b[50]), (word)6); /* backup boot sector */

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

    fr_DWORD( &(b[0x43]), pfmt->binary_volume_label);
    pc_cppad( &(b[0x47]), (byte*)pfmt->text_volume_label, 11);
    pc_cppad( &(b[0x52]), (byte*)"FAT32",8);
    fr_WORD(&(b[0x01fe]), (word)0xaa55);

    /* Count the size of the area managed by the fat.   */
    ldata_area = ltotsecs;
    ldata_area -= pfmt->numfats * pfmt->secpfat;
    ldata_area -= pfmt->secreserved;

    /* Note: numroot must be an even multiple op INOPBLOCK   */
    ldata_area -= pfmt->numroot/INOPBLOCK;

    /* Nibbles/fat entry if < 4087 clusters then 12 bit else 16   */
    lnclusters =  ldata_area/pfmt->secpalloc;
    fausize = 8;

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
            rtfs_set_errno(PEINVALIDPARMS);
            goto errex;
        }
    }

    if (pfmt->numroot % INOPBLOCK)
    {
        rtfs_set_errno(PEINVALIDPARMS);
        goto errex;
    }

    if (!devio_write_format(driveno, 0, &(b[0]), 1, use_raw) )
    {
        goto errex;
    }
    /* Put the initial free cluster value in right */
    rtfs_memset(&b[0], 0, 512);
    fr_DWORD( &(b[0]), (dword) 0x41615252ul);
    fr_DWORD( &(b[0x01e4]), (dword) FSINFOSIG);
    fr_DWORD( &(b[0x01e8]), (dword)(lnclusters-1));
    fr_DWORD( &(b[0x01ec]), (dword)0x00000003);
    fr_WORD( &(b[0x01fe]), (word)0xaa55);

    if (!devio_write_format(driveno, (dword) 7, &(b[0]), 1, use_raw) )
    {
        goto errex;
    }
    if (!devio_write_format(driveno, (dword) 1, &(b[0]), 1, use_raw) )
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
        j = (word) fausize;
        if (j==8) j=12;
        while(j > 3)
        {
            if (j%4 == 0 && fausize == 8)
                b[--j] = (byte) 0x0f;
            else
                b[--j] = (byte) 0xff;
        }

        blockno = pfmt->secreserved + (i * pfmt->secpfat);
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
    blockno = pfmt->secreserved + pfmt->numfats * pfmt->secpfat;
    rtfs_memset(&b[0], 0, 512);
    /* Bug fix 11-22-99 use <pfmt->secpalloc instead of 8 */
    for(k=0;k<pfmt->secpalloc;k++) /* Is 8 blocks per cluster? */
    {
        if (!devio_write_format(driveno, blockno+k, &(b[0]), 1, use_raw) )
            goto errex;
    }
    ret_val = TRUE;

errex:      /* Not only errors return through here. Everything does. */
    pc_free_scratch_blk(buf);
    return(ret_val);

}
CLUSTERTYPE pc_finode_cluster(DDRIVE *pdr, FINODE *finode)  /* __fatfn__ */
{
    if (pdr->fasize == 8)
        return ( (dword)finode->fcluster | ((dword)finode->fclusterhi << 16) );
    else
        return ( (CLUSTERTYPE)finode->fcluster );
}

void pc_pfinode_cluster(DDRIVE *pdr, FINODE *finode, CLUSTERTYPE value) /*__fatfn__ */
{
    finode->fcluster = (word)value;
    if (pdr->fasize == 8)
        finode->fclusterhi = (word)(value >> 16);
}
BOOLEAN pc_gblk0_32(word driveno, struct pcblk0 *pbl0, byte *b)                 /*__fn__*/
{
    if (pbl0->numroot == 0)
    {
        pbl0->secpfat2    = to_DWORD(b+0x24);
        pbl0->flags       = to_WORD(b+0x28);
        pbl0->fs_version  = to_WORD(b+0x2a);
        pbl0->rootbegin   = to_DWORD(b+0x2c);
        pbl0->infosec     = to_WORD(b+0x30);
        pbl0->backup      = to_WORD(b+0x32);
        copybuff( &pbl0->vollabel[0],b+0x47,11); /* Volume label FAT32 */
        if (!devio_read(driveno,(BLOCKT)(pbl0->infosec) ,b,1, FALSE))
        {
            return(FALSE);
        }
        /* 3-07-02 - Remove scan to find INFOSIG. Access at offset 484. */
        b += 484;
        pbl0->free_alloc = to_DWORD((void  *)&((struct fat32_info *)b)->free_alloc);
        pbl0->next_alloc = to_DWORD((void  *)&((struct fat32_info *)b)->next_alloc);
    }
    return(TRUE);
}
BOOLEAN pc_validate_partition_type(byte p_type)
{
    if ( (p_type == 0x01) || (p_type == 0x04) || (p_type == 0x06) ||
         (p_type == 0x0E) ||   /* Windows FAT16 Partition */
         (p_type == 0x0B) ||   /* FAT32 Partition */
         (p_type == 0x0C) ||   /* FAT32 Partition */
         (p_type == 0x55) )    /* FAT32 Partition */
         return(TRUE);
    else
         return(FALSE);
}
/* Update a fat32 voulme s info sector   */
BOOLEAN fat_flushinfo(DDRIVE *pdr)                                      /*__fn__*/
{
    byte  *pf;
    BLKBUFF *buf;

    if (pdr->fasize == 8)
    {
        /* Use read_blk, to take advantage of the failsafe cache */
        buf = pc_read_blk(pdr, (dword) pdr->infosec);
        if(!buf)
        {
            rtfs_set_errno(PEIOERRORREADINFO32); /* fat_flushinfo: failed writing info block */
            goto info_error;
        }
        /* Merge in the new values   */
        pf = buf->data;      /* Now we do not have to use the stack */
        /* 3-07-02 - Remove scan to find INFOSIG. Access at offset 484. */
        pf += 484;
        fr_DWORD((byte *) (&((struct fat32_info *)pf)->free_alloc), pdr->known_free_clusters );
/*        fr_DWORD((byte *) (&((struct fat32_info *)pf)->next_alloc), pdr->free_contig_pointer ); */
        /* 2-10-2007 - put  free_contig_base in allocation hint field. This forces cluster
           allocations to initially scan from the base of the FAT for free clusters rather
           than from the previous "most likely" location */
        fr_DWORD((byte *) (&((struct fat32_info *)pf)->next_alloc), pdr->free_contig_base );
        /* Use write_blk, to take advantage of the failsafe cache */
        if (!pc_write_blk(buf))
        {
            rtfs_set_errno(PEIOERRORWRITEINFO32); /* fat_flushinfo: failed writing info block */
            pc_discard_buf(buf);
info_error:
            pc_report_error(PCERR_FAT_FLUSH);
            return(FALSE);
        }
        pc_release_buf(buf); /* Leave it cached */
    }
    return (TRUE);
}

#if (RTFS_WRITE)

BOOLEAN fatxx_pfaxxterm(DDRIVE   *pdr, CLUSTERTYPE  clno);
byte * fatxx_pfswap(DDRIVE *pdr, CLUSTERTYPE index, BOOLEAN for_write);

/* Put a DWORD value into the fat at index                */
BOOLEAN fatxx_pfpdword(DDRIVE *pdr, dword index, dword *pvalue)          /*__fatfn__*/
{
    dword  *ppage;
    dword offset;
    /* Make sure we have access to the page. Mark it for writing   */
    ppage = (dword  *)fatxx_pfswap(pdr,index,TRUE);

    if (!ppage)
        return(FALSE);
    else
    {
        /* there are 128 entries per page   */
        offset = (dword) (index & 0x7f);
        ppage[(int)offset] = *pvalue;
    }
    return(TRUE);
}
#endif

/* Get a DWORD value from the fat at index                */
BOOLEAN fatxx_pfgdword(DDRIVE *pdr, dword index, dword *value)          /*__fatfn__*/
{
    dword  *ppage;
    dword offset;
    /* Make sure we have access to the page. Do not Mark it for writing   */
    ppage = (dword  *)fatxx_pfswap(pdr,index,FALSE);

    if (!ppage)
        return(FALSE);
    else
    {
        /* there are 128 entries per page   */
        offset = (dword) (index & 0x7f);
        *value = ppage[(int)offset];
    }
    return(TRUE);
}


#endif
