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

#include "pixmaps/online.xpm"
#include "pixmaps/offline.xpm"
#include "pixmaps/away.xpm"
#include "pixmaps/na.xpm"
#include "pixmaps/occ.xpm"
#include "pixmaps/dnd.xpm"
#include "pixmaps/message.xpm"

#include "licq_icqd.h"
#include "licq_events.h"

#include <ctype.h>
#include <gtk/gtk.h>

void do_colors()
{
        red = new GdkColor;
        green = new GdkColor;
        blue = new GdkColor;
 
        red->red = 30000;
        red->green = 0;
        red->blue = 0;
        red->pixel = (gulong)(255);
 
        green->red = 0;
        green->green = 30000;
        green->blue = 0;
        green->pixel = (gulong)(255*256);
 
        blue->red = 0;
        blue->green = 0;
        blue->blue = 30000;
        blue->pixel = (gulong)(255*256); 
}

void do_pixmaps()
{
        style = gtk_widget_get_style(main_window);
 
        online = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                              &style->bg[GTK_STATE_NORMAL],
                                              (gchar **)online_xpm);
 
        away = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                            &style->bg[GTK_STATE_NORMAL],
                                            (gchar **)away_xpm);
 
        na = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                          &style->bg[GTK_STATE_NORMAL],
                                          (gchar **)na_xpm);
 
        dnd = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                           &style->bg[GTK_STATE_NORMAL],
                                           (gchar **)dnd_xpm);
 
        occ = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                           &style->bg[GTK_STATE_NORMAL],
                                           (gchar **)occ_xpm); 

        offline = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                               &style->bg[GTK_STATE_NORMAL],
                                               (gchar **)offline_xpm);
 
        message = gdk_pixmap_create_from_xpm_d(main_window->window, &bm,
                                               &style->bg[GTK_STATE_NORMAL],
                                               (gchar **)message_xpm); 
}

void verify_numbers(GtkEditable *e, gchar *text, gint len, gint *pos, gpointer d)
{
	int i;
   	gchar *result = g_new(gchar, len);

   	for (i=0; i<len; i++)
	{
		if(!isdigit(text[i]))
			result[i] = NULL;
		else
			result[i] = text[i];
	}

	gtk_signal_handler_block_by_func(GTK_OBJECT(e),
                                         GTK_SIGNAL_FUNC(verify_numbers),
                                         d);

	gtk_editable_insert_text(GTK_EDITABLE(e), result, len, pos);

	gtk_signal_handler_unblock_by_func(GTK_OBJECT(e),
                                           GTK_SIGNAL_FUNC(verify_numbers),
                                       	   d);

	gtk_signal_emit_stop_by_name(GTK_OBJECT(e), "insert_text");

	g_free(result);
}

void user_function(ICQEvent *event)
{
	GSList *temp = catcher;
	struct e_tag_data *etd;

	while(temp)
	{
		etd = (struct e_tag_data *)temp->data;

		if(etd->e_tag->Equals(event))
		{
			finish_event(etd, event);
			return;
		}

		/* Not this one, go on to the next */
		temp = temp->next;
	}

	return;
}

void finish_event(struct e_tag_data *etd, ICQEvent *event)
{
	/* Make sure we have the right event and event tag */
	if( (etd->e_tag == NULL && event != NULL) ||
	    (etd->e_tag != NULL && !etd->e_tag->Equals(event)) )
	    	return;

	guint id;
	gchar temp[60];
	
	/* Get the id for the status bar */
	id = gtk_statusbar_get_context_id(GTK_STATUSBAR(etd->statusbar), "sta");
	
	/* Get the current text */
	strcpy(temp, etd->buf);
	
	if(event == NULL)
	{
		strcat(temp, "error");
	}

	else
	{
	/* Pop and then push the current text by the right event result */
		switch(event->Result())
		{
		case EVENT_ACKED:
		case EVENT_SUCCESS:
			strcat(temp, "done");
			break;
		case EVENT_FAILED:
			strcat(temp, "failed");
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

	gtk_statusbar_pop(GTK_STATUSBAR(etd->statusbar), id);
	gtk_statusbar_push(GTK_STATUSBAR(etd->statusbar), id, temp);
	
	/* Update the buffer for the e_tag_data */
	strcpy(etd->buf, "");
	strcpy(etd->buf, temp);

	/* Remove this item from the GSList */
	catcher = g_slist_remove(catcher, etd);
	
	/* Get the sub command and see if do more work if needed */
	switch(event->SubCommand())
	{
	case ICQ_CMDxSUB_MSG:
		finish_message(event);
		break;
	}
}

void finish_message(ICQEvent *event)
{
	struct conversation *c =
		(struct conversation *)g_new0(struct conversation, 1);

	c = convo_find(event->Uin());

	/* If the window isn't open, there isn't anything left to do */
	if(c == NULL)
		return;

	/* Check to make sure it sent, and if it did, put the text in */
	g_strreverse(c->etag->buf);

	if(strncmp(c->etag->buf, "en", 2) == 0)
	{
		ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
		const gchar *name = g_strdup_printf("%s",
						owner->GetAlias());
		gUserManager.DropOwner();

		gtk_editable_delete_text(GTK_EDITABLE(c->entry), 0, -1);
		gtk_editable_delete_text(GTK_EDITABLE(c->spoof_uin), 0, -1);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->spoof_button),
					     FALSE);
		gtk_window_set_focus(GTK_WINDOW(c->window), c->entry);
		
		gtk_text_freeze(GTK_TEXT(c->entry));
		gtk_text_insert(GTK_TEXT(c->text), 0, blue, 0, name, -1);
		gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, c->for_user, -1);
		gtk_text_thaw(GTK_TEXT(c->entry));
	}
}
