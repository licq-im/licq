#include "main.h"

#include "plugin.h"
#include "icqd.h"
#include "user.h"

#include <gtk/gtk.h>

const char *LP_Name()
{
	return NAME;
}

const char *LP_Version()
{
	return VERSION;
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
	   g_strdup_printf("%s - %ld", owner->GetAlias(), owner->Uin());

	gUserManager.DropOwner();

	icq_daemon->icqLogon(ICQ_STATUS_ONLINE);

	main_window = main_window_new(title, 400, 200);	

	main_window_show();

	/* Attach plugin signals to a callback */
	gdk_input_add( _Pipe, GDK_INPUT_READ, pipe_callback, (gpointer)NULL);

	/* Start the event loop */
	gtk_main();

	icq_daemon->UnregisterPlugin();

	return 0;
}
