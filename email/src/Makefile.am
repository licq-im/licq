#HACK, FIXME this in a clean manner
libdir = $(prefix)/lib/licq/
plugindir = $(libdir)
licq_incdir = @LICQ_INCLUDES@


INCLUDES = -Wall -fno-rtti -fno-exceptions -I$(srcdir) -I$(licq_incdir)

noinst_HEADERS = email.h eventdesc.h

lib_LTLIBRARIES = licq_email.la

licq_email_la_SOURCES = email.cpp eventdesc.cpp main.cpp

licq_email_la_LDFLAGS = -s -module -avoid-version -rpath $(plugindir)
licq_email_la_LIBADD = -lncurses

