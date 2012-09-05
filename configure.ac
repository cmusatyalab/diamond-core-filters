# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([Diamond Core Filters], [2.0.5], [diamond@cs.cmu.edu])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 foreign subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([include/lib_filimage.h])
AC_CONFIG_HEADER([config.h])

# Checks for the standard compile programs 
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_RANLIB

# Add command-line options.

AC_ARG_WITH(staticlib, 
    [AS_HELP_STRING([--with-staticlib=DIR],
        [add DIR to search path for static libraries])],
    [ pfx="`(cd ${withval} ; pwd)`"
      ADD_LIB_SEARCH(${pfx})
    ])
AC_ARG_WITH(opencv, 
    [AS_HELP_STRING([--with-opencv=DIR], [root of opencv install dir])],
    [ pfx="`(cd ${withval} ; pwd)`"
      CVCPPFLAGS="-I${pfx}/include"
      CVLDFLAGS=" ${pfx}/lib/libopencv_objdetect.a ${pfx}/lib/libopencv_imgproc.a ${pfx}/lib/libopencv_core.a -lrt"
      ADD_LIB_SEARCH(${pfx}/lib)
    ],
    [ CVCPPFLAGS="-I/opt/diamond-filter-kit/include"
      CVLDFLAGS=" -L/opt/diamond-filter-kit/lib -lopencv_objdetect -lopencv_imgproc -lopencv_core -lrt"
    ])
AC_SUBST(CVCPPFLAGS)
AC_SUBST(CVLDFLAGS)

#
# Paths to search for static libraries when building searchlet code.
#
ADD_LIB_SEARCH([/opt/diamond-filter-kit/lib])
ADD_LIB_SEARCH([/usr/lib])
ADD_LIB_SEARCH([/lib])

# Checks for libraries.

# Look for the various diamond libraries that we need
PKG_CHECK_MODULES(OPENDIAMOND, [opendiamond >= 7.0])
AC_SUBST(DIAMOND_BUNDLE_PREDICATE,
    [$($PKG_CONFIG opendiamond --variable=bindir)/diamond-bundle-predicate])


#
# Find static libraries we need for compiling the searchlet code.
#
CHECK_STATIC_LIB([libtiff.a])
CHECK_STATIC_LIB([libjpeg.a])
CHECK_STATIC_LIB([libpng.a])
CHECK_STATIC_LIB([libz.a])
CHECK_STATIC_LIB([libarchive.a])

OBJROOT=`pwd`
AC_SUBST(OBJROOT)


AC_SUBST(BUILDROOT)
AC_ARG_WITH(buildroot,
    [AS_HELP_STRING([--with-buildroot=DIR],
        [set target build hierachy to DIR])],
    [ BUILDROOT="`(cd ${withval} ; pwd)`"],
    [ BUILDROOT=${OBJROOT}/target]
    ) 

AC_CONFIG_FILES([Makefile])
AC_CONFIG_COMMANDS([mkdir], [$MKDIR_P codecs predicates])
AC_OUTPUT
