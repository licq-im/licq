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

struct key_request
{
	GtkWidget *window;
	GtkWidget *label_status;
	gboolean open;

	ICQUser *user;
	struct e_tag_data *etag;
};

// Global list
GSList *kr_list;

gboolean send_key_request(gpointer);
gboolean key_request_close_window(gpointer);
void close_key_request(GtkWidget *, gpointer);
struct key_request *kr_find(gulong uin);
struct key_request *kr_new(ICQUser *u);

void create_key_request_window(GtkWidget *widget, ICQUser *user)
{
	// Do we need to be here?
	struct key_request *kr = kr_find(user->Uin());

	// Nope
	if(kr != 0)
		return;

	// Yep, make a new one
	kr = kr_new(user);

	// Make the window
	kr->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(kr->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(kr->window), g_strdup_printf(
		"Licq - Secure Channel with %s", user->GetAlias()));
	g_signal_connect(G_OBJECT(kr->window), "destroy",
		G_CALLBACK(close_key_request), (gpointer)kr);

	// We'll need a table for our stuff
	GtkWidget *table = gtk_table_new(2, 4, false);
	gtk_container_add(GTK_CONTAINER(kr->window), table);

	// An informational label
	const char info[] = "Secure channel is established using SSL\n"
			    "with Diffie-Hellman key exchange and\n"
		            "the TLS version 1 protocol.";
	GtkWidget *label = gtk_label_new(info);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1,
		GTK_EXPAND, GTK_EXPAND, 6, 6);

	char secure_info[128];

	switch(user->SecureChannelSupport())
	{
		case SECURE_CHANNEL_SUPPORTED:
			strncpy(secure_info, g_strdup_printf(
				"The remote uses Licq %s/SSL.",
				CUserEvent::LicqVersionToString(user->LicqVersion())), 128);
			break;

		case SECURE_CHANNEL_NOTSUPPORTED:
			strncpy(secure_info, g_strdup_printf(
				"The remote uses Licq %s, however it\n"
				"has no secure channel support compiled in.\n"
				"This probably won't work.",
					CUserEvent::LicqVersionToString(user->LicqVersion())), 128);
			break;

		default:
			strncpy(secure_info, "This only works with other Licq "
				"clients >= v0.85\nThe remote doesn't seem to "
				"use such a client.\nThis might not work.",
				128);
			break;
	}

	GtkWidget *secure_label = gtk_label_new(secure_info);
	gtk_table_attach(GTK_TABLE(table), secure_label, 0, 1, 1, 2,
		GTK_EXPAND, GTK_EXPAND, 0, 0);

	// The status label
	kr->label_status = gtk_label_new("");
	gtk_table_attach(GTK_TABLE(table), kr->label_status, 0, 1, 2, 3,
		GTK_EXPAND, GTK_EXPAND, 6, 6);

	// A box with the 2 buttons
	GtkWidget *h_box = gtk_hbox_new(true, 5);
	GtkWidget *send = gtk_button_new_with_mnemonic("_Send");
	GtkWidget *close = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_box_pack_start(GTK_BOX(h_box), send, true, true, 5);
	gtk_box_pack_start(GTK_BOX(h_box), close, true, true, 5);
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 3, 4,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL ),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL ),
		3, 3);

	// Button signals
	g_signal_connect(G_OBJECT(send), "clicked",
		G_CALLBACK(send_key_request), (gpointer)kr);
	g_signal_connect(G_OBJECT(close), "clicked",
		G_CALLBACK(close_key_request), (gpointer)kr);
	
	// Take care of the status label
	if(icq_daemon->CryptoEnabled())
	{
		kr->open = !user->Secure();
		if(user->Secure())
			gtk_label_set_text(GTK_LABEL(kr->label_status),
				"Ready to close channel");
		else
			gtk_label_set_text(GTK_LABEL(kr->label_status),
				"Ready to request channel");
		gtk_timeout_add_full(100, send_key_request, NULL, kr, NULL);
	}

	else
	{
		gtk_label_set_text(GTK_LABEL(kr->label_status),
			"Your client does not support OpenSSL.\n"
			"Rebuild Licq with OpenSSL support.");
		gtk_widget_set_sensitive(send, false);
	}

	gtk_widget_show_all(kr->window);
}

struct key_request *kr_find(gulong uin)
{
	struct key_request *kr;
	GSList *temp_kr_list = kr_list;

	while(temp_kr_list)
	{
		kr = (struct key_request *)temp_kr_list->data;
		if(kr->user->Uin() == uin)
			return kr;

		temp_kr_list = temp_kr_list->next;
	}

	// It wasn't found
	return 0;
}

struct key_request *kr_new(ICQUser *u)
{
	struct key_request *kr = g_new0(struct key_request, 1);
	kr->etag = g_new0(struct e_tag_data, 1);
	kr->user = u;
	kr_list = g_slist_append(kr_list, kr);

	return kr;
}

gboolean send_key_request(gpointer _kr)
{
	struct key_request *kr = (struct key_request *)_kr;

	if(kr->open)
	{
		gtk_label_set_text(GTK_LABEL(kr->label_status),
			"Requesting secure channel ... ");
		kr->etag->e_tag = icq_daemon->icqOpenSecureChannel(
			kr->user->Uin());
	}

	else
	{
		gtk_label_set_text(GTK_LABEL(kr->label_status),
			"Closing secure channel ... ");
		kr->etag->e_tag = icq_daemon->icqCloseSecureChannel(
			kr->user->Uin());
	}

	catcher = g_slist_append(catcher, kr->etag);
	return FALSE;
}

gboolean key_request_close_window(gpointer _kr)
{
	struct key_request *kr = (struct key_request *)_kr;
	kr_list = g_slist_remove(kr_list, kr);
	gtk_widget_destroy(kr->window);
	return FALSE;
}

void close_key_request(GtkWidget *widget, gpointer _kr)
{
	struct key_request *kr = (struct key_request *)_kr;
	kr_list = g_slist_remove(kr_list, kr);
	gtk_widget_destroy(kr->window);
}

void finish_secure(ICQEvent *event)
{
	struct key_request *kr = kr_find(event->Uin());

	// Window isn't open.. cya
	if(kr == 0)
		return;

	char result[41];
	
	switch(event->Result())
	{
		case EVENT_FAILED:
			strncpy(result,
				"Remote client does not support OpenSSL.", 41);
			break;

		case EVENT_ERROR:
			strncpy(result, "Could not connect to remote client.",
				41);
			break;

		case EVENT_SUCCESS:
			if(kr->open)
				strncpy(result, "Secure channel established.",
					41);
			else
				strncpy(result, "Secure channel closed.", 41);
			break;
		case EVENT_ACKED:
		case EVENT_TIMEDOUT:
		case EVENT_CANCELLED:
		default:
			break;
	}

	gtk_label_set_text(GTK_LABEL(kr->label_status), result);
	if (event->Result() == EVENT_SUCCESS)
		gtk_timeout_add_full(500, key_request_close_window, NULL, kr, NULL);
}

