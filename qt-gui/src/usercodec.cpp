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
#ifdef USE_KDE
#include <kglobal.h>
#include <kcharsets.h>
#endif

QStringList UserCodec::encodings;

// -----------------------------------------------------------------------------

#ifndef USE_KDE
const char * UserCodec::encodings_array[][2] = {
  { QT_TR_NOOP("Unicode"), "UTF-8" },
  { QT_TR_NOOP("Western European"), "ISO 8859-1" },
  { QT_TR_NOOP("Western European"), "ISO 8859-15" },
  { QT_TR_NOOP("Central European"), "ISO 8859-2" },
  { QT_TR_NOOP("Central European"), "ISO 8859-3" },
  { QT_TR_NOOP("Central European"), "ISO 8859-4" },
  { QT_TR_NOOP("Central European"), "ISO 8859-10" },
  { QT_TR_NOOP("Russian"), "ISO 8859-5" },
  { QT_TR_NOOP("Russian"), "KOI8-R" },
  { QT_TR_NOOP("Russian"), "CP 1251" },
  { QT_TR_NOOP("Arabic"), "ISO 8859-6-I" },
  { QT_TR_NOOP("Greek"), "ISO 8859-7" },
  { QT_TR_NOOP("Hebrew"), "ISO 8859-8-I" },
  { QT_TR_NOOP("Turkish"), "ISO 8859-9" },
  { QT_TR_NOOP("Turkish"), "CP 1254" }
};
#endif

QTextCodec* UserCodec::codecForICQUser(ICQUser *u) {

  char* preferred_charset = u->UserCharset();

  if ( preferred_charset && *preferred_charset ) {
      if (QTextCodec * codec = QTextCodec::codecForName(preferred_charset))
        return codec;
  }

  // resturn default charset
  return QTextCodec::codecForLocale();
}

QString UserCodec::encodingForIndex(uint index) {
#ifdef USE_KDE
  return KGlobal::charsets()->encodingForName(encodings[index]);
#else
  return encodings_array[index][1];
#endif
}

void UserCodec::initializeEncodingNames() {
  if (encodings.count() == 0) {
#ifdef USE_KDE
    encodings = KGlobal::charsets()->descriptiveEncodingNames();
#else
    for (uint i=0; i<(sizeof(encodings_array)/sizeof(encodings_array)); i++) {
      encodings.append(qApp->translate("UserCodec", encodings_array[i][0]) + " (" + encodings_array[i][1] + ")");
    }
#endif
  }
}
