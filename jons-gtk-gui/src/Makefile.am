
#hack, fix this in a clean manner
libdir = $(prefix)/lib/licq

INCLUDES = -Wall -fno-rtti -fno-exceptions -D_REENTRANT \
	-I$(prefix)/include/licq -DG_LOG_DOMAIN=\"licq_jons-gtk-gui\" \
	`gtk-config --cflags`

lib_LTLIBRARIES = licq_jons-gtk-gui.la

noinst_HEADERS = licq_gtk.h

licq_jons_gtk_gui_la_SOURCES =  about_window.cpp \
				add_user_window.cpp \
				auth_user_window.cpp \
				away_window.cpp \
				chat_window.cpp \
				contact_list.cpp \
				convo.cpp \
				extras.cpp \
				file_window.cpp \
				history_window.cpp \
				key_request.cpp \
				log_window.cpp \
				main.cpp \
				main_window.cpp \
				menu.cpp \
				more_window.cpp \
				option_window.cpp \
				pipe.cpp \
				plugin_window.cpp \
				random_chat.cpp \
				register_user.cpp \
				search_user_window.cpp \
				security_window.cpp \
				status.cpp \
				system_message.cpp \
				system_status.cpp \
				user_menu.cpp \
				user_info_window.cpp 

licq_jons_gtk_gui_la_LDFLAGS = -s -module -avoid-version -rpath $(libdir) \
	 `gtk-config --libs`

