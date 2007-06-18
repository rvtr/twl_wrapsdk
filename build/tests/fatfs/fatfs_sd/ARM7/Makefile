#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - CAMERA - demos - camera-1
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

LCFILE_SPEC	=	main.lsf

#----------------------------------------------------------------------------

#TWL_CODEGEN   =       THUMB
TWL_PROC      =       ARM7

TARGET_BIN	=	main.axf

SRCS		=	main.c

INCDIR      += $(TWLSDK_ROOT)/include/twl \
               $(TWLSDK_ROOT)/include/twl/devices/sdmc/ARM7 \
               $(TWLSDK_ROOT)/include/twl/fatfs/ARM7 \

LLIBRARY_DIRS = $(TWLSDK_ROOT)/lib/ARM-BB/Release
LLIBRARIES = libfatfs_sp.twl.a \
             libsd_sp.twl.a \

#SRCDIR		=	# using default
#LCFILE		=	# using default

include	$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
