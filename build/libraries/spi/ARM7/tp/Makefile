#! make -f
#----------------------------------------------------------------------------
# Project:  NitroSDK - libraries - spi
# File:     Makefile
#
# Copyright 2003-2005 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: Makefile,v $
# Revision 1.7  2005/03/06 23:52:59  terui
# Update copyright.
#
# Revision 1.6  2005/03/05 02:02:04  terui
# DEBUGビルド時にRELEASEビルドのコンパイルオプションでビルドされるように改造。
#
# Revision 1.5  2004/09/06 13:30:37  terui
# INCDIRをlibraries/spi/ARM7/includeを指すようにpath変更。
#
# Revision 1.4  2004/08/27 08:49:21  yasu
# Fix around NITRO_CODEGEN_ALL
#
# Revision 1.3  2004/07/14 14:12:07  yasu
# Change NITRO_CODEGEN=ALL -> NITRO_CODEGEN_ALL=True
#
# Revision 1.2  2004/07/14 11:52:57  yasu
# Build ARM & THUMB code always
#
# Revision 1.1  2004/05/25 01:05:33  terui
# TPライブラリをSPIライブラリから分離
#
# $NoKeywords: $
#----------------------------------------------------------------------------

SUBDIRS			=

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL 	?= True

TWL_PROC		= ARM7

SRCDIR			= ./src
SRCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/spi/ARM7/tp/src \

SRCS			= tp_sp.c tp_sampling.c twl_tp_sampling.c

TARGET_LIB		= libtp_sp$(TWL_LIBSUFFIX).a
INCDIR          = ../include
INCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/spi/ARM7/include

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
