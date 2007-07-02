#! make -f
#---------------------------------------------------------------------------
#  Project:  NitroSDK - tools - makelst
#  File:     Makefile
#
#  Copyright 2006,2007 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#---------------------------------------------------------------------------
include	$(NITROSDK_ROOT)/build/buildtools/commondefs

TARGETS		= makelst.exe

SOURCES_C	= makelst.c arch.c elf.c loader_subset.c elf_loader.c searcharg.c

SOURCES		= $(SOURCES_C)

OBJECTS		= $(SOURCES_C:.c=.o)

HEADERS		= 

INSTALL_DIR     = .
INSTALL_TARGETS	= $(TARGETS)

LDIRT_CLEAN	= $(OBJECTS) $(TARGETS)

include	$(NITROSDK_ROOT)/build/buildtools/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJECTS)
			$(CC_X86) $+ -o $@

makelcf.o:		makelcf.c makelcf.h version.h

version.h:		$(SOURCES) $(HEADERS) $(MAKEFILE)
			@for i in $^ ;				\
			do					\
				date -r $$i +'#define SDK_DATE_OF_LATEST_FILE %Y%m%dUL';	\
			done | sort | tail -1 > $@
