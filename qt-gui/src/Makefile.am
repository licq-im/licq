
EXTRA_DIST = xpm

#hack, FIXME this in a clean manner
libdir=$(prefix)/lib/licq

INCLUDES = -D_REENTRANT \
	-I$(top_srcdir)/../../src/inc $(X_CFLAGS) $(QT_INCLUDES) $(KDE_INCLUDES)

lib_LTLIBRARIES = licq_qt-gui.la

noinst_HEADERS = adduserdlg.h authuserdlg.h awaymsgdlg.h refusedlg.h \
	chatdlg.h editgrp.h editfile.h eventdesc.h ewidgets.h \
	filedlg.h gui-defines.h icqfunctions.h licq_qt-gui.conf.h licqgui.h \
	mainwin.h messagebox.h mledit.h optionsdlg.h outputwin.h passworddlg.h\
	plugindlg.h registeruser.h searchuserdlg.h \
	securitydlg.h showawaymsgdlg.h sigman.h skin.h skinbrowser.h userbox.h\
	utilitydlg.h wharf.h randomchatdlg.h forwarddlg.h chatjoin.h \
	mmlistview.h

licq_qt_gui_la_SOURCES = adduserdlg.cpp authuserdlg.cpp awaymsgdlg.cpp \
	refusedlg.cpp chatdlg.cpp editgrp.cpp editfile.cpp eventdesc.cpp \
	ewidgets.cpp filedlg.cpp icqfunctions.cpp \
	licqgui.cpp mainwin.cpp messagebox.cpp mledit.cpp optionsdlg.cpp \
	outputwin.cpp passworddlg.cpp plugindlg.cpp \
	registeruser.cpp searchuserdlg.cpp securitydlg.cpp showawaymsgdlg.cpp \
	sigman.cpp skin.cpp skinbrowser.cpp userbox.cpp utilitydlg.cpp \
	wharf.cpp randomchatdlg.cpp forwarddlg.cpp chatjoin.cpp mmlistview.cpp

licq_qt_gui_la_LDFLAGS = $(X_LIBS) $(QT_LDFLAGS) $(KDE_LDFLAGS) \
	-module -avoid-version -rpath $(libdir) -R$(QT_LIBDIR)
licq_qt_gui_la_LIBADD = $(KDE_LIBS) -lqt $(X_PRE_LIBS) -lX11 -lXext \
	$(XSS_LIBS) $(X_EXTRA_LIBS) 

METASOURCES = AUTO

