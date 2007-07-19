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
			  camera_i2c_micron.c \
			  camera_i2c_sharp.c \
			  camera_control.c \

MICRON_INI_FILE		:= \
			  MT9V113-MTM9-2.ini \
			  MT9V113-MTM10.ini \
			  MT9V113-nin00.ini \

MICRON_CONVERT_EXE	:= convert_micron.pl

MICRON_INIC_FILE	:= $(MICRON_INI_FILE:%.ini=%.autogen.c)

SHARP_DAT_FILE		:= \
			  5030_15fpsFIX_x8_CC_FilterKIM_MatrixOn3_PLL.dat \
			  5030_15fps_5fps_adjust2_x8_CC_FilterKIM_MatrixOn3_PLL_1676MHz-1676MHz.dat \

SHARP_CONVERT_EXE	:= convert_sharp.pl

SHARP_DATC_FILE	:= $(SHARP_DAT_FILE:%.dat=%.autogen.c)

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

%.autogen.c: %.ini $(MICRON_CONVERT_EXE)
	perl $(MICRON_CONVERT_EXE) $< $@

%.autogen.c: %.dat $(SHARP_CONVERT_EXE)
	perl $(SHARP_CONVERT_EXE) $< $@

$(OBJDIR)/camera_i2c_micron.o: $(MICRON_INIC_FILE)
$(OBJDIR)/camera_i2c_sharp.o: $(SHARP_DATC_FILE)

#===== End of Makefile =====
