SUBDIRS = daemon

EXTRA_DIST = inc

INCLUDES = -Wall -I$(srcdir)/inc -DINSTALL_PREFIX=\"@prefix@\" @SOCKS_INCDIR@

noinst_HEADERS = licq.conf.h licq.h

bin_PROGRAMS = licq
licq_SOURCES = licq.cpp main.cpp
licq_LDADD = $(top_builddir)/src/daemon/liblicq-daemon.la @SOCKS_LIBS@ 
licq_LDFLAGS = -export-dynamic @SOCKS_LIBDIR@
