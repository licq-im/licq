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

#include "licq_icqd.h"
#include "licq_log.h"

#include <gtk/gtk.h>

struct network_window *nw;
gboolean nw_shown = FALSE;
gboolean hidden = FALSE;

void new_log_window()
{
	if(nw_shown)
	{
		return;
	}

	if(hidden)
	{
		gtk_widget_show_all(nw->window);
		hidden = FALSE;
		return; 
	}

	GtkWidget *box_text;
	GtkWidget *scroll;
	GtkWidget *ok;
	GtkWidget *clear;
	GtkWidget *table;
	GtkWidget *h_box;

	nw = g_new0(struct network_window, 1);

	/* Create the window */
	nw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(nw->window), "Network Activity Log");
	gtk_window_set_position(GTK_WINDOW(nw->window), GTK_WIN_POS_CENTER);

	/* Destroy event for window */
	gtk_signal_connect(GTK_OBJECT(nw->window), "delete_event",
			   GTK_SIGNAL_FUNC(log_window_close), nw->window);

	/* Create the table */
	table = gtk_table_new(2, 3, FALSE);

	/* Create the h_box and box_text */
	h_box = gtk_hbox_new(FALSE, 5);
	box_text = gtk_hbox_new(FALSE, 0);

	/* The text box */
	nw->text = gtk_text_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(box_text), nw->text, TRUE, TRUE, 0);
	gtk_widget_set_usize(nw->text, 400, 125);

	/* Make the scrolled window part */
	scroll = gtk_vscrollbar_new(GTK_TEXT(nw->text)->vadj);
	gtk_box_pack_start(GTK_BOX(box_text), scroll, TRUE, TRUE, 0);

	gtk_table_attach(GTK_TABLE(table), box_text, 0, 2, 0, 2,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
                         3, 3);

	/* The "OK" button */
	ok = gtk_button_new_with_label("OK");
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(log_window_close), nw->window);
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 5);

	/* The "Clear" button */
	clear = gtk_button_new_with_label("Clear");
	gtk_signal_connect(GTK_OBJECT(clear), "clicked",
			   GTK_SIGNAL_FUNC(log_window_clear), NULL);
	gtk_box_pack_start(GTK_BOX(h_box), clear, TRUE, TRUE, 5);

	/* Attach the h_box to the table */
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 2, 3,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	gtk_container_add(GTK_CONTAINER(nw->window), table);
}

void log_window_show(GtkWidget *widget, gpointer data)
{
	if(nw == NULL)
	{
		new_log_window();
		log_window_show(NULL, NULL);
	}

	else if(!nw_shown)
	{
		gtk_widget_show_all(nw->window);
		nw_shown = TRUE;
		hidden = FALSE;
	}

}

void log_pipe_callback(gpointer data, gint pipe, GdkInputCondition condition)
{
	/* If the window doesn't exist, wait for it to exist */
	if(nw == NULL)
		new_log_window();

	gchar buf[4];
	gchar *for_user; /* The text for the window */

	read(pipe, buf, 1);

	/* Get the message */
	for_user = log->NextLogMsg();

	/* Insert the message */
	gtk_text_freeze(GTK_TEXT(nw->text));
	gtk_text_insert(GTK_TEXT(nw->text), 0, 0, 0, for_user, -1);
	gtk_text_thaw(GTK_TEXT(nw->text));

	/* Get rid of this message */
	log->ClearLog();
}

gint log_window_close(GtkWidget *widget, GtkWidget *window)
{
	gtk_widget_hide_all(nw->window);
	nw_shown = FALSE;
	hidden = TRUE;
	return TRUE;
}

void log_window_clear(GtkWidget *widget, gpointer data)
{
	gtk_text_freeze(GTK_TEXT(nw->text));
	gtk_editable_delete_text(GTK_EDITABLE(nw->text), 0, -1);
	gtk_text_thaw(GTK_TEXT(nw->text));
}
