
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

#
LIBDIR += -L../../lib/libsearchlet -L../../lib/libfilter 
LIBDIR += -L../../lib/libfilterexec -L../../lib/libtools
LIBDIR += -L../../lib/liblog -L../../lib/libodisk 
LIBDIR += -L../../transport/socket/hoststub -L../../lib/libdctl 
LIBDIR += -L../../lib/libdconfig


SHARED_HEADERS = rgb.h image_tools.h common_consts.h img_search.h
SHARED_HEADERS += image_common.h texture_tools.h fil_tools.h 
SHARED_HEADERS += face_tools.h fil_data2ii.h facedet.h filter_api.h
SHARED_HEADERS += face.h fil_histo.h histo.h gtk_image_tools.h
SHARED_HEADERS += fil_texture.h face_data.h

SHARED_COMMON_SRCS = image_tools.c histo.c face_tools.c texture_tools.c
SHARED_COMMON_SRCS += facedet.c  fil_data2ii.c

SHARED_COMMON_OBJS = image_tools.o histo.o face_tools.o texture_tools.o
SHARED_COMMON_OBJS += facedet.o  fil_data2ii.o

SHARED_SEARCHLET_SRCS = fil_histo.c fil_texture.cc fil_face.c

SHARED_SEARCHLET_OBJS = fil_histo.o fil_texture.o fil_face.o

      
SHARED_APP_SRCS = rgb.c img_search.cc
SHARED_APP_SRCS += texture_search.cc vj_face_search.cc
SHARED_APP_SRCS += window_search.cc example_search.cc
SHARED_APP_SRCS += rgb_histo_search.cc rgb_img.cc
SHARED_APP_SRCS += regex_search.cc ii_img.cc
SHARED_APP_SRCS += gtk_image_tools.c
                            
SHARED_APP_OBJS = rgb.o img_search.o
SHARED_APP_OBJS += texture_search.o vj_face_search.o
SHARED_APP_OBJS += window_search.o example_search.o
SHARED_APP_OBJS += rgb_histo_search.o rgb_img.o
SHARED_APP_OBJS += regex_search.o ii_img.o
SHARED_APP_OBJS += gtk_image_tools.c
 
SHARED_SRCS = $(SHARED_SEARCHLET_SRCS) $(SHARED_APP_SRCS) $(SHARED_HEADERS)
SHARED_SRCS += $(SHARED_COMMON_SRCS)                            


# objects used to build the searchlet(s)
SEARCHLET_SRCS = $(SHARED_SEARCHLET_SRCS) $(SHARED_COMMON_SRCS)
SEARCHLET_SRCS += fil_file.c 
SEARCHLET_SRCS += fil_image_tools.c  
SEARCHLET_SRCS += merge_faces.c fil_tools.c 
SEARCHLET_SRCS += fil_regex.c
SEARCHLET_SRCS += fil_data2cv.c 
SEARCHLET_SRCS += fil_synth.c 


SEARCHLET_OBJS = $(SHARED_SEARCHLET_OBJS) $(SHARED_COMMON_OBJS)
SEARCHLET_OBJS += fil_file.o 
SEARCHLET_OBJS += fil_image_tools.o  
SEARCHLET_OBJS += merge_faces.o fil_tools.o 
SEARCHLET_OBJS += fil_regex.o fil_data2cv.o 
SEARCHLET_OBJS += fil_synth.o  



# objects needed to build the main
SNAPFIND_SRCS = $(SHARED_APP_SRCS) $(SHARED_COMMON_SRCS)
SNAPFIND_SRCS += snapfind.cc face_image.c sfind_search.c 
SNAPFIND_SRCS +=  read_config.cc 
SNAPFIND_SRCS += fil_tools.c stats_win.c rgb.c gui_thread.c  
SNAPFIND_SRCS += search_support.cc snap_popup.cc 
SNAPFIND_SRCS += sfind_tools.c fil_file.c

SNAPFIND_OBJS = $(SHARED_APP_OBJS) $(SHARED_COMMON_OBJS)
SNAPFIND_OBJS += snapfind.o face_image.o sfind_search.o read_config.o \
		fil_tools.o stats_win.o gui_thread.o \
	 	fil_image_tools.o search_support.o snap_popup.o \
		sfind_tools.o fil_file.o



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
