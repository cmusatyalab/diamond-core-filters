AC_SUBST(CVCPPFLAGS)
AC_SUBST(CVLDFLAGS)

AC_ARG_WITH(opencv, 
    [--with-opencv=DIR - root of opencv install dir],
    [ pfx="`(cd ${withval} ; pwd)`"
      CVCPPFLAGS="-I${pfx}/include/ -I${pfx}/include/opencv/"
      CVLDFLAGS="-L${pfx}/lib/"
    ]
)

AC_DEFUN(SNAPFIND_OPTIONS_SYS,
  [AC_ARG_WITH($1,
    [  --with-$1=DIR    $1 was installed in DIR],
    [ pfx="`(cd ${withval} ; pwd)`"
      CPPFLAGS="${CPPFLAGS} -I${pfx}/include"
      LDFLAGS="${LDFLAGS} -L${pfx}/lib"
      PATH="${PATH}:${pfx}/bin"])
    ])

AC_DEFUN(SNAPFIND_OPTION_LIBRARY,
  [AC_ARG_WITH($1-includes,
    [  --with-$1-includes=DIR   $1 include files are in DIR],
    [ pfx="`(cd ${withval} ; pwd)`"
      CPPFLAGS="${CPPFLAGS} -I${pfx}"])
   AC_ARG_WITH($1-libraries,
    [  --with-$1-libraries=DIR  $1 library files are in DIR],
    [ pfx="`(cd ${withval} ; pwd)`"
      LDFLAGS="${LDFLAGS} -L${pfx}"]) 
    ])

