/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQICQ_CHAT_H
#define LICQICQ_CHAT_H

#include <licq/icq/chat.h>

#include <deque>
#include <list>
#include <pthread.h>
#include <string>

#include "socket.h"

namespace LicqIcq
{
class CPChat_ColorFond;
class User;

class CChatClient
{
public:
  CChatClient();
  CChatClient(const User* u);
  CChatClient(const CChatClient &);
  CChatClient& operator=(const CChatClient &);
  ~CChatClient();

  // Initialize from the handshake buffer (does not set the session
  // or port fields however)
  bool LoadFromHandshake_v2(Licq::Buffer&);
  bool LoadFromHandshake_v4(Licq::Buffer&);
  bool LoadFromHandshake_v6(Licq::Buffer&);
  bool LoadFromHandshake_v7(Licq::Buffer&);

  unsigned long m_nVersion;
  unsigned short m_nPort;
  unsigned long myUin;
  unsigned long m_nIp;
  unsigned long m_nIntIp;
  char m_nMode;
  unsigned short m_nSession;
  unsigned long m_nHandshake;
  unsigned short m_nId;

protected:
  CChatClient(Licq::Buffer&);
  bool LoadFromBuffer(Licq::Buffer&);

  friend class CPChat_ColorFont;
};

typedef std::list<CChatClient> ChatClientList;
typedef std::list<CChatClient *> ChatClientPList;


class ChatUser : public CChatUser
{
public:
  ChatUser();
  ~ChatUser();

  CChatClient* m_pClient;
  DcSocket sock;
  std::deque<unsigned char> chatQueue;
  unsigned short state;
  std::string myLinebuf;

  pthread_mutex_t mutex;
};

struct SChatReverseConnectInfo
{
  int nId;
  bool bTryDirect;
  ChatUser* u;
  CChatManager* m;
};

} // namespace LicqIcq

#endif
