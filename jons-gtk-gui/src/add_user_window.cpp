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

#include <stdlib.h>
#include "icqd.h"

#include <gtk/gtk.h>

void menu_system_add_user(GtkWidget *window, gpointer data)
{
	GtkWidget *ok;
	GtkWidget *cancel;
	GtkWidget *label;
	GtkWidget *h_box;
	GtkWidget *v_box;
	struct add_user *a = g_new0(struct add_user, 1);

	/* Make up the window for adding users */
	a->window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_title(GTK_WINDOW(a->window), "Licq - Add User");
	gtk_window_set_position(GTK_WINDOW(a->window), GTK_WIN_POS_CENTER);

	/* The main box */
	v_box = gtk_vbox_new(FALSE, 5);

	/* Work on the first hbox */
	h_box = gtk_hbox_new(FALSE, 5);
	label = gtk_label_new("Add Uin: ");
	a->entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(h_box), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(h_box), a->entry, TRUE, TRUE, 0);
	
	/* Add the first hbox to the top of the vbox */
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 5);

	/* Work on the send hbox */
	h_box = gtk_hbox_new(FALSE, 0);
	a->check_box = gtk_check_button_new_with_label("Alert User");
	gtk_box_pack_start(GTK_BOX(h_box), a->check_box, TRUE, TRUE, 0);

	/* Add the second hbox to the vbox */
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 0);

	/* Work on the third hbox, with the buttons */
	h_box = gtk_hbox_new(FALSE, 5);
	ok = gtk_button_new_with_label("Add");
	gtk_box_pack_start(GTK_BOX(h_box), ok, TRUE, TRUE, 10);
	cancel = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(h_box), cancel, TRUE, TRUE, 10);

	/* Add the third and final hbox to the vbox */
	gtk_box_pack_start(GTK_BOX(v_box), h_box, TRUE, TRUE, 5);

	/* Connect all the signals to functions */
	gtk_signal_connect(GTK_OBJECT(cancel), "clicked",
			   GTK_SIGNAL_FUNC(dialog_close), a->window);
	gtk_signal_connect(GTK_OBJECT(a->window), "destroy",
			   GTK_SIGNAL_FUNC(dialog_close), a->window);
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
			   GTK_SIGNAL_FUNC(add_user_callback), a);

	/* Show the widgets and grab the focus */
	gtk_container_add(GTK_CONTAINER(a->window), v_box);
	gtk_widget_show_all(a->window);
	gtk_window_set_focus(GTK_WINDOW(a->window), a->entry);
}

void add_user_callback(GtkWidget *widget, struct add_user *a)
{
	unsigned long uin =
		atol((const char *)gtk_entry_get_text(GTK_ENTRY(a->entry)));

	icq_daemon->AddUserToList(uin);

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(a->check_box)))
		icq_daemon->icqAlertUser(uin);

	gtk_widget_destroy(a->window);
}

void dialog_close(GtkWidget *widget, GtkWidget *destroy)
{
	gtk_widget_destroy(destroy);
}
