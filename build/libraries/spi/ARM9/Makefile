#! make -f
#----------------------------------------------------------------------------
# Project:  NitroSDK - libraries - spi
# File:     Makefile.CORE
#
# Copyright 2005 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: Makefile.CORE,v $
# Revision 1.1  2005/05/26 02:50:14  terui
# ライブラリを作成するコア部分をMakefileから切り出して新規追加
#
# $NoKeywords: $
#----------------------------------------------------------------------------

SUBDIRS		=

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL	?= True

# INCDIR			?= ../include
INCDIR            = $(TWL_NITROSDK_ROOT)/build/libraries/spi/include

SRCDIR			= ./src
SRCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/spi/ARM9/src \

SRCS			= tp.c mic.c pm.c

TARGET_LIB		= libspi$(TWL_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
