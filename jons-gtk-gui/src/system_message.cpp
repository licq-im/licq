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
	sm->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(sm->window), title);

	/* The box */
	v_box = gtk_vbox_new(FALSE, 5);

	/* The scrolling window */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);

	/* The text box to display messages */
	sm->text = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(sm->text), FALSE);
	gtk_text_set_word_wrap(GTK_TEXT(sm->text), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(sm->text), TRUE);
	gtk_widget_set_usize(sm->text, 300, 100);
	gtk_container_add(GTK_CONTAINER(scroll), sm->text);

	/* Dispaly what type of system message it is */
	switch(event->SubCommand())
	{
	case ICQ_CMDxSUB_AUTHxREQUEST:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
			        "Authorization Request\n-------------------\n\n", -1);
		menu_system_auth_user(NULL, ((CEventAuthRequest *)event)->Uin());
		break;
	
	case ICQ_CMDxSUB_AUTHxGRANTED:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"Authorization Granted\n--------------------\n\n", -1);
		break;

	case ICQ_CMDxSUB_AUTHxREFUSED:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"Authorization Refused\n--------------------\n\n", -1);
		break;

	case ICQ_CMDxSUB_ADDEDxTOxLIST:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"Added To Contact List\n-------------------\n\n", -1);
		break;

	case ICQ_CMDxSUB_WEBxPANEL:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"Web Panel\n--------\n\n", -1);
		break;

	case ICQ_CMDxSUB_EMAILxPAGER:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"E-mail Pager\n-----------\n\n", -1);
		break;

	case ICQ_CMDxSUB_CONTACTxLIST:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"Contact\n-------\n\n", -1);
		break;

	default:
		g_print("Unknown system message type\n");
	}

	/* Display the system message */
	const gchar *message = event->Text();
	gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0, message, -1);
  delete event;
	gUserManager.DropOwner();

	/* Pack it */
	gtk_box_pack_start(GTK_BOX(v_box), scroll, FALSE, FALSE, 0);

	/* The close button */
	button = gtk_button_new_with_label("Close");
	
	/* Pack it */
	gtk_box_pack_start(GTK_BOX(v_box), button, FALSE, FALSE, 0);

	/* Connect signals */
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(dialog_close), sm->window);
	gtk_signal_connect(GTK_OBJECT(sm->window), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), sm->window);

	/* Add the v_box to the window and show the widgets */
	gtk_container_add(GTK_CONTAINER(sm->window), v_box);
	gtk_widget_show_all(sm->window);
	gtk_window_set_position(GTK_WINDOW(sm->window), GTK_WIN_POS_CENTER);

	contact_list_refresh();
	system_status_refresh();
}
