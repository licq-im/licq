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

#include "licq_user.h"
#include "licq_icqd.h"

#include <gtk/gtk.h>

GtkWidget *system_status;

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

void system_status_refresh()
{
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	gushort num_owner_events = owner->NewMessages();
	gUserManager.DropOwner();

	gulong num_user_event = ICQUser::getNumUserEvents() - num_owner_events;

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(system_status),
						"Status");
	if(num_owner_events > 0)
	{
		gtk_statusbar_pop(GTK_STATUSBAR(system_status), id);
		gtk_statusbar_push(GTK_STATUSBAR(system_status), id, "SysMsg");
	}

	else if(num_user_event > 0)
	{
		gchar *lbl;

		if(num_user_event == 1)
			lbl = "msg";

		else
			lbl = "msgs";

		const gchar *label =
			g_strdup_printf("%ld %s", num_user_event, lbl);

		gtk_statusbar_pop(GTK_STATUSBAR(system_status), id);
		gtk_statusbar_push(GTK_STATUSBAR(system_status), id, label);
	}

	else
	{
		gtk_statusbar_pop(GTK_STATUSBAR(system_status), id);
		gtk_statusbar_push(GTK_STATUSBAR(system_status), id, "No msgs");
	}

	if (num_owner_events > 0 || num_user_event > 0)
		licq_tray_start_flashing();
	else
		licq_tray_stop_flashing();
}

void system_status_click(GtkWidget *w, GdkEventButton *event, gpointer d)
{
	/* Make sure we have a double click here */
	if(!(event->type == GDK_2BUTTON_PRESS && event->button == 1))
		return;

	/* If no events are pending, leave */
	if(ICQUser::getNumUserEvents() == 0)
		return;

	/* Check for system messages first */
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	gushort owner_events = owner->NewMessages();

	if(owner_events > 0)
	{
		system_message_window();
		gUserManager.DropOwner();
		return;
	}

	else
		gUserManager.DropOwner();

	/* Now for the user messages */
	gulong uin = 0;

	FOR_EACH_USER_START(LOCK_R)
	{
		if(pUser->NewMessages() > 0)
			uin = pUser->Uin();
	}	
	FOR_EACH_USER_END

	ICQUser *user = gUserManager.FetchUser(uin, LOCK_R);

	if(uin != 0)
		//struct conversation *c = 
		convo_new(user, TRUE);

	gUserManager.DropUser(user);

	contact_list_refresh();
	system_status_refresh();
}
