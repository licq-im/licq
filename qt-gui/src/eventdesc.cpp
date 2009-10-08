/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
 *
 * Licq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Licq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Licq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <qobject.h>
#include <qstring.h>
#include <qapplication.h>

#include "eventdesc.h"
#include "licq_message.h"
#include "licq_icq.h"


static const int MAX_EVENT = 26;

static const char *szEventTypes[27] =
{ QT_TRANSLATE_NOOP("Event","Plugin Event"),
  QT_TRANSLATE_NOOP("Event","Message"),
  QT_TRANSLATE_NOOP("Event","Chat Request"),
  QT_TRANSLATE_NOOP("Event","File Transfer"),
  QT_TRANSLATE_NOOP("Event","URL"),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event","Authorization Request"),
  QT_TRANSLATE_NOOP("Event","Authorization Refused"),
  QT_TRANSLATE_NOOP("Event","Authorization Granted"),
  QT_TRANSLATE_NOOP("Event","System Server Message"),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event","Added to Contact List"),
  QT_TRANSLATE_NOOP("Event","Web Panel"),
  QT_TRANSLATE_NOOP("Event","Email Pager"),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event","Contact List"),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event",""),
  QT_TRANSLATE_NOOP("Event","SMS")
};

QString EventDescription(const CUserEvent* e)
{
  QString desc;
  if (e->SubCommand() == ICQ_CMDxSUB_EMAILxALERT)
    desc = "New Email Alert";
  else if (e->SubCommand() > MAX_EVENT ||
      szEventTypes[e->SubCommand()][0] == '\0')
    desc = qApp->translate("Event", "Unknown Event");
  else
  {
    desc = qApp->translate("Event", szEventTypes[e->SubCommand()]);
    if (e->IsCancelled())
      desc += ' ' + qApp->translate("Event", "(cancelled)");
  }
  return desc;
}

