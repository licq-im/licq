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

void system_message_window()
{
	GtkWidget *button;
	GtkWidget *scroll;
	GtkWidget *v_box;
	struct system_message *sm = g_new0(struct system_message, 1);
	const gchar *title = "Incoming System Message";
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	CUserEvent *event = owner->EventPop();

	/* Make the window */
	sm->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(sm->window), title);

	/* The box */
	v_box = gtk_vbox_new(FALSE, 5);

	/* The text box to display messages */
	sm->text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(sm->text), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(sm->text), GTK_WRAP_WORD);
	gtk_widget_set_size_request(sm->text, 300, 100);

	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(sm->text));
	/* Display what type of system message it is */
	switch(event->SubCommand())
	{
	case ICQ_CMDxSUB_AUTHxREQUEST:
		gtk_text_buffer_set_text(tb,
    					"Authorization Request\n-------------------\n\n", -1);
		menu_system_auth_user(0, ((CEventAuthRequest *)event)->Uin());
		break;
	
	case ICQ_CMDxSUB_AUTHxGRANTED:
		gtk_text_buffer_set_text(tb,
				"Authorization Granted\n--------------------\n\n", -1);
		break;

	case ICQ_CMDxSUB_AUTHxREFUSED:
		gtk_text_buffer_set_text(tb,
				"Authorization Refused\n--------------------\n\n", -1);
		break;

	case ICQ_CMDxSUB_ADDEDxTOxLIST:
		gtk_text_buffer_set_text(tb,
				"Added To Contact List\n-------------------\n\n", -1);
		break;

	case ICQ_CMDxSUB_WEBxPANEL:
		gtk_text_buffer_set_text(tb,
				"Web Panel\n--------\n\n", -1);
		break;

	case ICQ_CMDxSUB_EMAILxPAGER:
		gtk_text_buffer_set_text(tb,
				"E-mail Pager\n-----------\n\n", -1);
		break;

	default:
		g_print("Unknown system message type\n");
	}

	/* Display the system message */
	time_t time_recv = event->Time();
	const gchar *time = g_strdup_printf("Time: %s\n",
		ctime(&time_recv));
	const gchar *message = event->Text();
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(tb, &iter);
	gtk_text_buffer_insert(tb, &iter, time, -1);
	gtk_text_buffer_get_end_iter(tb, &iter);
	gtk_text_buffer_insert(tb, &iter, message, -1);

	delete event;
	
	gUserManager.DropOwner();

	/* Pack it */
	scroll = gtk_scrolled_window_new(0, 0);
	gtk_container_add(GTK_CONTAINER(scroll), sm->text);
	gtk_box_pack_start(GTK_BOX(v_box), scroll, FALSE, FALSE, 0);

	/* The close button */
	button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	
	/* Pack it */
	gtk_box_pack_start(GTK_BOX(v_box), button, FALSE, FALSE, 0);

	/* Connect signals */
	g_signal_connect(G_OBJECT(button), "clicked",
			   G_CALLBACK(dialog_close), sm->window);
	g_signal_connect(G_OBJECT(sm->window), "destroy",
			   G_CALLBACK(dialog_close), sm->window);
	
	/* Add the v_box to the window and show the widgets */
	gtk_container_add(GTK_CONTAINER(sm->window), v_box);
	gtk_widget_show_all(sm->window);
	gtk_window_set_position(GTK_WINDOW(sm->window), GTK_WIN_POS_CENTER);

	contact_list_refresh();
	system_status_refresh();
}
