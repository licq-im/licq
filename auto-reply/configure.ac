dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/autoreply.h)
AC_CONFIG_AUX_DIR(admin)
AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

dnl All versioning is done via the following line
AM_INIT_AUTOMAKE(Licq-AutoReply, 1.3.8)
AM_CONFIG_HEADER(config.h)

dnl Chuck #defines for PACKAGE and VERSION into config.h
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")

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

dnl libtool checks
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])
AM_PROG_LIBTOOL
AC_LIBTOOL_CXX
LIBTOOL="$LIBTOOL --silent"

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h pwd.h sys/time.h paths.h)
AC_HEADER_TIME
LICQ_SYS_ERRLIST
LICQ_CHECK_LICQ
AX_BOOST_BASE([1.31.1])

dnl Checks for library functions.
AC_CHECK_FUNCS(inet_addr inet_aton strerror)

dnl Switch to C++ mode and check for needed C++ headers
AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_CHECK_HEADER(vector,,
  AC_MSG_ERROR(You need to have the libstdc++ headers installed))

AC_LANG_RESTORE

AC_OUTPUT(
  src/Makefile
	Makefile)

