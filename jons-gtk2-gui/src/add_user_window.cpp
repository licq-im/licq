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

#include <stdlib.h>
#include <gtk/gtk.h>

struct add_user {
      GtkWidget *window;
      GtkWidget *entry;
      GtkWidget *check_box;
};

void
window_close(GtkWidget *widget, GtkWidget *destroy)
{
	gtk_widget_destroy(destroy);
}

void
destroy_cb(GtkWidget *widget, gpointer **p)
{
  if (p != NULL && *p != NULL) {
		g_free(*p);
		*p = NULL;
	}
}

static void
add_user_callback(GtkWidget *widget, struct add_user *au)
{
	unsigned long uin =	strtoul(entry_get_chars(au->entry).c_str(), NULL, 10);


  if (uin != 0) {
		icq_daemon->AddUserToList(uin);

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(au->check_box)))
			icq_daemon->icqAlertUser(uin);
  	
  	window_close(NULL, au->window);
	}
  else {
	  GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(au->window),
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "Invalid UIN entered: %s",
                                    entry_get_chars(au->entry).c_str());
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
  	gtk_window_set_focus(GTK_WINDOW(au->window), au->entry);
  }
}

void
menu_system_add_user(GtkWidget *window, gpointer data)
{
	static struct add_user *au = NULL;
	
	if (au == NULL)
		au = g_new0(struct add_user, 1);
	else {
		gtk_window_present(GTK_WINDOW(au->window));
		return;
	}

	/* Make up the window for adding users */
	au->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
	gtk_window_set_title(GTK_WINDOW(au->window), "Licq - Add User");
	gtk_window_set_position(GTK_WINDOW(au->window), GTK_WIN_POS_CENTER);

	/* The main box */
	GtkWidget *v_box = gtk_vbox_new(FALSE, 5);

	/* Work on the first hbox */
	GtkWidget *h_box = gtk_hbox_new(FALSE, 5);
	au->entry = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(au->entry), MAX_LENGTH_UIN);
	gtk_box_pack_start(GTK_BOX(h_box), gtk_label_new("Add Uin: "), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), au->entry, TRUE, TRUE, 0);

	/* Accept only numbers in the au->entry */
	g_signal_connect(G_OBJECT(au->entry), "insert-text",
	    G_CALLBACK(verify_numbers), 0);
	
	/* Add the first hbox to the top of the vbox */
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 5);

	/* Work on the send hbox */
	h_box = gtk_hbox_new(FALSE, 0);
	au->check_box = gtk_check_button_new_with_label("Alert User");
	gtk_box_pack_start(GTK_BOX(h_box), au->check_box, TRUE, TRUE, 0);

	/* Add the second hbox to the vbox */
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 0);

	/* Work on the third hbox, with the buttons */
	h_box = hbutton_box_new();
	GtkWidget *ok = gtk_button_new_with_mnemonic("_Add");
	GtkWidget *cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_container_add(GTK_CONTAINER(h_box), ok);
	gtk_container_add(GTK_CONTAINER(h_box), cancel);

	/* Add the third and final hbox to the vbox */
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 5);

	/* Connect all the signals to functions */
  g_signal_connect(G_OBJECT(cancel), "clicked",
			   G_CALLBACK(window_close), au->window);
	g_signal_connect(G_OBJECT(au->window), "destroy",
			   G_CALLBACK(destroy_cb), &au);
	g_signal_connect(G_OBJECT(ok), "clicked",
			   G_CALLBACK(add_user_callback), au);

	/* Show the widgets and grab the focus */
	gtk_container_set_border_width(GTK_CONTAINER(au->window), 10);
	gtk_container_add(GTK_CONTAINER(au->window), v_box);
	gtk_window_set_focus(GTK_WINDOW(au->window), au->entry);
	gtk_widget_show_all(au->window);
}
