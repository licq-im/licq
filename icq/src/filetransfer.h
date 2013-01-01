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

#ifndef LICQICQ_FILETRANSFER_H
#define LICQICQ_FILETRANSFER_H

#include <licq/icq/filetransfer.h>

#include <licq/pipe.h>
#include <licq/socketmanager.h>

#include "socket.h"

namespace Licq
{
class Packet;
}

namespace LicqIcq
{
class FileTransferManager;

void *FileTransferManager_tep(void *);
void *FileWaitForSignal_tep(void *);
void FileWaitForSignal_cleanup(void *);

typedef std::list<FileTransferManager*> FileTransferManagerList;
typedef std::list<Licq::IcqFileTransferEvent*> FileTransferEventList;

struct SFileReverseConnectInfo
{
  int nId;
  bool bTryDirect;
  FileTransferManager* m;
};

class FileTransferManager : public Licq::IcqFileTransferManager
{
public:
  FileTransferManager(const Licq::UserId& userId);

  ~FileTransferManager();

  void AcceptReverseConnection(Licq::TCPSocket*);

  static FileTransferManager* FindByPort(unsigned short);

  // From Licq::IcqFileTransferManager
  bool receiveFiles(const std::string& directory);
  void sendFiles(const std::list<std::string>& pathNames, unsigned short nPort);
  void CloseFileTransfer();
  unsigned short LocalPort() const;
  bool startReceivingFile(const std::string& fileName);
  void ChangeSpeed(unsigned short);
  int Pipe();
  Licq::IcqFileTransferEvent* PopFileTransferEvent();

private:
  static FileTransferManagerList ftmList;
  static pthread_mutex_t thread_cancel_mutex;
  bool m_bThreadRunning;
  pthread_t m_tThread;
  Licq::Pipe myEventsPipe;
  Licq::Pipe myThreadPipe;
  pthread_t thread_ft;
  bool m_bThreadCreated;

  std::list<std::string> myPathNames;

  struct timeval tv_lastupdate;
  unsigned char m_nResult;
  unsigned short m_nSession, m_nSpeed, m_nState;

  unsigned short m_nPort;
  int m_nFileDesc;
  std::list<std::string>::iterator myPathNameIter;

  FileTransferEventList ftEvents;

  Licq::TCPSocket ftServer;
  DcSocket mySock;
  Licq::SocketManager sockman;

  bool StartFileTransferServer();
  bool ConnectToFileServer(unsigned short nPort);
  bool SendFileHandshake();
  bool ProcessPacket();
  bool SendFilePacket();
  void PushFileTransferEvent(unsigned char t);
  void PushFileTransferEvent(Licq::IcqFileTransferEvent* e);
  void CloseConnection();

  bool SendBuffer(Licq::Buffer*);
  bool SendPacket(Licq::Packet*);

friend void *FileTransferManager_tep(void *);
friend void *FileWaitForSignal_tep(void *);
friend void FileWaitForSignal_cleanup(void *);

};

typedef LicqIcq::FileTransferManager CFileTransferManager;

} // namespace LicqIcq

#endif
