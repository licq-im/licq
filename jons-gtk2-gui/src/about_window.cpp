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

void create_about_window()
{
	struct utsname un;
	uname(&un);

	ICQOwner *o = gUserManager.FetchOwner(LOCK_R);

  GtkWidget *about =
      gtk_message_dialog_new(
          GTK_WINDOW(main_window),
          GTK_DIALOG_DESTROY_WITH_PARENT,
          GTK_MESSAGE_INFO,
          GTK_BUTTONS_OK,
          "Licq Version: %s\n"
			    "Jon's GTK2 GUI Version: %s\n"
			    "GTK+ Version: %d.%d.%d\n"
			    "Compiled On: %s\n"
			    "Currently Running On: %s %s [%s]\n\n"
			    "Author: Jon Keating <jon@licq.org>\n"
			    "http://jons-gtk-gui.sourceforge.net\n\n"
			    "%s (%ld)\n"
			    "%d Contacts",
			    icq_daemon->Version(), VERSION,
			    gtk_major_version, gtk_minor_version,
			    gtk_micro_version, __DATE__,
			    un.sysname, un.release, un.machine,
			    o->GetAlias(), o->Uin(), gUserManager.NumUsers());

	gUserManager.DropOwner();

  gtk_dialog_run(GTK_DIALOG(about));
  gtk_widget_destroy(about);
}
