#
#
#
#                          Diamond 1.0
# 
#            Copyright (c) 2002-2004, Intel Corporation
#                         All Rights Reserved
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following
#      disclaimer in the documentation and/or other materials provided
#      with the distribution.
#
#    * Neither the name of Intel nor the names of its contributors may
#      be used to endorse or promote products derived from this software 
#      without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
# Compiler Flags. Warning: -O causes problems w/ pthreads
#

CC = gcc
CPLUSPLUS = g++

CFLAGS += -DDEBUG 
CFLAGS += -Wall
CFLAGS += -D_REENTRANT
CFLAGS += -D_THREAD_SAFE
CFLAGS_SHARED = $(CFLAGS) -fPIC FLEX = flex 
# # Set the include paths.  #
INCDIR += -I ../../lib/libsearchlet
INCDIR += -I../../lib/libtools
INCDIR += -I../../lib/libfilter
INCDIR += -I../../lib/libodisk
INCDIR += -I../../lib/liblog
INCDIR += -I../../lib/libod
INCDIR += -I../../lib/libdconfig
INCDIR += -I/usr/local/include/opencv
INCDIR += `pkg-config gtk+-2.0 gthread-2.0 --cflags-only-I` 


# # Set the libraries and the library paths we will need
# to build these files.
LIBS += -ldl -lfl -lpthread -lm 
LIBS += -Wl,-whole-archive -lsearchlet -lfilterexec -lfilter -ltools -llog 
LIBS += -Wl,-no-whole-archive -lfl -lodisk
LIBS += -lhstub -ldctl -ldconfig
LIBS += -lopencv   -lcvaux -llog
#LIBS += -lglib

#
LIBDIR += -L../../lib/libsearchlet -L../../lib/libfilter 
LIBDIR += -L../../lib/libfilterexec -L../../lib/libtools
LIBDIR += -L../../lib/liblog -L../../lib/libodisk 
LIBDIR += -L../../transport/socket/hoststub -L../../lib/libdctl 
LIBDIR += -L../../lib/libdconfig

include ../common_searches/Makefile.template

# objects used to build the searchlet(s)
SEARCHLET_SRCS = $(SHARED_SEARCHLET_SRCS) $(SHARED_COMMON_SRCS)
SEARCHLET_SRCS += fil_regex.c
SEARCHLET_SRCS += fil_data2cv.c 
SEARCHLET_SRCS += fil_synth.c  

SEARCHLET_OBJS = $(SHARED_SEARCHLET_OBJS) $(SHARED_COMMON_OBJS)
SEARCHLET_OBJS += fil_regex.o fil_data2cv.o 
SEARCHLET_OBJS += fil_synth.o 



# objects needed to build the main
SNAPFIND_SRCS = $(SHARED_APP_SRCS) $(SHARED_COMMON_SRCS)
SNAPFIND_SRCS += snapfind.cc sfind_search.cc
SNAPFIND_SRCS +=  read_config.cc 
SNAPFIND_SRCS += stats_win.c rgb.c   
SNAPFIND_SRCS += snap_popup.cc 
SNAPFIND_SRCS += sfind_tools.c 

SNAPFIND_OBJS = $(SHARED_APP_OBJS) $(SHARED_COMMON_OBJS)
SNAPFIND_OBJS += snapfind.o sfind_search.o read_config.o 
SNAPFIND_OBJS += stats_win.o snap_popup.o 
SNAPFIND_OBJS += sfind_tools.o  fil_histo.o

DEBUG_SRCS = face.c
DEBUG_OBJS = face.o

#
# All the source files that are needed for depend.
#
SRCS = $(SNAPFIND_SRCS) $(SEARCHLET_SRCS) $(DEBUG_SRCS)
OBJS = $(SNAPFIND_OBJS) $(SEARCHLET_OBJS) $(DEBUG_OBJS)

#
# Compilation Targets
#


#
# Other objects to cleanup
CLEAN_OTHERS += read_config.cc $(SHARED_SRCS)

TARGETS = $(SHARED_HEADERS) snapfind snap_searchlet.so

include ${DIAMOND_ROOT}/Makefile.template

# XXX
CFLAGS += -finline-functions -funroll-loops
# turns on unsafe fp optimizations; we don't require strict IEEE
CFLAGS += -ffast-math

ifeq ($(OSTYPE),linux)
DYNAMIC = -rdynamic
endif

ifeq ($(OSTYPE),darwin)
DYNAMIC = -dynamic
endif

cln:
	$(RM) core.*
	$(RM) /tmp/fspec* /tmp/filsp* /tmp/objfil*
	$(RM) /var/tmp/fspec* /var/tmp/filsp* /var/tmp/objfil*

clean-targets:	clean-experiments
	$(RM) $(TARGETS)

default: $(TARGETS)

snap_searchlet.so: $(SEARCHLET_OBJS)
	$(CPLUSPLUS) $(CFLAGS_SO) -o $@ -shared -static $(SEARCHLET_OBJS)  -lm -Wl,-static -lopencv  -lcvaux $(LIBS) $(LIBDIR)

libssearch.a: $(SEARCHLET_OBJS)
	$(AR) r $@ $(SEARCHLET_OBJS)

read_config.cc:	read_config.l
	$(FLEX) -Pyyhist -o$@ $<


snapfind: $(SNAPFIND_OBJS)
	$(CPLUSPLUS) $(CFLAGS) $(DYNAMIC) -o $@ $(SNAPFIND_OBJS) $(LIBS) $(LIBDIR) `pkg-config gtk+-2.0 gthread-2.0 --cflags --libs`

search:	search.c
	$(CC) $(CFLAGS) $(DYNAMIC) -o $@ $< $(LIBDIR) `pkg-config gtk+-2.0 gthread-2.0 --cflags --libs`

profile: profile.o $(SEARCHLET_OBJS)
	$(CC) $(CFLAGS) -o profile profile.o $(SEARCHLET_OBJS) $(LIBS) $(LIBDIR) -lfilterexec -ltools -lodisk

foo: foo.o rgb_histo_search.o snap_search.o window_search.o example_search.o
	$(CC) $(CFLAGS) -o foo foo.o rgb_histo_search.o snap_search.o window_search.o example_search.o

histo_bench: histo_bench.o $(SEARCHLET_OBJS)
	$(CPLUSPLUS) $(CFLAGS) $(DYNAMIC) -rdynamic -o histo_bench histo_bench.o $(SEARCHLET_OBJS) $(LIBS)  $(LIBDIR) 



face_benchmark: face_benchmark.o $(SEARCHLET_OBJS)
	$(CPLUSPLUS) $(CFLAGS) $(DYNAMIC) -rdynamic -o face_benchmark face_benchmark.o $(SEARCHLET_OBJS) $(LIBS)  $(LIBDIR) 

experiment_%: experiment_%.o $(SEARCHLET_OBJS)
	$(CC) $(CFLAGS) -o $@ $< \
  -static $(SEARCHLET_OBJS)  $(LIBS) $(LIBDIR) -lpthread -ldl -lm -Wl,-static -lopencv  

#	$(CC) $(CFLAGS) -o $@ $< $(SEARCHLET_OBJS) $(LIBS) $(LIBDIR) -lfilterexec -ltools -lodisk

EXPERIMENT_OBJS := $(wildcard experiment_*.o)
EXPERIMENT_TGTS = $(EXPERIMENT_OBJS:%.o=%)

experiments:	$(EXPERIMENT_TGTS)

clean-experiments:
	$(RM) $(EXPERIMENT_TGTS)


$(SHARED_SRCS):
	ln -f -s ../common_searches/$@
                                 


%.eps:	%.gviz
	dot $< -Gsize="8,10" -Tps -o$@

deps.gviz::
	cp /dev/null $@
	./hviz 5 $(SRCS) >> $@
