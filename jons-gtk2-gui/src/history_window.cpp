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
#include "event_description.h"

#include <gtk/gtk.h>
#include <time.h>

struct history
{
	GtkWidget *text;
	GtkWidget *check;
	ICQUser *user;
};

const gchar *line = "\n----------------------------\n";

void reverse_history(GtkWidget *widget, struct history *hist);

void list_history(GtkWidget *widget, ICQUser *user)
{
	GtkWidget *window;
	GtkWidget *v_box;
	GtkWidget *h_box;
	GtkWidget *scroll;
	GtkWidget *button;
	struct history *hist;
	const gchar *title = g_strdup_printf("History with %s", user->GetAlias());
	gchar szHdr[256];
	time_t _time;
	HistoryList hist_list;
	HistoryListIter history_iter;

	hist = g_new0(struct history, 1);
	hist->user = user;

	// Make the window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), title);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	// Make the boxes
	v_box = gtk_vbox_new(FALSE, 5);
	h_box = gtk_hbox_new(FALSE, 5);

	// Make the scrolled window
	scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				       GTK_POLICY_NEVER,
				       GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scroll, 300, 225);

	// Make the text box
	hist->text = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(hist->text), GTK_WRAP_WORD);
	gtk_container_add(GTK_CONTAINER(scroll), hist->text);

	// Pack the scrolled window
	gtk_box_pack_start(GTK_BOX(v_box), scroll, TRUE, TRUE, 0);

	// The close button
	button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(window_close), window);

	// The reverse check button
	hist->check = gtk_check_button_new_with_label("Reverse");
	g_signal_connect(G_OBJECT(hist->check), "toggled",
			   G_CALLBACK(reverse_history), hist);

	// Pack them
	gtk_box_pack_start(GTK_BOX(h_box), hist->check, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), button, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);
	
	// Add in the history
	if(!user->GetHistory(hist_list))
		return;

	// The three colors, blue, red, and white
	GdkColor clrBlue, clrRed, clrWhite;

	clrBlue.red   = 0;
	clrBlue.green = 0;
	clrBlue.blue  = 0xFFFF;
	clrBlue.pixel = gulong(255 * 256);

	clrRed.red   = 0xFFFF;
	clrRed.green = 0;
	clrRed.blue  = 0;
	clrRed.pixel = 255;

	clrWhite.red   = 0xFFFF;
	clrWhite.green = 0xFFFF;
	clrWhite.blue  = 0xFFFF;
	clrWhite.pixel = 255;

	history_iter = hist_list.begin();
	GtkTextBuffer *tb = gtk_text_buffer_new(NULL);

	// Easy way, small memory
	GdkColor *clrColor;
	char szDesc[36], szDate[30];
	tm *tmStupid;
	
	while(history_iter != hist_list.end())
	{
		_time = (*history_iter)->Time();
		tmStupid = localtime(&_time);
		strftime(szDate, 29, "%c", tmStupid);
		strcpy(szDesc, event_description(*history_iter));
		
		if ((*history_iter)->Direction() == D_RECEIVER)
		{
			clrColor = &clrRed;
			snprintf(szHdr, 255,
			         "%s from %s\n%s [%c%c%c%c]\n\n",
				 szDesc,
				 user->GetAlias(),
				 szDate,
				 (*history_iter)->IsDirect() ? 'D' : '-',
				 (*history_iter)->IsMultiRec() ? 'M' : '-',
				 (*history_iter)->IsUrgent() ? 'U' : '-',
				 (*history_iter)->IsEncrypted() ? 'E' : '-');
		}
		else
		{
			clrColor = &clrBlue;
			snprintf(szHdr, 255,
			         "%s to %s\n%s [%c%c%c%c]\n\n",
				 szDesc,
				 user->GetAlias(),
				 szDate,
				 (*history_iter)->IsDirect() ? 'D' : '-',
				 (*history_iter)->IsMultiRec() ? 'M' : '-',
				 (*history_iter)->IsUrgent() ? 'U' : '-',
				 (*history_iter)->IsEncrypted() ? 'E' : '-');
		}

		szHdr[255] = '\0';
		GtkTextTag *tag = gtk_text_buffer_create_tag(tb, NULL, 
				"foreground-gdk", clrColor,
				"background-gdk", &clrWhite,
				NULL);
		GtkTextIter iter;
		gtk_text_buffer_get_end_iter(tb, &iter);
		gtk_text_buffer_insert_with_tags(tb, &iter, 
    		szHdr, -1, tag, NULL);
		gtk_text_buffer_get_end_iter(tb, &iter);
		gtk_text_buffer_insert_with_tags(tb, &iter, 
    		(*history_iter)->Text(), -1, tag, NULL);
		gtk_text_buffer_get_end_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, "\n\n", 2);
		history_iter++;
 	}

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(hist->text), tb);
	g_object_unref(G_OBJECT(tb));

	gtk_container_add(GTK_CONTAINER(window), v_box);
	gtk_widget_show_all(window);
}

void reverse_history(GtkWidget *widget, struct history *hist)
{
	HistoryList hist_list;
	HistoryListIter history_iter;
	time_t _time;
	char sz_date[35];

	GtkTextBuffer *tb = gtk_text_buffer_new(NULL);

	if(!hist->user->GetHistory(hist_list))
		return;
	
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hist->check)))
	  history_iter = --(hist_list.end());
 	else
		history_iter = hist_list.begin();

  while(1)
  {
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hist->check)))
		{
			if(history_iter == --(hist_list.begin()))
				break;
 		}
		else
		{
			if(history_iter == hist_list.end())
				break;
		}
    _time = (*history_iter)->Time();
    sprintf(sz_date, "%s\n", ctime(&_time));

		GtkTextIter iter;
		gtk_text_buffer_get_end_iter(tb, &iter);
		GtkTextTag *tag = gtk_text_buffer_create_tag(tb, NULL, 
				"foreground-gdk", 
				(*history_iter)->Direction() == D_RECEIVER ? red : blue,
				NULL);
		gtk_text_buffer_insert_with_tags(tb, &iter, sz_date, -1, tag, NULL);
		gtk_text_buffer_get_end_iter(tb, &iter);
		gtk_text_buffer_insert_with_tags(tb, &iter, 
				(*history_iter)->Text(), -1, 
				tag, NULL);
		gtk_text_buffer_get_end_iter(tb, &iter);
    gtk_text_buffer_insert(tb, &iter, line, -1);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hist->check)))
			history_iter--;
		else
			history_iter++;
	}
 
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(hist->text), tb);
	g_object_unref(G_OBJECT(tb));
}
