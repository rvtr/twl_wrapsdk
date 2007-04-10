#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - mi/ARM9
# File:     Makefile
#
# Copyright 2007 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: $
# $NoKeywords: $
#----------------------------------------------------------------------------

SUBDIRS			=

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL 	?= True

SRCDIR			= ../common .
SRCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/mi/common/src \
                   $(TWL_NITROSDK_ROOT)/build/libraries/mi/ARM9/src \

INCDIR			= ../common
INCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/mi/common/include \

SRCS			= mi_wram.c \
			  mi_dma.c \
			  mi_edma.c \
			  mi_dma_hblank.c \
			  mi_dma_vblank.c \
			  mi_dma_mainmem.c \
			  mi_dma_gxcommand.c \
			  mi_memory.c \
			  mi_swap.c \
			  mi_uncompress.c \
			  mi_byteAccess.c \
			  mi_stream.c \
			  mi_card.c \
			  mi_dma_card.c \
			  mi_uncomp_stream.c \
			  mi_compress.c \
			  mi_exMemory.c \
			  mi_init.c

TARGET_LIB		= libmi$(TWL_LIBSUFFIX).a


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(TWL_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
