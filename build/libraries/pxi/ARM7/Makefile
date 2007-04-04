#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - pxi
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

TWL_PROC		= ARM7

SRCDIR			= ../common .
SRCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/pxi/common/src \
                   $(TWL_NITROSDK_ROOT)/build/libraries/pxi/ARM7/src \

SRCS			= pxi_init.c pxi_fifo.c

TARGET_LIB		= libpxi_sp$(TWL_LIBSUFFIX).a


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
