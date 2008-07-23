// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2001-2006 Licq developers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "usercodec.h"
#include "licq_user.h"
#include <qapplication.h>
#include <qtextcodec.h>

#include "licq_user.h"
#include "licq_chat.h"

QString *uc_DefaultEncoding;

UserCodec::encoding_t UserCodec::m_encodings[] = {
  { QT_TR_NOOP("Unicode"), "UTF-8", 106, ENCODING_DEFAULT, true },
  { QT_TR_NOOP("Unicode-16"), "ISO-10646-UCS-2", 1000 , ENCODING_DEFAULT, true },

  { QT_TR_NOOP("Arabic"), "ISO 8859-6", 82, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Arabic"), "CP 1256", 2256, ENCODING_ARABIC, true },

  { QT_TR_NOOP("Baltic"), "ISO 8859-13", 109, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Baltic"), "CP 1257", 2257, ENCODING_BALTIC, true },

  { QT_TR_NOOP("Central European"), "ISO 8859-2", 5, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Central European"), "CP 1250", 2250, ENCODING_EASTEUROPE, true },

  { QT_TR_NOOP("Chinese"), "GBK", -2025, ENCODING_GB2312, false },
  { QT_TR_NOOP("Chinese Traditional"), "Big5", 2026, ENCODING_CHINESEBIG5, true },

  { QT_TR_NOOP("Cyrillic"), "ISO 8859-5", 8, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Cyrillic"), "KOI8-R", 2084, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Cyrillic"), "CP 1251", 2251, ENCODING_RUSSIAN, true },

  { QT_TR_NOOP("Esperanto"), "ISO 8859-3", 6, ENCODING_DEFAULT, false },
  
  { QT_TR_NOOP("Greek"), "ISO 8859-7", 10, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Greek"), "CP 1253", 2253, ENCODING_GREEK, true },
  
  // Visual Hebrew is avoided on purpose -- its not usable for communications
  { QT_TR_NOOP("Hebrew"), "ISO 8859-8-I", 85, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Hebrew"), "CP 1255", 2255, ENCODING_HEBREW, true },

  { QT_TR_NOOP("Japanese"), "Shift-JIS", 17, ENCODING_SHIFTJIS, true },
  { QT_TR_NOOP("Japanese"), "JIS7", 16, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Japanese"), "eucJP", 18, ENCODING_DEFAULT, false },

  { QT_TR_NOOP("Korean"), "eucKR", 38, ENCODING_DEFAULT, true },

  { QT_TR_NOOP("Western European"), "ISO 8859-1", 4, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Western European"), "ISO 8859-15", 111, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Western European"), "CP 1252", 2252, ENCODING_ANSI, true },

  { QT_TR_NOOP("Tamil"), "TSCII", 2028, ENCODING_DEFAULT, true },

  { QT_TR_NOOP("Thai"), "TIS-620", 2259, ENCODING_THAI, true },

  { QT_TR_NOOP("Turkish"), "ISO 8859-9", 12, ENCODING_DEFAULT, false },
  { QT_TR_NOOP("Turkish"), "CP 1254", 2254, ENCODING_TURKISH, true },

  { QT_TR_NOOP("Ukrainian"), "KOI8-U", 2088, ENCODING_DEFAULT, false },
  
  { 0, 0, 0, 0, false } // end marker
};

QTextCodec* UserCodec::defaultEncoding()
{
  QTextCodec *codec = QTextCodec::codecForName(uc_DefaultEncoding->latin1());
  if (codec)
    return codec;
  return QTextCodec::codecForLocale();
}

QTextCodec* UserCodec::codecForICQUser(ICQUser *u)
{
  const char* preferred_encoding = u->UserEncoding();

  if ( preferred_encoding && *preferred_encoding ) {
      if (QTextCodec * codec = QTextCodec::codecForName(preferred_encoding))
        return codec;
  }

  return defaultEncoding();
}

QTextCodec *UserCodec::codecForProtoUser(const char *szId, unsigned long nPPID)
{
  QTextCodec *codec = defaultEncoding();

  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  if (u)
  {
    codec = UserCodec::codecForICQUser(u);
    gUserManager.DropUser(u);
  }

  return codec;
}

QTextCodec *UserCodec::codecForCChatUser(CChatUser *u)
{
  if (nameForCharset(u->FontEncoding()) != QString::null)
    return QTextCodec::codecForName(nameForCharset(u->FontEncoding()));

  // return default encoding
  return codecForProtoUser(u->Id(), u->PPID());
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

unsigned char UserCodec::charsetForName(QString name)
{
  encoding_t *it = &m_encodings[0];
  while (it->encoding != NULL) {
     if (it->encoding == name)
       return it->charset;
     it++;
  }

  return ENCODING_DEFAULT;
}

QString UserCodec::nameForCharset(unsigned char charset)
{
  if (charset == ENCODING_DEFAULT)
    return QString::null;

  encoding_t *it = &m_encodings[0];
  while (it->encoding != NULL) {
     if (it->charset == charset)
       return QString::fromLatin1(it->encoding);
     it++;
  }

  return QString::null;
}
