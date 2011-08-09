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
LDADD        = lib/libhelper.a $(STATIC_LIBS) $(CVLDFLAGS)
LDADD       += $(OPENDIAMOND_LIBS)

# Filter size reduction
AM_CFLAGS   += -fdata-sections -ffunction-sections
AM_CXXFLAGS += -fdata-sections -ffunction-sections
AM_LDFLAGS  += -Wl,--gc-sections

AM_V_BUNDLE   = $(AM_V_BUNDLE_$(V))
AM_V_BUNDLE_  = $(AM_V_BUNDLE_$(AM_DEFAULT_VERBOSITY))
AM_V_BUNDLE_0 = @echo "  BUNDLE" $@;
BUNDLE_COMMAND = $(AM_V_BUNDLE) $(DIAMOND_BUNDLE_PREDICATE) -o $@ $^

CLEANFILES =
include lib/build.mk
include filters/build.mk
include codecs/build.mk
include predicates/build.mk

noinst_LIBRARIES = lib/libhelper.a
filterdir = $(datadir)/diamond/filters
codecdir = $(datadir)/diamond/codecs
predicatedir = $(datadir)/diamond/predicates
filter_PROGRAMS = $(FILTERS)
codec_DATA = $(CODECS)
predicate_DATA = $(PREDICATES)
