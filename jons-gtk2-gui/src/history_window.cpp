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
#include "utilities.h"

#include <gtk/gtk.h>
#include <time.h>

#include <string>
using namespace std;

struct history
{
	GtkWidget *text;
	GtkWidget *reverse;
	GtkWidget *progress;
	GtkTextTag *red, *blue, *bold;
  HistoryList hlist;
  HistoryListIter hlist_iter;
  unsigned long uin;
  size_t loaded;
	guint timeout_id;
};

static const int load_slice = 100;
static const int time_slice = 200;

GtkTextBuffer *
text_buffer_create(struct history *hist)
{
	GtkTextBuffer *tb = gtk_text_buffer_new(NULL);
	
	hist->red = gtk_text_buffer_create_tag(tb, NULL, 
			"foreground-gdk", red, NULL);
	hist->blue = gtk_text_buffer_create_tag(tb, NULL, 
			"foreground-gdk", blue, NULL);
	hist->bold = gtk_text_buffer_create_tag(tb, NULL, 
			"weight", PANGO_WEIGHT_BOLD, NULL);
			
	return tb;
}
 
gboolean
load_history(struct history *hist)
{
	bool reverse = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hist->reverse));
    
  if (reverse) {
    if (hist->hlist_iter == hist->hlist.begin())
      return FALSE;
  }
  else {
    if (hist->hlist_iter == hist->hlist.end())
      return FALSE;
  }
      
	time_t _time;
	tm *tmStupid;
	char szDesc[36], szDate[30];
	gchar szHdr[256];

	GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(hist->text));
	GtkTextIter iter;

  ICQUser *user = gUserManager.FetchUser(hist->uin, LOCK_R);
	const char *encoding = user->UserEncoding();
	string alias(s_convert_to_utf8(user->GetAlias(), encoding));
	gboolean ret = TRUE;
  for (int i = 0; i < load_slice; ++i) {
    if (reverse) {
      if (hist->hlist_iter == hist->hlist.begin()) {
        ret = FALSE;
        break;
      }
      --(hist->hlist_iter);
    }

    HistoryListIter h_iter = hist->hlist_iter;
    _time = (*h_iter)->Time();
	  tmStupid = localtime(&_time);
	  strftime(szDate, 29, "%c", tmStupid);
	  strcpy(szDesc, event_description(*(hist->hlist_iter)));

    GtkTextTag *tag;
		if ((*hist->hlist_iter)->Direction() == D_RECEIVER)
			tag = hist->red;
		else
			tag = hist->blue;

	  snprintf(szHdr, 255,
			  "%s %s %s\n%s [%c%c%c%c]\n\n",
			  szDesc,
        (*h_iter)->Direction() == D_RECEIVER ? "from" : "to",
			  alias.c_str(),
			  szDate,
			  (*h_iter)->IsDirect() ? 'D' : '-',
			  (*h_iter)->IsMultiRec() ? 'M' : '-',
			  (*h_iter)->IsUrgent() ? 'U' : '-',
			  (*h_iter)->IsEncrypted() ? 'E' : '-');

	  szHdr[255] = '\0';
    gtk_text_buffer_get_end_iter(tb, &iter);
	  gtk_text_buffer_insert_with_tags(tb, &iter,
        szHdr, -1, tag, hist->bold, NULL);
	  gtk_text_buffer_get_end_iter(tb, &iter);
	  gtk_text_buffer_insert_with_tags(tb, &iter, 
    	  s_convert_to_utf8((*h_iter)->Text(), encoding).c_str(), -1, tag, NULL);
	  gtk_text_buffer_get_end_iter(tb, &iter);
    gtk_text_buffer_insert(tb, &iter, "\n\n\n", 3);

    hist->loaded++;
    
		if (!reverse) {
      ++(hist->hlist_iter);
      if (hist->hlist_iter == hist->hlist.end()) {
        ret = FALSE;
        break;
      }
    }
  }
  
	gUserManager.DropUser(user);

  char txt[50];
  snprintf(txt, 30, "%u / %u", hist->loaded, hist->hlist.size());
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(hist->progress), txt);
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(hist->progress),
      (double)hist->loaded / hist->hlist.size());
  return ret;
}

void
reverse_cb(GtkWidget *, struct history *hist)
{
	if (hist->timeout_id != 0)
		g_source_remove(hist->timeout_id);
	GtkTextBuffer *tb = text_buffer_create(hist);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(hist->text), tb);
	g_object_unref(G_OBJECT(tb));
	hist->loaded = 0;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(hist->reverse)))
		hist->hlist_iter = hist->hlist.end();
	else
		hist->hlist_iter = hist->hlist.begin();
	if (load_history(hist))
    hist->timeout_id = 
    	g_timeout_add(time_slice, GSourceFunc(load_history), hist);
	else
    hist->timeout_id = 0;
}

void
hist_destroy_cb(GtkWidget *, struct history *hist)
{
	if (hist->timeout_id != 0)
		g_source_remove(hist->timeout_id);
	delete hist;
}

void 
list_history(GtkWidget *widget, ICQUser *user)
{
	struct history *hist = new history;
  if (!user->GetHistory(hist->hlist)) {
	  GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "History for %s could not be loaded.",
                                    user->GetAlias());
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    delete hist;
		return;
  }
	hist->uin = user->Uin();

	// Make the window
	gchar *title = g_strdup_printf("History with %s", user->GetAlias());
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), title);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  g_free(title);
	g_signal_connect(G_OBJECT(window), "destroy",
      G_CALLBACK(hist_destroy_cb), hist);
	
	// Make the boxes
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);
	GtkWidget *h_box = gtk_hbox_new(FALSE, 5);

	// Make the scrolled window
	GtkWidget *scroll = gtk_scrolled_window_new(0, 0);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
      GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scroll, 300, 225);

	// Make the text box
	GtkTextBuffer *tb = text_buffer_create(hist);
  if (hist->hlist.size() == 0)
		gtk_text_buffer_set_text(tb, "No history for this contact.", -1);
	hist->text = gtk_text_view_new_with_buffer(tb);
	g_object_unref(G_OBJECT(tb));

	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(hist->text), GTK_WRAP_WORD);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(hist->text), FALSE);
	gtk_container_add(GTK_CONTAINER(scroll), hist->text);

	// Pack the scrolled window
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_add(GTK_CONTAINER(frame), scroll);
	gtk_box_pack_start(GTK_BOX(v_box), frame, TRUE, TRUE, 5);
  
  // Add the loading progress bar
  hist->progress = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(v_box), hist->progress, FALSE, FALSE, 5);
  
	// The close button
	GtkWidget *button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(window_close), window);

	// The reverse check button
	hist->reverse = gtk_check_button_new_with_mnemonic("_Reverse");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hist->reverse), TRUE);
	g_signal_connect(G_OBJECT(hist->reverse), "toggled",
      G_CALLBACK(reverse_cb), hist);

	// Pack them
	gtk_box_pack_start(GTK_BOX(h_box), hist->reverse, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(h_box), button, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(v_box), h_box, FALSE, FALSE, 0);
	
	HistoryList hist_list;
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	gtk_container_add(GTK_CONTAINER(window), v_box);
	gtk_widget_show_all(window);

  hist->hlist_iter = hist->hlist.end();
  hist->loaded = 0;

  if (hist->hlist.size() == 0)
		gtk_widget_set_sensitive(hist->reverse, FALSE);
	else {
		if (load_history(hist))
    	hist->timeout_id = 
					g_timeout_add(time_slice, GSourceFunc(load_history), hist);
		else {
			hist->timeout_id = 0;
	  	if (hist->hlist.size() == 1)
				gtk_widget_set_sensitive(hist->reverse, FALSE);
		}
	}
}
