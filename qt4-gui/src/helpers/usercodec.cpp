/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2001-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "usercodec.h"

#include <QApplication>

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserCodec */

UserCodec::encoding_t UserCodec::m_encodings[] = {
  { QT_TR_NOOP("Unicode"), "UTF-8", true },
  { QT_TR_NOOP("Unicode-16"), "ISO-10646-UCS-2", true },

  { QT_TR_NOOP("Arabic"), "ISO 8859-6", false },
  { QT_TR_NOOP("Arabic"), "CP 1256", true },

  { QT_TR_NOOP("Baltic"), "ISO 8859-13", false },
  { QT_TR_NOOP("Baltic"), "CP 1257", true },

  { QT_TR_NOOP("Central European"), "ISO 8859-2", false },
  { QT_TR_NOOP("Central European"), "CP 1250", true },

  { QT_TR_NOOP("Chinese"), "GBK", false },
  { QT_TR_NOOP("Chinese Traditional"), "Big5", true },

  { QT_TR_NOOP("Cyrillic"), "ISO 8859-5", false },
  { QT_TR_NOOP("Cyrillic"), "KOI8-R", false },
  { QT_TR_NOOP("Cyrillic"), "CP 1251", true },

  { QT_TR_NOOP("Esperanto"), "ISO 8859-3", false },

  { QT_TR_NOOP("Greek"), "ISO 8859-7", false },
  { QT_TR_NOOP("Greek"), "CP 1253", true },

  // Visual Hebrew is avoided on purpose -- its not usable for communications
  { QT_TR_NOOP("Hebrew"), "ISO 8859-8-I", false },
  { QT_TR_NOOP("Hebrew"), "CP 1255", true },

  { QT_TR_NOOP("Japanese"), "Shift-JIS", true },
  { QT_TR_NOOP("Japanese"), "JIS7", false },
  { QT_TR_NOOP("Japanese"), "eucJP", false },

  { QT_TR_NOOP("Korean"), "eucKR", true },

  { QT_TR_NOOP("Western European"), "ISO 8859-1", false },
  { QT_TR_NOOP("Western European"), "ISO 8859-15", false },
  { QT_TR_NOOP("Western European"), "CP 1252", true },

  { QT_TR_NOOP("Tamil"), "TSCII", true },

  { QT_TR_NOOP("Thai"), "TIS-620", true },

  { QT_TR_NOOP("Turkish"), "ISO 8859-9", false },
  { QT_TR_NOOP("Turkish"), "CP 1254", true },

  { QT_TR_NOOP("Ukrainian"), "KOI8-U", false },

  { 0, 0, false } // end marker
};

QString UserCodec::nameForEncoding(int i)
{
  return qApp->translate("UserCodec", m_encodings[i].script) + " ( " + m_encodings[i].encoding + " )";
}

int UserCodec::encodingForName(const QString& name)
{
  for (int i = 0; m_encodings[i].encoding != NULL; ++i)
    if (m_encodings[i].encoding == name)
      return i;
  return 0;
}
