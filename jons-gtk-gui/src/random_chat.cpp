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

#include <gtk/gtk.h>

#include "licq_gtk.h"
#include "licq_icqd.h"
#include "licq_icq.h"

struct random_chat *rcw;
struct random_chat *src;

void random_chat_search_window()
{
	// Only one window
	if(rcw != NULL)
	{
		gdk_window_raise(rcw->window->window);
		return;
	}

	rcw = g_new0(struct random_chat, 1);
	rcw->etag = g_new0(struct e_tag_data, 1);

	// Create the window
	rcw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(rcw->window), "Random Chat Search");

	// Connect the destroy event
	gtk_signal_connect(GTK_OBJECT(rcw->window), "destroy",
		GTK_SIGNAL_FUNC(dialog_close), rcw->window);

	// The table for all the widgets
	GtkWidget *table = gtk_table_new(3, 2, false);
	gtk_container_add(GTK_CONTAINER(rcw->window), table);

	// Label and a drop down combo box to select the group to search in
	GtkWidget *label = gtk_label_new("Search Group:");
	rcw->group_list = gtk_combo_new();

	// Attach them to the table
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL,
		GTK_FILL, 3, 3);
	gtk_table_attach(GTK_TABLE(table), rcw->group_list, 1, 2, 0, 1,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		3, 3);

	// Populate the list
	GList *items = NULL;
	items = g_list_append(items, const_cast<char *>("General"));
	items = g_list_append(items, const_cast<char *>("Romance"));
	items = g_list_append(items, const_cast<char *>("Games"));
	items = g_list_append(items, const_cast<char *>("Students"));
	items = g_list_append(items, const_cast<char *>("20 Something"));
	items = g_list_append(items, const_cast<char *>("30 Something"));
	items = g_list_append(items, const_cast<char *>("40 Something"));
	items = g_list_append(items, const_cast<char *>("50 Plus"));
	items = g_list_append(items, const_cast<char *>("Men Seeking Women"));
	items = g_list_append(items, const_cast<char *>("Women Seeking Men"));

	// Set the list to these items
	gtk_combo_set_popdown_strings(GTK_COMBO(rcw->group_list), items);

	// We don't want the list to be edited!
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(rcw->group_list)->entry),
		false);

	// Now the "Search" and "Close" buttons in a box
	GtkWidget *h_box = gtk_hbox_new(true, 0);
	rcw->search = gtk_button_new_with_label("Search");
	GtkWidget *cancel = gtk_button_new_with_label("Cancel");
	GtkWidget *close = gtk_button_new_with_label("Close");
	
	// Pack the buttons and attach the box to the table
	gtk_box_pack_start(GTK_BOX(h_box), rcw->search, true, true, 5);
	gtk_box_pack_start(GTK_BOX(h_box), cancel, true, true, 5);
	gtk_box_pack_start(GTK_BOX(h_box), close, true, true, 5);
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 1, 2,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		3, 3);

	// Connect their signals
	gtk_signal_connect(GTK_OBJECT(rcw->search), "clicked",
		GTK_SIGNAL_FUNC(random_search_callback), NULL);
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
		GTK_SIGNAL_FUNC(random_cancel_callback), NULL);
	gtk_signal_connect(GTK_OBJECT(close), "clicked",
		GTK_SIGNAL_FUNC(random_close_callback), NULL);

	// Statusbar
	rcw->etag->statusbar = gtk_statusbar_new();
	strcpy(rcw->etag->buf, "");
	gtk_table_attach(GTK_TABLE(table), rcw->etag->statusbar, 0, 2, 2, 3,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		3, 3);

	// Show everything now
	gtk_widget_show_all(rcw->window);
}

void random_search_callback(GtkWidget *widget, gpointer data)
{
	// Only search once
	gtk_widget_set_sensitive(rcw->search, false);

	// Get the group we are searching in
	unsigned long group = ICQ_RANDOMxCHATxGROUP_NONE;
	const gchar *search = gtk_entry_get_text(GTK_ENTRY(
		GTK_COMBO(rcw->group_list)->entry));

	if(strcmp(search, "General") == 0)
		group = ICQ_RANDOMxCHATxGROUP_GENERAL;
	else if(strcmp(search, "Romance") == 0)
		group = ICQ_RANDOMxCHATxGROUP_ROMANCE;
	else if(strcmp(search, "Games") == 0)
		group = ICQ_RANDOMxCHATxGROUP_GAMES;
	else if(strcmp(search, "Students") == 0)
		group = ICQ_RANDOMxCHATxGROUP_STUDENTS;
	else if(strcmp(search, "20 Something") == 0)
		group = ICQ_RANDOMxCHATxGROUP_20SOME;
	else if(strcmp(search, "30 Something") == 0)
		group = ICQ_RANDOMxCHATxGROUP_30SOME;
	else if(strcmp(search, "40 Something") == 0)
		group = ICQ_RANDOMxCHATxGROUP_40SOME;
	else if(strcmp(search, "50 Plus") == 0)
		group = ICQ_RANDOMxCHATxGROUP_50PLUS;
	else if(strcmp(search, "Men Seeking Women") == 0)
		group = ICQ_RANDOMxCHATxGROUP_MxSEEKxF;
	else if(strcmp(search, "Women Seeking Men") == 0)
		group = ICQ_RANDOMxCHATxGROUP_FxSEEKxM;

	// Search now.. and wait to catch the return
	rcw->etag->e_tag = icq_daemon->icqRandomChatSearch(group);

	// We'll want to catch it
	catcher = g_slist_append(catcher, rcw->etag);

	// Change the statusbar text
	guint id = gtk_statusbar_get_context_id(
		GTK_STATUSBAR(rcw->etag->statusbar), "id");
	gtk_statusbar_pop(GTK_STATUSBAR(rcw->etag->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(rcw->etag->statusbar), id,
		"Searching for random chat partner ... ");
	strcpy(rcw->etag->buf, "Searching for random chat partner ... ");
}

void random_cancel_callback(GtkWidget *widget, gpointer data)
{
	// You may search again
	gtk_widget_set_sensitive(rcw->search, true);

	// Cancel the event and screw catching it
	icq_daemon->CancelEvent(rcw->etag->e_tag);
	catcher = g_slist_remove(catcher, rcw->etag);
}

void random_close_callback(GtkWidget *widget, gpointer data)
{
	catcher = g_slist_remove(catcher, rcw->etag);
	gtk_widget_destroy(rcw->window);
	g_free(rcw->etag);
	rcw->etag = 0;
	g_free(rcw);
	rcw = 0;
}

void set_random_chat_window()
{
	// Only one window
	if(src != NULL)
	{
		gdk_window_raise(src->window->window);
		return;
	}

	src = g_new0(struct random_chat, 1);
	src->etag = g_new0(struct e_tag_data, 1);

	// Make the window
	src->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(src->window), "Set Random Chat Group");

	// The combo box
	src->group_list = gtk_combo_new();

	GList *list = NULL;

	list = g_list_append(list, const_cast<char *>("(None)"));
	list = g_list_append(list, const_cast<char *>("General"));
	list = g_list_append(list, const_cast<char *>("Romance"));
	list = g_list_append(list, const_cast<char *>("Games"));
	list = g_list_append(list, const_cast<char *>("Students"));
	list = g_list_append(list, const_cast<char *>("20 Something"));
	list = g_list_append(list, const_cast<char *>("30 Something"));
	list = g_list_append(list, const_cast<char *>("40 Something"));
	list = g_list_append(list, const_cast<char *>("50 Plus"));
	list = g_list_append(list, const_cast<char *>("Men Seeking Women"));
	list = g_list_append(list, const_cast<char *>("Women Seeking Men"));

	gtk_combo_set_popdown_strings(GTK_COMBO(src->group_list), list);

	// Set the entry so it can't be editable
	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(src->group_list)->entry),
		false);

	// Set it to show the current group
	ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
	char current_group[18];
	switch(o->RandomChatGroup())
	{
		case ICQ_RANDOMxCHATxGROUP_GENERAL:
			strcpy(current_group, "General");  break;
		case ICQ_RANDOMxCHATxGROUP_ROMANCE:
			strcpy(current_group, "Romance");  break;
		case ICQ_RANDOMxCHATxGROUP_GAMES:
			strcpy(current_group, "Games");  break;
		case ICQ_RANDOMxCHATxGROUP_STUDENTS:
			strcpy(current_group, "Students");  break;
		case ICQ_RANDOMxCHATxGROUP_20SOME:
			strcpy(current_group, "20 Something");  break;
		case ICQ_RANDOMxCHATxGROUP_30SOME:
			strcpy(current_group, "30 Something");  break;
		case ICQ_RANDOMxCHATxGROUP_40SOME:
			strcpy(current_group, "40 Something");  break;
		case ICQ_RANDOMxCHATxGROUP_50PLUS:
			strcpy(current_group, "50 Plus");  break;
		case ICQ_RANDOMxCHATxGROUP_MxSEEKxF:
			strcpy(current_group, "Men Seeking Women");  break;
		case ICQ_RANDOMxCHATxGROUP_FxSEEKxM:
			strcpy(current_group, "Women Seeking Men"); break;
		default:
			strcpy(current_group, "(None)");
	}
	gUserManager.DropOwner();
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(src->group_list)->entry),
		current_group);


	// Create a table and attach it
	GtkWidget *table = gtk_table_new(2, 3, false);
	gtk_container_add(GTK_CONTAINER(src->window), table);
	gtk_table_attach(GTK_TABLE(table), src->group_list, 0, 2, 0, 1,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		3, 3);

	// Set and close buttons in a hbox
	src->search = gtk_button_new_with_label("Set");
	src->close = gtk_button_new_with_label("Close");
	GtkWidget *h_box = gtk_hbox_new(true, 0);
	gtk_box_pack_start(GTK_BOX(h_box), src->search, true, true, 5);
	gtk_box_pack_start(GTK_BOX(h_box), src->close, true, true, 5);

	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 1, 2,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		3, 3);

	gtk_signal_connect(GTK_OBJECT(src->search), "clicked",
		GTK_SIGNAL_FUNC(set_random_set_callback), (gpointer)src);
	gtk_signal_connect(GTK_OBJECT(src->close), "clicked",
		GTK_SIGNAL_FUNC(set_random_close_callback), (gpointer)src);

		
	// Statusbar
	src->etag->statusbar = gtk_statusbar_new();
	strcpy(src->etag->buf, "");
	gtk_table_attach(GTK_TABLE(table), src->etag->statusbar, 0, 2, 2, 3,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		3, 3);

	gtk_widget_show_all(src->window);
}

void set_random_set_callback(GtkWidget *widget, gpointer data)
{
	unsigned long group = ICQ_RANDOMxCHATxGROUP_NONE;

	const gchar *szGroup = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(
		src->group_list)->entry));

	if(strcmp(szGroup, "General") == 0)
		group = ICQ_RANDOMxCHATxGROUP_NONE;
	else if(strcmp(szGroup, "Romance") == 0)
		group = ICQ_RANDOMxCHATxGROUP_ROMANCE;
	else if(strcmp(szGroup, "Games") == 0)
		group = ICQ_RANDOMxCHATxGROUP_GAMES;
	else if(strcmp(szGroup, "Students") == 0)
		group = ICQ_RANDOMxCHATxGROUP_STUDENTS;
	else if(strcmp(szGroup, "20 Something") == 0)
		group = ICQ_RANDOMxCHATxGROUP_20SOME;
	else if(strcmp(szGroup, "30 Something") == 0)
		group = ICQ_RANDOMxCHATxGROUP_30SOME;
	else if(strcmp(szGroup, "40 Something") == 0)
		group = ICQ_RANDOMxCHATxGROUP_40SOME;
	else if(strcmp(szGroup, "50 Plus") == 0)
		group = ICQ_RANDOMxCHATxGROUP_50PLUS;
	else if(strcmp(szGroup, "Men Seeking Women") == 0)
		group = ICQ_RANDOMxCHATxGROUP_MxSEEKxF;
	else if(strcmp(szGroup, "Women Seeking Men") == 0)
		group = ICQ_RANDOMxCHATxGROUP_FxSEEKxM;

	src->etag->e_tag = icq_daemon->icqSetRandomChatGroup(group);

	catcher = g_slist_append(catcher, src->etag);

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(
		src->etag->statusbar), "id");
	gtk_statusbar_pop(GTK_STATUSBAR(src->etag->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(src->etag->statusbar), id,
		"Setting random chat group ... ");
	strcpy(src->etag->buf, "Setting random chat group ... ");
}

void set_random_close_callback(GtkWidget *widget, gpointer data)
{
	catcher = g_slist_remove(catcher, src->etag);
	gtk_widget_destroy(src->window);
	g_free(src->etag);
	src->etag = 0;
	g_free(src);
	src = 0;
}
