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

#include "licq_buffer.h"
#include "licq_chat.h"
#include "licq_icq.h"
#include "licq_icqd.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <list.h>

GSList *rc_list;

// Only needs to be in this scope
typedef list<chat_window *> ChatDlgList;
ChatDlgList chat_list;

void list_request_chat(GtkWidget *widget, ICQUser *user)
{
	/* Do we even want to be here? */
	struct request_chat *rc = rc_find(user->Uin());

	// No get outta here, bitch!
	if(rc != NULL)
		return;

	rc = rc_new(user);

	GtkWidget *scroll;
	GtkWidget *statusbar;
	GtkWidget *multiparty;
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

	/*Create the scrolled window with text and attach it to the window */
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);
	rc->text_box = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(rc->text_box), TRUE);
	gtk_text_set_word_wrap(GTK_TEXT(rc->text_box), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(rc->text_box), TRUE);
	gtk_container_add(GTK_CONTAINER(scroll), rc->text_box);
	gtk_table_attach(GTK_TABLE(table), scroll, 0, 2, 0, 1,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 3, 3);

	/* The send as buttons */
	rc->send_norm = gtk_radio_button_new_with_label(NULL, "Send Normal");
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
	
	/* The button box */
	h_box = gtk_hbox_new(TRUE, 5);
	multiparty = gtk_button_new_with_label("Multi-Party");
	ok = gtk_button_new_with_label("Single-Party");
	cancel = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(h_box), multiparty, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), cancel, TRUE, TRUE, 0);
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 3, 4,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);
	
	/* Connect the signals */
	gtk_signal_connect(GTK_OBJECT(multiparty), "clicked",
			   GTK_SIGNAL_FUNC(multi_request_chat), (gpointer)rc);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(ok_request_chat), (gpointer)rc);
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			   GTK_SIGNAL_FUNC(cancel_request_chat), (gpointer)rc);

	gtk_widget_show_all(rc->window);
	g_free(title);
}

struct request_chat *rc_new(ICQUser *user)
{
	struct request_chat *rc;

	/* Does it already exist? */
	rc = rc_find(user->Uin());

	/* Don't make it if it's already there, return it */
	if(rc != NULL)
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
	return NULL;
}

void multi_request_chat(GtkWidget *widget, gpointer _rc)
{
	struct request_chat *rc = (struct request_chat *)_rc;
	// I'm tired now... good night... 
	// Here's the plan:
	// Make a multi_request_chat struct
	// Make it have a window
	// a CLIST of the current chats with names
	// and an ok and cancel button
	// OK sends it
	// cancel goes back to the rc->window
	// ok?
	// ok.
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

	/* Actually send the chat request */
	rc->etd->e_tag = icq_daemon->icqChatRequest(rc->user->Uin(),
		gtk_editable_get_chars(GTK_EDITABLE(rc->text_box), 0, -1),
		send_as);

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

void chat_accept_window(CEventChat *c_event, gulong uin)
{
	GtkWidget *label;
	GtkWidget *accept;
	GtkWidget *refuse;
	struct remote_chat_request *r_cr = g_new0(struct remote_chat_request, 1);

	/* Fill in the structure */
	r_cr->uin = uin;
	r_cr->c_event = c_event;

	/* Make the dialog window */
	r_cr->dialog = gtk_dialog_new();
	
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

	label =gtk_label_new(g_strdup_printf("Chat with %s (%ld)\nReason:\n%s",
				       alias, uin, c_event->Text()));

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
					 r_cr->c_event->Sequence());

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
					 r_cr->c_event->Sequence());
}

void chat_start_as_server(gulong uin, CEventChat *c)
{
	/* Make the window and the chat manager */
	struct chat_window *cw = chat_window_create(uin);
	
	if(!cw->chatman->StartAsServer())
		return;

	icq_daemon->icqChatRequestAccept(uin, cw->chatman->LocalPort(),
					 c->Sequence());
}

void chat_start_as_client(ICQEvent *event)
{
	CExtendedAck *ea = event->ExtendedAck();
	
	if(ea == NULL)
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
	cw->font_remote = gdk_font_load("-*-helvetica-medium-r-normal--*-120-*-*-*-*-iso8859-1");
	strcpy(cw->font_name, "helvetica");
	cw->font_size = 120;
	
	/* Take care of creating the colors now */
	cw->back_color = new GdkColor;
	cw->fore_color = new GdkColor;

	/* Default background color */
	cw->back_color->red = 257 * cw->chatman->ColorBg()[0];
	cw->back_color->green = 257 * cw->chatman->ColorBg()[1];
	cw->back_color->blue = 257 * cw->chatman->ColorBg()[2];

	/* Default foreground color */
	cw->fore_color->red = 257 * cw->chatman->ColorFg()[0];
	cw->fore_color->green = 257 * cw->chatman->ColorFg()[1];
	cw->fore_color->blue = 257 * cw->chatman->ColorFg()[2];

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
	cw->text_remote = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(cw->text_remote), FALSE);
	gtk_text_set_word_wrap(GTK_TEXT(cw->text_remote), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(cw->text_remote), TRUE);
	scroll1 = gtk_scrolled_window_new(NULL,
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
	cw->frame_local = gtk_frame_new(g_strdup_printf("Local - %s",
					o->GetAlias()));
	gUserManager.DropUser(o);
	cw->text_local = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(cw->text_local), TRUE);
	gtk_text_set_word_wrap(GTK_TEXT(cw->text_local), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(cw->text_local), TRUE);
	scroll2 = gtk_scrolled_window_new(NULL,
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
	cw->text_irc = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(cw->text_irc), FALSE);
	gtk_text_set_word_wrap(GTK_TEXT(cw->text_irc), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(cw->text_irc), TRUE);
	scroll1 = gtk_scrolled_window_new(NULL,
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
       		{ "/_Chat",        NULL,         NULL, 0, "<Branch>" },
       		{ "/Chat/_Audio", "<control>A",
		  GtkItemFactoryCallback(chat_audio), 0, "<ToggleItem>" },
       		{ "/Chat/sep1",	   NULL,	 NULL, 0, "<Separator>" },
       		{ "/Chat/_Close",  "<control>C",
		  GtkItemFactoryCallback(chat_close), 0, NULL},
		{ "/_More",	   NULL,	 NULL, 0, "<Branch>" },
		{ "/More/_Beep",   "<control>B",
		  GtkItemFactoryCallback(chat_beep_users), 0, NULL},
		{ "/More/Change Font", NULL,
		   GtkItemFactoryCallback(chat_change_font), 0, NULL},
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
	if(((struct chat_window *)cw)->audio)
		((struct chat_window *)cw)->audio = FALSE;
	else
		((struct chat_window *)cw)->audio = TRUE;
}

void chat_close(gpointer temp_cw, guint action, GtkWidget *widget)
{
	/* Take the structure and put it in a variable in this scope */
	struct chat_window *cw = (struct chat_window *)temp_cw;

	if(cw->hold_cuser == NULL)
	{
		cw->chat_user = NULL;
		gdk_input_remove(cw->input_tag);
		cw->chatman->CloseChat();
	}

	/* Remove the user's name from the list box */
	else
	{
		if(cw->chat_user == cw->hold_cuser)
			cw->chat_user = NULL;
	}

	if(cw->chatman->ConnectedUsers() == 0)
		gtk_widget_destroy(cw->window);
}

void chat_pipe_callback(gpointer g_cw, gint pipe,
			GdkInputCondition condition)
{
	struct chat_window *cw = (struct chat_window *)g_cw;

	/* Read out any pending events */
	gchar buf[32];
	read(pipe, buf, 32);

	CChatEvent *e = NULL;
	while((e = cw->chatman->PopChatEvent()) != NULL)
	{
		CChatUser *user = e->Client();
		cw->hold_cuser = user;

		switch(e->Command())
		{
			case CHAT_DISCONNECTION:
			{
				message_box(g_strdup_printf("%s closed "
					"connection", user->Name()));
				chat_close((gpointer)cw, 0, NULL);
				break;
			}

			case CHAT_CONNECTION:
			{
				gchar *filler[1]; 
				filler[0] = user->Name();
				
				gtk_clist_insert(GTK_CLIST(cw->list_users),
						 cw->chatman->ConnectedUsers(),
						 filler);

				if(cw->chat_user == NULL)
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

				gtk_frame_set_label(
					GTK_FRAME(cw->frame_remote),
					g_strdup_printf("Remote - %s",
							user->Name()));

				/* Get their back color */
				cw->back_color->red = 257 * user->ColorBg()[0];
				cw->back_color->green = 257 * user->ColorBg()[1];
				cw->back_color->blue = 257 * user->ColorBg()[2];
				
				/* Get their fore color */
				cw->fore_color->red = 257 * user->ColorFg()[0];
				cw->fore_color->green = 257 * user->ColorFg()[1];
				cw->fore_color->blue = 257 * user->ColorFg()[2];

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
				gtk_text_insert(GTK_TEXT(cw->text_irc),
						cw->font_remote,
						cw->fore_color,
						cw->back_color,
						g_strdup_printf("%s> ",
							user->Name()),
						-1);
				gtk_text_insert(GTK_TEXT(cw->text_irc),
						cw->font_remote,
						cw->fore_color,
						cw->back_color,
						g_strdup_printf("%s\n",
							e->Data()),
						-1);
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
							cw->font_remote,
							cw->fore_color,
							cw->back_color,
							"<Beep Beep!>\n",
							-1);
					
					gtk_text_insert(GTK_TEXT(cw->text_irc),
							0, 0, 0,
							"<Beep Beep!>\n",
							-1);
				}

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
							cw->font_remote,
							cw->fore_color,
							cw->back_color,
							e->Data(), -1);
				}
				
				break;
			}

			case CHAT_COLORxFG:
			{
				if(user == cw->chat_user)
				{
					cw->fore_color->red =
						257 * user->ColorFg()[0];
					cw->fore_color->green =
						257 * user->ColorFg()[1];
					cw->fore_color->blue =
						257 * user->ColorFg()[2];
				}

				break;
			}

			case CHAT_COLORxBG:
			{
				if(user == cw->chat_user)
				{
					cw->back_color->red =
						257 * user->ColorBg()[0];
					cw->back_color->green =
						257 * user->ColorBg()[1];
					cw->back_color->blue =
						257 * user->ColorBg()[2];
				}

				break;
			}

			case CHAT_FONTxFAMILY:
			{
				strcpy(cw->font_name, user->FontFamily());
				cw->font_remote = gdk_font_load(
					g_strdup_printf("-*-%s-%s-%c-normal--*-%d-*-*-*-*-iso8859-1",
					cw->font_name,
					cw->remote_bold ? "bold" : "medium",
					cw->remote_italic ? 'i' : 'r',
					cw->font_size));

				break;
			}

			case CHAT_FONTxFACE:
			{
				if(user == cw->chat_user)
				{
				user->FontBold() ? cw->remote_bold = TRUE :
						   cw->remote_bold = FALSE;
				user->FontItalic() ? cw->remote_italic = TRUE
						 : cw->remote_italic = FALSE;
				
				cw->font_remote = gdk_font_load(
						g_strdup_printf(
						"-*-%s-%s-%c-normal--*-%d-*-*-*-*-iso8859-1",
						cw->font_name,
						cw->remote_bold ? "bold" : "medium",
						cw->remote_italic ? 'i' : 'r',
						cw->font_size));
				}
				
				break;
			}

			case CHAT_FONTxSIZE:
			{
				if(user == cw->chat_user)
				{
				cw->font_size = 10 * user->FontSize();
				cw->font_remote = gdk_font_load(
					g_strdup_printf("-*-%s-%s-%c-normal--*-%d-*-*-*-*-iso8859-1",
					cw->font_name,
					cw->remote_bold ? "bold" : "medium",
					cw->remote_italic ? 'i' : 'r',
					cw->font_size));
				}

				break;
			}

			default:
			{
				gLog.Error("%sInternal Error: invalid command from chat manager (%d)\n", 
				L_ERRORxSTR, e->Command());
				break;
			}
		}

		cw->hold_cuser = NULL;
		delete e;
	}
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
				gtk_text_insert(GTK_TEXT(cw->text_local), 0, 0, 0,
						gtk_entry_get_text(GTK_ENTRY(
							cw->entry_irc)), -1);
				gtk_text_insert(GTK_TEXT(cw->text_local), 0, 0,
					        0, "\n", -1);

				gtk_text_insert(GTK_TEXT(cw->text_irc), 0, 0, 0,
						g_strdup_printf("%s> ",
							cw->chatman->Name()), -1);
				gtk_text_insert(GTK_TEXT(cw->text_irc), 0, 0, 0,
						gtk_entry_get_text(GTK_ENTRY(
							cw->entry_irc)),
						-1);
				gtk_text_insert(GTK_TEXT(cw->text_irc), 0, 0, 0,
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
				gtk_text_insert(GTK_TEXT(cw->text_irc), 0, 0, 0,
						g_strdup_printf("%s> ",
							cw->chatman->Name()), -1);
				gtk_text_insert(GTK_TEXT(cw->text_irc), 0, 0, 0,
						new_text, -1);
				gtk_text_insert(GTK_TEXT(cw->text_irc), 0, 0, 0,
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

void chat_change_font(gpointer cw, guint action, GtkWidget *widget)
{
	GtkWidget *font_sel_dlg;

	font_sel_dlg = gtk_font_selection_dialog_new("Licq - Select Font");

	gtk_widget_show_all(font_sel_dlg);
}
