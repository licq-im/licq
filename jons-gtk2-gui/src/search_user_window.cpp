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
#include "utilities.h"

#include "licq_icqd.h"
#include "licq_user.h"
#include "licq_languagecodes.h"
#include "licq_countrycodes.h"

#include <gtk/gtk.h>
#include <stdlib.h>

#include <iostream>
#include <string>

using namespace std;

struct search_user
{
	GtkWidget *window;
	GtkWidget *nick_name;
	GtkWidget *first_name;
	GtkWidget *last_name;
	GtkWidget *age;
	GtkWidget *gender;
	GtkWidget *language;
	GtkWidget *city;
	GtkWidget *state;
	GtkWidget *country;
	GtkWidget *c_name;
	GtkWidget *c_department;
	GtkWidget *c_position;
	GtkWidget *email;
	GtkWidget *keyword;
	GtkWidget *online_only;
	GtkWidget *uin;
	GtkWidget *list;
	GtkWidget *clear;
	GtkWidget *statusbar;
	unsigned long e_tag;
};

struct search_user *su = NULL;
static gint num_found_users = 0;

GtkWidget *
left_aligned_label(const char *lbl)
{
	GtkWidget *label = gtk_label_new(lbl);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	return label;
}

const char *genders[] = {"Unspecified", "Female", "Male"};
static const int NUM_GENDERS = 3;
const char *ages[] = {"Unspecified", "18 - 22", "23 - 29", "30 - 39", "40 - 49",
		"50 - 59", "60+"};
static const int NUM_AGES = 7;

GtkWidget *
new_entry(const char *label, GtkWidget *table, int c, int r)
{	
	gtk_table_attach(GTK_TABLE(table), left_aligned_label(label),
			c, c + 1, r, r + 1,	GTK_FILL, GTK_FILL, 3, 3);
	GtkWidget *entry = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table), entry,
			c + 1, c + 2, r, r + 1, 
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GTK_FILL, 3, 3);
	return entry;
}

void search_user_window()
{
	/* Only one search window */
	if (su != NULL) {
		gdk_window_raise(su->window->window);
		return;
	}

	su = g_new0(struct search_user, 1);

	num_found_users = 0;

	/* Create the window */
	su->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(su->window), "Licq - User Search");

	/* destroy event for window */
	g_signal_connect(G_OBJECT(su->window), "destroy",
			   G_CALLBACK(search_close), su->window);
	
	/* top vbox */
	GtkWidget *main_vbox = gtk_vbox_new(FALSE, 6);
  gtk_container_add(GTK_CONTAINER(su->window), main_vbox);
	
	/* hbox around notebook and buttons */
	GtkWidget *top_hbox = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_start(GTK_BOX(main_vbox), top_hbox, TRUE, TRUE, 5);
	
	/* Create the notebook */
	GtkWidget *notebook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX(top_hbox), notebook, TRUE, TRUE, 5);
	GtkWidget *whitepages = gtk_table_new(9, 4, FALSE);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), whitepages,
			gtk_label_new("Whitepages"));
	
	su->nick_name = new_entry("Alias:", whitepages, 0, 0);
	su->first_name = new_entry("First Name:", whitepages, 0, 1);
	su->last_name = new_entry("Last Name:", whitepages, 0, 2);

	gtk_table_attach(GTK_TABLE(whitepages), 
			left_aligned_label("Age Range:"),
			0, 1, 3, 4, GTK_FILL, GTK_FILL, 3, 3);
	su->age = gtk_option_menu_new();
	GtkWidget *age_menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(su->age), age_menu);
	for (int i = 0; i < NUM_AGES; ++i)
		gtk_menu_shell_append(GTK_MENU_SHELL(age_menu), 
				gtk_menu_item_new_with_label(ages[i]));
	gtk_option_menu_set_history(GTK_OPTION_MENU(su->age), 0);
	gtk_table_attach(GTK_TABLE(whitepages), su->age,
			1, 2, 3, 4, GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GTK_FILL, 3, 3);
				
	gtk_table_attach(GTK_TABLE(whitepages), left_aligned_label("Gender:"),
			0, 1, 4, 5, GTK_FILL, GTK_FILL, 3, 3);
	su->gender = gtk_option_menu_new();
	GtkWidget *gender_menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(su->gender), gender_menu);
	for (int i = 0; i < NUM_GENDERS; ++i)
		gtk_menu_shell_append(GTK_MENU_SHELL(gender_menu), 
				gtk_menu_item_new_with_label(genders[i]));
	gtk_option_menu_set_history(GTK_OPTION_MENU(su->gender), 0);
	gtk_table_attach(GTK_TABLE(whitepages), su->gender,
			1, 2, 4, 5, GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GTK_FILL, 3, 3);
	
	gtk_table_attach(GTK_TABLE(whitepages), left_aligned_label("Language:"),
			0, 1, 5, 6, GTK_FILL, GTK_FILL, 3, 3);
	su->language = gtk_option_menu_new();
	GtkWidget *language_menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(su->language), language_menu);
	for (unsigned int i = 0; i < NUM_LANGUAGES; ++i)
		gtk_menu_shell_append(GTK_MENU_SHELL(language_menu), 
				gtk_menu_item_new_with_label(gLanguages[i].szName));
	gtk_option_menu_set_history(GTK_OPTION_MENU(su->language), 0);
	gtk_table_attach(GTK_TABLE(whitepages), su->language,
			1, 2, 5, 6, GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GTK_FILL, 3, 3);
			
	su->city = new_entry("City:", whitepages, 2, 0);
	su->state = new_entry("State:", whitepages, 2, 1);
	
	gtk_table_attach(GTK_TABLE(whitepages), left_aligned_label("Country:"),
			2, 3, 2, 3, GTK_FILL, GTK_FILL, 3, 3);
	su->country = gtk_option_menu_new();
	GtkWidget *country_menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(su->country), country_menu);
	for (unsigned int i = 0; i < NUM_COUNTRIES; ++i)
		gtk_menu_shell_append(GTK_MENU_SHELL(country_menu), 
				gtk_menu_item_new_with_label(gCountries[i].szName));
	gtk_option_menu_set_history(GTK_OPTION_MENU(su->country), 0);
	gtk_table_attach(GTK_TABLE(whitepages), su->country,
			3, 4, 2, 3, GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GTK_FILL, 3, 3);
	
	su->c_name = new_entry("Company Name:", whitepages, 2, 3);
	su->c_department = new_entry("Company Department:", whitepages, 2, 4);
	su->c_position = new_entry("Company Position:", whitepages, 2, 5);

	gtk_table_attach(GTK_TABLE(whitepages), left_aligned_label("E-mail Address:"),
			0, 1, 6, 7,	GTK_FILL, GTK_FILL, 3, 3);
	su->email = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(whitepages), su->email, 
			1, 4, 6, 7,	GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GTK_FILL, 3, 3);

	gtk_table_attach(GTK_TABLE(whitepages), left_aligned_label("Keyword:"),
			0, 1, 7, 8,	GTK_FILL, GTK_FILL, 3, 3);
	su->keyword = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(whitepages), su->keyword, 
			1, 4, 7, 8,	GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GTK_FILL, 3, 3);
	
	su->online_only = gtk_check_button_new_with_label("Return Online Users Only");
	gtk_table_attach(GTK_TABLE(whitepages), su->online_only,
			0, 4, 8, 9,	GTK_FILL, GTK_FILL, 3, 3);
	
	/* Another new tab (UIN Tab), a new table */
	GtkWidget *uin_hbox = gtk_hbox_new(FALSE, 6);
	gtk_box_pack_start(GTK_BOX(uin_hbox), gtk_label_new("UIN:"),
			FALSE, FALSE, 5);
	su->uin = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(su->uin), MAX_LENGTH_UIN);
	gtk_box_pack_start(GTK_BOX(uin_hbox), su->uin,
			FALSE, FALSE, 5);
	
	/* Only numbers allowed in the uin entry box */
	g_signal_connect(G_OBJECT(su->uin), "insert-text",
			   G_CALLBACK(verify_numbers), 0);
	
	/* Append to the notebook */
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), uin_hbox,
			gtk_label_new("UIN:"));

	/* The v_box for the clear and search buttons */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 5);

	/* The "Search" button */
	GtkWidget *search = gtk_button_new_with_mnemonic("_Search");
	g_signal_connect(G_OBJECT(search), "clicked",
			G_CALLBACK(search_callback), 0);

	/* The "Clear List" button */
	su->clear = gtk_button_new_with_mnemonic("     _Clear List     ");
	g_signal_connect(G_OBJECT(su->clear), "clicked",
			G_CALLBACK(clear_callback), 0);
	// disabled until there's something to clear
	gtk_widget_set_sensitive(su->clear, FALSE);
	
	/* The "Done" button */
	GtkWidget *done = gtk_button_new_with_mnemonic("_Done");
	g_signal_connect(G_OBJECT(done), "clicked",
			G_CALLBACK(search_close), 0);
	
	/* we want these at the bottom */
	gtk_box_pack_end(GTK_BOX(vbox), done, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), su->clear, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), search, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(top_hbox), vbox, TRUE, TRUE, 5);
	
	/* Found users list with a scroll bar */
	GtkWidget *scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	GtkListStore *store = gtk_list_store_new(7,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	su->list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
			"Alias", r_text, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"UIN", r_text, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Name", r_text, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"E-mail", r_text, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Status", r_text, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Sex & Age", r_text, "text", 5, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Authorize", r_text, "text", 6, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(su->list), col);
	
	/* Double click on a user */
	g_signal_connect(G_OBJECT(su->list), "button_press_event",
			G_CALLBACK(search_list_double_click), 0);
	
	gtk_widget_set_size_request(GTK_WIDGET(su->list), 230, 300);
	gtk_container_add(GTK_CONTAINER(scroll), su->list);
  gtk_box_pack_start(GTK_BOX(main_vbox), scroll, TRUE, TRUE, 5);

	/* The statusbar */
	su->statusbar = gtk_statusbar_new();
  gtk_box_pack_start(GTK_BOX(main_vbox), su->statusbar, TRUE, TRUE, 5);
	
	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(su->statusbar),
			"sta");
	gtk_statusbar_pop(GTK_STATUSBAR(su->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(su->statusbar), id,
			"Enter Search Parameters");

	//gtk_widget_set_size_request(GTK_WIDGET(su->window), 435, 465); 
	gtk_widget_show_all(su->window);
}

void
change_status(GtkWidget *statusbar, const char *st_name, 
		const char *newstatus)
{
	guint id = gtk_statusbar_get_context_id(
			GTK_STATUSBAR(statusbar), st_name);
	gtk_statusbar_pop(GTK_STATUSBAR(statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), id, newstatus);
}

void clear_callback(GtkWidget *widget, gpointer data)
{
	num_found_users = 0;
	GtkListStore *store = 
			GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(su->list)));
	gtk_list_store_clear(store);
	gtk_widget_set_sensitive(su->clear, FALSE);
	change_status(su->statusbar, "sta", "");
}

const char *
get_text(GtkWidget *w)
{
	return gtk_editable_get_chars(GTK_EDITABLE(w), 0, -1);
}

gint
get_index(GtkWidget *w)
{
	return gtk_option_menu_get_history(GTK_OPTION_MENU(w));
}

void 
search_callback(GtkWidget *widget, gpointer data)
{
	change_status(su->statusbar, "sta", "Searching, this may take awhile.");
	
	unsigned short mins[7] = {0, 18, 23, 30, 40, 50, 60};
  unsigned short maxs[7] = {0, 22, 29, 39, 49, 59, 120};

	gulong uin = strtoul(get_text(su->uin), 0, 10);

	if(uin >= 10000)
		su->e_tag = icq_daemon->icqSearchByUin(uin);
	else
    su->e_tag = icq_daemon->icqSearchWhitePages(
				get_text(su->first_name),
				get_text(su->last_name),
				get_text(su->nick_name),
				get_text(su->email),
				mins[get_index(su->age)], // age min
				maxs[get_index(su->age)], // age max
				(char)get_index(su->gender),
				GetLanguageByIndex(get_index(su->language))->nCode,
				get_text(su->city),
				get_text(su->state),
				GetCountryByIndex(get_index(su->country))->nCode,
				get_text(su->c_name),
				get_text(su->c_department),
				get_text(su->c_position),
				get_text(su->keyword),
				false);
}

void 
search_list_double_click(GtkWidget *widget, GdkEventButton *eb, gpointer data)
{
	// we'll consider left button double click only
  if (eb->type != GDK_2BUTTON_PRESS || eb->button != 1)
		return;

	/* Get which cell was clicked in */
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(su->list));

	GtkTreeIter iter;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;

	gchar *c_uin;
	gtk_tree_model_get(model, &iter, 1, &c_uin, -1);
	gulong uin = strtoul(c_uin, 0, 10);
	g_free(c_uin);

	// either invalid uin or we have this user already
  if (uin == 0 || gUserManager.FetchUser(uin, LOCK_N) != NULL)
		return;

	icq_daemon->AddUserToList(uin);

	char *for_statusbar = g_strdup_printf("User (%ld) added", uin);
	change_status(su->statusbar, "sta", for_statusbar);
	g_free(for_statusbar);
}

void search_result(ICQEvent *event)
{	
	/* Make sure it's the right event */
	if (su == NULL || !event->Equals(su->e_tag))
		return;

	CSearchAck *search_ack = event->SearchAck();

	if (event->Result() == EVENT_SUCCESS)
		search_done(search_ack->More());
	else if (event->Result() == EVENT_ACKED)
		search_found(search_ack);
	else
		search_failed();
}

void 
search_done(bool more)
{
	if (more)
		change_status(su->statusbar, "sta",
			"More users found, narrow your search and try again.");
	else
		change_status(su->statusbar, "sta",
			"Search complete, double click user to add her/him.");
}

void 
search_found(CSearchAck *s)
{
	string name(s_convert_to_utf8(s->FirstName()) + ' ' + 
  		s_convert_to_utf8(s->LastName()));

	char *uin = g_strdup_printf("%ld", s->Uin());
	string alias(s_convert_to_utf8(s->Alias()));
	string email(s_convert_to_utf8(s->Email()));

	GtkListStore *store = 
			GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(su->list)));
	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
			0, alias.c_str(),
			1, uin,
			2, name.c_str(),
			3, email.c_str(),
			-1);
	g_free(uin);
	switch (s->Status()) {
		case 0: gtk_list_store_set(store, &iter, 4, "Offline", -1);
						break;
		case 1: gtk_list_store_set(store, &iter, 4, "Online", -1);
						break;
		default: 
						gtk_list_store_set(store, &iter, 4, "Unknown", -1);
						break;
	}
	
	char *gender = "?";
	if (s->Gender() == 1)
		gender = "F";
	else if (s->Gender() == 2)
		gender = "M";
	char *sex_age;
	if (s->Age())
		sex_age = g_strdup_printf("%s/%d", gender, s->Age());
	else
		sex_age = g_strdup_printf("%s/?", gender);
	gtk_list_store_set(store, &iter, 5, sex_age, -1);
	g_free(sex_age);

	if (s->Auth())
		gtk_list_store_set(store, &iter, 6, "No", -1);
	else
		gtk_list_store_set(store, &iter, 6, "Yes", -1);
	
	num_found_users++;
	gtk_widget_set_sensitive(su->clear, TRUE);
}

void search_failed()
{
	change_status(su->statusbar, "sta", "Search failed.");
}

void 
search_close(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(su->window);
	g_free(su);
	su = NULL;
}
