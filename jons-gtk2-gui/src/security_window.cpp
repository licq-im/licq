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
#include "utilities.h"

#include <gtk/gtk.h>

struct security_window
{
        GtkWidget *window;
        GtkWidget *check_auth;
        GtkWidget *check_web;
        GtkWidget *check_hideip;
        GtkTooltips *tooltips;
	struct e_tag_data *etag;
};

struct user_security
{
	GtkWidget *window;
	GtkWidget *check_auth;
	GtkWidget *check_web;
	GtkWidget *check_hideip;
	GtkWidget *ign_new;
	GtkWidget *ign_web;
	GtkWidget *ign_mass;
	GtkWidget *ign_pager;
	gint page;
	struct e_tag_data *etag;
};

struct user_security *us;

GtkWidget *make_user_security_clist();
void switch_page(GtkNotebook *, GtkNotebookPage *, gint, gpointer);
void refresh_clist(GtkWidget *, gint);
void close_user_security_window(GtkWidget *, gpointer);
void ok_user_security(GtkWidget *, gpointer);
void remove_user_security(GtkWidget *, GdkEventButton *, gpointer);

void menu_security_users_window(GtkWidget *widget, gpointer data)
{
	/* Check to see if the window is currently open */
	if(us != 0)
	{
		gtk_window_present(GTK_WINDOW(us->window));
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
	g_signal_connect(G_OBJECT(us->window), "destroy",
			   G_CALLBACK(close_user_security_window), 0);
	
	/* Create the table */
	table = gtk_table_new(3, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(us->window), table);

	/* Create the notebook */
	notebook = gtk_notebook_new();
	g_signal_connect(G_OBJECT(notebook), "switch-page",
			   G_CALLBACK(switch_page), 0);
	
	/* The scroll bar for the first clist */
	scroll1 = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll1),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scroll1, 380, 175);
	
	/* The first clist is the Visible Users */
	visible_list = make_user_security_clist();
	gtk_container_add(GTK_CONTAINER(scroll1), visible_list);
	refresh_clist(visible_list, 0);

	/* The scroll bar for the second clist */
	scroll2 = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll2),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scroll2, 380, 175);

	/* The second is the invisible list */
	invisible_list = make_user_security_clist();
	gtk_container_add(GTK_CONTAINER(scroll2), invisible_list);
	refresh_clist(invisible_list, 1);

	/* The scroll bar for the third clist */
	scroll3 = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll3),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scroll2, 380, 175);

	/* The ignore list is 3rd */
	ignore_list = make_user_security_clist();
	gtk_container_add(GTK_CONTAINER(scroll3), ignore_list);
	refresh_clist(ignore_list, 2);

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

	/* Third option */
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
	ok = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	close = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	h_box = gtk_hbox_new(TRUE, 15);

	/* Connect button signals */
	g_signal_connect(G_OBJECT(ok), "clicked",
			   G_CALLBACK(ok_user_security), 0);
	g_signal_connect(G_OBJECT(close), "clicked",
			   G_CALLBACK(close_user_security_window), 0);
	
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
	GtkListStore *store = gtk_list_store_new(4, 
			G_TYPE_ULONG, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	GtkWidget *clist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	GtkCellRenderer *r_text = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col = 
			gtk_tree_view_column_new_with_attributes("UIN",
					r_text, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(clist), col);
	col = gtk_tree_view_column_new_with_attributes("Name",
					r_text, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(clist), col);
	col = gtk_tree_view_column_new_with_attributes("E-mail",
					r_text, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(clist), col);
	g_signal_connect(G_OBJECT(clist), "button_press_event",
			   G_CALLBACK(remove_user_security), 0);
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(clist));
	gtk_tree_selection_set_mode(sel, GTK_SELECTION_BROWSE);
	
	return clist;
}

void switch_page(GtkNotebook *notebook, GtkNotebookPage *_page,
		 gint page_num, gpointer data)
{
	if(us == 0)
		return;

	us->page = page_num;
}

void refresh_clist(GtkWidget *w, gint page)
{
	gint users = 0;

	GtkListStore *store = 
			GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(w)));
	gtk_list_store_clear(store);
	GtkTreeIter iter;
	
	switch(page)
	{
	case 0:
		FOR_EACH_USER_START(LOCK_R)
		{
			if(pUser->VisibleList())
			{
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
					0, pUser->Uin(),
					1, (s_convert_to_utf8(pUser->GetAlias(), pUser->UserEncoding())).c_str(),
					2, pUser->GetEmailPrimary(),
					3, (gpointer)pUser, -1);
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
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
					0, pUser->Uin(),
					1, (s_convert_to_utf8(pUser->GetAlias(), pUser->UserEncoding())).c_str(),
					2, pUser->GetEmailPrimary(),
					3, (gpointer)pUser, -1);
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
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
					0, pUser->Uin(),
					1, (s_convert_to_utf8(pUser->GetAlias(), pUser->UserEncoding())).c_str(),
					2, pUser->GetEmailPrimary(),
					3, (gpointer)pUser, -1);
			users++;
			}

			FOR_EACH_USER_CONTINUE
		}
		FOR_EACH_USER_END
		break;	
	default:
		break;
	}
}

void close_user_security_window(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(us->window);
	us = 0;
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

	ICQUser *user;

	/* Get which cells was clicked on, to find the user */
	GtkTreeModel *model;
	GtkTreeIter iter;	
	GtkTreeSelection *sel =	gtk_tree_view_get_selection(GTK_TREE_VIEW(clist));
	if (!gtk_tree_selection_get_selected(sel, &model, &iter))
		return;
	
	/* Now the user */
	gtk_tree_model_get(model, &iter, 3, (gpointer *)&user, -1);

	/* No user selected */
	if(user == 0)
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

	refresh_clist(clist, us->page);
}
