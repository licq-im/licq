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

using namespace std;

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

void convo_show(struct conversation *c)
{
	GtkWidget *scroll;
	GtkWidget *close;
	GtkWidget *button_box;
	GtkWidget *options_box;
	GtkWidget *vertical_box;
	
	/* Handle the etag stuff */
	c->etag = g_new0(struct e_tag_data, 1);

	/* Make the convo window */
	c->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_policy(GTK_WINDOW(c->window), TRUE, TRUE, TRUE);
	gtk_widget_realize(c->window);

	/* Make new buttons with labels */
	c->send = gtk_button_new_with_label("Send");
	c->cancel = gtk_button_new_with_label("Cancel");
	close = gtk_button_new_with_label("Close");

	/* Set cancel to grayed out at first */
	//gtk_widget_set_sensitive(c->cancel, FALSE);

	/* Make the boxes */
	button_box = gtk_hbox_new(TRUE, 0);
	vertical_box = gtk_vbox_new(FALSE, 0);
	options_box = gtk_hbox_new(FALSE, 5);

	/* The entry box */
	c->entry = gtk_text_new(0, 0);
	gtk_text_set_editable(GTK_TEXT(c->entry), TRUE);
	gtk_signal_connect(GTK_OBJECT(c->entry), "key_press_event",
			  GTK_SIGNAL_FUNC(key_press_convo), (gpointer)c);

	gtk_widget_set_usize(c->entry, 320, 75); 

	/* The viewing messages box area */
	c->text = gtk_text_new(0, 0);
	gtk_text_set_editable(GTK_TEXT(c->text), FALSE);
	gtk_text_set_word_wrap(GTK_TEXT(c->text), TRUE);
	gtk_text_set_line_wrap(GTK_TEXT(c->text), TRUE);

	/* Scroll bar for the messages that are being viewed */
	scroll = gtk_scrolled_window_new(0, GTK_TEXT(c->text)->vadj);
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
			   GTK_SIGNAL_FUNC(convo_send), (gpointer)c);

	gtk_box_pack_start(GTK_BOX(button_box), close, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(button_box), c->cancel, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(button_box), c->send, TRUE, TRUE, 5);

	/* Take care of the rest of the widgets */
	gtk_box_pack_start(GTK_BOX(vertical_box), scroll, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(vertical_box), c->entry, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vertical_box), button_box, FALSE, FALSE, 5);

	/* Add the send through server button */
	c->send_server = gtk_check_button_new_with_label("Send through server");

	/* Send the message normal */
	c->send_normal = gtk_radio_button_new_with_label(0, "Normal");

	/* Send the message urgently */
	c->send_urgent = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(c->send_normal),
				"Urgent");

	/* Send the message to contact list */
	c->send_list = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(c->send_normal),
				"To Contact List");

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

	/* Don't forget the delete_event signal */
	gtk_signal_connect(GTK_OBJECT(c->window), "delete_event",
			   GTK_SIGNAL_FUNC(convo_delete), c);

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
                                g_free(*it);
                                FlashList.erase(it);
				break;
                        }                
                }

		for(it = FlashList.begin(); it != FlashList.end(); it++)
		{
			x--;
			if(x <= 0)
				(*it)->nRow--;
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
		if (enter_sends && !(state & GDK_SHIFT_MASK) &&
		    !(state & GDK_CONTROL_MASK))
		{
			gtk_signal_emit_stop_by_name(GTK_OBJECT(entry),
					"key_press_event");
			convo_send(0, (gpointer)c);
		}
		else if(enter_sends)
		{
			gtk_signal_emit_stop_by_name(GTK_OBJECT(entry),
						     "key_press_event");
			int pos = gtk_editable_get_position(GTK_EDITABLE(entry));
			gtk_editable_insert_text(GTK_EDITABLE(entry), "\n", 1,
						 &pos);
		}
	}

	return TRUE;
}

void convo_nick_timestamp(GtkWidget *text, const char *nick, time_t message_time, GdkColor *color)
{
	// How about their alias and an optional timestamp?
	if (show_convo_timestamp)
	{
		char szTime[26];
		struct tm *_tm = localtime(&message_time);
		strftime(szTime, 26, timestamp_format, _tm);
		szTime[25] = '\0';

		char *szTempStamp = g_strdup_printf("[%s] ", szTime);
		gtk_text_insert(GTK_TEXT(text), 0, NULL, 0, szTempStamp, -1);
		g_free(szTempStamp);
	}
	
	gtk_text_insert(GTK_TEXT(text), 0, color, 0, nick, -1);
	gtk_text_insert(GTK_TEXT(text), 0, color, 0, " : ", -1);
}

void convo_send(GtkWidget *widget, gpointer _c)
{
	struct conversation *c = (struct conversation *)_c;

	/* Set the 2 button widgets */
	if(GTK_WIDGET_IS_SENSITIVE(c->send))
		gtk_widget_set_sensitive(c->send, false);
	gtk_widget_set_sensitive(c->cancel, true);

	gchar *buf;
	gchar *buf2;
	gboolean urgent = FALSE;

	buf =
	  g_strdup_printf("%s", gtk_editable_get_chars(GTK_EDITABLE(c->entry), 0, -1));
 	const gchar *message = buf;	
	buf2 = g_strdup_printf(":  %s\n", buf);
	c->for_user = buf2;

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
	gtk_text_freeze(GTK_TEXT(c->text));
	convo_nick_timestamp(c->text, c->user->GetAlias(), u_event->Time(), red);

	switch (u_event->SubCommand())
	{
	  case ICQ_CMDxSUB_MSG:
	  {
		gtk_text_insert(GTK_TEXT(c->text), 0,
				bIgnoreBW ? 0 : c->clrFore,
				bIgnoreBW ? 0 : c->clrBack,
		                u_event->Text(), -1);
		gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, "\n", -1);
		gtk_text_thaw(GTK_TEXT(c->text));
		gtk_adjustment_set_value(GTK_TEXT(c->text)->vadj,
			 GTK_ADJUSTMENT(GTK_TEXT(c->text)->vadj)->upper);
		break;
 	  }

	  case ICQ_CMDxSUB_URL:
	  {	
		const gchar *for_user_u =
		   g_strdup_printf("\n%s has sent you a URL!\n%s\n",
					c->user->GetAlias(),
					u_event->Text());

		gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, for_user_u, -1);
		gtk_text_thaw(GTK_TEXT(c->text));
		g_free(const_cast<char *>(for_user_u));
		break;
	  }

	  case ICQ_CMDxSUB_CHAT:
	  {	
		const gchar *chat_d = u_event->Text();

		if(u_event->IsCancelled())
		{
			gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, chat_d, -1);
			gtk_text_thaw(GTK_TEXT(c->text));
		}

		else
		{
			const gchar *for_user_c =
				g_strdup_printf("\n%s requests to chat with you!\n%s\n",
				c->user->GetAlias(), chat_d);
			gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, for_user_c,
				-1);
			gtk_text_thaw(GTK_TEXT(c->text));
	
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
			gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, file_d, -1);
			gtk_text_thaw(GTK_TEXT(c->text));
		}

		else
		{
			const gchar *for_user_f =
		    	g_strdup_printf("\n%s requests to send you a file!\n%s\n",
				    c->user->GetAlias(), file_d);

			gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, for_user_f, -1);
			gtk_text_thaw(GTK_TEXT(c->text));
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
	{
		delete (c->clrBack);
	}
	if (c->clrFore)
	{
		delete (c->clrFore);
	}

	cnv = g_slist_remove(cnv, c);
	catcher = g_slist_remove(catcher, c->etag);

	gtk_widget_destroy(c->window);

	g_free(c->etag);
	g_free(c);
}
