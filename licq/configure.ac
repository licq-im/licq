dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/licq.h)
AC_CONFIG_AUX_DIR(admin)
AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

dnl All versioning is done via these defines
INT_VERSION=1035
AM_INIT_AUTOMAKE(licq, 1.3.6)

AM_CONFIG_HEADER(config.h)

dnl Chuck #defines for PACKAGE and VERSION into config.h
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_DEFINE_UNQUOTED(INT_VERSION,$INT_VERSION, [Licq version number as int])
AC_DEFINE_UNQUOTED(USE_FIFO, 1, [Undefine if it causes problems]) 


dnl Expand prefix and exec_prefix
test "x${prefix}" = "xNONE" && prefix="${ac_default_prefix}"
test "x${exec_prefix}" = "xNONE" && exec_prefix='${prefix}'

dnl (May) need to expand libdir and datadir more than once:
dnl $libdir => $exec_prefix/lib => $prefix/lib => /some/path/lib
install_libdir=$(eval "echo \"$(eval "echo \"${libdir}\"")\"")
AC_DEFINE_UNQUOTED(INSTALL_LIBDIR, "${install_libdir}/licq/", [Directory where plugins go])

dnl $datadir => $datarootdir => $prefix/share => /some/path/share
install_datadir=$(eval "echo \"$(eval "echo \"${datadir}\"")\"")
AC_DEFINE_UNQUOTED(INSTALL_SHAREDIR, "${install_datadir}/licq/", [Directory where shared data go])


AC_ARG_ENABLE(
  hebrew,
  [  --enable-hebrew  include support for hebrew reverse string],
  USE_HEBREW=yes, USE_HEBREW=no)
if test "$USE_HEBREW" = "yes"; then 
  AC_DEFINE_UNQUOTED(USE_HEBREW, 1, [reverse string support])
fi

dnl Also substitute PACKAGE and VERSION in the AC_OUTPUT files
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
LICQDAEMON_CXX_FLAGS
LICQ_CXX_WEXTRA
AC_PROG_CXXCPP
AC_ISC_POSIX
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PATH_PROG(PROG_STRIP,strip,:)
AC_PATH_PROG(PROG_SIZE,size,:)

dnl Required for GPGME...
AC_SYS_LARGEFILE

dnl Set up some operating system specific options
case "$host" in
	*-*-openbsd*)
		CFLAGS="$CFLAGS -pthread"
		CPPFLAGS="$CPPFLAGS -D_REENTRANT"
		CXXFLAGS="$CXXFLAGS -pthread"
		echo "Setting OpenBSD compilation options"
		;;
	*-*-freebsd*)
		CFLAGS="$CFLAGS -pthread"
		CPPFLAGS="$CPPFLAGS -D_THREAD_SAFE"
		CXXFLAGS="$CXXFLAGS -pthread"
		echo "Setting FreeBSD compilation options"
		;;
	*-*-solaris*)
		CPPFLAGS="$CPPFLAGS -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DUSE_SOLARIS -DPIC -fPIC"
		LDFLAGS="$LDFLAGS -mimpure-text"
		echo "Setting Solaris compilation options"
		;;
	*-*-linux*)
 		CPPFLAGS="$CPPFLAGS -fPIC -DPIC -D_REENTRANT"
		echo "Setting Linux compilation options"
		;;
  *-*-aix*)
    CPPFLAGS="$CPPFLAGS -D_THREAD_SAFE"
    LIBS="$LIBS -lc_r"
    echo "Setting AIX compilation options"
    ;;
  *-*-irix*)
    LDFLAGS="$LDFLAGS -L/usr/lib32"
    echo "Setting Irix compilation options"
    ;;
	*)
		echo "Licq may not compile on this system. Good luck :-)"
		;;
esac

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h fcntl.h getopt.h limits.h malloc.h pwd.h unistd.h paths.h)

dnl Check for headers we can use to get endian for system and maybe some bswap functions
AC_CHECK_HEADERS(byteswap.h endian.h machine/endian.h sys/byteorder.h)

dnl Checks for library functions.
AC_CHECK_LIB(xnet,inet_addr)
AC_CHECK_LIB(socket, socket)
LICQ_CHECK_GETHOSTBYNAME
AC_CHECK_LIB(c_r,pthread_cancel, [], [
  AC_CHECK_LIB(pthread, pthread_cancel, [], [
    AC_CHECK_LIB(thread, pthread_cancel, [], [
      AC_CHECK_LIB(pthreads, pthread_cancel, [], [
        cat <<EOF;

Please install/update your POSIX threads (pthreads) library. Updates should be
available from either your system vendor, or, for GNU/Linux systems, go to
http://pauillac.inria.fr/~xleroy/linuxthreads/.  GNU Pth can also be used if
configured with --enable-pthread.

EOF
exit 1
      ])
    ])
  ])
])


AC_CHECK_LIB(c_r, dlopen, [], [
  AC_CHECK_LIB(c, dlopen, [], [
    AC_CHECK_LIB(dl, dlopen, [], [
      echo "I can't find the library containing dlopen()"
      exit 1
    ])
  ])
])

AC_MSG_CHECKING([for dlopen modes])
AC_TRY_COMPILE([
#include <stdio.h>
#include <dlfcn.h>
],[
dlopen(NULL, RTLD_NOW)
],[
  AC_MSG_RESULT([RTLD_NOW])
  AC_DEFINE_UNQUOTED(DLOPEN_POLICY, RTLD_NOW, [2nd dlopen parameter])
],[
  AC_TRY_COMPILE([
  #include <stdio.h>
  #include <dlfcn.h>
  ],[
  dlopen(NULL, RTLD_LAZY)
  ],[
    AC_MSG_RESULT([RTLD_LAZY])
    AC_DEFINE_UNQUOTED(DLOPEN_POLICY, RTLD_LAZY, [2nd dlopen parameter])
  ],[
    AC_MSG_RESULT([cannot determine, bailing])
    exit 1;
  ])
])


AC_CHECK_FUNCS(inet_addr inet_aton mktime select strdup strerror hstrerror readdir_r)

AC_CHECK_FUNC(pthread_kill_other_threads_np,[AC_DEFINE([HAVE_PTHREAD_KILL_OTHER_THREADS_NP],
[], [we are not using NTPL])])
AC_CHECK_SOCKS5
LICQ_CHECK_GPGME
LICQ_CHECK_OPENSSL
AX_BOOST_BASE([1.33.1])

dnl Switch to C++ mode and check for needed C++ headers
AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_CHECK_HEADER(vector,,
  AC_MSG_ERROR(You need to have the libstdc++ headers installed))

AC_LANG_RESTORE

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_C_SOCKLEN_T
AC_HEADER_TIME
AC_STRUCT_TM
LICQ_SYS_ERRLIST
LICQ_CHECK_GMTOFF
LICQ_CHECK_TIMEZONE

dnl libtool stuff
AC_OBJEXT
AC_EXEEXT
AC_LIBTOOL_DLOPEN
AC_DISABLE_SHARED
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])
AC_PROG_LIBTOOL
AC_LIBTOOL_CXX

AM_GNU_GETTEXT([external])
AM_ICONV
if test "$am_cv_func_iconv" != "yes"; then
  AC_MSG_ERROR(Please install the iconv library.)
fi

AC_OUTPUT( 
  po/Makefile.in \
  bin/Makefile \
  share/Makefile \
  src/Makefile \
  include/Makefile \
  Makefile )


echo ""
echo ""

if test "$WITH_SOCKS5" = "no"; then
  echo "SOCKS5 support is disabled."
else
  echo "SOCKS5 support is enabled."
fi

if test "$WITH_GPGME" = "no"; then
  echo "GPGME support is disabled."
else
  echo "GPGME support is enabled."
fi

if test "$WITH_OPENSSL" = "no"; then
  echo ""
  echo "OpenSSL support is not available.  Licq will not be able to"
  echo "establish secure communication channels. This is not required"
  echo "for proper function (i.e. it is optional). If you really desire this"
  echo "functionality then install OpenSSL (libraries and headers) and"
  echo "rebuild Licq.  OpenSSL can be found at http://www.openssl.org"
  echo "or as RPM from your local distribution site."
  echo ""
  echo "Note you need version 0.9.5a or greater."
else
  echo "OpenSSL secure channel support is enabled."
fi

dnl Check for msgfmt and gmsgfmt
if test "$GMSGFMT" = ":"; then 
  if test "$MSGFMT" = ":"; then
    echo "** msgfmt and gmsgfmt could not be found. You're loosing daemon localization."
  fi
fi

echo ""
echo "--- Now type \"gmake\" to build Licq ---"
echo "--- If gmake is not working, try \"make\" ---"
echo ""

