#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - ctrdg
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

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL =	True

TWL_PROC	=	ARM9

SRCDIR		=	../common .
SRCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/ctrdg/common/src \
                   $(TWL_NITROSDK_ROOT)/build/libraries/ctrdg/ARM9/src \

SRCS		=	\
			ctrdg.c				\
			ctrdg_proc.c			\
			ctrdg_vib.c			\
			ctrdg_backup.c			\
			ctrdg_flash_common.c		\
			ctrdg_flash_AT29LV512.c		\
			ctrdg_flash_LE26FV10N1TS-10.c	\
			ctrdg_flash_LE39FW512.c		\
			ctrdg_flash_MX29L010.c		\
			ctrdg_flash_MX29L512.c		\
			ctrdg_sram.c		\
			ctrdg_task.c


TARGET_LIB	=	libctrdg$(TWL_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	=	$(TARGETS)
INSTALL_DIR	=	$(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
