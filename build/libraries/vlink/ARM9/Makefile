#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - os
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

SUBDIRS		=
SUBMAKES	=


#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL 	?= True

SRCDIR		= ../common .
SRCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/os/common/src \
                   $(TWL_NITROSDK_ROOT)/build/libraries/os/ARM9/src \

SRCS			= \
			  osemu.c		\
			  osemu2.c		\


TARGET_LIB		= libvlink$(TWL_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(TWL_INSTALL_LIBDIR)

ifdef NITRO_CALLTRACE
CCFLAGS			+= -DOS_PROFILE_AVAILABLE -DOS_PROFILE_CALL_TRACE
endif

ifdef NITRO_FUNCTIONCOST
CCFLAGS			+= -DOS_PROFILE_AVAILABLE -DOS_PROFILE_FUNCTION_COST
endif

ifdef NITRO_TCM_APPLY
CCFLAGS			+= -DSDK_TCM_APPLY
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
