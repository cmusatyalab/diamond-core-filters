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
INCDIR += -I../../lib/libtools
INCDIR += -I../../lib/libsearchlet
INCDIR += -I../../lib/libodisk
INCDIR += -I../../lib/liblog
INCDIR += -I../../lib/libfilterexec
INCDIR += -I../../lib/libod
INCDIR += -I../../lib/libfilter
INCDIR += -I../../apps/common_searches
INCDIR += -I/usr/local/include/opencv


# to build these files.
LIBS += -ldl -lfl -lpthread -lm
LIBS += -Wl,-whole-archive -lsearchlet -lfilterexec -lfilter -ltools -llog
LIBS += -Wl,-no-whole-archive -lfl -lodisk
LIBS += -lhstub -ldctl -ldconfig
LIBS += -lopencv
                                                                                
#
LIBDIR += -L../../lib/libsearchlet -L../../lib/libfilter
LIBDIR += -L../../lib/libfilterexec -L../../lib/libtools
LIBDIR += -L../../lib/liblog -L../../lib/libodisk
LIBDIR += -L../../transport/socket/hoststub -L../../lib/libdctl
LIBDIR += -L../../lib/libdconfig


OBJS = snap_bench.o 
SRCS = snap_bench.c

TARGETS = snap_bench 


include ${DIAMOND_ROOT}/Makefile.template

#
#

snap_bench: $(OBJS)
	$(CPLUSPLUS) $(CFLAGS) -o snap_bench $(OBJS) $(LIBDIR) $(LIBS)




