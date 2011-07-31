# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([SnapFind], [2.0], [diamond@cs.cmu.edu])
AC_CONFIG_SRCDIR([include/lib_sfimage.h])
AC_CONFIG_HEADER([config.h])
# Checks for the standard compile programs 
AC_PROG_CXX
AC_PROG_CC

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
      CVCPPFLAGS="-I${pfx}/include/ -I${pfx}/include/opencv/"
      CVLDFLAGS=" ${pfx}/lib/libcvaux.a ${pfx}/lib/libcv.a ${pfx}/lib/libcxcore.a"
      ADD_LIB_SEARCH(${pfx}/lib)
    ],
    [ CVCPPFLAGS="-I/opt/diamond-filter-kit/include -I/opt/diamond-filter-kit/include/opencv"
      CVLDFLAGS=" -L/opt/diamond-filter-kit/lib -lcvaux -lcv -lcxcore"
    ])
AC_SUBST(CVCPPFLAGS)
AC_SUBST(CVLDFLAGS)

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

OBJROOT=`pwd`
AC_SUBST(OBJROOT)


AC_SUBST(BUILDROOT)
AC_ARG_WITH(buildroot,
    [AS_HELP_STRING([--with-buildroot=DIR],
        [set target build hierachy to DIR])],
    [ BUILDROOT="`(cd ${withval} ; pwd)`"],
    [ BUILDROOT=${OBJROOT}/target]
    ) 

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
