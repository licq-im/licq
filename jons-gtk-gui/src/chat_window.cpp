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
#include <gdk/gdkkeysyms.h>
#include <list.h>

#include "licq_icq.h"
#include "licq_gtk.h"

GSList *rc_list;

typedef list<chat_window *> ChatDlgList;
ChatDlgList chat_list;

void list_request_chat(GtkWidget *widget, ICQUser *user)
{
	/* Do we even want to be here? */
	struct request_chat *rc = rc_find(user->Uin());

	// No get outta here, bitch!
	if(rc != 0)
		return;

	rc = rc_new(user);

	GtkWidget *scroll;
	GtkWidget *statusbar;
	GtkWidget *multiparty;
	GtkWidget *single;
	GtkWidget *ok;
	GtkWidget *cancel;
	GtkWidget *table;
	GtkWidget *h_box;
	gchar *title = g_strdup_printf("Licq - Request Chat With %s",
					     user->GetAlias());

	/* Make the request_chat structure */
	rc->etd = g_new0(struct e_tag_data, 1);

	rc->user = user;

	/* Create the table and hbox */
	table = gtk_table_new(4, 2, FALSE);
	h_box = gtk_hbox_new(FALSE, 5);

	/* Create the window */
	rc->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(rc->window), title);
	gtk_window_set_position(GTK_WINDOW(rc->window), GTK_WIN_POS_CENTER);
	gtk_container_add(GTK_CONTAINER(rc->window), table);
	gtk_signal_connect(GTK_OBJECT(rc->window), "destroy",
		GTK_SIGNAL_FUNC(cancel_request_chat), (gpointer)rc);

	/*Create the scrolled window with text and attach it to the window */
	scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);
	rc->text_box = gtk_text_new(0, 0);
	gtk_text_set_editable(GTK_TEXT(rc->text_box), TRUE);
	gtk_text_set_word_wrap(GTK_TEXT(rc->text_box), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(rc->text_box), TRUE);
	gtk_container_add(GTK_CONTAINER(scroll), rc->text_box);
	gtk_table_attach(GTK_TABLE(table), scroll, 0, 2, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 3, 3);
	gtk_widget_show(scroll);

	/* The send as buttons */
	rc->send_norm = gtk_radio_button_new_with_label(0, "Send Normal");
	rc->send_urg = gtk_radio_button_new_with_label_from_widget(
			GTK_RADIO_BUTTON(rc->send_norm), "Send Urgent");
	rc->send_list = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(rc->send_norm), "Send To List");

	/* Pack them to a box */
	gtk_box_pack_start(GTK_BOX(h_box), rc->send_norm, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), rc->send_urg, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), rc->send_list, FALSE, FALSE, 0);

	/* Attach the box to the table */
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 1, 2,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);
	
	/* Progress bar */
	statusbar = gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table), statusbar, 0, 2, 2, 3,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	/* e_tag stuff */
	rc->etd->statusbar = statusbar;
	strcpy(rc->etd->buf, "");

	// Ok.. this is where the combo box for multi party chats goes
	// but it isn't shown until it's needed
	h_box = gtk_hbox_new(FALSE, 5);
	GtkWidget *label = gtk_label_new("Current Chats:");
	rc->chat_list = gtk_combo_new();
	gtk_box_pack_start(GTK_BOX(h_box), label, false, false, 0);
	gtk_box_pack_start(GTK_BOX(h_box), rc->chat_list, false, false, 0);
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 3, 4,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	/* The button box */
	h_box = gtk_hbox_new(TRUE, 5);
	multiparty = gtk_button_new_with_label("Multi-Party");
	single = gtk_button_new_with_label("Single-Party");
	ok = gtk_button_new_with_label("Invite");
	cancel = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(h_box), multiparty, true, true, 0);
	gtk_box_pack_start(GTK_BOX(h_box), single, true, true, 0);
	gtk_box_pack_start(GTK_BOX(h_box), ok, true, true, 0);
	gtk_box_pack_start(GTK_BOX(h_box), cancel, true, true, 0);
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 4, 5,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);
	
	/* Connect the signals */
	gtk_signal_connect(GTK_OBJECT(multiparty), "clicked",
			   GTK_SIGNAL_FUNC(multi_request_chat), (gpointer)rc);
	gtk_signal_connect(GTK_OBJECT(single), "clicked",
			   GTK_SIGNAL_FUNC(single_request_chat), (gpointer)rc);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(ok_request_chat), (gpointer)rc);
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			   GTK_SIGNAL_FUNC(cancel_request_chat), (gpointer)rc);

	// Set chat list sensitive till needed and show everything
	gtk_widget_set_sensitive(rc->chat_list, false);
	gtk_widget_show_all(rc->window);
	
	g_free(title);
}

struct request_chat *rc_new(ICQUser *user)
{
	struct request_chat *rc;

	/* Does it already exist? */
	rc = rc_find(user->Uin());

	/* Don't make it if it's already there, return it */
	if(rc != 0)
		return rc;

	rc = g_new0(struct request_chat, 1);
	rc->user = user;
	rc_list = g_slist_append(rc_list, rc);

	return rc;
}

struct request_chat *rc_find(gulong uin)
{
	struct request_chat *rc;
	GSList *temp_rc_list = rc_list;

	while(temp_rc_list)
	{
		rc = (struct request_chat *)temp_rc_list->data;
		if(rc->user->Uin() == uin)
			return rc;

		temp_rc_list = temp_rc_list->next;
	}

	/* It wasn't found, return null */
	return 0;
}

void multi_request_chat(GtkWidget *widget, gpointer _rc)
{
	struct request_chat *rc = (struct request_chat *)_rc;

	// Save some time by checking to see if we should be here
	if(GTK_WIDGET_SENSITIVE(rc->chat_list))
		return;

	// Make a GList with the current chats in it
	GList *items = 0;

	ChatDlgList::iterator iter;
	for(iter = chat_list.begin(); iter != chat_list.end(); iter++)
		items = g_list_append(items, (*iter)->chatman->ClientsStr());

	// Make sure there are chats to show
	if(items == 0)
		return;
		
	gtk_combo_set_popdown_strings(GTK_COMBO(rc->chat_list), items);

	// Allright show it
	gtk_widget_set_sensitive(rc->chat_list, true);
}

void single_request_chat(GtkWidget *widget, gpointer _rc)
{
	struct request_chat *rc = (struct request_chat *)_rc;

	// Just take care of it.. I think it'll take some more time
	// to check if we should be here or not.
	gtk_widget_set_sensitive(rc->chat_list, false);
}

void ok_request_chat(GtkWidget *widget, gpointer _rc)
{
	struct request_chat *rc = (struct request_chat *)_rc;
	guint id;
	guint send_as = ICQ_TCPxMSG_NORMAL;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rc->send_urg)))
		send_as = ICQ_TCPxMSG_URGENT;
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rc->send_list)))
		send_as = ICQ_TCPxMSG_LIST;
	
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(rc->etd->statusbar),
					  "sta");
	gtk_statusbar_pop(GTK_STATUSBAR(rc->etd->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(rc->etd->statusbar), id,
			   "Requesting Chat ... ");

	strcpy(rc->etd->buf, "");
	strcpy(rc->etd->buf, "Requesting Chat ... ");

	// Are we doing single or multi?
	if(GTK_WIDGET_SENSITIVE(rc->chat_list))
	{
		// Multi
		unsigned short nPort = 0;

		// Find the chat
		ChatDlgList::iterator iter;
		for(iter = chat_list.begin(); iter != chat_list.end(); iter++)
		{
			if(strcmp((*iter)->chatman->ClientsStr(),
				  gtk_entry_get_text(GTK_ENTRY(
				  	GTK_COMBO(rc->chat_list)->entry)) )
				== 0)
			{
				nPort = (*iter)->chatman->LocalPort();
				break;
			}
		}

		if(iter == chat_list.end())
		{
			gtk_statusbar_pop(GTK_STATUSBAR(rc->etd->statusbar),
				id);
			gtk_statusbar_push(GTK_STATUSBAR(rc->etd->statusbar),
				id, "Requesting Chat ... Invalid Chat");
			message_box("Invalid Multi-Party Chat");
			return;
		}

		rc->etd->e_tag = icq_daemon->icqMultiPartyChatRequest(
			rc->user->Uin(), 
			gtk_editable_get_chars(GTK_EDITABLE(rc->text_box),
				0, -1),
			gtk_entry_get_text(GTK_ENTRY(
				GTK_COMBO(rc->chat_list)->entry)),
			nPort,
			send_as, false);
	}
	
	else
	{
		// Single
		rc->etd->e_tag = icq_daemon->icqChatRequest(rc->user->Uin(),
			gtk_editable_get_chars(GTK_EDITABLE(rc->text_box),
				0, -1),
			send_as, false);
	}


	/* The event catcher list */
	catcher = g_slist_append(catcher, rc->etd);
}

void cancel_request_chat(GtkWidget *widget, gpointer _rc)
{
	struct request_chat *rc = (struct request_chat *)_rc;
	icq_daemon->CancelEvent(rc->etd->e_tag);
	catcher = g_slist_remove(catcher, rc->etd);
	close_request_chat(rc);
}

void close_request_chat(struct request_chat *rc)
{
	rc_list = g_slist_remove(rc_list, rc);
	gtk_widget_destroy(rc->window);
}

void chat_accept_window(CEventChat *c_event, gulong uin,
	bool auto_accept)
{
	struct remote_chat_request *r_cr = g_new0(struct remote_chat_request, 1);

	/* Fill in the structure */
	r_cr->uin = uin;
	r_cr->c_event = c_event;

	/* Make the dialog window */
	r_cr->dialog = gtk_dialog_new();

	if(auto_accept)
	{
		chat_accept(0, (gpointer)r_cr);
		return;
	}
	
	GtkWidget *label;
	GtkWidget *accept;
	GtkWidget *refuse;
	
	/* Make the buttons */
	accept = gtk_button_new_with_label("Accept");
	refuse = gtk_button_new_with_label("Refuse");

	/* Add the buttons */
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(r_cr->dialog)->action_area),
			  accept);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(r_cr->dialog)->action_area),
			  refuse);

	/* Get the label for the window with the chat reason */
	ICQUser *u = gUserManager.FetchUser(uin, LOCK_R);
	gchar *alias = u->GetAlias();
	gUserManager.DropUser(u);

	gchar *forLabel = g_strdup_printf("Chat with %s (%ld)\nReason:\n%s",
		alias, uin, c_event->Text());
	label =gtk_label_new(forLabel);
	g_free(forLabel);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(r_cr->dialog)->vbox), label);

	/* Connect all the signals */
	gtk_signal_connect(GTK_OBJECT(accept), "clicked",
			   GTK_SIGNAL_FUNC(chat_accept), (gpointer)r_cr);
	gtk_signal_connect(GTK_OBJECT(refuse), "clicked",
			   GTK_SIGNAL_FUNC(chat_refuse), (gpointer)r_cr);

	/* Show everything */
	gtk_widget_show_all(r_cr->dialog);
}

void chat_accept(GtkWidget *widget, gpointer data)
{
	/* Don't do a lot of casting, just make another one here */
	struct remote_chat_request *r_cr = (struct remote_chat_request *)data;

	/* Close the request window */
	gtk_widget_destroy(r_cr->dialog);

	/* Join a multiparty chat (we connect to them) */
	if(r_cr->c_event->Port() != 0)
		chat_join_multiparty(r_cr);
	/* Single party chat (they connect to us) */
	else
		chat_start_as_server(r_cr->uin, r_cr->c_event);
}

void chat_refuse(GtkWidget *widget, gpointer data)
{
	struct remote_chat_request *r_cr = (struct remote_chat_request *)data;
	/* Refuse the chat */
	icq_daemon->icqChatRequestRefuse(r_cr->uin, "",
					 r_cr->c_event->Sequence(),
					 r_cr->c_event->MessageID(),
					 r_cr->c_event->IsDirect());

	/* Close the request window */
	gtk_widget_destroy(r_cr->dialog);
}

void chat_join_multiparty(struct remote_chat_request *r_cr)
{
	/* Make the window and the chat manager */
	struct chat_window *cw = chat_window_create(r_cr->uin);

	if(!cw->chatman->StartAsClient(r_cr->c_event->Port()))
		return;

	icq_daemon->icqChatRequestAccept(r_cr->uin, cw->chatman->LocalPort(),
					 r_cr->c_event->Sequence(),
					 r_cr->c_event->MessageID(),
					 r_cr->c_event->IsDirect());
}

void chat_start_as_server(gulong uin, CEventChat *c)
{
	/* Make the window and the chat manager */
	struct chat_window *cw = chat_window_create(uin);
	
	if(!cw->chatman->StartAsServer())
		return;

	icq_daemon->icqChatRequestAccept(uin, cw->chatman->LocalPort(),
					 c->Sequence(),
					 c->MessageID(), c->IsDirect());
}

void chat_start_as_client(ICQEvent *event)
{
	CExtendedAck *ea = event->ExtendedAck();
	
	if(ea == 0)
	{
		gLog.Error("%sInternal error: chat_start_as_client(): chat request acknowledgement without extended result.\n", L_ERRORxSTR);
		return;
	}

	/* Refused */
	if(!ea->Accepted())
	{
		return;
	}

	/* Make the window and the chat manager */
	struct chat_window *cw = chat_window_create(event->Uin());

	if(!cw->chatman->StartAsClient(ea->Port()))
		return;

	gtk_frame_set_label(GTK_FRAME(cw->frame_remote),
			    "Remote - Waiting for joiners...");
	gLog.Info("%sChat: Waiting for joiners.\n", L_TCPxSTR);
}

struct chat_window *chat_window_create(gulong uin)
{
	GtkWidget *scroll1;
	GtkWidget *scroll2;
	GtkWidget *v_box;
	GtkWidget *menu_bar;
	GtkWidget *label;
	struct chat_window *cw;

	cw = g_new0(struct chat_window, 1);

	// Add to the chat list
	chat_list.push_back(cw);

	/* Take care of the chat manager stuff */
	cw->chatman = new CChatManager(icq_daemon, uin);
	cw->input_tag = gdk_input_add(cw->chatman->Pipe(), GDK_INPUT_READ,
				      chat_pipe_callback, (gpointer)cw);

	/* Take care of the font now */
	cw->r_font = gdk_font_load("-*-helvetica-medium-r-normal--*-120-*-*-*-*-iso8859-1");
	strncpy(cw->r_font_name, "helvetica", 50);
	cw->r_font_size = 120;
	
	/* Take care of creating the colors now */
	cw->r_back_color = new GdkColor;
	cw->r_fore_color = new GdkColor;
	cw->l_back_color = new GdkColor;
	cw->l_fore_color = new GdkColor;

	/* Default background color */
	cw->r_back_color->red = 257 * cw->chatman->ColorBg()[0];
	cw->r_back_color->green = 257 * cw->chatman->ColorBg()[1];
	cw->r_back_color->blue = 257 * cw->chatman->ColorBg()[2];

	/* Default foreground color */
	cw->r_fore_color->red = 257 * cw->chatman->ColorFg()[0];
	cw->r_fore_color->green = 257 * cw->chatman->ColorFg()[1];
	cw->r_fore_color->blue = 257 * cw->chatman->ColorFg()[2];

	ICQUser *u = gUserManager.FetchUser(uin, LOCK_R);
	cw->user = u;
	gUserManager.DropUser(u);

	/* Last position index */
	cw->last_pos = 0;

	/* Make the window */
	cw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(cw->window), "Licq - Chat");
	gtk_window_set_position(GTK_WINDOW(cw->window), GTK_WIN_POS_CENTER);

	/* Create the v_box */
	v_box = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(cw->window), v_box);
	gtk_widget_show(v_box);

	/* Create the menu */
	menu_bar = chat_create_menu(cw);
	gtk_box_pack_start(GTK_BOX(v_box), menu_bar, FALSE, FALSE, 0);

	/* Create the notepad */
	cw->notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(v_box), cw->notebook, FALSE, FALSE, 0);
	
	/* Create the table */
	cw->table = gtk_table_new(4, 1, FALSE);

	/* Create the remote frame with the remote scrolled text window */
	cw->frame_remote = gtk_frame_new("Remote");
	cw->text_remote = gtk_text_new(0, 0);
	gtk_text_set_editable(GTK_TEXT(cw->text_remote), FALSE);
	gtk_text_set_word_wrap(GTK_TEXT(cw->text_remote), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(cw->text_remote), TRUE);
	scroll1 = gtk_scrolled_window_new(0,
					  GTK_TEXT(cw->text_remote)->vadj);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll1),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scroll1, 320, 75);
	gtk_container_add(GTK_CONTAINER(scroll1), cw->text_remote);
	gtk_container_add(GTK_CONTAINER(cw->frame_remote), scroll1);
	gtk_table_attach(GTK_TABLE(cw->table), cw->frame_remote, 0, 1, 1, 2,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 10, 10);

	/* Create the local frame with local text box */
	ICQOwner *o = gUserManager.FetchOwner(LOCK_R);

	gchar *forLFrame = g_strdup_printf("Local - %s", o->GetAlias());
	cw->frame_local = gtk_frame_new(forLFrame);
	g_free(forLFrame);
	gUserManager.DropUser(o);

	cw->text_local = gtk_text_new(0, 0);
	gtk_text_set_editable(GTK_TEXT(cw->text_local), TRUE);
	gtk_text_set_word_wrap(GTK_TEXT(cw->text_local), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(cw->text_local), TRUE);
	scroll2 = gtk_scrolled_window_new(0,
					  GTK_TEXT(cw->text_local)->vadj);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll2),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scroll2, 320, 75);
	gtk_container_add(GTK_CONTAINER(scroll2), cw->text_local);
	gtk_container_add(GTK_CONTAINER(cw->frame_local), scroll2);
	gtk_table_attach(GTK_TABLE(cw->table), cw->frame_local, 0, 1, 2, 3,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 10, 10);

	/* The IRC Mode window with user list window */

	/* First thing is the table */
	cw->table_irc = gtk_table_new(2, 2, FALSE);

	/* Create the main text box in a scrolled window */
	cw->text_irc = gtk_text_new(0, 0);
	gtk_text_set_editable(GTK_TEXT(cw->text_irc), FALSE);
	gtk_text_set_word_wrap(GTK_TEXT(cw->text_irc), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(cw->text_irc), TRUE);
	scroll1 = gtk_scrolled_window_new(0,
					  GTK_TEXT(cw->text_irc)->vadj);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll1),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_usize(scroll1, 320, 175);
	gtk_container_add(GTK_CONTAINER(scroll1), cw->text_irc);
	gtk_table_attach(GTK_TABLE(cw->table_irc), scroll1, 0, 1, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 5, 5);

	/* Create the entry box */
	cw->entry_irc = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(cw->table_irc), cw->entry_irc,
			 0, 1, 1, 2, GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 5, 5);

	/* The list of users for the irc panel */
	cw->list_users = gtk_clist_new(1);
	gtk_table_attach(GTK_TABLE(cw->table_irc), cw->list_users,
			 1, 2, 0, 2, GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 5, 5);

	gtk_widget_set_usize(cw->list_users, 55, 100);
	
	/* The notebook for panel or irc chat mode */
	label = gtk_label_new("Pane Mode");
	gtk_notebook_append_page(GTK_NOTEBOOK(cw->notebook),
				 cw->table, label);
	label = gtk_label_new("IRC Mode");
	gtk_notebook_append_page(GTK_NOTEBOOK(cw->notebook),
				 cw->table_irc, label);

	/* We need every key press to know how to send it */
	gtk_signal_connect(GTK_OBJECT(cw->text_local),
			   "key-press-event",
			   GTK_SIGNAL_FUNC(chat_send), cw);
	gtk_signal_connect(GTK_OBJECT(cw->entry_irc), "key-press-event",
			   GTK_SIGNAL_FUNC(chat_send), cw);

	gtk_widget_show_all(cw->window);

	return cw;
}

GtkWidget *chat_create_menu(struct chat_window *cw)
{
	GtkItemFactoryEntry menu_items[] =
	{
       		{ "/_Chat",        0,         0, 0, "<Branch>" },
       		{ "/Chat/_Audio", "<control>A",
		  GtkItemFactoryCallback(chat_audio), 0, "<ToggleItem>" },
//		{ "/Chat/_Kick", "<control>K",
//		  GtkItemFactoryCallback(chat_kick), 0, 0},
//		{ "/Chat/Kick _No Vote", "<control>N",
//		  GtkItemFactoryCallback(chat_kick_no_vote), 0, 0},
		{ "/Chat/_Save", "<control>S",
		  GtkItemFactoryCallback(chat_save), 0, 0},
       		{ "/Chat/sep1",	   0,	 0, 0, "<Separator>" },
       		{ "/Chat/_Close",  "<control>C",
		  GtkItemFactoryCallback(chat_close), 0, 0},
		{ "/_More",	   0,	0, 0, "<Branch>" },
		{ "/More/_Beep",   "<control>B",
		  GtkItemFactoryCallback(chat_beep_users), 0, 0},
//		{ "/More/Change Font", 0,
//		   GtkItemFactoryCallback(chat_change_font), 0, 0},
	};

	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	gint size = sizeof(menu_items) / sizeof(menu_items[0]);

	/* Make a new accelerators group */
	accel_group = gtk_accel_group_new();

	/* Create the item factory */
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
					    accel_group);

	/* Generate the menu items. */
	gtk_item_factory_create_items(item_factory, size, menu_items,
				      (gpointer)cw);

	/* Attach the new accelerator group to the chat window */
	gtk_window_add_accel_group(GTK_WINDOW(cw->window), accel_group);

	return (gtk_item_factory_get_widget(item_factory, "<main>"));
}

void chat_audio(gpointer cw, guint action, GtkWidget *widget)
{
	((struct chat_window *)cw)->audio = !((struct chat_window *)cw)->audio;
}

void start_kick_window(struct chat_window *cw)
{
	cw->kw = g_new0(struct kick_window, 1);

	// Make a window to get the user to kick.  Then create a "tally" window
	// to keep a tally of the voting.
	cw->kw->winKick = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(cw->kw->winKick), "Kick User");

	cw->kw->cmbUsers = gtk_combo_new();
	GList *users = 0;

	list<CChatUser *>::iterator iter;
	for(iter = cw->ChatUsers.begin(); iter != cw->ChatUsers.end(); iter++)
	{
		gchar *temp = g_strdup_printf("%s (%ld)", (*iter)->Name(),
			(*iter)->Uin());
		g_list_append(users, temp);
		g_free(temp);
	}

	gtk_combo_set_popdown_strings(GTK_COMBO(cw->kw->cmbUsers), users);

	gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(cw->kw->cmbUsers)->entry), false);

	cw->kw->btnKick = gtk_button_new_with_label("Kick");
	GtkWidget *btnCancel = gtk_button_new_with_label("Cancel");

	// Leave the "clicked" signal for btnKick to each function
	// since it varies
	gtk_signal_connect(GTK_OBJECT(btnCancel), "clicked",
		GTK_SIGNAL_FUNC(dialog_close), cw->kw->winKick);
	gtk_signal_connect(GTK_OBJECT(cw->kw->winKick), "destroy",
		GTK_SIGNAL_FUNC(dialog_close), cw->kw->winKick);

	GtkWidget *tblTable = gtk_table_new(2, 2, false);
	gtk_container_add(GTK_CONTAINER(cw->kw->winKick), tblTable);

	// Label for the combo
	GtkWidget *lblLabel = gtk_label_new("User:");
	gtk_table_attach(GTK_TABLE(tblTable), lblLabel, 0, 1, 0, 1,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL), 3, 3);

	// Attach the combo, and then the 2 buttons that are in a hbox
	gtk_table_attach(GTK_TABLE(tblTable), cw->kw->cmbUsers, 1, 2, 0, 1,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL), 3, 3);

	GtkWidget *boxButtons = gtk_hbox_new(true, 0);
	gtk_box_pack_start(GTK_BOX(boxButtons), cw->kw->btnKick, true, true, 5);
	gtk_box_pack_start(GTK_BOX(boxButtons), btnCancel, true, true, 5);
	
	gtk_table_attach(GTK_TABLE(tblTable), boxButtons, 0, 1, 1, 2,
		GtkAttachOptions(GTK_EXPAND | GTK_FILL),
		GtkAttachOptions(GTK_EXPAND | GTK_FILL), 3, 3);

}

void chat_kick(gpointer _cw, guint action, GtkWidget *widget)
{
	struct chat_window *cw = (struct chat_window *)_cw;
	start_kick_window(cw);
	
	gtk_signal_connect(GTK_OBJECT(cw->kw->btnKick), "clicked",
		GTK_SIGNAL_FUNC(kick_callback), (gpointer)cw);

	gtk_widget_show_all(cw->kw->winKick);
}

unsigned long start_kick_callback(struct chat_window *cw)
{
	// Get their UIN and vote to kick them
	gchar *toKick = gtk_editable_get_chars(GTK_EDITABLE(cw->kw->cmbUsers),
		0, -1);
	g_strreverse(toKick);

	// The '('
	toKick++;

	// Stop at the ')'
	char *stop = ")";
	unsigned long Uin = strtoul(toKick, &stop, 10);
	return Uin;
}

void kick_callback(GtkWidget *widget, gpointer _cw)
{
	struct chat_window *cw = (struct chat_window *)_cw;
	unsigned long Uin = start_kick_callback(cw);	

	cw->chatman->SendKick(Uin);

	// Close the kick window
	gtk_widget_destroy(cw->kw->winKick);

	// Create a tally window
}

void chat_kick_no_vote(gpointer _cw, guint action, GtkWidget *widget)
{
	struct chat_window *cw = (struct chat_window *)_cw;
	start_kick_window(cw);

	gtk_signal_connect(GTK_OBJECT(cw->kw->btnKick), "clicked",
		GTK_SIGNAL_FUNC(kick_no_vote_callback), (gpointer)cw);

	gtk_widget_show_all(cw->kw->winKick);
}

void kick_no_vote_callback(GtkWidget *widget, gpointer _cw)
{
	struct chat_window *cw = (struct chat_window *)_cw;
	unsigned long Uin = start_kick_callback(cw);

	cw->chatman->SendKickNoVote(Uin);

	// Close the kick window
	gtk_widget_destroy(cw->kw->winKick);

	// Create the tally window
}

void chat_save(gpointer _cw, guint action, GtkWidget *widget)
{
	struct chat_window *cw = (struct chat_window *)_cw;	

	GtkWidget *dlg_save_chat = gtk_file_selection_new("Licq - Save Chat");

	// Connect the signals
	gtk_signal_connect(
		GTK_OBJECT(GTK_FILE_SELECTION(dlg_save_chat)->ok_button),
		"clicked", GTK_SIGNAL_FUNC(save_chat_ok),
		_cw);
	gtk_signal_connect(
		GTK_OBJECT(GTK_FILE_SELECTION(dlg_save_chat)->cancel_button),
		"clicked", GTK_SIGNAL_FUNC(save_chat_cancel),
		0);
	gtk_signal_connect(GTK_OBJECT(dlg_save_chat), "delete_event",
		GTK_SIGNAL_FUNC(save_chat_cancel), 0);

	gtk_widget_show_all(dlg_save_chat);
}

void save_chat_ok(GtkWidget *widget, gpointer _cw)
{
	struct chat_window *cw = (struct chat_window *)_cw;
}

void save_chat_cancel(GtkWidget *widget, gpointer data)
{
}

void chat_close(gpointer temp_cw, guint action, GtkWidget *widget)
{
	/* Take the structure and put it in a variable in this scope */
	struct chat_window *cw = (struct chat_window *)temp_cw;

	if(cw->hold_cuser == 0)
	{
		cw->chat_user = 0;
		gdk_input_remove(cw->input_tag);
		cw->chatman->CloseChat();
	}

	/* Remove the user's name from the list box */
	else
	{
		if(cw->chat_user == cw->hold_cuser)
			cw->chat_user = 0;
	}

	// We are unwanted now
	if(cw->chatman->ConnectedUsers() == 0)
	{
		// XXX Ask if the user wants to save the chat should go here
		gtk_widget_destroy(cw->window);
		delete cw->r_back_color;
		delete cw->r_fore_color;
		delete cw->l_back_color;
		delete cw->l_fore_color;
	}
}

void chat_pipe_callback(gpointer g_cw, gint pipe,
			GdkInputCondition condition)
{
	struct chat_window *cw = (struct chat_window *)g_cw;

	/* Read out any pending events */
	gchar buf[32];
	read(pipe, buf, 32);

	CChatEvent *e = 0;
	while((e = cw->chatman->PopChatEvent()) != 0)
	{
		CChatUser *user = e->Client();
		cw->hold_cuser = user;

		switch(e->Command())
		{
			case CHAT_DISCONNECTION:
			{
//				list<CChatUser *>::iterator iter;
//				for(iter = cw->ChatUsers.begin();
//				    iter != cw->ChatUsers.end(); ++iter)
//				{
//				  if((*iter)->Uin() == user->Uin())
//				    cw->ChatUsers.erase(iter);
//				}

				gchar *temp = g_strdup_printf("%s closed "
					"connection", user->Name());
				message_box(temp);
				g_free(temp);

				chat_close((gpointer)cw, 0, 0);
				break;
			}

			case CHAT_DISCONNECTIONxKICKED:
			{
//				list<CChatUser *>::iterator iter;
//				for(iter = cw->ChatUsers.begin();
//				    iter != cw->ChatUsers.end(); ++iter)
//				{
//					if((*iter)->Uin() == user->Uin())
//						cw->ChatUsers.erase(iter);
//				}

				gchar *temp = g_strdup_printf("%s was kicked",
					user->Name());
				message_box(temp);
				g_free(temp);

				chat_close((gpointer)cw, 0, 0);
				break;
			}

			case CHAT_KICKxYOU:
			{
				gchar *temp = g_strdup_printf("You have been kicked!");
				message_box(temp);
				g_free(temp);

				break;
			}

			case CHAT_CONNECTION:
			{
//				cw->ChatUsers.push_back(user);
				gchar *filler[1]; 
				filler[0] = const_cast<char *>(user->Name());
				
				gtk_clist_insert(GTK_CLIST(cw->list_users),
						 cw->chatman->ConnectedUsers(),
						 filler);

				if(cw->chat_user == 0)
				{
					cw->chat_user = user;
					gtk_notebook_set_page(GTK_NOTEBOOK(
							      cw->notebook), 0);
				}
				else
				{
					gtk_notebook_set_page(GTK_NOTEBOOK(
							      cw->notebook), 1);
					break;
				}

				gchar *temp = g_strdup_printf("Remote - %s",
					user->Name());
				gtk_frame_set_label(
					GTK_FRAME(cw->frame_remote), temp);
				g_free(temp);

				/* Get their back color */
				cw->r_back_color->red =
					257 * user->ColorBg()[0];
				cw->r_back_color->green =
					257 * user->ColorBg()[1];
				cw->r_back_color->blue =
					257 * user->ColorBg()[2];
				
				/* Get their fore color */
				cw->r_fore_color->red =
					257 * user->ColorFg()[0];
				cw->r_fore_color->green =
					257 * user->ColorFg()[1];
				cw->r_fore_color->blue =
					257 * user->ColorFg()[2];

				break;
			}
			
			case CHAT_NEWLINE:
			{
				/* Add it to the paned view */
				if(user == cw->chat_user)
				{
				      gtk_text_insert(GTK_TEXT(cw->text_remote),
						      0, 0, 0, "\n", -1);
				}

				/* Add it to the irc view */
				gchar *temp = g_strdup_printf("%s> ",
					user->Name());
				gtk_text_insert(GTK_TEXT(cw->text_irc),
						cw->r_font,
						cw->r_fore_color,
						cw->r_back_color,
						temp, -1);
				g_free(temp);

				temp = g_strdup_printf("%s\n", e->Data());
				gtk_text_insert(GTK_TEXT(cw->text_irc),
						cw->r_font,
						cw->r_fore_color,
						cw->r_back_color,
						temp, -1);
				g_free(temp);
				break;
			}

			case CHAT_BEEP:
			{
				/* Is audio enabled? */
				if(cw->audio)
					gdk_beep();
				/* If not show a <Beep Beep!> */
				else
				{
					if(user == cw->chat_user)
						gtk_text_insert(
							GTK_TEXT(cw->text_remote),
							cw->r_font,
							cw->r_fore_color,
							cw->r_back_color,
							"<Beep Beep!>\n",
							-1);
					
					gchar *temp = g_strdup_printf(
						"%s> <Beep Beep!>\n",
						user->Name());
					gtk_text_insert(GTK_TEXT(cw->text_irc),
							0, 0, 0,
							temp, -1);
					g_free(temp);
				}

				break;
			}

			case CHAT_LAUGH:
			{
				break;
			}

			case CHAT_BACKSPACE:
			{
				if(user == cw->chat_user)
				{
					gtk_text_backward_delete(
						GTK_TEXT(cw->text_remote),
						1);
				}
				
				break;
			}

			case CHAT_CHARACTER:
			{
				if(user == cw->chat_user)
				{
					gtk_text_insert(GTK_TEXT(cw->text_remote),
							cw->r_font,
							cw->r_fore_color,
							cw->r_back_color,
							e->Data(), -1);
				}
				
				break;
			}

			case CHAT_COLORxFG:
			{
				if(user == cw->chat_user)
				{
					cw->r_fore_color->red =
						257 * user->ColorFg()[0];
					cw->r_fore_color->green =
						257 * user->ColorFg()[1];
					cw->r_fore_color->blue =
						257 * user->ColorFg()[2];
				}

				break;
			}

			case CHAT_COLORxBG:
			{
				if(user == cw->chat_user)
				{
					cw->r_back_color->red =
						257 * user->ColorBg()[0];
					cw->r_back_color->green =
						257 * user->ColorBg()[1];
					cw->r_back_color->blue =
						257 * user->ColorBg()[2];
				}

				break;
			}

			case CHAT_FONTxFAMILY:
			{
				strncpy(cw->r_font_name, user->FontFamily(), 50);
				gchar *temp = g_strdup_printf(
					"-*-%s-%s-%c-normal--*-%d-*-*-*-*-iso8859-1",
					cw->r_font_name,
					cw->r_bold ? "bold" : "medium",
					cw->r_italic ? 'i' : 'r',
					cw->r_font_size);
				cw->r_font = gdk_font_load(temp);
				g_free(temp);

				break;
			}

			case CHAT_FONTxFACE:
			{
				if(user == cw->chat_user)
				{
				  user->FontBold() ? cw->r_bold = TRUE :
				    cw->r_bold = FALSE;
				  user->FontItalic() ? cw->r_italic = TRUE :
				    cw->r_italic = FALSE;
				
				  gchar *temp = g_strdup_printf(
					  "-*-%s-%s-%c-normal--*-%d-*-*-*-*-iso8859-1",
					  cw->r_font_name,
					  cw->r_bold ? "bold" : "medium",
					  cw->r_italic ? 'i' : 'r',
					  cw->r_font_size);
				  cw->r_font = gdk_font_load(temp);
				  g_free(temp);
				}
				
				break;
			}

			case CHAT_FONTxSIZE:
			{
				if(user == cw->chat_user)
				{
				  cw->r_font_size = 10 * user->FontSize();
				  
				  gchar *temp = g_strdup_printf("-*-%s-%s-%c-normal--*-%d-*-*-*-*-iso8859-1",
				  	cw->r_font_name,
					cw->r_bold ? "bold" : "medium",
					cw->r_italic ? 'i' : 'r',
					cw->r_font_size);
				  cw->r_font = gdk_font_load(temp);
				  g_free(temp);
				}

				break;
			}

			case CHAT_FOCUSxOUT:
			{
				if(user == cw->chat_user)
				{
					gchar *new_text =
						g_strdup_printf(
							"Remote - %s - Away",
							user->Name());
					gtk_frame_set_label(GTK_FRAME(
						cw->frame_remote), new_text);
					g_free(new_text);
				}

				break;
			}	

			case CHAT_FOCUSxIN:
			{
				if(user == cw->chat_user)
				{
					gchar *new_text =
						g_strdup_printf(
							"Remote - %s",
							user->Name());
					gtk_frame_set_label(GTK_FRAME(
						cw->frame_remote), new_text);
					g_free(new_text);
				}

				break;
			}

			case CHAT_SLEEPxON:
			{
				if(user == cw->chat_user)
				{
					gchar *new_text =
						g_strdup_printf(
							"Remote - %s - Sleep",
							user->Name());
					gtk_frame_set_label(GTK_FRAME(
						cw->frame_remote), new_text);
					g_free(new_text);
				}

				break;
			}

			case CHAT_SLEEPxOFF:
			{
				if(user == cw->chat_user)
				{
					gchar *new_text =
						g_strdup_printf(
							"Remote - %s",
							user->Name());
					gtk_frame_set_label(GTK_FRAME(
						cw->frame_remote), new_text);
					g_free(new_text);
				}

				break;
			}
				
			case CHAT_KICK:
			{
				g_print("Kick user %ld?\n", user->ToKick());
				break;
			}


			default:
			{
				gLog.Error("%sInternal Error: invalid command from chat manager (%d)\n", 
				L_ERRORxSTR, e->Command());
				break;
			}
		}

		cw->hold_cuser = 0;
		delete e;
	}
}

const gint find_list_row(const gchar *name, GtkWidget *clist)
{
return 0;
}

void chat_send(GtkWidget *widget, GdkEventKey *event, struct chat_window *cw)
{
	switch(event->keyval)
	{
		case GDK_BackSpace:
		{
			cw->chatman->SendBackspace();
			break;
		}

		case GDK_Linefeed:
		case GDK_Return:
		{
			/* We're in the IRC mode, handle cw->text_local and **
			** cw->text_irc accordingly.                        */
			if(gtk_notebook_current_page(GTK_NOTEBOOK(cw->notebook))
			   == 1)
			{
				gtk_text_insert(GTK_TEXT(cw->text_local),
					cw->l_font, 0, 0,
					gtk_entry_get_text(GTK_ENTRY(
						cw->entry_irc)), -1);
				gtk_text_insert(GTK_TEXT(cw->text_local),
					cw->l_font, 0, 0, "\n", -1);

				gchar *temp = g_strdup_printf("%s> ",
					cw->chatman->Name());
				gtk_text_insert(GTK_TEXT(cw->text_irc),
					cw->l_font, 0, 0, temp, -1);
				g_free(temp);

				gtk_text_insert(GTK_TEXT(cw->text_irc), 
					cw->l_font , 0, 0,
					gtk_entry_get_text(GTK_ENTRY(
						cw->entry_irc)), -1);
				gtk_text_insert(GTK_TEXT(cw->text_irc),
					cw->l_font, 0, 0,
					"\n", -1);
			
				gtk_entry_set_text(GTK_ENTRY(cw->entry_irc), "");
			}

			/* We're in the Pane mode */
			else if(gtk_notebook_current_page(GTK_NOTEBOOK(
				cw->notebook)) == 0)
			{
				gchar *new_text =
					gtk_editable_get_chars(GTK_EDITABLE(
							       cw->text_local),
							       cw->last_pos,
							       -1);
				gchar *temp = g_strdup_printf("%s> ",
					cw->chatman->Name());
				gtk_text_insert(GTK_TEXT(cw->text_irc),
					cw->l_font, 0, 0, temp, -1);
				g_free(temp);

				gtk_text_insert(GTK_TEXT(cw->text_irc),
					cw->l_font, 0, 0,
					new_text, -1);
				gtk_text_insert(GTK_TEXT(cw->text_irc),
					cw->l_font, 0, 0,
					"\n", -1);
				cw->last_pos = gtk_editable_get_position(
						 GTK_EDITABLE(cw->text_local));
				// Eat the new line
				cw->last_pos++;
			
				g_free(new_text);
			}			
			
			cw->chatman->SendNewline();
			break;
		}

		default:
		{
		}
	}

	/* Limit what we want */
	if(event->keyval < GDK_space || event->keyval > GDK_questiondown)
		return;

	cw->chatman->SendCharacter(event->string[0]);
}

void chat_beep_users(gpointer cw, guint action, GtkWidget *widget)
{
	((struct chat_window *)cw)->chatman->SendBeep();
	gdk_beep();
}

void chat_change_font(gpointer _cw, guint action, GtkWidget *widget)
{
	struct chat_window *cw = (struct chat_window *)_cw;

	cw->font_sel_dlg = gtk_font_selection_dialog_new("Licq - Select Font");

	gtk_signal_connect(GTK_OBJECT(cw->font_sel_dlg), "delete_event",
		GTK_SIGNAL_FUNC(font_dlg_close), (gpointer)cw);
	gtk_signal_connect(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(cw->font_sel_dlg)->
		cancel_button), "clicked",
		GTK_SIGNAL_FUNC(font_dlg_close), (gpointer)cw);
	gtk_signal_connect(GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(cw->font_sel_dlg)->
		ok_button), "clicked",
		GTK_SIGNAL_FUNC(font_dlg_ok), (gpointer)cw);

	gtk_widget_show_all(cw->font_sel_dlg);
}

void font_dlg_close(GtkWidget *widget, gpointer _cw)
{
	struct chat_window *cw = (struct chat_window *)_cw;
	gtk_widget_destroy(cw->font_sel_dlg);
}

void font_dlg_ok(GtkWidget *widget, gpointer _cw)
{
	// Only changes the font face for now...

	struct chat_window *cw = (struct chat_window *)_cw;

	// Get the font name and font if they exist
	char *tmp = gtk_font_selection_dialog_get_font_name(
		GTK_FONT_SELECTION_DIALOG(cw->font_sel_dlg));
	if(tmp != 0)
	{
		int len = 0;
		do
		{
			tmp++;
		} while(*tmp != '-');
	
		tmp++;

		g_print("First: %s\n", tmp);
		
		do
		{
			tmp++;
			len++;
		} while(*tmp != '-');

		g_print("Second: %s\n", tmp);

		for(int i = len; i > 0; i--)
			tmp--;
		
		if(len < 45)
			strncpy(cw->l_font_name, tmp, len);
		else
			strncpy(cw->l_font_name, "clean", 6);

		g_print("Final: %s\n", cw->l_font_name);

		cw->chatman->ChangeFontFamily(cw->l_font_name);
	}

	GdkFont *font_tmp = gtk_font_selection_dialog_get_font(
		GTK_FONT_SELECTION_DIALOG(cw->font_sel_dlg));
	if(font_tmp != 0)
		cw->l_font = font_tmp;

	// Ok, close it now
	gtk_widget_destroy(cw->font_sel_dlg);
}
