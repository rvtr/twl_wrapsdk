/*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc. 2000
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*/
#include <rtfs.h>

/* */
/* New format logic */
/* */
/* */
/* */
void get_format_parameters(dword nblocks, int *psector_p_alloc, int *pnum_root_entries);
word pc_fat_size(word nreserved, word cluster_size, word n_fat_copies,
                 word root_sectors, dword volume_size, 
                 int *nibs_per_entry);


void pc_calculate_chs(dword total, dword *cylinders, int *heads, int *secptrack)
{
    dword c, h, s;

    if (total >= 255 * 255 * 63)
    {
        /* this block is just an optimization of the loop below */
        s = 63;
        h = 255;
        c = total / (255 * 63);
        if (c > 1023)
            c = 1023;
    }
    else
    {
        c = h = s = 1;
        if (total >= 63 * 63 * 63)
            s = 63;
        while (c < 1023 && (c + 1) * h * s <= total)
        {
            if (h > c || h == 255)
                c++;
            else if (s > h || s == 63)
                h++;
            else
                s++;
        }
    }

    if (cylinders)
        *cylinders = c;
    if (heads)
        *heads = (int) h;
    if (secptrack)
        *secptrack = (int) s;
}


/* */
/*1. Floppy driver must return partition info. Must format */
/*2. other drivers must return info. must format. See ATAPI */


/* 10-24-2000 added LBA formatting. Submit to main tree */


/***************************************************************************
    PC_GET_MEDIA_PARMS -  Get media parameters.

Description
    Queries the drive s associated device driver for a description of the
    installed media. This information is used by the pc_format_media,
    pc_partition_media and pc_format_volume routines. The application may 
    use the results of this call to calculate how it wishes the media to 
    be partitioned.

    Note that the floppy device driver uses a back door to communicate
    with the format routine through the geometry structure. This allows us 
    to not have floppy specific code in the format routine but still use the 
    exact format parameters that DOS uses when it formats a floppy.

    See the following definition of the pgeometry structure:
        typedef struct dev_geometry {
            int dev_geometry_heads;     -- - Must be < 256
            int dev_geometry_cylinders; -- - Must be < 1024
            int dev_geometry_secptrack; -- - Must be < 64
            BOOLEAN fmt_parms_valid;    -- If the device io control call sets this
                                        -- TRUE then it it telling the applications
                                        -- layer that these format parameters should
                                        -- be used. This is a way to format floppy
                                        -- disks exactly as they are fromatted by dos.
            FMTPARMS fmt;
        } DEV_GEOMETRY;

typedef struct dev_geometry  *PDEV_GEOMETRY;




Returns
    Returns TRUE if it was able to get the parameters otherwise
    it returns FALSE.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PEDEVICEFAILURE - Device driver get device geometry request failed
    PEINVALIDPARMS  - Device driver returned bad values
****************************************************************************/

BOOLEAN pc_get_media_parms(byte *path, PDEV_GEOMETRY pgeometry) /* __apifn__*/
{
int driveno;
dword total;
DDRIVE *pdr;
    CHECK_MEM(BOOLEAN, 0)

     rtfs_set_errno(0);  /* pc_get_media_parms: clear error status */
    /* Make sure its a valid drive number */
    driveno = pc_parse_raw_drive(path);
    if (driveno < 0)
    {
inval:
        rtfs_set_errno(PEINVALIDDRIVEID); /* pc_get_media_parms: invlid drive ID */
        return(FALSE);
    }
    pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr || !(pdr->drive_flags&DRIVE_FLAGS_VALID) )
        goto inval;

    /* Ask - for device geometry */
    if (pdr->dev_table_perform_device_ioctl(driveno, DEVCTL_GET_GEOMETRY, (void *) pgeometry))
    {
        rtfs_set_errno(PEDEVICEFAILURE); /* pc_get_media_parms: device get geometry failed */
        return (FALSE);
    }

    /* The geometry consists of cylinders, heads, and sectors
         per track.
       The ATA spec says that the maximum values for these are 
         65536 cylinders, 16 heads, and 255 sectors per track.
       However, the MBR spec says the maximum values are
         1023 cylinders, 255 heads, and 63 sectors per track.
       So, we create new values for cylinders, heads, and sectors
         per track that work for the MBR but come as close as we
         can get to the real disk size.
    */
    total = pgeometry->dev_geometry_lbas ? pgeometry->dev_geometry_lbas : (pgeometry->dev_geometry_cylinders * pgeometry->dev_geometry_heads * pgeometry->dev_geometry_secptrack);
    pc_calculate_chs(total, &pgeometry->dev_geometry_cylinders, &pgeometry->dev_geometry_heads, &pgeometry->dev_geometry_secptrack);

    return(TRUE);
}

/***************************************************************************
    PC_FORMAT_MEDIA -  Device level format

Description
    This routine performs a device level format on the specified
    drive.

Returns
    Returns TRUE if it was able to perform the operation otherwise
    it returns FALSE.

    Note: The the logical drive must be claimed before this routine is 
    called and later released by the caller.
    
    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PEDEVICEFAILURE - Device driver format request failed
****************************************************************************/

BOOLEAN pc_format_media(byte *path, PDEV_GEOMETRY pgeometry) /* __apifn__*/
{
int driveno;
DDRIVE *pdr;

    CHECK_MEM(BOOLEAN, 0)

    rtfs_set_errno(0); /* pc_format_media: clear error status */
    /* Make sure its a valid drive number */
    driveno = pc_parse_raw_drive(path);
    if (driveno < 0)
    {
inval:
        rtfs_set_errno(PEINVALIDDRIVEID); /* pc_format_media: invalid drive id */
        return(FALSE);
    }

    pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr || !(pdr->drive_flags&DRIVE_FLAGS_VALID) )
    {
        goto inval;
    }

    /* Make sure that is it closed up */
    pc_dskfree(driveno);

    /* Format the device geometry */
    if (pdr->dev_table_perform_device_ioctl(driveno, DEVCTL_FORMAT, (void *) pgeometry))
    {
        rtfs_set_errno(PEDEVICEFAILURE); /* pc_format_media: driver format failed */
        return(FALSE);
    }
    return(TRUE);
}


/***************************************************************************
    PC_PARTITION_MEDIA -  Write partition table

Description
    A partition list is provided (a list of partition sizes
    in units of LBA s) by the user. 

Returns
    Returns TRUE if it was able to perform the operation otherwise
    it returns FALSE.

    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PEINVALIDPARMS  - Inconsistent or missing parameters
    PEIOERRORWRITE  - Error writing partition table
    An ERTFS system error 
****************************************************************************/

BOOLEAN pc_partition_media(byte *path, PDEV_GEOMETRY pgeometry, dword * partition_list) /* __apifn__*/
{
int driveno;
BOOLEAN ret_val;
BLKBUFF *buf;
byte *  pbuf;
dword   partition_size;
int     partition_number;
int     nibs_per_entry;
PTABLE  part;
DDRIVE *pdr;
int root_entries;
int secpalloc;
/*word secpfat;*/
int root_sectors;
word utemp;
word utemp2;
dword   dwTemp;
dword   starting_lba;
word    cyl;
byte    head,sec;
dword   disk_size;
word    starting_cylinder;


    CHECK_MEM(BOOLEAN, 0)
    ret_val = FALSE;
    rtfs_set_errno(0); /* pc_partition_media: clear error status */
    /* Make sure it s a valid drive number */
    driveno = pc_parse_raw_drive(path);
    if (driveno < 0)
    {
inval:
        rtfs_set_errno(PEINVALIDDRIVEID);
        return(ret_val);
    }
    pdr = pc_drno_to_drive_struct(driveno);
    if (!pdr)
        goto inval;

    if (!pgeometry || !partition_list || !partition_list)
    {
        rtfs_set_errno(PEINVALIDPARMS); /* pc_partition_media: bad arguments */
        return(ret_val);
    }
    /* Zero the partition table to start */
    rtfs_memset(&part, 0, sizeof(part));
    starting_cylinder = 0;

/* 10-24-2000 - New code to support lba formatting */
    /* The first partition starts at cylinder=0, head=1, sector=1 */
    if (pgeometry->dev_geometry_lbas)
        starting_lba = (dword) pgeometry->dev_geometry_secptrack; 
    else
    {
        starting_lba = 0; /* Not used */
/* 10-24-2000 - This was the original code */
        disk_size = (dword) pgeometry->dev_geometry_cylinders;
        disk_size = disk_size * (dword) pgeometry->dev_geometry_heads;
        disk_size = disk_size * (dword) pgeometry->dev_geometry_secptrack;

        /* The first partition starts at 1 */
        starting_cylinder = 1;
    }


    for (partition_number = 0; partition_number < 4; partition_number++)
    {
        partition_size = (dword) *(partition_list+partition_number);
        if (!partition_size)
            break;  /* End of list */
        if (pgeometry->dev_geometry_lbas)
        {
/* 10-24-2000 - New code to support lba formatting */
            /* Apparently, we must still align to cylinders even in LBA mode */
            dwTemp = (dword) pgeometry->dev_geometry_heads; 
            dwTemp *= (dword) pgeometry->dev_geometry_secptrack;
            partition_size = ((partition_size / dwTemp) * dwTemp) - (starting_lba % dwTemp); 
        }
        else
        {
/* 10-24-2000 - This was the original code */
            /* Multiply time # heads and secptrack */
            partition_size = partition_size * (dword) pgeometry->dev_geometry_heads;
            partition_size = partition_size * (dword) pgeometry->dev_geometry_secptrack;
        }
        /* Look up root_entries and cluster size */
        get_format_parameters(partition_size, &secpalloc, &root_entries);
        if( secpalloc == -1 )
        {
            /* Media capacity is too large */
            rtfs_set_errno(PEDEVICEUNKNOWNMEDIA);
            return( FALSE );
        }
        root_sectors = (int) (root_entries/16);

        /* Calculate sectors per fat */
        /*secpfat = */pc_fat_size(  (word)1 /* reserved */, (word)secpalloc,
                            (word)2 /*numfats*/, (word)root_sectors  /* root sectors */,
                            partition_size, &nibs_per_entry);

        /* Now fill in the partition entry 0   */
        part.ents[partition_number].boot =   0x80;   /* Set this to 0x80  for bootable */
        if (pgeometry->dev_geometry_lbas)
        {
/* 10-24-2000 - New code to support lba formatting */
            /* Do CHS */
            /* SECTOR */
            dwTemp = (starting_lba % pgeometry->dev_geometry_secptrack) + 1;
            sec = (byte) dwTemp;
            /* HEAD */
            dwTemp = starting_lba / pgeometry->dev_geometry_secptrack;
            dwTemp = dwTemp % pgeometry->dev_geometry_heads;
            head = (byte) dwTemp;
            /* CYLINDER */
            dwTemp = starting_lba / pgeometry->dev_geometry_secptrack;
            dwTemp = dwTemp / pgeometry->dev_geometry_heads;
            if (dwTemp > 1023)
                dwTemp = 1023;
            cyl = (word) dwTemp;

            /* Load the starting CHS */
            part.ents[partition_number].s_head = head;
            utemp = (word)((cyl & 0xff) << 8);   /* Low 8 bit to hi bite */
            utemp2 = (word)((cyl >> 2) & 0xc0);  /* Hi 2 bits to bits 6 + 7 */ 
            utemp |= utemp2;
            utemp |= sec;
            fr_WORD((byte *)&(part.ents[partition_number].s_cyl), utemp);

            /* Load the starting LBA */
            fr_DWORD((byte *)&(part.ents[partition_number].r_sec), starting_lba); 
        }
        else
        {
/* 10-24-2000 - This was the original code */
            part.ents[partition_number].s_head = 1;    /* Start at head 1 */
            fr_WORD((byte *) &(part.ents[partition_number].s_cyl), (word)starting_cylinder);
        }

        if (partition_size > (dword) 0xffff)
        {
            if (nibs_per_entry == 8)
            {
/* 10-24-2000 - New code to support lba formatting */
            if (pgeometry->dev_geometry_lbas)
                part.ents[partition_number].p_typ = 0x0c;    /* DOS 32 bit LBA mode (was: 0x0b) */
            else
/* 10-24-2000 - This was the original code */
                part.ents[partition_number].p_typ = 0x0b;    /* DOS 32 bit */
            }
            else
                part.ents[partition_number].p_typ = 6;             /* Huge */
        }
        else
        {
            if (nibs_per_entry == 4)
                part.ents[partition_number].p_typ = 4;             /* DOS 16 bit */
            else
                part.ents[partition_number].p_typ = 1;             /* DOS 12 bit */
        }
        if (pgeometry->dev_geometry_lbas)
        {
/* 10-24-2000 - New code to support lba formatting */
            /* Load the LBA size of the partition */
            fr_DWORD((byte *)&(part.ents[partition_number].p_size), partition_size);

            /* Advance the starting_lba */
            starting_lba += partition_size;

            /* Do CHS */
            /* SECTOR */
            dwTemp = ((starting_lba-1) % pgeometry->dev_geometry_secptrack) + 1;
            sec = (byte) dwTemp;
            /* HEAD */
            dwTemp = (starting_lba-1) / pgeometry->dev_geometry_secptrack;
            dwTemp = dwTemp % pgeometry->dev_geometry_heads;
            head = (byte) dwTemp;
            /* CYLINDER */
            dwTemp = (starting_lba-1) / pgeometry->dev_geometry_secptrack;
            dwTemp = dwTemp / pgeometry->dev_geometry_heads;
            if (dwTemp > 1023)
                dwTemp = 1023;
            cyl = (word) dwTemp;

            /* Load the ending CHS */
            part.ents[partition_number].e_head = head;
            utemp = (word)((cyl & 0xff) << 8);   /* Low 8 bit to hi bite */
            utemp2 = (word)((cyl >> 2) & 0xc0);  /* Hi 2 bits to bits 6 + 7 */ 
            utemp |= utemp2;
            utemp |= sec;
            fr_WORD((byte *)&(part.ents[partition_number].e_cyl), utemp);
        }
        else
        {
/* 10-24-2000 - This was the original code */
            /* Ending head is NHEADS-1 since heads go from 0 to n     */
            part.ents[partition_number].e_head = (byte) (pgeometry->dev_geometry_heads-1);
            /* Ending cylinder is in the top ten bits, secptrack in lower 6 ??     */
            /* Relative sector starting */
            fr_DWORD((byte *)&(part.ents[partition_number].r_sec), (dword) pgeometry->dev_geometry_secptrack*starting_cylinder); 

            /* Set up for the next partition and use new value to calculate ending cyl */
            starting_cylinder = (word)((word)starting_cylinder + (word) *(partition_list+partition_number));
            utemp = (word)(((starting_cylinder-1) & 0xff) << 8);   /* Low 8 bit to hi bite */
            utemp2 = (word)(((starting_cylinder-1) >> 2) & 0xc0);  /* Hi 2 bits to bits 6 + 7 */ 
            utemp |= utemp2;
            utemp |= (word) pgeometry->dev_geometry_secptrack;
 
            fr_WORD((byte *)&(part.ents[partition_number].e_cyl), utemp);
            /* And partition size   */
            fr_DWORD((byte *)&(part.ents[partition_number].p_size), partition_size);
        }

    } /* for (partition_number = 0; partition_number < 4;.. */

    /* Now for the signature   */
    fr_WORD((byte *)&(part.signature), 0xAA55);
    /* Grab some working space   */
    buf = pc_scratch_blk();
    if (!buf)
    {
        /* pc_scratch_blk set errno */
        return(ret_val);
    }

    pbuf = buf->data;
    rtfs_memset(pbuf, 0, 512);
    /* Now copy the partition into a block   */
    /* The info starts at buf[1be]           */
    /* Don't use sizeof here since the structure does not pack to exact size */
    copybuff((pbuf + 0x1be), &part, 0x42);
    /* try the write   */
    if (!devio_write(driveno, 0, pbuf, 1, TRUE))
    {
        if (!get_errno())
            rtfs_set_errno(PEIOERRORWRITE);  /* pc_partition_media: write failed */
    }
    else
        ret_val = TRUE;

    pc_free_scratch_blk(buf);
    return(ret_val);
}


/***************************************************************************
    PC_FORMAT_VOLUME -  Format a volume

Description
    This routine formats the volume referred to by drive letter.
    drive structure is queried to determine if the device is parttioned
    or not. If the device is partitioned then the partition table is read
    and the volume within the partition is formatted. If it is a non
    partitioned device the device is formatted according to the supplied 
    pgeometry parameters. The pgeometry parameter contains the the media
    size in HCN format. It also contains a 

    Note: The the logical drive must be claimed before this routine is 
    called and later released by the caller.

Returns
    Returns TRUE if it was able to perform the operation otherwise
    it returns FALSE.
    
    errno is set to one of the following
    0               - No error
    PEINVALIDDRIVEID- Drive component is invalid
    PEIOERRORREADMBR- Partitioned device. IO error reading
    PEINVALIDMBR    - Partitioned device has no master boot record
    PEINVALIDMBROFFSET - Requested partition has no entry in master boot record
    PEINVALIDPARMS  - Inconsistent or missing parameters
    PEIOERRORWRITE  - Error writing during format 
    An ERTFS system error 
****************************************************************************/

BOOLEAN pc_format_volume(byte *path, PDEV_GEOMETRY pgeometry) /* __apifn__*/
{
DDRIVE *pdr;
BOOLEAN raw_mode_io = TRUE;
FMTPARMS fmt;
int driveno;
dword partition_size = 0;
dword ltemp;
word n_cyls = 0;
int root_entries;
int secpalloc;
word secpfat;
int root_sectors;
int     nibs_per_entry,partition_status;

    CHECK_MEM(BOOLEAN, 0)
    rtfs_set_errno(0); /* pc_format_volume: clear error status */
    /* Make sure it s a valid drive number */
    driveno = pc_parse_raw_drive(path);
    if (driveno < 0 || !pgeometry)
    {
        rtfs_set_errno(PEINVALIDDRIVEID); /* pc_format_volume: bad arguments */
        return(FALSE);
    }
    pdr = pc_drno_to_drive_struct(driveno);

    /* Check the drive structure for format strategy */
    if (!pdr || !(pdr->drive_flags&DRIVE_FLAGS_VALID))
    {
        rtfs_set_errno(PEINVALIDDRIVEID); /* pc_format_volume: bad arguments */
        return(FALSE);
    }
    /* Make sure all is flushed */
    pc_dskfree(driveno);

    /* If the format parms were provided by the driver format now */
    if (pgeometry->fmt_parms_valid)
    {
        if( ((pgeometry->fmt.numcyl * pgeometry->fmt.numhead * pgeometry->fmt.secptrk) / pgeometry->fmt.secpalloc) > 0xFFFF) {
	        return(pc_mkfs32(driveno, &pgeometry->fmt, TRUE)); /* TRUE == RAW IO */	//ctr modified
        }else{
	        return(pc_mkfs16(driveno, &pgeometry->fmt, TRUE)); /* TRUE == RAW IO */
        }
    }

    /* If the device is partitioned read the partition table into the
       drive structure.
    */
    partition_status = 0;
    if (pdr->drive_flags & DRIVE_FLAGS_PARTITIONED)
    {
        partition_status = pc_read_partition_table(driveno, pdr);
        if (partition_status == READ_PARTION_OK)
        {
            partition_size = pdr->partition_size;
            ltemp = partition_size / 
                (dword)(pgeometry->dev_geometry_heads * pgeometry->dev_geometry_secptrack);
            n_cyls = (word) ltemp;
            raw_mode_io = FALSE;
        }
        else if (partition_status != READ_PARTION_NO_TABLE || pdr->partition_number != 0)
        {
            /* pc_format_volume: errno set to PEDEVICE by pc_read_partition_table */
            return(FALSE);
        }
    }

    if (!(pdr->drive_flags & DRIVE_FLAGS_PARTITIONED) || partition_status == READ_PARTION_NO_TABLE)
    {
        partition_size = (dword) pgeometry->dev_geometry_cylinders;
        partition_size = partition_size * (dword) pgeometry->dev_geometry_heads;
        partition_size = partition_size * (dword) pgeometry->dev_geometry_secptrack;
        n_cyls = (word) pgeometry->dev_geometry_cylinders;
        raw_mode_io = TRUE;
    }

    /* Look up root_entries and cluster size */
    get_format_parameters(partition_size, &secpalloc, &root_entries);
    if( secpalloc == -1 )
    {
        /* Media capacity is too large */
        rtfs_set_errno(PEDEVICEUNKNOWNMEDIA);
        return( FALSE );
    }
    root_sectors = (int) (root_entries/16);

   /* Calculate sectors per fat */
    secpfat = pc_fat_size(  (word)1 /* reserved */, (word)secpalloc,
                           (word)2 /*numfats*/, (word)root_sectors  /* root sectors */,
                           partition_size, &nibs_per_entry);

    rtfs_strcpy(&fmt.oemname[0],
                rtfs_strtab_user_string(USTRING_SYS_OEMNAME) );
    fmt.physical_drive_no =  (byte) driveno;
    fmt.binary_volume_label = BIN_VOL_LABEL;
    rtfs_strcpy(fmt.text_volume_label,
                rtfs_strtab_user_string(USTRING_SYS_VOLUME_LABEL) );


    fmt.secpalloc =      (byte)  secpalloc;
    fmt.numfats     =    (byte)  2;
    fmt.secptrk     =    (word) pgeometry->dev_geometry_secptrack;
    fmt.numhead     =    (word) pgeometry->dev_geometry_heads;
    fmt.numcyl     =     (word) n_cyls;
    if (nibs_per_entry == 8) /* FAT32 Format */
    {
        fmt.secreserved =    (word) 32;
        if (pdr->drive_flags & DRIVE_FLAGS_PARTITIONED)
        {
            if (pgeometry->dev_geometry_lbas)
                /* 10-24-2000 - New code to support lba formatting */
                fmt.numhide     =    (unsigned long)  0; /*PS Does not work as fmt.secptrk here */
            else
                /* 10-24-2000 - This was the original code */
                fmt.numhide     =    (unsigned long)  fmt.secptrk;
        }
        else
            fmt.numhide     =    (unsigned long)  pgeometry->fmt.numhide;	//ctr modified
        fmt.secpfat     =    (word) 0;
        fmt.numroot     =    (word) 0;
        fmt.mediadesc   =    (byte)  0xF8;
        return(pc_mkfs32(driveno, &fmt, raw_mode_io));
    }
    else
    {
        fmt.secreserved =    (word) 1;
        fmt.numhide     =    pgeometry->fmt.numhide;	//ctr modified
        fmt.secpfat     =    (word) secpfat;
        fmt.numroot     =    (word) root_entries;
        fmt.mediadesc   =    (byte)  0xF8;
        return(pc_mkfs16(driveno, &fmt, raw_mode_io));
    }
}


/*****************************************************************************
    PC_MKFS  -  Make a file system on a disk MS-DOS 4.0 version

 Description
    Given a drive number and a format parameter block. Put an MS-DOS
    file system on the drive:
    The disk MUST already have a low level format. All blocks on the drive
    should be intitialize with E5s or zeros. 
    
    see pcmkfs in the samples directory.

    Some common parameters. Note: For other drive types use debug to get the
    parameters from block zero after FORMAT has been run. 

            360     720     20M    80M          (DRIVE SIZE)
oemname     =====  UP TO YOU. ONLY 8 Chars matter. Right filled with spaces 
secpalloc   2       2       4       8
secreserved 1       1       1       1
numfats     2       2       2       2
numroot     0x70    0x70    0x200   0x200
mediadesc   0xFD    0xF9    0xF8    0xF8
secpfat     2       3       44      88
secptrk     9       9       0x11    0x11
numhead     2       2       4       8
numcyl      40      80      612     1224

    Note: If pc_mkfs is called secpfat equal zero, secpfat will be calculated
          internally.

Returns
    Returns TRUE if the filesystem disk was successfully initialized.
See Also:
    pcmkfs.c format utility program
****************************************************************************/


word pc_fat_size(word nreserved, word cluster_size, word n_fat_copies,
                 word root_sectors, dword volume_size, 
                 int *nibs_per_entry)   /*__fn__*/
{

    dword  fat_size;
    dword  total_clusters;
    word entries_per_block;
    
#if (FAT32)
    if ((root_sectors == 0) || ((volume_size>>11) >= 512))/* FAT32 Format */
    {
        fat_size = (volume_size + 128*cluster_size) / (128*cluster_size + 1);
        *nibs_per_entry = 8;
        return ((word)fat_size);
    }
#endif

    /* Calulate total cluster size. Assuming zero size fat:
        We round up to the nearest cluster boundary */
    total_clusters = volume_size - nreserved - root_sectors;
    total_clusters /= cluster_size;

    /* Calculate the number of fat entries per block in the FAT. If
        < 4087 clusters total the fat entries are 12 bits hence 341 
        will fit. else 256 will fit
        we add in n_fat_copies * 12 here since it take 12 blocks to represent
        4087 clusters in 3 nibble form. So we add in the worst case FAT size
        here to enhance the accuracy of our guess of the total clusters.
    */  

    if (total_clusters <= (dword) (4087 + (n_fat_copies * 12)) )
    {
        entries_per_block = 341;
        *nibs_per_entry = 3;
    }
    else
    {
        entries_per_block = 256;
        *nibs_per_entry = 4;
    }

    fat_size = (total_clusters + entries_per_block - 1)/entries_per_block;

    return((word) fat_size);    
}

/* Choose format parameters based on the number of blocks in the volume */
void get_format_parameters(dword nblocks, int *psectors_per_alloc, int *pnum_root_entries)
{
    int sectors_per_alloc;
    int num_root_entries;

#if (FAT32)
    /* FAT12 */
    if (nblocks <= 1000) /* <= .5 MEG */
        {sectors_per_alloc = 1; num_root_entries = 32;}
    else if (nblocks <= 10240) /* <= 5 MEG */
        {sectors_per_alloc = 8; num_root_entries = 512;}
    else if (nblocks <= 32768) /* <= 16 MEG */
        {sectors_per_alloc = 4; num_root_entries = 512;}

    /* FAT16 */
    else if (nblocks <= 262144) /* <= 128 MEG */
        {sectors_per_alloc = 4; num_root_entries = 512;}
    else if (nblocks <= 524288) /* <= 256 MEG */
        {sectors_per_alloc = 8; num_root_entries = 512;}
    else if (nblocks <= 1048576) /* <= 512 MEG */
        {sectors_per_alloc = 16; num_root_entries = 512;}

    /* FAT32 */
    else if (nblocks <= 16777216) /* <= 8 GIG */
        {sectors_per_alloc = 8; num_root_entries = 512;}
    else if (nblocks <= 33554432) /* <= 16 GIG */
        {sectors_per_alloc = 16; num_root_entries = 512;}
    else if (nblocks <= 67108864) /* <= 32 GIG */
        {sectors_per_alloc = 32; num_root_entries = 512;}
    else /* > 32 GIG */
        {sectors_per_alloc = 64; num_root_entries = 512;}
#else
    /* FAT12 */
    if (nblocks <= 1000) /* <= .5 MEG */
        {sectors_per_alloc = 1; num_root_entries = 32;}
    else if (nblocks <= 10240) /* <= 5 MEG */
        {sectors_per_alloc = 8; num_root_entries = 512;}
    else if (nblocks <= 32768) /* <= 16 MEG */
        {sectors_per_alloc = 4; num_root_entries = 512;}

    /* FAT16 */
    else if (nblocks <= 262144) /* <= 128 MEG */
        {sectors_per_alloc = 4; num_root_entries = 512;}
    else if (nblocks <= 524288) /* <= 256 MEG */
        {sectors_per_alloc = 8; num_root_entries = 512;}
    else if (nblocks <= 1048576) /* <= 512 MEG */
        {sectors_per_alloc = 16; num_root_entries = 512;}
    else if (nblocks <= 2097152) /* <= 1 GIG */
        {sectors_per_alloc = 32; num_root_entries = 512;}
    else if (nblocks <= 4194304) /* <= 2 GIG */
        {sectors_per_alloc = 64; num_root_entries = 512;}
    else /* error */
        {sectors_per_alloc = -1; num_root_entries = -1;}
#endif /* (FAT32) */

    *psectors_per_alloc = sectors_per_alloc;
    *pnum_root_entries = num_root_entries;
}


