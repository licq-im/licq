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

GtkWidget *system_status;

void system_status_click(GtkWidget *, GdkEventButton *, gpointer);
void system_message_window();
void licq_tray_start_flashing();
void licq_tray_stop_flashing();

GtkWidget *system_status_new(gint borderwidth)
{
	system_status = gtk_statusbar_new();

	gtk_container_set_border_width(GTK_CONTAINER(system_status), borderwidth);

	/* The event box for new messages */
	GtkWidget *event_box = gtk_event_box_new();

	/* Add the system status bar into the event box */
	gtk_container_add(GTK_CONTAINER(event_box), system_status);

	/* Get any clicks on the system status bar*/
	g_signal_connect(G_OBJECT(event_box), "button_press_event",
			G_CALLBACK(system_status_click), 0);
	
	gtk_widget_show(system_status);
	gtk_widget_show(event_box);

	return event_box;
}

void 
system_status_refresh()
{
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	gushort num_owner_events = owner->NewMessages();
	gUserManager.DropOwner();

	gulong num_user_event = ICQUser::getNumUserEvents() - num_owner_events;

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(system_status),
						"Status");
	gtk_statusbar_pop(GTK_STATUSBAR(system_status), id);
	if (num_owner_events > 0)
		gtk_statusbar_push(GTK_STATUSBAR(system_status), id, "SysMsg");
	else if (num_user_event > 0) {
		gchar *label;
    if (num_user_event == 1)
    	label = g_strdup_printf("%ld msg", num_user_event);
    else
    	label = g_strdup_printf("%ld msgs", num_user_event);

		gtk_statusbar_push(GTK_STATUSBAR(system_status), id, label);
    g_free(label);
	}
	else
		gtk_statusbar_push(GTK_STATUSBAR(system_status), id, "No msgs");

	if (num_owner_events > 0 || num_user_event > 0)
		licq_tray_start_flashing();
	else
		licq_tray_stop_flashing();
}

void 
system_status_click(GtkWidget *w, GdkEventButton *event, gpointer d)
{
	/* Make sure we have a double click here */
	if (event->type != GDK_2BUTTON_PRESS || event->button != 1)
		return;

	/* If no events are pending, leave */
	if (ICQUser::getNumUserEvents() == 0)
		return;

	/* Check for system messages first */
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	gushort owner_events = owner->NewMessages();

	if (owner_events > 0) {
		system_message_window();
		gUserManager.DropOwner();
		return;
	}
	else
		gUserManager.DropOwner();

	/* Now for the user messages */
	FOR_EACH_USER_START(LOCK_R)
	{
		if(pUser->NewMessages() > 0)
			convo_open(pUser, false);
	}	
	FOR_EACH_USER_END

	contact_list_refresh();
	system_status_refresh();
}
