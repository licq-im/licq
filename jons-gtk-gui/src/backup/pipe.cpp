#include "pipe.h"

#include "icqd.h"

#include <gtk/gtk.h>

void pipe_callback(gpointer data, gint _Pipe, GdkInputCondition condition)
{
	char buf[16];
	read(_Pipe, buf, 1);

	switch(buf[0])
	{
	  case 'S':   /* It's a signal */
	  {
		s = icq_daemon->PopPluginSignal();
		pipe_signal(s);
		break;
	  }

	  case 'E': /* It's an event */
	  {
		e = icq_daemon->PopPluginEvent();
		pipe_event(e);
		break;
	  }

	  default: /* What is it.....? */
		g_print("Unknown signal from daemon: %c.\n", buf[0]);
	}
}

void pipe_signal(CICQSignal *sig)
{
	switch(sig->Signal())
	{
	  case SIGNAL_LOGON:
	  {
		contact_list_refresh();
		break;
	  }

	  case SIGNAL_UPDATExLIST:
	  {
		contact_list_refresh();
		break;
	  }

	  case SIGNAL_UPDATExUSER:
	  {
		contact_list_refresh();
		break;
	  }

	  default:
		g_print("Error: Unknown signal type: %s.", sig->Signal());
	}
}

void pipe_event(ICQEvent *eve)
{
}
