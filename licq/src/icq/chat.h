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
#include <pthread.h>
#include <string>

#include "socket.h"

namespace LicqIcq
{

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
