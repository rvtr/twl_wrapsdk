#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - SND - libraries
# File:     Makefile
#
# Copyright 2004,2005 Nintendo.  All rights reserved.
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

SUBDIRS     =

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL ?= True

# Codegen for sub processer
TWL_PROC    =   ARM7

SRCDIR		=	../common ./src
SRCDIR		+= $(TWL_NITROSDK_ROOT)/build/libraries/snd/common/src \
                   $(TWL_NITROSDK_ROOT)/build/libraries/snd/ARM7/src \

SRCS	=	\
		snd_global.c  		\
		snd_channel.c		\
		snd_util.c		\
		snd_main.c		\
		snd_capture.c		\
		snd_exchannel.c 	\
		snd_seq.c 		\
		snd_midiplayer.c 	\
		snd_bank.c 		\
		snd_work.c 		\
		snd_alarm.c		\
		snd_command.c		\
		snd_data.c		\
		snd_i2s.c		\


TARGET_LIB	=	libsnd_sp$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------

# DEBUG版ビルドの場合、RELEASE版でビルドして
# DEBUG版のライブラリを装います。

ifdef	TWL_DEBUG
TWL_BUILD_TYPE	= RELEASE
endif

include $(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	=	$(TARGETS)
INSTALL_DIR	=	$(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:       $(TARGETS)

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
