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

#include <gtk/gtk.h>

GtkWidget *vertical_box;
GtkWidget *contact_list;

void main_window_delete_event(GtkWidget *mainwindow, gpointer data)
{
	gtk_widget_hide(mainwindow);
}

GtkWidget* main_window_new(const gchar* window_title,
			   gint height,
			   gint width)
{
	GtkWidget *scroll_bar;
	GtkWidget *status_bar;
	GtkWidget *event_box;
	GtkWidget *event_box2;

	/* Create the main window */
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/* Set the title */
	gtk_window_set_title(GTK_WINDOW(main_window), window_title);
	
	/* Make the window fully resizable */
	gtk_window_set_policy(GTK_WINDOW(main_window), TRUE, TRUE, TRUE);

	gtk_widget_realize(main_window);

	/* Call main_window_delete_event when the delete_event is called */
	gtk_signal_connect(GTK_OBJECT(main_window), "delete_event",
			   GTK_SIGNAL_FUNC(main_window_delete_event), NULL);

	/* Add the vertical box in */
	vertical_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(main_window), vertical_box);
	gtk_widget_show(vertical_box);

	/* Add in the menu */
	menu_create();

	/* Add a scroll bar for the contact list */
	scroll_bar = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_bar),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scroll_bar, width - 77, height - 61);

	/* Add in the contact list */
	contact_list = contact_list_new(height - 30, width - 37);
	
	gtk_container_add(GTK_CONTAINER(scroll_bar), contact_list);
	gtk_box_pack_start(GTK_BOX(vertical_box), scroll_bar, 
			   TRUE, TRUE, 0); 
	contact_list_refresh();

	/* Add in the event box for the status bar */
	event_box = gtk_event_box_new();

	/* Add in the status bar menu */
	status_bar = status_bar_new(25, width, 2);

	/* Add the status bar to the event box */
	gtk_container_add(GTK_CONTAINER(event_box), status_bar);

	/* Get any events for it */
	gtk_signal_connect(GTK_OBJECT(event_box), "button-release-event",
			   GTK_SIGNAL_FUNC(status_popup_menu),
			   GTK_OBJECT(_status_menu));

	/* Refresh the status */
	status_bar_refresh();

	/* The event box for new messages */
	event_box2 = gtk_event_box_new();

	/* Add the system status bar */
	system_status = system_status_new(25, width, 2);

	/* Add the system status bar into the event box */
	gtk_container_add(GTK_CONTAINER(event_box2), system_status);

	/* Get any clicks on the system status bar*/
	gtk_signal_connect(GTK_OBJECT(event_box2), "button_press_event",
			   GTK_SIGNAL_FUNC(system_status_click), NULL);

	/* Pack the system status bar first */
	gtk_box_pack_start(GTK_BOX(vertical_box), event_box2, FALSE,FALSE,0);

	/* Now add the mode status bar in */
	gtk_box_pack_start(GTK_BOX(vertical_box), event_box,
			   FALSE, FALSE, 0);

	/* Refresh the system status */
	system_status_refresh();

	/* Show the widgets */
	gtk_widget_show(scroll_bar);
	gtk_widget_show(contact_list);
	gtk_widget_show(event_box);
	gtk_widget_show(event_box2);
	gtk_widget_show(status_bar);
	gtk_widget_show(system_status);

	return main_window;
}

void main_window_show()
{
	gtk_widget_show(main_window);
}
