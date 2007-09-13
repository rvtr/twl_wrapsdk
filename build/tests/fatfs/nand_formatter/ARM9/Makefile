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
# $Log: Makefile,v $
# $NoKeywords: $
#----------------------------------------------------------------------------

SUBDIRS		=


#----------------------------------------------------------------------------

#TWL_CODEGEN   =       THUMB

TARGET_BIN	=	main.axf

SRCS		=	main.c

#SRCDIR		=	# using default
#LCFILE		=	# using default


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

# .sbin
#$(BINDIR)/$(TARGET_BIN_BASENAME).sbin : $(BINDIR)/$(TARGET_BIN_BASENAME).axf
	objcopy -I elf32-little -O binary $(BINDIR)/$(TARGET_BIN_BASENAME).axf $(BINDIR)/$(TARGET_BIN_BASENAME).sbin


include	$(TWLSDK_ROOT)/build/buildtools/modulerules








#===== End of Makefile =====
