dnl aclocal.m4 generated automatically by aclocal 1.2

dnl Copyright (c) 1998 N. D. Bellamy

## ------------------------------------------------------------------------
## Find a file (or one of more files in a list of dirs)
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_FIND_FILE,
[
$3=NO
for i in $2;
do
  for j in $1;
  do
    if test -r "$i/$j"; then
      $3=$i
      break 2
    fi
  done
done
])

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
        qt_library_dirs="$qt_library_dirs $QTDIR/lib"
      fi
  
      if test "x$QTLIB" != x; then
        qt_library_dirs="$qt_library_dirs $QTLIB"
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
  
  AC_CACHE_VAL(ac_cv_header_qtlib, [
    
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
        qt_include_dirs="$qt_include_dirs $QTDIR/include"
      fi

      if test "x$QTINC" != x; then
        qt_include_dirs="$qt_include_dirs $QTINC"
      fi

      for qt_dir in $qt_include_dirs; do
        if test -r "$qt_dir/qlistbox.h"; then
          ac_qt_includes=$qt_dir
          break
        fi
      done
    fi

    ac_cv_header_qtlib=$ac_qt_includes
  
  ])

  if test "$ac_cv_header_qtlib" = no; then
    have_qt_inc="no"
  else
    have_qt_inc="yes"
  fi

  AC_MSG_RESULT([$ac_cv_header_qtlib])
  QT_INCLUDES="-I$ac_cv_header_qtlib"
  QT_INCDIR="$ac_cv_header_qtlib"
  AC_SUBST(QT_INCLUDES)
  AC_SUBST(QT_INCDIR)
])

AC_DEFUN(AC_PATH_QT_MOC,
[
  AC_PATH_PROG(
    QT_MOC,
    moc,
    /usr/bin/moc,
    $PATH:/usr/bin:/usr/X11R6/bin:$QTDIR/bin:/usr/lib/qt/bin:/usr/local/qt/bin)
])

## ------------------------------------------------------------------------
## Now, the same with KDE
## $(KDE_LDFLAGS) will be the kdeliblocation (if needed)
## and $(kde_includes) will be the kdehdrlocation (if needed)
## ------------------------------------------------------------------------
##
AC_DEFUN(AC_PATH_KDE,
[
AC_REQUIRE([AC_PATH_QT_LIB])dnl
AC_REQUIRE([AC_PATH_QT_INC])

AC_ARG_WITH(kde,
  [  --with-kde       compile with KDE support. ],
  [  ac_kde="$withval" ])


AC_MSG_CHECKING([for KDE])
dnl if test "${prefix}" != NONE; then
dnl  kde_libraries=${prefix}/lib
dnl  kde_includes=${prefix}/include
dnl  AC_MSG_RESULT(["will be installed in" $prefix])
dnl else
ac_kde_includes=NO ac_kde_libraries=NO
kde_libraries=""
kde_includes=""

AC_CACHE_VAL(ac_cv_have_kde,
[#try to guess kde locations

kde_incdirs="/usr/lib/kde/include /usr/local/kde/include /usr/kde/include /usr/include/kde /usr/include $x_includes $qt_includes"

test -n "$KDEDIR" && kde_incdirs="$KDEDIR/include $KDEDIR $kde_incdirs"
AC_FIND_FILE(ksock.h, $kde_incdirs, kde_incdir)
ac_kde_includes=$kde_incdir

kde_libdirs="/usr/lib/kde/lib /usr/local/kde/lib /usr/kde/lib /usr/lib/kde /usr/lib /usr/X11R6/lib /usr/X11R6/kde/lib"
test -n "$KDEDIR" && kde_libdirs="$KDEDIR/lib $KDEDIR $kde_libdirs"
AC_FIND_FILE(libkdecore.la, $kde_libdirs, kde_libdir)
ac_kde_libraries=$kde_libdir

if test "$ac_kde_includes" = NO || test "$ac_kde_libraries" = NO; then
  ac_cv_have_kde="have_kde=no"
else
  ac_cv_have_kde="have_kde=yes \
    ac_kde_includes=$ac_kde_includes ac_kde_libraries=$ac_kde_libraries"
fi])


eval "$ac_cv_have_kde"

if test "$have_kde" != yes; then
  if test "${prefix}" = NONE; then
    ac_kde_prefix=$ac_default_prefix
  else
    ac_kde_prefix=$prefix
  fi
  dnl AC_MSG_RESULT(["will be installed in" $ac_kde_prefix])
  kde_libraries=${ac_kde_prefix}/lib
  kde_includes=${ac_kde_prefix}/include

else
  ac_cv_have_kde="have_kde=yes \
    ac_kde_includes=$ac_kde_includes ac_kde_libraries=$ac_kde_libraries"
  dnl AC_MSG_RESULT([libraries $ac_kde_libraries, headers $ac_kde_includes])
  kde_libraries=$ac_kde_libraries
  kde_includes=$ac_kde_includes
fi
dnl fi
AC_SUBST(kde_libraries)
AC_SUBST(kde_includes)

if test "$kde_includes" = "$x_includes" || test "$kde_includes" = "$qt_includes" ; then
  KDE_INCLUDES=""
else
  KDE_INCLUDES="-I$kde_includes"
  all_includes="$all_includes $KDE_INCLUDES"
fi

if test "$kde_libraries" = "$x_libraries" || test "$kde_libraries" = "$qt_libraries" ; then
  KDE_LDFLAGS=""
else
  KDE_LDFLAGS="-L$kde_libraries"
  all_libraries="$all_libraries $KDE_LDFLAGS"
fi

AC_SUBST(KDE_LDFLAGS)
AC_SUBST(KDE_INCLUDES)
AC_SUBST(all_includes)
AC_SUBST(all_libraries)

test -n "$ac_kde" && have_kde=$ac_kde
test -n "$ac_kde" || have_kde=no
if test "$have_kde" = yes; then
  AC_MSG_RESULT([libraries $ac_kde_libraries, headers $ac_kde_includes])
else
  AC_MSG_RESULT(["no"])
fi

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


# Macro to add for using GNU gettext.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 5

AC_DEFUN(AM_WITH_NLS,
  [AC_MSG_CHECKING([whether NLS is requested])
    dnl Default is enabled NLS
    AC_ARG_ENABLE(nls,
      [  --disable-nls           do not use Native Language Support],
      USE_NLS=$enableval, USE_NLS=yes)
    AC_MSG_RESULT($USE_NLS)
    AC_SUBST(USE_NLS)

    USE_INCLUDED_LIBINTL=no

    dnl If we use NLS figure out what method
    if test "$USE_NLS" = "yes"; then
      AC_DEFINE(ENABLE_NLS)
#      AC_MSG_CHECKING([whether included gettext is requested])
#      AC_ARG_WITH(included-gettext,
#        [  --with-included-gettext use the GNU gettext library included here],
#        nls_cv_force_use_gnu_gettext=$withval,
#        nls_cv_force_use_gnu_gettext=no)
#      AC_MSG_RESULT($nls_cv_force_use_gnu_gettext)
      nls_cv_force_use_gnu_gettext="no"

      nls_cv_use_gnu_gettext="$nls_cv_force_use_gnu_gettext"
      if test "$nls_cv_force_use_gnu_gettext" != "yes"; then
        dnl User does not insist on using GNU NLS library.  Figure out what
        dnl to use.  If gettext or catgets are available (in this order) we
        dnl use this.  Else we have to fall back to GNU NLS library.
	dnl catgets is only used if permitted by option --with-catgets.
	nls_cv_header_intl=
	nls_cv_header_libgt=
	CATOBJEXT=NONE

	AC_CHECK_HEADER(libintl.h,
	  [AC_CACHE_CHECK([for gettext in libc], gt_cv_func_gettext_libc,
	    [AC_TRY_LINK([#include <libintl.h>], [return (int) gettext ("")],
	       gt_cv_func_gettext_libc=yes, gt_cv_func_gettext_libc=no)])

	   if test "$gt_cv_func_gettext_libc" != "yes"; then
	     AC_CHECK_LIB(intl, bindtextdomain,
	       [AC_CACHE_CHECK([for gettext in libintl],
		 gt_cv_func_gettext_libintl,
		 [AC_CHECK_LIB(intl, gettext,
		  gt_cv_func_gettext_libintl=yes,
		  gt_cv_func_gettext_libintl=no)],
		 gt_cv_func_gettext_libintl=no)])
	   fi

	   if test "$gt_cv_func_gettext_libc" = "yes" \
	      || test "$gt_cv_func_gettext_libintl" = "yes"; then
	      AC_DEFINE(HAVE_GETTEXT)
	      AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
		[test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)dnl
	      if test "$MSGFMT" != "no"; then
		AC_CHECK_FUNCS(dcgettext)
		AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
		AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
		  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
		AC_TRY_LINK(, [extern int _nl_msg_cat_cntr;
			       return _nl_msg_cat_cntr],
		  [CATOBJEXT=.gmo
		   DATADIRNAME=share],
		  [CATOBJEXT=.mo
		   DATADIRNAME=lib])
		INSTOBJEXT=.mo
	      fi
	    fi

	    # Added by Martin Baulig 12/15/98 for libc5 systems
	    if test "$gt_cv_func_gettext_libc" != "yes" \
	       && test "$gt_cv_func_gettext_libintl" = "yes"; then
	       INTLLIBS=-lintl
	       LIBS=`echo $LIBS | sed -e 's/-lintl//'`
	    fi
	])

        if test "$CATOBJEXT" = "NONE"; then
	  AC_MSG_CHECKING([whether catgets can be used])
	  AC_ARG_WITH(catgets,
	    [  --with-catgets          use catgets functions if available],
	    nls_cv_use_catgets=$withval, nls_cv_use_catgets=no)
	  AC_MSG_RESULT($nls_cv_use_catgets)

	  if test "$nls_cv_use_catgets" = "yes"; then
	    dnl No gettext in C library.  Try catgets next.
	    AC_CHECK_LIB(i, main)
	    AC_CHECK_FUNC(catgets,
	      [AC_DEFINE(HAVE_CATGETS)
	       INTLOBJS="\$(CATOBJS)"
	       AC_PATH_PROG(GENCAT, gencat, no)dnl
#	       if test "$GENCAT" != "no"; then
#		 AC_PATH_PROG(GMSGFMT, gmsgfmt, no)
#		 if test "$GMSGFMT" = "no"; then
#		   AM_PATH_PROG_WITH_TEST(GMSGFMT, msgfmt,
#		    [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], no)
#		 fi
#		 AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
#		   [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
#		 USE_INCLUDED_LIBINTL=yes
#		 CATOBJEXT=.cat
#		 INSTOBJEXT=.cat
#		 DATADIRNAME=lib
#		 INTLDEPS='$(top_srcdir)/intl/libintl.a'
#		 INTLLIBS=$INTLDEPS
#		 LIBS=`echo $LIBS | sed -e 's/-lintl//'`
#		 nls_cv_header_intl=intl/libintl.h
#		 nls_cv_header_libgt=intl/libgettext.h
#              fi
            ])
	  fi
        fi

        if test "$CATOBJEXT" = "NONE"; then
	  dnl Neither gettext nor catgets in included in the C library.
	  dnl Fall back on GNU gettext library.
	  nls_cv_use_gnu_gettext=yes
        fi
      fi

      if test "$nls_cv_use_gnu_gettext" != "yes"; then
        AC_DEFINE(ENABLE_NLS)
#      else
#        dnl Mark actions used to generate GNU NLS library.
#        INTLOBJS="\$(GETTOBJS)"
#        AM_PATH_PROG_WITH_TEST(MSGFMT, msgfmt,
#	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep 'dv '`"], msgfmt)
#        AC_PATH_PROG(GMSGFMT, gmsgfmt, $MSGFMT)
#        AM_PATH_PROG_WITH_TEST(XGETTEXT, xgettext,
#	  [test -z "`$ac_dir/$ac_word -h 2>&1 | grep '(HELP)'`"], :)
#        AC_SUBST(MSGFMT)
#	USE_INCLUDED_LIBINTL=yes
#        CATOBJEXT=.gmo
#        INSTOBJEXT=.mo
#        DATADIRNAME=share
#	INTLDEPS='$(top_srcdir)/intl/libintl.a'
#	INTLLIBS=$INTLDEPS
#	LIBS=`echo $LIBS | sed -e 's/-lintl//'`
#        nls_cv_header_intl=intl/libintl.h
#        nls_cv_header_libgt=intl/libgettext.h
      fi

      dnl Test whether we really found GNU xgettext.
      if test "$XGETTEXT" != ":"; then
	dnl If it is no GNU xgettext we define it as : so that the
	dnl Makefiles still can work.
	if $XGETTEXT --omit-header /dev/null 2> /dev/null; then
	  : ;
	else
	  AC_MSG_RESULT(
	    [found xgettext program is not GNU xgettext; ignore it])
	  XGETTEXT=":"
	fi
      fi

      # We need to process the po/ directory.
      POSUB=po
    else
      DATADIRNAME=share
      nls_cv_header_intl=intl/libintl.h
      nls_cv_header_libgt=intl/libgettext.h
    fi
    AC_LINK_FILES($nls_cv_header_libgt, $nls_cv_header_intl)
    AC_OUTPUT_COMMANDS(
     [case "$CONFIG_FILES" in *po/Makefile.in*)
        sed -e "/POTFILES =/r po/POTFILES" po/Makefile.in > po/Makefile
      esac])


#    # If this is used in GNU gettext we have to set USE_NLS to `yes'
#    # because some of the sources are only built for this goal.
#    if test "$PACKAGE" = gettext; then
#      USE_NLS=yes
#      USE_INCLUDED_LIBINTL=yes
#    fi

    dnl These rules are solely for the distribution goal.  While doing this
    dnl we only have to keep exactly one list of the available catalogs
    dnl in configure.in.
    for lang in $ALL_LINGUAS; do
      GMOFILES="$GMOFILES $lang.gmo"
      POFILES="$POFILES $lang.po"
    done

    if test "$CATOBJEXT" = "NONE"; then
      CATOBJEXT=""
    fi
              
    dnl Make all variables we use known to autoconf.
    AC_SUBST(USE_INCLUDED_LIBINTL)
    AC_SUBST(CATALOGS)
    AC_SUBST(CATOBJEXT)
    AC_SUBST(DATADIRNAME)
    AC_SUBST(GMOFILES)
    AC_SUBST(INSTOBJEXT)
    AC_SUBST(INTLDEPS)
    AC_SUBST(INTLLIBS)
    AC_SUBST(INTLOBJS)
    AC_SUBST(POFILES)
    AC_SUBST(POSUB)
  ])

AC_DEFUN(AM_GNU_GETTEXT,
  [AC_REQUIRE([AC_PROG_MAKE_SET])dnl
   AC_REQUIRE([AC_PROG_CC])dnl
   AC_REQUIRE([AC_PROG_RANLIB])dnl
   AC_REQUIRE([AC_ISC_POSIX])dnl
   AC_REQUIRE([AC_HEADER_STDC])dnl
   AC_REQUIRE([AC_C_CONST])dnl
   AC_REQUIRE([AC_C_INLINE])dnl
   AC_REQUIRE([AC_TYPE_OFF_T])dnl
   AC_REQUIRE([AC_TYPE_SIZE_T])dnl
   AC_REQUIRE([AC_FUNC_ALLOCA])dnl
   AC_REQUIRE([AC_FUNC_MMAP])dnl

   AC_CHECK_HEADERS([argz.h limits.h locale.h nl_types.h malloc.h string.h \
unistd.h sys/param.h])
   AC_CHECK_FUNCS([getcwd munmap putenv setenv setlocale strchr strcasecmp \
strdup __argz_count __argz_stringify __argz_next])

   if test "${ac_cv_func_stpcpy+set}" != "set"; then
     AC_CHECK_FUNCS(stpcpy)
   fi
   if test "${ac_cv_func_stpcpy}" = "yes"; then
     AC_DEFINE(HAVE_STPCPY)
   fi

   AM_LC_MESSAGES
   AM_WITH_NLS

   if test "x$CATOBJEXT" != "x"; then
     if test "x$ALL_LINGUAS" = "x"; then
       LINGUAS=
     else
       AC_MSG_CHECKING(for catalogs to be installed)
       NEW_LINGUAS=
       for lang in ${LINGUAS=$ALL_LINGUAS}; do
         case "$ALL_LINGUAS" in
          *$lang*) NEW_LINGUAS="$NEW_LINGUAS $lang" ;;
         esac
       done
       LINGUAS=$NEW_LINGUAS
       AC_MSG_RESULT($LINGUAS)
     fi

     dnl Construct list of names of catalog files to be constructed.
     if test -n "$LINGUAS"; then
       for lang in $LINGUAS; do CATALOGS="$CATALOGS $lang$CATOBJEXT"; done
     fi
   fi

   dnl The reference to <locale.h> in the installed <libintl.h> file
   dnl must be resolved because we cannot expect the users of this
   dnl to define HAVE_LOCALE_H.
   if test $ac_cv_header_locale_h = yes; then
     INCLUDE_LOCALE_H="#include <locale.h>"
   else
     INCLUDE_LOCALE_H="\
/* The system does not provide the header <locale.h>.  Take care yourself.  */"
   fi
   AC_SUBST(INCLUDE_LOCALE_H)

   dnl Determine which catalog format we have (if any is needed)
   dnl For now we know about two different formats:
   dnl   Linux libc-5 and the normal X/Open format
   test -d intl || mkdir intl
   if test "$CATOBJEXT" = ".cat"; then
     AC_CHECK_HEADER(linux/version.h, msgformat=linux, msgformat=xopen)

     dnl Transform the SED scripts while copying because some dumb SEDs
     dnl cannot handle comments.
     sed -e '/^#/d' $srcdir/intl/$msgformat-msg.sed > intl/po2msg.sed
   fi
   dnl po2tbl.sed is always needed.
   sed -e '/^#.*[^\\]$/d' -e '/^#$/d' \
     $srcdir/intl/po2tbl.sed.in > intl/po2tbl.sed

   dnl In the intl/Makefile.in we have a special dependency which makes
   dnl only sense for gettext.  We comment this out for non-gettext
   dnl packages.
   if test "$PACKAGE" = "gettext"; then
     GT_NO="#NO#"
     GT_YES=
   else
     GT_NO=
     GT_YES="#YES#"
   fi
   AC_SUBST(GT_NO)
   AC_SUBST(GT_YES)

   dnl If the AC_CONFIG_AUX_DIR macro for autoconf is used we possibly
   dnl find the mkinstalldirs script in another subdir but ($top_srcdir).
   dnl Try to locate is.
   MKINSTALLDIRS=
   if test -n "$ac_aux_dir"; then
     MKINSTALLDIRS="$ac_aux_dir/mkinstalldirs"
   fi
   if test -z "$MKINSTALLDIRS"; then
     MKINSTALLDIRS="\$(top_srcdir)/mkinstalldirs"
   fi
   AC_SUBST(MKINSTALLDIRS)

   dnl *** For now the libtool support in intl/Makefile is not for real.
   l=
   AC_SUBST(l)

   dnl Generate list of files to be processed by xgettext which will
   dnl be included in po/Makefile.
   test -d po || mkdir po
   if test "x$srcdir" != "x."; then
     if test "x`echo $srcdir | sed 's@/.*@@'`" = "x"; then
       posrcprefix="$srcdir/"
     else
       posrcprefix="../$srcdir/"
     fi
   else
     posrcprefix="../"
   fi
   rm -f po/POTFILES
   sed -e "/^#/d" -e "/^\$/d" -e "s,.*,	$posrcprefix& \\\\," -e "\$s/\(.*\) \\\\/\1/" \
	< $srcdir/po/POTFILES.in > po/POTFILES
  ])


# Search path for a program which passes the given test.
# Ulrich Drepper <drepper@cygnus.com>, 1996.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

dnl AM_PATH_PROG_WITH_TEST(VARIABLE, PROG-TO-CHECK-FOR,
dnl   TEST-PERFORMED-ON-FOUND_PROGRAM [, VALUE-IF-NOT-FOUND [, PATH]])
AC_DEFUN(AM_PATH_PROG_WITH_TEST,
[# Extract the first word of "$2", so it can be a program name with args.
set dummy $2; ac_word=[$]2
AC_MSG_CHECKING([for $ac_word])
AC_CACHE_VAL(ac_cv_path_$1,
[case "[$]$1" in
  /*)
  ac_cv_path_$1="[$]$1" # Let the user override the test with a path.
  ;;
  *)
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_dir in ifelse([$5], , $PATH, [$5]); do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if [$3]; then
	ac_cv_path_$1="$ac_dir/$ac_word"
	break
      fi
    fi
  done
  IFS="$ac_save_ifs"
dnl If no 4th arg is given, leave the cache variable unset,
dnl so AC_PATH_PROGS will keep looking.
ifelse([$4], , , [  test -z "[$]ac_cv_path_$1" && ac_cv_path_$1="$4"
])dnl
  ;;
esac])dnl
$1="$ac_cv_path_$1"
if test -n "[$]$1"; then
  AC_MSG_RESULT([$]$1)
else
  AC_MSG_RESULT(no)
fi
AC_SUBST($1)dnl
])

# Check whether LC_MESSAGES is available in <locale.h>.
# Ulrich Drepper <drepper@cygnus.com>, 1995.
#
# This file can be copied and used freely without restrictions.  It can
# be used in projects which are not available under the GNU Public License
# but which still want to provide support for the GNU gettext functionality.
# Please note that the actual code is *not* freely available.

# serial 1

AC_DEFUN(AM_LC_MESSAGES,
  [if test $ac_cv_header_locale_h = yes; then
    AC_CACHE_CHECK([for LC_MESSAGES], am_cv_val_LC_MESSAGES,
      [AC_TRY_LINK([#include <locale.h>], [return LC_MESSAGES],
       am_cv_val_LC_MESSAGES=yes, am_cv_val_LC_MESSAGES=no)])
    if test $am_cv_val_LC_MESSAGES = yes; then
      AC_DEFINE(HAVE_LC_MESSAGES)
    fi
  fi])
