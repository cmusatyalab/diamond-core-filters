INCDIR += -I../../lib/libtools
INCDIR += -I../../lib/libsearchlet
INCDIR += -I../../lib/libodisk
INCDIR += -I../../lib/liblog
INCDIR += -I../../lib/libfilterexec
INCDIR += -I../../lib/libod
INCDIR += -I../../lib/libfilter
INCDIR += -I../../apps/common_searches


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




