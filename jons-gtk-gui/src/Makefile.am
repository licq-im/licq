CXXFLAGS += -fno-rtti -fno-exceptions
LDFLAGS += -shared -nostdlib
INCLUDES += -I$(top_srcdir) -I$(includedir) \
		-DG_LOG_DOMAIN=\"licq_gtk\" -I../../../src/inc
CFLAGS += `gtk-config --cflags`

bin_PROGRAMS = licq_gtk.so

licq_gtk_so_SOURCES = \
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

licq_gtk_so_LDFLAGS += `gtk-config --libs`
licq_gtk_so_LDADD = 
