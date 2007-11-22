/*
* rtkernfn.c - Miscelaneous portable functions
*
* ERTFS portable process management and other functions.
* This file is portable but it requires interaction with the
* porting layer functions in portrtfs.c.
*
*   Copyright EBS Inc. 1987-2003
*   All rights reserved.
*   This code may not be redistributed in source or linkable object form
*   without the consent of its author.
*
*/

#include <rtfs.h>


BOOLEAN rtfs_resource_init(void)   /*__fn__*/
{

    prtfs_cfg->critical_semaphore = rtfs_port_alloc_mutex();
    if (!prtfs_cfg->critical_semaphore)
        return(FALSE);

/* Note: cfg_NDRIVES semaphores are allocated and assigned to the individual
   drive structure within routine pc_ertfs_init() */

    /* Initialize a user table */
    rtfs_memset((byte *)prtfs_cfg->rtfs_user_table, 0, sizeof(RTFS_SYSTEM_USER)*prtfs_cfg->cfg_NUM_USERS);

    return(TRUE);
}

PRTFS_SYSTEM_USER rtfs_get_system_user(void)
{
int i,j;
dword t;

    t = rtfs_port_get_taskid();
    if (prtfs_cfg->cfg_NUM_USERS == 1)
        return(&prtfs_cfg->rtfs_user_table[0]);

    for (i = 0; i < prtfs_cfg->cfg_NUM_USERS; i++)
    {
        if (t == prtfs_cfg->rtfs_user_table[i].task_handle)
            return(&prtfs_cfg->rtfs_user_table[i]);
    }
    /* Did not find one.. so assign one from between 1 and n */
    for (i = 1; i < prtfs_cfg->cfg_NUM_USERS; i++)
    {
        if (!prtfs_cfg->rtfs_user_table[i].task_handle)
        {
return_it:
            rtfs_memset((byte *)&prtfs_cfg->rtfs_user_table[i], 0, sizeof(RTFS_SYSTEM_USER));
            prtfs_cfg->rtfs_user_table[i].task_handle = t;
            return(&prtfs_cfg->rtfs_user_table[i]);
        }
    }
    /* We are out of user structures so use element 0 */
    i = 0;
    /*  Bug fix 02-01-2007 - If we are using the default user (0), make sure the
        current working directory objects are freed and the finode access counts
        are reduced */
    for(j = 0; j < prtfs_cfg->cfg_NDRIVES; j++)
    {
        if(prtfs_cfg->rtfs_user_table[i].lcwd[j])
        {
            pc_freeobj((DROBJ *) prtfs_cfg->rtfs_user_table[i].lcwd[j]);
            prtfs_cfg->rtfs_user_table[i].lcwd[j] = 0;
        }
    }
    goto return_it;
}

void  pc_free_user(void)                            /*__fn__*/
{
int i;
PRTFS_SYSTEM_USER s;
    s = rtfs_get_system_user();
    if (s)
    {
        for (i = 0; i < prtfs_cfg->cfg_NDRIVES; i++)
        {
            if (s->lcwd[i])
            {
                pc_freeobj((DROBJ *) s->lcwd[i]);
                s->lcwd[i] = 0;
            }
        }
        rtfs_memset((byte *)s, 0, sizeof(*s));
    }
}

/* pc_free_all_users() - Run down the user list releasing drive resources
 *
 * This routine is called by RTFS when it closes a drive.
 * The routine must release the current directory object for that drive
 * for each user. If a user does not have a CWD for the drive it should
 * not call pc_freeobj.
 *
 * In the reference port we cycle through our array of user structures
 * to provide the enumeration. Other implementations are equally valid.
 */

void  pc_free_all_users(int driveno)                            /*__fn__*/
{
int i;
    if (prtfs_cfg->cfg_NUM_USERS == 1)
    {
        if (prtfs_cfg->rtfs_user_table[0].lcwd[driveno])
        {
            pc_freeobj((DROBJ *)prtfs_cfg->rtfs_user_table[0].lcwd[driveno]);
            prtfs_cfg->rtfs_user_table[0].lcwd[driveno] = 0;
        }
    }
    else
    {
        for (i = 0; i < prtfs_cfg->cfg_NUM_USERS; i++)
        {
            if (prtfs_cfg->rtfs_user_table[i].task_handle && prtfs_cfg->rtfs_user_table[i].lcwd[driveno])
            {
                pc_freeobj((DROBJ *)prtfs_cfg->rtfs_user_table[i].lcwd[driveno]);
                prtfs_cfg->rtfs_user_table[i].lcwd[driveno] = 0;
            }
        }
    }
}


/* int rtfs_set_driver_errno() - set device driver errno for the calling task

   Saves driver errno for the calling task in array based on callers taskid.

   Note: This routine must not be called from the interrupt service layer

   Returns nothing
*/
void rtfs_set_driver_errno(dword error)    /*__fn__*/
{
    rtfs_get_system_user()->rtfs_driver_errno = error;
}


/* ********************************************************************

dword rtfs_get_driver_errno() - get device driver errno for the calling task

  Returns device driver errno for the calling task in array based on
  callers taskid.
*/

dword rtfs_get_driver_errno(void)    /*__fn__*/
{
    return(rtfs_get_system_user()->rtfs_driver_errno);
}


/* int rtfs_set_errno() - set errno for the calling task

   Saves errno for the calling task in array based on callers taskid.

   Returns -1
*/
int rtfs_set_errno(int error)    /*__fn__*/
{
    rtfs_get_system_user()->rtfs_errno = error;
    return(-1);
}


/* ********************************************************************

int get_errno() - get errno for the calling task

  Returns errno for the calling task in array based on callers taskid.
*/

int get_errno(void)    /*__fn__*/
{
    return(rtfs_get_system_user()->rtfs_errno);
}

/* Miscelaneous functions */

void pc_report_error(int error_number)          /*__fn__*/
{
    RTFS_PRINT_STRING_1(USTRING_PORTKERN_03, 0); /* "pc_report_error was called with error" */
    RTFS_PRINT_LONG_1((dword) error_number, PRFLG_NL);
}

/* This routine will be called if an IO error occurs. It must return
   either CRITICAL_ERROR_ABORT to have the operation aborted and
   the drive to be unmounted or CRITICAL_ERROR_RETRY to force a retry
   of the operation.

   This routine prompts the user to Abort or Retry. If console input
   is not implemented it return Abort. This routine may be modified
   to take corrective action (such as ask the user to reinsert the media)
   before return Retry or Abort
*/

KS_CONSTANT int med_st[] =
{
    USTRING_SYS_NULL, /* ""                  */
    USTRING_CRITERR_02, /* "BAD_FORMAT"         */
    USTRING_CRITERR_03, /* "CRERR_NO_CARD"      */
    USTRING_CRITERR_04, /* "CRERR_BAD_CARD"     */
    USTRING_CRITERR_05, /* "CRERR_CHANGED_CARD" */
    USTRING_CRITERR_06, /* "CRERR_CARD_FAILURE"  */
};

int  critical_error_handler(int driveno, int media_status, dword sector)
{
byte inbuf[20];
byte *p;
DDRIVE *pdr;
BOOLEAN needs_flush;

/* If you do not have console input you may return abort unconditionally */
/*  return (CRITICAL_ERROR_ABORT); */

    RTFS_ARGSUSED_PVOID((void *) sector);
    pdr = pc_drno_to_drive_struct(driveno);

    if (pdr->fat_is_dirty || pc_test_all_fil(pdr))
        needs_flush = TRUE;
    else
        needs_flush = FALSE;

    RTFS_PRINT_STRING_1(USTRING_CRITERR_07,0); /* "Media status == " */
    RTFS_PRINT_STRING_1(med_st[media_status],PRFLG_NL);
    RTFS_PRINT_STRING_2(USTRING_CRITERR_08, pdr->volume_label, PRFLG_NL); /* "Volume == " */

    if (needs_flush)
        RTFS_PRINT_STRING_1(USTRING_CRITERR_09, PRFLG_NL); /* "Volume is dirty" */
    else
        RTFS_PRINT_STRING_1(USTRING_CRITERR_10, PRFLG_NL); /* "Volume is clean" */

    /* default to ABORT. If rtfs_print_prompt_user() is not implemented
       then ABORT will be the default behavior */
    p = inbuf;
    CS_OP_ASSIGN_ASCII(p,'A');
    CS_OP_INC_PTR(p);
    CS_OP_TERM_STRING(p);
    return( CRITICAL_ERROR_ABORT); /* ctr modified */
//    for (;;)
//    {
//        /* "Type A to abort R to Retry" */
//        rtfs_print_prompt_user(UPROMPT_CRITERR, inbuf);
//        if (CS_OP_CMP_ASCII(inbuf,'A') || CS_OP_CMP_ASCII(inbuf,'a'))
//            return (CRITICAL_ERROR_ABORT);
//        if (CS_OP_CMP_ASCII(inbuf,'R') || CS_OP_CMP_ASCII(inbuf,'r'))
//            return (CRITICAL_ERROR_RETRY);
//    }
}

/**************************************************************************
    PC_NUM_DRIVES -  Return total number of drives in the system

 Description
    This routine returns the number of drives in the system

 Returns
    The number
*****************************************************************************/
int pc_num_drives(void)                                 /* __fn__ */
{
    return(prtfs_cfg->cfg_NDRIVES);
}

/**************************************************************************
    PC_NUSERFILES -  Return total number of uses allowed in the system

 Description
    This routine returns the number of user in the system

 Returns
    The number
*****************************************************************************/
int pc_num_users(void)                                  /* __fn__ */
{
    return(prtfs_cfg->cfg_NUM_USERS);
}

/**************************************************************************
    PC_NUSERFILES -  Return total number of userfiles alloed in the system

 Description
    This routine returns the number of user files in the system

 Returns
    The number
*****************************************************************************/

int pc_nuserfiles(void)                                 /* __fn__ */
{
    return(prtfs_cfg->cfg_NUSERFILES);
}

/**************************************************************************
    PC_VALIDATE_DRIVENO -  Verify that a drive number is <= prtfs_cfg->cfg_NDRIVES

 Description
    This routine is called when a routine is handed a drive number and
    needs to know if it is within the number of drives set during
    the congiguration.

 Returns
    TRUE if the drive number is valid or FALSE.
*****************************************************************************/

BOOLEAN pc_validate_driveno(int driveno)                            /* __fn__ */
{
    if ((driveno < 0) || (driveno > 25) || (!prtfs_cfg->drno_to_dr_map[driveno]) )
        return(FALSE);
    else
        return(TRUE);
}

/**************************************************************************
    PC_MEMORY_INIT -  Initialize and allocate File system structures.

    THIS ROUTINE MUST BE CALLED BEFORE ANY FILE SYSTEM ROUTINES !!!!!!
    IT IS CALLED BY THE PC_ERTFS_INIT() Function

 Description
    This routine must be called before any file system routines. Its job
    is to allocate tables needed by the file system. We chose to implement
    memory management this way to provide maximum flexibility for embedded
    system developers. In the reference port we use malloc to allocate the
    various chunks of memory we need, but we could just have easily comiled
    the tables into the BSS section of the program.

    Use whatever method makes sense in you system.

    Note the total number of bytes allocated by this routine is:
        (sizeof(DDRIVE) * prtfs_cfg->cfg_NDRIVES) + (sizeof(PC_FILE)*NUSERFILES) +
        (sizeof(BLKBUFF)*NBLKBUFFS)+ (sizeof(DROBJ)*NDROBJS) +
        (sizeof(FINODE)*NFINODES)


 Returns
    TRUE on success or no ON Failure.
*****************************************************************************/
BOOLEAN pc_memory_init(void)                                                /*__fn__*/
{
    int i,j,l;
    DROBJ *pobj;
    FINODE *pfi;
    DDRIVE *pdrive;

    /* Call the kernel level initialization code */
    if (!rtfs_resource_init())
        return(FALSE);
/*
    =!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=
    We simply assign our pointers to the placeholders in
    the BSS
    =!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=
 */
    /* Initialize the drive structures */
    l =sizeof(DDRIVE); l *= prtfs_cfg->cfg_NDRIVES;
    rtfs_memset(prtfs_cfg->mem_drives_structures, (byte) 0, (int)l);

    /* Initialize the directory block buffer array */
    if (!pc_initialize_block_pool(
        &prtfs_cfg->buffcntxt, prtfs_cfg->cfg_NBLKBUFFS,
        prtfs_cfg->mem_block_pool,  prtfs_cfg->cfg_BLK_HASHTBLE_SIZE,
        prtfs_cfg->mem_block_hash_table))
        return(FALSE);

    /* Initialize buffer pools for each drive */
    for (pdrive=prtfs_cfg->mem_drives_structures,i = 0;
        i < prtfs_cfg->cfg_NDRIVES; i++, pdrive++)
    {
        /* Use the globally shared block buffer pool.
            this can be overriden through the API to assign a private buffer pool to the drive */
        pdrive->pbuffcntxt = &prtfs_cfg->buffcntxt;

        /* Initialize the fat block buffer array */
        if (!pc_initialize_fat_block_pool(
            &pdrive->fatcontext,
            prtfs_cfg->cfg_FAT_BUFFER_SIZE[i], prtfs_cfg->fat_buffers[i],
            prtfs_cfg->cfg_FAT_HASHTBL_SIZE[i], prtfs_cfg->fat_hash_table[i],
            prtfs_cfg->fat_primary_cache[i], prtfs_cfg->fat_primary_index[i]))
            return(FALSE);

    }
    /* make a NULL terminated freelist of the DROBJ pool using
        pdrive as the link. This linked freelist structure is used by the
        DROBJ memory allocator routine. */
    pobj = prtfs_cfg->mem_drobj_freelist = prtfs_cfg->mem_drobj_pool;
    pobj->is_free = TRUE;
    for (i = 0,j = 1; i < prtfs_cfg->cfg_NDROBJS-1; i++, j++)
    {
        pobj = prtfs_cfg->mem_drobj_freelist + j;
        pobj->is_free = TRUE;
        prtfs_cfg->mem_drobj_freelist[i].pdrive = (DDRIVE *) pobj;
    }
    prtfs_cfg->mem_drobj_freelist[prtfs_cfg->cfg_NDROBJS-1].pdrive = 0;

    /* Make a NULL terminated FINODE freelist using
        pnext as the link. This linked freelist is used by the FINODE
        memory allocator routine */
    pfi = prtfs_cfg->mem_finode_freelist = prtfs_cfg->mem_finode_pool;
    for (i = 0; i < prtfs_cfg->cfg_NFINODES-1; i++)
    {
        pfi->is_free = TRUE;
        pfi++;
        prtfs_cfg->mem_finode_freelist->pnext = pfi;
        prtfs_cfg->mem_finode_freelist++;
        prtfs_cfg->mem_finode_freelist->pnext = 0;
    }
    /* Mark all user files free */
    for (i = 0; i < prtfs_cfg->cfg_NUSERFILES; i++)
        prtfs_cfg->mem_file_pool[i].is_free = TRUE;

    prtfs_cfg->mem_finode_freelist = prtfs_cfg->mem_finode_pool;

    return(TRUE);
}

/**************************************************************************
    PC_MEMORY_DROBJ -  Allocate a DROBJ structure
 Description
    If called with a null pointer, allocates and zeroes the space needed to
    store a DROBJ structure. If called with a NON-NULL pointer the DROBJ
    structure is returned to the heap.

 Returns
    If an ALLOC returns a valid pointer or NULL if no more core. If a free
    the return value is the input.

*****************************************************************************/

DROBJ *pc_memory_drobj(DROBJ *pobj)                             /*__fn__*/
{
DROBJ *preturn;
    preturn = 0;
    if (pobj)
    {
        OS_CLAIM_FSCRITICAL()
        if (!pobj->is_free)
        {
            pobj->is_free = TRUE;
           /* Free it by putting it at the head of the freelist
                NOTE: pdrive is used to link the freelist */
            pobj->pdrive = (DDRIVE *) prtfs_cfg->mem_drobj_freelist;
            prtfs_cfg->mem_drobj_freelist = pobj;
        }
        OS_RELEASE_FSCRITICAL()
    }
    else
    {
        /* Alloc: return the first structure from the freelist */
        OS_CLAIM_FSCRITICAL()
        preturn =  prtfs_cfg->mem_drobj_freelist;
        if (preturn)
        {
            prtfs_cfg->mem_drobj_freelist = (DROBJ *) preturn->pdrive;
            rtfs_memset(preturn, (byte) 0, sizeof(DROBJ));
            OS_RELEASE_FSCRITICAL()
        }
        else
        {
            OS_RELEASE_FSCRITICAL()
            rtfs_set_errno(PERESOURCE); /* pc_memory_drobj: out drobj of resources */
            pc_report_error(PCERR_DROBJALLOC);
        }
    }
    return(preturn);
}


/**************************************************************************
    PC_MEMORY_FINODE -  Allocate a FINODE structure
 Description
    If called with a null pointer, allocates and zeroes the space needed to
    store a FINODE structure. If called with a NON-NULL pointer the FINODE
    structure is returned to the heap.

 Returns
    If an ALLOC returns a valid pointer or NULL if no more core. If a free
    the return value is the input.

*****************************************************************************/

FINODE *pc_memory_finode(FINODE *pinode)                            /*__fn__*/
{
FINODE *preturn;
BLKBUFF *pfile_buffer;

    if (pinode)
    {
        pfile_buffer = 0;
        OS_CLAIM_FSCRITICAL()
        if (!pinode->is_free)
        {
            /* Free it by putting it at the head of the freelist */
            pfile_buffer = pinode->pfile_buffer;
            pinode->pfile_buffer = 0;
            pinode->is_free = TRUE;
            pinode->pnext = prtfs_cfg->mem_finode_freelist;
            prtfs_cfg->mem_finode_freelist = pinode;
        }
        OS_RELEASE_FSCRITICAL()
        if (pfile_buffer)
            pc_free_scratch_blk(pfile_buffer);
        preturn = pinode;
    }
    else
    {
        /* Alloc: return the first structure from the freelist */
        OS_CLAIM_FSCRITICAL()
        preturn =  prtfs_cfg->mem_finode_freelist;
        if (preturn)
        {
            prtfs_cfg->mem_finode_freelist = preturn->pnext;
            /* Zero the structure */
            rtfs_memset(preturn, (byte) 0, sizeof(FINODE));
            OS_RELEASE_FSCRITICAL()
        }
        else
        {
            OS_RELEASE_FSCRITICAL()
            rtfs_set_errno(PERESOURCE); /* pc_memory_finode: out finode of resources */
            pc_report_error(PCERR_FINODEALLOC);
        }
    }
    return(preturn);
}

