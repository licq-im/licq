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

#include <gtk/gtk.h>

void list_more_window(GtkWidget *widget, ICQUser *u)
{
	GtkWidget *fix_radios;
	GtkWidget *h_box;
	GtkWidget *notebook;
	GtkWidget *v_box;
	GtkWidget *general_box;
	GtkWidget *status_box;
	GtkWidget *accept_box;
	GtkWidget *custom_box;
	GtkWidget *label;
	GtkWidget *ok;
	GtkWidget *cancel;

	const gchar *title = g_strdup_printf("More Options for %s", u->GetAlias());
	struct more_window *mw = g_new0(struct more_window, 1);

	/* Set the user in the struct */
	mw->user = u;

	/* Make the window */
	mw->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(mw->window), title);
	gtk_window_set_position(GTK_WINDOW(mw->window), GTK_WIN_POS_CENTER);

	/* Make the boxes */
	h_box = gtk_hbox_new(FALSE, 5);	
	v_box = gtk_vbox_new(FALSE, 5);
	general_box = gtk_vbox_new(FALSE, 5);
	status_box = gtk_vbox_new(FALSE, 5);
	accept_box = gtk_vbox_new(FALSE, 5);
	custom_box = gtk_vbox_new(FALSE, 5);

	/* The notebook */
	notebook = gtk_notebook_new();

	/* THE GENERAL TAB */

	/* The online notify */
	mw->notify = gtk_check_button_new_with_label("Online Notify");

	/* Set the state of the online notify button */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->notify),
				     mw->user->OnlineNotify());

	// Auto accept chat and file and auto secure
	mw->autochat = gtk_check_button_new_with_label("Auto Accept Chat");
	mw->autofile = gtk_check_button_new_with_label("Auto Accept File");
	mw->autosecure = gtk_check_button_new_with_label("Auto Request Secure");

	// Set the state of the auto accept buttons
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->autochat),
		mw->user->AutoChatAccept());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->autofile),
		mw->user->AutoFileAccept());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->autosecure),
		mw->user->AutoSecure());

	mw->realip = gtk_check_button_new_with_label("Use Real IP (LAN)");

	/* Set the state of the use real ip button */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->realip),
				     mw->user->SendRealIp());

	/* The ignore list */
	mw->ignore = gtk_check_button_new_with_label("Ignore List");

	/* Set the state of the ignore button */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->ignore),
				     mw->user->IgnoreList());

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(general_box), mw->notify, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(general_box), mw->autochat, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(general_box), mw->autofile, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(general_box), mw->autosecure, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(general_box), mw->realip, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(general_box), mw->ignore, FALSE, FALSE, 5);

	/* New h_box */
	h_box = gtk_hbox_new(FALSE, 5);
	
	fix_radios = gtk_radio_button_new_with_label(NULL, "None");

	/* The visible list */
	mw->visible = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(fix_radios), "Visible List");

	/* Set the state of the visible button */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->visible),
				     mw->user->VisibleList());

	/* The invisible list */
	mw->invisible = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(fix_radios), "Invisible List");

	/* Set the state of the invisible button */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->invisible),
				     mw->user->InvisibleList());

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(h_box), fix_radios, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), mw->visible, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), mw->invisible, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(general_box), h_box, FALSE, FALSE, 20);

	/* THE ACCEPT IN ... TAB */
	
	/* Accept in Away and set the button */
	mw->accept_away = gtk_check_button_new_with_label("Accept in Away");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->accept_away),
				     mw->user->AcceptInAway());

	/* Accept in NA and set the button */
	mw->accept_na = gtk_check_button_new_with_label("Accept in N/A");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->accept_na),
				     mw->user->AcceptInNA());

	/* Accept in Occupied and set the button */
	mw->accept_occ = gtk_check_button_new_with_label("Accept in Occupied");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->accept_occ),
				     mw->user->AcceptInOccupied());

	/* Accept in DND and set the button */
	mw->accept_dnd = gtk_check_button_new_with_label("Accept in Do Not Disturb");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->accept_dnd),
				     mw->user->AcceptInDND());

	/* Pack them */
	gtk_box_pack_start(GTK_BOX(accept_box),
			   mw->accept_away, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(accept_box),
			   mw->accept_na, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(accept_box),
			   mw->accept_occ, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(accept_box),
			   mw->accept_dnd, FALSE, FALSE, 5);

	/* THE STATUS TAB */

	fix_radios = gtk_radio_button_new_with_label(NULL, "None");

	/* Appear online to user */
	mw->online_to = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(fix_radios), "Online to User");

	/* Appear away to user */
	mw->away_to = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(fix_radios), "Away to User");

	/* Appear n/a to user */
	mw->na_to = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(fix_radios), "N/A to User");

	/* Appear occupied to user */
	mw->occ_to = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(fix_radios), "Occupied to User");

	/* Appear dnd to user */
	mw->dnd_to = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(fix_radios), "Do Not Disturb to User");

	switch(mw->user->StatusToUser())
	{
		case ICQ_STATUS_ONLINE:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->online_to), TRUE);
			break;
		case ICQ_STATUS_AWAY:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->away_to), TRUE);
			break;
		case ICQ_STATUS_NA:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->na_to), TRUE);
			break;
		case ICQ_STATUS_OCCUPIED:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->occ_to), TRUE);
			break;
		case ICQ_STATUS_DND:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->dnd_to), TRUE);
			break;
		default:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fix_radios), TRUE);
	}
	
	/* Pack them */
	gtk_box_pack_start(GTK_BOX(status_box), fix_radios, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(status_box), mw->online_to, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(status_box), mw->away_to, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(status_box), mw->na_to, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(status_box), mw->occ_to, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(status_box), mw->dnd_to, FALSE, FALSE, 5);

	/* THE CUSTOM RESPONSE TAB */

	mw->custom_check =
		gtk_check_button_new_with_label("Use Custom Auto Response");
	if(strcmp(mw->user->CustomAutoResponse(), ""))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->custom_check),
					    TRUE);

	mw->custom_text = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(mw->custom_text), TRUE);
	gtk_text_freeze(GTK_TEXT(mw->custom_text));
	gtk_text_insert(GTK_TEXT(mw->custom_text), 0, 0, 0,
			mw->user->CustomAutoResponse(), -1);
	gtk_text_thaw(GTK_TEXT(mw->custom_text));

	/* Pack them in */
	gtk_box_pack_start(GTK_BOX(custom_box), mw->custom_check, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(custom_box), mw->custom_text, FALSE, FALSE, 5);
	
	/* Add everything to the notebook */
	label = gtk_label_new("General");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), general_box, label);
	label = gtk_label_new("Accept In ...");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), accept_box, label);
	label = gtk_label_new("Status to User");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), status_box, label);
	label = gtk_label_new("Custom Response");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), custom_box, label);

	/* The buttons */
	ok = gtk_button_new_with_label("OK");
	cancel = gtk_button_new_with_label("Cancel");

	/* Add them to the h_box */
	h_box = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), cancel, TRUE, TRUE, 5);

	/* Connect the signals of the buttons */
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			   GTK_SIGNAL_FUNC(dialog_close), mw->window);
	gtk_signal_connect(GTK_OBJECT(mw->window), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), mw->window);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(more_ok_callback), mw);

	/* Add the notebook and the buttons */
	gtk_box_pack_start(GTK_BOX(v_box), notebook, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 5);

	/* Add the main box to the window */
	gtk_container_add(GTK_CONTAINER(mw->window), v_box);

	/* Show the widgets */
	gtk_widget_show_all(mw->window);
}

void more_ok_callback(GtkWidget *widget, struct more_window *mw)
{
	mw->user->SetOnlineNotify(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->notify)));

	mw->user->SetAutoChatAccept(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->autochat)));

	mw->user->SetAutoFileAccept(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->autofile)));

	mw->user->SetAutoSecure(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->autosecure)));
	mw->user->SetSendRealIp(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->realip)));

	mw->user->SetIgnoreList(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->ignore)));

	mw->user->SetVisibleList(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->visible)));

	mw->user->SetInvisibleList(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->invisible)));

	mw->user->SetAcceptInAway(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->accept_away)));

	mw->user->SetAcceptInNA(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->accept_na)));

	mw->user->SetAcceptInOccupied(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->accept_occ)));

	mw->user->SetAcceptInDND(
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->accept_dnd)));

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->online_to)))
		mw->user->SetStatusToUser(ICQ_STATUS_ONLINE);
	
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->away_to)))
		mw->user->SetStatusToUser(ICQ_STATUS_AWAY);

	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->na_to)))
		mw->user->SetStatusToUser(ICQ_STATUS_NA);

	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->occ_to)))
		mw->user->SetStatusToUser(ICQ_STATUS_OCCUPIED);

	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->dnd_to)))
		mw->user->SetStatusToUser(ICQ_STATUS_DND);

	else
		mw->user->SetStatusToUser(ICQ_STATUS_OFFLINE);

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->custom_check)))
		mw->user->SetCustomAutoResponse(
			gtk_editable_get_chars(GTK_EDITABLE(mw->custom_text),
						0, -1));
	else
		mw->user->SetCustomAutoResponse(NULL);
	
	dialog_close(NULL, mw->window);
}
