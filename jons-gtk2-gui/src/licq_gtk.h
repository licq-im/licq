/*
 * Licq GTK GUI Plugin
 *
 * Copyright (C) 2000, Jon Keating <jon@licq.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __LICQ_GTK_H
#define __LICQ_GTK_H

#include "licq_icqd.h"

#include <gtk/gtk.h>

/* Program used constants */
const int MAX_LENGTH_UIN = 10;

struct e_tag_data
{
	GtkWidget *statusbar;
	gchar buf[60];
	unsigned long e_tag;
};

/******************* Global Variables ******************/

/* Globals in contact_list.cpp */
extern GdkColor *red, *blue, *online_color, *offline_color, *away_color;
extern GdkPixbuf *online, *away, *na, *dnd, *occ, *offline, *ffc,
	*invisible, *message_icon, *file_icon, *chat_icon, *url_icon,
	*secure_icon, *birthday_icon, *securebday_icon, *blank_icon,
	*charset_icon;
extern void stop_flashing(ICQUser *u);

/* Globals in main.cpp */
extern GtkWidget *main_window;
extern CICQDaemon *icq_daemon;
extern gint _pipe;
extern GSList *catcher;

/* Globals in main_window.cpp */
extern GtkWidget *contact_list;
extern GtkWidget *vertical_box;

/* Globals in option_window.cpp */
extern bool show_offline_users;
extern bool show_ignored_users;
extern bool show_convo_timestamp;
extern bool recv_colors;
extern char timestamp_format[50];
extern bool enter_sends;
extern bool flash_events;
extern unsigned long auto_logon;
extern bool remember_window_pos;
extern unsigned long auto_away_time;
extern unsigned long auto_na_time;
extern unsigned long auto_offline_time;

/* Globals for window dimensions / location */
extern short int windowX;
extern short int windowY;
extern short int windowH;
extern short int windowW;

/* Globals in tray.cpp */
extern GObject *trayicon;


/********************* Functions ******************/

/* Functions in add_user_window.cpp */
void
window_close(GtkWidget *widget, GtkWidget *destroy);
void
destroy_cb(GtkWidget *widget, gpointer **p);

/* Functions in auth_user_window.cpp */
void menu_system_auth_user(GtkWidget *, const unsigned long);

/* Functions in chat_window.cpp */
void chat_accept_window(CEventChat *, gulong, bool auto_accept = false);
void chat_accept(GtkWidget *, gpointer);

/* Functions in contact_list.cpp */
void contact_list_refresh();
gint flash_icons(gpointer);

/* Functions in convo.cpp */
void convo_open(ICQUser *user, bool refresh);

/* Functions in extras.cpp */
void do_colors();
void do_pixmaps();
void message_box(const char *);
void verify_numbers(GtkEditable *, gchar *, gint, gint *, gpointer);
void finish_info(CICQSignal *);

/* Functions in file_window.cpp */
void file_accept_window(ICQUser *, CUserEvent *, bool auto_accept = false);

/* Functions in main.cpp */
const char *config_file();

/* Functions in main_window.cpp */
GtkWidget *main_window_new(const gchar *);
void main_window_show();

/* Functions in menu.cpp */
GtkWidget *menu_new_item(GtkWidget *, const char *, GtkSignalFunc, 
		bool sensitive = true);
GtkWidget *menu_new_item_with_pixmap(GtkWidget *, const char *,
		GtkSignalFunc, GdkPixbuf *, gpointer data = NULL);
void menu_create();
void menu_separator(GtkWidget *_menu);

/* Functions in option_window.cpp */
void load_options();
void save_window_pos(void);

/* Functions in pipe.cpp */
void pipe_callback(gpointer, gint, GdkInputCondition);

/* Functions in status.cpp */
void status_bar_refresh();

/* Functions in system_status.cpp */
void system_status_refresh();

/* Functions in user_info_window.cpp */
void list_info_user(GtkWidget *, ICQUser *);
#endif /* __LICQ_GTK_H */
