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

#include "user.h"

#include <gtk/gtk.h>

void away_msg_window(gushort status)
{
	struct away_dialog *away_d = g_new0(struct away_dialog, 1);

	GtkWidget *ok;
	GtkWidget *cancel;
	GtkWidget *h_box;
	GtkWidget *v_box;
	const gchar *title =
		g_strdup_printf("Set %s Response",
		ICQUser::StatusToStatusStr(status, FALSE));

	/* Make the main window */
	away_d->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(away_d->window), title);
	gtk_window_set_position(GTK_WINDOW(away_d->window), GTK_WIN_POS_CENTER);

	/* The text box */
	away_d->text = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(away_d->text), TRUE);
	gtk_widget_set_usize(GTK_WIDGET(away_d->text), 300, 100);

	/* The boxes */
	h_box = gtk_hbox_new(TRUE, 5);
	v_box = gtk_vbox_new(FALSE, 5);

	/* Pack the text box into the v_box */
	gtk_box_pack_start(GTK_BOX(v_box), away_d->text, FALSE, FALSE, 0);

	/* Make the buttons now */
	ok = gtk_button_new_with_label("OK");
	cancel = gtk_button_new_with_label("Cancel");

	/* Pack the buttons */
	gtk_box_pack_start(GTK_BOX(h_box), ok, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), cancel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

	/* Connect the signals now */
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			   GTK_SIGNAL_FUNC(away_close), away_d->window);
	gtk_signal_connect(GTK_OBJECT(away_d->window), "destroy",
			   GTK_SIGNAL_FUNC(away_close), away_d->window);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(set_away_msg), away_d);

	/* Get the window ready to be shown, show it, and set is_shown */
	gtk_container_add(GTK_CONTAINER(away_d->window), v_box);
	gtk_widget_show_all(away_d->window);
	gtk_window_set_focus(GTK_WINDOW(away_d->window), away_d->text);
	gtk_grab_add(away_d->window);
}

void set_away_msg(GtkWidget *widget, struct away_dialog *away_d)
{
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_W);
	owner->SetAutoResponse(gtk_editable_get_chars(GTK_EDITABLE(away_d->text), 0, -1));
	gUserManager.DropOwner();

	gtk_grab_remove(away_d->window);
	gtk_widget_destroy(away_d->window);
}

void away_close(GtkWidget *widget, GtkWidget *window)
{
	gtk_grab_remove(window);
	gtk_widget_destroy(window);
}

void list_read_message(GtkWidget *widget, ICQUser *user)
{
	/* Keep track of this event.. we need this */
	struct main_progress *m_prog = g_new0(main_progress, 1);

	GtkWidget *h_box;
	GtkWidget *v_box;
	GtkWidget *scroll;
	GtkWidget *text_box;
	GtkWidget *close;
	struct user_away_window *uaw = g_new0(struct user_away_window, 1);
	const gchar *title = g_strdup_printf("Auto Response for %s", user->GetAlias());

	uaw->user = user;

	/* Make the window */
	uaw->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(uaw->window), title);
	gtk_window_set_position(GTK_WINDOW(uaw->window), GTK_WIN_POS_CENTER);

	/* Make the boxes */
	h_box = gtk_hbox_new(FALSE, 5);
	v_box = gtk_vbox_new(FALSE, 5);

	/* The scrolling window */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);

	/* The text box */
	text_box = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(text_box), FALSE);
	gtk_text_set_word_wrap(GTK_TEXT(text_box), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(text_box), TRUE);

	/* Add the text box to the scrolling window */
	gtk_container_add(GTK_CONTAINER(scroll), text_box);

	/* Pack the scrolled window into the v_box */
	gtk_box_pack_start(GTK_BOX(v_box), scroll, FALSE, FALSE, 5);

	/* The Show Again check button */
	uaw->show_again = gtk_check_button_new_with_label("Show Again");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(uaw->show_again),
				     user->ShowAwayMsg());

	/* The close button */
	close = gtk_button_new_with_label("Close");
	gtk_signal_connect(GTK_OBJECT(close), "clicked",
			   GTK_SIGNAL_FUNC(close_away_window), uaw);

	/* Pack everything */
	gtk_box_pack_start(GTK_BOX(h_box), uaw->show_again, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(h_box), close, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	gtk_container_add(GTK_CONTAINER(uaw->window), v_box);
	gtk_widget_show_all(uaw->window);

	/* Get the response... put it in the main progress bar */
	m_prog->e_tag = icq_daemon->icqFetchAutoResponse(user->Uin());
	
	gchar *temp = g_strdup_printf("A/R for %s .. ", user->GetAlias());
	strcpy(m_prog->buffer, temp);

	/* Add it to the GList */
	m_prog_list = g_list_append(m_prog_list, m_prog);

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_progress),
						"main_prog");
	gtk_statusbar_pop(GTK_STATUSBAR(status_progress), id);
	gtk_statusbar_push(GTK_STATUSBAR(status_progress), id, m_prog->buffer);

	if(strcmp("", user->AutoResponse()))
	{
		gtk_text_freeze(GTK_TEXT(text_box));
		gtk_text_insert(GTK_TEXT(text_box), 0, 0, 0,
				user->AutoResponse(), -1);
		gtk_text_thaw(GTK_TEXT(text_box));
	}	
}

void close_away_window(GtkWidget *widget, struct user_away_window *uaw)
{
	uaw->user->SetShowAwayMsg(gtk_toggle_button_get_active(
					GTK_TOGGLE_BUTTON(uaw->show_again)));
	dialog_close(NULL, uaw->window);
}
