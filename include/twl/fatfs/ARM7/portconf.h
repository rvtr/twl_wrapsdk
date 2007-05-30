/*****************************************************************************
*Filename: PORTCONF.H - RTFS porting layer tuning constants
*
*
* EBS - RTFS (Real Time File Manager)
*
* Copyright EBS Inc, 1993-2003
* All rights reserved.
* This code may not be redistributed in source or linkable object form
* without the consent of its author.
*
* Description:
*   This file contains porting layer tuning constants for configuring RTFS.
*   It is included by rtfsconf.h.
*
****************************************************************************/

#ifndef __PORTCONF__
#define __PORTCONF__ 1

/* CPU Configuration section */

#define KS_LITTLE_ENDIAN 1 /* See porting reference guide for explanation */
#define KS_LITTLE_ODD_PTR_OK 0 /* See porting reference guide for explanation */
#define KS_CONSTANT const /* See porting reference guide for explanation */
#define KS_FAR  /* See porting reference guide for explanation */


/* Compile time constants to control device inclusion
	See the reference guide for an explanation
*/


#define INCLUDE_SD				0
#define INCLUDE_IDE				0 /* - Include the IDE driver */
#define INCLUDE_PCMCIA			0 /* - Include the pcmcia driver */
#define INCLUDE_PCMCIA_SRAM		0 /* - Include the pcmcia static ram card driver */
#define INCLUDE_COMPACT_FLASH	0 /* - Support compact flash (requires IDE and PCMCIA) */
#define INCLUDE_FLASH_FTL		0 /* - Include the linear flash driver */
#define INCLUDE_ROMDISK			0 /* - Include the rom disk driver */
#define INCLUDE_RAMDISK			0 /* - Include the rom disk driver */
#define INCLUDE_MMCCARD			0 /* - Include the multi media flash card driver */
#define INCLUDE_SMARTMEDIA		0 /* - Include the smart media flash card driver */
#define INCLUDE_FLOPPY			0 /* - Include the floppy disk driver */
#define INCLUDE_HOSTDISK		0 /* - Include the host disk disk simulator */
#define INCLUDE_UDMA			0 /* - Include ultra dma support for the ide driver */
#define INCLUDE_82365_PCMCTRL	0 /* - Include the 82365 pcmcia controller driver */


#endif /* __PORTCONF__ */
