dnl Copyright (c) 1998 N. D. Bellamy

AC_DEFUN(AC_PATH_QT_LIB,
[
  AC_REQUIRE_CPP()
  AC_REQUIRE([AC_PATH_X])
  AC_MSG_CHECKING(for QT libraries)
  
  ac_qt_libraries="no"
  
  AC_ARG_WITH(qt-libraries,
    [  --with-qt-libraries     where the QT libraries are located. ],
    [  ac_qt_libraries="$withval" ])

  AC_CACHE_VAL(ac_cv_lib_qtlib, [
    
    dnl Did the user give --with-qt-libraries?
    
    if test "$ac_qt_libraries" = no; then

      dnl No they didn't, so lets look for them...
    
      dnl If you need to add extra directories to check, add them here.
      
      qt_library_dirs="\
        /usr/lib \
        /usr/local/lib \
        /usr/lib/qt \
        /usr/lib/qt/lib \
        /usr/local/lib/qt \
        /usr/local/qt/lib \
        /usr/X11/lib \
        /usr/X11/lib/qt \
        /usr/X11R6/lib \
        /usr/X11R6/lib/qt"
  
      if test "x$QTDIR" != x; then
        qt_library_dirs="$QTDIR/lib $qt_library_dirs"
      fi
  
      if test "x$QTLIB" != x; then
        qt_library_dirs="$QTLIB $qt_library_dirs"
      fi
    
      for qt_dir in $qt_library_dirs; do
        for qt_check_lib in $qt_dir/libqt.so*; do
          if test -r $qt_check_lib; then
            ac_qt_libraries=$qt_dir
            break 2
          fi
        done
      done
    fi

    ac_cv_lib_qtlib=$ac_qt_libraries
  ])

  dnl Define a shell variable for later checks

  if test "$ac_cv_lib_qtlib" = no; then
    have_qt_lib="no"
  else
    have_qt_lib="yes"
  fi
  
  AC_MSG_RESULT([$ac_cv_lib_qtlib])
  QT_LDFLAGS="-L$ac_cv_lib_qtlib"
  QT_LIBDIR="$ac_cv_lib_qtlib"
  AC_SUBST(QT_LDFLAGS)
  AC_SUBST(QT_LIBDIR)
])

AC_DEFUN(AC_PATH_QT_INC,
[
  AC_REQUIRE_CPP()
  AC_REQUIRE([AC_PATH_X])
  AC_MSG_CHECKING(for QT includes)
  
  ac_qt_includes="no"
  
  AC_ARG_WITH(qt-includes,
    [  --with-qt-includes      where the QT headers are located. ],
    [  ac_qt_includes="$withval" ])
  
  AC_CACHE_VAL(ac_cv_header_qtinc, [
    
    dnl Did the user give --with-qt-includes?
    
    if test "$ac_qt_includes" = no; then

      dnl No they didn't, so lets look for them...

      dnl If you need to add extra directories to check, add them here.
      
      qt_include_dirs="\
        /usr/lib/qt/include \
        /usr/include/qt \
        /usr/local/qt/include \
        /usr/local/include/qt \
        /usr/X11/include/qt \
        /usr/X11/include/X11/qt \
        /usr/X11R6/include \        
        /usr/X11R6/include/qt \
        /usr/X11R6/include/X11/qt"

      if test "x$QTDIR" != x; then
        qt_include_dirs="$QTDIR/include $qt_include_dirs"
      fi
      
      if test "x$QTINC" != x; then
        qt_include_dirs="$QTINC $qt_include_dirs"
      fi
      
      for qt_dir in $qt_include_dirs; do
        if test -r "$qt_dir/qtabwidget.h"; then
          ac_qt_includes=$qt_dir
          break
        fi
      done
    fi

    ac_cv_header_qtinc=$ac_qt_includes
  
  ])

  if test "$ac_cv_header_qtinc" = no; then
    have_qt_inc="no"
  else
    have_qt_inc="yes"
  fi

  AC_MSG_RESULT([$ac_cv_header_qtinc])
  QT_INCLUDES="-I$ac_cv_header_qtinc"
  QT_INCDIR="$ac_cv_header_qtinc"
  AC_SUBST(QT_INCLUDES)
  AC_SUBST(QT_INCDIR)
])

AC_DEFUN(AC_PATH_QT_MOC,
[
  AC_PATH_PROG(
    QT_MOC,
    moc,
    /usr/bin/moc,
    $QTDIR/bin:/usr/bin:/usr/X11R6/bin:/usr/lib/qt/bin:/usr/local/qt/bin:$PATH)
])

AC_DEFUN(AC_CHECK_SOCKS5,
[
  AC_MSG_CHECKING(whether to enable SOCKS5 support)

  WITH_SOCKS5="no"
  SOCKS_LIBS=""
  SOCKS_LIBDIR=""
  SOCKS_INCDIR=""
	
  AC_ARG_ENABLE(
    socks5,
    [  --enable-socks5         enable SOCKS5 firewall support],
    WITH_SOCKS5=yes)

  AC_ARG_WITH(
    socks5-inc,
    [  --with-socks5-inc=PATH  include path for SOCKS5 headers],
    socks_incdir="$withval", socks_incdir="no")
  
  AC_ARG_WITH(
    socks5-lib,
    [  --with-socks5-lib=PATH  library path for SOCKS5 libraries],
    socks_libdir="$withval", socks_libdir="no")

  if test "$WITH_SOCKS5" = "no"; then
    AC_MSG_RESULT(no)
  else
    AC_MSG_RESULT(yes)

    if test "$socks_libdir" = "no"; then
	AC_CHECK_LIB(socks5, SOCKSconnect, SOCKS_LIBS="-lsocks5")
    else
    	AC_MSG_CHECKING(where to look for the SOCKS5 library)
	SOCKS_LIBS="-lsocks5"
	SOCKS_LIBDIR="-L$socks_libdir"
	AC_MSG_RESULT($socks_libdir)
    fi

    if test "$socks_incdir" = "no"; then
        AC_CHECK_HEADER(socks.h)
    else
    	AC_MSG_CHECKING(where to look for the SOCKS5 headers)
	SOCKS_INCDIR="-I$socks_incdir"
	AC_MSG_RESULT($socks_incdir)
    fi
    AC_DEFINE(USE_SOCKS5)
  fi

  dnl Substitute these even if they're null, so as not to mess up makefiles
  
  AC_SUBST(SOCKS_LIBS)
  AC_SUBST(SOCKS_LIBDIR)
  AC_SUBST(SOCKS_INCDIR)
])

dnl Like AC_CHECK_HEADER, but it uses the already-computed -I directories.

AC_DEFUN(AC_CHECK_X_HEADER, [
  ac_save_CPPFLAGS="$CPPFLAGS"
  if test \! -z "$includedir" ; then
    CPPFLAGS="$CPPFLAGS -I$includedir"
  fi
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"
  AC_CHECK_HEADER([$1],[$2],[$3])
  CPPFLAGS="$ac_save_CPPFLAGS"
])
  
dnl Like AC_CHECK_LIB, but it used the -L dirs set up by the X checks.

AC_DEFUN(AC_CHECK_X_LIB, [
  ac_save_CPPFLAGS="$CPPFLAGS"
  ac_save_LDFLAGS="$LDFLAGS"

  if test \! -z "$includedir" ; then
    CPPFLAGS="$CPPFLAGS -I$includedir"
  fi
  
  dnl note: $X_CFLAGS includes $x_includes
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"

  if test \! -z "$libdir" ; then
    LDFLAGS="$LDFLAGS -L$libdir"
  fi
  
  dnl note: $X_LIBS includes $x_libraries
  
  LDFLAGS="$LDFLAGS $X_LIBS"
  AC_CHECK_LIB([$1], [$2], [$3], [$4], [$5])
  CPPFLAGS="$ac_save_CPPFLAGS"
  LDFLAGS="$ac_save_LDFLAGS"]
)

dnl Check if it is possible to turn off run time type information (RTTI)
AC_DEFUN(AC_PROG_CXX_FNO_RTTI,
[AC_CACHE_CHECK(whether ${CXX-g++} accepts -fno-rtti, ac_cv_prog_cxx_fno_rtti,
[echo 'void f(){}' > conftest.cc
if test -z "`${CXX-g++} -fno-rtti -c conftest.cc 2>&1`"; then
  ac_cv_prog_cxx_fno_rtti=yes
  CXXFLAGS="${CXXFLAGS} -fno-rtti"
else
  ac_cv_prog_cxx_fno_rtti=no
fi
rm -f conftest*
])])

dnl Check if the type socklen_t is defined anywhere
AC_DEFUN(AC_C_SOCKLEN_T,
[AC_CACHE_CHECK(for socklen_t, ac_cv_c_socklen_t,
[ AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
],[
socklen_t foo;
],[
  ac_cv_c_socklen_t=yes
],[
  ac_cv_c_socklen_t=no
  AC_DEFINE(socklen_t,int)
])])])

dnl Check for sys_errlist[] and sys_nerr, check for declaration
dnl Check nicked from aclocal.m4 from GNU bash 2.01
AC_DEFUN(AC_SYS_ERRLIST,
[AC_MSG_CHECKING([for sys_errlist and sys_nerr])
AC_CACHE_VAL(ac_cv_sys_errlist,
[AC_TRY_LINK([#include <errno.h>],
[extern char *sys_errlist[];
 extern int sys_nerr;
 char *msg = sys_errlist[sys_nerr - 1];],
    ac_cv_sys_errlist=yes, ac_cv_sys_errlist=no)])dnl
AC_MSG_RESULT($ac_cv_sys_errlist)
if test $ac_cv_sys_errlist = yes; then
AC_DEFINE(HAVE_SYS_ERRLIST)
fi
])
