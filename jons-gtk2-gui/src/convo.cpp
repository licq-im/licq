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
#include "utilities.h"

#include <string.h>
#include <time.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <list>
#include <iostream>
using namespace std; // for std::list

struct conversation
{
	GtkWidget *window;
	GtkWidget *entry;
	GtkWidget *text;
	GtkWidget *send;
	GtkWidget *close_or_cancel;
	GtkWidget *send_server;
	GtkWidget *send_urgent;
	GtkWidget *send_list;
	GtkWidget *charset;
	GdkColor *clrFore;
	GdkColor *clrBack;
	gchar *for_user;
	ICQUser *user;
	struct e_tag_data *etag;
};

list<conversation *> cnv;

/********** Structures ******************/
/* Functions in convo.cpp */
conversation *convo_find(unsigned long);
void convo_show(conversation *);
void convo_nick_timestamp(GtkWidget *, const char *, time_t, GdkColor *);
void convo_send(GtkWidget *, conversation *c);
gboolean key_press_convo(GtkWidget *, GdkEventKey *, gpointer);
void verify_convo_send(GtkWidget *, guint, gchar *, conversation *);
void convo_cancel(GtkWidget *, conversation *);
gint convo_delete(GtkWidget *, GdkEvent *, conversation *);
void convo_close(GtkWidget *, conversation *);
void convo_recv(gulong uin);

void convo_open_cb(ICQUser *user)
{
	convo_open(user, true);
}

void convo_open(ICQUser *user, bool refresh)
{
	conversation *c = convo_find(user->Uin());

	if(c != 0)
		gtk_window_present(GTK_WINDOW(c->window));
	else
	{
		c = g_new0(conversation, 1);

		c->window = 0;
		c->user = user;
		
		c->clrBack = new GdkColor;
		c->clrFore = new GdkColor;
		c->for_user = 0;

		cnv.push_back(c);
		convo_show(c);

		while (c->user->NewMessages() > 0)
			convo_recv(c->user->Uin());
		
		if (refresh)
			system_status_refresh();

		// Stop the flashing if necessary
		if (flash_events)
			stop_flashing(c->user);
	}

	if (c->user->Status() == ICQ_STATUS_OFFLINE)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->send_server), TRUE);
}

conversation *
convo_find(unsigned long uin)
{
	for (list<conversation *>::iterator i = cnv.begin(); i != cnv.end(); ++i)
		if ((*i)->user->Uin() == uin)
			return *i;

	return 0;
}

void
toggle_close_cancel(conversation *c, int which)
{
	if (which == 1)
		gtk_button_set_label(GTK_BUTTON(c->close_or_cancel), GTK_STOCK_CLOSE);
	else
		gtk_button_set_label(GTK_BUTTON(c->close_or_cancel), GTK_STOCK_CANCEL);
}

void convo_close_or_cancel(GtkWidget *widget, conversation *c)
{
	if (strcmp(gtk_button_get_label(GTK_BUTTON(widget)), GTK_STOCK_CANCEL) == 0)
		convo_cancel(widget, c);
	else
		convo_close(widget, c);
}

void
charset_select(GtkWidget *w, struct conversation *c)
{
	char *p =	(char *)g_object_get_data(G_OBJECT(w), "encoding");
	if (strcmp(p, c->user->UserEncoding()) == 0)
		return;
		
	unsigned long uin = c->user->Uin();
	ICQUser *u = gUserManager.FetchUser(uin, LOCK_W);
	u->SetUserEncoding(p);
	u->SaveLicqInfo();
	gUserManager.DropUser(u);
	c->user = gUserManager.FetchUser(uin, LOCK_R);
}

void
charset_popup(GtkWidget *w, struct conversation *c)
{
	/* Add the character set menu */
	GtkWidget *menu = gtk_menu_new();
	const char *user_enc = c->user->UserEncoding();
	if (user_enc == NULL || *user_enc == 0)
		user_enc = "UTF-8";
	for (encoding *ep = encodings; ep->name != NULL; ++ep) {
		char *lbl = g_strdup_printf("%s (%s)", ep->name, ep->enc);
		GtkWidget *item = gtk_check_menu_item_new_with_label(lbl);
		g_free(lbl);
		if (strcmp(user_enc, ep->enc) == 0)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
		g_object_set_data(G_OBJECT(item), "encoding", ep->enc);
		g_signal_connect(G_OBJECT(item), "activate",
				G_CALLBACK(charset_select), c);
	}
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), 0, 0, 0, 0, 0, gtk_get_current_event_time());
}
	
void convo_show(conversation *c)
{
	GtkWidget *scroll;
	GtkWidget *button_box;
	GtkWidget *options_box;
	
	/* Handle the etag stuff */
	c->etag = g_new0(struct e_tag_data, 1);

	/* Make the convo window */
	c->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(GTK_WINDOW(c->window), TRUE);

	/* Make the boxes */
	GtkWidget *vertical_box = gtk_vbox_new(FALSE, 0);

	/* The viewing messages box area */
	c->text = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(c->text), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(c->text), GTK_WRAP_WORD);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(c->text), FALSE);

	// Insert some tags we'll use later on
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(c->text));
	gtk_text_buffer_create_tag(tb, "local", "foreground-gdk", blue, NULL);
	gtk_text_buffer_create_tag(tb, "remote", "foreground-gdk", red, NULL);
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(tb, &iter);
	gtk_text_buffer_create_mark(tb, "last_pos", &iter, TRUE);

	/* Scrolled window for the messages that are being viewed */
	scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_ALWAYS);
	gtk_container_add(GTK_CONTAINER(scroll), c->text);
	gtk_widget_set_size_request(scroll, -1, 150);

	GtkWidget *frame = gtk_frame_new(0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN); 
	gtk_container_add(GTK_CONTAINER(frame), scroll);
	gtk_box_pack_start(GTK_BOX(vertical_box), frame, TRUE, TRUE, 5);
  
	/* The entry box */
	c->entry = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(c->entry), TRUE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(c->entry), GTK_WRAP_WORD);
	g_signal_connect(G_OBJECT(c->entry), "key_press_event",
			G_CALLBACK(key_press_convo), (gpointer)c);

	gtk_widget_set_size_request(c->entry, -1, 75); 

	frame = gtk_frame_new(0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN); 
	gtk_container_add(GTK_CONTAINER(frame), c->entry);
	gtk_box_pack_start(GTK_BOX(vertical_box), frame, FALSE, FALSE, 5);

	options_box = gtk_hbox_new(FALSE, 5);

	/* Add the send through server button */
	c->send_server = gtk_check_button_new_with_mnemonic("Se_nd through server");
	gtk_box_pack_start(GTK_BOX(options_box), c->send_server, FALSE, FALSE, 5);

	/* Send the message urgently */
	c->send_urgent = gtk_check_button_new_with_mnemonic("U_rgent");
	gtk_box_pack_start(GTK_BOX(options_box), c->send_urgent, FALSE, FALSE, 5);

	/* Send the message to contact list */
	c->send_list = gtk_check_button_new_with_mnemonic("M_ultiple recipients");
	gtk_box_pack_start(GTK_BOX(options_box), c->send_list, FALSE, FALSE, 5);
	
	/* Character set */
	GtkWidget *bbox = hbutton_box_new();
	c->charset = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(c->charset), 
			gtk_image_new_from_pixbuf(charset_icon));
	gtk_container_add(GTK_CONTAINER(bbox), c->charset);
	gtk_box_pack_end(GTK_BOX(options_box), bbox, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(c->charset), "clicked", 
			G_CALLBACK(charset_popup), c);

	/* Now pack the options_box */
	gtk_box_pack_start(GTK_BOX(vertical_box), options_box, FALSE, FALSE, 5);

	// Box with progress bar and the 2 buttons
	button_box = gtk_hbox_new(FALSE, 0);
	/* Progress of message */
	c->etag->statusbar = gtk_statusbar_new();
	gtk_widget_set_size_request(c->etag->statusbar, 300, -1); 
	
	/* Make new buttons with labels */
  GtkWidget *hbbox = hbutton_box_new();
	c->send = gtk_button_new_with_mnemonic("_Send");
	c->close_or_cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
  gtk_container_add(GTK_CONTAINER(hbbox), c->send);
  gtk_container_add(GTK_CONTAINER(hbbox), c->close_or_cancel);

	/* Get the signals connected for the buttons */
	g_signal_connect(G_OBJECT(c->close_or_cancel), "clicked",
			   G_CALLBACK(convo_close_or_cancel), c);
	g_signal_connect(G_OBJECT(c->send), "clicked",
			   G_CALLBACK(convo_send), c);
	
	gtk_box_pack_start(GTK_BOX(button_box), c->etag->statusbar, TRUE, TRUE, 5);
	gtk_box_pack_end(GTK_BOX(button_box), hbbox, FALSE, FALSE, 0);

	/* Take care of the rest of the widgets */
	gtk_box_pack_start(GTK_BOX(vertical_box), button_box, FALSE, FALSE, 5);

	/* If the user is in occupied or dnd mode, set the urgent button */
	if(c->user->Status() == ICQ_STATUS_DND || 
			c->user->Status() == ICQ_STATUS_OCCUPIED)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->send_urgent),	TRUE);

	/* Add the main box into the window */
	gtk_container_add(GTK_CONTAINER(c->window), vertical_box);
	gtk_container_set_border_width(GTK_CONTAINER(c->window), 10);

	/* Set the title of the window */
	gchar *win_title = g_strdup_printf("Conversation with %s", 
			s_convert_to_utf8(c->user->GetAlias(), c->user->UserEncoding()).c_str());
	gtk_window_set_title(GTK_WINDOW(c->window), win_title);
	g_free(win_title);
	
	/* Set the focus of the window */
	gtk_window_set_focus(GTK_WINDOW(c->window), c->entry);

	/* Don't forget the delete_event signal */
	g_signal_connect(G_OBJECT(c->window), "delete_event",
			G_CALLBACK(convo_delete), c);
	
	gtk_widget_show_all(c->window);
	gtk_button_set_label(GTK_BUTTON(c->close_or_cancel), GTK_STOCK_CLOSE);
}

gboolean key_press_convo(GtkWidget *entry, GdkEventKey *eventkey, gpointer data)
{
	if (eventkey->keyval == GDK_Return) {
		conversation *c = (conversation *)data;
		guint state = eventkey->state;
		
		// We send when:
		// - enter_sends is true and plain Enter was presses
		// - enter_sends is false and Shift/Ctrl+Enter was pressed
		if ((!enter_sends && (state & GDK_SHIFT_MASK)) ||
				(enter_sends && !(state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)))) {
			convo_send(0, c);
			return TRUE;
		}
	}

	return FALSE;
}

void convo_nick_timestamp(GtkWidget *text, const char *nick, 
		time_t message_time, const char *color)
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	bool addnl = (gtk_text_buffer_get_char_count(tb) > 0);
	GtkTextIter iter;
	
	// How about their alias and an optional timestamp?
	if (show_convo_timestamp) {
		char szTime[26];
		struct tm *_tm = localtime(&message_time);
		strftime(szTime, 26, timestamp_format, _tm);
		szTime[25] = '\0';

		char *szTempStamp = g_strdup_printf("[%s] ", szTime);
		gtk_text_buffer_get_end_iter(tb, &iter);
		if (addnl) {
			gtk_text_buffer_insert(tb, &iter, "\n", 1);
			addnl = false;
			gtk_text_buffer_get_end_iter(tb, &iter);
		}
		gtk_text_buffer_insert(tb, &iter, szTempStamp, -1);
		g_free(szTempStamp);
	}
	
	gtk_text_buffer_get_end_iter(tb, &iter);
	if (addnl) {
		gtk_text_buffer_insert(tb, &iter, "\n", 1);
		gtk_text_buffer_get_end_iter(tb, &iter);
	}

	gtk_text_buffer_insert_with_tags_by_name(tb, &iter, nick, -1, color, NULL);
	gtk_text_buffer_get_end_iter(tb, &iter);
	gtk_text_buffer_insert_with_tags_by_name(tb, &iter, ": ", 2, color, NULL);
}

void convo_send(GtkWidget *widget, conversation *c)
{
	/* Set the 2 button widgets */
	if (GTK_WIDGET_IS_SENSITIVE(c->send))
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
	if ((c->user->Status() == ICQ_STATUS_DND ||
	   c->user->Status() == ICQ_STATUS_OCCUPIED) &&
	   !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_urgent)))
		urgent = TRUE;

	strcpy(c->etag->buf, "Sending message ");

	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server)))
		strcat(c->etag->buf, "directly ... ");
	else
		strcat(c->etag->buf, "through server ... ");

	/* Send the message */
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_urgent)) ||
		  urgent)
	  c->etag->e_tag = icq_daemon->icqSendMessage(c->user->Uin(), message,
	     	(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server))),
	     	ICQ_TCPxMSG_URGENT);
	/* Send to contact list */
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_list)))
	  c->etag->e_tag = icq_daemon->icqSendMessage(c->user->Uin(), message,
				(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server))),
        ICQ_TCPxMSG_LIST);
	else /* Just send it normally */
	  c->etag->e_tag = icq_daemon->icqSendMessage(c->user->Uin(), message,
    		(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->send_server))),
        ICQ_TCPxMSG_NORMAL);

	/* Take care of the etd buffer and add it to the slist */
	status_change(c->etag->statusbar, "sta", c->etag->buf);
	catcher = g_slist_append(catcher, c->etag);
}

void convo_cancel(GtkWidget *widget, conversation *c)
{
	/* Set the buttons sensitivity accordingly */
	gtk_widget_set_sensitive(c->send, TRUE);
	toggle_close_cancel(c, 1);

	/* Actually cancel this event */
	icq_daemon->CancelEvent(c->etag->e_tag);

	/* Remove the event from the slist */
	catcher = g_slist_remove(catcher, c->etag);

	c->etag->buf[0] = '\0';
	status_change(c->etag->statusbar, "sta", c->etag->buf);
}

#include <iostream>
using namespace std;

void
scroll_to_the_end(GtkWidget *tv)
{
	GtkTextIter iter;
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
	gtk_text_buffer_get_end_iter(tb, &iter);
	GtkTextMark *mark = gtk_text_buffer_get_mark(tb, "last_pos");
	gtk_text_buffer_move_mark(tb, mark, &iter);
	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(tv), mark, 0, FALSE, 0, 0);
}

void convo_recv(gulong uin)
{
	conversation *c = convo_find(uin);

	/* If the window doesn't exist, don't show anything */
	if (c == 0) {
		system_status_refresh();
		return;
	}

	CUserEvent *u_event = c->user->EventPop();

	/* Make sure we really have an event */
	if (u_event == 0)
		return;

	// Use theme colors if it is black on white
	bool bIgnoreBW = false;

	// Get the color that it was sent in if it's wanted
	if (recv_colors) {
		if (!c->clrBack)
			c->clrBack = new GdkColor;
		if (!c->clrFore)
			c->clrFore = new GdkColor;

		CICQColor *pIcqColor = u_event->Color();

		if (pIcqColor->Foreground() == 0x00000000 &&
				pIcqColor->Background() == 0x00FFFFFF)
			bIgnoreBW = true;
		else {
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
	else {
		if (c->clrFore) {
			delete c->clrFore;
			c->clrFore = 0;
		}
		if (c->clrBack)	{
			delete c->clrBack;
			c->clrBack = 0;
		}
	}

	// How about their alias and an optional timestamp?
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(c->text));
	convo_nick_timestamp(c->text, 
			s_convert_to_utf8(c->user->GetAlias(), c->user->UserEncoding()).c_str(), 
			u_event->Time(), "remote");
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(tb, &iter);
	gchar *txt;
	
	switch (u_event->SubCommand()) {
	  case ICQ_CMDxSUB_MSG:
			txt = convert_to_utf8(u_event->Text(), c->user->UserEncoding());
			if (!bIgnoreBW)	{
				GtkTextTag *tag = gtk_text_buffer_create_tag(tb, NULL, 
						"foreground-gdk", c->clrFore,
						"background-gdk", c->clrBack,
						NULL);
				gtk_text_buffer_insert_with_tags(tb, &iter, txt, -1, tag, NULL);
			}
			else
				gtk_text_buffer_insert(tb, &iter, txt, -1);
			g_free(txt);
			break;

	  case ICQ_CMDxSUB_URL:
			txt = convert_to_utf8(u_event->Text(), c->user->UserEncoding());
			if (true) {
				gchar *for_user_u =
			  	 g_strdup_printf("\n%s has sent you a URL!\n%s\n",
							c->user->GetAlias(),
							txt);

				gtk_text_buffer_insert(tb, &iter, for_user_u, -1);
				g_free(for_user_u);
			}
			g_free(txt);
			break;

	  case ICQ_CMDxSUB_CHAT:
			txt = convert_to_utf8(u_event->Text(), c->user->UserEncoding());

			if(u_event->IsCancelled())
				gtk_text_buffer_insert(tb, &iter, txt, -1);
			else
			{
				gchar *for_user_c =
					g_strdup_printf("\n%s requests to chat with you!\n%s\n",
					c->user->GetAlias(), txt);
				gtk_text_buffer_insert(tb, &iter, for_user_c, -1);

				CEventChat *c_event = (CEventChat *)u_event;
				chat_accept_window(c_event, uin);
				g_free(for_user_c);
			}
			g_free(txt);
			break;

	  case ICQ_CMDxSUB_FILE:
			if(u_event->IsCancelled())
				gtk_text_buffer_insert(tb, &iter, u_event->Text(), -1);
			else
			{
				gchar *for_user_f =
		    		g_strdup_printf("\n%s requests to send you a file!\n%s\n",
				    	c->user->GetAlias(), u_event->Text());

				gtk_text_buffer_insert(tb, &iter, for_user_f, -1);
				file_accept_window(c->user, u_event);
				g_free(for_user_f);
			}
			break;

	  default: // Not good
			break;
	} // switch

	scroll_to_the_end(c->text);
}

gboolean convo_delete(GtkWidget *widget, GdkEvent *event, conversation *c)
{
	convo_close(0, c);
	return false;
}

void convo_close(GtkWidget *widget, conversation *c)
{
	if (c->clrBack)
		delete c->clrBack;
	if (c->clrFore)
		delete c->clrFore;

	cnv.remove(c);
	catcher = g_slist_remove(catcher, c->etag);

	gtk_widget_destroy(c->window);

	if (c->for_user != 0)
		g_free(c->for_user);
	g_free(c->etag);
	g_free(c);
}

void finish_message(ICQEvent *event)
{
	conversation *c = convo_find(event->Uin());

	/* If the window isn't open, there isn't anything left to do */
	if (c == 0)
		return;

	/* Check to make sure it sent, and if it did, put the text in */
	if (event->Result() == EVENT_ACKED || event->Result() == EVENT_SUCCESS) {
		ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
		const gchar *name = owner->GetAlias();
		gUserManager.DropOwner();

		GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(c->entry));
		gtk_text_buffer_set_text(tb, "", -1);
		gtk_window_set_focus(GTK_WINDOW(c->window), c->entry);
		
		convo_nick_timestamp(c->text, name, time(NULL), "local");
		tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(c->text));
		GtkTextIter iter;
		gtk_text_buffer_get_end_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, c->for_user, -1);
		scroll_to_the_end(c->text);
	}

	gtk_widget_set_sensitive(c->send, TRUE);
	toggle_close_cancel(c, 1);
}

