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

#ifndef USERCODEC_H
#define USERCODEC_H

#include <qobject.h>

class QTextCodec;
class ICQUser;
class CChatUser;

class UserCodec {
public:
  static QTextCodec * codecForICQUser(ICQUser *u);
  static QTextCodec * codecForCChatUser(CChatUser *u);
  static QString encodingForIndex(uint index);
  static QString encodingForName(QString descriptiveName);
#ifndef USE_KDE
  static const char * encodings_array[][2];
#endif
  static QStringList encodings();
private:
  static QStringList* m_encodings;
  static void initializeEncodingNames();
};      

#endif
