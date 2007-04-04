#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries_sp - os
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
#SUBMAKES		= Makefile.CALLTRACE 	\
#			  Makefile.FUNCTIONCOST

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL 	?= TRUE

# Codegen for sub processer
TWL_PROC		= ARM7

SRCDIR			= ../common .
SRCDIR			+= $(TWL_NITROSDK_ROOT)/build/libraries/os/common/src \
                   $(TWL_NITROSDK_ROOT)/build/libraries/os/ARM7/src \

SRCS			= \
			  os_init.c		\
			  os_interrupt.c	\
			  os_irqHandler.c	\
			  os_irqTable.c		\
			  os_spinLock.c		\
			  os_printf.c		\
			  os_system.c		\
			  os_entropy.c		\
			  os_terminate_sp.c \
			  os_thread.c		\
			  os_context.c		\
			  os_emulator.c		\
			  os_message.c		\
			  os_mutex.c		\
			  os_arena.c		\
			  os_alloc.c		\
			  os_exception.c	\
			  os_timer.c		\
			  os_tick.c		\
			  os_alarm.c		\
			  os_valarm.c		\
			  os_profile.c		\
			  os_reset.c		\
			  os_ownerInfo.c	\

TARGET_LIB		= libos_sp$(TWL_LIBSUFFIX).a


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


#===== End of Makefile =====
