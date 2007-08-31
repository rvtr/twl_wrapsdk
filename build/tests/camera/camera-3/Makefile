#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - build
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

SUBDIRS =	\
            ARM7 \
            ARM9 \

#----------------------------------------------------------------------------

TARGET_BIN		=	camera-3.srl

MAKEROM_ARM9	=	ARM9/bin/$(TWL_BUILDTYPE_ARM9)/main.axf
MAKEROM_ARM7	=	ARM7/bin/$(TWL_BUILDTYPE_ARM7)/main.axf

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

ifeq	($(TWL_PLATFORM),TS)
do-build:		$(TARGETS)
endif

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
