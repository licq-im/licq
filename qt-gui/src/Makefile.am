EXTRA_DIST = xpm

#hack, FIXME this in a clean manner
libdir=$(prefix)/lib/licq
licq_incdir = $(LICQ_INCLUDES)

AM_CPPFLAGS = -D_REENTRANT \
	-I$(licq_incdir) $(X_CFLAGS) $(QT_INCLUDES) $(KDE_INCLUDES)

EXTRA_LTLIBRARIES = licq_qt-gui.la licq_kde-gui.la

lib_LTLIBRARIES = @LIB_NAME@

noinst_HEADERS = adduserdlg.h authuserdlg.h awaymsgdlg.h refusedlg.h \
	chatdlg.h editgrp.h editfile.h eventdesc.h ewidgets.h \
	filedlg.h gui-defines.h licq_qt-gui.conf.h licqgui.h \
	mainwin.h messagebox.h mledit.h mlview.h mlview2.h mlview3.h optionsdlg.h outputwin.h\
	plugindlg.h registeruser.h searchuserdlg.h \
	securitydlg.h showawaymsgdlg.h sigman.h skin.h skinbrowser.h userbox.h\
	utilitydlg.h wharf.h randomchatdlg.h forwarddlg.h chatjoin.h \
	mmlistview.h mmsenddlg.h userinfodlg.h usereventdlg.h keyrequestdlg.h \
	jfcstyle.h usercodec.h reqauthdlg.h licqdialog.h userselectdlg.h \
	editfilelistdlg.h emoticon.h

licq_gui = adduserdlg.cpp authuserdlg.cpp awaymsgdlg.cpp \
	refusedlg.cpp chatdlg.cpp editgrp.cpp editfile.cpp eventdesc.cpp \
	ewidgets.cpp filedlg.cpp \
	licqgui.cpp mainwin.cpp messagebox.cpp mledit.cpp mlview2.cpp mlview3.cpp optionsdlg.cpp \
	outputwin.cpp plugindlg.cpp \
	registeruser.cpp searchuserdlg.cpp securitydlg.cpp showawaymsgdlg.cpp \
	sigman.cpp skin.cpp skinbrowser.cpp userbox.cpp utilitydlg.cpp \
	wharf.cpp randomchatdlg.cpp forwarddlg.cpp chatjoin.cpp mmlistview.cpp \
	mmsenddlg.cpp userinfodlg.cpp usereventdlg.cpp keyrequestdlg.cpp \
	jfcstyle.cpp usercodec.cpp reqauthdlg.cpp licqdialog.cpp userselectdlg.cpp \
	editfilelistdlg.cpp emoticon.cpp

licq_qt_gui_la_SOURCES = $(licq_gui)
licq_kde_gui_la_SOURCES = $(licq_gui) wrap_kde_malloc.cpp

licq_qt_gui_la_LDFLAGS = $(X_LIBS) $(QT_LDFLAGS) \
	-module -avoid-version -rpath $(libdir) -R$(QT_LIBDIR) 
licq_qt_gui_la_LIBADD = $(LIB_QT) $(X_PRE_LIBS) -lX11 -lXext \
	$(XSS_LIBS) $(X_EXTRA_LIBS)

licq_kde_gui_la_LDFLAGS = $(KDE_LDFLAGS) $(QT_LDFLAGS) $(X_LIBS) \
	-module -avoid-version -rpath $(libdir) -R$(QT_LIBDIR)
licq_kde_gui_la_LIBADD = $(KDE_LIBS) $(X_PRE_LIBS) -lX11 -lXext \
	$(XSS_LIBS) $(X_EXTRA_LIBS)

METASOURCES = AUTO
KDE_OPTIONS = qtonly
