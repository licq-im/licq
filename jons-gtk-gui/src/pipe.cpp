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
		CICQSignal *s = icq_daemon->PopPluginSignal();
		pipe_signal(s);
		break;
	  }

	  case 'E': /* It's an event */
	  {
		ICQEvent *e = icq_daemon->PopPluginEvent();
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
		g_print("GOT SIGNAL_LOGON\n");
		status_bar_refresh();
		contact_list_refresh();
		break;
	  }

	  case SIGNAL_UPDATExLIST:
	  {
		g_print("GOT SIGNAL_UPDATExLIST\n");
		contact_list_refresh();
		break;
	  }

	  case SIGNAL_UPDATExUSER:
	  {
		g_print("GOT SIGNAL_UPDATExUSER\n");
		if(sig->SubSignal() == USER_EVENTS)
			convo_recv(sig->Uin());
		contact_list_refresh();
		break;
	  }

	  default:
		g_print("Error: Unknown signal type: %ld.", sig->Signal());
	}

	delete sig;
}

void pipe_event(ICQEvent *event)
{
	switch(event->m_nCommand)
	{
	/* Event commands for a user */
	case ICQ_CMDxTCP_START:
	case ICQ_CMDxSND_THRUxSERVER:
	//case ICQ_CMDxSND_USERxGETINFO:
	//case ICQ_CMDxSND_USERxGETDETAILS:
	//case ICQ_CMDxSND_UPDATExDETAIL:
	//case ICQ_CMDxSND_UPDATExBASIC:
	//case ICQ_CMDxSND_META:
		break;
	case ICQ_CMDxSND_USERxLIST:
		g_print("GOT ICQ_CMDxSND_USERxLIST\n");
		contact_list_refresh();
		break;

	case ICQ_CMDxSND_SETxSTATUS:
	case ICQ_CMDxSND_USERxADD:
		status_bar_refresh();
		break;

	default:
		g_print("Error: pipe_event(): Unknown event from daemon: %d.\n", event->m_nCommand);
	}

	delete event;
}
