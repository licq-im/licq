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

#include "icqd.h"
#include "user.h"

#include <gtk/gtk.h>

void list_start_convo(GtkWidget *widget, ICQUser *user)
{
	struct conversation *c = NULL;

	c = convo_find(user->Uin());

	if(c != NULL)
		gdk_window_raise(c->window->window);

	else
	{
		if(user->NewMessages() > 0)
			c = convo_new(user, TRUE);

		else
			c = convo_new(user, FALSE);
	}
}

void list_send_url(GtkWidget *widget, ICQUser *user)
{
	GtkWidget *label;
	GtkWidget *send;
	GtkWidget *close;
	GtkWidget *h_box;
	GtkWidget *v_box;

	struct send_url *url = g_new0(struct send_url, 1);

	url->user = user;

	/* Work on the title */
	const gchar *title = g_strdup_printf("URL to %s", user->GetAlias());

	/* Set up the window */
	url->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_container_border_width(GTK_CONTAINER(url->window), 10);
	gtk_window_set_title(GTK_WINDOW(url->window), title);
	gtk_signal_connect(GTK_OBJECT(url->window), "delete_event",
			   GTK_SIGNAL_FUNC(destroy_dialog), url->window);

	/* Set up the boxes */
	h_box = gtk_hbox_new(FALSE, 0);
	v_box = gtk_vbox_new(FALSE, 5);

	/* Make the entries */
	url->entry_u = gtk_entry_new();
	url->entry_d = gtk_entry_new();

	label = gtk_label_new("         URL: ");
	gtk_box_pack_start(GTK_BOX(h_box), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), url->entry_u, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 0);

	h_box = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("Description: ");
	gtk_box_pack_start(GTK_BOX(h_box), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), url->entry_d, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 0);

	h_box = gtk_hbox_new(FALSE, 0);

	/* The send through server and spoof area */
	url->send_server = gtk_check_button_new_with_label("Server");
	url->spoof_button = gtk_check_button_new_with_label("Spoof UIN");
	url->spoof_uin = gtk_entry_new_with_max_length(8);

	gtk_widget_set_usize(url->spoof_uin, 80, 20);

	/* Verify that the url->spoof_uin is digits only */
	gtk_signal_connect(GTK_OBJECT(url->spoof_uin), "insert-text", 
			   GTK_SIGNAL_FUNC(verify_numbers), url);

	/* No UIN can be entered unless url->spoof_button is checked */
	gtk_entry_set_editable(GTK_ENTRY(url->spoof_uin), FALSE);

	/* Connect signal */
	gtk_signal_connect(GTK_OBJECT(url->spoof_button), "toggled",
			   GTK_SIGNAL_FUNC(url_spoof_button_callback), url);

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(h_box), url->send_server, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), url->spoof_button, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), url->spoof_uin, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 0);

	h_box = gtk_hbox_new(FALSE, 0);

	/* Make the buttons */
	close = gtk_button_new_with_label("Close");
	gtk_signal_connect(GTK_OBJECT(close), "clicked",
			   GTK_SIGNAL_FUNC(url_close), url);
	send = gtk_button_new_with_label("Send");
	gtk_signal_connect(GTK_OBJECT(send), "clicked",
			   GTK_SIGNAL_FUNC(url_send), url);

	gtk_box_pack_start(GTK_BOX(h_box), close, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), send, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 0);

	/* All the h_boxes are in v_box.. so add v_box to add them all */
	gtk_container_add(GTK_CONTAINER(url->window), v_box);

	/* Show all the widgets at once */
	gtk_widget_show_all(url->window);

	gtk_widget_grab_focus(url->entry_u);
}

void url_send(GtkWidget *widget, struct send_url *url)
{
	struct main_progress *m_prog = g_new0(main_progress, 1);

	const char *url_to_send = gtk_entry_get_text(GTK_ENTRY(url->entry_u));
	const char *desc = gtk_entry_get_text(GTK_ENTRY(url->entry_d));
	gulong uin = 0;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(url->spoof_button)))
	{
		uin = atol((const char *)gtk_editable_get_chars(GTK_EDITABLE(url->spoof_uin), 0, -1));
	}

	m_prog->e_tag =
		icq_daemon->icqSendUrl(url->user->Uin(), url_to_send, desc,
       	  (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(url->send_server))),
	  FALSE, uin);

	gchar *temp = g_strdup_printf("URL -> %s ", url->user->GetAlias());
	
	strcpy(m_prog->buffer, temp);
	
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(url->send_server)))
		strcat(m_prog->buffer, "(direct) .. ");
	else
		strcat(m_prog->buffer, "(server) .. ");

	/* Add it to the GList */
	m_prog_list = g_list_append(m_prog_list, m_prog);

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_progress),
						"main_prog");
	gtk_statusbar_pop(GTK_STATUSBAR(status_progress), id);
	gtk_statusbar_push(GTK_STATUSBAR(status_progress), id, m_prog->buffer);
}

void url_spoof_button_callback(GtkWidget *widget, struct send_url *url)
{
	gtk_editable_set_editable(GTK_EDITABLE(url->spoof_uin),
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(url->spoof_button)));
}

void url_close(GtkWidget *widget, struct send_url *url)
{
	gtk_widget_destroy(url->window);
}

void list_delete_user(GtkWidget *widget, ICQUser *user)
{
	GtkWidget *yes;
	GtkWidget *no;
	GtkWidget *label;
	GtkWidget *h_box;
	GtkWidget *v_box;
	struct delete_user *d = g_new0(struct delete_user, 1);

	d->user = user;

	const gchar *text = g_strdup_printf("Are you sure you want to delete\n%s (UIN: %ld)\nfrom your list?", d->user->GetAlias(), d->user->Uin());

	/* Make the dialog window */
	d->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(d->window), "Licq - Delete User");

	/* The main box */
	v_box = gtk_vbox_new(FALSE, 5);

	/* The first box */
	h_box = gtk_hbox_new(FALSE, 5);
	label = gtk_label_new(text);
	gtk_box_pack_start(GTK_BOX(h_box), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 5);

	/* The second box, with buttons */
	h_box = gtk_hbox_new(FALSE, 10);
	yes = gtk_button_new_with_label("Yes");
	gtk_box_pack_start(GTK_BOX(h_box), yes, TRUE, TRUE, 0);
	no = gtk_button_new_with_label("No");
	gtk_box_pack_start(GTK_BOX(h_box), no, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 5);

	/* Connect the signals */
	gtk_signal_connect(GTK_OBJECT(no), "clicked",
			   GTK_SIGNAL_FUNC(dialog_close), d->window);
	gtk_signal_connect(GTK_OBJECT(d->window), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), d->window);
	gtk_signal_connect(GTK_OBJECT(yes), "clicked",
			   GTK_SIGNAL_FUNC(delete_user_callback), d);

	/* Show the widgets */
	gtk_container_add(GTK_CONTAINER(d->window), v_box);
	gtk_widget_show_all(d->window);
}

void delete_user_callback(GtkWidget *widget, struct delete_user *d)
{
	icq_daemon->RemoveUserFromList(d->user->Uin());
	gtk_widget_destroy(d->window);
}

void destroy_dialog(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(widget);
}
