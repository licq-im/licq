/**
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

	switch (buf[0]) {
	  case 'S':   /* It's a signal */
			pipe_signal(icq_daemon->PopPluginSignal());
			break;

	  case 'E': /* It's an event */
			pipe_event(icq_daemon->PopPluginEvent());
			break;

	  case 'X': /* Shutdown */
			gtk_main_quit();
			break;

	  case '0':
		case '1':
			break;

		default: /* What is it.....? */
			gLog.Warn("%sUnknown notification type from daemon: %c.\n", 
					L_WARNxSTR, buf[0]);
	}
}

void
update_user(CICQSignal *sig)
{
	if (sig->SubSignal() == USER_EVENTS) {
		ICQUser *u = gUserManager.FetchUser(sig->Uin(), LOCK_R);
		if (u == NULL) {
			gUserManager.DropUser(u);
			return;
		}
		CUserEvent *ue = u->EventPeekLast();
		gUserManager.DropUser(u);

		if (ue == NULL) {
			gUserManager.DropUser(u);
			return;
		}
		else if (ue->SubCommand() == ICQ_CMDxSUB_CHAT && u->AutoChatAccept()) {
			ue = u->EventPop();
			gUserManager.DropUser(u);
			chat_accept_window((CEventChat *)ue, sig->Uin(), true);
			return;
		}
		else if(ue->SubCommand() == ICQ_CMDxSUB_FILE &&	u->AutoFileAccept()) {
			ue = u->EventPop();
			file_accept_window(u, ue, true);
			gUserManager.DropUser(u);
			return;
		}

		gUserManager.DropUser(u);
		convo_recv(sig->Uin());
	}
	else {
		finish_info(sig);

		if (sig->Uin() == gUserManager.OwnerUin())
			status_bar_refresh();
	}
	contact_list_refresh();
}

void pipe_signal(CICQSignal *sig)
{
	switch (sig->Signal()) {
	  case SIGNAL_UPDATExLIST:
			contact_list_refresh();
			break;

	  case SIGNAL_UPDATExUSER:
			update_user(sig);
			break;

	  case SIGNAL_LOGON:
			status_bar_refresh();
			contact_list_refresh();
			break;

	  case SIGNAL_LOGOFF:
			break;

		case SIGNAL_UI_VIEWEVENT:
		case SIGNAL_UI_MESSAGE:
			// no clue what these do...
			break;
			
		case SIGNAL_ADDxSERVERxLIST:
			icq_daemon->icqRenameUser(sig->Uin());
			break;
			
	  default:
			gLog.Warn("%snknown signal received from the daemon: %ld", 
					L_WARNxSTR, sig->Signal());
	}

	delete sig;
}

void pipe_event(ICQEvent *event)
{
	if (event->Command() == ICQ_CMDxTCP_START) { // direct connection check
		user_function(event);
		delete event;
		return;
	}

	switch(event->SNAC()) {
		/* Event commands for a user */
		case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxMESSAGE):
		case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxREPLYxMSG):
		case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER):
			user_function(event);
			break;

		case MAKESNAC(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA):
			if (event->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxLAST_USER ||
					event->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxFOUND)
				search_result(event);
			else if (event->SubCommand() == ICQ_CMDxSND_SYSxMSGxREQ ||
							event->SubCommand() == ICQ_CMDxSND_SYSxMSGxDONExACK)
				owner_function(event);
			else
				user_function(event);
      break;

		case ICQ_CMDxSND_LOGON:
		case ICQ_CMDxSND_USERxLIST:
		case ICQ_CMDxSND_REGISTERxUSER:
			if (event->Command() != ICQ_CMDxSND_REGISTERxUSER)
				contact_list_refresh();
			owner_function(event);
			break;

		case MAKESNAC(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS):
		case MAKESNAC(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST):
			status_bar_refresh();
			break;

		default:
			gLog.Warn("%sInternal Error: pipe_event(): Unknown event from daemon: 0x%08lX.\n", L_WARNxSTR, event->SNAC());
	}

	delete event;
}
