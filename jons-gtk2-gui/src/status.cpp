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

GtkWidget *_status_menu;

gint status_popup_menu(GtkWidget *, GdkEventButton *);
void status_ffc(GtkWidget *, gpointer);
void status_online(GtkWidget *, gpointer);
void status_away(GtkWidget *, gpointer);
void status_na(GtkWidget *, gpointer);
void status_occ(GtkWidget *, gpointer);
void status_dnd(GtkWidget *, gpointer);
void status_off(GtkWidget *, gpointer);
void status_invisible(GtkWidget *, GtkWidget *);
void status_change(gushort);
void away_msg_window(gushort);

void licq_tray_set_content(GObject *, GdkPixbuf *, int);

GtkWidget *status_bar_new()
{
	/* Create the status menu */
	_status_menu = gtk_statusbar_new();

	/* Set the border width */
	gtk_container_set_border_width(GTK_CONTAINER(_status_menu),	2);

	/* Add in the event box for the status bar */
	GtkWidget *event_box = gtk_event_box_new();

	/* Get any events for it */
	g_signal_connect(G_OBJECT(event_box), "button_press_event",
			   G_CALLBACK(status_popup_menu), GTK_OBJECT(_status_menu));
	
	/* Add the status bar to the event box */
	gtk_container_add(GTK_CONTAINER(event_box), _status_menu);

	gtk_widget_show(_status_menu);
	gtk_widget_show(event_box);

	return event_box;
}

void status_bar_refresh()
{
	/* Lock the owner and get the status and then drop the owner */
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	const gchar *status = owner->StatusStr();
	guint status_n = owner->Status();
	gUserManager.DropOwner();

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(_status_menu),
						"Status");

	gtk_statusbar_pop(GTK_STATUSBAR(_status_menu), id);

	gtk_statusbar_push(GTK_STATUSBAR(_status_menu), id, status);
	if (trayicon != 0) {
		GdkPixbuf *cur_icon = offline;

		switch (status_n) {
			case ICQ_STATUS_OFFLINE:
				cur_icon = offline;
				break;
 			case ICQ_STATUS_DND:
				cur_icon = dnd;
				break;
 			case ICQ_STATUS_OCCUPIED:
				cur_icon = occ;
				break;
			case ICQ_STATUS_NA:
				cur_icon = na;
				break;
			case ICQ_STATUS_AWAY:
				cur_icon = away;
				break;
 			case ICQ_STATUS_FREEFORCHAT:
				cur_icon = ffc;
				break;
			default:
				cur_icon = online;
		}
		licq_tray_set_content(trayicon, cur_icon, 1);
	}
}

gint status_popup_menu(GtkWidget *status, GdkEventButton *event)
{
	/* Use the right mouse button */
	if(event->button == 1)
		return FALSE;

	GtkWidget *_menu;
	GtkWidget *_menu_bar;
	GtkWidget *_root_menu;
	GtkWidget *_menu_item;
 
	/* The menu we don't show.. */
	_menu = gtk_menu_new();

	_menu_item =
		menu_new_item_with_pixmap(_menu, "Free For Chat",
			GTK_SIGNAL_FUNC(status_ffc), ffc);
	_menu_item =
		menu_new_item_with_pixmap(_menu, "Online",
			GTK_SIGNAL_FUNC(status_online), online);

	_menu_item =
		menu_new_item_with_pixmap(_menu, "Away",
			GTK_SIGNAL_FUNC(status_away), away);

	_menu_item =
		menu_new_item_with_pixmap(_menu, "Not Available",
			GTK_SIGNAL_FUNC(status_na), na);

	_menu_item =
		menu_new_item_with_pixmap(_menu, "Occupied",
			GTK_SIGNAL_FUNC(status_occ), occ);

	_menu_item =
		menu_new_item_with_pixmap(_menu, "Do Not Disturb",
			GTK_SIGNAL_FUNC(status_dnd), dnd);

	_menu_item =
		menu_new_item_with_pixmap(_menu, "Offline",
			GTK_SIGNAL_FUNC(status_off), offline);

	// Separator
	menu_separator(_menu);

	_menu_item = 
		menu_new_item_with_pixmap(_menu, "Invisible",
			GTK_SIGNAL_FUNC(status_invisible), invisible);
	_root_menu =
		menu_new_item(0, "", 0);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(_root_menu), _menu);

	_menu_bar = gtk_menu_bar_new();
	gtk_widget_show(_menu_bar);

	gtk_menu_shell_append(GTK_MENU_SHELL(_menu_bar), _root_menu);

	gtk_menu_popup(GTK_MENU(_menu), 0, 0, 0, 0,
			event->button, event->time); 

	return TRUE;
}

void status_ffc(GtkWidget *popup, gpointer data)
{
	status_change(ICQ_STATUS_FREEFORCHAT);
	away_msg_window(ICQ_STATUS_FREEFORCHAT);
}

void status_online(GtkWidget *popup, gpointer data)
{
	status_change(ICQ_STATUS_ONLINE);
}

void status_away(GtkWidget *popup, gpointer data)
{
	status_change(ICQ_STATUS_AWAY);
	away_msg_window(ICQ_STATUS_AWAY);
}

void status_na(GtkWidget *popup, gpointer data)
{
	status_change(ICQ_STATUS_NA);
	away_msg_window(ICQ_STATUS_NA);
}

void status_occ(GtkWidget *popup, gpointer data)
{
	status_change(ICQ_STATUS_OCCUPIED);
	away_msg_window(ICQ_STATUS_OCCUPIED);
}

void status_dnd(GtkWidget *popup, gpointer data)
{
	status_change(ICQ_STATUS_DND);
	away_msg_window(ICQ_STATUS_DND);
}

void status_off(GtkWidget *popup, gpointer data)
{
	status_change(ICQ_STATUS_OFFLINE);

	/* The server is no longer sending commands.. so update it manually */
	status_bar_refresh();
}

void status_invisible(GtkWidget *popup, GtkWidget *invisible_check)
{
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	
	if(!(owner->StatusFull() & ICQ_STATUS_FxPRIVATE))
	{
		icq_daemon->icqSetStatus(owner->StatusFull() | ICQ_STATUS_FxPRIVATE);
	}
	
	else
	{
		icq_daemon->icqSetStatus(owner->StatusFull() & (~ICQ_STATUS_FxPRIVATE));
	}

	gUserManager.DropOwner();
}
			
void status_change(gushort status)
{
	if(status == ICQ_STATUS_OFFLINE)
	{
		icq_daemon->icqLogoff();
		return;
	}

	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);

	if(owner->StatusOffline())
		icq_daemon->icqLogon(status);

	else
	{
		if(owner->StatusInvisible())
			icq_daemon->icqSetStatus(status | ICQ_STATUS_FxPRIVATE);
		else
			icq_daemon->icqSetStatus(status & (~ICQ_STATUS_FxPRIVATE));
	}

	gUserManager.DropOwner();
}
