
#hack, fix this in a clean manner
libdir = $(prefix)/lib/licq

INCLUDES = -Wall -pedantic -fno-rtti -fno-exceptions -D_REENTRANT \
	-I$(top_srcdir)/../../src/inc -DG_LOG_DOMAIN=\"licq_jons-gtk-gui\" \
	`gtk-config --cflags`

lib_LTLIBRARIES = licq_jons-gtk-gui.la

noinst_HEADERS = licq_gtk.h

licq_jons_gtk_gui_la_SOURCES = add_user_window.cpp auth_user_window.cpp \
	away_window.cpp contact_list.cpp convo.cpp main.cpp main_window.cpp \
	menu.cpp pipe.cpp status.cpp system_message.cpp system_status.cpp \
	user_menu.cpp

licq_jons_gtk_gui_la_LDFLAGS = -s -module -avoid-version -rpath $(libdir) \
	 `gtk-config --libs`

