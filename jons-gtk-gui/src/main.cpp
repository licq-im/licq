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

#include "licq_plugin.h"
#include "licq_icqd.h"
#include "licq_log.h"
#include "licq_user.h"

#include <sys/time.h>
#include <gtk/gtk.h>

GtkWidget *main_window;
CICQDaemon *icq_daemon;
gint _pipe;
CPluginLog *log;
gint log_pipe;
struct timeval timer;
GSList *catcher;

const char *LP_Name()
{
	return NAME;
}

const char *LP_Version()
{
	return PLUGIN_VERSION;
} 

const char *LP_Status()
{
	return STATUS;
}

const char *LP_Usage()
{
	return USAGE;
}

const char *LP_Description()
{
	return DESCRIPTION;
}

bool LP_Init(int argc, char **argv)
{
	return( gtk_init_check(&argc, &argv) );
}

int LP_Main(CICQDaemon *icqdaemon)
{
	int _Pipe;
	icq_daemon = icqdaemon;

	/* Register the plugin */
	_Pipe = icq_daemon->RegisterPlugin(SIGNAL_ALL);

	ICQOwner *owner = gUserManager.FetchOwner(LOCK_R);

	/* Get the title for the main window */
	gchar *title = 
	   g_strdup_printf("%ld", owner->Uin());

	/* Do we need to register a new user? */
	if(owner->Uin() == 0)
	{
		registration_wizard();
	}

	else
	{
		main_window = main_window_new(title, 445, 200);	
		main_window_show();
		contact_list_refresh();
		system_status_refresh();
		status_bar_refresh();
	}

	gUserManager.DropOwner();
	
	/* Start the timer for fixing that stupid problem w/ contact list */
	timer.tv_sec = 0;
	timer.tv_usec = 0;

	/* Attach plugin signals to a callback */
	gdk_input_add( _Pipe, GDK_INPUT_READ, pipe_callback, (gpointer)NULL);

	/* The log window */
	log = new CPluginLog();
	log_pipe = gdk_input_add(log->Pipe(), GDK_INPUT_READ,
			         log_pipe_callback, (gpointer)NULL);
	gLog.AddService(new CLogService_Plugin(log,
					L_INFO | L_WARN | L_ERROR | L_UNKNOWN));

	/* Start the event loop */
	gtk_main();

	gtk_widget_destroy(main_window);

	icq_daemon->icqLogoff();

	icq_daemon->UnregisterPlugin();

	return 0;
}
