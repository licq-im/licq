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

#include "pixmaps/online.xpm"
#include "pixmaps/offline.xpm"
#include "pixmaps/away.xpm"
#include "pixmaps/na.xpm"
#include "pixmaps/occ.xpm"
#include "pixmaps/dnd.xpm"
#include "pixmaps/message.xpm"

#include "icqd.h"
#include "user.h"

#include <string.h>
#include <gtk/gtk.h>

GtkWidget *contact_list_new(gint height, gint width)
{
	GtkWidget *_contact_l;

	/* Create the contact list using a 2 column clist */
	_contact_l = gtk_clist_new(3);

	/* Set the selection mode */
	gtk_clist_set_selection_mode(GTK_CLIST(_contact_l),
				     GTK_SELECTION_BROWSE);

	/* Set the shadow mode and column widths */
	gtk_clist_set_shadow_type(GTK_CLIST(_contact_l),GTK_SHADOW_ETCHED_IN);
	gtk_clist_set_column_width(GTK_CLIST(_contact_l), 0, 0);
	gtk_clist_set_column_width(GTK_CLIST(_contact_l), 1, 16);
	gtk_clist_set_column_width(GTK_CLIST(_contact_l), 2, width - 25);

	/* The first column is for sorting only...... do not show it!
	 * *cough*dirtyhack*cough*
	 */
	gtk_clist_set_column_visibility(GTK_CLIST(_contact_l), 0, FALSE);

	/* Size the contact list */
	gtk_widget_set_usize(_contact_l, width, height);

	/* A double click on a user name */
	gtk_signal_connect(GTK_OBJECT(_contact_l), "button_press_event",
			   GTK_SIGNAL_FUNC(contact_list_click), NULL);

	return _contact_l;
}

GtkWidget *contact_list_refresh()
{
	gchar *blah[3];
	gint num_users = 0;
	GdkBitmap *bm;
	GdkPixmap *online, *offline, *away, *na, *dnd, *occ, *message;
	GtkStyle *style;
	GdkColor *red, *green, *blue;
	red = new GdkColor;
	green = new GdkColor;
	blue = new GdkColor;

	red->red = 30000;
	red->green = 0;
	red->blue = 0;
	red->pixel = (gulong)(255);

	green->red = 0;
	green->green = 30000;
	green->blue = 0;
	green->pixel = (gulong)(255*256);

	blue->red = 0;
	blue->green = 0;
	blue->blue = 30000;
	blue->pixel = (gulong)(255);

	blah[0] = "";
	blah[1] = "";
	blah[2] = "";

	style = gtk_widget_get_style(main_window);

	online = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
					      &style->bg[GTK_STATE_NORMAL],
					      (gchar **)online_xpm);

	away = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
					    &style->bg[GTK_STATE_NORMAL],
					    (gchar **)away_xpm);

	na = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
					  &style->bg[GTK_STATE_NORMAL],
					  (gchar **)na_xpm);

	dnd = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
					   &style->bg[GTK_STATE_NORMAL],
					   (gchar **)dnd_xpm);

	occ = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
					   &style->bg[GTK_STATE_NORMAL],
					   (gchar **)occ_xpm);

	offline = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
					       &style->bg[GTK_STATE_NORMAL],
					       (gchar **)offline_xpm);

	message = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
					       &style->bg[GTK_STATE_NORMAL],
					       (gchar **)message_xpm);

	/* Don't update the clist window, so we can update all the users */
	gtk_clist_freeze(GTK_CLIST(contact_list));

	/* Clean out the list */
	gtk_clist_clear(GTK_CLIST(contact_list));

	gtk_clist_set_auto_sort(GTK_CLIST(contact_list), FALSE);

	/* Now go through all the users */
	FOR_EACH_USER_START(LOCK_R)
	{
		/* Make an empty row first */
		gtk_clist_insert(GTK_CLIST(contact_list), num_users, blah);

		if(pUser->NewMessages() > 0)
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
					     1, message, bm);
		
	else {
		/* Get the status of the user */
		gushort user_status = pUser->Status();

		switch(user_status)
		{
		  case ICQ_STATUS_FREEFORCHAT:
		  {
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
					     1, online, bm);
			gtk_clist_set_foreground(GTK_CLIST(contact_list),
						 num_users, blue);
			break;
		  }
	
		  case ICQ_STATUS_AWAY:
		  {
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
					     1, away, bm);
			gtk_clist_set_foreground(GTK_CLIST(contact_list),
						 num_users, green);
			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
					   0, "A");

			break;
		  }
 		 
	  	  case ICQ_STATUS_ONLINE:
		  {
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
					     1, online, bm);
			gtk_clist_set_foreground(GTK_CLIST(contact_list),
						 num_users, blue);
			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
					   0, "+");
			break;
		  }

		  case ICQ_STATUS_NA:
		  {
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
					     1, na, bm);
			gtk_clist_set_foreground(GTK_CLIST(contact_list),
						 num_users, green);
			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
					   0, "N");
			break;
		  }
	
		  case ICQ_STATUS_DND:
		  {
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
					     1, dnd, bm);
			gtk_clist_set_foreground(GTK_CLIST(contact_list),
						 num_users, green);
			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
					   0, "X");
			break;
		  }
		
		  case ICQ_STATUS_OCCUPIED:
		  {
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
					     1, occ, bm);
			gtk_clist_set_foreground(GTK_CLIST(contact_list),
						 num_users, green);
			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
					   0, "x");
			break;
		  }
		
		  case ICQ_STATUS_OFFLINE:
		  {
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
					     1, offline, bm);
			gtk_clist_set_foreground(GTK_CLIST(contact_list),
						 num_users, red);
			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
					   0, "~");
			break;
		  }
		
		  default:
			g_print("Unknown status\n");
		} //switch
	} //else

		gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
				      2, pUser->GetAlias());

		gtk_clist_set_row_data(GTK_CLIST(contact_list), 
					 num_users, (gpointer)pUser);

		num_users++;
		FOR_EACH_USER_CONTINUE
	}
	FOR_EACH_USER_END

	gtk_clist_set_compare_func(GTK_CLIST(contact_list), NULL);
	gtk_clist_set_sort_column(GTK_CLIST(contact_list), 0);
	gtk_clist_sort(GTK_CLIST(contact_list));

	/* Now show the updated list */
	gtk_clist_thaw(GTK_CLIST(contact_list));
}

void contact_list_click(GtkWidget *contact_list,
			GdkEventButton *event,
			gpointer data)
{
	gint row;
	gint column;
	ICQUser *user;
	struct conversation *c = NULL;

	/* Get which cell was clicked in to find that user */
	gtk_clist_get_selection_info(GTK_CLIST(contact_list),
				     (gint)event->x,
				     (gint)event->y,
				     &row,
				     &column);

	/* Now the the user */
	user = (ICQUser *)gtk_clist_get_row_data(GTK_CLIST(contact_list), row);

	/* A double click */
	if(event->type == GDK_2BUTTON_PRESS && event->button == 1)
	{
		c = convo_find(user->Uin());

		if(c != NULL)
			gdk_window_raise(c->window->window);
		else
		{
			if(user->NewMessages() > 0)
				c = convo_new(user, TRUE);

			else
				c = convo_new(user, FALSE);

			system_status_refresh();
		}

	}

	/* A right click.. make the popup menu */
	else if(event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		GtkWidget *_menu, *_item;

		_menu = gtk_menu_new();

		_item = gtk_menu_item_new_with_label("Start Conversation");
		gtk_signal_connect(GTK_OBJECT(_item), "activate",
				   GTK_SIGNAL_FUNC(list_start_convo), user);
		gtk_menu_append(GTK_MENU(_menu), _item);
		gtk_widget_show(_item);

		_item = gtk_menu_item_new_with_label("Send URL");
		gtk_signal_connect(GTK_OBJECT(_item), "activate",
				   GTK_SIGNAL_FUNC(list_send_url), user);
		gtk_menu_append(GTK_MENU(_menu), _item);
		gtk_widget_show(_item);

		_item = gtk_menu_item_new_with_label("Delete User");
		gtk_signal_connect(GTK_OBJECT(_item), "activate",
				   GTK_SIGNAL_FUNC(list_delete_user), user);
		gtk_menu_append(GTK_MENU(_menu), _item);
		gtk_widget_show(_item);
		
		gtk_menu_popup(GTK_MENU(_menu), NULL, NULL, NULL, NULL, 
			       event->button, event->time);
	
	}
}
