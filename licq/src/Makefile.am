#EXTRA_DIST = inc

#INCLUDES = -Wall -I$(srcdir)/inc -DINSTALL_PREFIX=\"@prefix@\" @SOCKS_INCDIR@
INCLUDES = -Wall -I$(top_srcdir)/include -DINSTALL_PREFIX=\"@prefix@\" @SOCKS_INCDIR@

noinst_HEADERS = licq.conf.h licq.h  \
         time-fix.h sar.conf.h support.h

bin_PROGRAMS = licq
licq_SOURCES = licq.cpp main.cpp \
         icqpacket.cpp proxy.cpp socket.cpp icqd.cpp \
         icqd-tcp.cpp icqd-srv.cpp icqd-threads.cpp \
	 icqevent.cpp buffer.cpp user.cpp history.cpp \
	 utility.cpp countrycodes.c log.cpp translate.cpp \
	 file.cpp message.cpp support.c pthread_rdwr.c \
	 onevent.cpp plugind.cpp sar.cpp languagecodes.c \
         icqd-chat.cpp sighandler.c icqd-filetransfer.cpp \
         hebrev.c icqcolor.cpp

#licq_LDADD = $(top_builddir)/src/daemon/liblicq-daemon.la @SOCKS_LIBS@ 
licq_LDADD = @SOCKS_LIBS@ 
licq_LDFLAGS = -export-dynamic @SOCKS_LIBDIR@
