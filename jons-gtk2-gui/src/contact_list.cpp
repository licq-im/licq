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

#include <string.h>
#include <gtk/gtk.h>
#include <sys/time.h>
#include <map>

using namespace std;

GdkColor *red, *blue, *online_color, *offline_color, *away_color;
GdkPixbuf *online, *offline, *away, *na, *dnd, *occ, *ffc,
	*invisible, *message_icon, *file_icon, *chat_icon, *url_icon,
	*secure_icon, *birthday_icon, *securebday_icon, *blank_icon;

struct SFlash
{
	GdkPixbuf *icon;
	GtkTreePath *path;
	gboolean bFlashOn;
};

map<gulong,SFlash *> FlashList;
typedef map<gulong,SFlash *>::iterator FlashList_iter;

enum {
	COL_SORT,
	COL_STATUS_IMAGE,
	COL_ALIAS,
	COL_ALIAS_COLOR,
	COL_SSL_IMAGE,
	COL_PUSER,
	N_COLS
};

GdkColor *get_status_color(unsigned long);
gboolean contact_list_click(GtkWidget *, GdkEventButton *, gpointer);
void add_to_popup(const gchar *, GtkWidget *, GtkSignalFunc, ICQUser *);

GtkWidget *contact_list_new(gint height, gint width)
{
	GtkWidget *_contact_l;

	/* Create the contact list using a 6 column clist */
	GtkListStore *store = gtk_list_store_new(N_COLS, 
			G_TYPE_INT, GDK_TYPE_PIXBUF, G_TYPE_STRING, GDK_TYPE_COLOR,
			GDK_TYPE_PIXBUF, G_TYPE_POINTER);
	_contact_l = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), COL_SORT, 
			GTK_SORT_ASCENDING);
	g_object_unref(G_OBJECT(store));
	
	GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
	g_object_set(G_OBJECT(r_text), "ypad", 0, NULL);
	GtkCellRenderer *r_pic = gtk_cell_renderer_pixbuf_new();
	g_object_set(G_OBJECT(r_pic), "ypad", 0, NULL);

	GtkTreeViewColumn *col = 
			gtk_tree_view_column_new_with_attributes("Pic", r_pic, "pixbuf", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(_contact_l), col);

	// Here we'll need 2 renderers - one for text and one for a pic
	// for a possible ssl icon
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(col, r_text, FALSE);
	gtk_tree_view_column_pack_start(col, r_pic, FALSE);
	gtk_tree_view_column_add_attribute(col, r_text, "text", COL_ALIAS);
	gtk_tree_view_column_add_attribute(col, r_text, "foreground-gdk", COL_ALIAS_COLOR);
	gtk_tree_view_column_add_attribute(col, r_pic, "pixbuf", COL_SSL_IMAGE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(_contact_l), col);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(_contact_l), FALSE);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(_contact_l));
	
	/* Size the contact list */
	//gtk_widget_set_size_request(_contact_l, width, height);

	/* A double click on a user name */
	g_signal_connect(G_OBJECT(_contact_l), "button_press_event",
			   G_CALLBACK(contact_list_click), 0);

	return _contact_l;
}

GdkPixbuf *
status_icon(gulong user_status)
{
	if((gushort)user_status != ICQ_STATUS_OFFLINE && 
			(user_status & ICQ_STATUS_FxPRIVATE))
		return invisible;
	else if((gushort)user_status == ICQ_STATUS_OFFLINE)
		return offline;
	else if(user_status & ICQ_STATUS_DND)
		return dnd;
	else if(user_status & ICQ_STATUS_OCCUPIED)
		return occ;
	else if(user_status & ICQ_STATUS_NA)
		return na;
	else if(user_status & ICQ_STATUS_AWAY)
		return away;
	else if(user_status & ICQ_STATUS_FREEFORCHAT)
		return ffc;
	else if(user_status & ICQ_STATUS_OCCUPIED)
		return occ;
	else
		return online;
}

int
sort_order(gulong user_status)
{
	if((gushort)user_status != ICQ_STATUS_OFFLINE &&
		 (user_status & ICQ_STATUS_FxPRIVATE))
		return 25; // ","
	else if((gushort)user_status == ICQ_STATUS_OFFLINE)
		return 100; //"~", -1);
	else if(user_status & ICQ_STATUS_DND)
		return 50; //"X", -1);
	else if(user_status & ICQ_STATUS_OCCUPIED)
		return 60; //"x", -1);
	else if(user_status & ICQ_STATUS_NA)
		return 40; //"N", -1);
	else if(user_status & ICQ_STATUS_AWAY)
		return 30; //"A", -1);
	else if(user_status & ICQ_STATUS_FREEFORCHAT)
		return 10; //"*", -1);
	else if(user_status & ICQ_STATUS_OCCUPIED)
		return 60; //"x", -1);
	else
		return 20; // "+", -1);
}

gint flash_icons(gpointer data)
{
	// If we aren't supposed to flash or there are no things to flash
	if (!flash_events || FlashList.empty())
		return -1;
		
	GtkListStore *store = 
			GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(contact_list)));
	
	FlashList_iter it;
	for(it = FlashList.begin(); it != FlashList.end(); it++)
	{
		GtkTreeIter iter;
		SFlash *sf = it->second;
		
		if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, sf->path))
			continue;
		if (!sf->bFlashOn) {
			sf->bFlashOn = true;
			gtk_list_store_set(store, &iter, COL_STATUS_IMAGE, blank_icon, -1);
		}
		else {
			sf->bFlashOn = false;
			gtk_list_store_set(store, &iter, COL_STATUS_IMAGE, sf->icon, -1);
		}
	}
	
	return -1;
}

void stop_flashing(ICQUser *u)
{
	FlashList_iter it = FlashList.find(u->Uin());
	if (it != FlashList.end()) {
		GtkListStore *store = 
				GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(contact_list)));
		GtkTreeIter iter;
		SFlash *sf = it->second;
		
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, it->second->path))
			gtk_list_store_set(store, &iter, COL_STATUS_IMAGE, 
					status_icon(u->StatusFull()), -1);
		g_free(sf->path);
		g_free(sf);
		FlashList.erase(it);
	}
}

void contact_list_refresh()
{
	list<unsigned long> AutoSecureList;
	gint num_users = 0;

	GtkListStore *store = 
			GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(contact_list)));
	gtk_list_store_clear(store);
	GtkTreeIter iter;

	/* Now go through all the users */
	FOR_EACH_USER_START(LOCK_R)
	{
		/* If they are on the ignore list and the user has the option
		 * "Show ignored users" disabled, then don't show them */
		if(pUser->IgnoreList() && !show_ignored_users)
		{
		 	FOR_EACH_USER_CONTINUE
		}

		/* If they are offline and we do not want to see offline users,
		 * just keep goin going! */
		if(pUser->Status() == ICQ_STATUS_OFFLINE && !show_offline_users)
		{
			FOR_EACH_USER_CONTINUE
		}
		
		/* Make an empty row first */
		gtk_list_store_append(store, &iter);

		/* Get the status of the user */
		gulong user_status = pUser->StatusFull();

		// The icon to set
		if (pUser->NewMessages() > 0)
		{
			CUserEvent *ue = pUser->EventPeekFirst();
			GdkPixbuf *icon;

			switch(ue->SubCommand())
			{
				case ICQ_CMDxSUB_MSG:
					icon = message_icon;
					break;

				case ICQ_CMDxSUB_URL:
					icon = url_icon;
					break;

				case ICQ_CMDxSUB_FILE:
					icon = file_icon;
					break;

				case ICQ_CMDxSUB_CHAT:
					icon = chat_icon;
					break;

				default:
					icon = message_icon;
					break;
			}

			gtk_list_store_set(store, &iter, COL_SORT, 1, COL_STATUS_IMAGE, icon, -1);

			if (flash_events && FlashList.find(pUser->Uin()) == FlashList.end()) {
				SFlash *flash = g_new0(struct SFlash, 1);

				flash->path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
				flash->icon = icon;
				flash->bFlashOn = false;
				FlashList[pUser->Uin()] = flash;
			}
		} 
   	else
		{
			gtk_list_store_set(store, &iter, COL_STATUS_IMAGE, 
					status_icon(user_status), -1);
			gtk_list_store_set(store, &iter, COL_SORT, 
					sort_order(user_status), -1);
		} // else

		gtk_list_store_set(store, &iter, COL_ALIAS_COLOR, 
				get_status_color(user_status), -1);

		// See if they are not offline and want to be auto secured
		if(pUser->Status() != ICQ_STATUS_OFFLINE && pUser->AutoSecure())
		{
			// Ok, now *can* they be auto secured?
			if((pUser->SecureChannelSupport() == SECURE_CHANNEL_SUPPORTED) && 
					!pUser->Secure())
				AutoSecureList.push_back(pUser->Uin());
		}

		gtk_list_store_set(store, &iter, COL_ALIAS, pUser->GetAlias(), -1);
		if(pUser->Secure() && (pUser->Birthday() == 0))
			gtk_list_store_set(store, &iter, COL_SSL_IMAGE, securebday_icon, -1);
		else if(pUser->Secure())
			gtk_list_store_set(store, &iter, COL_SSL_IMAGE, secure_icon, -1);
		else if(pUser->Birthday() == 0)
			gtk_list_store_set(store, &iter, COL_SSL_IMAGE, birthday_icon, -1);

		gtk_list_store_set(store, &iter, COL_PUSER, (gpointer)pUser, -1);

		num_users++;
		FOR_EACH_USER_CONTINUE
	}
	FOR_EACH_USER_END

	// Now do the auto secure stuff
	list<unsigned long>::iterator it;
	for (it = AutoSecureList.begin(); it != AutoSecureList.end(); it++)
		icq_daemon->icqOpenSecureChannel(*it);
}

GdkColor *get_status_color(unsigned long nStatus)
{
	if (nStatus == ICQ_STATUS_OFFLINE)
		return offline_color;
	
	if ((nStatus & ICQ_STATUS_AWAY) || (nStatus & ICQ_STATUS_NA) ||
	    (nStatus & ICQ_STATUS_DND) || (nStatus & ICQ_STATUS_OCCUPIED))
	    return away_color;

	return online_color;
}

void convo_open_cb(ICQUser *user);
void list_send_url(GtkWidget *, ICQUser *);
void list_request_file(GtkWidget *, ICQUser *);
void list_request_chat(GtkWidget *, ICQUser *);
void create_key_request_window(GtkWidget *, ICQUser *);
void list_read_message(GtkWidget *, ICQUser *);
void list_history(GtkWidget *, ICQUser *);
void list_more_window(GtkWidget *, ICQUser *);
void list_delete_user(GtkWidget *, ICQUser *);

gboolean contact_list_click(GtkWidget *contact_list,
			GdkEventButton *event,
			gpointer data)
{
	gchar str_status[30];
	ICQUser *user = 0;

	GtkTreeIter iter;
	GtkTreeView *tv = GTK_TREE_VIEW(contact_list);

	GtkTreePath *path;
	if (!gtk_tree_view_get_path_at_pos(tv, (gint)event->x, (gint)event->y, &path, 
			NULL, NULL, NULL))
		return FALSE;
	gtk_tree_model_get_iter(gtk_tree_view_get_model(tv), &iter, path);
	gtk_tree_model_get(gtk_tree_view_get_model(tv), &iter, COL_PUSER, &user, -1);
	
	/* No user was clicked on */
	if(user == 0)
		return FALSE;

	/* A left mouse double-click */
	if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
		convo_open(user, true);
	/* A right click.. make the popup menu */
	else if(event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		GtkWidget *_menu;
		GtkWidget *item;
		GtkWidget *separator;

		_menu = gtk_menu_new();

		/* The non-sensitive user name as a title */
		item = gtk_menu_item_new_with_label(user->GetAlias());
		gtk_menu_shell_append(GTK_MENU_SHELL(_menu), item);
		gtk_widget_set_sensitive(item, FALSE);
		gtk_widget_show(item);

		/* A separator */
		separator = gtk_hseparator_new();
		item = gtk_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(_menu), item);
		gtk_container_add(GTK_CONTAINER(item), separator);
		gtk_widget_set_sensitive(item, FALSE);
		gtk_widget_show_all(item);

		add_to_popup("Start Conversation", _menu,
			     GTK_SIGNAL_FUNC(convo_open_cb), user);

		add_to_popup("Send URL", _menu,
			     GTK_SIGNAL_FUNC(list_send_url), user);

		add_to_popup("Send Chat Request", _menu,
		     	     GTK_SIGNAL_FUNC(list_request_chat), user);

		add_to_popup("Send File Request", _menu,
		             GTK_SIGNAL_FUNC(list_request_file), user);
	
		if (user->Secure())
			add_to_popup("Close Secure Channel", _menu,
				GTK_SIGNAL_FUNC(create_key_request_window),
				user);
		else	
			add_to_popup("Request Secure Channel", _menu,
				GTK_SIGNAL_FUNC(create_key_request_window),
				user);

		/* A separator */
    separator = gtk_hseparator_new();
    item = gtk_menu_item_new(); 
    gtk_menu_shell_append(GTK_MENU_SHELL(_menu), item);
    gtk_container_add(GTK_CONTAINER(item), separator);
    gtk_widget_set_sensitive(item, FALSE);
    gtk_widget_show_all(item); 

		if (user->Status() != ICQ_STATUS_ONLINE && 
				user->Status() != ICQ_STATUS_OFFLINE)	{
			strcpy(str_status, "Read ");
			strcat(str_status, user->StatusStrShort());
			strcat(str_status, " Message");

			add_to_popup(str_status, _menu,
				     GTK_SIGNAL_FUNC(list_read_message), user);
		}

		add_to_popup("Info", _menu,
			GTK_SIGNAL_FUNC(list_info_user), user);

		add_to_popup("History", _menu,
			     GTK_SIGNAL_FUNC(list_history), user);

		add_to_popup("More...", _menu,
			     GTK_SIGNAL_FUNC(list_more_window), user);

		add_to_popup("Delete User", _menu,
			     GTK_SIGNAL_FUNC(list_delete_user), user);
	
		gtk_menu_popup(GTK_MENU(_menu), 0, 0, 0, 0, 
			       event->button, event->time);
	}
	return FALSE;
}

void add_to_popup(const gchar *label, GtkWidget *menu,
		  GtkSignalFunc func, ICQUser *user)
{
	GtkWidget *item = gtk_menu_item_new_with_label(label);
	g_signal_connect(G_OBJECT(item), "activate",
			   G_CALLBACK(func), user);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	gtk_widget_show(item);
}
