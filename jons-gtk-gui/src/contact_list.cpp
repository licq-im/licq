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

#include "licq_icqd.h"
#include "licq_user.h"

#include <string.h>
#include <gtk/gtk.h>
#include <sys/time.h>
#include <list.h>

GdkColor *red, *blue, *online_color, *offline_color, *away_color;
struct status_icon *online, *offline, *away, *na, *dnd, *occ, *ffc,
	*invisible, *message_icon, *file_icon, *chat_icon, *url_icon,
	*secure_icon, *birthday_icon, *securebday_icon, *blank_icon;
gboolean bFlashOn = false;
gint nToFlash = -1;

list<unsigned long> AutoSecureList;
list<SFlash *> FlashList;

GtkWidget *contact_list_new(gint height, gint width)
{
	GtkWidget *_contact_l;

	/* Create the contact list using a 3 column clist */
	_contact_l = gtk_clist_new(3);
	gtk_clist_set_row_height(GTK_CLIST(_contact_l), 17);

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

gint flash_icons(gpointer data)
{
	// If we aren't supposed to flash or there are no things to flash
	if(!flash_events || (nToFlash < 0))
		return -1;
		
	list<SFlash *>::iterator it;
	for(it = FlashList.begin(); it != FlashList.end(); it++)
	{
		if(!(*it)->bFlashOn)
		{
			(*it)->bFlashOn = true;
			gtk_clist_set_pixmap(GTK_CLIST(contact_list),
				(*it)->nRow, 1, blank_icon->pm,
				blank_icon->bm);
		}
		else
		{
			(*it)->bFlashOn = false;
			gtk_clist_set_pixmap(GTK_CLIST(contact_list),
				(*it)->nRow, 1, (*it)->icon->pm,
				(*it)->icon->bm);
		}
	}

	return -1;
}

void contact_list_refresh()
{
	gchar *blah[3];
	gint num_users = 0;
	nToFlash = -1;

	/* Don't update the clist window, so we can update all the users */
	gtk_clist_freeze(GTK_CLIST(contact_list));

	blah[0] = "";
	blah[1] = "";
	blah[2] = "";

	/* Clean out the list */
	gtk_clist_clear(GTK_CLIST(contact_list));

	gtk_clist_set_auto_sort(GTK_CLIST(contact_list), FALSE);

	/* Now go through all the users */
	FOR_EACH_USER_START(LOCK_R)
	{
		/* If they are on the ignore list and the user has the option
		 * "Show ignored users" diabled, then don't show them */
		if(pUser->IgnoreList() && !show_ignored_users)
		{
		 	FOR_EACH_USER_CONTINUE
		}

		/* If they are offline and we do not want to see offline users,
		 * just keep oin going! */
		 if(pUser->Status() == ICQ_STATUS_OFFLINE &&
		    !show_offline_users)
		{
			FOR_EACH_USER_CONTINUE
		}
		
		/* Make an empty row first */
		gtk_clist_insert(GTK_CLIST(contact_list), num_users, blah);

		// The icon to set
		if(pUser->NewMessages() > 0)
		{
			CUserEvent *ue = pUser->EventPeekFirst();
			struct status_icon *icon;

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

			gtk_clist_set_pixmap(GTK_CLIST(contact_list), num_users,
				1, icon->pm, icon->bm);
			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
				0, "!");

			if(flash_events)
			{
				struct SFlash *flash = g_new0(struct SFlash, 1);
				flash->nRow = ++nToFlash; 
				flash->icon = icon;
				flash->bFlashOn = false;
				flash->nUin = pUser->Uin();

				FlashList.push_back(flash);
			}

		} 

         	else
		{
			struct status_icon *cur_icon = offline;

			/* Get the status of the user */
			gulong user_status = pUser->StatusFull();

			if((gushort)user_status != ICQ_STATUS_OFFLINE &&
			   (user_status & ICQ_STATUS_FxPRIVATE))
			{
				cur_icon = invisible;
				gtk_clist_set_foreground(GTK_CLIST(contact_list)
					, num_users, online_color);
				gtk_clist_set_text(GTK_CLIST(contact_list),
					num_users, 0, ",");
			}

			else if((gushort)user_status == ICQ_STATUS_OFFLINE)
			{
				cur_icon = offline;
				gtk_clist_set_foreground(GTK_CLIST(contact_list)
					, num_users, offline_color);
				gtk_clist_set_text(GTK_CLIST(contact_list),
					num_users, 0, "~");
			}

		  	else if(user_status & ICQ_STATUS_DND)
		  	{
				cur_icon = dnd;
				gtk_clist_set_foreground(GTK_CLIST(contact_list)
					, num_users, away_color);
				gtk_clist_set_text(GTK_CLIST(contact_list),
					num_users, 0, "X");
		  	}
	
		  	else if(user_status & ICQ_STATUS_OCCUPIED)
		  	{
				cur_icon = occ;
				gtk_clist_set_foreground(GTK_CLIST(contact_list)
					, num_users, away_color);
				gtk_clist_set_text(GTK_CLIST(contact_list),
					num_users, 0, "x");
		  	}
 		 
	  	  	else if(user_status & ICQ_STATUS_NA)
		  	{
				cur_icon = na;
				gtk_clist_set_foreground(GTK_CLIST(contact_list)
					, num_users, away_color);
				gtk_clist_set_text(GTK_CLIST(contact_list),
					num_users, 0, "N");
		  	}

		  	else if(user_status & ICQ_STATUS_AWAY)
		  	{
				cur_icon = away;
				gtk_clist_set_foreground(GTK_CLIST(contact_list)
					, num_users, away_color);
				gtk_clist_set_text(GTK_CLIST(contact_list),
					num_users, 0, "A");
		  	}
	
		  	else if(user_status & ICQ_STATUS_FREEFORCHAT)
		  	{
				cur_icon = ffc;
				gtk_clist_set_foreground(GTK_CLIST(contact_list)
					, num_users, online_color);
				gtk_clist_set_text(GTK_CLIST(contact_list),
					num_users, 0, "*");
		  	}
		
		  	else if(user_status & ICQ_STATUS_OCCUPIED)
		  	{
				cur_icon = occ;
				gtk_clist_set_foreground(GTK_CLIST(contact_list)
					, num_users, away_color);
				gtk_clist_set_text(GTK_CLIST(contact_list),
					num_users, 0, "x");
		  	}

			else
			{
				cur_icon = online;
				gtk_clist_set_foreground(GTK_CLIST(contact_list)
					, num_users, online_color);
				gtk_clist_set_text(GTK_CLIST(contact_list),
					num_users, 0, "+");
			}
		
			gtk_clist_set_pixmap(GTK_CLIST(contact_list),
				num_users, 1, cur_icon->pm, cur_icon->bm);
		} // else

		// See if they are not offline and want to be auto secured
		if(pUser->Status() != ICQ_STATUS_OFFLINE && pUser->AutoSecure())
		{
			// Ok, now *can* they be auto secured?
			if((pUser->SecureChannelSupport() == SECURE_CHANNEL_SUPPORTED) && !pUser->Secure())
				AutoSecureList.push_back(pUser->Uin());
		}

		if(pUser->Secure() && (pUser->Birthday() == 0))
		{
			gtk_clist_set_pixtext(GTK_CLIST(contact_list),
				num_users, 2, pUser->GetAlias(), 4,
				securebday_icon->pm, securebday_icon->bm);
		}
		else if(pUser->Secure())
		{
			gtk_clist_set_pixtext(GTK_CLIST(contact_list),
				num_users, 2, pUser->GetAlias(), 4,
				secure_icon->pm, secure_icon->bm);
		}
		else if(pUser->Birthday() == 0)
		{
			gtk_clist_set_pixtext(GTK_CLIST(contact_list),
				num_users, 2, pUser->GetAlias(), 4,
				birthday_icon->pm, birthday_icon->bm);
		}
		else
		{
			gtk_clist_set_text(GTK_CLIST(contact_list), num_users,
				      2, pUser->GetAlias());
		}

		gtk_clist_set_row_data(GTK_CLIST(contact_list), 
					 num_users, (gpointer)pUser);

		num_users++;
		FOR_EACH_USER_CONTINUE
	}
	FOR_EACH_USER_END

	// Now do the auto secure stuff
	list<unsigned long>::iterator it;
	for(it = AutoSecureList.begin(); it != AutoSecureList.end(); it++)
	{
		icq_daemon->icqOpenSecureChannel(*it);
	}

	AutoSecureList.clear();

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

	/* No user was clicked on */
	if(user == NULL)
		return;

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

		add_to_popup("Send Chat Request", _menu,
		     	     GTK_SIGNAL_FUNC(list_request_chat), user);

		add_to_popup("Send File Request", _menu,
		             GTK_SIGNAL_FUNC(list_request_file), user);
	
		if(user->Secure())
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
                gtk_menu_append(GTK_MENU(_menu), item);
                gtk_container_add(GTK_CONTAINER(item), separator);
                gtk_widget_set_sensitive(item, FALSE);
                gtk_widget_show_all(item); 

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
