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
	GtkWidget *v_box;
	struct system_message *sm = g_new0(struct system_message, 1);
	const gchar *title = "Incoming System Message";
	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
	CUserEvent *event = owner->GetEvent(0);

	/* Make the window */
	sm->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(sm->window), title);

	/* The box */
	v_box = gtk_vbox_new(FALSE, 5);

	/* The text box to display messages */
	sm->text = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(sm->text), FALSE);
	gtk_widget_set_usize(sm->text, 300, 100);

	/* Dispaly what type of system message it is */
	switch(event->SubCommand())
	{
	case ICQ_CMDxSUB_REQxAUTH:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
			        "Authorization Request\n", -1);
		break;
	
	case ICQ_CMDxSUB_AUTHORIZED:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"Authorization Granted\n", -1);
		break;

	case ICQ_CMDxSUB_ADDEDxTOxLIST:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"Added To Contact List\n", -1);
		break;

	case ICQ_CMDxSUB_WEBxPANEL:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"Web Panel\n", -1);
		break;

	case ICQ_CMDxSUB_EMAILxPAGER:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"E-mail Pager\n", -1);
		break;

	case ICQ_CMDxSUB_CONTACTxLIST:
		gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0,
				"Contact\n", -1);
		break;

	default:
		g_print("Unknown system message type\n");
	}

	/* Display the system message */
	const gchar *message = event->Text();
	gtk_text_insert(GTK_TEXT(sm->text), 0, 0, 0, message, -1);
	owner->ClearEvent(0);
	gUserManager.DropOwner();

	/* Pack it */
	gtk_box_pack_start(GTK_BOX(v_box), sm->text, FALSE, FALSE, 0);

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
}
