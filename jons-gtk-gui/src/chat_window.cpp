/* Copyright shit */

#include "licq_gtk.h"

#include "licq_buffer.h"
#include "licq_chat.h"
#include "licq_icq.h"
#include "licq_icqd.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

GSList *rc_list;

void list_request_chat(GtkWidget *widget, ICQUser *user)
{
	/* Do we even want to be here? */
	struct request_chat *rc = rc_find(user->Uin());

	if(rc != NULL)
		return;

	rc = rc_new(user);

	GtkWidget *scroll;
	GtkWidget *statusbar;
	GtkWidget *ok;
	GtkWidget *cancel;
	GtkWidget *close;
	GtkWidget *table;
	GtkWidget *h_box;
	const gchar *title = g_strdup_printf("Licq - Request Chat With %s",
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

	/* Create the scrolled window with text and attach it to the window */
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
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

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
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

	/* Progress bar */
	statusbar = gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table), statusbar, 0, 2, 2, 3,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);

	/* e_tag stuff */
	rc->etd->statusbar = statusbar;
	strcpy(rc->etd->buf, "");
	
	/* The button box */
	h_box = gtk_hbox_new(TRUE, 5);
	ok = gtk_button_new_with_label("OK");
	cancel = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), cancel, TRUE, TRUE, 0);
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 3, 4,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 3, 3);
	
	/* Connect the signals */
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(ok_request_chat), rc);
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			   GTK_SIGNAL_FUNC(cancel_request_chat), rc);

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

void ok_request_chat(GtkWidget *widget, struct request_chat *rc)
{
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

void cancel_request_chat(GtkWidget *widget, struct request_chat *rc)
{
	icq_daemon->CancelEvent(rc->etd->e_tag);
	catcher = g_slist_remove(catcher, rc->etd);
	close_request_chat(rc);
}

void close_request_chat(struct request_chat *rc)
{
	rc_list = g_slist_remove(rc_list, rc);
	gtk_widget_destroy(rc->window);
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
	GtkWidget *menu_bar;
	GtkWidget *close;
	struct chat_window *cw;

	cw = g_new0(struct chat_window, 1);

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
	cw->fore_color->blue = cw->chatman->ColorFg()[2];

	ICQUser *u = gUserManager.FetchUser(uin, LOCK_R);
	cw->user = u;
	gUserManager.DropUser(u);

	/* Make the window */
	cw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(cw->window), "Licq - Chat");
	gtk_window_set_position(GTK_WINDOW(cw->window), GTK_WIN_POS_CENTER);

	/* Create the table */
	cw->table = gtk_table_new(4, 1, FALSE);
	gtk_container_add(GTK_CONTAINER(cw->window), cw->table);

	/* Create the menu */
	menu_bar = chat_create_menu(cw);
	gtk_table_attach(GTK_TABLE(cw->table), menu_bar, 0, 1, 0, 1,
			 GTK_FILL | GTK_EXPAND, GTK_FILL, 5, 5);

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
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND,10, 10);

	/* Dreate the local fram with local text box */
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
			 GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND,10, 10);

	/* We need every key press to know how to send it */
	cw->input_tag = gtk_signal_connect(GTK_OBJECT(cw->text_local),
			   "key-press-event",
			   GTK_SIGNAL_FUNC(chat_send), cw);

	gtk_widget_show_all(cw->window);

	return cw;
}

GtkWidget *chat_create_menu(struct chat_window *cw)
{
	GtkWidget *chat_menu;
	GtkWidget *menu_item;
	GtkWidget *title_menu;
	GtkWidget *menu_bar;

	/* This is the main menu, don't show it */
	chat_menu = gtk_menu_new();

	/* Do we want the beep (and laugh when working)? Default is yes */
	menu_item = gtk_check_menu_item_new_with_label("Audio");
	gtk_menu_append(GTK_MENU(chat_menu), menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item),
				       TRUE);
	gtk_signal_connect(GTK_OBJECT(menu_item), "toggled",
			   GTK_SIGNAL_FUNC(chat_menu_audio_callback), cw);
	gtk_widget_show(menu_item);

	/* Actually set the default here */
	cw->audio = TRUE;

	menu_separator(chat_menu);

	menu_item = gtk_menu_item_new_with_label("Close");
	gtk_menu_append(GTK_MENU(chat_menu), menu_item);
	gtk_widget_show(menu_item);
	gtk_signal_connect(GTK_OBJECT(menu_item), "activate",
			   GTK_SIGNAL_FUNC(chat_close), cw);

	title_menu = menu_new_item(NULL, "Chat", NULL);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(title_menu), chat_menu);
	
	menu_bar = gtk_menu_bar_new();
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), title_menu);

	return menu_bar;
}

void chat_menu_audio_callback(GtkWidget *widget, struct chat_window *cw)
{
	cw->audio ? cw->audio = FALSE : cw->audio = TRUE;	
}
void chat_close(GtkWidget *widget, struct chat_window *cw)
{
	gdk_input_remove(cw->input_tag);
	cw->chatman->CloseChat();
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

		switch(e->Command())
		{
			case CHAT_DISCONNECTION:
			{
				gchar *name = user->Name();
				g_print("%s closed connection.\n", name);
				break;
			}

			case CHAT_CONNECTION:
			{
				gtk_frame_set_label(
					GTK_FRAME(cw->frame_remote),
					g_strdup_printf("Remote - %s",
							user->Name()));
				break;
			}

			case CHAT_NEWLINE:
			{
				gtk_text_insert(GTK_TEXT(cw->text_remote),
						0, 0, 0, "\n", -1);
				break;
			}

			case CHAT_BEEP:
			{
				/* Is audio enabled? */
				if(cw->audio)
					gdk_beep();
				break;
			}

			case CHAT_BACKSPACE:
			{
				gtk_text_backward_delete(
					GTK_TEXT(cw->text_remote),
					1);
				break;
			}

			case CHAT_CHARACTER:
			{
				gtk_text_insert(GTK_TEXT(cw->text_remote),
						cw->font_remote,
						cw->fore_color,
						cw->back_color, e->Data(), -1);
				break;
			}

			case CHAT_COLORxFG:
			{
				cw->fore_color->red = 257 * user->ColorFg()[0];
				cw->fore_color->green = 257 * user->ColorFg()[1];
				cw->fore_color->blue = 257 * user->ColorFg()[2];
				break;
			}

			case CHAT_COLORxBG:
			{
				cw->back_color->red = 257 * user->ColorBg()[0];
				cw->back_color->green = 257 * user->ColorBg()[1];
				cw->back_color->blue = 257 * user->ColorBg()[2];
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
				if(user->FontBold())
				{
					cw->remote_bold = TRUE;
					cw->font_remote = gdk_font_load(
						g_strdup_printf(
						"-*-%s-%s-%c-normal--*-%d-*-*-*-*-iso8859-1",
						cw->font_name,
						cw->remote_bold ? "bold" : "medium",
						cw->remote_italic ? 'i' : 'r',
						cw->font_size));
				}
				
				if(user->FontItalic())
				{
					cw->remote_italic = TRUE;
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
				cw->font_size = 10 * user->FontSize();
				cw->font_remote = gdk_font_load(
					g_strdup_printf("-*-%s-%s-%c-normal--*-%d-*-*-*-*-iso8859-1",
					cw->font_name,
					cw->remote_bold ? "bold" : "medium",
					cw->remote_italic ? 'i' : 'r',
					cw->font_size));
				break;
			}

			default:
			{
				gLog.Error("%sInternal Error: invalid command from chat manager (%d)\n", 
				L_ERRORxSTR, e->Command());
				break;
			}
		}

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
			cw->chatman->SendNewline();
			break;
		}

		default:
		{
			cw->chatman->SendCharacter(event->string[0]);
			break;
		}
	}
}
