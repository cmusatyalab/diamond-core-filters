
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
#
# Set the include paths.
#
INCDIR += -I ../../lib/libsearchlet
INCDIR += -I../../lib/libtools
INCDIR += -I../../lib/libfilter
INCDIR += -I../../lib/libodisk
INCDIR += -I../../lib/liblog
INCDIR += -I../../lib/libod
INCDIR += -I../../lib/libdconfig
INCDIR += `pkg-config gtk+-2.0 gthread-2.0 --cflags-only-I` 


#
# Set the libraries and the library paths we will need
# to build these files.
LIBS += -ldl -lfl -lpthread -lm 
LIBS += -Wl,-whole-archive -lsearchlet -lfilterexec -lfilter -ltools -llog 
LIBS += -Wl,-no-whole-archive -lfl -lodisk
LIBS += -lhstub -ldctl -ldconfig
LIBS += -lopencv 
LIBDIR += -L../../lib/libsearchlet -L../../lib/libfilter -L../../lib/libfilterexec \
          -L../../lib/libtools -L../../lib/liblog -L../../lib/libodisk \
          -L../../transport/socket/hoststub -L../../lib/libdctl \
	  -L../../lib/libdconfig

#
# Other objects to cleanup
CLEAN_OTHERS += read_config.cc

SHARED_HEADERS = rgb.h image_tools.h common_consts.h

SHARED_SEARCHLET_SRCS = image_tools.c

SHARED_SEARCHLET_OBJS = image_tools.o
      
SHARED_APP_SRCS = rgb.c 
                            
SHARED_APP_OBJS = image_tools.o rgb.o
 
SHARED_SRCS = $(SHARED_SEARCHLET_SRCS) $(SHARED_APP_SRCS) $(SHARED_HEADERS)
                            


# objects used to build the searchlet(s)
SEARCHLET_SRCS = $(SHARED_SEARCHLET_SRCS)
SEARCHLET_SRCS += fil_data2ii.c fil_face.c fil_file.c facedet.c
SEARCHLET_SRCS += fil_image_tools.c  
SEARCHLET_SRCS += histo.c fil_histo.c merge_faces.c fil_tools.c 
SEARCHLET_SRCS += texture_tools.c fil_regex.c
SEARCHLET_SRCS += fil_data2cv.c fil_texture.cc 
SEARCHLET_SRCS += fil_synth.c face_tools.c


SEARCHLET_OBJS = $(SHARED_SEARCHLET_OBJS)
SEARCHLET_OBJS += fil_data2ii.o fil_face.o fil_file.o facedet.o 
SEARCHLET_OBJS += fil_image_tools.o histo.o 
SEARCHLET_OBJS += fil_histo.o merge_faces.o fil_tools.o texture_tools.o 
SEARCHLET_OBJS += fil_regex.o fil_data2cv.o fil_texture.o 
SEARCHLET_OBJS += fil_synth.o  face_tools.o



# objects needed to build the main
SNAPFIND_SRCS = $(SHARED_APP_SRCS)
SNAPFIND_SRCS += snapfind.cc face_tools.c face_image.c sfind_search.c 
SNAPFIND_SRCS +=  read_config.cc histo.c 
SNAPFIND_SRCS += fil_tools.c stats_win.c rgb.c gui_thread.c  
SNAPFIND_SRCS += texture_tools.c gtk_image_tools.c 
SNAPFIND_SRCS += snap_search.cc window_search.cc example_search.cc
SNAPFIND_SRCS += texture_search.cc vj_face_search.cc search_support.cc
SNAPFIND_SRCS += snap_popup.cc rgb_img.cc ii_img.cc regex_search.cc
SNAPFIND_SRCS += facedet.c sfind_tools.c fil_data2ii.c fil_file.c

SNAPFIND_OBJS = $(SHARED_APP_OBJS)
SNAPFIND_OBJS += snapfind.o face_tools.o face_image.o sfind_search.o read_config.o \
	histo.o fil_tools.o stats_win.o gui_thread.o \
	texture_tools.o rgb_histo_search.o \
	texture_search.o \
	snap_search.o window_search.o example_search.o gtk_image_tools.o \
	vj_face_search.o search_support.o snap_popup.o rgb_img.o ii_img.o \
	regex_search.o facedet.o sfind_tools.o fil_data2ii.o fil_file.o



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

TARGETS = $(SHARED_HEADERS) snapfind snap_searchlet.so

include ${DIAMOND_ROOT}/Makefile.template

# XXX
CFLAGS += -finline-functions -fkeep-inline-functions -funroll-loops
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
	$(CPLUSPLUS) $(CFLAGS_SO) -o $@ -shared -static $(SEARCHLET_OBJS)  -lm -Wl,-static -lopencv 

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

face_benchmark: face_benchmark.o $(SEARCHLET_OBJS)
	$(CC) $(CFLAGS) $(DYNAMIC) -rdynamic -o face_benchmark face_benchmark.o $(SEARCHLET_OBJS) $(LIBS)  $(LIBDIR) 

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
