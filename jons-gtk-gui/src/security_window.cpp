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

#include <gtk/gtk.h>

struct user_security *us;

void menu_security_users_window(GtkWidget *widget, gpointer data)
{
	/* Check to see if the window is currently open */
	if(us != NULL)
	{
		gdk_window_raise(us->window->window);
		return;
	}

	GtkWidget *notebook;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *scroll1;
	GtkWidget *scroll2;
	GtkWidget *scroll3;
	GtkWidget *visible_list;
	GtkWidget *invisible_list;
	GtkWidget *ignore_list;
	GtkWidget *v_box;
	GtkWidget *spam_box;
	GtkWidget *h_box;
	GtkWidget *statusbar;
	GtkWidget *ok;
	GtkWidget *close;
	GtkTooltips *tooltips;

	us = g_new0(struct user_security, 1);

	/* The etag stuff */
	us->etag = g_new0(struct e_tag_data, 1);

	/* Setup the tooltips for the general tab */
	tooltips = gtk_tooltips_new();

	/* Create the window */	
	us->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(us->window), "Licq - User Security");
	gtk_window_set_position(GTK_WINDOW(us->window), GTK_WIN_POS_CENTER);

	/* Destroy signal for the window */	
	gtk_signal_connect(GTK_OBJECT(us->window), "destroy",
			   GTK_SIGNAL_FUNC(close_user_security_window), NULL);

	/* Create the table */
	table = gtk_table_new(3, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(us->window), table);

	/* Create the notebook */
	notebook = gtk_notebook_new();
	gtk_signal_connect(GTK_OBJECT(notebook), "switch-page",
			   GTK_SIGNAL_FUNC(switch_page), NULL);

	/* The scroll bar for the first clist */
	scroll1 = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll1),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scroll1, 380, 175);

	/* The first clist is the Visible Users */
	visible_list = make_user_security_clist();
	gtk_container_add(GTK_CONTAINER(scroll1), visible_list);
	refresh_clist(GTK_CLIST(visible_list), 0);

	/* The scroll bar for the second clist */
	scroll2 = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll2),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scroll2, 380, 175);

	/* The second is the invisible list */
	invisible_list = make_user_security_clist();
	gtk_container_add(GTK_CONTAINER(scroll2), invisible_list);
	refresh_clist(GTK_CLIST(invisible_list), 1);

	/* The scroll bar for the third clist */
	scroll3 = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll3),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scroll2, 380, 175);

	/* The ignore list is 3rd */
	ignore_list = make_user_security_clist();
	gtk_container_add(GTK_CONTAINER(scroll3), ignore_list);
	refresh_clist(GTK_CLIST(ignore_list), 2);

	/* Now the general tab */
	v_box = gtk_vbox_new(FALSE, 5);
	
	/* First option */
	us->check_auth = gtk_check_button_new_with_label("Authorization Required");
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips), us->check_auth,
		"Determines whether regular ICQ clients require your authorization to add you to their contact list.", NULL);
	gtk_box_pack_start(GTK_BOX(v_box), us->check_auth, FALSE, FALSE, 0);

	/* Second option */
	us->check_web = gtk_check_button_new_with_label("Web Presence");
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips), us->check_web,
		"Web Presence allows users to see if you are online through your web indicator.", NULL);	
	gtk_box_pack_start(GTK_BOX(v_box), us->check_web, FALSE, FALSE, 0);

	/* Third optoin */
	us->check_hideip = gtk_check_button_new_with_label("Hide IP");
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips), us->check_hideip,
		"Hiding IP is a minor prevention for regular ICQ clients to not reveal your IP to users.", NULL);
	gtk_box_pack_start(GTK_BOX(v_box), us->check_hideip, FALSE, FALSE, 0);

	/* Set the check boxes accordingly */
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(us->check_auth),
				    owner->GetAuthorization());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(us->check_web),
				    owner->WebAware());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(us->check_hideip),
				    owner->HideIp());

	gUserManager.DropOwner();

	/* The spam tab */
	spam_box = gtk_vbox_new(FALSE, 5);
	
	/* Ignore stuff.. */
	us->ign_new = gtk_check_button_new_with_label("Ignore New Users");
	us->ign_mass = gtk_check_button_new_with_label("Ignore Mass Messages");
	us->ign_web = gtk_check_button_new_with_label("Ignore Web Panel");
	us->ign_pager = gtk_check_button_new_with_label("Ignore E-mail Pager");

	/* Set the check boxes */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(us->ign_new),
				     icq_daemon->Ignore(IGNORE_NEWUSERS));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(us->ign_mass),
				     icq_daemon->Ignore(IGNORE_MASSMSG));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(us->ign_web),
				     icq_daemon->Ignore(IGNORE_WEBPANEL));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(us->ign_pager),
				     icq_daemon->Ignore(IGNORE_EMAILPAGER));

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(spam_box), us->ign_new, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(spam_box), us->ign_mass, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(spam_box), us->ign_web, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(spam_box), us->ign_pager, FALSE, FALSE, 0);

	/* Add everything to the notebook */
	label = gtk_label_new("Visible List");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll1, label);
	label = gtk_label_new("Invisible List");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll2, label);
	label = gtk_label_new("Ignore List");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll3, label);
	label = gtk_label_new("General");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), v_box, label);
	label = gtk_label_new("Spam");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), spam_box, label);

	/* Attach the notebook to the table */
	gtk_table_attach(GTK_TABLE(table), notebook, 0, 1, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	/* The statusbar */
	statusbar = gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table), statusbar, 0, 2, 1, 2,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	/* Final items for the etag info */
	us->etag->statusbar = statusbar;
	strcpy(us->etag->buf, "");

	/* The buttons and an h_box for them to go into */
	ok = gtk_button_new_with_label("Apply");
	close = gtk_button_new_with_label("Close");
	h_box = gtk_hbox_new(TRUE, 15);

	/* Connect button signals */
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(ok_user_security), NULL);
	gtk_signal_connect(GTK_OBJECT(close), "clicked",
			   GTK_SIGNAL_FUNC(close_user_security_window), NULL);

	/* Pack them into the h_box */
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), close, TRUE, TRUE, 0);

	/* Attach the h_box to the table */
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 2, 3,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	/* Show all the widgets now */
	gtk_widget_show_all(us->window);
}

GtkWidget *make_user_security_clist()
{
	GtkWidget *clist = gtk_clist_new(3);

	/* Double click on a user to remove from this list */
	gtk_signal_connect(GTK_OBJECT(clist), "button_press_event",
			   GTK_SIGNAL_FUNC(remove_user_security), NULL);
	
	gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_BROWSE);
	gtk_clist_column_titles_show(GTK_CLIST(clist));
	gtk_clist_column_titles_passive(GTK_CLIST(clist));
	gtk_clist_set_shadow_type(GTK_CLIST(clist), GTK_SHADOW_ETCHED_IN);

	/* The widths */
	gtk_clist_set_column_width(GTK_CLIST(clist), 0, 60);
	gtk_clist_set_column_width(GTK_CLIST(clist), 1, 110);
	gtk_clist_set_column_width(GTK_CLIST(clist), 2, 170);

	/* Set the titles */
	gtk_clist_set_column_title(GTK_CLIST(clist), 0, "UIN");
	gtk_clist_set_column_title(GTK_CLIST(clist), 1, "Name");
	gtk_clist_set_column_title(GTK_CLIST(clist), 2, "E-mail");

	return clist;
}

void switch_page(GtkNotebook *notebook, GtkNotebookPage *_page,
		 gint page_num, gpointer data)
{
	if(us == NULL)
		return;

	us->page = page_num;
}

void refresh_clist(GtkCList *clist, gint page)
{
	gint users = 0;
	gchar *user[3];

	gtk_clist_freeze(clist);
	gtk_clist_clear(clist);

	switch(page)
	{
	case 0:
		FOR_EACH_USER_START(LOCK_R)
		{
			if(pUser->VisibleList())
			{
			user[0] = g_strdup_printf("%ld", pUser->Uin());
			user[1] = g_strdup_printf("%s",pUser->GetAlias());
			user[2] = pUser->GetEmailPrimary();

			gtk_clist_insert(clist, users, user);
			gtk_clist_set_row_data(clist, users, (gpointer)pUser);
			users++;
			}

			FOR_EACH_USER_CONTINUE
		}
		FOR_EACH_USER_END
		break;
	case 1:
		FOR_EACH_USER_START(LOCK_R)
		{
			if(pUser->InvisibleList())
			{
			user[0] = g_strdup_printf("%ld", pUser->Uin());
			user[1] = g_strdup_printf("%s",pUser->GetAlias());
			user[2] = pUser->GetEmailPrimary();

			gtk_clist_insert(clist, users, user);
			gtk_clist_set_row_data(clist, users, (gpointer)pUser);
			users++;
			}

			FOR_EACH_USER_CONTINUE
		}
		FOR_EACH_USER_END
		break;	
	case 2:
		FOR_EACH_USER_START(LOCK_R)
		{
			if(pUser->IgnoreList())
			{
			user[0] = g_strdup_printf("%ld", pUser->Uin());
			user[1] = g_strdup_printf("%s",pUser->GetAlias());
			user[2] = pUser->GetEmailPrimary();

			gtk_clist_insert(clist, users, user);
			gtk_clist_set_row_data(clist, users, (gpointer)pUser);
			users++;
			}

			FOR_EACH_USER_CONTINUE
		}
		FOR_EACH_USER_END
		break;	
	default:
		break;
	}

	gtk_clist_thaw(clist);
}

void close_user_security_window(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(us->window);
	us = NULL;
}

void ok_user_security(GtkWidget *widget, gpointer data)
{
	/* Set the ignore options */
	icq_daemon->SetIgnore(IGNORE_NEWUSERS,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(us->ign_new)));
	icq_daemon->SetIgnore(IGNORE_MASSMSG,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(us->ign_mass)));
	icq_daemon->SetIgnore(IGNORE_WEBPANEL,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(us->ign_web)));
	icq_daemon->SetIgnore(IGNORE_EMAILPAGER,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(us->ign_pager)));

	const gchar *buffer = "Updating Security options .. ";
	
	us->etag->e_tag =
	   icq_daemon->icqSetSecurityInfo(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(us->check_auth)),
	      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(us->check_hideip)),
	        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(us->check_web)));

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(us->etag->statusbar), "sta");
	gtk_statusbar_pop(GTK_STATUSBAR(us->etag->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(us->etag->statusbar), id, buffer);

	strcpy(us->etag->buf, buffer);
	catcher = g_slist_append(catcher, us->etag);
}

void remove_user_security(GtkWidget *clist, GdkEventButton *event, gpointer data)
{
	/* If it's not a double click, we don't care */
	if(!(event->button == 1 && event->type == GDK_2BUTTON_PRESS))
		return;

	gint row;
	gint column;
	ICQUser *user;

	/* Get which cells was clicked on, to find the user */
	gtk_clist_get_selection_info(GTK_CLIST(clist), (gint)event->x,
				     (gint)event->y, &row, &column);

	/* Now the user */
	user = (ICQUser *)gtk_clist_get_row_data(GTK_CLIST(clist), row);

	/* No user selected */
	if(user == NULL)
		return;

	/* What list are they on? */
	switch(us->page)
	{
	case 0:
		user->SetVisibleList(FALSE);
		break;
	case 1:
		user->SetInvisibleList(FALSE);
		break;
	case 2:
		user->SetIgnoreList(FALSE);
		break;
	default:
		break;
	}

	refresh_clist(GTK_CLIST(clist), us->page);
}
