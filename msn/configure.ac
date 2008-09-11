dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/msn.h)
AC_CONFIG_AUX_DIR(admin)
AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

dnl All versioning is done via the following line
AM_INIT_AUTOMAKE(Licq-MSN-Protocol, 1.3.6)
AM_CONFIG_HEADER(config.h)

dnl Chuck #defines for PACKAGE and VERSION into config.h
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_DEFINE_UNQUOTED(INT_VERSION, $INT_VERSION, [Version number as an integer])

dnl Also substitute PACKAGE and VERSION in the AC_OUTPUT files
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
LICQ_CXX_FLAGS
LICQ_CXX_WEXTRA
AC_ISC_POSIX
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PATH_PROG(PROG_STRIP,strip,:)
AC_PATH_PROG(PROG_SIZE,size,:)

dnl libtool checks
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])
AM_PROG_LIBTOOL
AC_LIBTOOL_CXX
LIBTOOL="$LIBTOOL --silent"

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h dirent.h getopt.h malloc.h pwd.h sys/time.h)
AC_HEADER_TIME
LICQ_SYS_ERRLIST
LICQ_CHECK_LICQ
LICQ_CHECK_OPENSSL
AX_BOOST_BASE([1.31.1])

dnl Checks for library functions.
AC_CHECK_FUNCS(inet_addr inet_aton strerror strndup)
AC_CHECK_LIB(ssl, MD5, [], AC_MSG_ERROR([
*** OpenSSL is required by MSN. Please visit http://www.openssl.org and
*** install the latest version]))

dnl Switch to C++ mode and check for needed C++ headers
AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_CHECK_HEADER(vector,,
  AC_MSG_ERROR(You need to have the libstdc++ headers installed))

AC_LANG_RESTORE

msn_gcc_major_version=0
AC_MSG_CHECKING([whether the compiler is gcc 4 or greater])
if test x"$GXX" = xno; then
  AC_MSG_RESULT([no])
else
  # On some distros, there are snapshots available as gcc4
  if test -z "$ac_cv_prog_CC" || test x"$CC" = xgcc4; then
    our_gcc="$CC"
  else
    our_gcc="$ac_cv_prog_CC"
  fi
  # new major versions must be added here
  case `$our_gcc --version | sed -e 's,\..*,.,' -e q` in
    *4.)
      msn_gcc_major_version=4
      ;;
    *3.)
      msn_gcc_major_version=3
      ;;
    *2.)
      msn_gcc_major_version=2
      ;;
    *1.)
      msn_gcc_major_version=1
      ;;
  esac
  if test "$msn_gcc_major_version" -ge 4; then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
  fi
fi

# Remember that all following tests will run with this CXXFLAGS by default
if test "$msn_gcc_major_version" -ge 4; then
  CXXFLAGS=`echo $CXXFLAGS | sed 's/-O.//1'`
  CFLAGS=`echo $CFLAGS | sed 's/-O.//1'`
fi

AC_OUTPUT(
  src/Makefile
	Makefile)

