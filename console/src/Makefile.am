#HACK, FIXME this in a clean manner
libdir = $(prefix)/lib/licq/
plugindir = $(libdir)
licq_incdir = @LICQ_INCLUDES@


INCLUDES = -Wall -fno-rtti -fno-exceptions -I$(srcdir) -I$(licq_incdir)

noinst_HEADERS = console.h event_data.h window.h

lib_LTLIBRARIES = licq_console.la

licq_console_la_SOURCES = console.cpp console_menu.cpp console_print.cpp \
	console_tab.cpp main.cpp window.cpp


licq_console_la_LDFLAGS = -s -module -avoid-version 
