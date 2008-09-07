AM_CPPFLAGS = -Wall @WEXTRA@

libdir=@libdir@/licq/
plugindir = $(libdir)
licq_incdir = @LICQ_INCLUDES@


INCLUDES = -I$(srcdir) -I$(licq_incdir)

noinst_HEADERS = msn.h msnpacket.h msnbuffer.h msnevent.h

lib_LTLIBRARIES = protocol_msn.la

protocol_msn_la_SOURCES = plugin.cpp msn.cpp msn-ssl.cpp msn-sb.cpp \
                          msn-srv.cpp msnpacket.cpp msnbuffer.cpp msnevent.cpp

protocol_msn_la_LDFLAGS = -module -avoid-version
protocol_msn_la_LIBADD =

