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
#include <sys/types.h>
#include <dirent.h>

struct plugin_window
{
	GtkWidget *window;
	GtkWidget *l_clist;
	GtkWidget *a_clist;
};

struct plugin_window *pw;

gboolean plugin_close_callback(GtkWidget *, gpointer);
void plugin_enable_callback(GtkWidget *, gpointer);
void plugin_disable_callback(GtkWidget *, gpointer);
void plugin_unload_callback(GtkWidget *, gpointer);
void plugin_details_callback(GtkWidget *, gpointer);
void plugin_configure_callback(GtkWidget *, gpointer);
void plugin_load_callback(GtkWidget *, gpointer);
void plugin_refresh_callback(GtkWidget *, gpointer);

void create_plugin_window()
{
	// Only open a window if it's necessary
	if(pw != 0)
	{
		gtk_window_present(GTK_WINDOW(pw->window));
		return;
	}

	// Create the structure and then make the window
	pw = g_new0(struct plugin_window, 1);
	
	pw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(pw->window), "Licq - Plugins");
	gtk_window_set_position(GTK_WINDOW(pw->window), GTK_WIN_POS_CENTER);
	g_signal_connect(G_OBJECT(pw->window), "destroy",
		G_CALLBACK(plugin_close_callback), 0);
	
	// The loaded frame and box inside the frame
	GtkWidget *l_frame = gtk_frame_new("Loaded");
	GtkWidget *l_box = gtk_vbox_new(false, 0);
	gtk_container_add(GTK_CONTAINER(l_frame), l_box);

	// Scroll window for the clist
	GtkWidget *scroll_win = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
		GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);

	// The loaded clist
	GtkListStore *store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, 
			G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	pw->l_clist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
			"Id", r_text, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(pw->a_clist), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Name", r_text, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(pw->a_clist), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Version", r_text, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(pw->a_clist), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Status", r_text, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(pw->a_clist), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Description", r_text, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(pw->a_clist), col);
	//gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(pw->a_clist), FALSE);

	gtk_container_add(GTK_CONTAINER(scroll_win), pw->l_clist);
	gtk_box_pack_start(GTK_BOX(l_box), scroll_win, true, true, 5);

	// The "Enable", "Disable", "Unload", "Details", and "Configure" buttons
	GtkWidget *enable = gtk_button_new_with_mnemonic("_Enable");
	g_signal_connect(G_OBJECT(enable), "clicked",
		G_CALLBACK(plugin_enable_callback), 0);

	GtkWidget *disable = gtk_button_new_with_mnemonic("_Disable");
	g_signal_connect(G_OBJECT(disable), "clicked",
		G_CALLBACK(plugin_disable_callback), 0);

	GtkWidget *unload = gtk_button_new_with_mnemonic("_Unload");
	g_signal_connect(G_OBJECT(unload), "clicked",
		G_CALLBACK(plugin_unload_callback), 0);
		
	GtkWidget *details = gtk_button_new_with_mnemonic("De_tails");
	g_signal_connect(G_OBJECT(details), "clicked",
		G_CALLBACK(plugin_details_callback), 0);

	GtkWidget *configure = gtk_button_new_with_mnemonic("C_onfigure");
	g_signal_connect(G_OBJECT(configure), "clicked",
		G_CALLBACK(plugin_configure_callback), 0);

	// Pack the buttons into an hbox then pack the hbox into the vbox
	GtkWidget *buttons_box = gtk_hbox_new(true, 0);
	gtk_box_pack_start(GTK_BOX(buttons_box), enable, true, true, 5);
	gtk_box_pack_start(GTK_BOX(buttons_box), disable, true, true, 5);
	gtk_box_pack_start(GTK_BOX(buttons_box), unload, true, true, 5);
	gtk_box_pack_start(GTK_BOX(buttons_box), details, true, true, 5);
	gtk_box_pack_start(GTK_BOX(buttons_box), configure, true, true, 5);
	gtk_box_pack_start(GTK_BOX(l_box), buttons_box, true, true, 5);

	// Now create the available frame and box inside the frame
	GtkWidget *a_frame = gtk_frame_new("Available");
	GtkWidget *a_box = gtk_vbox_new(false, 0);
	gtk_container_add(GTK_CONTAINER(a_frame), a_box);

	// The availabe clist now in a scrolled window
	scroll_win = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
		GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	store = gtk_list_store_new(1, G_TYPE_STRING);
	pw->a_clist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	col = gtk_tree_view_column_new_with_attributes(
			"", gtk_cell_renderer_text_new(), "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(pw->a_clist), col);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(pw->a_clist), FALSE);
	gtk_container_add(GTK_CONTAINER(scroll_win), pw->a_clist);
	gtk_box_pack_start(GTK_BOX(a_box), scroll_win, true, true, 5);

	// "Load" button
	GtkWidget *load = gtk_button_new_with_mnemonic("_Load");
	g_signal_connect(G_OBJECT(load), "clicked",
		G_CALLBACK(plugin_load_callback), 0);
	
	// Pack it 
	buttons_box = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(buttons_box), load, true, true, 130);
	gtk_box_pack_start(GTK_BOX(a_box), buttons_box, true, true, 5);

	// Done and Refresh buttons
	GtkWidget *done = gtk_button_new_with_label(" Done ");
	g_signal_connect(G_OBJECT(done), "clicked",
		G_CALLBACK(plugin_close_callback), 0);

	GtkWidget *refresh = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	g_signal_connect(G_OBJECT(refresh), "clicked",
		G_CALLBACK(plugin_refresh_callback), 0);

	// Pack them
	buttons_box = gtk_hbox_new(true, 0);
	gtk_box_pack_start(GTK_BOX(buttons_box), done, true, true, 35);
	gtk_box_pack_start(GTK_BOX(buttons_box), refresh, true, true, 35);

	//  A new vbox that is in the main window that will hold the frames
	GtkWidget *main_box = gtk_vbox_new(false, 5);
	gtk_container_add(GTK_CONTAINER(pw->window), main_box);
	gtk_box_pack_start(GTK_BOX(main_box), l_frame, false, false, 5);
	gtk_box_pack_start(GTK_BOX(main_box), a_frame, false, false, 5);
	gtk_box_pack_start(GTK_BOX(main_box), buttons_box, false, false, 5);

	// Fill in the clists
	plugin_refresh_callback(0, 0);

	// Show it all now
	gtk_widget_show_all(pw->window);
}

gboolean plugin_close_callback(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(pw->window);
	g_free(pw);
	pw = 0;
	return true;
}

void plugin_enable_callback(GtkWidget *widget, gpointer data)
{
	// Make sure we have something to work with
	GtkTreeSelection *sel = 
			gtk_tree_view_get_selection(GTK_TREE_VIEW(pw->l_clist));
	GtkTreeIter iter;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;
	// Get the ID
	gchar *szId;
	gtk_tree_model_get(model, &iter, 0, &szId, -1);
	
	gint Id = atoi(szId);

	// Enable it
	icq_daemon->PluginEnable(Id);
	plugin_refresh_callback(0, 0);
}

void plugin_disable_callback(GtkWidget *widget, gpointer data)
{
	// Make sure we have something to work with
	GtkTreeSelection *sel = 
			gtk_tree_view_get_selection(GTK_TREE_VIEW(pw->l_clist));
	GtkTreeIter iter;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;
	// Get the ID
	gchar *szId;
	gtk_tree_model_get(model, &iter, 0, &szId, -1);
	
	gint Id = atoi(szId);

	// Disable it
	icq_daemon->PluginDisable(Id);
	plugin_refresh_callback(0, 0);
	
}

void plugin_unload_callback(GtkWidget *widget, gpointer data)
{
	// Make sure we have something to work with
	GtkTreeSelection *sel = 
			gtk_tree_view_get_selection(GTK_TREE_VIEW(pw->l_clist));
	GtkTreeIter iter;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;
	// Get the ID
	gchar *szId;
	gtk_tree_model_get(model, &iter, 0, &szId, -1);
	gint Id = atoi(szId);

	// Unload it
	icq_daemon->PluginShutdown(Id);
	plugin_refresh_callback(0, 0);
}

void plugin_details_callback(GtkWidget *widget, gpointer data)
{
	// Make sure we have something to work with
	GtkTreeSelection *sel = 
			gtk_tree_view_get_selection(GTK_TREE_VIEW(pw->l_clist));
	GtkTreeIter iter;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;
	// Get the ID
	gchar *szId;
	gtk_tree_model_get(model, &iter, 0, &szId, -1);
	gint Id = atoi(szId);

	// Find the right plugin
	PluginsList l;
	PluginsListIter it;
	icq_daemon->PluginList(l);
	for(it = l.begin(); it != l.end(); it++)
	{
		if((*it)->Id() == Id)
			break;
	}

	if(it == l.end())
		return;

	gchar *for_user = g_strdup_printf("  Licq Plugin %s %s  \n  %s  \n",
		(*it)->Name(), (*it)->Version(), (*it)->Usage());
	message_box(for_user);
	g_free(for_user);
}

void plugin_configure_callback(GtkWidget *widget, gpointer data)
{
	message_box("  This is not implemented yet!  \n");
}

void plugin_load_callback(GtkWidget *widget, gpointer data)
{
	// Make sure we have something to work with
	GtkTreeSelection *sel = 
			gtk_tree_view_get_selection(GTK_TREE_VIEW(pw->a_clist));
	GtkTreeIter iter;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;
	// Get the ID
	gchar *filename;
	gtk_tree_model_get(model, &iter, 0, &filename, -1);
	
	gchar *path = g_strdup_printf("%s/%s", LIB_DIR, filename);

	// Load it
	gchar *sz[] = { "licq", 0 };
	icq_daemon->PluginLoad(path, 1, sz);

	if(path)
		g_free(path);

	plugin_refresh_callback(0, 0);
}

void plugin_refresh_callback(GtkWidget *widget, gpointer data)
{
	// Daemon stuff to get std::list and an iterator for the plugins..
	PluginsList l;
	PluginsListIter it;
	icq_daemon->PluginList(l);
	
	// Freeze the list and then clear it
	GtkListStore *store = 
			GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(pw->l_clist)));
	gtk_list_store_clear(store);
	GtkTreeIter iter;

	// Fill it now
	for(it = l.begin(); it != l.end(); it++)
	{
		char *txt = g_strdup_printf("%d", (*it)->Id());

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 
				0, txt,
				1, (*it)->Name(),
				2, (*it)->Version(),
				3, (*it)->Status(),
				4, (*it)->Description(), -1);

		g_free(txt);
	}

	// Thaw the list

	// The available list.. freeze and clear it
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(pw->a_clist)));
	gtk_list_store_clear(store);

	// Fill it now
	DIR *pDir = opendir(LIB_DIR);
	if(pDir == 0)
		return;

	struct dirent *pDirent = new dirent;

	while((pDirent = readdir(pDir)))
	{
		// Look for 'licq_*.so' files
		if((strstr(pDirent->d_name, "licq_") == pDirent->d_name) &&
		   (strstr(pDirent->d_name + strlen(pDirent->d_name) - 3,
		   ".so")))
		{
			gchar *file[] = { pDirent->d_name };
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter, 0, file, -1);
		}
	}

	// We're done with these kind fellows
	closedir(pDir);
	if(pDirent)
		delete pDirent;
}
