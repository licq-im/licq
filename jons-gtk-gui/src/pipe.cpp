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

#include "licq_icqd.h"

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

	  case 'X': /* Shutdown */
	  {
		gtk_main_quit();
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
		status_bar_refresh();
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
		if(sig->SubSignal() == USER_EVENTS)
		{
			ICQUser *u = gUserManager.FetchUser(sig->Uin(), LOCK_R);
			if(u == NULL)
			{
				gUserManager.DropUser(u);
				return;
			}
			CUserEvent *ue = u->EventPeekLast();
			gUserManager.DropUser(u);
			
			if(ue == NULL)
			{
				gUserManager.DropUser(u);
				return;
			}
			else if(ue->SubCommand() == ICQ_CMDxSUB_CHAT &&
				u->AutoChatAccept())
			{
				ue = u->EventPop();
				gUserManager.DropUser(u);
				chat_accept_window((CEventChat *)ue, sig->Uin(),
					true);
				return;
				
			}
			else if(ue->SubCommand() == ICQ_CMDxSUB_FILE &&
				u->AutoFileAccept())
			{
				ue = u->EventPop();
				file_accept_window(u, ue, true);
				gUserManager.DropUser(u);
				return;
			}

			gUserManager.DropUser(u);
			convo_recv(sig->Uin());
		}
		else
			finish_info(sig);
		contact_list_refresh();
		break;
	  }

	  case SIGNAL_LOGOFF:  break;

	  default:
		g_print("Error: Unknown signal type: %ld.", sig->Signal());
	}

	//delete sig;
}

void pipe_event(ICQEvent *event)
{
	switch(event->Command())
	{
	/* Event commands for a user */
	case ICQ_CMDxTCP_START:
	case ICQ_CMDxSND_THRUxSERVER:
	case ICQ_CMDxSND_USERxGETINFO:
	case ICQ_CMDxSND_USERxGETDETAILS:
	case ICQ_CMDxSND_UPDATExDETAIL:
	case ICQ_CMDxSND_UPDATExBASIC:
	case ICQ_CMDxSND_META:
	case ICQ_CMDxSND_RANDOMxSEARCH:
	case ICQ_CMDxSND_SETxRANDOMxCHAT:
		user_function(event);
		break;

	case ICQ_CMDxSND_LOGON:
	case ICQ_CMDxSND_USERxLIST:
	case ICQ_CMDxSND_REGISTERxUSER:
		if(event->Command() != ICQ_CMDxSND_REGISTERxUSER)
			contact_list_refresh();
		owner_function(event);
		break;

	case ICQ_CMDxSND_SETxSTATUS:
	case ICQ_CMDxSND_USERxADD:
		status_bar_refresh();
		break;

	case ICQ_CMDxSND_SEARCHxINFO:
	case ICQ_CMDxSND_SEARCHxUIN:
		search_result(event);
		break;

	default:
		gLog.Warn("%sInternal Error: pipe_event(): Unknown event from daemon: %d.\n", L_WARNxSTR, event->Command());
	}

	delete event;
}
