#HACK, FIXME this in a clean manner
libdir = $(prefix)/lib/licq
plugindir = $(libdir)
licq_incdir = @LICQ_INCLUDES@

INCLUDES = -Wall -fno-rtti -fno-exceptions -I$(srcdir) -I$(licq_incdir)

noinst_HEADERS = autoreply.h eventdesc.h

lib_LTLIBRARIES = licq_autoreply.la

licq_autoreply_la_SOURCES = autoreply.cpp eventdesc.cpp main.cpp

licq_autoreply_la_LDFLAGS = -s -module -avoid-version -rpath $(plugindir)

