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

#include "pixmaps/online.xpm"
#include "pixmaps/offline.xpm"
#include "pixmaps/away.xpm"
#include "pixmaps/na.xpm"
#include "pixmaps/occ.xpm"
#include "pixmaps/dnd.xpm"
#include "pixmaps/ffc.xpm"
#include "pixmaps/invisible.xpm"
#include "pixmaps/message.xpm"
#include "pixmaps/file.xpm"
#include "pixmaps/chat.xpm"
#include "pixmaps/url.xpm"
#include "pixmaps/secure_on.xpm"
#include "pixmaps/birthday.xpm"
#include "pixmaps/securebday.xpm"
#include "pixmaps/blank.xpm"
#include "pixmaps/charset.xpm"

#include "licq_gtk.h"

#include "licq_countrycodes.h"
#include "licq_languagecodes.h"

#include <ctype.h>
#include <gtk/gtk.h>

#include <iostream>
using namespace std;

extern GtkWidget *register_window;

/*************** Finishing Events *****************************/
void finish_away(ICQEvent *event);
void finish_chat(ICQEvent *event);
void finish_file(ICQEvent *event);
void finish_random(ICQEvent *event);
void finish_secure(ICQEvent *event);
void finish_message(ICQEvent *event);

/*************** Finishing Signals *****************************/
void finish_info(CICQSignal *signal);

void wizard_message(int);


void do_colors()
{
  red = new GdkColor;
  blue = new GdkColor;

	red->red = 60000;
  red->green = 0;
  red->blue = 0;
  red->pixel = (gulong)(255);

  blue->red = 0;
  blue->green = 0;
	blue->blue = 60000;
  blue->pixel = (gulong)(255*256); 
}

void 
do_pixmaps()
{
	online = gdk_pixbuf_new_from_xpm_data((const char **)(online_xpm));
	away = gdk_pixbuf_new_from_xpm_data((const char **)(away_xpm));
	na = gdk_pixbuf_new_from_xpm_data((const char **)(na_xpm));
	occ = gdk_pixbuf_new_from_xpm_data((const char **)(occ_xpm));
	dnd = gdk_pixbuf_new_from_xpm_data((const char **)(dnd_xpm));
	offline = gdk_pixbuf_new_from_xpm_data((const char **)(offline_xpm));
	ffc = gdk_pixbuf_new_from_xpm_data((const char **)(ffc_xpm));
	invisible = gdk_pixbuf_new_from_xpm_data((const char **)(invisible_xpm));
	message_icon = gdk_pixbuf_new_from_xpm_data((const char **)(message_xpm));
	url_icon = gdk_pixbuf_new_from_xpm_data((const char **)(url_xpm));
	file_icon = gdk_pixbuf_new_from_xpm_data((const char **)(file_xpm));
	chat_icon = gdk_pixbuf_new_from_xpm_data((const char **)(chat_xpm));
	secure_icon = gdk_pixbuf_new_from_xpm_data((const char **)(secure_on_xpm));
	birthday_icon = gdk_pixbuf_new_from_xpm_data((const char **)(birthday_xpm));
	securebday_icon = gdk_pixbuf_new_from_xpm_data((const char **)(securebday_xpm));
	blank_icon = gdk_pixbuf_new_from_xpm_data((const char **)(blank_xpm));
	charset_icon = gdk_pixbuf_new_from_xpm_data((const char **)(charset_xpm));
}

/* Used to create a message box ... will be improved down the road */
void message_box(const char *message)
{
	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *ok;

	/* Create the dialog */
	dialog = gtk_dialog_new();
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 3);

	/* The label */
	label = gtk_label_new(message);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);

	/* The ok button */
	ok = gtk_button_new_from_stock(GTK_STOCK_OK); 
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
			  ok);

	/* Close the window on the OK button */
	g_signal_connect(G_OBJECT(ok), "clicked",
			   G_CALLBACK(window_close), dialog);
	
	/* Show the message box*/
	gtk_widget_show_all(dialog);
}

void verify_numbers(GtkEditable *e, gchar *text, gint len, gint *pos, 
		gpointer d)
{
	int i;
	gchar *result = g_new(gchar, len);

  for (i = 0; i < len; i++)
	{
		if(!isdigit(text[i]))
			result[i] = '\0';
		else
			result[i] = text[i];
	}

	g_signal_handlers_block_by_func(G_OBJECT(e), (gpointer)verify_numbers, d);
	gtk_editable_insert_text(GTK_EDITABLE(e), result, len, pos);

	g_signal_handlers_unblock_by_func(G_OBJECT(e), (gpointer)verify_numbers, d);

	g_signal_stop_emission_by_name(G_OBJECT(e), "insert_text");
	
	g_free(result);
}

void owner_function(ICQEvent *event)
{
	/* For the main window, if it's a new registered users */
	char *title = g_strdup_printf("%ld", gUserManager.OwnerUin());

	if (event->Command() == ICQ_CMDxSND_REGISTERxUSER) {
		if(event->Result() == EVENT_SUCCESS) {
			wizard_message(5);
			main_window = main_window_new(title);
			main_window_show();
			system_status_refresh();
			window_close(0, register_window);
		}
		else
			wizard_message(4);
	}
	g_free(title);
}

void finish_event(struct e_tag_data *etd, ICQEvent *event)
{
	/* Make sure we have the right event and event tag */
	if( (etd->e_tag == 0 && event != 0) ||
	    (etd->e_tag != 0 && !event->Equals(etd->e_tag)) )
	  return;

	guint id = 0;
	gchar temp[60];
	
	/* Get the id for the status bar, if the statusbar exists */
	if (etd->statusbar)
		id = gtk_statusbar_get_context_id(GTK_STATUSBAR(etd->statusbar),
			"sta");

	/* Get the current text */
	strcpy(temp, etd->buf);
	
	if (event == 0)
		strcat(temp, "error");
	else if (event->SubCommand() == ICQ_CMDxSUB_SECURExOPEN ||
					 event->SubCommand() == ICQ_CMDxSUB_SECURExCLOSE) {
		catcher = g_slist_remove(catcher, etd);
		finish_secure(event);
		return;
	}
	else {
	/* Pop and then push the current text by the right event result */
		switch(event->Result()) {
			case EVENT_ACKED:
			case EVENT_SUCCESS:
				strcat(temp, "done");
				break;
			case EVENT_FAILED:
				strcat(temp, "failed");
				if (event->SubCommand() == ICQ_CMDxSND_RANDOMxSEARCH)
					message_box("No random chat user found!");
				break;
			case EVENT_TIMEDOUT:
				strcat(temp, "timed out");
				break;
			case EVENT_ERROR:
				strcat(temp, "error");
				break;
			default:
				strcat(temp, "unknown");
				break;
		}
	}

	if (etd->statusbar) {
		gtk_statusbar_pop(GTK_STATUSBAR(etd->statusbar), id);
		gtk_statusbar_push(GTK_STATUSBAR(etd->statusbar), id, temp);
	}

	/* Update the buffer for the e_tag_data */
	strcpy(etd->buf, temp);

	/* Remove this item from the GSList */
	catcher = g_slist_remove(catcher, etd);

	/* Get the sub command and see if do more work if needed */
	switch(event->SubCommand()) {
		case ICQ_CMDxSUB_MSG:
			finish_message(event);
			break;
		case ICQ_CMDxSUB_CHAT:
			finish_chat(event);
			break;
		case ICQ_CMDxSUB_FILE:
			finish_file(event);
			break;
		case ICQ_CMDxTCP_READxAWAYxMSG:
		case ICQ_CMDxTCP_READxOCCUPIEDxMSG:
		case ICQ_CMDxTCP_READxNAxMSG:
		case ICQ_CMDxTCP_READxDNDxMSG:
		case ICQ_CMDxTCP_READxFFCxMSG:
			finish_away(event);
			break;
		default:
			cerr << event->SubCommand() << endl;
	} 

	// No sub command for this..
	if(event->Command() == ICQ_CMDxSND_RANDOMxSEARCH)
		finish_random(event);
}

void user_function(ICQEvent *event)
{
  GSList *temp = catcher;
	struct e_tag_data *etd;

  while (temp) {
		etd = (struct e_tag_data *)temp->data;

		if (event->Equals(etd->e_tag)) {
			finish_event(etd, event);
			return;
		}

		/* Not this one, go on to the next */
		temp = temp->next;
	}

	return;
}
