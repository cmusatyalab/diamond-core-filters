# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(FULL-PACKAGE-NAME, VERSION, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([include/lib_sfimage.h]) AC_CONFIG_HEADER([config.h])
# Checks for the standard compile programs 
AC_PROG_CXX
AC_PROG_CC


#
# Load snapfind macros
#
#AC_CONFIG_AUX_DIR(configs)
sinclude(configs/snapfind_macros.m4)

#
# Paths to search for static libraries when building searchlet code.
#
ADD_LIB_SEARCH([/opt/diamond-filter-kit/lib])
ADD_LIB_SEARCH([/opt/diamond-filter-kit/opencv/lib])
ADD_LIB_SEARCH([/usr/lib])
ADD_LIB_SEARCH([/lib])

# Checks for libraries.

# Look for the various diamond libraries that we need
PKG_CHECK_MODULES(OPENDIAMOND, [opendiamond >= 6.1])
AC_SUBST(OPENDIAMOND_CFLAGS)
AC_SUBST(DIAMOND_BUNDLE_SEARCH,
    [$($PKG_CONFIG opendiamond --variable=bindir)/diamond-bundle-search])


#
# Find static libraries we need for compiling the searchlet code.
#
CHECK_STATIC_LIB([libtiff.a])
CHECK_STATIC_LIB([libjpeg.a])
CHECK_STATIC_LIB([libpng.a])
CHECK_STATIC_LIB([libz.a])
CHECK_STATIC_LIB([libarchive.a])

# deal with the cv stuff
AC_CHECK_LIB([cxcore], [cvAlloc])
AC_CHECK_LIB([cvaux], [cvFindFace])
AC_CHECK_LIB([cv], [cvResize])


# standard system libraries.
AC_CHECK_LIB([m], [atan2])

OBJROOT=`pwd`
AC_SUBST(OBJROOT)


AC_SUBST(BUILDROOT)
AC_ARG_WITH(buildroot, [--with-buildroot=DIR set target build hierachy to DIR],
    [ BUILDROOT="`(cd ${withval} ; pwd)`"],
    [ BUILDROOT=${OBJROOT}/target]
    ) 


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([memset pow regcomp sqrt strdup strtol])

AC_CONFIG_FILES([Makefile
		Makefile.template
		searches/Makefile
		searches/text_attr/Makefile
		searches/num_attr/Makefile
		searches/gabor_texture/Makefile
		searches/ocv_face/Makefile
		searches/rgbimg/Makefile
		searches/dog_texture/Makefile
		searches/rgb_histogram/Makefile
		searches/img_diff/Makefile
		searches/thumbnailer/Makefile
		searches/null/Makefile
		searches/shingling/Makefile
		lib/Makefile
		lib/libsfimage/Makefile
		lib/libresults/Makefile
		lib/libocvimage/Makefile
		target/Makefile
		target/lib/Makefile])
AC_OUTPUT
