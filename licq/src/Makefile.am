AM_CPPFLAGS = -Wall @WEXTRA@ -I$(top_srcdir)/include @SOCKS_INCDIR@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LIBS = @LTLIBINTL@ @LIBS@

noinst_HEADERS = licq.conf.h licq.h  \
	 time-fix.h sar.conf.h support.h

bin_PROGRAMS = licq
licq_SOURCES = licq.cpp main.cpp \
	 icqpacket.cpp proxy.cpp socket.cpp icqd.cpp \
	 icqd-tcp.cpp icqd-srv.cpp icqd-threads.cpp oscarservice.cpp \
	 icqevent.cpp buffer.cpp user.cpp history.cpp \
	 utility.cpp countrycodes.c log.cpp translate.cpp \
	 file.cpp message.cpp support.c mutex.cpp \
	 onevent.cpp plugind.cpp sar.cpp languagecodes.c \
	 icqd-chat.cpp sighandler.c icqd-filetransfer.cpp \
	 hebrev.c icqcolor.cpp fifo.cpp  protoplugind.cpp gpg.cpp \
	 occupationcodes.c homepagecodes.c interestcodes.c \
	 organizationcodes.c backgroundcodes.c providers.c rtf.cc md5.cpp

licq_LDADD = @SOCKS_LIBS@ 
licq_LDFLAGS = -export-dynamic @SOCKS_LIBDIR@ @LTLIBICONV@
