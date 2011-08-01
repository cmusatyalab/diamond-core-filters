ACLOCAL_AMFLAGS = -I m4
DISTCHECK_CONFIGURE_FLAGS = CFLAGS=-m32 CXXFLAGS=-m32 LDFLAGS=-m32

EXTRA_DIST  = CREDITS INSTALL LICENSE

AM_CPPFLAGS  = -I$(top_srcdir)/include $(CVCPPFLAGS) $(OPENDIAMOND_CFLAGS)
AM_CPPFLAGS += -pthread
AM_CFLAGS    = -D_GNU_SOURCE -D_THREAD_SAFE -D_REENTRANT
AM_CXXFLAGS  = -D_GNU_SOURCE -D_THREAD_SAFE -D_REENTRANT
AM_CFLAGS   += -W -Wall -Wno-unused-parameter
AM_CXXFLAGS += -W -Wall -Wno-unused-parameter
AM_CFLAGS   += -Werror-implicit-function-declaration
AM_LDFLAGS   = -pthread
LDADD        = lib/libsnapfind.a $(STATIC_LIBS) $(CVLDFLAGS)
LDADD       += $(OPENDIAMOND_LIBS)

# Filter size reduction
AM_CFLAGS   += -fdata-sections -ffunction-sections
AM_CXXFLAGS += -fdata-sections -ffunction-sections
AM_LDFLAGS  += -Wl,--gc-sections

include lib/build.mk
include searches/build.mk

noinst_LIBRARIES = lib/libsnapfind.a
filterdir = $(datadir)/diamond/filters
searchdir = $(datadir)/diamond/searches
filter_PROGRAMS = $(FILTERS)
search_DATA = $(SEARCHES)
