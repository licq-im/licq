#HACK, FIXME this in a clean manner
libdir = $(prefix)/lib/licq/
plugindir = $(libdir)
licq_incdir = @LICQ_INCLUDES@


INCLUDES = -Wall -fno-rtti -fno-exceptions -I$(srcdir) -I$(licq_incdir)

noinst_HEADERS = forwarder.h

lib_LTLIBRARIES = licq_forwarder.la

licq_forwarder_la_SOURCES = forwarder.cpp main.cpp

licq_forwarder_la_LDFLAGS = -s -module -avoid-version -rpath $(plugindir)
licq_forwarder_la_LIBADD = -lncurses

