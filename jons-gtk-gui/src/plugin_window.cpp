

#include "licq_gtk.h"
#include <gtk/gtk.h>
#include <sys/types.h>
#include <dirent.h>

#include "licq_icqd.h"

struct plugin_window *pw;

void create_plugin_window()
{
	// Only open a window if it's necessary
	if(pw != NULL)
	{
		gdk_window_raise(pw->window->window);
		return;
	}

	// Create the structure and then make the window
	pw = g_new0(struct plugin_window, 1);
	
	pw->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(pw->window), "Licq - Plugins");
	gtk_window_set_position(GTK_WINDOW(pw->window), GTK_WIN_POS_CENTER);
	gtk_signal_connect(GTK_OBJECT(pw->window), "destroy",
		GTK_SIGNAL_FUNC(plugin_close_callback), NULL);

	// The loaded frame and box inside the frame
	GtkWidget *l_frame = gtk_frame_new("Loaded");
	GtkWidget *l_box = gtk_vbox_new(false, 0);
	gtk_container_add(GTK_CONTAINER(l_frame), l_box);

	// Scroll window for the clist
	GtkWidget *scroll_win = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	// The loaded clist
	gchar *l_titles[] =
		{ "Id", "Name", "Version", "Status", "Description" }; 
	pw->l_clist = gtk_clist_new_with_titles(5, l_titles);
	gtk_clist_set_column_width(GTK_CLIST(pw->l_clist), 0, 20);
	gtk_clist_set_column_width(GTK_CLIST(pw->l_clist), 1, 85);
	gtk_clist_set_column_width(GTK_CLIST(pw->l_clist), 2, 50);
	gtk_clist_set_column_width(GTK_CLIST(pw->l_clist), 3, 50);
	gtk_clist_set_column_width(GTK_CLIST(pw->l_clist), 4, 90);
	gtk_container_add(GTK_CONTAINER(scroll_win), pw->l_clist);
	gtk_widget_set_usize(scroll_win, 335, 100);
	gtk_box_pack_start(GTK_BOX(l_box), scroll_win, true, true, 5);

	// The "Enable", "Disable", "Unload", "Details", and "Configure" buttons
	GtkWidget *enable = gtk_button_new_with_label("Enable");
	gtk_signal_connect(GTK_OBJECT(enable), "clicked",
		GTK_SIGNAL_FUNC(plugin_enable_callback), NULL);

	GtkWidget *disable = gtk_button_new_with_label("Disable");
	gtk_signal_connect(GTK_OBJECT(disable), "clicked",
		GTK_SIGNAL_FUNC(plugin_disable_callback), NULL);

	GtkWidget *unload = gtk_button_new_with_label("Unload");
	gtk_signal_connect(GTK_OBJECT(unload), "clicked",
		GTK_SIGNAL_FUNC(plugin_unload_callback), NULL);
		
	GtkWidget *details = gtk_button_new_with_label("Details");
	gtk_signal_connect(GTK_OBJECT(details), "clicked",
		GTK_SIGNAL_FUNC(plugin_details_callback), NULL);

	GtkWidget *configure = gtk_button_new_with_label("Configure");
	gtk_signal_connect(GTK_OBJECT(configure), "clicked",
		GTK_SIGNAL_FUNC(plugin_configure_callback), NULL);

	// Pack the buttons into an hbox then pack the hbox into the vbox
	GtkWidget *buttons_box = gtk_hbox_new(false, 0);
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
	pw->a_clist = gtk_clist_new(1);
	gtk_widget_set_usize(scroll_win, 335, 80);
	gtk_container_add(GTK_CONTAINER(scroll_win), pw->a_clist);
	gtk_box_pack_start(GTK_BOX(a_box), scroll_win, true, true, 5);

	// "Load" button
	GtkWidget *load = gtk_button_new_with_label("Load");
	gtk_signal_connect(GTK_OBJECT(load), "clicked",
		GTK_SIGNAL_FUNC(plugin_load_callback), NULL);
	
	// Pack it 
	buttons_box = gtk_hbox_new(false, 0);
	gtk_box_pack_start(GTK_BOX(buttons_box), load, true, true, 130);
	gtk_box_pack_start(GTK_BOX(a_box), buttons_box, true, true, 5);

	// Done and Refresh buttons
	GtkWidget *done = gtk_button_new_with_label(" Done ");
	gtk_signal_connect(GTK_OBJECT(done), "clicked",
		GTK_SIGNAL_FUNC(plugin_close_callback), 0);

	GtkWidget *refresh = gtk_button_new_with_label("Refresh");
	gtk_signal_connect(GTK_OBJECT(refresh), "clicked",
		GTK_SIGNAL_FUNC(plugin_refresh_callback), 0);

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

void plugin_close_callback(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(pw->window);
	g_free(pw);
	pw = 0;
}

void plugin_enable_callback(GtkWidget *widget, gpointer data)
{
	// Make sure we have something to work with
	if(!GTK_CLIST(pw->l_clist)->selection)
		return;

	// Get the ID
	gchar *szId;
	gtk_clist_get_text(GTK_CLIST(pw->l_clist),
		(gint)(GTK_CLIST(pw->l_clist)->selection->data),
		0, &szId);
	gint Id = atoi(szId);

	// Enable it
	icq_daemon->PluginEnable(Id);
	plugin_refresh_callback(0, 0);
}

void plugin_disable_callback(GtkWidget *widget, gpointer data)
{
	// Make sure we have something to work with
	if(!GTK_CLIST(pw->l_clist)->selection)
		return;

	// Get the ID
	gchar *szId;
	gtk_clist_get_text(GTK_CLIST(pw->l_clist),
		(gint)(GTK_CLIST(pw->l_clist)->selection->data),
		0, &szId);
	gint Id = atoi(szId);

	// Disable it
	icq_daemon->PluginDisable(Id);
	plugin_refresh_callback(0, 0);
	
}

void plugin_unload_callback(GtkWidget *widget, gpointer data)
{
	// Make sure we have something to work with
	if(!GTK_CLIST(pw->l_clist)->selection)
		return;

	// Get the ID
	gchar *szId;
	gtk_clist_get_text(GTK_CLIST(pw->l_clist),
		(gint)(GTK_CLIST(pw->l_clist)->selection->data),
		0, &szId);
	gint Id = atoi(szId);

	// Unload it
	icq_daemon->PluginShutdown(Id);
	plugin_refresh_callback(0, 0);
}

void plugin_details_callback(GtkWidget *widget, gpointer data)
{
	// Make sure we have something to work with
	if(!GTK_CLIST(pw->l_clist)->selection)
		return;

	// Get the ID
	gchar *szId;
	gtk_clist_get_text(GTK_CLIST(pw->l_clist),
		(gint)(GTK_CLIST(pw->l_clist)->selection->data),
		0, &szId);
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
	if(!GTK_CLIST(pw->a_clist)->selection)
		return;

	// Get the filename
	gchar *filename;
	gtk_clist_get_text(GTK_CLIST(pw->a_clist),
		(gint)(GTK_CLIST(pw->a_clist)->selection->data),
		0, &filename);

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
	gtk_clist_freeze(GTK_CLIST(pw->l_clist));
	gtk_clist_clear(GTK_CLIST(pw->l_clist));

	// Fill it now
	gchar *text[6];
	text[5] = 0;
	for(it = l.begin(); it != l.end(); it++)
	{
		text[0] = g_strdup_printf("%d", (*it)->Id());
		text[1] = const_cast<char *>((*it)->Name());
		text[2] = const_cast<char *>((*it)->Version());
		text[3] = const_cast<char *>((*it)->Status());
		text[4] = const_cast<char *>((*it)->Description());

		gtk_clist_append(GTK_CLIST(pw->l_clist), text);

		g_free(text[0]);
	}

	// Thaw the list
	gtk_clist_thaw(GTK_CLIST(pw->l_clist));

	// The available list.. freeze and clear it
	gtk_clist_freeze(GTK_CLIST(pw->a_clist));
	gtk_clist_clear(GTK_CLIST(pw->a_clist));

	// Fill it now
	DIR *pDir = opendir(LIB_DIR);
	if(pDir == 0)
	{
		gtk_clist_thaw(GTK_CLIST(pw->a_clist));
		return;
	}

	struct dirent *pDirent = new dirent;

	while((pDirent = readdir(pDir)))
	{
		// Look for 'licq_*.so' files
		if((strstr(pDirent->d_name, "licq_") == pDirent->d_name) &&
		   (strstr(pDirent->d_name + strlen(pDirent->d_name) - 3,
		   ".so")))
		{
			gchar *file[] = { pDirent->d_name };
			gtk_clist_append(GTK_CLIST(pw->a_clist), file);
		}
	}

	// We're done with these kind fellows
	closedir(pDir);
	if(pDirent)
		delete pDirent;
	
	// Thaw the list
	gtk_clist_thaw(GTK_CLIST(pw->a_clist));
}
