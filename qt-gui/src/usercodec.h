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
  struct encoding_t {
	const char *script;
	const char *encoding;
	int mib;
	unsigned char charset;
	bool isMinimal;
  };

  // Retrieves the codec for an ICQUser object
  static QTextCodec * codecForICQUser(ICQUser *u);
  // Retrieves the codec for an CChatUser object
  static QTextCodec * codecForCChatUser(CChatUser *u);
  // Helper function to retrieve the codec for an UIN
  static QTextCodec * codecForUIN(uint uin);
  static QString encodingForMib(int mib);
  static QString nameForEncoding(const QString &encoding);
  static QString encodingForName(const QString &descriptiveName);
  static unsigned char charsetForName(QString name);
  static QString nameForCharset(unsigned char charset);

public:
  static encoding_t m_encodings[];
};      

#endif
