#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - CDC - libraries
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
# $Log: Makefile,v $
# $NoKeywords: $
#----------------------------------------------------------------------------

SUBDIRS     =

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL ?= True

# Codegen for sub processer
TWL_PROC    =   ARM7

SRCS	=	cdc.c		\
		cdc_api.c	\
		cdc_filter_coefficient.c	\
		cdc_dsmode_access.c

INCDIR	+=	$(TWL_NITROSDK_ROOT)/build/libraries/spi/ARM7/include \
		$(TWL_NITROSDK_ROOT)/build/libraries/spi/ARM7/pm/include

TARGET_LIB	=	libcdc_sp$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	=	$(TARGETS)
INSTALL_DIR	=	$(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:       $(TARGETS)

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
