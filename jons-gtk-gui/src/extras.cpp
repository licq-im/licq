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

#include "icqd.h"
#include "icqevent.h"

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
	struct conversation *c =
		(struct conversation *)g_new0(struct conversation, 1);
	struct user_away_window *uaw =
		(struct user_away_window *)g_new0(struct user_away_window, 1);

	guint id;

	c = convo_find(event->m_nDestinationUin);
	uaw = uaw_find(event->m_nDestinationUin);

	/* Have a status bar on the contact list for this to go to... */
	if(c == NULL && uaw == NULL)
	{
		id = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_progress)
						  , "prog");
		check_other_event(event, status_progress, id);
		return;
	}

	/* It's for the convo window */
	if(uaw == NULL)
	{	
		/* Make sure we have the right event and event tag */
		if( (c->e_tag == NULL && event != NULL) ||
	    	    (c->e_tag != NULL && !c->e_tag->Equals(event)) )
			return;

		id = gtk_statusbar_get_context_id(GTK_STATUSBAR(c->progress),
						  "prog");

		if(event == NULL)
		{
			strcat(c->prog_buf, "error");
			gtk_statusbar_pop(GTK_STATUSBAR(c->progress), id);
			gtk_statusbar_push(GTK_STATUSBAR(c->progress), id,
					   c->prog_buf);
		}

		check_event(event, c->progress, id, c->prog_buf);

		/* Check to make sure it sent, and if it did, put the text */
		g_strreverse(c->prog_buf);
	
		if(strncmp(c->prog_buf, "en", 2))
		{
			ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);
			const gchar *name = g_strdup_printf("%s",
							owner->GetAlias());
			gUserManager.DropOwner();
		
			gtk_editable_delete_text(GTK_EDITABLE(c->entry), 0, -1);
			gtk_editable_delete_text(GTK_EDITABLE(c->spoof_uin),
						 0, -1);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->spoof_button), FALSE);
			gtk_window_set_focus(GTK_WINDOW(c->window), c->entry);
			gtk_text_freeze(GTK_TEXT(c->entry));
			gtk_text_insert(GTK_TEXT(c->text), 0, blue, 0, name, -1);
			gtk_text_insert(GTK_TEXT(c->text), 0, 0, 0, c->for_user,
					-1);
			gtk_text_thaw(GTK_TEXT(c->entry));
		}

		return;
	}

	/* It must be a user_away_window then */
	if( (uaw->e_tag == NULL && event != NULL) ||
	    (uaw->e_tag != NULL && !uaw->e_tag->Equals(event)) )
		return;

	gtk_text_freeze(GTK_TEXT(uaw->text_box));
	gtk_text_insert(GTK_TEXT(uaw->text_box), 0, 0, 0,
			uaw->user->AutoResponse(), -1);
	gtk_text_thaw(GTK_TEXT(uaw->text_box));

	check_event(event, uaw->progress, id, uaw->buffer);
}

void check_event(ICQEvent *event, GtkWidget *widget,
		 guint &id, gchar *p_buf)
{
	switch(event->m_eResult)
	{
	case EVENT_ACKED:
	case EVENT_SUCCESS:
		strcat(p_buf, "done");
		gtk_statusbar_pop(GTK_STATUSBAR(widget), id);
		gtk_statusbar_push(GTK_STATUSBAR(widget), id,
				   p_buf);	
		break;
	case EVENT_FAILED:
		strcat(p_buf, "failed");
		gtk_statusbar_pop(GTK_STATUSBAR(widget), id);
		gtk_statusbar_push(GTK_STATUSBAR(widget), id,
				   p_buf);
		break;
	case EVENT_TIMEDOUT:
		strcat(p_buf, "timed out");
		gtk_statusbar_pop(GTK_STATUSBAR(widget), id);
		gtk_statusbar_push(GTK_STATUSBAR(widget), id,
				   p_buf);
		break;
	case EVENT_ERROR:
		strcat(p_buf, "error");
		gtk_statusbar_pop(GTK_STATUSBAR(widget), id);
		gtk_statusbar_push(GTK_STATUSBAR(widget), id,
				   p_buf);
		break;
	default:
		break;
	}
}

void check_other_event(ICQEvent *event, GtkWidget *widget, guint &id)
{
	struct main_progress *mp;
	GList *mpl = m_prog_list;
	guint flag = 0;

	while(mpl)
	{
		mp = (struct main_progress *)mpl->data;
		
		if(! ((mp->e_tag == NULL && event != NULL) ||
		      (mp->e_tag != NULL && !mp->e_tag->Equals(event))) )
		{
			flag = 1;
			break;
		}

		mpl = mpl->next;
	}	
	
	if(flag == 0)
		return;

	if(event == NULL)
	{
		strcat(mp->buffer, "error");
		gtk_statusbar_pop(GTK_STATUSBAR(widget), id);
		gtk_statusbar_push(GTK_STATUSBAR(widget), id, mp->buffer);
	}

	check_event(event, widget, id, mp->buffer);

	m_prog_list = g_list_remove(m_prog_list, mp);
}
