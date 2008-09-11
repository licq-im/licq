AC_PREREQ(2.57)
AC_INIT(licq_osd_plugin, 1.3.6, martinmaurer@gmx.at)
AC_DEFINE([INT_VERSION], [], [Description])
AC_CONFIG_AUX_DIR(admin)
AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AC_CONFIG_SRCDIR([src/my_xosd.h])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

dnl Chuck #defines for PACKAGE and VERSION into config.h
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_DEFINE_UNQUOTED(INT_VERSION,$INT_VERSION)

dnl Also substitute PACKAGE and VERSION in the AC_OUTPUT files
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AC_DEFINE(GETTEXT_PACKAGE, "licq-osd", [Gettext domain name])
GETTEXT_PACKAGE=licq-osd
AC_SUBST(GETTEXT_PACKAGE)

AC_PROG_CC

#AC_PROG_INTLTOOL([0.25])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.12.1)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
LICQ_CXX_WEXTRA
AC_PROG_CXXCPP
AC_ISC_POSIX
AC_PROG_MAKE_SET
AC_PROG_INSTALL

AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])
#AC_PROG_LIBTOOL
AM_PROG_LIBTOOL
LIBTOOL="$LIBTOOL --silent"

# Checks for libraries.
AM_PATH_LIBXOSD([],[echo "Please install the libxosd2 library (http://www.ignavus.net/software.html)" ; exit 1])

# Checks for new version of xosd.
# ensure LIBS is set
#AC_CHECK_LIB([xosd], [xosd_create], [], [])
#LIBS_old=$LIBS
#NEWXOSD=""
#AC_CHECK_LIB(xosd, xosd_set_linetimeout, NEWXOSD="-DNEWXOSD", [])
#AC_SUBST(NEWXOSD)
#LIBS=$LIBS_old

# CVS has become stable
# the new plugin version will compile with licq >=1.3.0
# older licq versions are no longer supported.
# (if you need them anyways - use an 1.2.7.x version of the plugin)
# contact me (martinmaurer@gmx.at) if you have any problems
#AC_ARG_WITH(cvs, AC_HELP_STRING([--with-cvs], [compile for licq >=1.2.8]),
#        [case "${enableval}" in
#                yes)    
#						AC_DEFINE(CVSLICQ, 1)
#						cvs="yes"
#                                ;;
#                no)     
#						AC_DEFINE(CVSLICQ, 0)
#                                                cvs="no"
#                                ;;
#                *)              AC_MSG_ERROR([bad value ${enableval} for --with-cvs: yes/no allowed])
#                                ;;
#        esac],
#        [ AC_DEFINE(CVSLICQ, 0)
#          cvs="no" ]
#)
#AC_SUBST(CVSLICQ)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h dirent.h getopt.h malloc.h pwd.h sys/time.h)
AC_CHECK_HEADERS([unistd.h])
LICQ_CHECK_LICQ
AX_BOOST_BASE([1.31.1])

AC_OUTPUT( po/Makefile.in
  src/Makefile
  Makefile
)

# build for cvs licq ..............: $cvs

AC_MSG_NOTICE( [
Configuration for $PACKAGE $VERSION :
-------------------------------------
 install path (libdir+licq path)..: $libdir/licq
   Look for any WARNINGs output by the configure script 
   (run ./configure --help to see all options).
   If you're satisfied, type 'make' to build $PACKAGE $VERSION,
   and then 'make install' for installation.
] )
