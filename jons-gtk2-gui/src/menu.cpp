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

#include "licq_gtk.h"

#include <gtk/gtk.h>

void menu_separator(GtkWidget *);
void menu_system_quit(GtkWidget *, gpointer);
void menu_system_refresh(GtkWidget *, gpointer);

void create_about_window();
void menu_system_add_user(GtkWidget *, gpointer);
void log_window_show(GtkWidget *, gpointer);
void menu_options_create();
void create_plugin_window();
void random_chat_search_window();
void set_random_chat_window();
void search_user_window();
void menu_security_users_window(GtkWidget *, gpointer);
void menu_daemon_stats();
void gtk_not_implemented(GtkWidget *, gpointer);

GtkWidget *menu;
GtkWidget *user_list_menu;

GtkWidget *menu_new_item(GtkWidget *_menu, const char *str,
	GtkSignalFunc s_func, bool sensitive)
{
	GtkWidget *menu_item;
	menu_item = gtk_menu_item_new_with_label(str);

	/* If menu is passed into this function, append menu_item to menu */
	if(_menu)
		gtk_menu_shell_append(GTK_MENU_SHELL(_menu), menu_item);

	if (!sensitive)
		gtk_widget_set_sensitive(menu_item, false);
		
	gtk_widget_show(menu_item);

	/* If s_func is passed into this function, connect it to the object */
	if(s_func)
		/* Cast s_func WHEN it is passed in, not here */
		g_signal_connect(G_OBJECT(menu_item), "activate", s_func, 0);

	return menu_item;
}

GtkWidget *
menu_new_item_with_pixmap(GtkWidget *_menu, const char *text,
		GtkSignalFunc s_func, GdkPixbuf *icon, gpointer data)
{
	GtkWidget *h_box = gtk_hbox_new(false, 0);

	GtkWidget *pixmap = gtk_image_new_from_pixbuf(icon);
	GtkWidget *label = gtk_label_new(text);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);

	gtk_box_pack_start(GTK_BOX(h_box), pixmap, false, false, 3);
	gtk_box_pack_start(GTK_BOX(h_box), label, true, true, 0);

	GtkWidget *menu_item = gtk_menu_item_new();
	gtk_container_add(GTK_CONTAINER(menu_item), h_box);

	gtk_widget_show_all(menu_item);

	gtk_menu_shell_append(GTK_MENU_SHELL(_menu), menu_item);
	
	if (s_func)
		g_signal_connect(G_OBJECT(menu_item), "activate",
			G_CALLBACK(s_func), data);
	
	return menu_item;
}

void menu_separator(GtkWidget *_menu)
{
	GtkWidget *separator;
	GtkWidget *menu_item;
	
	separator = gtk_hseparator_new();
	menu_item = gtk_menu_item_new();

	gtk_menu_shell_append(GTK_MENU_SHELL(_menu), menu_item);
	gtk_container_add(GTK_CONTAINER(menu_item), separator);
	gtk_widget_set_sensitive(menu_item, FALSE);
	
	gtk_widget_show(menu_item);
	gtk_widget_show(separator);
}
	
void menu_create()
{
	GtkWidget *item;
	GtkWidget *system_item;
	GtkWidget *menu_bar;
	GtkWidget *sub_menu;
	

	/* This is the main menu...  do not show it */
	menu = gtk_menu_new();

	/* System functions sub menu here */
	sub_menu = gtk_menu_new();
	gtk_widget_show(sub_menu);

	item = menu_new_item(menu, "System Functions", 0);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub_menu);
	gtk_widget_show(item);

	item = menu_new_item(sub_menu, "Edit Info",
		GTK_SIGNAL_FUNC(list_info_user));

	item = menu_new_item(sub_menu, "Set Random Group",
		GTK_SIGNAL_FUNC(set_random_chat_window));

	// User functions sub menu here
	sub_menu = gtk_menu_new();
	gtk_widget_show(sub_menu);

	item = menu_new_item(menu, "User Functions", 0);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub_menu);
	gtk_widget_show(item);

	item = menu_new_item(sub_menu, "Add User",
			     GTK_SIGNAL_FUNC(menu_system_add_user));
 
	item = menu_new_item(sub_menu, "Authorize User",
			     GTK_SIGNAL_FUNC(menu_system_auth_user));

	item = menu_new_item(sub_menu, "Search For User",
			     GTK_SIGNAL_FUNC(search_user_window));

	item = menu_new_item(sub_menu, "Random Chat",
			     GTK_SIGNAL_FUNC(random_chat_search_window));

	/* Groups Menu */

	// User functions sub menu here
	sub_menu = gtk_menu_new();
	gtk_widget_show(sub_menu);

	item = menu_new_item(menu, "Groups", 0, 0);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), sub_menu);
	gtk_widget_show(item);

	item = menu_new_item(sub_menu, "All User",
		    	 GTK_SIGNAL_FUNC(gtk_not_implemented));

	menu_separator(sub_menu);

			// Groups Here

	menu_separator(sub_menu);

	item = menu_new_item(sub_menu, "Online Notify",
		    	 GTK_SIGNAL_FUNC(gtk_not_implemented));

	item = menu_new_item(sub_menu, "Visible List",
		    	 GTK_SIGNAL_FUNC(gtk_not_implemented));

	item = menu_new_item(sub_menu, "Invisible List",
		    	 GTK_SIGNAL_FUNC(gtk_not_implemented));

	item = menu_new_item(sub_menu, "Ignore List",
		    	 GTK_SIGNAL_FUNC(gtk_not_implemented));	

	item = menu_new_item(sub_menu, "New Users",
		    	 GTK_SIGNAL_FUNC(gtk_not_implemented));	
		     

	/* The rest of the menu options */
	item = menu_new_item(menu, "Options",
			     GTK_SIGNAL_FUNC(menu_options_create));

	item = menu_new_item(menu, "Security Options",
			     GTK_SIGNAL_FUNC(menu_security_users_window));

	item = menu_new_item(menu, "Plugin Manager",
			     GTK_SIGNAL_FUNC(create_plugin_window));

	item = menu_new_item(menu, "Network Log",
				GTK_SIGNAL_FUNC(log_window_show));

	item = menu_new_item(menu, "Refresh",
				GTK_SIGNAL_FUNC(menu_system_refresh));

	menu_separator(menu);

	item = menu_new_item(menu, "Daemon Stats",
				GTK_SIGNAL_FUNC(menu_daemon_stats));

	item = menu_new_item(menu, "About",
				GTK_SIGNAL_FUNC(create_about_window));

	item = menu_new_item(menu, "Exit",
				  GTK_SIGNAL_FUNC(menu_system_quit));

	/* The root menu text for the System menu */
	system_item = menu_new_item(0, "System", 0);
	gtk_widget_show(system_item);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(system_item), menu);

	/* The menu bar to place the mens */
	menu_bar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vertical_box), menu_bar, FALSE, FALSE, 0);
	gtk_widget_show(menu_bar);

	/* Append the "System" to the menu bar */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), system_item);
}

void
menu_system_quit(GtkWidget *blah, gpointer data)
{
	save_window_pos();
	gtk_main_quit();
} 

void
menu_system_refresh(GtkWidget *window, gpointer data)
{
	contact_list_refresh();
}

void
gtk_not_implemented(GtkWidget *blah, gpointer data)
{ 
 GtkWidget *dialog;

  dialog = gtk_message_dialog_new(NULL, 
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, 
     	GTK_BUTTONS_OK, "%s", "Not Implemented");
  g_signal_connect(G_OBJECT (dialog), "response", 
     G_CALLBACK(gtk_widget_destroy), 0);
  gtk_widget_show(dialog);
}