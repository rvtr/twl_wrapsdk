#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - buildsetup - create register map
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
# $NoKeywords:$
#----------------------------------------------------------------------------

WORKDIR			=	nitro/hw/ARM7
TARGET			=	$(WORKDIR)/ioreg.h

CSVSRC			=	io_register_list.csv
SCRIPT			=	./header_generator_ioreg_sp.pl

NEWDIRS			=	$(WORKDIR)

#----------------------------------------------------------------------------

TARGET_PREFIX		=	$(subst .h,,$(TARGET))
TARGET_SUBFILES		=	$(TARGET_PREFIX)_*.h
TARGET_TMPCSV		=	$(TARGET_PREFIX).csv

# build
TARGETS			=	$(TARGET) $(TARGET_SUBFILES)

# install
INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR		=	$(TWL_INSTALL_INCDIR)/$(WORKDIR)
override DO_INSTALL	=	$(DO_INSTALL_WILDCARD)

# clean/clobber
LDIRT_CLEAN		=	$(TARGET_TMPCSV) $(TARGETS) \
				$(firstword $(subst /, ,$(WORKDIR)))
LDIRT_CLOBBER		=	


include	$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:		$(TARGET)


$(TARGET):		$(CSVSRC) $(SCRIPT)
	$(CP)     $(CSVSRC) $(TARGET_TMPCSV)
	$(PERL)   $(SCRIPT) $(TARGET_TMPCSV)

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#----- End of Makefile -----
