INCLUDES = -Wall -W -I$(top_srcdir)/src/inc -I$(top_srcdir)/src @SOCKS_INCDIR@

noinst_LTLIBRARIES = liblicq-daemon.la
liblicq_daemon_la_SOURCES = icqpacket.cpp socket.cpp icqd.cpp icqd-udp.cpp \
         icqd-tcp.cpp icqd-threads.cpp remoteserver.cpp \
			   icqevent.cpp buffer.cpp user.cpp history.cpp \
			   utility.cpp countrycodes.c log.cpp translate.cpp \
			   file.cpp message.cpp support.c pthread_rdwr.c \
			   onevent.cpp plugind.cpp sar.cpp languagecodes.c

liblicq_daemon_la_LDFLAGS = -static
