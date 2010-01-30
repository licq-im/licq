AM_CPPFLAGS = -Wall @WEXTRA@

libdir=@libdir@/licq/
plugindir = $(libdir)
licq_incdir = @LICQ_INCLUDES@


INCLUDES = -I$(srcdir) -I$(licq_incdir)

noinst_HEADERS = forwarder.h

lib_LTLIBRARIES = licq_forwarder.la

licq_forwarder_la_SOURCES = forwarder.cpp main.cpp

licq_forwarder_la_LDFLAGS = -module -avoid-version

