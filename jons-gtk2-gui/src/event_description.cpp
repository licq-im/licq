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

#include "licq_message.h"
#include "licq_icq.h"

const int MAX_EVENT = 26;

static const char *szEventTypes[27] = 
{
	"Plugin Event",
	"Message",
	"Chat Request",
	"File Transfer",
	"URL",
	"",
	"Authorization Request",
	"Authorization Refused",
	"Authorization Granted",
	"",
	"",
	"",
	"Added to Contact List",
	"Web Panel",
	"E-mail Pager",
	"",
	"",
	"",
	"",
	"Contact List",
	"",
	"",
	"",
	"",
	"",
	"",
	"User Info"
};

const char *event_description(CUserEvent *e)
{
	char *szDesc = new char[35];

	if (e->SubCommand() > MAX_EVENT ||
	    szEventTypes[e->SubCommand()][0] == '\0')
	{
		strcpy(szDesc, "Unknown Event");
	}
	else
	{
		strcpy(szDesc, szEventTypes[e->SubCommand()]);
		if (e->IsCancelled())
		{
			strcat(szDesc, " (cancelled)");
		}
	}

	return szDesc;
}
