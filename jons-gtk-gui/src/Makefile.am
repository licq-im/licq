CXXFLAGS += -fno-rtti -fno-exceptions
LDFLAGS += -shared -nostdlib
INCLUDES += -I$(top_srcdir) -I$(includedir) -DG_LOG_DOMAIN=\"licq_jons-gtk-gui\" -I../../../src/inc `gtk-config --cflags`

bin_PROGRAMS = licq_jons-gtk-gui.so

licq_jons_gtk_gui_so_SOURCES = \
	add_user_window.cpp \
	auth_user_window.cpp \
	away_window.cpp \
	contact_list.cpp \
	convo.cpp \
	main.cpp \
	main_window.cpp \
	menu.cpp \
	pipe.cpp \
	status.cpp \
	system_message.cpp \
	system_status.cpp \
	user_menu.cpp 

licq_jons_gtk_gui_so_LDFLAGS += `gtk-config --libs`
licq_jons_gtk_gui_so_LDADD = 
