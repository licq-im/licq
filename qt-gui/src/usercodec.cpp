// -*- c-basic-offset: 2 -*-
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "usercodec.h"
#include "licq_user.h"
#include <qapplication.h>
#include <qtextcodec.h>

#include "licq_user.h"
#include "licq_chat.h"

UserCodec::encoding_t UserCodec::m_encodings[] = {
  { QT_TR_NOOP("Unicode"), "UTF-8", 106, true },

  { QT_TR_NOOP("Arabic"), "ISO 8859-6", 82, false },
  { QT_TR_NOOP("Arabic"), "CP 1256", 2256, true },

  { QT_TR_NOOP("Baltic"), "ISO 8859-13", 109, false },
  { QT_TR_NOOP("Baltic"), "CP 1257", 2257, true },

  { QT_TR_NOOP("Central European"), "ISO 8859-2", 5, false },
  { QT_TR_NOOP("Central European"), "CP 1250", 2250, true },

  { QT_TR_NOOP("Chinese"), "GBK", -2025, false },
  { QT_TR_NOOP("Chinese Traditional"), "Big5", 2026, true },

  { QT_TR_NOOP("Cyrillic"), "ISO 8859-5", 8, false },
  { QT_TR_NOOP("Cyrillic"), "KOI8-R", 2084, false },
  { QT_TR_NOOP("Cyrillic"), "CP 1251", 2251, true },

  { QT_TR_NOOP("Esperanto"), "ISO 8859-3", 6, false },
  
  { QT_TR_NOOP("Greek"), "ISO 8859-7", 10, false },
  { QT_TR_NOOP("Greek"), "CP 1253", 2253, true },
  
  // Visual Hebrew is avoided on purpose -- its not usable for communications
  { QT_TR_NOOP("Hebrew"), "ISO 8859-8-I", 85, false },
  { QT_TR_NOOP("Hebrew"), "CP 1255", 2255, true },

  { QT_TR_NOOP("Japanese"), "Shift-JIS", 17, true },
  { QT_TR_NOOP("Japanese"), "JIS7", 16, false },
  { QT_TR_NOOP("Japanese"), "eucJP", 18, false },

  { QT_TR_NOOP("Korean"), "eucKR", 38, true },

  { QT_TR_NOOP("Western European"), "ISO 8859-1", 4, false },
  { QT_TR_NOOP("Western European"), "ISO 8859-15", 111, false },
  { QT_TR_NOOP("Western European"), "CP 1252", 2252, true },

  { QT_TR_NOOP("Tamil"), "TSCII", 2028, true },

  { QT_TR_NOOP("Thai"), "TIS-620", 2259, true },

  { QT_TR_NOOP("Turkish"), "ISO 8859-9", 12, false },
  { QT_TR_NOOP("Turkish"), "CP 1254", 2254, true },

  { QT_TR_NOOP("Ukrainian"), "KOI8-U", 2088, false },
  
  { 0, 0, 0, false } // end marker
};

QTextCodec* UserCodec::codecForICQUser(ICQUser *u)
{
  char *preferred_encoding = u->UserEncoding();

  if ( preferred_encoding && *preferred_encoding ) {
      if (QTextCodec * codec = QTextCodec::codecForName(preferred_encoding))
        return codec;
  }

  // return default encoding
  return QTextCodec::codecForLocale();
}

QTextCodec* UserCodec::codecForUIN(uint uin)
{
  QTextCodec *codec = QTextCodec::codecForLocale();
  
  ICQUser *u = gUserManager.FetchUser(uin, LOCK_R);
  if (u != NULL)
  {
     codec = UserCodec::codecForICQUser(u);
     gUserManager.DropUser(u);
  }
  
  return codec;
}

QTextCodec *UserCodec::codecForCChatUser(CChatUser *u)
{
  if (ICQUser *user = gUserManager.FetchUser(u->Uin(), LOCK_R)) {
    QTextCodec *codec = UserCodec::codecForICQUser(user);
    gUserManager.DropUser(user);
    return codec;
  }

  // return default encoding
  return QTextCodec::codecForLocale();
}

QString UserCodec::encodingForMib(int mib)
{
  encoding_t *it = &m_encodings[0];
  while (it->encoding != NULL) {
     if (it->mib == mib)
       return QString::fromLatin1(it->encoding);
     it++;
  }
  
  return QString::null;
}

QString UserCodec::nameForEncoding(const QString &encoding)
{
  encoding_t *it = &m_encodings[0];
  while (it->encoding != NULL) {
    if (QString::fromLatin1(it->encoding) == encoding)
      return qApp->translate("UserCodec", it->script) + " ( " + it->encoding + " )";
    it++;
  }
  
  return QString::null;
}

QString UserCodec::encodingForName(const QString &descriptiveName)
{
  int left = descriptiveName.find( " ( " );
  return descriptiveName.mid( left + 3, descriptiveName.find( " )", left ) - left - 3 );
}
