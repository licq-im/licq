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
#include "licq_chat.h"
#include "licq_log.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <list>

struct request_chat
{
  GtkWidget *window;
  GtkWidget *text_box;
  GtkWidget *send_norm;
  GtkWidget *send_urg;
  GtkWidget *send_list;
  GtkWidget *chat_list;
  ICQUser *user;
  struct e_tag_data *etd;
};

struct remote_chat_request
{
  gulong uin;
  CEventChat *c_event;
};

struct kick_window
{
  GtkWidget *winKick;
  GtkWidget *cmbUsers;
  GtkWidget *btnKick;
};

struct chat_window
{
  // Chat manager stuff
  CChatManager *chatman;
  CChatUser *chat_user;
  CChatUser *hold_cuser;
  std::list<CChatUser *> ChatUsers;

  // Kick and tally window
  struct kick_window *kw;

  // UI
  GtkWidget *window;
  GtkWidget *notebook;
  GtkWidget *table;
  GtkWidget *table_irc;
  GtkWidget *text_local;
  GtkWidget *text_remote;
  GtkWidget *text_irc;
  GtkWidget *entry_irc;
  GtkWidget *list_users;
  GtkWidget *frame_local;
  GtkWidget *frame_remote;
  GtkWidget *font_sel_dlg;

  // Remote
  GtkTextTag *r_tag, *r_tag_irc;

  // Local
  GtkTextTag *l_tag, *l_tag_irc;
  
  // Extra - but important!
  gboolean pane_mode;
  gboolean audio;
  guint input_tag;
	char *local_name;
};

GSList *rc_list;

#include <iostream>
using namespace std; // for list

void
scroll_to_the_end(GtkWidget *);

typedef list<chat_window *> ChatDlgList;
ChatDlgList chat_list;

void chat_join_multiparty(struct remote_chat_request *);
struct chat_window *chat_window_create(gulong);
GtkWidget* chat_create_menu(struct chat_window *);
void start_kick_window(struct chat_window *);
unsigned long start_kick_callback(struct chat_window *);
gboolean chat_send(GtkWidget *, GdkEventKey *, struct chat_window *);
void ok_request_chat(GtkWidget *widget, struct request_chat *rc);
void chat_refuse(GtkWidget *, gpointer);
void chat_start_as_server(gulong, CEventChat *);
void chat_start_as_client(ICQEvent *);
void chat_audio(gpointer, guint, GtkWidget *);

void chat_kick(struct chat_window *, guint, GtkWidget *);
void chat_kick_no_vote(struct chat_window *, guint, GtkWidget *);
void kick_callback(GtkWidget *, gpointer);
void kick_no_vote_callback(GtkWidget *, gpointer);

void chat_save(gpointer, guint, GtkWidget *);
void save_chat_ok(GtkWidget *, gpointer);
void save_chat_cancel(GtkWidget *, gpointer);

void chat_close(struct chat_window *, guint, GtkWidget *);
void chat_pipe_callback(struct chat_window *cw, gint, GdkInputCondition);
void chat_beep_users(struct chat_window *, guint, GtkWidget *);
void chat_change_font(struct chat_window *, guint, GtkWidget *);
//void font_dlg_close(GtkWidget *, gpointer);
void font_dlg_ok(GtkWidget *, struct chat_window *cw);

void
set_tag_colors(GtkTextTag *tag, int *color_fg, int *color_bg)
{
  char fg[8], bg[8];
  snprintf(fg, 8, "#%02x%02x%02x", color_fg[0], color_fg[1], color_fg[2]);
  snprintf(bg, 8, "#%02x%02x%02x", color_bg[0], color_bg[1], color_bg[2]);
  g_object_set(tag, "foreground", fg, "background", bg, NULL);
}  
        
void
set_tag_fg_color(GtkTextTag *tag, int *color_fg)
{
  char fg[8];
  snprintf(fg, 8, "#%02x%02x%02x", color_fg[0], color_fg[1], color_fg[2]);
  g_object_set(tag, "foreground", fg, NULL);
}  

void
set_tag_bg_color(GtkTextTag *tag, int *color_bg)
{
  char bg[8];
  snprintf(bg, 8, "#%02x%02x%02x", color_bg[0], color_bg[1], color_bg[2]);
  g_object_set(tag, "background", bg, NULL);
}  

void
set_tag_font(GtkTextTag *tag, const char *font_name, int size, PangoStyle style)
{
  PangoFontDescription *font;
  g_object_get(tag, "font-desc", &font, NULL);
  if (font == NULL)
    font = pango_font_description_new();

  pango_font_description_set_family(font, font_name);
  pango_font_description_set_size(font, PANGO_SCALE * size);
  pango_font_description_set_style(font, style);

  g_object_set(tag, "font-desc", font, NULL);
}  
        
void
set_tag_font_family(GtkTextTag *tag, const char *font_name)
{
  PangoFontDescription *font;
  g_object_get(tag, "font-desc", &font, NULL);
  if (font == NULL)
    font = pango_font_description_new();
  pango_font_description_set_family(font, font_name);
  g_object_set(tag, "font-desc", font, NULL);
}  

void
set_tag_font_size(GtkTextTag *tag, int size)
{
  PangoFontDescription *font;
  g_object_get(tag, "font-desc", &font, NULL);
  if (font == NULL)
    font = pango_font_description_new();

  pango_font_description_set_size(font, PANGO_SCALE * size);

  g_object_set(tag, "font-desc", font, NULL);
}  

void
set_tag_font_style(GtkTextTag *tag, PangoStyle style)
{
  PangoFontDescription *font;
  g_object_get(tag, "font-desc", &font, NULL);
  if (font == NULL)
    font = pango_font_description_new();

  pango_font_description_set_style(font, style);

  g_object_set(tag, "font-desc", font, NULL);
}  

void
set_tag_font_weight(GtkTextTag *tag, PangoWeight weight)
{
  PangoFontDescription *font;
  g_object_get(tag, "font-desc", &font, NULL);
  if (font == NULL)
    font = pango_font_description_new();

  pango_font_description_set_weight(font, weight);

  g_object_set(tag, "font-desc", font, NULL);
}  

void
close_request_chat(struct request_chat *rc)
{
  rc_list = g_slist_remove(rc_list, rc);
	g_free(rc);
}

void
cancel_request_chat(GtkWidget *widget, struct request_chat *rc)
{
  icq_daemon->CancelEvent(rc->etd->e_tag);
  catcher = g_slist_remove(catcher, rc->etd);
  close_request_chat(rc);
}

struct request_chat *
rc_find(gulong uin)
{
  GSList *temp_rc_list = rc_list;

  while (temp_rc_list) {
    struct request_chat *rc = (struct request_chat *)temp_rc_list->data;
    if (rc->user->Uin() == uin)
      return rc;

    temp_rc_list = temp_rc_list->next;
  }

  /* It wasn't found, return null */
  return NULL;
}

struct request_chat *
rc_new(ICQUser *user)
{
  /* Does it already exist? */
  struct request_chat *rc = rc_find(user->Uin());

  /* Don't make it if it's already there, return it */
  if (rc != 0)
    return rc;

  rc = g_new0(struct request_chat, 1);
  rc->user = user;
  rc_list = g_slist_append(rc_list, rc);

  return rc;
}

void 
single_request_chat(GtkWidget *widget, struct request_chat *rc)
{
  // Just take care of it.. I think it'll take some more time
  // to check if we should be here or not.
  gtk_widget_set_sensitive(rc->chat_list, false);
}

void 
multi_request_chat(GtkWidget *widget, struct request_chat *rc)
{
  // Save some time by checking to see if we should be here
  if (GTK_WIDGET_SENSITIVE(rc->chat_list))
    return;

  // Make a GList with the current chats in it
  GList *items = NULL;

  ChatDlgList::iterator iter;
  for (iter = chat_list.begin(); iter != chat_list.end(); ++iter)
    items = g_list_append(items, (*iter)->chatman->ClientsStr());

  // Make sure there are chats to show
  if (items == NULL)
    return;
    
  gtk_combo_set_popdown_strings(GTK_COMBO(rc->chat_list), items);

  // Allright show it
  gtk_widget_set_sensitive(rc->chat_list, true);
}

void
list_request_chat(GtkWidget *widget, ICQUser *user)
{
  /* Do we even want to be here? */
  struct request_chat *rc = rc_find(user->Uin());

  // No get outta here, bitch!
  if (rc != 0) {
		gtk_window_present(GTK_WINDOW(rc->window));
    return;
	}

  rc = rc_new(user);

  /* Make the request_chat structure */
  rc->etd = g_new0(struct e_tag_data, 1);
  rc->user = user;

  /* Create the window */
  gchar *title = 
			g_strdup_printf("Licq - Request Chat With %s",
      		s_convert_to_utf8(user->GetAlias(), user->UserEncoding()).c_str());

  rc->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(rc->window), title);
	gtk_container_set_border_width(GTK_CONTAINER(rc->window), 10);
  g_free(title);

  gtk_window_set_position(GTK_WINDOW(rc->window), GTK_WIN_POS_CENTER);
  g_signal_connect(G_OBJECT(rc->window), "destroy",
			G_CALLBACK(cancel_request_chat), rc);

  /* Create the table and hbox */
  GtkWidget *v_box = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(rc->window), v_box);

  /* Create the scrolled window with text and add it into the window */
  GtkWidget *scroll = gtk_scrolled_window_new(0, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
      GTK_POLICY_NEVER,
      GTK_POLICY_AUTOMATIC);
  rc->text_box = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(rc->text_box), TRUE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(rc->text_box), GTK_WRAP_WORD);
  gtk_container_add(GTK_CONTAINER(scroll), rc->text_box);
	GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_add(GTK_CONTAINER(frame), scroll);
  gtk_widget_show_all(frame);
	gtk_box_pack_start(GTK_BOX(v_box), frame, FALSE, FALSE, 0);

  /* The send as buttons */
  rc->send_norm = 
			gtk_radio_button_new_with_mnemonic(0, "Send _Normal");
  rc->send_urg = 
      gtk_radio_button_new_with_mnemonic_from_widget(
          GTK_RADIO_BUTTON(rc->send_norm), "Send _Urgent");
  rc->send_list = 
      gtk_radio_button_new_with_mnemonic_from_widget(
          GTK_RADIO_BUTTON(rc->send_norm), "Send To _List");

  /* Pack them to a box */
  GtkWidget *h_box = gtk_hbox_new(FALSE, 5);
  gtk_box_pack_start(GTK_BOX(h_box), rc->send_norm, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(h_box), rc->send_urg, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(h_box), rc->send_list, FALSE, FALSE, 0);

  /* Attach the box to the table */
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);
  
  /* Progress bar */
  rc->etd->statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(v_box), rc->etd->statusbar, FALSE, FALSE, 0);

  // Ok.. this is where the combo box for multi party chats goes
  // but it isn't shown until it's needed
  h_box = gtk_hbox_new(FALSE, 5);
  GtkWidget *label = gtk_label_new("Current Chats:");
  rc->chat_list = gtk_combo_new();
  gtk_box_pack_start(GTK_BOX(h_box), label, false, false, 0);
  gtk_box_pack_start(GTK_BOX(h_box), rc->chat_list, false, false, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);

  /* The button box */
  h_box = hbutton_box_new();
  GtkWidget *multiparty = gtk_button_new_with_mnemonic("_Multi-Party");
  GtkWidget *single = gtk_button_new_with_mnemonic("_Single-Party");
  GtkWidget *ok = gtk_button_new_with_mnemonic("_Invite");
  GtkWidget *cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
  gtk_container_add(GTK_CONTAINER(h_box), multiparty);
  gtk_container_add(GTK_CONTAINER(h_box), single);
  gtk_container_add(GTK_CONTAINER(h_box), ok);
  gtk_container_add(GTK_CONTAINER(h_box), cancel);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);
  
  /* Connect the signals */
  g_signal_connect(G_OBJECT(multiparty), "clicked",
         G_CALLBACK(multi_request_chat), rc);
  g_signal_connect(G_OBJECT(single), "clicked",
         G_CALLBACK(single_request_chat), rc);
  g_signal_connect(G_OBJECT(ok), "clicked",
         G_CALLBACK(ok_request_chat), rc);
  g_signal_connect(G_OBJECT(cancel), "clicked",
         G_CALLBACK(window_close), rc->window);
  
  // Set chat list sensitive till needed and show everything
  gtk_widget_set_sensitive(rc->chat_list, false);
  gtk_widget_show_all(rc->window);
}

void
ok_request_chat(GtkWidget *widget, struct request_chat *rc)
{
  guint send_as;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rc->send_urg)))
    send_as = ICQ_TCPxMSG_URGENT;
  else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rc->send_list)))
    send_as = ICQ_TCPxMSG_LIST;
  else
    send_as = ICQ_TCPxMSG_NORMAL;
  
  strcpy(rc->etd->buf, "Requesting Chat ... ");
  status_change(rc->etd->statusbar, "sta", rc->etd->buf);

  string txt(textview_get_chars(rc->text_box));

  // Are we doing single or multi?
  if (GTK_WIDGET_SENSITIVE(rc->chat_list)) {
    // Multi
    unsigned short nPort = 0;

    // Find the chat
		string client_str(entry_get_chars(GTK_COMBO(rc->chat_list)->entry));
    ChatDlgList::iterator iter;
    for (iter = chat_list.begin(); iter != chat_list.end(); ++iter)
      if (strcmp((*iter)->chatman->ClientsStr(), client_str.c_str()) == 0) {
        nPort = (*iter)->chatman->LocalPort();
        break;
      }

    if (iter == chat_list.end()) {
      status_change(rc->etd->statusbar, "sta", 
					"Requesting Chat ... Invalid Chat");
      message_box("Invalid Multi-Party Chat");
      return;
    }

    rc->etd->e_tag = icq_daemon->icqMultiPartyChatRequest(
        rc->user->Uin(), 
        txt.c_str(),
        client_str.c_str(),
        nPort,
        send_as, 
        false);
  }
  else
    // Single
    rc->etd->e_tag = icq_daemon->icqChatRequest(
        rc->user->Uin(),
        txt.c_str(),
        send_as, 
        false);

  /* The event catcher list */
  catcher = g_slist_append(catcher, rc->etd);
}

void
chat_accept_cb(GtkWidget *dialog, gint response_id,
		struct remote_chat_request *r_cr)
{
	if (response_id == GTK_RESPONSE_ACCEPT) {
  	/* Join a multiparty chat (we connect to them) */
  	if (r_cr->c_event->Port() != 0)
    	chat_join_multiparty(r_cr);
  	/* Single party chat (they connect to us) */
  	else
    	chat_start_as_server(r_cr->uin, r_cr->c_event);
	}
	else {
  	/* Refuse the chat */
  	icq_daemon->icqChatRequestRefuse(r_cr->uin, "",
          	 r_cr->c_event->Sequence(),
          	 r_cr->c_event->MessageID(),
          	 r_cr->c_event->IsDirect());
	}

  /* Close the request window */
 	if (dialog != NULL)
	  gtk_widget_destroy(dialog);
}

void
chat_accept_window(CEventChat *c_event, gulong uin, bool auto_accept)
{
  struct remote_chat_request *r_cr = g_new0(struct remote_chat_request, 1);

  /* Fill in the structure */
  r_cr->uin = uin;
  r_cr->c_event = c_event;

  if (auto_accept) {
    chat_accept_cb(0, GTK_RESPONSE_ACCEPT, r_cr);
    return;
  }
  
  /* Make the dialog window */
  GtkWidget *dialog = gtk_dialog_new_with_buttons(
			"Accept Chat",
			GTK_WINDOW(main_window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			"_Accept", GTK_RESPONSE_ACCEPT,
			"_Refuse", GTK_RESPONSE_REJECT,
			NULL);

  /* Get the label for the window with the chat reason */
  ICQUser *u = gUserManager.FetchUser(uin, LOCK_R);
  string alias(s_convert_to_utf8(u->GetAlias(), u->UserEncoding()));
	string txt(s_convert_to_utf8(c_event->Text(), u->UserEncoding()));
  gUserManager.DropUser(u);

  gchar *forLabel = g_strdup_printf("Chat with %s (%ld)\nReason:\n%s",
    	alias.c_str(), uin, txt.c_str());
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), 5);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
			gtk_label_new(forLabel));
  g_free(forLabel);

  /* Connect all the signals */
  g_signal_connect(G_OBJECT(dialog), "response",
      G_CALLBACK(chat_accept_cb), r_cr);
  
  /* Show everything */
  gtk_widget_show_all(dialog);
}

void
chat_join_multiparty(struct remote_chat_request *r_cr)
{
  /* Make the window and the chat manager */
  struct chat_window *cw = chat_window_create(r_cr->uin);

  cw->chatman->StartAsClient(r_cr->c_event->Port());

  icq_daemon->icqChatRequestAccept(r_cr->uin, cw->chatman->LocalPort(),
           r_cr->c_event->Clients(),
           r_cr->c_event->Sequence(),
           r_cr->c_event->MessageID(),
           r_cr->c_event->IsDirect());
}

void
chat_start_as_server(gulong uin, CEventChat *c)
{
  /* Make the window and the chat manager */
  struct chat_window *cw = chat_window_create(uin);
  
  if (!cw->chatman->StartAsServer())
    return;

  icq_daemon->icqChatRequestAccept(uin, cw->chatman->LocalPort(),
           c->Clients(),
           c->Sequence(),
           c->MessageID(), c->IsDirect());
}

void
chat_start_as_client(ICQEvent *event)
{
  CExtendedAck *ea = event->ExtendedAck();
  
  if (ea == 0) {
    gLog.Error("%sInternal error: chat_start_as_client(): chat request acknowledgement without extended result.\n", L_ERRORxSTR);
    return;
  }

  /* Refused */
  if (!ea->Accepted())
    return;

  /* Make the window and the chat manager */
  struct chat_window *cw = chat_window_create(event->Uin());

  cw->chatman->StartAsClient(ea->Port());

  gtk_frame_set_label(GTK_FRAME(cw->frame_remote),
			"Remote - Waiting for joiners...");
  gLog.Info("%sChat: Waiting for joiners.\n", L_TCPxSTR);
}

bool
chat_close_base(chat_window *cw)
{
  if (cw->hold_cuser == 0) {
    cw->chat_user = 0;
    gtk_input_remove(cw->input_tag);
    cw->chatman->CloseChat();
  }
  else {
	  /* Remove the user's name from the list box */
    if (cw->chat_user == cw->hold_cuser)
      cw->chat_user = 0;
  }
  // We are unwanted now
  if (cw->chatman->ConnectedUsers() == 0)
    return true;

  return false;
}

gboolean
call_chat_close(GtkWidget *, GdkEvent *, struct chat_window *cw)
{
  if (chat_close_base(cw))
    return FALSE;
  return TRUE;
}

void
last_pos_mark_add(GtkWidget *tv)
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(tv));
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(tb, &iter);
	gtk_text_buffer_create_mark(tb, "last_pos", &iter, TRUE);
}

string
cchatuser_utf8(const char *txt, CChatUser *user)
{
  ICQUser *u = gUserManager.FetchUser(user->Uin(), LOCK_R);
	string utf8_txt(s_convert_to_utf8(txt, u->UserEncoding()));
  gUserManager.DropUser(u);
	return utf8_txt;
}

struct chat_window *
chat_window_create(gulong uin)
{
  struct chat_window *cw = g_new0(struct chat_window, 1);

  // Add to the chat list
  chat_list.push_back(cw);

  /* Take care of the chat manager stuff */
  cw->chatman = new CChatManager(icq_daemon, uin);
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  cw->local_name = convert_to_utf8(cw->chatman->Name(), o->UserEncoding());
  gUserManager.DropUser(o);
  cw->input_tag = gtk_input_add_full(cw->chatman->Pipe(), GDK_INPUT_READ,
			(GdkInputFunction)chat_pipe_callback, NULL, cw, NULL);

  /* Make the window */
  cw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(cw->window), "Licq - Chat");
  gtk_window_set_position(GTK_WINDOW(cw->window), GTK_WIN_POS_CENTER);

  /* Create the v_box */
  GtkWidget *main_vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(cw->window), main_vbox);

  /* Create the menu and add it to the main vbox */
  GtkWidget *menu_bar = chat_create_menu(cw);
  gtk_box_pack_start(GTK_BOX(main_vbox), menu_bar, FALSE, FALSE, 0);

  /* Create the notebook and add it to the main vbox */
  cw->notebook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX(main_vbox), cw->notebook, FALSE, FALSE, 0);
  
  /* Create the vbox */
  GtkWidget *vbox = gtk_vbox_new(FALSE, 5);

  /* Create the remote frame with the remote scrolled text window */
  cw->frame_remote = gtk_frame_new("Remote");
  cw->text_remote = gtk_text_view_new();
	last_pos_mark_add(cw->text_remote);
  GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cw->text_remote));
  cw->r_tag = gtk_text_buffer_create_tag(tb, NULL, NULL);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(cw->text_remote), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(cw->text_remote), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cw->text_remote), GTK_WRAP_WORD);
  GtkWidget *scroll = gtk_scrolled_window_new(0, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scroll, 320, 75);
  gtk_container_add(GTK_CONTAINER(scroll), cw->text_remote);
  gtk_container_add(GTK_CONTAINER(cw->frame_remote), scroll);
  gtk_box_pack_start(GTK_BOX(vbox), cw->frame_remote, TRUE, TRUE, 0);

  /* Create the local frame with local text box */
  gchar *forLFrame = g_strdup_printf("Local - %s", cw->local_name);
  cw->frame_local = gtk_frame_new(forLFrame);
  g_free(forLFrame);

  cw->text_local = gtk_text_view_new();
	last_pos_mark_add(cw->text_local);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(cw->text_local), TRUE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cw->text_local), GTK_WRAP_WORD);
  scroll = gtk_scrolled_window_new(0, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scroll, 320, 75);
  gtk_container_add(GTK_CONTAINER(scroll), cw->text_local);
  gtk_container_add(GTK_CONTAINER(cw->frame_local), scroll);
  gtk_box_pack_start(GTK_BOX(vbox), cw->frame_local, TRUE, TRUE, 0);

  // Ensure that when switching tabs cursor will end up in the right widget
	GList *focus_chain = NULL;
	focus_chain = g_list_append(focus_chain, cw->text_local);
	gtk_container_set_focus_chain(GTK_CONTAINER(vbox), focus_chain);
	
	gtk_notebook_append_page(GTK_NOTEBOOK(cw->notebook),
			vbox, gtk_label_new("Pane Mode"));

  /* The IRC Mode window with user list window */

  /* First thing is the table */
  cw->table_irc = gtk_table_new(2, 2, FALSE);

  /* Create the entry box */
  cw->entry_irc = gtk_entry_new();
  gtk_table_attach(GTK_TABLE(cw->table_irc), cw->entry_irc,
       0, 1, 1, 2, GtkAttachOptions(GTK_FILL | GTK_EXPAND),
       GTK_FILL, 5, 5);

  /* Create the main text box in a scrolled window */
  cw->text_irc = gtk_text_view_new();
	last_pos_mark_add(cw->text_irc);
  tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cw->text_irc));
  cw->r_tag_irc = gtk_text_buffer_create_tag(tb, NULL, NULL);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(cw->text_irc), FALSE);
  gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(cw->text_irc), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cw->text_irc), GTK_WRAP_WORD);
  scroll = gtk_scrolled_window_new(0, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(scroll, 320, 175);
  gtk_container_add(GTK_CONTAINER(scroll), cw->text_irc);
	GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_add(GTK_CONTAINER(frame), scroll);
  gtk_table_attach(GTK_TABLE(cw->table_irc), frame, 0, 1, 0, 1,
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GTK_FILL, 5, 5);

  /* The list of users for the irc panel */
  GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
  cw->list_users = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
  g_object_unref(G_OBJECT(store));
  GtkCellRenderer *r = gtk_cell_renderer_text_new();
  GtkTreeViewColumn *col =
      gtk_tree_view_column_new_with_attributes("User", r, "text", 0, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(cw->list_users), col);
  gtk_table_attach(GTK_TABLE(cw->table_irc), cw->list_users,
			1, 2, 0, 2, GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GTK_FILL, 5, 5);
  
  // Ensure that when switching tabs cursor will end up in the right widget
	GList *focus_chain2 = NULL;
	focus_chain2 = g_list_append(focus_chain2, cw->entry_irc);
	gtk_container_set_focus_chain(GTK_CONTAINER(cw->table_irc), focus_chain2);
	
  gtk_notebook_append_page(GTK_NOTEBOOK(cw->notebook),
			cw->table_irc, gtk_label_new("IRC Mode"));

  /* We need every key press to know how to send it */
  g_signal_connect(G_OBJECT(cw->text_local), "key-press-event",
			G_CALLBACK(chat_send), cw);
  g_signal_connect(G_OBJECT(cw->entry_irc), "key-press-event",
			G_CALLBACK(chat_send), cw);
  g_signal_connect(G_OBJECT(cw->window), "delete_event",
			G_CALLBACK(call_chat_close), cw);
  
  gtk_widget_show_all(cw->window);

  set_tag_font(cw->r_tag, "Sans", 10, PANGO_STYLE_NORMAL);
  set_tag_font(cw->r_tag_irc, "Sans", 10, PANGO_STYLE_NORMAL);
  
  int *fg = cw->chatman->ColorFg(), *bg = cw->chatman->ColorBg();
  if (fg[0] != bg[0] || fg[1] != bg[1] || fg[2] != bg[2]) {
    set_tag_colors(cw->r_tag, fg, bg);
    set_tag_colors(cw->r_tag_irc, fg, bg);
  }

  return cw;
}

GtkWidget *
chat_create_menu(struct chat_window *cw)
{
  GtkItemFactoryEntry menu_items[] = {
    { "/_Chat",        0,         0, 0, "<Branch>" },
      { "/Chat/_Audio", "<control>A",
          GtkItemFactoryCallback(chat_audio), 0, "<ToggleItem>" },
      { "/Chat/_Kick", "<control>K",
          GtkItemFactoryCallback(chat_kick), 0, 0},
      { "/Chat/Kick _No Vote", "<control>N",
          GtkItemFactoryCallback(chat_kick_no_vote), 0, 0},
      { "/Chat/_Save", "<control>S",
          GtkItemFactoryCallback(chat_save), 0, 0},
      { "/Chat/sep1",     0,   0, 0, "<Separator>" },
      { "/Chat/_Close",  "<control>C",
          GtkItemFactoryCallback(chat_close), 0, 0},
    { "/_More",     0,  0, 0, "<Branch>" },
      { "/More/_Beep",   "<control>B",
          GtkItemFactoryCallback(chat_beep_users), 0, 0},
      { "/More/Change Font", 0,
         GtkItemFactoryCallback(chat_change_font), 0, 0},
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

void 
chat_audio(gpointer cw, guint action, GtkWidget *widget)
{
  ((struct chat_window *)cw)->audio = !((struct chat_window *)cw)->audio;
}

void 
start_kick_window(struct chat_window *cw)
{
  cw->kw = g_new0(struct kick_window, 1);

  // Make a window to get the user to kick.  Then create a "tally" window
  // to keep a tally of the voting.
  cw->kw->winKick = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(cw->kw->winKick), "Kick User");

  cw->kw->cmbUsers = gtk_combo_new();
  GList *users = 0;

  list<CChatUser *>::iterator iter;
  for (iter = cw->ChatUsers.begin(); iter != cw->ChatUsers.end(); iter++) {
    gchar *temp = g_strdup_printf("%s (%ld)", (*iter)->Name(), (*iter)->Uin());
    g_list_append(users, temp);
    g_free(temp);
  }

  gtk_combo_set_popdown_strings(GTK_COMBO(cw->kw->cmbUsers), users);

  cw->kw->btnKick = gtk_button_new_with_mnemonic("_Kick");
  GtkWidget *btnCancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);

  // Leave the "clicked" signal for btnKick to each function
  // since it varies
  g_signal_connect(G_OBJECT(btnCancel), "clicked",
			G_CALLBACK(window_close), cw->kw->winKick);
  g_signal_connect(G_OBJECT(cw->kw->winKick), "destroy",
			G_CALLBACK(window_close), cw->kw->winKick);
  
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

void 
chat_kick(struct chat_window *cw, guint action, GtkWidget *widget)
{
  start_kick_window(cw);
  
  g_signal_connect(G_OBJECT(cw->kw->btnKick), "clicked",
    	G_CALLBACK(kick_callback), (gpointer)cw);

  gtk_widget_show_all(cw->kw->winKick);
}

unsigned long 
start_kick_callback(struct chat_window *cw)
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

void 
kick_callback(GtkWidget *widget, gpointer _cw)
{
  struct chat_window *cw = (struct chat_window *)_cw;
  unsigned long Uin = start_kick_callback(cw);  

  cw->chatman->SendKick(Uin);

  // Close the kick window
  gtk_widget_destroy(cw->kw->winKick);

  // Create a tally window
}

void 
chat_kick_no_vote(struct chat_window *cw, guint action, GtkWidget *widget)
{
  start_kick_window(cw);

  g_signal_connect(G_OBJECT(cw->kw->btnKick), "clicked",
    	G_CALLBACK(kick_no_vote_callback), (gpointer)cw);
  
  gtk_widget_show_all(cw->kw->winKick);
}

void
kick_no_vote_callback(GtkWidget *widget, gpointer _cw)
{
  struct chat_window *cw = (struct chat_window *)_cw;
  unsigned long Uin = start_kick_callback(cw);

  cw->chatman->SendKickNoVote(Uin);

  // Close the kick window
  gtk_widget_destroy(cw->kw->winKick);

  // Create the tally window
}

void
chat_save(gpointer _cw, guint action, GtkWidget *widget)
{
  GtkWidget *dlg_save_chat = gtk_file_selection_new("Licq - Save Chat");

  // Connect the signals
  g_signal_connect(
    G_OBJECT(GTK_FILE_SELECTION(dlg_save_chat)->ok_button),
    "clicked", GTK_SIGNAL_FUNC(save_chat_ok),
    _cw);
  g_signal_connect(
    G_OBJECT(GTK_FILE_SELECTION(dlg_save_chat)->cancel_button),
    "clicked", G_CALLBACK(save_chat_cancel),
    0);
  g_signal_connect(G_OBJECT(dlg_save_chat), "delete_event",
    G_CALLBACK(save_chat_cancel), 0);
  
  gtk_widget_show_all(dlg_save_chat);
}

void save_chat_ok(GtkWidget *widget, gpointer _cw)
{
}

void save_chat_cancel(GtkWidget *widget, gpointer data)
{
}

void
chat_close(struct chat_window *cw, guint action, GtkWidget *widget)
{
  if (chat_close_base(cw)) {
    gtk_widget_destroy(cw->window);
    g_free(cw);
  }
}

void
chat_pipe_callback(struct chat_window *cw, gint pipe, GdkInputCondition)
{
  /* Read out any pending events */
  gchar buf[32];
  read(pipe, buf, 32);

  CChatEvent *e = 0;
  GtkTextBuffer *tb, *tb_irc;
  GtkTextIter iter, iter_irc;
  
  tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cw->text_remote));
  tb_irc = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cw->text_irc));
  gtk_text_buffer_get_end_iter(tb, &iter);
  gtk_text_buffer_get_end_iter(tb_irc, &iter_irc);

  while ((e = cw->chatman->PopChatEvent()) != 0) {
    CChatUser *user = e->Client();
		string user_name = cchatuser_utf8(user->Name(), user);
    cw->hold_cuser = user;

    switch (e->Command()) {
      case CHAT_ERRORxBIND:
        message_box("Unable to bind to a port.\nSee Network Log for details.");
        chat_close(cw, 0, 0);
        break;
      
      case CHAT_ERRORxCONNECT:
        message_box("Unable to connect to the remote chat.\nSee Network Log "
                    "for details.");
        chat_close(cw, 0, 0);
        break;
      
      case CHAT_ERRORxRESOURCES:
        message_box("Unable to create new thread.\nSee Network Log for "
                    "details.");
        chat_close(cw, 0, 0);
        break;

      case CHAT_DISCONNECTION:
      {
//        list<CChatUser *>::iterator iter;
//        for(iter = cw->ChatUsers.begin();
//            iter != cw->ChatUsers.end(); ++iter)
//        {
//          if((*iter)->Uin() == user->Uin())
//            cw->ChatUsers.erase(iter);
//        }

        string msg = user_name + " closed connection";
        message_box(msg.c_str());

        chat_close(cw, 0, 0);
        return;
      }

      case CHAT_DISCONNECTIONxKICKED:
      {
//        list<CChatUser *>::iterator iter;
//        for(iter = cw->ChatUsers.begin();
//            iter != cw->ChatUsers.end(); ++iter)
//        {
//          if((*iter)->Uin() == user->Uin())
//            cw->ChatUsers.erase(iter);
//        }

        string msg = user_name + " was kicked";
        message_box(msg.c_str());

        chat_close(cw, 0, 0);
        break;
      }

      case CHAT_KICKxYOU:
        message_box("You have been kicked!");
        break;

      case CHAT_CONNECTION:
      {
				// cw->ChatUsers.push_back(user);
        GtkTreeIter iter;
        GtkListStore *store = 
            GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(cw->list_users)));
        
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, user_name.c_str(), -1);
        if (cw->chat_user == 0) {
          cw->chat_user = user;
          gtk_notebook_set_current_page(GTK_NOTEBOOK(cw->notebook), 0);
        }
        else {
          gtk_notebook_set_current_page(GTK_NOTEBOOK(cw->notebook), 1);
          break;
        }

        string frame_title = "Remote - " + user_name;
        gtk_frame_set_label(GTK_FRAME(cw->frame_remote), frame_title.c_str());

        /* Get their back color */
        set_tag_colors(cw->r_tag, user->ColorFg(), user->ColorBg());
        set_tag_colors(cw->r_tag_irc, user->ColorFg(), user->ColorBg());
        
        break;
      }
      
      case CHAT_NEWLINE:
      {
        /* Add it to the paned view */
        if (user == cw->chat_user)
          gtk_text_buffer_insert(tb, &iter, "\n", 1);

        /* Add it to the irc view */
        string name_prompt = user_name + "> ";
        gtk_text_buffer_insert_with_tags(tb_irc, &iter_irc, 
            name_prompt.c_str(), -1, cw->r_tag_irc, NULL);

        gtk_text_buffer_insert_with_tags(tb_irc, &iter_irc, 
            e->Data(), -1, cw->r_tag_irc, NULL);
        gtk_text_buffer_insert_with_tags(tb_irc, &iter_irc, 
            "\n", 1, cw->r_tag_irc, NULL);
        scroll_to_the_end(cw->text_irc);
        break;
      }

      case CHAT_BEEP:
        /* Is audio enabled? */
        if (cw->audio)
          gdk_beep();
        	/* If not show a <Beep Beep!> */
        else {
          if (user == cw->chat_user) {
            gtk_text_buffer_insert_with_tags(tb, &iter, 
                "<Beep Beep!>\n", -1, cw->r_tag, NULL);
            scroll_to_the_end(cw->text_remote);
          }
            
          string txt = user_name + "> <Beep Beep!>\n";
          gtk_text_buffer_insert_with_tags(tb_irc, &iter_irc, 
              txt.c_str(), -1, cw->r_tag_irc, NULL);

          scroll_to_the_end(cw->text_remote);
        }
        break;

      case CHAT_LAUGH:
        break;

      case CHAT_BACKSPACE:
        if (user == cw->chat_user) {
          GtkTextMark *mark = gtk_text_buffer_get_insert(tb);
          GtkTextIter iter1, iter2;
          gtk_text_buffer_get_iter_at_mark(tb, &iter1, mark);
          gtk_text_buffer_get_iter_at_mark(tb, &iter2, mark);
          gtk_text_iter_backward_chars(&iter1, 1);
          gtk_text_buffer_delete(tb, &iter1, &iter2);
        }
        break;

      case CHAT_CHARACTER:
        if (user == cw->chat_user) {
          gtk_text_buffer_insert_with_tags(tb, &iter, 
              e->Data(), -1, cw->r_tag, NULL);
          scroll_to_the_end(cw->text_remote);
        }
        break;

      case CHAT_COLORxFG:
        if (user == cw->chat_user)
          set_tag_fg_color(cw->r_tag, user->ColorFg());
        set_tag_fg_color(cw->r_tag_irc, user->ColorFg());
        break;

      case CHAT_COLORxBG:
        if (user == cw->chat_user)
          set_tag_bg_color(cw->r_tag, user->ColorBg());
        set_tag_bg_color(cw->r_tag_irc, user->ColorBg());
        break;

      case CHAT_FONTxFAMILY:
        if (user == cw->chat_user)
          set_tag_font_family(cw->r_tag, user->FontFamily());
        set_tag_font_family(cw->r_tag_irc, user->FontFamily());
        break;

      case CHAT_FONTxFACE:
        PangoWeight weight;
        PangoStyle style;
        if (user->FontBold())
          weight = PANGO_WEIGHT_BOLD;
        else
          weight = PANGO_WEIGHT_NORMAL;
        if (user->FontItalic())
          style = PANGO_STYLE_ITALIC;
        else
          style = PANGO_STYLE_NORMAL;

        if (user == cw->chat_user) {
          set_tag_font_style(cw->r_tag, style);
          set_tag_font_weight(cw->r_tag, weight);
        }
        set_tag_font_style(cw->r_tag_irc, style);
        set_tag_font_weight(cw->r_tag_irc, weight);
        break;

      case CHAT_FONTxSIZE:
        if (user == cw->chat_user)
          set_tag_font_size(cw->r_tag, user->FontSize());
        set_tag_font_size(cw->r_tag_irc, user->FontSize());
        break;

      case CHAT_FOCUSxOUT:
        if (user == cw->chat_user) {
          string new_text =  "Remote - " + user_name + " - Away";
          gtk_frame_set_label(GTK_FRAME(cw->frame_remote), new_text.c_str());
        }
        break;

      case CHAT_SLEEPxON:
        if (user == cw->chat_user) {
          string new_text =  "Remote - " + user_name + " - Sleep";
          gtk_frame_set_label(GTK_FRAME(cw->frame_remote), new_text.c_str());
        }
        break;

      case CHAT_FOCUSxIN:
      case CHAT_SLEEPxOFF:
        if (user == cw->chat_user) {
          string new_text =  "Remote - " + user_name;
          gtk_frame_set_label(GTK_FRAME(cw->frame_remote), new_text.c_str());
        }
        break;

      case CHAT_KICK:
        g_print("Kick user %ld?\n", user->ToKick());
        break;

      default:
        gLog.Error("%sInternal Error: invalid command from chat manager (%d)\n", 
            L_ERRORxSTR, e->Command());
        break;
    }

    cw->hold_cuser = 0;
    delete e;
  }
}

const gint 
find_list_row(const gchar *name, GtkWidget *clist)
{
  return 0;
}

void
chat_send_irc_mode(struct chat_window *cw)
{
  GtkTextBuffer *tb;
  GtkTextIter iter;
  
  tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cw->text_local));
  gtk_text_buffer_get_end_iter(tb, &iter);
  const gchar *new_txt = gtk_entry_get_text(GTK_ENTRY(cw->entry_irc));
  gtk_text_buffer_insert(tb, &iter, new_txt, -1);
  scroll_to_the_end(cw->text_local);
  gtk_text_buffer_insert(tb, &iter, "\n", 1);

  gchar *temp = g_strdup_printf("%s> ", cw->local_name);
  tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cw->text_irc));
  gtk_text_buffer_get_end_iter(tb, &iter);
  gtk_text_buffer_insert(tb, &iter, temp, -1);
  g_free(temp);

  gtk_text_buffer_insert(tb, &iter, new_txt, -1);
  scroll_to_the_end(cw->text_irc);
  gtk_text_buffer_insert(tb, &iter, "\n", 1);
  gtk_entry_set_text(GTK_ENTRY(cw->entry_irc), "");
}

void
chat_send_pane_mode(struct chat_window *cw)
{
  GtkTextBuffer *tb1, *tb2;
  GtkTextIter iter, iter1, iter2;
  GtkTextMark *mark;

  tb1 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cw->text_local));
  mark = gtk_text_buffer_get_mark(tb1, "last_pos");
  gtk_text_buffer_get_iter_at_mark(tb1, &iter1, mark); 
  gtk_text_buffer_get_end_iter(tb1, &iter2); 
  gchar *new_text = gtk_text_buffer_get_text(tb1, &iter1, &iter2, FALSE);
  
  // since we scroll_to_the_end before inserting \n in text_local our last_pos 
	// is alwaysjust before the last \n - get rid of it before inserting it into 
	// text_irc
	gchar *temp = g_strdup_printf("%s> %s", cw->local_name, 
			(new_text[0] == '\n' ? new_text + 1 : new_text));
  g_free(new_text);
  
  tb2 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cw->text_irc));
  gtk_text_buffer_get_end_iter(tb2, &iter);
  gtk_text_buffer_insert(tb2, &iter, temp, -1);
  g_free(temp);

	scroll_to_the_end(cw->text_irc);
  gtk_text_buffer_insert(tb2, &iter, "\n", 1);
  scroll_to_the_end(cw->text_local);
}

gboolean
chat_send(GtkWidget *widget, GdkEventKey *event, struct chat_window *cw)
{
  gboolean ret_val;

  switch (event->keyval) {
    case GDK_BackSpace:
      cw->chatman->SendBackspace();
      return FALSE;

    case GDK_Linefeed:
    case GDK_Return:
      if (gtk_notebook_get_current_page(GTK_NOTEBOOK(cw->notebook)) == 1) {
        // IRC mode
				chat_send_irc_mode(cw);
        ret_val = TRUE;
      }
      else {
      	// Pane mode
        chat_send_pane_mode(cw);
        ret_val = FALSE;
      }      
      
      cw->chatman->SendNewline();
      return ret_val;
  }

  /* Limit what we want */
  if (event->keyval < GDK_space || event->keyval > GDK_questiondown)
    return TRUE;

  cw->chatman->SendCharacter(event->string[0]);
  return FALSE;
}

void
chat_beep_users(struct chat_window *cw, guint action, GtkWidget *widget)
{
  cw->chatman->SendBeep();
  gdk_beep();
}

void
chat_change_font(struct chat_window *cw, guint action, GtkWidget *widget)
{
  cw->font_sel_dlg = gtk_font_selection_dialog_new("Licq - Select Font");

  g_signal_connect(
			G_OBJECT(GTK_FONT_SELECTION_DIALOG(cw->font_sel_dlg)->cancel_button),
			"clicked",
    	G_CALLBACK(window_close), cw->font_sel_dlg);
  g_signal_connect(
			G_OBJECT(GTK_FONT_SELECTION_DIALOG(cw->font_sel_dlg)->ok_button),
			"clicked",
    	G_CALLBACK(font_dlg_ok), cw);
  
  gtk_widget_show_all(cw->font_sel_dlg);
}

void
font_dlg_ok(GtkWidget *widget, struct chat_window *cw)
{
  // Only changes the font face for now...
  // Get the font name and font if they exist
  char *tmp = gtk_font_selection_dialog_get_font_name(
	    GTK_FONT_SELECTION_DIALOG(cw->font_sel_dlg));
  if (tmp != 0) {
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
    
    //if(len < 45)
    //  strncpy(cw->l_font_name, tmp, len);
    //else
    //  strncpy(cw->l_font_name, "clean", 6);

    //g_print("Final: %s\n", cw->l_font_name);

    //cw->chatman->ChangeFontFamily(cw->l_font_name, ENCODING_DEFAULT, 
		//		STYLE_DONTCARE);
  }

  gchar *fname = gtk_font_selection_dialog_get_font_name(
    GTK_FONT_SELECTION_DIALOG(cw->font_sel_dlg));
  //if (fname != 0)
  //  cw->l_font = pango_font_description_from_string(fname);
  g_free(fname);
  
  // Ok, close it now
  gtk_widget_destroy(cw->font_sel_dlg);
}

void
finish_chat(ICQEvent *event)
{
  struct request_chat *rc = g_new0(struct request_chat, 1);

  rc = rc_find(event->Uin());

  if (rc == 0)
    return;
    
  close_request_chat(rc);
  chat_start_as_client(event);
}

