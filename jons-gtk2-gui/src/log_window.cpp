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

#include "licq_icqd.h"
#include "licq_log.h"
#include "licq_countrycodes.h"

#include <gtk/gtk.h>
#include <iostream>

using namespace std; // for ofstream

struct network_window *nw = NULL;
gboolean nw_shown = FALSE;
gboolean hidden = FALSE;

void 
new_log_window()
{
	if (nw_shown)
		return;

	if (hidden) {
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
	g_signal_connect(G_OBJECT(nw->window), "delete_event",
			   G_CALLBACK(log_window_close), nw->window);
	
	/* Create the table */
	table = gtk_table_new(2, 3, FALSE);

	/* Create the h_box and box_text */
	h_box = gtk_hbox_new(FALSE, 5);
	box_text = gtk_hbox_new(FALSE, 0);

	/* The text box */
	nw->text = gtk_text_view_new();
	scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll), nw->text);
	gtk_box_pack_start(GTK_BOX(box_text), scroll, TRUE, TRUE, 0);
	gtk_widget_set_size_request(nw->text, 400, 125);

	/* Make the scrolled window part */

	gtk_table_attach(GTK_TABLE(table), box_text, 0, 2, 0, 2,
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			GtkAttachOptions(GTK_FILL | GTK_EXPAND),
      3, 3);

	/* The "OK" button */
	ok = gtk_button_new_from_stock(GTK_STOCK_OK);
	g_signal_connect(G_OBJECT(ok), "clicked",
			   G_CALLBACK(log_window_close), nw->window);
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 5);

	// The "Save" button
	GtkWidget *btnSave = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect(G_OBJECT(btnSave), "clicked",
			   G_CALLBACK(log_window_save), 0);
	gtk_box_pack_start(GTK_BOX(h_box), btnSave, true, true, 5);

	/* The "Clear" button */
	clear = gtk_button_new_from_stock(GTK_STOCK_CLEAR);
	g_signal_connect(G_OBJECT(clear), "clicked",
			   G_CALLBACK(log_window_clear), 0);
	gtk_box_pack_start(GTK_BOX(h_box), clear, TRUE, TRUE, 5);

	/* Attach the h_box to the table */
	gtk_table_attach(GTK_TABLE(table), h_box, 0, 2, 2, 3,
			 GtkAttachOptions(GTK_FILL | GTK_EXPAND),
			 GTK_FILL, 3, 3);

	gtk_container_add(GTK_CONTAINER(nw->window), table);
}

void 
log_window_show(GtkWidget *widget, gpointer data)
{
	if (nw == NULL)	{
		new_log_window();
		log_window_show(0, 0);
	}
	else if (!nw_shown) {
		gtk_widget_show_all(nw->window);
		GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(nw->text));
		GtkTextIter end;
	  gtk_text_buffer_get_end_iter(tb, &end);
  	GtkTextMark *tm = gtk_text_buffer_create_mark(tb, NULL, &end, TRUE);
  	gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(nw->text), tm, 0, FALSE, 0, 0);
		nw_shown = TRUE;
		hidden = FALSE;
	}
}

void 
log_pipe_callback(gpointer data, gint pipe, GdkInputCondition condition)
{
	/* If the window doesn't exist, create it */
	if (nw == NULL)
		new_log_window();

	gchar buf[4];
	read(pipe, buf, 1);

	/* Get the message */
	gchar *for_user = convert_to_utf8(logg->NextLogMsg());
	if (for_user != NULL) {
		GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(nw->text));
		GtkTextIter iter, end;

    // when we're over 510 lines, we'll cut off the first 500
		gtk_text_buffer_get_end_iter(tb, &end);
    if (gtk_text_iter_get_line(&end) > 510) {
      gtk_text_buffer_get_start_iter(tb, &iter);
    	gtk_text_buffer_get_iter_at_line(tb, &end, 500);
      gtk_text_buffer_delete(tb, &iter, &end);
    }

		/* Insert the message */
		gtk_text_buffer_get_end_iter(tb, &iter);
		gtk_text_buffer_insert(tb, &iter, for_user, -1);

		/* Scroll down to the new bottom if not hidden */
		if (!hidden) {
			gtk_text_buffer_get_end_iter(tb, &end);
			GtkTextMark *tm = gtk_text_buffer_create_mark(tb, NULL, &end, TRUE);
			gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(nw->text), tm, 0, FALSE, 0, 0);
		}
		
    g_free(for_user);
	}
	
	/* Get rid of this message */
	logg->ClearLog();
}

void log_window_save(GtkWidget *widget, gpointer data)
{
	GtkWidget *fileSelect = gtk_file_selection_new("Save Network Log");

	// Make sure the window closes then a button is selected
	g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(fileSelect)->ok_button),
			   "clicked", G_CALLBACK(log_window_save_ok),
			   (gpointer)fileSelect);
	g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(fileSelect)->cancel_button),
			   "clicked", G_CALLBACK(log_window_save_cancel),
			   (gpointer)fileSelect);
	
	// Show the window
	gtk_widget_show_all(fileSelect);
}

void log_window_save_ok(GtkWidget *widget, gpointer _fs)
{
	GtkWidget *fileSelect = (GtkWidget *)_fs;
	
	const gchar *filename = 
      gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileSelect));

	ofstream strmFileOut(filename, ios::out);
	if (strmFileOut) {
		GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(nw->text));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(tb, &start);
    gtk_text_buffer_get_end_iter(tb, &end);
		strmFileOut << gtk_text_buffer_get_text(tb, &start, &end, FALSE);
  }
  else {
    GtkWidget *dialog = 
        gtk_message_dialog_new(GTK_WINDOW(nw->window), 
                               GTK_DIALOG_DESTROY_WITH_PARENT,
                               GTK_MESSAGE_ERROR, 
                               GTK_BUTTONS_CLOSE, 
                               "Couldn't save log to file: %s", 
                               filename);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }

	strmFileOut.close();
	gtk_widget_destroy(fileSelect);
}

void log_window_save_cancel(GtkWidget *widget, gpointer _fs)
{
	gtk_widget_destroy((GtkWidget *)_fs);
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
	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(nw->text));
	gtk_text_buffer_set_text(tb, "", 0);
}
