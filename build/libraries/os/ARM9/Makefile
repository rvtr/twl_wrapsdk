#! make -f
#----------------------------------------------------------------------------
# Project:  NitroSDK - libraries - os
# File:     Makefile
#
# Copyright 2003-2005 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: Makefile,v $
# Revision 1.3  2005/05/25 11:42:54  terui
# Makefile.CORE Ç include Ç∑ÇÈÇÊÇ§Ç…èCê≥
#
# Revision 1.2  2004/09/13 05:37:28  yasu
# merged with NitroSDK_2_00pr6_branch
#
# Revision 1.1.2.1  2004/09/13 04:39:29  yasu
# build libos for TS_VERSION=0,100 and 200
#
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
			  os_init.c		\
			  os_interrupt.c	\
			  os_irqHandler.c	\
			  os_irqTable.c		\
			  os_spinLock.c		\
			  os_printf.c		\
			  os_system.c		\
			  os_entropy.c		\
			  os_thread.c		\
			  os_context.c		\
			  os_emulator.c		\
			  os_message.c		\
			  os_mutex.c		\
			  os_cache.c		\
			  os_arena.c		\
			  os_alloc.c		\
			  os_tcm.c		\
			  os_protectionUnit.c	\
			  os_protectionRegion.c	\
			  os_exception.c	\
			  os_timer.c		\
			  os_tick.c		\
			  os_alarm.c		\
			  os_valarm.c		\
			  os_profile.c		\
			  os_reset.c		\
			  os_ownerInfo.c	\
			  os_vramExclusive.c	\
			  os_china.c		\
			  os_argument.c		\
			  os_terminate_proc.c	\


TARGET_LIB		= libos$(TWL_LIBSUFFIX).a

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
