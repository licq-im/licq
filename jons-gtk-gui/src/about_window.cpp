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

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "licq_gtk.h"
#include "licq_icqd.h"

#include <sys/utsname.h>

GtkWidget *about_window;

void create_about_window()
{
	// Only one about window
	if(about_window != NULL)
	{
		gdk_window_raise(about_window->window);
		return;
	}
	
	// Make the window now
	about_window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_window_set_position(GTK_WINDOW(about_window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(about_window), "About Licq");
	gtk_widget_set_usize(about_window, 225, 180);

	// Window delete event
	gtk_signal_connect(GTK_OBJECT(about_window), "destroy",
		GTK_SIGNAL_FUNC(close_about_window), NULL);

	// A vertical box for the label and ok button
	GtkWidget *v_box = gtk_vbox_new(false, 5);
	gtk_container_add(GTK_CONTAINER(about_window), v_box);

	ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
	struct utsname un;
	uname(&un);

	// A label to be in the box.. with centered text
	GtkWidget *label = gtk_label_new(
		g_strdup_printf("Licq Version: %s\n"
			 "Jon's GTK+ GUI Plugin Version: %s\n"
			 "GTK+ Version: %d.%d.%d\nCompiled On: %s\n"
			 "Currently Running On: %s %s [%s]\n\n"
			 "Author: Jon Keating <jon@licq.org>\n"
			 "http://jons-gtk-gui.sourceforge.net\n\n"
			 "%s (%ld)\n"
			 "%d Contacts", icq_daemon->Version(), VERSION,
			 gtk_major_version, gtk_minor_version,
			 gtk_micro_version, __DATE__,
			 un.sysname, un.release, un.machine,
			 o->GetAlias(), o->Uin(), gUserManager.NumUsers()));

	gUserManager.DropOwner();

	gtk_box_pack_start(GTK_BOX(v_box), label, false, false, 0);

	GtkWidget *ok = gtk_button_new_with_label("OK");
	gtk_signal_connect(GTK_OBJECT(ok), "clicked",
		GTK_SIGNAL_FUNC(close_about_window), NULL);
	gtk_box_pack_start(GTK_BOX(v_box), ok, true, true, 0);

	gtk_widget_show_all(about_window);
}

void close_about_window(GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(about_window);
	about_window = NULL;
}
