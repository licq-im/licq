#ifndef __LICQ_GTK_H
#define __LICQ_GTK_H

#include "icqd.h"
#include "icqevent.h"
#include "user.h"

#include <gtk/gtk.h>

#define NAME		"GTK Plugin"
#define PLUGIN_VERSION	"0.10"
#define STATUS		"Running"
#define USAGE		"None yet"
#define DESCRIPTION	"GTK+ plugin for licq 0.75"

/********** Structures ******************/

struct conversation
{
	GtkWidget *window;
	GtkWidget *entry;
	GtkWidget *text;
	GtkWidget *send_server;
	GtkWidget *spoof_button;
	GtkWidget *spoof_uin;
	GtkWidget *progress;
	gchar prog_buf[60];
	ICQUser *user;
	CICQEventTag *e_tag;
};

struct send_url
{
	GtkWidget *window;
	GtkWidget *entry_u;
	GtkWidget *entry_d;
	GtkWidget *send_server;
	GtkWidget *spoof_button;
	GtkWidget *spoof_uin;
	ICQUser *user;
};

struct info_user
{
	GtkWidget *window;
	ICQUser *user;
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

struct main_progress
{
	CICQEventTag *e_tag;
	gchar buffer[55];
};

struct more_window
{
	GtkWidget *window;
	GtkWidget *notify;
	GtkWidget *visible;
	GtkWidget *invisible;
	GtkWidget *ignore;
	ICQUser *user;
};

struct history
{
	GtkWidget *text;
	GtkWidget *check;
	ICQUser *user;
};


/******************* Global Variables ******************/

/* Globals in away_window.cpp */


/* Globals in contact_list.cpp */
extern GdkColor *red, *green, *blue;
extern GdkBitmap *bm;
extern GtkStyle *style;
extern GdkPixmap *online, *offline, *away, *na, *dnd, *occ, *message;


/* Globals in convo.cpp */
extern GList *cnv;


/* Globals in history_window.cpp */
extern const gchar *line;


/* Globals in main.cpp */
extern GtkWidget *main_window;
extern CICQDaemon *icq_daemon;
extern gint _pipe;


/* Globals in main_window.cpp */
extern GtkWidget *vertical_box;
extern GtkWidget *contact_list;
extern GtkWidget *status_progress;
extern GList *m_prog_list;


/* Globals in menu.cpp */
extern GtkWidget *menu;
extern GtkWidget *user_list_menu;


/* Globals in pipe.cpp */


/* Globals in status.cpp */
extern GtkWidget *_status_menu;


/* Globals in system_status.cpp */
extern GtkWidget *system_status;

/********************* Functions ******************/

/* Functions in add_user_window.cpp */
extern void menu_system_add_user(GtkWidget *, gpointer);
extern void add_user_callback(GtkWidget *, struct add_user *);
extern void dialog_close(GtkWidget *, GtkWidget *);


/* Functions in auth_user_window.cpp */
extern void menu_system_auth_user(GtkWidget *, const unsigned long);
extern void auth_user_callback(GtkWidget *, struct auth_user *);


/* Functions in away_window.cpp */
extern void away_msg_window(gushort);
extern void set_away_msg(GtkWidget *, struct away_dialog *);
extern void away_close(GtkWidget *, GtkWidget *);


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
extern void convo_send(GtkWidget *, struct conversation *);
extern void convo_recv(gulong);
extern void spoof_button_callback(GtkWidget *, struct conversation *);
extern gboolean convo_close(GtkWidget *, struct conversation *);
extern gboolean convo_delete_event(GtkWidget *,GdkEventAny *,struct conversation *);


/* Functions in extras.cpp */
extern void do_colors();
extern void do_pixmaps();
extern void verify_numbers(GtkEditable *, gchar *, gint, gint *, gpointer);
extern void user_function(ICQEvent *);
extern void check_event(ICQEvent *, GtkWidget *, guint &, gchar *);
extern void check_other_event(ICQEvent *, GtkWidget *, guint &);


/* Functions in history_window.cpp */
extern void list_history(GtkWidget *, ICQUser *);
extern void reverse_history(GtkWidget *, struct history *);


/* Functions in main_window.cpp */
extern GtkWidget *main_window_new(const gchar *, gint, gint);
extern void main_window_show();


/* Functions in menu.cpp */
extern GtkWidget *menu_new_item(GtkWidget *, const char *, GtkSignalFunc);
extern void menu_create();
extern void menu_system_quit(GtkWidget *, gpointer);
extern void menu_system_refresh(GtkWidget *, gpointer);


/* Functions in more_window.cpp */
extern void list_more_window(GtkWidget *, ICQUser *);
extern void more_ok_callback(GtkWidget *, struct more_window *);


/* Functions in pipe.cpp */
extern void pipe_callback(gpointer, gint, GdkInputCondition);
extern void pipe_signal(CICQSignal *);
extern void pipe_event(ICQEvent *);


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
extern void status_change(gushort);


/* Functions in system_message.cpp */
extern void system_message_window();


/* Functions in system_status.cpp */
extern GtkWidget *system_status_new(gint, gint, gint);
extern void system_status_refresh();
extern void system_status_click(GtkWidget *, GdkEventButton *, gpointer);


/* Functions in user_info_window.cpp */
extern void list_info_user(GtkWidget *, ICQUser *);
extern void update_user_info(GtkWidget *, struct info_user *);
extern void do_entry(GtkWidget *&, GtkWidget *&, const gchar *, const gchar *);
extern void pack_hbox(GtkWidget *&, GtkWidget *, GtkWidget *);


/* Function in user_menu.cpp */
extern void list_start_convo(GtkWidget *, ICQUser *);
extern void list_send_url(GtkWidget *, ICQUser *);
extern void url_send(GtkWidget *, struct send_url *);
extern void url_close(GtkWidget *, struct send_url *);
extern void url_spoof_button_callback(GtkWidget *, struct send_url *);
extern void list_delete_user(GtkWidget *, ICQUser *);
extern void delete_user_callback(GtkWidget *, struct delete_user *);
extern void destroy_dialog(GtkWidget *, gpointer);


#endif /* __LICQ_GTK_H */
