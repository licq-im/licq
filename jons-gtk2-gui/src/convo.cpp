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

#include "licq_color.h"
#include "licq_icqd.h"
#include "licq_user.h"

#include <string.h>
#include <time.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

using namespace std; // for std::list
GSList *cnv;

struct conversation *convo_new(ICQUser *u, gboolean events)
{
	struct conversation *c;

	if(!events)
	{
		c = convo_find(u->Uin());

		if(c != 0)
		{
			if(u->Status() == ICQ_STATUS_OFFLINE)
				gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(c->send_server), true);
			return c;
		}
	}

	c = g_new0(struct conversation, 1);

	c->user = u;
	c->clrBack = new GdkColor;
	c->clrFore = new GdkColor;
	c->for_user = 0;

	cnv = g_slist_append(cnv, c);

	if(events)
	{
		convo_show(c);

		while(c->user->NewMessages() > 0)
			convo_recv(c->user->Uin());
	}

	else
		convo_show(c);

	if(u->Status() == ICQ_STATUS_OFFLINE)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->send_server),
			true);

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
			return c;
		}
	
		conversations = conversations->next;
	}

	return 0;
}

void
toggle_close_cancel(struct conversation *c, int which)
{
	if (which == 1)
		gtk_button_set_label(GTK_BUTTON(c->cancel), GTK_STOCK_CLOSE);
	else
		gtk_button_set_label(GTK_BUTTON(c->cancel), GTK_STOCK_CANCEL);
}

void convo_close_or_cancel(GtkWidget *widget, struct conversation *c)
{
	if (strcmp(gtk_button_get_label(GTK_BUTTON(widget)), GTK_STOCK_CANCEL) == 0)
		convo_cancel(widget, c);
	else
		convo_close(widget, c);
}

void convo_show(struct conversation *c)
{
	GtkWidget *scroll;
	GtkWidget *button_box;
	GtkWidget *options_box;
	GtkWidget *vertical_box;
	
	/* Handle the etag stuff */
	c->etag = g_new0(struct e_tag_data, 1);

	/* Make the convo window */
	c->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(c->window), TRUE);
	gtk_widget_realize(c->window);

	/* Make new buttons with labels */
	c->send = gtk_button_new_with_mnemonic("_Send");
	c->cancel = gtk_button_new_from_stock(GTK_STOCK_CLOSE);

	/* Set cancel to grayed out at first */
	//gtk_widget_set_sensitive(c->cancel, FALSE);

	/* Make the boxes */
	button_box = gtk_hbox_new(TRUE, 0);
	vertical_box = gtk_vbox_new(FALSE, 0);
	options_box = gtk_hbox_new(FALSE, 5);

	/* The entry box */
	c->entry = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(c->entry), TRUE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(c->entry), GTK_WRAP_WORD);
	g_signal_connect(G_OBJECT(c->entry), "key_press_event",
			  G_CALLBACK(key_press_convo), (gpointer)c);

	gtk_widget_set_size_request(c->entry, 320, 75); 

	GtkWidget *frame1 = gtk_frame_new(0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_IN); 
	gtk_container_add(GTK_CONTAINER(frame1), c->entry);
	gtk_widget_show(frame1);

	/* The viewing messages box area */
	c->text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(c->text), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(c->text), GTK_WRAP_WORD);

	/* Scroll bar for the messages that are being viewed */
	scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_ALWAYS);
	gtk_widget_show(scroll);
	gtk_container_add(GTK_CONTAINER(scroll), c->text);
	gtk_widget_show(c->text);
	gtk_widget_set_size_request(scroll, 320, 150);

	GtkWidget *frame = gtk_frame_new(0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN); 
	gtk_widget_show(frame);
	gtk_container_add(GTK_CONTAINER(frame), scroll);
  
	/* Get the signals connected for the buttons */
	g_signal_connect(G_OBJECT(c->cancel), "clicked",
			   G_CALLBACK(convo_close_or_cancel), c);
	g_signal_connect(G_OBJECT(c->send), "clicked",
			   G_CALLBACK(convo_send), (gpointer)c);
	
	gtk_box_pack_start(GTK_BOX(button_box), c->cancel, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(button_box), c->send, TRUE, TRUE, 5);

	/* Take care of the rest of the widgets */
	gtk_box_pack_start(GTK_BOX(vertical_box), frame, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(vertical_box), frame1, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vertical_box), button_box, FALSE, FALSE, 5);

	/* Add the send through server button */
	c->send_server = gtk_check_button_new_with_mnemonic("Se_nd through server");

	/* Send the message normal */
	c->send_normal = gtk_radio_button_new_with_label(0, "Normal");

	/* Send the message urgently */
	c->send_urgent = gtk_radio_button_new_with_mnemonic_from_widget(
				GTK_RADIO_BUTTON(c->send_normal),
				"U_rgent");

	/* Send the message to contact list */
	c->send_list = gtk_radio_button_new_with_mnemonic_from_widget(
				GTK_RADIO_BUTTON(c->send_normal),
				"To Contact _List");

	/* Let's pack them now! */
	gtk_box_pack_start(GTK_BOX(options_box), c->send_server,
			   FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(options_box), c->send_normal,
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(options_box), c->send_urgent,
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(options_box), c->send_list,
			   FALSE, FALSE, 0);

	/* Now pack the options_box */
	gtk_box_pack_start(GTK_BOX(vertical_box), options_box, FALSE, FALSE, 5);

	/* If the user is in occupied or dnd mode, set the urgent button */
	if(c->user->Status() == ICQ_STATUS_DND ||
           c->user->Status() == ICQ_STATUS_OCCUPIED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->send_urgent),
				TRUE);

	/* Progress of message */
	c->progress = gtk_statusbar_new();
	g_signal_connect(G_OBJECT(c->progress), "text-pushed",
			   G_CALLBACK(verify_convo_send), c);
	
	/* Pack it */
	gtk_box_pack_start(GTK_BOX(vertical_box), c->progress, FALSE, FALSE, 5);

	/* Add the main box into the window */
	gtk_container_add(GTK_CONTAINER(c->window), vertical_box);
	gtk_container_set_border_width(GTK_CONTAINER(c->window), 10);

	/* Set the title of the window */
	gchar *temp = c->user->GetAlias();
	const gchar *win_title = g_strdup_printf("Conversation with %s", temp);
	gtk_window_set_title(GTK_WINDOW(c->window), win_title);

	/* Set the focus of the window */
	gtk_window_set_focus(GTK_WINDOW(c->window), c->entry);

	/* Don't forget the delete_event signal */
	g_signal_connect(G_OBJECT(c->window), "delete_event",
			   G_CALLBACK(convo_delete), c);
	
	/* More e_tag_data stuff */
	c->etag->statusbar = c->progress;
	strcpy(c->etag->buf, c->prog_buf);

	gtk_widget_show_all(c->window);

	// Stop the flashing if necessary
	if((c->user->NewMessages() > 0) && flash_events)
	{
		// Stop the flashing for this user
		nToFlash--;
		list<SFlash *>::iterator it;
		int x = 0;
		for(it = FlashList.begin(); it != FlashList.end(); it++)
		{
			x++;
			if((*it)->nUin == c->user->Uin())
			{
				gtk_tree_path_free((*it)->path);
				g_free(*it);
				FlashList.erase(it);
				break;
			}                
		}
	}

	// Clear the flash list
	else if((c->user->NewMessages() > 0) && !flash_events)
	{
		nToFlash = -1;
		FlashList.clear();
	}
}

gboolean key_press_convo(GtkWidget *entry, GdkEventKey *eventkey, gpointer data)
{
	struct conversation *c = (struct conversation *)data;
	guint state;

	state = eventkey->state;
	if(eventkey->keyval == GDK_Return)
	{
		if(!enter_sends && (state & GDK_SHIFT_MASK))
			convo_send(0, (gpointer)c);
		if(enter_sends && !(state & GDK_SHIFT_MASK) &&
		    !(state & GDK_CONTROL_MASK))
		{
			convo_send(0, (gpointer)c);
			return TRUE;
		}
	}

	return FALSE;
}

void convo_nick_timestamp(GtkWidget *text, const char *nick, time_t message_time, GdkColor *color)
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	bool addnl = (gtk_text_buffer_get_char_count(tb) > 0);
	
	// How about their alias and an optional timestamp?
	if (show_convo_timestamp)
	{
		char szTime[26];
		struct tm *_tm = localtime(&message_time);
		strftime(szTime, 26, timestamp_format, _tm);
		szTime[25] = '\0';

		char *szTempStamp = g_strdup_printf("[%s] ", szTime);
		GtkTextIter iter;
		gtk_text_buffer_get_end_iter(tb, &iter);
		if (addnl) {
			gtk_text_buffer_insert(tb, &iter, "\n", 1);
			addnl = false;
			gtk_text_buffer_get_end_iter(tb, &iter);
		}
		gtk_text_buffer_insert(tb, &iter, szTempStamp, -1);
		g_free(szTempStamp);
	}
	
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(tb, &iter);
	if (addnl) {
		gtk_text_buffer_insert(tb, &iter, "\n", 1);
		gtk_text_buffer_get_end_iter(tb, &iter);
	}
	GtkTextTag *color_tag = 
			gtk_text_buffer_create_tag(tb, NULL, "foreground-gdk", color, NULL);
	gtk_text_buffer_insert_with_tags(tb, &iter, nick, -1, color_tag, NULL);
	gtk_text_buffer_get_end_iter(tb, &iter);
	gtk_text_buffer_insert_with_tags(tb, &iter, " : ", 3, color_tag, NULL);
}

void convo_send(GtkWidget *widget, gpointer _c)
{
	struct conversation *c = (struct conversation *)_c;

	/* Set the 2 button widgets */
	if(GTK_WIDGET_IS_SENSITIVE(c->send))
		gtk_widget_set_sensitive(c->send, false);
	toggle_close_cancel(c, 2);

	gchar *buf;
	gboolean urgent = FALSE;

	GtkTextIter start, end;
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(c->entry));
	gtk_text_buffer_get_start_iter(tb, &start);
	gtk_text_buffer_get_end_iter(tb, &end);
	buf = gtk_text_buffer_get_text(tb, &start, &end, FALSE);
 	const gchar *message = buf;	
	if (c->for_user != 0)
		g_free(c->for_user);
	c->for_user = buf;

	c->user->SetSendServer(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server)));

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
	     ICQ_TCPxMSG_URGENT);
	}

	/* Send to contact list */
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_list)))
	{
	  c->etag->e_tag = icq_daemon->icqSendMessage(c->user->Uin(), message,
             (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server))),
             ICQ_TCPxMSG_LIST);
	}

	else /* Just send it normally */
	{
	  c->etag->e_tag = icq_daemon->icqSendMessage(c->user->Uin(), message,
             (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server))),
             ICQ_TCPxMSG_NORMAL);
	}

	guint id = gtk_statusbar_get_context_id(GTK_STATUSBAR(c->progress),
						"prog");
	gtk_statusbar_pop(GTK_STATUSBAR(c->progress), id);
	gtk_statusbar_push(GTK_STATUSBAR(c->progress), id, c->prog_buf);
  
	/* Take care of the etd buffer and add it to the slist */
	memcpy(c->etag->buf, c->prog_buf, 60);
	catcher = g_slist_append(catcher, c->etag);
}

void verify_convo_send(GtkWidget *widget, guint id, gchar *text,
		       struct conversation *c)
{
	gchar temp[60];
	strcpy(temp, text);
	g_strreverse(temp);

	if(strncmp(temp, " ...", 4) == 0)
		return;
	else
	{
		gtk_widget_set_sensitive(c->send, TRUE);
		toggle_close_cancel(c, 1);
	}
}

void convo_cancel(GtkWidget *widget, struct conversation *c)
{
	/* Set the buttons sensitivity accordingly */
	gtk_widget_set_sensitive(c->send, TRUE);
	toggle_close_cancel(c, 1);

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
	if(c == 0)
	{
		system_status_refresh();
		return;
	}

	CUserEvent *u_event = c->user->EventPop();

	/* Make sure we really have an event */
	if(u_event == 0)
	{
		return;
	}

	// Use theme colors if it is black on white
	bool bIgnoreBW = false;

	// Get the color that it was sent in if it's wanted
	if (recv_colors)
	{
		if (!c->clrBack)
		{
			c->clrBack = new GdkColor;
		}
		
		if (!c->clrFore)
		{
			c->clrFore = new GdkColor;
		}

		CICQColor *pIcqColor = u_event->Color();

		if (pIcqColor->Foreground() == 0x00000000 &&
		    pIcqColor->Background() == 0x00FFFFFF)
		{
			bIgnoreBW = true;
		}
		else
		{
			c->clrFore->red   = pIcqColor->ForeRed() * 257;
			c->clrFore->green = pIcqColor->ForeGreen() * 257;
			c->clrFore->blue  = pIcqColor->ForeBlue() * 257;
			c->clrFore->pixel = 255;
			c->clrBack->red   = pIcqColor->BackRed() * 257;
			c->clrBack->green = pIcqColor->BackGreen() * 257;
			c->clrBack->blue  = pIcqColor->BackBlue() * 257;
			c->clrBack->pixel = 255;
		}
	}
	else
	{
		if (c->clrFore)
		{
			delete c->clrFore;
			c->clrFore = 0;
		}

		if (c->clrBack)
		{
			delete c->clrBack;
			c->clrBack = 0;
		}
	}

	// How about their alias and an optional timestamp?
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(c->text));
	convo_nick_timestamp(c->text, c->user->GetAlias(), u_event->Time(), red);

	switch (u_event->SubCommand())
	{
	  case ICQ_CMDxSUB_MSG:
	  {
		GtkTextIter iter;
		gtk_text_buffer_get_end_iter(tb, &iter);
		if (!bIgnoreBW)
		{
			GtkTextTag *tag = gtk_text_buffer_create_tag(tb, NULL, 
					"foreground-gdk", c->clrFore,
					"background-gdk", c->clrBack,
					NULL);
			gtk_text_buffer_insert_with_tags(tb, &iter, u_event->Text(), -1,
					tag, NULL);
		}
		else
			gtk_text_buffer_insert(tb, &iter, u_event->Text(), -1);
		gtk_text_buffer_get_end_iter(tb, &iter);
		GtkTextMark *mark = gtk_text_buffer_create_mark(tb, NULL, &iter, TRUE);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(c->text), mark, 0, FALSE, 0, 0);
		break;
 	  }

	  case ICQ_CMDxSUB_URL:
	  {	
		const gchar *for_user_u =
		   g_strdup_printf("\n%s has sent you a URL!\n%s\n",
					c->user->GetAlias(),
					u_event->Text());

		GtkTextIter iter;
		gtk_text_buffer_get_end_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, for_user_u, -1);
		g_free(const_cast<char *>(for_user_u));
		break;
	  }

	  case ICQ_CMDxSUB_CHAT:
	  {	
		const gchar *chat_d = u_event->Text();

		if(u_event->IsCancelled())
		{
			GtkTextIter iter;
			gtk_text_buffer_get_end_iter(tb, &iter);
			gtk_text_buffer_insert(tb, &iter, chat_d, -1);
		}

		else
		{
			const gchar *for_user_c =
				g_strdup_printf("\n%s requests to chat with you!\n%s\n",
				c->user->GetAlias(), chat_d);
			GtkTextIter iter;
			gtk_text_buffer_get_end_iter(tb, &iter);
			gtk_text_buffer_insert(tb, &iter, for_user_c, -1);
	
			CEventChat *c_event = (CEventChat *)u_event;
			chat_accept_window(c_event, uin);
		}
		break;
	  }

	  case ICQ_CMDxSUB_FILE:
	  {
		const gchar *file_d = u_event->Text();
		
		if(u_event->IsCancelled())
		{
			GtkTextIter iter;
			gtk_text_buffer_get_end_iter(tb, &iter);
			gtk_text_buffer_insert(tb, &iter, file_d, -1);
		}

		else
		{
			const gchar *for_user_f =
		    	g_strdup_printf("\n%s requests to send you a file!\n%s\n",
				    c->user->GetAlias(), file_d);

			GtkTextIter iter;
			gtk_text_buffer_get_end_iter(tb, &iter);
			gtk_text_buffer_insert(tb, &iter, for_user_f, -1);
			file_accept_window(c->user, u_event);
		}
		break;
	  }

	  default: // Not good
		break;
	} // switch
}

gboolean convo_delete(GtkWidget *widget, GdkEvent *event, struct conversation *c)
{
	convo_close(0, c);
	return false;
}

void convo_close(GtkWidget *widget, struct conversation *c)
{
	if (c->clrBack)
		delete (c->clrBack);

	if (c->clrFore)
		delete (c->clrFore);

	cnv = g_slist_remove(cnv, c);
	catcher = g_slist_remove(catcher, c->etag);

	gtk_widget_destroy(c->window);

	if (c->for_user != 0)
		g_free(c->for_user);
	g_free(c->etag);
	g_free(c);
}
