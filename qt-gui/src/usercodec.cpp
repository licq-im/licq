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
#ifdef USE_KDE
#include <kglobal.h>
#include <kcharsets.h>
#endif

#include "licq_user.h"
#include "licq_chat.h"

QStringList* UserCodec::m_encodings;

// -----------------------------------------------------------------------------

#ifndef USE_KDE
const char * UserCodec::encodings_array[][2] = {
  { QT_TR_NOOP("Unicode"), "UTF-8" },
  { QT_TR_NOOP("Western European"), "ISO 8859-1" },
  { QT_TR_NOOP("Western European"), "ISO 8859-15" },
  { QT_TR_NOOP("Western European"), "CP 1252" },
  { QT_TR_NOOP("Central European"), "CP 1250" },
  { QT_TR_NOOP("Central European"), "ISO 8859-2" },
  { QT_TR_NOOP("Esperanto"), "ISO 8859-3" },
  { QT_TR_NOOP("Baltic"), "ISO 8859-13" },
  { QT_TR_NOOP("Baltic"), "CP 1257" },
  { QT_TR_NOOP("Cyrillic"), "ISO 8859-5" },
  { QT_TR_NOOP("Cyrillic"), "KOI8-R" },
  { QT_TR_NOOP("Cyrillic"), "CP 1251" },
  { QT_TR_NOOP("Arabic"), "ISO 8859-6-I" },
  { QT_TR_NOOP("Greek"), "ISO 8859-7" },
  { QT_TR_NOOP("Hebrew"), "ISO 8859-8-I" },
  { QT_TR_NOOP("Chinese Traditional"), "Big5" },
  { QT_TR_NOOP("Chinese Simplified"), "gbk" },
  { QT_TR_NOOP("Chinese Simplified"), "gbk2312" },
  { QT_TR_NOOP("Turkish"), "ISO 8859-9" },
  { QT_TR_NOOP("Turkish"), "CP 1254" }
};
#endif

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

QString UserCodec::encodingForIndex(uint index) {
  if ( !m_encodings ) UserCodec::initializeEncodingNames();

#ifdef USE_KDE
  return KGlobal::charsets()->encodingForName(( *m_encodings )[index]);
#else
  return encodings_array[index][1];
#endif
}

QStringList UserCodec::encodings()
{
  if ( !m_encodings ) UserCodec::initializeEncodingNames();
  return ( *m_encodings );
}

void UserCodec::initializeEncodingNames()
{
  if (!m_encodings) {
    m_encodings = new QStringList;
#ifdef USE_KDE
    ( *m_encodings ) = KGlobal::charsets()->descriptiveEncodingNames();
#else
    for (uint i=0; i<(sizeof(encodings_array)/sizeof(encodings_array[0])); i++) {
      ( *m_encodings ).append(qApp->translate("UserCodec", encodings_array[i][0]) + " ( " + encodings_array[i][1] + " )");
    }
#endif
  }
}

QString UserCodec::encodingForName(QString descriptiveName)
{
#ifdef USE_KDE
  return KGlobal::charsets()->encodingForName(descriptiveName);
#else
  int left = descriptiveName.find( " ( " );
  return descriptiveName.mid( left + 3, descriptiveName.find( " )", left ) - left - 3 );
#endif
}
