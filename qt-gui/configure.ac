dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/licqgui.h)
AC_CONFIG_AUX_DIR(admin)
AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

dnl All versioning is done via the following line
AM_INIT_AUTOMAKE(Licq-Qt-GUI, 1.3.6)
AM_CONFIG_HEADER(config.h)

dnl Chuck #defines for PACKAGE and VERSION into config.h
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION")
AC_DEFINE_UNQUOTED(INT_VERSION,$INT_VERSION, [Qt-GUI version number as int])

dnl Also substitute PACKAGE and VERSION in the AC_OUTPUT files
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
LICQDAEMON_CXX_FLAGS
LICQ_CXX_WEXTRA
AC_PROG_INSTALL
dnl AC_PATH_PROG(PROG_STRIP,strip,:)
dnl AC_PATH_PROG(PROG_SIZE,size,:)

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
		if test $GXX != yes; then
			LDFLAGS="$LDFLAGS -mimpure-text"
		fi
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


dnl libtool stuff
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
m4_defun([_LT_AC_LANG_F77_CONFIG], [:])
AM_PROG_LIBTOOL
AC_LIBTOOL_CXX
LIBTOOL="$LIBTOOL --silent"
dnl AC_SUBST(LIBTOOL)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h locale.h)
AC_HEADER_TIME
LICQ_SYS_ERRLIST
LICQ_CHECK_LICQ
AX_BOOST_BASE([1.31.1])

dnl Check for X and extra X libs needed
AC_PATH_XTRA
if test "$no_x" = yes; then
  AC_MSG_ERROR(You need to have the X11 libraries and headers installed)
fi

dnl solaris has broken x11 headers, lets check for them
LICQ_CHECK_X11HEADERS

XSS_LIBS="no"
AC_CHECK_X_LIB(Xext, XScreenSaverRegister,[XSS_LIBS=""],[], [-lX11 -lXext -lm])
AC_CHECK_X_LIB(Xss, XScreenSaverRegister,[XSS_LIBS="-lXss"],[],[-lX11 -lXext -lm])
if test \! "$XSS_LIBS" = "no"; then
  AC_CHECK_X_HEADER(X11/extensions/scrnsaver.h,[AC_DEFINE(USE_SCRNSAVER, 1, [use X screensaver support])])
else
  dnl Don't include the screensaver libs if we can't find the header.
  XSS_LIBS=""
fi
AC_SUBST(XSS_LIBS)

dnl Check for Qt & KDE
AC_PATH_QT
AC_PATH_QT_INC

if test "$have_qt_inc" = "no"; then
  AC_MSG_ERROR([

I can't find the Qt header files.

Install the Qt development package, or if you have them installed, override
this check with the --with-qt-includes argument, and I'll take your word for
it.
])

fi

AC_PATH_KDE
AC_PATH_KDE_INCLUDES

AC_PATH_QT_LIB

if test "$have_qt_lib" = "no"; then
  AC_MSG_ERROR([

I can't find the Qt libraries. It is recommended to set the QTDIR environment
variable.

Install the Qt libraries, or if you have them installed, override this check
with the --with-qt-libraries argument, and I'll take your word for it.
])

fi

dnl Check for Qt version
AC_CHECK_QT_SETUP

AC_PATH_QT_MOC
AC_PATH_QT_LUPDATE
AC_PATH_QT_LRELEASE

AC_PATH_KDE_LIBRARIES
AC_PATH_DCOP
AC_PATH_KDE_CONFIG
LICQ_CHECK_GPGME

dnl Switch to C++ mode and check for needed C++ headers
AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_CHECK_HEADER(vector,,
  AC_MSG_ERROR(You need to have the libstdc++ headers installed))

AC_LANG_RESTORE

dnl Check for qinterlacestyle.h
LICQ_CHECK_QT_HEADER

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_C_SOCKLEN_T

AC_OUTPUT(
  po/Makefile \
  src/Makefile \
  share/Makefile \
  Makefile)

echo ""
echo "Please remember to use GNU make, often installed as gmake."
echo ""
echo "Install prefix : $prefix"
echo "Licq Includes  : $LICQ_INCLUDES"
if test "$have_kde" = yes; then
  echo "KDE            : includes in $kde_includes, libs in $kde_libraries"
else
  echo "KDE            : disabled."
fi
echo "Qt includes    : $ac_cv_header_qtinc"
echo "Qt libraries   : $ac_qt_libdir, $ac_qt_libname"
echo "Qt moc         : $MOC"
if test "$WITH_GPGME" = yes; then
  echo "GPG            : enabled."
else
  echo "GPG            : disabled."
fi
echo ""

