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

#include "licq_icqd.h"
#include "licq_user.h"

#include <string.h>
#include <gtk/gtk.h>
#include <sys/time.h>

GdkColor *red, *blue, *green;
GtkStyle *style;
GdkBitmap *bm;
GdkPixmap *online, *offline, *away, *na, *dnd, *occ, *message;

GtkWidget *contact_list_new(gint height, gint width)
{
	GtkWidget *_contact_l;

	/* Create the contact list using a 3 column clist */
	_contact_l = gtk_clist_new(3);

	/* Set the shadow mode and column widths */
	gtk_clist_set_shadow_type(GTK_CLIST(_contact_l),GTK_SHADOW_ETCHED_IN);
	gtk_clist_set_column_width(GTK_CLIST(_contact_l), 0, 0);
	gtk_clist_set_column_width(GTK_CLIST(_contact_l), 1, 16);
	gtk_clist_set_column_width(GTK_CLIST(_contact_l), 2, width - 25);

	/* The first column is for sorting only...... do not show it! */
	gtk_clist_set_column_visibility(GTK_CLIST(_contact_l), 0, FALSE);

	/* Size the contact list */
	gtk_widget_set_usize(_contact_l, width, height);

	/* A double click on a user name */
	gtk_signal_connect(GTK_OBJECT(_contact_l), "button_press_event",
			   GTK_SIGNAL_FUNC(contact_list_click), NULL);

	gtk_clist_set_button_actions(GTK_CLIST(_contact_l), 0, GTK_BUTTON_IGNORED);

	return _contact_l;
}

void contact_list_refresh()
{
	gchar *blah[3];
	gint num_users = 0;
	GtkWidget *pix;

	do_colors(); 	/* Make the colors */
	do_pixmaps();   /* Make the pixmap */

	blah[0] = "";
	blah[1] = "";
	blah[2] = "";

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
		{
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
					     1, message, bm);

			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
					   0, "!");
		} 

	else {
		/* Get the status of the user */
		gushort user_status = pUser->Status();

		switch(user_status)
		{
		  case ICQ_STATUS_FREEFORCHAT:
		  {
			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users					     ,1 , online, bm);
			gtk_clist_set_foreground(GTK_CLIST(contact_list),
						 num_users, blue);
			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
					   0, "*");
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
			pix = gtk_pixmap_new(online, bm);
			gtk_widget_show(pix);
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
	} // else

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
	gchar str_status[30];
	ICQUser *user;
	struct conversation *c = NULL;
	struct timeval check_timer;

	/* Get which cell was clicked in to find that user */
	gtk_clist_get_selection_info(GTK_CLIST(contact_list),
				     (gint)event->x,
				     (gint)event->y,
				     &row,
				     &column);

	/* Now the the user */
	user = (ICQUser *)gtk_clist_get_row_data(GTK_CLIST(contact_list), row);

	/* A left mouse click */
	if(event->button == 1)
	{
		/* Fix the stupid contact list double click problem */
		gettimeofday(&check_timer, NULL);

		if(!((check_timer.tv_sec == timer.tv_sec) &&
		   (check_timer.tv_usec - timer.tv_usec) < 2000))
		{
			timer.tv_sec = check_timer.tv_sec;
			timer.tv_usec = check_timer.tv_usec;
			return;
		}

		timer.tv_sec = 0;
		timer.tv_usec = 0;
	
		c = convo_find(user->Uin());
		
		if(c != NULL)
			gdk_window_raise(c->window->window);
		else
		{
			if(user->NewMessages() > 0)
				c = convo_new(user, TRUE);

			else
				c = convo_new(user, FALSE);

			contact_list_refresh();
			system_status_refresh();
		}
	}

	/* A right click.. make the popup menu */
	else if(event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		GtkWidget *_menu;
		GtkWidget *item;
		GtkWidget *separator;

		_menu = gtk_menu_new();

		/* The non-sensitive user name as a title */
		item = gtk_menu_item_new_with_label(user->GetAlias());
		gtk_menu_append(GTK_MENU(_menu), item);
		gtk_widget_set_sensitive(item, FALSE);
		gtk_widget_show(item);

		/* A separator */
		separator = gtk_hseparator_new();
		item = gtk_menu_item_new();
		gtk_menu_append(GTK_MENU(_menu), item);
		gtk_container_add(GTK_CONTAINER(item), separator);
		gtk_widget_set_sensitive(item, FALSE);
		gtk_widget_show_all(item);

		add_to_popup("Start Conversation", _menu,
			     GTK_SIGNAL_FUNC(list_start_convo), user);

		add_to_popup("Send URL", _menu,
			     GTK_SIGNAL_FUNC(list_send_url), user);

		if(user->Status() != ICQ_STATUS_ONLINE && 
		   user->Status() != ICQ_STATUS_OFFLINE)
		{
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
	
		gtk_menu_popup(GTK_MENU(_menu), NULL, NULL, NULL, NULL, 
			       event->button, event->time);
	
	}
}

void add_to_popup(const gchar *label, GtkWidget *menu,
		  GtkSignalFunc func, ICQUser *user)
{
	GtkWidget *item = gtk_menu_item_new_with_label(label);
	gtk_signal_connect(GTK_OBJECT(item), "activate",
			   GTK_SIGNAL_FUNC(func), user);
	gtk_menu_append(GTK_MENU(menu), item);
	gtk_widget_show(item);
}

