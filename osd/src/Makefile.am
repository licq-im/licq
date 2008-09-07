AM_CPPFLAGS = -Wall @WEXTRA@

libdir=@libdir@/licq/
localedir=$(datadir)/locale

AM_CXXFLAGS =  -I$(LICQ_INCLUDES) -I.. @LIBXOSD_CFLAGS@ -DLOCALEDIR=\"$(localedir)\"
noinst_HEADERS = my_xosd.h licq_osd.conf.h
lib_LTLIBRARIES = licq_osd.la

licq_osd_la_SOURCES = licq-osd.cpp my_xosd.cpp

licq_osd_la_LDFLAGS = -module -avoid-version
licq_osd_la_LIBADD =  @LIBXOSD_LIBS@
