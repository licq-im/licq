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

#include "licq_icqd.h"
#include "licq_user.h"

#include <string.h>
#include <gtk/gtk.h>

GSList *cnv;

struct conversation *convo_new(ICQUser *u, gboolean events)
{
	struct conversation *c;

	if(!events)
	{
		c = convo_find(u->Uin());

		if(c != NULL)
			return c;
	}

	c = g_new0(struct conversation, 1);

	c->user = u;

	cnv = g_slist_append(cnv, c);

	if(events)
	{
		convo_show(c);

		while(c->user->NewMessages() > 0)
			convo_recv(c->user->Uin());
	}

	else
		convo_show(c);

	return c;
}

struct conversation *convo_find(unsigned long uin)
{
	struct conversation *c;
	GSList *conversations = cnv;

	while(conversations)
	{
		c = (struct conversation *)conversations->data;
		if(c->user->Uin() == uin)
		{
			g_free(conversations);
			return c;
		}
	
		conversations = conversations->next;
	}

	g_free(conversations);
	return NULL;
}

void convo_show(struct conversation *c)
{
	GtkWidget *scroll;
	GtkWidget *close;
	GtkWidget *button_box;
	GtkWidget *options_box;
	GtkWidget *vertical_box;
	
	/* Handle the etag stuff */
	struct e_tag_data *etd = g_new0(struct e_tag_data, 1);

	c->etag = etd;

	/* Make the convo window */
	c->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_policy(GTK_WINDOW(c->window), TRUE, TRUE, TRUE);
	gtk_widget_realize(c->window);

	/* Make new buttons with labels */
	c->send = gtk_button_new_with_label("Send");
	c->cancel = gtk_button_new_with_label("Cancel");
	close = gtk_button_new_with_label("Close");

	/* Set cancel to grayed out at first */
	gtk_widget_set_sensitive(c->cancel, FALSE);

	/* Make the boxes */
	button_box = gtk_hbox_new(TRUE, 0);
	vertical_box = gtk_vbox_new(FALSE, 0);
	options_box = gtk_hbox_new(FALSE, 0);

	/* The entry box */
	c->entry = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(c->entry), TRUE);
	gtk_widget_set_usize(c->entry, 320, 75); 

	/* The viewing messages box area */
	c->text = gtk_text_new(NULL, NULL);
	gtk_text_set_editable(GTK_TEXT(c->text), FALSE);
	gtk_text_set_word_wrap(GTK_TEXT(c->text), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(c->text), TRUE);

	/* Scroll bar for the messages that are being viewed */
	scroll = gtk_scrolled_window_new(NULL, GTK_TEXT(c->text)->vadj);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_ALWAYS);
	gtk_widget_show(scroll);
	gtk_container_add(GTK_CONTAINER(scroll), c->text);
	gtk_widget_show(c->text);
	gtk_widget_set_usize(scroll, 320, 150);

	/* Get the signals connected for the buttons */
	gtk_signal_connect(GTK_OBJECT(close), "clicked",
			   GTK_SIGNAL_FUNC(convo_close), c);
	gtk_signal_connect(GTK_OBJECT(c->cancel), "clicked",
			   GTK_SIGNAL_FUNC(convo_cancel), c);
	gtk_signal_connect(GTK_OBJECT(c->send), "clicked",
			   GTK_SIGNAL_FUNC(convo_send), c);

	gtk_box_pack_start(GTK_BOX(button_box), close, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(button_box), c->cancel, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(button_box), c->send, TRUE, TRUE, 5);

	/* Take care of the rest of the widgets */
	gtk_box_pack_start(GTK_BOX(vertical_box), scroll, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(vertical_box), c->entry, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vertical_box), button_box, FALSE, FALSE, 5);

	/* Add the send through server button and spoofing options */
	c->send_server = gtk_check_button_new_with_label("Send through server");
	c->spoof_button = gtk_check_button_new_with_label("Spoof UIN");
	c->spoof_uin = gtk_entry_new_with_max_length(MAX_LENGTH_UIN);

	/* Verify that the c->spoof_uin is digits only */
	gtk_signal_connect(GTK_OBJECT(c->spoof_uin), "insert-text",
			   GTK_SIGNAL_FUNC(verify_numbers), NULL);

	/* No UIN can be entered unless c->spoof_button is checked */
	gtk_widget_set_sensitive(c->spoof_uin, FALSE);

	/* Let's pack them now! */
	gtk_box_pack_start(GTK_BOX(options_box), c->send_server,
			   FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(options_box), c->spoof_button,
			   FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(options_box), c->spoof_uin,
			   FALSE, FALSE, 5);
	
	/* Now pack the options_box */
	gtk_box_pack_start(GTK_BOX(vertical_box), options_box, FALSE, FALSE, 5);

	/* More options, sending normal, urgent or to contact list */
	options_box = gtk_hbox_new(FALSE, 5);

	/* Send the message normal */
	c->send_normal = gtk_radio_button_new_with_label(NULL, "Send Normal");

	/* Send the message urgently */
	c->send_urgent = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(c->send_normal),
				"Send Urgent");

	/* Send the message to contact list */
	c->send_list = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(c->send_normal),
				"Send to Contact List");

	/* If the user is in occupied or dnd mode, set the urgent button */
	if(c->user->Status() == ICQ_STATUS_DND ||
           c->user->Status() == ICQ_STATUS_OCCUPIED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->send_urgent),
				TRUE);

	/* Pack it now */
	gtk_box_pack_start(GTK_BOX(options_box), c->send_normal, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(options_box), c->send_urgent, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(options_box), c->send_list, FALSE, FALSE, 5);
	
	/* Pack the options box */
	gtk_box_pack_start(GTK_BOX(vertical_box), options_box, FALSE, FALSE, 5);

	/* Connect signals for the newly added widgets */
	gtk_signal_connect(GTK_OBJECT(c->spoof_button), "toggled",
			   GTK_SIGNAL_FUNC(spoof_button_callback), c);

	/* Progress of message */
	c->progress = gtk_statusbar_new();
	gtk_signal_connect(GTK_OBJECT(c->progress), "text-pushed",
			   GTK_SIGNAL_FUNC(verify_convo_send), c);

	/* Pack it */
	gtk_box_pack_start(GTK_BOX(vertical_box), c->progress, FALSE, FALSE, 5);

	/* Add the main box into the window */
	gtk_container_add(GTK_CONTAINER(c->window), vertical_box);
	gtk_container_border_width(GTK_CONTAINER(c->window), 10);

	/* Set the title of the window */
	gchar *temp = c->user->GetAlias();
	const gchar *win_title = g_strdup_printf("Conversation with %s", temp);
	gtk_window_set_title(GTK_WINDOW(c->window), win_title);

	/* Set the focus of the window */
	gtk_window_set_focus(GTK_WINDOW(c->window), c->entry);

	/* Don't forget the delete signal */
	gtk_signal_connect(GTK_OBJECT(c->window), "destroy",
			   GTK_SIGNAL_FUNC(convo_close), c);

	/* More e_tag_data stuff */
	c->etag->statusbar = c->progress;
	strcpy(c->etag->buf, c->prog_buf);

	gtk_widget_show_all(c->window);
}

void spoof_button_callback(GtkWidget *widget, struct conversation *c)
{
	gtk_widget_set_sensitive(c->spoof_uin,
		gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->spoof_button)));
}

void convo_send(GtkWidget *widget, struct conversation *c)
{
	/* Set the 2 button widgets */
	gtk_widget_set_sensitive(c->send, FALSE);
	gtk_widget_set_sensitive(c->cancel, TRUE);

	gchar *buf;
	gchar *buf2;
	gboolean urgent = FALSE;

	buf =
	  g_strdup_printf("%s", gtk_editable_get_chars(GTK_EDITABLE(c->entry), 0, -1));
 	const gchar *message = buf;	
	buf2 = g_strdup_printf(":  %s\n", buf);
	c->for_user = buf2;

	c->user->SetSendServer(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server)));

	gulong uin = 0;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->spoof_button)))
	{
		uin = atol((const char *)gtk_editable_get_chars(GTK_EDITABLE(c->spoof_uin), 0, -1));
	}

	/* I don't like those popups to send urgent... so just send it **
 	** urgently unless the user says to send it to the contact list*/	
	if((c->user->Status() == ICQ_STATUS_DND ||
	   c->user->Status() == ICQ_STATUS_OCCUPIED) &&
	   gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_normal)))
		urgent = TRUE;

	strcpy(c->prog_buf, "Sending message ");

	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server)))
		strcat(c->prog_buf, "directly ... ");
	else
		strcat(c->prog_buf, "through server ... ");

	/* Send the message */
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_urgent)) ||
	   urgent)
	{ 
	   c->etag->e_tag = icq_daemon->icqSendMessage(c->user->Uin(), message,
	     (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server))),
	     ICQ_TCPxMSG_URGENT, uin);
	}

	/* Send to contact list */
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_list)))
	{
	  c->etag->e_tag = icq_daemon->icqSendMessage(c->user->Uin(), message,
             (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server))),
             ICQ_TCPxMSG_LIST, uin);
	}

	else /* Just send it normally */
	{
	  c->etag->e_tag = icq_daemon->icqSendMessage(c->user->Uin(), message,
             (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server))),
             ICQ_TCPxMSG_NORMAL, uin);
	}

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(c->progress),
						"prog");
	gtk_statusbar_pop(GTK_STATUSBAR(c->progress), id);
	gtk_statusbar_push(GTK_STATUSBAR(c->progress), id, c->prog_buf);

	/* Take care of the etd buffer and add it to the slist */
	c->etag->buf = c->prog_buf;
	catcher = g_slist_append(catcher, c->etag);
}

void verify_convo_send(GtkWidget *widget, guint id, gchar *text,
		       struct conversation *c)
{
	gchar temp[50];
	strcpy(temp, text);
	g_strreverse(temp);

	if(strncmp(temp, "en", 2) == 0)
		return;

	else
	{
		gtk_widget_set_sensitive(c->send, TRUE);
		gtk_widget_set_sensitive(c->cancel, FALSE);
	}
}

void convo_cancel(GtkWidget *widget, struct conversation *c)
{
	/* Set the buttons sensitivity accordingly */
	gtk_widget_set_sensitive(c->send, TRUE);
	gtk_widget_set_sensitive(c->cancel, FALSE);

	/* Actually cancel this event */
	icq_daemon->CancelEvent(c->etag->e_tag);

	/* Remove the event from the slist */
	catcher = g_slist_remove(catcher, c->etag);
}

void convo_recv(gulong uin)
{
	struct conversation *c;
	c = convo_find(uin);

	/* If the window doesn't exist, don't show anything */
	if(c == NULL)
	{
		system_status_refresh();
		return; 
	}

	CUserEvent *u_event = c->user->EventPop();

	/* Make sure we really have an event */
	if(u_event == NULL)
	{
		return;
	}

	if(u_event->SubCommand() == ICQ_CMDxSUB_MSG)
	{
		const gchar *message = u_event->Text();
	
		const gchar *for_user_m =
	            g_strdup_printf(":  %s\n", message);

		gtk_text_freeze(GTK_TEXT(c->text));
		gtk_text_insert(GTK_TEXT(c->text), 0, red, 0,
				c->user->GetAlias(), -1);
		gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, for_user_m, -1);
		gtk_text_thaw(GTK_TEXT(c->text));
 	}

	else if(u_event->SubCommand() == ICQ_CMDxSUB_URL)
	{	
		const char *url = u_event->Text();

		const gchar *for_user_u =
		   g_strdup_printf("\n%s has sent you a URL!\n%s\n",
					c->user->GetAlias(), url);

		gtk_text_freeze(GTK_TEXT(c->text));
		gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, for_user_u, -1);
		gtk_text_thaw(GTK_TEXT(c->text));

	}

	else if(u_event->SubCommand() == ICQ_CMDxSUB_FILE)
	{
		const gchar *file_d = u_event->Text();
		
		if(u_event->Command() == ICQ_CMDxTCP_CANCEL)
		{
			gtk_text_freeze(GTK_TEXT(c->text));
			gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, file_d, -1);
			gtk_text_thaw(GTK_TEXT(c->text));
		}

		else
		{
			const gchar *for_user_f =
		    	g_strdup_printf("\n%s requests to send you a file!\n%s\n",
				    c->user->GetAlias(), file_d);

			gtk_text_freeze(GTK_TEXT(c->text));
			gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, for_user_f, -1);
			gtk_text_thaw(GTK_TEXT(c->text));

			file_accept_window(c->user, u_event);
		}
	}
}

gboolean convo_close(GtkWidget *widget, struct conversation *c)
{
	gtk_widget_destroy(c->window);
	cnv = g_slist_remove(cnv, c);
	catcher = g_slist_remove(catcher, c->etag);
	return TRUE;
}
