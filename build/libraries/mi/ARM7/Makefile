#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - mi/ARM7
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

# Codegen for sub processer
TWL_PROC		= ARM7

SRCDIR			= ../common .
SRCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/mi/common/src \
                   $(TWL_NITROSDK_ROOT)/build/libraries/mi/ARM7/src \

INCDIR			= ../common
INCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/mi/common/include \

SRCS			= \
              mi_dma.c \
              mi_dma_patch.c \
			  mi_exDma.c \
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
			  mi_exMemory.c

TARGET_LIB		= libmi_sp$(TWL_LIBSUFFIX).a


#----------------------------------------------------------------------------

# DEBUG版ビルドの場合、RELEASE版でビルドして
# DEBUG版のライブラリを装います。

ifdef	NITRO_DEBUG
NITRO_BUILD_TYPE	= RELEASE
endif

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(TWL_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
