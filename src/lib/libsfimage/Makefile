#
#      Diamond (Release 1.0)
#      A system for interactive brute-force search
#
#      Copyright (c) 2002-2005, Intel Corporation
#      All Rights Reserved
#
#  This software is distributed under the terms of the Eclipse Public
#  License, Version 1.0 which can be found in the file named LICENSE.
#  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
#  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
#

ROOTDIR = ../..
SRCDIR = .

OBJROOT = /home/larry/build_cmu/snapfind/src
CVCPPFLAGS = -I/usr/local/include/ -I/usr/local/include/opencv/
CVLDFLAGS = -L/usr/local/lib/
                                                                                

LIBSRCS += fil_image_tools.c image_draw.c image_tools.c
LIBSRCS += readtiff.c rgb.c

LIBOBJS += fil_image_tools.o image_draw.o image_tools.o
LIBOBJS += readtiff.o rgb.o

SRCS = $(LIBSRCS)
OBJS = $(LIBOBJS) 

SHARED_HEADERS = rgb.h lib_sfimage.h

# Compilation Targets
#


TARGETS = libsfimage.a 

include $(OBJROOT)/Makefile.template 


