SUBDIRS = daemon

EXTRA_DIST = inc

INCLUDES = -Wall -I$(srcdir)/inc -DINSTALL_PREFIX=\"@prefix@\" @SOCKS_INCDIR@

noinst_HEADERS = licq.conf.h licq.h

bin_PROGRAMS = licq
licq_SOURCES = licq.cpp main.cpp
licq_LDADD = $(srcdir)/daemon/liblicq-daemon.la @SOCKS_LIBS@
licq_LDFLAGS = -export-dynamic @SOCKS_LIBDIR@

install-data-local:
	@echo "==============================================================="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=  Licq installation finished. Now go to the plugins/         ="
	@echo "=  subdir and configure, make and install the Qt GUI plugin!  ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "=                                                             ="
	@echo "==============================================================="
