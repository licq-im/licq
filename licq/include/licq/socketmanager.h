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

#ifndef LICQ_SOCKETMANAGER_H
#define LICQ_SOCKETMANAGER_H

#include <vector>
#include <list>
#include <sys/select.h> // fd_set

#include "thread/mutex.h"
#include "thread/readwritemutex.h"

namespace Licq
{
class INetSocket;

class SocketHashTable
{
public:
  SocketHashTable(unsigned short _nSize);
  ~SocketHashTable();

  INetSocket *Retrieve(int _nSd);
  void Store(INetSocket *s, int _nSd);
  void Remove(int _nSd);

protected:
  unsigned short HashValue(int _nSd);
  std::vector < std::list<INetSocket *> > m_vlTable;

  ReadWriteMutex myMutex;
};


class SocketSet
{
friend class SocketManager;

public:
  SocketSet();
  ~SocketSet();

  unsigned short Num();
  int Largest();
  fd_set socketSet();

protected:
  fd_set sFd;
  std::list<int> lFd;
  void Set(int _nSD);
  void Clear(int _nSD);

  Mutex myMutex;
};


class SocketManager
{
public:
  SocketManager();
  virtual ~SocketManager();

  INetSocket* FetchSocket(int _nSd);
  void DropSocket(INetSocket *s);
  void AddSocket(INetSocket *s);
  void CloseSocket (int nSd, bool bClearUser = true, bool bDelete = true);

  fd_set socketSet()   { return m_sSockets.socketSet(); }
  int LargestSocket()  { return m_sSockets.Largest(); }
  unsigned short Num() { return m_sSockets.Num(); }

protected:
  SocketSet m_sSockets;
  SocketHashTable m_hSockets;
  Mutex myMutex;
};

} // namespace Licq

#endif
