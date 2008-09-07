AM_CPPFLAGS = -Wall @WEXTRA@

libdir=@libdir@/licq/
plugindir = $(libdir)
licq_incdir = @LICQ_INCLUDES@


INCLUDES = -I$(srcdir) -I$(licq_incdir) -I/usr/include/ncurses -I/usr/include/cdk -I/usr/local/include/cdk

noinst_HEADERS = console.h event_data.h window.h

lib_LTLIBRARIES = licq_console.la

licq_console_la_SOURCES = console.cpp console_menu.cpp console_print.cpp \
	console_tab.cpp main.cpp window.cpp


licq_console_la_LDFLAGS = -module -avoid-version
