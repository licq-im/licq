AM_CPPFLAGS = -Wall @WEXTRA@

libdir=@libdir@/licq/
plugindir = $(libdir)
licq_incdir = @LICQ_INCLUDES@

INCLUDES = -I$(srcdir) -I$(licq_incdir)

noinst_HEADERS = autoreply.h

lib_LTLIBRARIES = licq_autoreply.la

licq_autoreply_la_SOURCES = autoreply.cpp main.cpp

licq_autoreply_la_LDFLAGS = -module -avoid-version

