/*
 * Licq GTK GUI Plugin
 *
 * Copyright (C) 2000, Jon Keating <jonkeating@norcom2000.com>
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

#include "icqd.h"
#include "user.h"

#include <gtk/gtk.h>
#include <stdlib.h>

struct search_user *su;
static gint num_found_users;

void search_user_window()
{
	/* Only one search window */
	if(su != NULL)
	{
		gdk_window_raise(su->window->window);
		return;
	}

	GtkWidget *notebook;
	GtkWidget *table;
	GtkWidget *table_in_nb;
	GtkWidget *label;
	GtkWidget *clear;
	GtkWidget *search;
	GtkWidget *v_box;
	GtkWidget *done;
	GtkWidget *scroll;

	su = (struct search_user *)g_new0(struct search_user, 1);

	num_found_users = 0;

	/* Create the window */
	su->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(su->window), "Licq - Search User");

	/* destroy event for window */
	gtk_signal_connect(GTK_OBJECT(su->window), "destroy",
			   GTK_SIGNAL_FUNC(search_close), su->window);

	/* Create the main table */
	table = gtk_table_new(3, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(su->window), table);

	/* Create the notebook */
	notebook = gtk_notebook_new();

	/* The table inside the name tab */
	table_in_nb = gtk_table_new(3, 2, FALSE);

	label = gtk_label_new("Nick Name:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 3);

	su->nick_name = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table_in_nb), su->nick_name, 1, 2, 0, 1,
			 GTK_FILL | GTK_EXPAND , GTK_FILL, 3, 3);

	label = gtk_label_new("First Name:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 1, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	su->first_name = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table_in_nb), su->first_name, 1, 2, 1, 2,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

	label = gtk_label_new("Last Name:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 2, 3,
			 GTK_FILL, GTK_FILL, 3, 3);

	su->last_name = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table_in_nb), su->last_name, 1, 2, 2, 3,
                         GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

	label = gtk_label_new("Name");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table_in_nb, label);

	/* A new tab (E-Mail Tab), a new table */
	table_in_nb = gtk_table_new(1, 2, FALSE);

	label = gtk_label_new("E-Mail Address:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 0, 1,
			 GTK_FILL, GTK_FILL, 3, 25);

	su->email = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table_in_nb), su->email, 1, 2, 0, 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 25);

	label = gtk_label_new("E-mail");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table_in_nb, label);

	/* Another new tab (UIN Tab), a new table */
	table_in_nb = gtk_table_new(1, 2, FALSE);

	label = gtk_label_new("UIN:");
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table_in_nb), label, 0, 1, 0, 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 5, 25);

	su->uin = gtk_entry_new_with_max_length(MAX_LENGTH_UIN);
	gtk_table_attach(GTK_TABLE(table_in_nb), su->uin, 1, 2, 0, 1,
			 GTK_FILL, GTK_FILL, 5, 25);

	/* Only numbers allowed in the uin entry box */
	gtk_signal_connect(GTK_OBJECT(su->uin), "insert-text",
			   GTK_SIGNAL_FUNC(verify_numbers), NULL);

	/* Append to the notebook */
	label = gtk_label_new("UIN");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table_in_nb, label);

	/* Add the notebook to the main table */
	gtk_table_attach(GTK_TABLE(table), notebook, 0, 1, 0, 2,
			 GTK_FILL, GTK_FILL, 3, 3);

	/* The v_box for the clear and search buttons */
	v_box = gtk_vbox_new(FALSE, 5);

	/* The "Search" button */
	search = gtk_button_new_with_label("Search");
	gtk_box_pack_start(GTK_BOX(v_box), search, TRUE, TRUE, 0);

	/* The search signal */
	gtk_signal_connect(GTK_OBJECT(search), "clicked",
			   GTK_SIGNAL_FUNC(search_callback), NULL);

	/* The "Clear List" button */
	clear = gtk_button_new_with_label("Clear List");
	gtk_box_pack_start(GTK_BOX(v_box), clear, TRUE, TRUE, 0);

	/* The clear signal */
        gtk_signal_connect(GTK_OBJECT(clear), "clicked",
                           GTK_SIGNAL_FUNC(clear_callback), NULL);
	/* The "Done" button */
	done = gtk_button_new_with_label("Done");
	gtk_box_pack_start(GTK_BOX(v_box), done, TRUE, TRUE, 0);

	/* The done signal */
	gtk_signal_connect(GTK_OBJECT(done), "clicked",
			   GTK_SIGNAL_FUNC(search_close), NULL);

	gtk_table_attach(GTK_TABLE(table), v_box, 2, 3, 1, 2,
                         0, 0, 3, 3);

	/* Found users list with a scroll bar */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);

	su->list = gtk_clist_new(4);
	gtk_clist_set_selection_mode(GTK_CLIST(su->list), GTK_SELECTION_BROWSE);
	gtk_clist_set_shadow_type(GTK_CLIST(su->list), GTK_SHADOW_ETCHED_IN);

	gtk_clist_column_titles_show(GTK_CLIST(su->list));
	gtk_clist_column_titles_passive(GTK_CLIST(su->list));

	/* Set the widths */
	gtk_clist_set_column_width(GTK_CLIST(su->list), 0, 70);
	gtk_clist_set_column_width(GTK_CLIST(su->list), 1, 65);
	gtk_clist_set_column_width(GTK_CLIST(su->list), 2, 100);
	gtk_clist_set_column_width(GTK_CLIST(su->list), 3, 100);

	/* Set the titles */
	gtk_clist_set_column_title(GTK_CLIST(su->list), 0, "UIN");
	gtk_clist_set_column_title(GTK_CLIST(su->list), 1, "Nick");
	gtk_clist_set_column_title(GTK_CLIST(su->list), 2, "Name");
	gtk_clist_set_column_title(GTK_CLIST(su->list), 3, "E-mail");

	/* Double click on a user */
	gtk_signal_connect(GTK_OBJECT(su->list), "button_press_event",
			   GTK_SIGNAL_FUNC(search_list_double_click), NULL);
 
	gtk_widget_set_usize(GTK_WIDGET(su->list), 230, 300);
	gtk_container_add(GTK_CONTAINER(scroll), su->list);

	gtk_table_attach(GTK_TABLE(table), scroll, 0, 3, 2, 3,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

	/* The label to keep track of the status */
	su->label = gtk_label_new("Enter Search Paramters");
	gtk_table_attach(GTK_TABLE(table), su->label, 0, 3, 3, 4,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

	gtk_widget_set_usize(GTK_WIDGET(su->window), 435, 465); 
	gtk_widget_show_all(su->window);
}

void clear_callback(GtkWidget *widget, gpointer data)
{
	num_found_users = 0;
	gtk_clist_clear(GTK_CLIST(su->list));
}

void search_callback(GtkWidget *widget, gpointer data)
{
	gtk_label_set_text(GTK_LABEL(su->label),
			   "Searching, this may take awhile.");

	gulong uin = 0;
	gchar *nick_name;
	gchar *first_name;
	gchar *last_name;
	gchar *email;

	uin = (gulong)(atol(gtk_editable_get_chars(GTK_EDITABLE(su->uin),
						   0, -1)));
	nick_name = gtk_editable_get_chars(GTK_EDITABLE(su->nick_name),
					   0, -1);
	first_name = gtk_editable_get_chars(GTK_EDITABLE(su->first_name),
					    0, -1);
	last_name = gtk_editable_get_chars(GTK_EDITABLE(su->last_name),
					   0, -1);
	email = gtk_editable_get_chars(GTK_EDITABLE(su->email),
				       0, -1);

	if(uin >= 1000)
		su->sequence = icq_daemon->icqSearchByUin(uin);
	else
		su->sequence = icq_daemon->icqSearchByInfo(nick_name,
						first_name, last_name,
						email);
}

void search_list_double_click(GtkWidget *widget,
			      GdkEventButton *eb,
			      gpointer data)
{
	gint row, column;
	gulong uin;
	ICQUser *user;

	/* Get which cell was clicked in */
	gtk_clist_get_selection_info(GTK_CLIST(su->list),
				     (gint)eb->x,
				     (gint)eb->y,
				     &row,
				     &column);

	if(!(eb->type == GDK_2BUTTON_PRESS && eb->button == 1))
		return;

	uin = (gulong)gtk_clist_get_row_data(GTK_CLIST(su->list), row);

	if((user = gUserManager.FetchUser(uin, LOCK_N)))
		return;

	icq_daemon->AddUserToList(uin);

	gchar *for_label = g_strdup_printf("User (%ld) added", uin);
	gtk_label_set_text(GTK_LABEL(su->label), for_label);
}

void search_result(ICQEvent *event)
{
	/* Make sure it's the right event */
	if(event->m_nSubSequence != su->sequence)
		return;
	
	if(event->m_eResult == EVENT_SUCCESS)
		search_done(event->m_sSearchAck->cMore);
	else if(event->m_eResult == EVENT_ACKED)
		search_found(event->m_sSearchAck);
	else
		search_failed();
}

void search_done(char more)
{
	if(more == (char)1)
		gtk_label_set_text(GTK_LABEL(su->label),
			"More users found, narrow your search and try again.");
	else
		gtk_label_set_text(GTK_LABEL(su->label),
			"Search complete, double click user to add him/her.");
}

void search_found(CSearchAck *ack)
{
	gchar *found_user[4];
	gchar *name = g_strdup_printf("%s %s", ack->szFirstName,
			              ack->szLastName);	

	gtk_clist_freeze(GTK_CLIST(su->list));

	found_user[0] = g_strdup_printf("%ld", ack->nUin);
	found_user[1] = ack->szAlias;
	found_user[2] = name;
	found_user[3] = ack->szEmail; 

	gtk_clist_insert(GTK_CLIST(su->list), num_found_users, found_user);
	gtk_clist_set_row_data(GTK_CLIST(su->list), num_found_users,
			       (gpointer)ack->nUin);

	num_found_users++;
	gtk_clist_thaw(GTK_CLIST(su->list));
}

void search_failed()
{
	gtk_label_set_text(GTK_LABEL(su->label), "Search failed.");
}

void search_close(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(su->window);
	su = NULL;
}
