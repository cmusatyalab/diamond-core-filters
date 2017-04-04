# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([Diamond Core Filters], [2.0.6], [diamond@cs.cmu.edu])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 foreign subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([include/lib_filimage.h])
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_HOST

# Checks for the standard compile programs 
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_RANLIB

#
# Paths to search for static libraries when building searchlet code.
#
# Debian multiarch support
pfx="${host_cpu}-${host_os}"
AC_CHECK_FILE([/usr/lib/${pfx}], [ADD_LIB_SEARCH([/usr/lib/${pfx}])])
ADD_LIB_SEARCH([/usr/lib])
AC_CHECK_FILE([/lib/${pfx}], [ADD_LIB_SEARCH([/lib/${pfx}])])
ADD_LIB_SEARCH([/lib])

# Checks for libraries.
PKG_PROG_PKG_CONFIG

# Look for the various diamond libraries that we need
PKG_CHECK_MODULES(OPENDIAMOND, [opendiamond >= 7.0])
AC_SUBST(DIAMOND_BUNDLE_PREDICATE,
    [$($PKG_CONFIG opendiamond --variable=bindir)/diamond-bundle-predicate])

#
# Find libraries we need for linking the searchlet code.
#
PKG_CHECK_MODULES(FILTERLIBS, [opencv libtiff-4 libjpeg libpng libarchive])

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
