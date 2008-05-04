/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#include "eventdesc.h"

#include <QApplication>
#include <QString>

#include <licq_icq.h>
#include <licq_message.h>

using namespace LicqQtGui;

static const int MAX_EVENT = 26;

static const char* szEventTypes[27] =
{
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Plugin Event"),
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Message"),
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Chat Request"),
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "File Transfer"),
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "URL"),
  NULL,
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Authorization Request"),
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Authorization Refused"),
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Authorization Granted"),
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "System Server Message"),
  NULL,
  NULL,
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Added to Contact List"),
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Web Panel"),
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Email Pager"),
  NULL,
  NULL,
  NULL,
  NULL,
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "Contact List"),
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  QT_TRANSLATE_NOOP("LicqQtGui::EventDescription", "SMS")
};

QString LicqQtGui::EventDescription(const CUserEvent* e)
{
  QString desc;

  if (e->SubCommand() == ICQ_CMDxSUB_EMAILxALERT)
    desc = qApp->translate("LicqQtGui::EventDescription", "New Email Alert");
  else
    if (e->SubCommand() > MAX_EVENT ||
        szEventTypes[e->SubCommand()][0] == '\0')
      desc = qApp->translate("LicqQtGui::EventDescription", "Unknown Event");
    else
    {
      desc = qApp->translate("LicqQtGui::EventDescription", szEventTypes[e->SubCommand()]);
      if (e->IsCancelled())
        desc += ' ' + qApp->translate("LicqQtGui::EventDescription", "(cancelled)");
    }

  return desc;
}
