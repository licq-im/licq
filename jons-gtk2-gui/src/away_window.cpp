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

#include <gtk/gtk.h>

#include "licq_icqd.h"
#include "licq_gtk.h"
#include "utilities.h"

GSList *uaw_list;

struct away_dialog
{
	GtkWidget *window;
	GtkWidget *text;
};

struct user_away_window
{
	GtkWidget *window;
	GtkWidget *show_again;
	GtkWidget *text_box;
	ICQUser *user;
	GtkWidget *progress;
	gchar buffer[30];
	struct e_tag_data *etag;
};

void
set_away_msg(GtkWidget *widget, struct away_dialog *away_d)
{
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_W);
	owner->SetAutoResponse(textview_get_chars(away_d->text).c_str());
	gUserManager.DropOwner();

	window_close(NULL, away_d->window);
}

void
away_msg_window(gushort status)
{
	
  static struct away_dialog *away_d = NULL;
  
  if (away_d == NULL)
    away_d = g_new0(struct away_dialog, 1);
  else {
    gtk_window_present(GTK_WINDOW(away_d->window));
    return;
  }

	gchar *title = g_strdup_printf("Set %s Response",
                                 ICQUser::StatusToStatusStr(status, FALSE));

	/* Make the main window */
	away_d->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(away_d->window), title);
	gtk_window_set_position(GTK_WINDOW(away_d->window), GTK_WIN_POS_CENTER);
	g_free(title);

	/* The text box */
	away_d->text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(away_d->text), TRUE);
	gtk_widget_set_size_request(GTK_WIDGET(away_d->text), 300, 100);

	// Insert the current away message
  ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
  GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(away_d->text));
  gtk_text_buffer_set_text(tb, owner->AutoResponse(), -1);
	gUserManager.DropOwner();

	/* The boxes */
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);

	/* Pack the text box into the v_box */
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_add(GTK_CONTAINER(frame), away_d->text);
	gtk_box_pack_start(GTK_BOX(v_box), frame, TRUE, TRUE, 0);

	/* Make the buttons now */
	GtkWidget *ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	GtkWidget *cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);

	/* Pack the buttons */
	GtkWidget *h_box = hbutton_box_new();
	gtk_container_add(GTK_CONTAINER(h_box), ok);
	gtk_container_add(GTK_CONTAINER(h_box), cancel);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

	/* Connect the signals now */
	g_signal_connect(G_OBJECT(cancel), "clicked",
			   G_CALLBACK(window_close), away_d->window);
	g_signal_connect(G_OBJECT(away_d->window), "destroy",
			   G_CALLBACK(destroy_cb), &away_d);
	g_signal_connect(G_OBJECT(ok), "clicked",
			   G_CALLBACK(set_away_msg), away_d);
	
	/* Get the window ready to be shown, show it, and set is_shown */
	gtk_container_add(GTK_CONTAINER(away_d->window), v_box);
	gtk_container_set_border_width(GTK_CONTAINER(away_d->window), 10);
	gtk_window_set_focus(GTK_WINDOW(away_d->window), away_d->text);
	gtk_widget_show_all(away_d->window);
}

/* The following will ensure only one window is open and that when the *
** Auto Response is received, it pops up in the box there.  Now the    *
** status of the connection will be brought back to this window        */ 

void 
destroy_away_window(GtkWidget *widget, struct user_away_window *uaw)
{
	uaw->user->SetShowAwayMsg(gtk_toggle_button_get_active(
					GTK_TOGGLE_BUTTON(uaw->show_again)));
	uaw_list = g_slist_remove(uaw_list, uaw);
	catcher = g_slist_remove(catcher, uaw->etag);
	g_free(uaw->etag);
  g_free(uaw);
}

struct user_away_window *
uaw_find(unsigned long uin)
{
	struct user_away_window *uaw;
	GSList *temp_uaw_list = uaw_list;

	while (temp_uaw_list) {
		uaw = (struct user_away_window *)temp_uaw_list->data;
		if (uaw->user->Uin() == uin)
			return uaw;
		temp_uaw_list = temp_uaw_list->next;
	}

	/* It wasn't found, return null */
	return 0;
}

struct user_away_window *
uaw_new(ICQUser *u)
{
	/* Does it exist already? */
	struct user_away_window *uaw = uaw_find(u->Uin());

	/* If it does, return it, if not, make one */
	if (uaw != 0) 
		return uaw;

	uaw = g_new0(struct user_away_window, 1);

	uaw->user = u;

	uaw_list = g_slist_append(uaw_list, uaw);

	//list_read_message(0, uaw->user);

	return uaw;
}

void
list_read_message(GtkWidget *widget, ICQUser *user)
{
	struct user_away_window *uaw = uaw_find(user->Uin());
	
	// we're in the process of retrieving the message (or it's displayed already)
  if (uaw != NULL) {
    gtk_window_present(GTK_WINDOW(uaw->window));
		return;
  }

	uaw = uaw_new(user);
	uaw->etag = g_new0(struct e_tag_data, 1);
	uaw->user = user;

	gchar *title = g_strdup_printf("Auto Response for %s", user->GetAlias());

	/* Make the window */
	uaw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(uaw->window), title);
	gtk_window_set_position(GTK_WINDOW(uaw->window), GTK_WIN_POS_CENTER);
	g_free(title);

	g_signal_connect(G_OBJECT(uaw->window), "destroy",
			   G_CALLBACK(destroy_away_window), uaw);

	/* Make the boxes */
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);

	/* The scrolling window */
	GtkWidget *scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
      GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC);

	/* The text box */
	uaw->text_box = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(uaw->text_box), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(uaw->text_box), GTK_WRAP_WORD);
	gtk_widget_set_size_request(uaw->text_box, 235, 60);
  
	/* Add the text box to the scrolling window */
  GtkWidget *frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(scroll), uaw->text_box);
	gtk_container_add(GTK_CONTAINER(frame), scroll);

	/* Pack the scrolled window into the v_box */
	gtk_box_pack_start(GTK_BOX(v_box), frame, TRUE, TRUE, 5);

	/* The Show Again check button */
	uaw->show_again = gtk_check_button_new_with_label("Show Again");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(uaw->show_again),
      user->ShowAwayMsg());

	/* The close button */
	GtkWidget *close = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(close), "clicked",
			   G_CALLBACK(window_close), uaw->window);

	/* Pack everything */
	GtkWidget *h_box = hbutton_box_new();
	gtk_container_add(GTK_CONTAINER(h_box), uaw->show_again);
	gtk_container_add(GTK_CONTAINER(h_box), close);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	/* The progress bar */
	uaw->progress = gtk_statusbar_new();
	strcpy(uaw->buffer, "Checking Response ... ");
	status_change(uaw->progress, "sta", uaw->buffer);

	/* Pack the progress bar */
	gtk_box_pack_start(GTK_BOX(v_box), uaw->progress, FALSE, FALSE, 5);

	gtk_container_set_border_width(GTK_CONTAINER(uaw->window), 10);
	gtk_container_add(GTK_CONTAINER(uaw->window), v_box);
	gtk_widget_show_all(uaw->window);

	/* etd stuff */
	uaw->etag->statusbar = uaw->progress;
	strcpy(uaw->etag->buf, uaw->buffer);
	
	/* Get the response */
	uaw->etag->e_tag = icq_daemon->icqFetchAutoResponse(user->Uin());

	/* Append it to gslist */
	catcher = g_slist_append(catcher, uaw->etag); 
}

void
finish_away(ICQEvent *event)
{
	struct user_away_window *uaw = uaw_find(event->Uin());

	/* If the window isn't open, don't bother */
	if (uaw == 0)
		return;

	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(uaw->text_box));
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(tb, &iter);
	gtk_text_buffer_insert(tb, &iter, uaw->user->AutoResponse(), -1);
}

