// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2001-2009 Licq developers
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

#ifndef USERCODEC_H
#define USERCODEC_H

#include <qobject.h>

#include <licq_types.h>

class QTextCodec;
class LicqUser;
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

  // Retrieves the default codec
  static QTextCodec * defaultEncoding();
  // Retrieves the codec for an LicqUser object
  static QTextCodec* codecForICQUser(const LicqUser* u);
  // Retrieves the codec for an CChatUser object
  static const QTextCodec* codecForCChatUser(CChatUser* u);
  // Retrieves the codec for a proto user
  static const QTextCodec* codecForUserId(const UserId& userId);

  static QString encodingForMib(int mib);
  static QString nameForEncoding(const QString &encoding);
  static QString encodingForName(const QString &descriptiveName);
  static unsigned char charsetForName(QString name);
  static QString nameForCharset(unsigned char charset);

public:
  static encoding_t m_encodings[];
};

extern QString *uc_DefaultEncoding;

#endif
