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

#include "licq_plugin.h"
#include "licq_log.h"

#include <sys/time.h>
#include <gtk/gtk.h>

CICQDaemon *icq_daemon;
gint _pipe;
CPluginLog *logg;
gint log_pipe;
GSList *catcher;

void log_pipe_callback(gpointer, gint, GdkInputCondition);
void registration_wizard();

const char *LP_Name()
{
	static const char name[] = "Jon's GTK2 GUI";
	return name;
}

const char *LP_Version()
{
	static const char version[] = "0.01";
	return version;
} 

const char *LP_Status()
{
	static const char status[] = "Running";
	return status;
}

const char *LP_Usage()
{
	static const char usage[] = "Usage: licq [ options ] -p jons-gtk2-gui";
	return usage;
}

const char *LP_Description()
{
	static const char desc[] = "Jon's GTK2 plugin for licq";
	return desc;
}

const char *LP_ConfigFile()
{
	return "licq_jons-gtk2-gui.conf";
}

bool LP_Init(int argc, char **argv)
{
	gtk_set_locale();
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
	gchar *title = g_strdup_printf("Licq (%ld)", owner->Uin());

	/* Do we need to register a new user? */
	if(owner->Uin() == 0)
		registration_wizard();

	main_window = main_window_new(title);	
	main_window_show();

	gUserManager.DropOwner();
	
	/* Attach plugin signals to a callback */
	gtk_input_add_full(_Pipe, GDK_INPUT_READ, pipe_callback, 
			NULL, (gpointer)0, NULL);

	/* The log window */
	logg = new CPluginLog();
	gtk_input_add_full(logg->Pipe(), GDK_INPUT_READ,
			         log_pipe_callback, NULL, (gpointer)0, NULL);
	gLog.AddService(new CLogService_Plugin(logg,
					L_INFO | L_WARN | L_ERROR | L_UNKNOWN));

	/* Start the event loop */
	gtk_main();

	icq_daemon->icqLogoff();
	icq_daemon->UnregisterPlugin();
	gLog.ModifyService(S_PLUGIN, 0);

	return 0;
}

const char *config_file()
{
	return LP_ConfigFile();
}
