/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#include "config.h"

#include <licq/socketmanager.h>

#include <licq/contactlist/user.h>
#include <licq/socket.h>
#include <licq/thread/mutexlocker.h>

using namespace std;
using Licq::INetSocket;
using Licq::SocketHashTable;
using Licq::SocketManager;
using Licq::SocketSet;
using Licq::UserId;

static const unsigned short SOCKET_HASH_SIZE = 128;

SocketSet::SocketSet()
{
  FD_ZERO(&sFd);
}

SocketSet::~SocketSet()
{
  // Empty
}

void SocketSet::Set(int _nSD)
{
  MutexLocker lock(myMutex);
  FD_SET(_nSD, &sFd);
  list<int>::iterator i = lFd.begin();
  while (i != lFd.end() && _nSD < *i)
    ++i;
  lFd.insert(i, _nSD);
}

void SocketSet::Clear(int _nSD)
{
  MutexLocker lock(myMutex);
  FD_CLR(_nSD, &sFd);
  list<int>::iterator i = lFd.begin();
  while (i != lFd.end() && *i != _nSD)
    ++i;
  if (i != lFd.end())
    lFd.erase(i);
}

unsigned short SocketSet::Num()
{
  MutexLocker lock(myMutex);
  return lFd.size();
}

int SocketSet::Largest()
{
  MutexLocker lock(myMutex);
  if (lFd.empty())
    return 0;
  else
    return *lFd.begin();
}

fd_set SocketSet::socketSet()
{
  MutexLocker lock(myMutex);
  return sFd;
}


SocketHashTable::SocketHashTable(unsigned short _nSize)
  : m_vlTable(_nSize)
{
  // Empty
}

SocketHashTable::~SocketHashTable()
{
  // Empty
}

INetSocket* SocketHashTable::Retrieve(int _nSd)
{
  myMutex.lockRead();

  INetSocket *s = NULL;
  list <INetSocket *> &l = m_vlTable[HashValue(_nSd)];

  int nSd;
  list<INetSocket *>::iterator iter;
  for (iter = l.begin(); iter != l.end(); ++iter)
  {
    (*iter)->Lock();
    nSd = (*iter)->Descriptor();
    (*iter)->Unlock();
    if (nSd == _nSd)
    {
      s = (*iter);
      break;
    }
  }
  if (iter == l.end()) s = NULL;

  myMutex.unlockRead();
  return s;
}

void SocketHashTable::Store(INetSocket *s, int _nSd)
{
  myMutex.lockWrite();
  list<INetSocket *> &l = m_vlTable[HashValue(_nSd)];
  l.push_front(s);
  myMutex.unlockWrite();
}

void SocketHashTable::Remove(int _nSd)
{
  myMutex.lockWrite();
  list<INetSocket *> &l = m_vlTable[HashValue(_nSd)];
  int nSd;
  list<INetSocket *>::iterator iter;
  for (iter = l.begin(); iter != l.end(); ++iter)
  {
    (*iter)->Lock();
    nSd = (*iter)->Descriptor();
    (*iter)->Unlock();
    if (nSd == _nSd)
    {
      l.erase(iter);
      break;
    }
  }
  myMutex.unlockWrite();
}

unsigned short SocketHashTable::HashValue(int _nSd)
{
  //return _nSd % m_vlTable.size();
  return _nSd & (unsigned long)(SOCKET_HASH_SIZE - 1);
}


SocketManager::SocketManager()
  : m_hSockets(SOCKET_HASH_SIZE)
{
  // Empty
}

SocketManager::~SocketManager()
{
  myMutex.lock();
  myMutex.unlock();
}

INetSocket* SocketManager::FetchSocket(int _nSd)
{
  MutexLocker lock(myMutex);
  INetSocket *s = m_hSockets.Retrieve(_nSd);
  if (s != NULL)
    s->Lock();
  return s;
}

void SocketManager::DropSocket(INetSocket *s)
{
  if (s != NULL)
    s->Unlock();
}

void SocketManager::AddSocket(INetSocket *s)
{
  s->Lock();
  m_hSockets.Store(s, s->Descriptor());
  m_sSockets.Set(s->Descriptor());
}

void SocketManager::CloseSocket(int nSd, bool bClearUser, bool bDelete)
{
  // Quick check that the socket is valid
  if (nSd == -1) return;

  MutexLocker lock(myMutex);

  // Clear from the socket list
  m_sSockets.Clear(nSd);

  // Fetch the actual socket
  INetSocket *s = m_hSockets.Retrieve(nSd);
  if (s == NULL)
    return;

  UserId userId = s->userId();
  unsigned char nChannel = s->Channel();

  // First remove the socket from the hash table so it won't be fetched anymore
  m_hSockets.Remove(nSd);


  // Now close the connection (we don't have to lock it first, because the
  // Remove function above guarantees that no one has a lock on the socket
  // before removing it from the hash table, and once removed from the has
  // table, no one can get a lock again.
  s->CloseConnection();
  if (bDelete) delete s;

  if (bClearUser)
  {
    Licq::UserWriteGuard u(userId);
    if (u.isLocked())
    {
      u->ClearSocketDesc(nChannel);
      if (u->OfflineOnDisconnect())
        u->statusChanged(Licq::User::OfflineStatus);
    }
  }
}
