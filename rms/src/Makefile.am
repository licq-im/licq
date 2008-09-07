AM_CPPFLAGS = -Wall @WEXTRA@

libdir=@libdir@/licq/
plugindir = $(libdir)
licq_incdir = @LICQ_INCLUDES@


INCLUDES = -I$(srcdir) -I$(licq_incdir)

noinst_HEADERS = rms.h

lib_LTLIBRARIES = licq_rms.la

licq_rms_la_SOURCES = rms.cpp main.cpp

licq_rms_la_LDFLAGS = -module -avoid-version
licq_rms_la_LIBADD = 

