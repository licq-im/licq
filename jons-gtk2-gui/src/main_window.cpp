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
#include <fstream>

using namespace std;

GtkWidget *vertical_box;
GtkWidget *contact_list;
GObject *trayicon;

gint flash_icons(gpointer);

void main_window_delete_event(GtkWidget *mainwindow, gpointer data)
{
	save_window_pos();
	gtk_main_quit();
}

GtkWidget* main_window_new(const gchar* window_title)
{
	gtk_timeout_add(1000, flash_icons, 0);

	/* Here's a good place to start the option defaults */
	const char *filename = g_strdup_printf("%s/licq_jons-gtk2-gui.conf",
					       BASE_DIR);
	fstream file(filename, ios::in | ios::out);

	if(file)
	{
		file.close();
		load_options();
	}
	else
	{
		file << "[appearance]\n";
		file.close();
		load_options();
	}

	GtkWidget *scroll_bar;

	/* Create the main window */
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/* set the position if that option is enabled */
	if (remember_window_pos)
		gtk_window_move(GTK_WINDOW(main_window), windowX, windowY);

	/* Set the title */
	gtk_window_set_title(GTK_WINDOW(main_window), window_title);
	
	/* Make the window fully resizable */
	gtk_window_set_resizable(GTK_WINDOW(main_window), TRUE);
	gtk_window_set_default_size(GTK_WINDOW(main_window), windowW, windowH);

	gtk_widget_realize(main_window);

	/* Call main_window_delete_event when the delete_event is called */
	g_signal_connect(G_OBJECT(main_window), "delete_event",
			   G_CALLBACK(main_window_delete_event), 0);
	
	// After the main window has been created, but before the contact
	// list or anything is shown, we need to make the colors and pixmaps
	do_colors();
	do_pixmaps();

	/* Add the vertical box in */
	vertical_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(main_window), vertical_box);
	gtk_widget_show(vertical_box);

	/* Add in the menu */
	menu_create();

	/* Add a scroll bar for the contact list */
	scroll_bar = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_bar),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);

	/* Add in the contact list */
	contact_list = contact_list_new(windowH - 56, windowW - 37);
	
	gtk_container_add(GTK_CONTAINER(scroll_bar), contact_list);
	gtk_box_pack_start(GTK_BOX(vertical_box), scroll_bar, 
			   TRUE, TRUE, 0); 
	//contact_list_refresh();

	/* Add the system status bar and pack it into the vbox */
	gtk_box_pack_start(GTK_BOX(vertical_box), system_status_new(2),
			FALSE, FALSE, 0);

	/* Now add the mode status bar in */
	gtk_box_pack_start(GTK_BOX(vertical_box), status_bar_new(2), 
			FALSE, FALSE, 0);

	/* Refresh the system status */
	//	system_status_refresh();

	/* Show the widgets */
	gtk_widget_show(scroll_bar);
	gtk_widget_show(contact_list);

	// Auto logon here
	if (auto_logon != ICQ_STATUS_OFFLINE)
		icq_daemon->icqLogon(auto_logon);

	trayicon = licq_init_tray();
	
	return main_window;
}

void main_window_show()
{
	gtk_widget_show(main_window);
}
