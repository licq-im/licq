/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#ifndef LICQ_ICQFILETRANSFER_H
#define LICQ_ICQFILETRANSFER_H

/*---------------------------------------------------------------------------
 * FileTransfer Manager - CFileTransferManager
 *
 * This class is used to handle file transfers from plugins.  It is much
 * the chat manager only a lot simpler.  It is used in the following manner:
 *
 * 1.  Construct a CFileTransferManager object.  The constructor just takes
 *     a pointer to the Licq daemon and the UIN of the user being sent or
 *     received from.
 *     Listen on the CFileTransferManager::Pipe() for a signal that an event
 *     is available for processing.
 *     To receive automatic notification every x seconds (removes the need
 *     for a seperate timer in the plugin) call SetUpdatesEnabled(x) where
 *     x is the interval (2 seconds is good).  This will cause an FT_UPDATE
 *     signal to be sent every x seconds while a transfer is going on.
 * 2a. To receive files, call CFileTransferManager::ReceiveFiles().
 * 2b. To send files, call CFileTransferManager::SendFiles(<files>, <port>),
 *     where <files> is a ConstFileList of files to send, and <port> is the
 *     port of the remote user to connect to (taken from CExtendedAck::Port()
 *     from the file transfer request acceptance).
 * 3.  Process the events using PopFileTransferEvent().  Each event contains
 *     a command and possibly a string as well:
 *     FT_STARTxTRANSFER: Signals the start of the transfer.  At this point
 *       certain values can be extracted from the FileManager, including the
 *       number of files, the total size, the name of the remote host...
 *     FT_STARTxFILE: Signals the start of an actual file being sent.  At
 *       this point more values can be found, including the file name, file
 *       size.  Also, from this point until receiving FT_DONExFILE the
 *       current position and bytes remaining are always available from the
 *       CFileTransferManager.  The data will be the name of the file being
 *       sent, but this information is also available directly from the
 *       CFileTransferManager.
 *     FT_DONExFILE: Signals the successful completion of the file.  The
 *       data argument will be the full pathname of the file.
 *     FT_DONExTRANSFER: Signals that the transfer is done and the other side
 *       has disconnected.
 *     FT_ERRORx<...>: This means some kind of error has occured either
 *       reading/writing to files or talking to the network.  More details
 *       are available in the log.  The type of error is also specified
 *       as FT_ERRORxFILE (file read/write error, PathName() contains the
 *       name of the offending file), FT_ERRORxHANDSHAKE (handshaking error
 *       by the other side), FT_ERRORxCLOSED (the remote side closed
 *       the connection unexpectedly), FT_ERRORxCONNECT (error reaching
 *       the remote host), FT_ERRORxBIND (error binding a port when D_RECEIVER)
 *       or FT_ERRORxRESOURCES (error creating a new thread).
 * 4.  Call CloseFileTransfer() when done or to cancel, or simply delete the
 *       CFileTransferManager object.
 *
 * Note that if a file already exists, it will be automatically appended
 * to, unless it is the same size or greater then the incoming file, in
 * which case the file will be saved as <filename>.<timestamp>
 *-------------------------------------------------------------------------*/

#include <cstring>
#include <list>
#include <string>

#include "pipe.h"
#include "socket.h"
#include "socketmanager.h"
#include "userid.h"

namespace Licq
{
class Packet;
}

// FileTransferEvent codes
const unsigned char FT_STARTxBATCH   = 1;
const unsigned char FT_STARTxFILE    = 2;
const unsigned char FT_UPDATE        = 3;
const unsigned char FT_DONExFILE     = 4;
const unsigned char FT_DONExBATCH    = 5;
const unsigned char FT_CONFIRMxFILE  = 6;
const unsigned char FT_ERRORxFILE      = 0xFF;
const unsigned char FT_ERRORxHANDSHAKE = 0xFE;
const unsigned char FT_ERRORxCLOSED    = 0xFD;
const unsigned char FT_ERRORxCONNECT   = 0xFC;
const unsigned char FT_ERRORxBIND      = 0xFB;
const unsigned char FT_ERRORxRESOURCES = 0xFA;

class CFileTransferManager;

struct SFileReverseConnectInfo
{
  int nId;
  bool bTryDirect;
  CFileTransferManager *m;
};

//=====FileTransferManager===================================================
extern "C"
{
  void *FileTransferManager_tep(void *);
  void *FileWaitForSignal_tep(void *);
  void FileWaitForSignal_cleanup(void *);
}

class CFileTransferEvent
{
public:
  unsigned char Command() { return m_nCommand; }
  const std::string& fileName() const { return myFileName; }

  ~CFileTransferEvent();

protected:
  CFileTransferEvent(unsigned char t, const std::string& fileName = std::string());
  unsigned char m_nCommand;
  std::string myFileName;

friend class CFileTransferManager;
};

typedef std::list<CFileTransferEvent *> FileTransferEventList;
typedef std::list<class CFileTransferManager *> FileTransferManagerList;

class CFileTransferManager
{
public:
  CFileTransferManager(const Licq::UserId& userId);
  ~CFileTransferManager();

  bool receiveFiles(const std::string& directory);
  void sendFiles(const std::list<std::string>& pathNames, unsigned short nPort);

  void CloseFileTransfer();

  // Available after construction
  uint16_t LocalPort() const            { return ftServer.getLocalPort(); }
  const std::string& localName() const { return myLocalName; }
  bool isReceiver() const { return myIsReceiver; }
  const Licq::UserId& userId() const { return myUserId; }

  // Available after FT_STARTxBATCH
  const std::string& remoteName() const { return myRemoteName; }
  unsigned short BatchFiles() { return m_nBatchFiles; }
  unsigned long BatchSize() { return m_nBatchSize; }
  time_t BatchStartTime() { return m_nBatchStartTime; }

  // Available between FT_CONFIRMxFILE and FT_STATE_

  // You must use this function to start receiving the incoming file, possibly
  // giving it a different name on the local machine.
  bool startReceivingFile(const std::string& fileName);

  // Available between FT_STARTxFILE and FT_DONExFILE
  unsigned long FilePos() { return m_nFilePos; }
  unsigned long BytesTransfered() { return m_nBytesTransfered; }
  unsigned long FileSize() { return m_nFileSize; }
  time_t StartTime() { return m_nStartTime; }
  const std::string& fileName() const { return myFileName; }
  const std::string& pathName() const { return myPathName; }

  // Batch information, available after first FT_STARTxFILE
  unsigned short CurrentFile() { return m_nCurrentFile; }
  unsigned long BatchBytesTransfered() { return m_nBatchBytesTransfered; }
  unsigned long BatchPos() { return m_nBatchPos; }

  void ChangeSpeed(unsigned short);
  void SetUpdatesEnabled(unsigned short n) { m_nUpdatesEnabled = n; }
  unsigned short UpdatesEnabled(void) { return m_nUpdatesEnabled; }

  int Pipe() { return myEventsPipe.getReadFd(); }
  CFileTransferEvent *PopFileTransferEvent();

  void AcceptReverseConnection(Licq::TCPSocket*);
  static CFileTransferManager *FindByPort(unsigned short);

protected:
  static FileTransferManagerList ftmList;

  static pthread_mutex_t thread_cancel_mutex;
  bool m_bThreadRunning;
  pthread_t m_tThread;

  Licq::Pipe myEventsPipe;
  Licq::Pipe myThreadPipe;
  FileTransferEventList ftEvents;
  pthread_t thread_ft;
  std::list<std::string> myPathNames;
  bool myIsReceiver;

  struct timeval tv_lastupdate;
  unsigned short m_nUpdatesEnabled;

  unsigned char m_nResult;
  unsigned short m_nSession, m_nSpeed, m_nState;
  Licq::UserId myUserId;

  std::string myLocalName;
  std::string myRemoteName;
  unsigned short m_nPort;
  unsigned long m_nFilePos, m_nBatchPos, m_nBytesTransfered, m_nBatchBytesTransfered;
  unsigned short m_nCurrentFile, m_nBatchFiles;
  unsigned long m_nFileSize, m_nBatchSize;
  time_t m_nStartTime, m_nBatchStartTime;
  std::string myFileName;
  std::string myPathName;
  std::string myDirectory;
  int m_nFileDesc;
  std::list<std::string>::iterator myPathNameIter;
  bool m_bThreadCreated;

  Licq::TCPSocket ftSock, ftServer;

  Licq::SocketManager sockman;

  bool StartFileTransferServer();
  bool ConnectToFileServer(unsigned short nPort);
  bool SendFileHandshake();
  bool ProcessPacket();
  bool SendFilePacket();
  void PushFileTransferEvent(unsigned char);
  void PushFileTransferEvent(CFileTransferEvent *);
  void CloseConnection();

  bool SendBuffer(Licq::Buffer*);
  bool SendPacket(Licq::Packet*);

friend void *FileTransferManager_tep(void *);
friend void *FileWaitForSignal_tep(void *);
friend void FileWaitForSignal_cleanup(void *);
};



#endif
