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
SRCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/cs/common/src \
                   $(TWL_NITROSDK_ROOT)/build/libraries/cs/ARM9/src \

INCDIR			= ../common
INCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/cs/common/include \

SRCS			= sjis2utf.c csjistab.c

TARGET_LIB		= libcs$(TWL_LIBSUFFIX).a


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(TWL_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
