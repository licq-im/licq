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
#include "licq_events.h"
#include "licq_filetransfer.h"
#include "licq_log.h"
#include "licq_chat.h"
#include "licq_user.h"

#include <sys/time.h>
#include <gtk/gtk.h>
#include <fstream.h>

/* Definitions to be passed to the licq daemon */
#define NAME		"Jon's GTK+ GUI"
#define PLUGIN_VERSION	"0.10"
#define STATUS		"Running"
#define USAGE		"Usage: None yet"
#define DESCRIPTION	"GTK+ plugin for licq"

/* Program used definitions */
#define MAX_LENGTH_UIN	8

/********** Structures ******************/

struct conversation
{
	GtkWidget *window;
	GtkWidget *entry;
	GtkWidget *text;
	GtkWidget *send;
	GtkWidget *cancel;
	GtkWidget *send_server;
	GtkWidget *send_normal;
	GtkWidget *send_urgent;
	GtkWidget *send_list;
	GtkWidget *progress;
	gchar prog_buf[60];
	gchar *for_user;
	ICQUser *user;
	struct e_tag_data *etag;
};

struct send_url
{
	GtkWidget *window;
	GtkWidget *entry_u;
	GtkWidget *entry_d;
	GtkWidget *send;
	GtkWidget *cancel;
	GtkWidget *send_server;
	GtkWidget *send_normal;
	GtkWidget *send_urgent;
	GtkWidget *send_list;
	ICQUser *user;
	struct e_tag_data *etag;
};

struct info_user
{
	GtkWidget *window;
	GtkWidget *alias;
	GtkWidget *name;
	GtkWidget *email1;
	GtkWidget *email2;
	GtkWidget *address;
	GtkWidget *city;
	GtkWidget *state;
	GtkWidget *zip;
	GtkWidget *country;
	GtkWidget *phone;
	GtkWidget *age;
	GtkWidget *gender;
	GtkWidget *homepage;
	GtkWidget *bday;
	GtkWidget *lang1;
	GtkWidget *lang2;
	GtkWidget *lang3;
	GtkWidget *company;
	GtkWidget *dept;
	GtkWidget *pos;
	GtkWidget *co_homepage;
	GtkWidget *co_address;
	GtkWidget *co_phone;
	GtkWidget *co_city;
	GtkWidget *co_state;
	GtkWidget *about;
	GtkWidget *update;
	GtkWidget *cancel;
	ICQUser *user;
	struct e_tag_data *etag;
};

struct system_message
{
	GtkWidget *window;
	GtkWidget *text;
};

struct add_user
{
	GtkWidget *window;
	GtkWidget *entry;
	GtkWidget *check_box;
};

struct delete_user
{
	GtkWidget *window;
	ICQUser *user;
};

struct auth_user
{
	GtkWidget *window;
	GtkWidget *entry;
	GtkWidget *text;
};

struct away_dialog
{
	GtkWidget *window;
	GtkWidget *text;
};

struct system_window
{
	GtkWidget *window;
	GtkWidget *text;
};

struct more_window
{
	GtkWidget *window;
	GtkWidget *notify;
	GtkWidget *autochat;
	GtkWidget *autofile;
	GtkWidget *realip;
	GtkWidget *visible;
	GtkWidget *invisible;
	GtkWidget *ignore;
	GtkWidget *accept_away;
	GtkWidget *accept_na;
	GtkWidget *accept_dnd;
	GtkWidget *accept_occ;
	GtkWidget *online_to;
	GtkWidget *away_to;
	GtkWidget *na_to;
	GtkWidget *dnd_to;
	GtkWidget *occ_to;
	GtkWidget *custom_check;
	GtkWidget *custom_text;
	ICQUser *user;
};

struct history
{
	GtkWidget *text;
	GtkWidget *check;
	ICQUser *user;
};

struct user_away_window
{
	GtkWidget *window;
	GtkWidget *show_again;
	GtkWidget *text_box;
	ICQUser *user;
	GtkWidget *progress;
	gchar buffer[30];
	struct e_tag_data *etag;
};

struct file_accept
{
	GtkWidget *window;
	GtkWidget *window2;
	ICQUser *user;
	CUserEvent *e;
	GtkWidget *text;  /* This is for the refuse part... */
};

struct file_send
{
	GtkWidget *window;		// Window
	GtkWidget *description;		// File description
	GtkWidget *file_path;		// File path to send
	GtkWidget *browse;		// Browse for new file
	GtkWidget *ok;			// Send
	GtkWidget *cancel;		// Cancel before it is sent
	GtkWidget *send_normal;		// Send it normally
	GtkWidget *send_urgent;		// Send it urgently
	GtkWidget *send_list;		// Send it to their list
	GtkWidget *file_select;		// File selection widget

	/* Internals */
	gulong uin;
	struct e_tag_data *etd;
};

struct file_window
{
	/* File sending/receiving internals */
	CFileTransferManager *ftman;
	gulong uin;
	gint input_tag;
	gulong sequence;

	/* For the window */
	GtkWidget *window;
	GtkWidget *current_file_name;
	GtkWidget *total_files;
	GtkWidget *local_file_name;
	GtkWidget *batch;
	GtkWidget *batch_progress;
	GtkWidget *batch_size;
	GtkWidget *progress;
	GtkWidget *file_size;
	GtkWidget *time;
	GtkWidget *bps;
	GtkWidget *eta;
	GtkWidget *status;
	GtkWidget *cancel;
};

struct search_user
{
	GtkWidget *window;
	GtkWidget *label;
	GtkWidget *nick_name;
	GtkWidget *first_name;
	GtkWidget *last_name;
	GtkWidget *email;
	GtkWidget *uin;
	GtkWidget *list;
	struct e_tag_data *etag;
};

struct network_window
{
	GtkWidget *window;
	GtkWidget *text;
};

struct security_window
{
        GtkWidget *window;
        GtkWidget *check_auth;
        GtkWidget *check_web;
        GtkWidget *check_hideip;
        GtkTooltips *tooltips;
	struct e_tag_data *etag;
};

struct user_security
{
	GtkWidget *window;
	GtkWidget *check_auth;
	GtkWidget *check_web;
	GtkWidget *check_hideip;
	GtkWidget *ign_new;
	GtkWidget *ign_web;
	GtkWidget *ign_mass;
	GtkWidget *ign_pager;
	gint page;
	struct e_tag_data *etag;
};

struct options_window
{
	GtkWidget *window;
	GtkWidget *show_ignored;
	GtkWidget *show_offline;
	GtkWidget *enter_sends;
};

struct remote_chat_request
{
	GtkWidget *dialog;
	gulong uin;
	CEventChat *c_event;
};

struct request_chat
{
	GtkWidget *window;
	GtkWidget *text_box;
	GtkWidget *send_norm;
	GtkWidget *send_urg;
	GtkWidget *send_list;
	GtkWidget *chat_list;
	ICQUser *user;
	struct e_tag_data *etd;
};

struct chat_window
{
	CChatManager *chatman;
	CChatUser *chat_user;
	CChatUser *hold_cuser;
	GtkWidget *window;
	GtkWidget *notebook;
	GtkWidget *table;
	GtkWidget *table_irc;
	GtkWidget *text_local;
	GtkWidget *text_remote;
	GtkWidget *text_irc;
	GtkWidget *entry_irc;
	GtkWidget *list_users;
	GtkWidget *frame_local;
	GtkWidget *frame_remote;
	GdkColor *back_color;
	GdkColor *fore_color;
	GdkFont *font_remote;
	gchar font_name[50];
	gint font_size;
	gboolean remote_bold;
	gboolean remote_italic;
	gboolean pane_mode;
	ICQUser *user;
	gboolean audio;
	gint last_pos;
	gint input_tag;
};

struct random_chat
{
	GtkWidget *window;
	GtkWidget *group_list;
	GtkWidget *search;
	GtkWidget *close;
	struct e_tag_data *etag;
};

struct key_request
{
	GtkWidget *window;
	GtkWidget *label_status;
	gboolean open;
	ICQUser *user;
	struct e_tag_data *etag;
};

struct e_tag_data
{
	GtkWidget *statusbar;
	gchar buf[60];
	CICQEventTag *e_tag;
};

struct status_icon
{
	GdkPixmap *pm;
	GdkBitmap *bm;
};

/******************* Global Variables ******************/

/* Globals in away_window.cpp */
extern GSList *uaw_list;

/* Globals in contact_list.cpp */
extern GdkColor *red, *blue, *online_color, *offline_color, *away_color;
extern struct status_icon *online, *away, *na, *dnd, *occ, *offline, *ffc,
	*invisible, *message_icon, *file_icon, *chat_icon, *url_icon,
	*secure_icon, *birthday_icon, *securebday_icon;

/* Globals in chat_window.cpp */
extern GSList *rc_list;

/* Globals in convo.cpp */
extern GSList *cnv;

/* Globals in history_window.cpp */
extern const gchar *line;

/* Globals in key_requst.cpp */
extern GSList *kr_list;

/* Globals in main.cpp */
extern GtkWidget *main_window;
extern CICQDaemon *icq_daemon;
extern gint _pipe;
extern CPluginLog *log;
extern gint log_pipe;
extern struct timeval timer;
extern GSList *catcher;

/* Globals in main_window.cpp */
extern GtkWidget *vertical_box;
extern GtkWidget *contact_list;
extern GtkWidget *status_progress;

/* Globals in menu.cpp */
extern GtkWidget *menu;
extern GtkWidget *user_list_menu;

/* Globals in option_window.cpp */
extern bool show_offline_users;
extern bool show_ignored_users;
extern bool enter_sends;

/* Globals in random_chat.cpp */
extern struct random_chat *rcw;
extern struct random_chat *src;

/* Globals in register_user.cpp */
extern GtkWidget *register_window;

/* Globals in status.cpp */
extern GtkWidget *_status_menu;

/* Globals in system_status.cpp */
extern GtkWidget *system_status;

/********************* Functions ******************/

/* Functions in about_window.cpp */
extern void create_about_window();
extern void close_about_window(GtkWidget *, gpointer);


/* Functions in add_user_window.cpp */
extern void menu_system_add_user(GtkWidget *, gpointer);
extern void add_user_callback(GtkWidget *, struct add_user *);
extern void dialog_close(GtkWidget *, GtkWidget *);


/* Functions in auth_user_window.cpp */
extern void menu_system_auth_user(GtkWidget *, const unsigned long);
extern void auth_user_grant(GtkWidget *, struct auth_user *);
extern void auth_user_refuse(GtkWidget *, struct auth_user *);


/* Functions in away_window.cpp */
extern void away_msg_window(gushort);
extern void set_away_msg(GtkWidget *, struct away_dialog *);
extern void away_close(GtkWidget *, GtkWidget *);
extern void list_read_message(GtkWidget *, ICQUser *);
extern void close_away_window(GtkWidget *, struct user_away_window *);
extern struct user_away_window *uaw_new(ICQUser *);
extern struct user_away_window *uaw_find(unsigned long);


/* Functions in chat_window.cpp */
extern void list_request_chat(GtkWidget *, ICQUser *);
extern struct request_chat *rc_new(ICQUser *);
extern struct request_chat *rc_find(gulong);
extern void multi_request_chat(GtkWidget *, gpointer);
extern void single_request_chat(GtkWidget *, gpointer);
extern void ok_request_chat(GtkWidget *, gpointer);
extern void cancel_request_chat(GtkWidget *, gpointer);
extern void close_request_chat(struct request_chat *);
extern void chat_accept_window(CEventChat *, gulong, bool auto_accept = false);
extern void chat_accept(GtkWidget *, gpointer);
extern void chat_refuse(GtkWidget *, gpointer);
extern void chat_join_multiparty(struct remote_chat_request *);
extern void chat_start_as_server(gulong, CEventChat *);
extern void chat_start_as_client(ICQEvent *);
extern struct chat_window *chat_window_create(gulong);
extern GtkWidget* chat_create_menu(struct chat_window *);
extern void chat_audio(gpointer, guint, GtkWidget *);
extern void chat_close(gpointer, guint, GtkWidget *);
extern void chat_pipe_callback(gpointer, gint, GdkInputCondition);
extern void chat_send(GtkWidget *, GdkEventKey *, struct chat_window *);
extern void chat_beep_users(gpointer, guint, GtkWidget *);
extern void chat_change_font(gpointer, guint, GtkWidget *);


/* Functions in contact_list.cpp */
extern GtkWidget *contact_list_new(gint, gint);
extern void contact_list_refresh();
extern void contact_list_order();
extern void contact_list_click(GtkWidget *, GdkEventButton *, gpointer);
extern void add_to_popup(const gchar *, GtkWidget *, GtkSignalFunc, ICQUser *);


/* Functions in convo.cpp */
extern struct conversation *convo_new(ICQUser *, gboolean);
extern struct conversation *convo_find(unsigned long);
extern void convo_show(struct conversation *);
extern void convo_send(GtkWidget *, gpointer);
extern gboolean key_press_convo(GtkWidget *, GdkEventKey *, gpointer);
extern void verify_convo_send(GtkWidget *, guint, gchar *,
			      struct conversation *);
extern void convo_cancel(GtkWidget *, struct conversation *);
extern void convo_recv(unsigned long);
extern gboolean convo_close(GtkWidget *, struct conversation *);


/* Functions in extras.cpp */
extern void do_colors();
extern void do_pixmaps();
extern struct status_icon *make_pixmap(struct status_icon *, gchar **);
extern void message_box(const char *);
extern void verify_numbers(GtkEditable *, gchar *, gint, gint *, gpointer);
extern void owner_function(ICQEvent *);
extern void user_function(ICQEvent *);
extern void finish_event(struct e_tag_data *, ICQEvent *);
extern void finish_message(ICQEvent *);
extern void finish_chat(ICQEvent *);
extern void finish_file(ICQEvent *);
extern void finish_away(ICQEvent *);
extern void finish_random(ICQEvent *);
extern void finish_secure(ICQEvent *);
extern void finish_info(CICQSignal *);


/* Functions in file_window.cpp */
extern void file_accept_window(ICQUser *, CUserEvent *, bool auto_accept = false);
extern void accept_file(GtkWidget *, gpointer);
extern void save_file(struct file_accept *);
extern void refuse_file(GtkWidget *, gpointer);
extern void refusal_ok(GtkWidget *, gpointer);
extern void cancel_file(GtkWidget *, gpointer);
extern void file_pipe_callback(gpointer, gint, GdkInputCondition);
extern void create_file_window(struct file_window *);
extern void update_file_info(struct file_window *);
extern gchar *encode_file_size(unsigned long);
extern void list_request_file(GtkWidget *, ICQUser *);
extern void fs_browse_click(GtkWidget *, gpointer);
extern void fs_ok_click(GtkWidget *, gpointer);
extern void fs_cancel_click(GtkWidget *, gpointer);
extern void file_select_ok(GtkWidget *, gpointer);
extern void file_select_cancel(GtkWidget *, gpointer);
extern struct file_send *fs_find(unsigned long);
extern void file_start_send(ICQEvent *);


/* Functions in history_window.cpp */
extern void list_history(GtkWidget *, ICQUser *);
extern void reverse_history(GtkWidget *, struct history *);


/* Functions in key_request.cpp */
extern void create_key_request_window(GtkWidget *, ICQUser *);
extern struct key_request *kr_find(gulong);
extern struct key_request *kr_new(ICQUser *);
extern void send_key_request(GtkWidget *, gpointer);
extern void close_key_request(GtkWidget *, gpointer);


/* Functions in log_window.cpp */
extern void new_log_window();
extern void log_window_show(GtkWidget *, gpointer);
extern void log_pipe_callback(gpointer, gint, GdkInputCondition);
extern gint log_window_close(GtkWidget *, GtkWidget *);
extern void log_window_clear(GtkWidget *, gpointer);


/* Functions in main_window.cpp */
extern GtkWidget *main_window_new(const gchar *, gint, gint);
extern void main_window_show();


/* Functions in menu.cpp */
extern GtkWidget *menu_new_item(GtkWidget *, const char *, GtkSignalFunc);
extern void menu_separator(GtkWidget *);
extern void menu_create();
extern void menu_system_quit(GtkWidget *, gpointer);
extern void menu_system_refresh(GtkWidget *, gpointer);


/* Functions in more_window.cpp */
extern void list_more_window(GtkWidget *, ICQUser *);
extern void more_ok_callback(GtkWidget *, struct more_window *);


/* Functions in option_window.cpp */
extern void menu_options_create();
extern void set_options(struct options_window *);
extern void done_options(GtkWidget *, gpointer);
extern void save_options();
extern void load_options();
extern void parse_line(char *, ifstream &);
extern void set_default_options();
extern void show_on_color_dlg(GtkWidget *, gpointer);
extern void color_dlg_ok(GtkWidget *, gpointer);
extern void color_dlg_cancel(GtkWidget *, gpointer);


/* Functions in pipe.cpp */
extern void pipe_callback(gpointer, gint, GdkInputCondition);
extern void pipe_signal(CICQSignal *);
extern void pipe_event(ICQEvent *);


/* Functions in random_chat.cpp */
extern void random_chat_search_window();
extern void random_search_callback(GtkWidget *, gpointer);
extern void random_cancel_callback(GtkWidget *, gpointer);
extern void random_close_callback(GtkWidget *, gpointer);
extern void set_random_chat_window();
extern void set_random_set_callback(GtkWidget *, gpointer);
extern void set_random_cancel_callback(GtkWidget *, gpointer);
extern void set_random_close_callback(GtkWidget *, gpointer);


/* Functions in register_user.cpp */
extern void registration_wizard();
extern void wizard_ok(GtkWidget *, gpointer);
extern void wizard_cancel(GtkWidget *, gpointer);
extern void current_button_callback(GtkWidget *, gpointer);
extern void wizard_message(int);


/* Functions in search_user_window.cpp */
extern void search_user_window();
extern void clear_callback(GtkWidget *, gpointer);
extern void search_callback(GtkWidget *, gpointer);
extern void search_list_double_click(GtkWidget *, GdkEventButton *, gpointer);
extern void search_result(ICQEvent *);
extern void search_done(bool);
extern void search_found(CSearchAck *);
extern void search_failed();
extern void search_close(GtkWidget *, gpointer);


/* Functions in security_window.cpp */
extern void menu_security_users_window(GtkWidget *, gpointer);
extern GtkWidget *make_user_security_clist();
extern void switch_page(GtkNotebook *, GtkNotebookPage *, gint, gpointer);
extern void refresh_clist(GtkCList *, gint);
extern void close_user_security_window(GtkWidget *, gpointer);
extern void ok_user_security(GtkWidget *, gpointer);
extern void remove_user_security(GtkWidget *, GdkEventButton *, gpointer);


/* Functions in status.cpp */
extern GtkWidget *status_bar_new(gint, gint, gint);
extern void status_bar_refresh();
extern gint status_popup_menu(GtkWidget *, GdkEventButton *);
extern void status_ffc(GtkWidget *, gpointer);
extern void status_online(GtkWidget *, gpointer);
extern void status_away(GtkWidget *, gpointer);
extern void status_na(GtkWidget *, gpointer);
extern void status_occ(GtkWidget *, gpointer);
extern void status_dnd(GtkWidget *, gpointer);
extern void status_off(GtkWidget *, gpointer);
extern void status_invisible(GtkWidget *, GtkWidget *);
extern void status_change(gushort);


/* Functions in system_message.cpp */
extern void system_message_window();


/* Functions in system_status.cpp */
extern GtkWidget *system_status_new(gint, gint, gint);
extern void system_status_refresh();
extern void system_status_click(GtkWidget *, GdkEventButton *, gpointer);


/* Functions in user_info_window.cpp */
extern void list_info_user(GtkWidget *, ICQUser *);
extern void user_info_save(GtkWidget *, struct info_user *);
extern gboolean user_info_close(GtkWidget *, struct info_user *);
extern void update_user_info(GtkWidget *, struct info_user *);
extern void cancel_user_info(GtkWidget *, struct info_user *);
extern void verify_user_info(GtkWidget *, guint, gchar *,  struct info_user *);
extern struct info_user *iu_new(ICQUser *);
extern struct info_user *iu_find(unsigned long);
extern void do_entry(GtkWidget *&, GtkWidget *&, const gchar *,
		     const gchar *, gboolean);
extern void pack_hbox(GtkWidget *&, GtkWidget *, GtkWidget *);


/* Function in user_menu.cpp */
extern void list_start_convo(GtkWidget *, ICQUser *);
extern void list_send_url(GtkWidget *, ICQUser *);
extern void url_send(GtkWidget *, struct send_url *);
extern void url_cancel(GtkWidget *, struct send_url *);
extern void url_close(GtkWidget *, struct send_url *);
extern void url_verified_close(GtkWidget *, guint, gchar*, struct send_url *);
extern void list_delete_user(GtkWidget *, ICQUser *);
extern void delete_user_callback(GtkWidget *, struct delete_user *);
extern void destroy_dialog(GtkWidget *, gpointer);


#endif /* __LICQ_GTK_H */
