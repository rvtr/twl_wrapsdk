#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - camera/ARM7
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

SRCDIR                  = ../common .

SRCS			= \
			  camera_i2c.c \
			  camera_control.c \

L_SET_FILE	:= A3AFX_EVT2_20fps_16M_PLLoff_hVGA_070209.set \
                   A3AFX_EVT2_30fps_16M_QVGA_070206.set \
                   A3AFX_EVT2_20fps_16M_070206.set \
                   320x240_noPLL_20fps_070306.set \
                   320x240_PLL_30fps_070306.set \
                   A3AFX_EVT3_30fps_Scaledown_16.7M_50Hz.set \
                   A3AFX_EVT3_30fps_Subsampling_16.7M_50Hz.set \

L_CONVERT_EXE	:= convert.pl
L_SETC_FILE	:= $(L_SET_FILE:%.set=%.autogen.c)
SRCS		+= $(L_SETC_FILE)

TARGET_LIB		= libcamera_sp$(TWL_LIBSUFFIX).a


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

%.autogen.c: %.set $(L_CONVERT_EXE)
	perl $(L_CONVERT_EXE) $< > $@

#===== End of Makefile =====
