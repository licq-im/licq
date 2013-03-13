/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1998-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "filetransfer.h"

#include <cstdio>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <sys/types.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <sys/time.h>

#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/daemon.h>

#include "gettext.h"
#include "icq.h"
#include "packet-tcp.h"
#include "user.h"

#define DEBUG_THREADS(x)

const unsigned short FT_STATE_DISCONNECTED = 0;
const unsigned short FT_STATE_HANDSHAKE = 1;
const unsigned short FT_STATE_WAITxFORxCLIENTxINIT = 2;
const unsigned short FT_STATE_WAITxFORxSERVERxINIT = 3;
const unsigned short FT_STATE_WAITxFORxSTART = 4;
const unsigned short FT_STATE_WAITxFORxFILExINFO = 5;
const unsigned short FT_STATE_RECEIVINGxFILE = 6;
const unsigned short FT_STATE_SENDINGxFILE = 7;
const unsigned short FT_STATE_CONFIRMINGxFILE = 8;

using namespace LicqIcq;
using Licq::IcqFileTransferEvent;
using Licq::Log;
using Licq::gDaemon;
using Licq::gLog;
using std::list;
using std::string;

//=====FILE==================================================================

//-----PacketFile---------------------------------------------------------------
CPacketFile::CPacketFile()
  : buffer(NULL)
{
  // Empty
}

CPacketFile::~CPacketFile()
{
  delete buffer;
}

//-----FileInitClient-----------------------------------------------------------
CPFile_InitClient::CPFile_InitClient(const string& localName,
                                    unsigned long _nNumFiles,
                                    unsigned long _nTotalSize)
{
  m_nSize = 20 + localName.size();
  InitBuffer();

  buffer->PackChar(0x00);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(_nNumFiles);
  buffer->PackUnsignedLong(_nTotalSize);
  buffer->PackUnsignedLong(0x64);
  buffer->PackString(localName.c_str());
}

CPFile_InitClient::~CPFile_InitClient()
{
  // Empty
}

//-----FileInitServer-----------------------------------------------------------
CPFile_InitServer::CPFile_InitServer(const string& localName)
{
  m_nSize = 8 + localName.size();
  InitBuffer();

  buffer->PackChar(0x01);
  buffer->PackUnsignedLong(0x64);
  buffer->PackString(localName.c_str());
}

CPFile_InitServer::~CPFile_InitServer()
{
  // Empty
}

//-----FileBatch----------------------------------------------------------------
CPFile_Info::CPFile_Info(const string& fileName)
{
  m_bValid = true;
  m_nError = 0;

  struct stat buf;

  // Remove any path from the filename
  size_t lastSlash = fileName.rfind('/');
  myFileName = (lastSlash == string::npos ? fileName : fileName.substr(lastSlash+1));

  if (stat(myFileName.c_str(), &buf) < 0)
  {
    m_bValid = false;
    m_nError = errno;
    return;
  }
  m_nFileSize = buf.st_size;

  m_nSize = myFileName.size() + 21;
  InitBuffer();

  buffer->PackUnsignedShort(0x02);
  // Add all the file names
  buffer->PackString(myFileName.c_str());
  // Add the empty file name
  buffer->PackString("");
  //Add the file length
  buffer->PackUnsignedLong(m_nFileSize);
  buffer->PackUnsignedLong(0x00);
  buffer->PackUnsignedLong(0x64);
}


CPFile_Info::~CPFile_Info()
{
  // Empty
}


//-----FileStart----------------------------------------------------------------
CPFile_Start::CPFile_Start(unsigned long nFilePos, unsigned long nFile)
{
  m_nSize = 17;
  InitBuffer();

  buffer->PackChar(0x03);
  buffer->PackUnsignedLong(nFilePos);
  buffer->PackUnsignedLong(0x00);
  buffer->PackUnsignedLong(0x64);
  buffer->PackUnsignedLong(nFile);
}

CPFile_Start::~CPFile_Start()
{
  // Empty
}

//-----FileSpeed----------------------------------------------------------------
CPFile_SetSpeed::CPFile_SetSpeed(unsigned long nSpeed)
{
  m_nSize = 5;
  InitBuffer();

  buffer->PackChar(0x05);
  buffer->PackUnsignedLong(nSpeed);
}

CPFile_SetSpeed::~CPFile_SetSpeed()
{
  // Empty
}

//=====FileTransferManager===========================================================
IcqFileTransferEvent::IcqFileTransferEvent(unsigned char t, const string& fileName)
{
  m_nCommand = t;
  myFileName = fileName;
}

IcqFileTransferEvent::~IcqFileTransferEvent()
{
  // Empty
}


FileTransferManagerList CFileTransferManager::ftmList;

pthread_mutex_t CFileTransferManager::thread_cancel_mutex
                                                   = PTHREAD_MUTEX_INITIALIZER;

Licq::IcqFileTransferManager::~IcqFileTransferManager()
{
  // Empty
}

FileTransferManager::FileTransferManager(const Licq::UserId& userId)
  : m_bThreadRunning(false)
{
  myUserId = userId;
  m_nSession = rand();

  {
    Licq::OwnerReadGuard o(gIcqProtocol.ownerId());
    myLocalName = o->getAlias();
  }

  m_nCurrentFile = m_nBatchFiles = 0;
  m_nFileSize = m_nBatchSize = m_nFilePos = m_nBatchPos = 0;
  m_nBytesTransfered = m_nBatchBytesTransfered = 0;
  m_nStartTime = m_nBatchStartTime = 0;
  m_nFileDesc = -1;
  m_nState = FT_STATE_DISCONNECTED;
  m_bThreadCreated = false;

  myRemoteName = myUserId.accountId();

  ftmList.push_back(this);
}


//-----CFileTransferManager::StartFileTransferServer-----------------------------------------
bool CFileTransferManager::StartFileTransferServer()
{
  if (gDaemon.StartTCPServer(&ftServer) == -1)
  {
    gLog.warning(tr("No more ports available, add more or close open chat/file sessions."));
    return false;
  }

  // Add the server to the sock manager
  sockman.AddSocket(&ftServer);
  sockman.DropSocket(&ftServer);

  return true;
}



bool CFileTransferManager::receiveFiles(const string& directory)
{
  myIsReceiver = true;

  if (directory.empty())
  {
    myDirectory = Licq::gDaemon.baseDir() + myUserId.accountId();
    if (access(Licq::gDaemon.baseDir().c_str(), F_OK) < 0 && mkdir(myDirectory.c_str(), 0700) == -1 &&
        errno != EEXIST)
    {
      gLog.warning(tr("Unable to create directory %s for file transfer."), myDirectory.c_str());
      myDirectory = Licq::gDaemon.baseDir();
    }
  }
  else
  {
    myDirectory = directory;
  }

  struct stat buf;
  stat(myDirectory.c_str(), &buf);
  if (!S_ISDIR(buf.st_mode))
  {
    gLog.warning(tr("Path %s is not a directory."), myDirectory.c_str());
    return false;
  }

  if (!StartFileTransferServer())
  {
    PushFileTransferEvent(Licq::FT_ERRORxBIND);
    return false;
  }

  // Create the socket manager thread
  if (pthread_create(&thread_ft, NULL, &FileTransferManager_tep, this) == -1)
  {
    PushFileTransferEvent(Licq::FT_ERRORxRESOURCES);
    return false;
  }

  m_bThreadCreated = true;

  return true;
}


//-----CFileTransferManager::StartAsClient-------------------------------------------
void CFileTransferManager::sendFiles(const list<string>& pathNames, unsigned short nPort)
{
  myIsReceiver = false;

  // Validate the pathnames
  if (pathNames.empty())
    return;

  struct stat buf;
  list<string>::const_iterator iter;
  for (iter = pathNames.begin(); iter != pathNames.end(); ++iter)
  {
    if (stat(iter->c_str(), &buf) == -1)
    {
      gLog.warning(tr("File Transfer: File access error %s: %s."),
         iter->c_str(), strerror(errno));
      continue;
    }
    myPathNames.push_back(*iter);
    m_nBatchFiles++;
    m_nBatchSize += buf.st_size;
  }
  myPathNameIter = myPathNames.begin();
  myPathName = *myPathNameIter;
  m_nPort = nPort;

  // start the server anyway, may need to do reverse connect
  if (!StartFileTransferServer())
  {
    PushFileTransferEvent(Licq::FT_ERRORxBIND);
    return;
  }

  // Create the socket manager thread
  if (pthread_create(&thread_ft, NULL, &FileTransferManager_tep, this) == -1)
  {
    PushFileTransferEvent(Licq::FT_ERRORxRESOURCES);
    return;
  }

  m_bThreadCreated = true;
}


//-----CFileTransferManager::ConnectToFileServer-----------------------------
bool CFileTransferManager::ConnectToFileServer(unsigned short nPort)
{
  bool bTryDirect;
  bool bSendIntIp;
  {
    UserReadGuard u(myUserId);
    if (!u.isLocked())
      return false;

    bTryDirect = u->Version() <= 6 || u->directMode();
    bSendIntIp = u->SendIntIp();
  }

  bool bSuccess = false;
  if (bTryDirect)
  {
    gLog.info(tr("File Transfer: Connecting to server."));
    bSuccess = gIcqProtocol.openConnectionToUser(myUserId, &mySock, nPort);
   }

  bool bResult = false;
  if (!bSuccess)
  {
    unsigned long nIp;
    {
      Licq::OwnerReadGuard o(gIcqProtocol.ownerId());
      nIp = bSendIntIp ? o->IntIp() : o->Ip();
    }

    // try reverse connect
    int nId = gIcqProtocol.requestReverseConnection(myUserId, 0, nIp, LocalPort(), nPort);

    if (nId != -1)
    {
      struct SFileReverseConnectInfo *r = new struct SFileReverseConnectInfo;
      r->nId = nId;
      r->m = this;
      r->bTryDirect = !bTryDirect;
      pthread_mutex_lock(&thread_cancel_mutex);
      pthread_create(&m_tThread, NULL, FileWaitForSignal_tep, r);
      m_bThreadRunning = true;
      pthread_mutex_unlock(&thread_cancel_mutex);
      bResult = true;
    }
  }
  else
    bResult = SendFileHandshake();

  return bResult;
}

bool CFileTransferManager::SendFileHandshake()
{
  gLog.info(tr("File Transfer: Shaking hands."));

  // Send handshake packet:
  unsigned short nVersion;
  {
    UserReadGuard u(myUserId);
    nVersion = u->ConnectionVersion();
  }
  if (!IcqProtocol::handshake_Send(&mySock, myUserId, LocalPort(), nVersion, false))
    return false;

  // Send init packet:
  CPFile_InitClient p(myLocalName, m_nBatchFiles, m_nBatchSize);
  if (!SendPacket(&p)) return false;

  gLog.info(tr("File Transfer: Waiting for server to respond."));

  m_nState = FT_STATE_WAITxFORxSERVERxINIT;

  sockman.AddSocket(&mySock);
  sockman.DropSocket(&mySock);

  return true;
}


//-----CFileTransferManager::AcceptReverseConnection-------------------------
void CFileTransferManager::AcceptReverseConnection(Licq::TCPSocket* s)
{
  if (mySock.Descriptor() != -1)
  {
    gLog.warning(tr("File Transfer: Attempted reverse connection when already connected."));
    return;
  }

  mySock.TransferConnectionFrom(*s);
  sockman.AddSocket(&mySock);
  sockman.DropSocket(&mySock);

  m_nState = FT_STATE_WAITxFORxCLIENTxINIT;

  // Reload socket info
  myThreadPipe.putChar('R');

  gLog.info(tr("File Transfer: Received reverse connection."));
}


//-----CFileTransferManager::ProcessPacket-------------------------------------------
bool CFileTransferManager::ProcessPacket()
{
  if (!mySock.RecvPacket())
  {
    if (mySock.Error() == 0)
      gLog.info(tr("File Transfer: Remote end disconnected."));
    else
      gLog.warning(tr("File Transfer: Lost remote end: %s"), mySock.errorStr().c_str());
    if (m_nState == FT_STATE_WAITxFORxFILExINFO)
      m_nResult = Licq::FT_DONExBATCH;
    else
      m_nResult = Licq::FT_ERRORxCLOSED;
    return false;
  }

  if (!mySock.RecvBufferFull())
    return true;
  CBuffer& b = mySock.RecvBuffer();

  switch(m_nState)
  {
    // Server States

    case FT_STATE_HANDSHAKE:
    {
      CBuffer tmp(b); // we need to save a copy for later

      if (!gIcqProtocol.Handshake_Recv(&mySock, LocalPort(), false))
        break;
      gLog.info(tr("File Transfer: Received handshake."));

      unsigned long nId = 0;
      if (mySock.Version() == 7 || mySock.Version() == 8)
      {
        CPacketTcp_Handshake_v7 hand(&tmp);
        nId = hand.Id();
      }

      if (nId != 0)
      {
        pthread_mutex_lock(&gIcqProtocol.mutex_reverseconnect);
        std::list<CReverseConnectToUserData *>::iterator iter;
        bool bFound = false;
        for (iter = gIcqProtocol.m_lReverseConnect.begin();
            iter != gIcqProtocol.m_lReverseConnect.end();  ++iter)
        {
          if ((*iter)->nId == nId && (*iter)->myIdString == myUserId.accountId())
          {
            bFound = true;
            (*iter)->bSuccess = true;
            (*iter)->bFinished = true;
            pthread_cond_broadcast(&gIcqProtocol.cond_reverseconnect_done);
            break;
          }
        }
        pthread_mutex_unlock(&gIcqProtocol.mutex_reverseconnect);

        if (bFound)
        {
          // Send init packet:
          CPFile_InitClient p(myLocalName, m_nBatchFiles, m_nBatchSize);
          if (!SendPacket(&p))
          {
            m_nResult = Licq::FT_ERRORxCLOSED;
            return false;
          }

          gLog.info(tr("File Transfer: Waiting for server to respond."));

          m_nState = FT_STATE_WAITxFORxSERVERxINIT;
          break;
        }
      }

      m_nState = FT_STATE_WAITxFORxCLIENTxINIT;
      break;
    }

    case FT_STATE_WAITxFORxCLIENTxINIT:
    {
      b.unpackUInt16LE(); // Packet length
      unsigned char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.info(tr("File Transfer: Speed set to %ld%%."), nSpeed);
        break;
      }
      if (nCmd != 0x00)
      {
        b.log(Log::Error, "File Transfer: Invalid client init packet");
        m_nResult = Licq::FT_ERRORxHANDSHAKE;
        return false;
      }
      b.UnpackUnsignedLong();
      m_nBatchFiles = b.UnpackUnsignedLong();
      m_nBatchSize = b.UnpackUnsignedLong();
      m_nSpeed = b.UnpackUnsignedLong();
      myRemoteName = b.unpackShortStringLE();

      m_nBatchStartTime = time(NULL);
      m_nBatchBytesTransfered = m_nBatchPos = 0;

      PushFileTransferEvent(Licq::FT_STARTxBATCH);

      // Send speed response
      CPFile_SetSpeed p1(100);
      if (!SendPacket(&p1))
      {
        m_nResult = Licq::FT_ERRORxCLOSED;
        return false;
      }

      // Send response
      CPFile_InitServer p(myLocalName);
      if (!SendPacket(&p))
      {
        m_nResult = Licq::FT_ERRORxCLOSED;
        return false;
      }

      gLog.info(tr("File Transfer: Waiting for file info."));
      m_nState = FT_STATE_WAITxFORxFILExINFO;
      break;
    }

    case FT_STATE_WAITxFORxFILExINFO:
    {
      b.unpackUInt16LE(); // Packet length
      unsigned char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.info(tr("File Transfer: Speed set to %ld%%."), nSpeed);
        break;
      }
      if (nCmd == 0x06 && b.getDataSize() == 3)
      {
        gLog.info(tr("File Transfer: Ignoring a possible erroneous packet."));
        break;
      }
      if (nCmd != 0x02)
      {
        b.log(Log::Error, "File Transfer: Invalid file info packet");
        m_nResult = Licq::FT_ERRORxHANDSHAKE;
        return false;
      }
      b.UnpackChar();
      myFileName = b.unpackShortStringLE();

      // Remove any preceeding path info from the filename for security
      // reasons
      size_t lastSlash = myFileName.rfind('/');
      if (lastSlash != string::npos)
        myFileName.erase(0, lastSlash+1);

      b.UnpackUnsignedShort(); // 0 length string...?
      b.UnpackChar();
      m_nFileSize = b.UnpackUnsignedLong();
      b.UnpackUnsignedLong();
      m_nSpeed = b.UnpackUnsignedLong();

      m_nBytesTransfered = 0;
      m_nCurrentFile++;
      
      gLog.info(tr("File Transfer: Waiting for plugin to confirm file receive.\n"));
      
      m_nState = FT_STATE_CONFIRMINGxFILE;
      PushFileTransferEvent(new IcqFileTransferEvent(Licq::FT_CONFIRMxFILE, myPathName));
      break;
    }
    
    case FT_STATE_CONFIRMINGxFILE:
    {
      // Still waiting for the plugin to confirm
      gLog.warning(tr("File Transfer: Still waiting for the plugin to confirm file receive..."));
      break;
    }

    case FT_STATE_RECEIVINGxFILE:
    {
      // if this is the first call to this function...
      if (m_nBytesTransfered == 0)
      {
        m_nStartTime = time(NULL);
        m_nBatchPos += m_nFilePos;
        gLog.info(tr("File Transfer: Receiving %s (%ld bytes)."),
            myFileName.c_str(), m_nFileSize);
        PushFileTransferEvent(new IcqFileTransferEvent(Licq::FT_STARTxFILE, myPathName));
        gettimeofday(&tv_lastupdate, NULL);
      }

      // Write the new data to the file and empty the buffer
      b.unpackUInt16LE(); // Packet length
      char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.info(tr("File Transfer: Speed set to %ld%%."), nSpeed);
        break;
      }

      if (nCmd != 0x06)
      {
        gLog.unknown(tr("File Transfer: Invalid data (%c) ignoring packet"),
           nCmd);
        break;
      }

      errno = 0;
      size_t nBytesWritten = write(m_nFileDesc, b.getDataPosRead(), b.getDataSize() - 3);
      if (nBytesWritten != b.getDataSize() - 3)
      {
        gLog.error(tr("File Transfer: Write error: %s."),
            errno == 0 ? "Disk full (?)" : strerror(errno));
        m_nResult = Licq::FT_ERRORxFILE;
        return false;
      }

      m_nFilePos += nBytesWritten;
      m_nBytesTransfered += nBytesWritten;
      m_nBatchPos += nBytesWritten;
      m_nBatchBytesTransfered += nBytesWritten;

      // Check if we need to send an update notification
      if (m_nUpdatesEnabled)
      {
        struct timeval tv_now;
        gettimeofday(&tv_now, NULL);
        if (tv_now.tv_sec >= tv_lastupdate.tv_sec + m_nUpdatesEnabled)
        {
          PushFileTransferEvent(Licq::FT_UPDATE);
          tv_lastupdate = tv_now;
        }
      }

      int nBytesLeft = m_nFileSize - m_nFilePos;
      if (nBytesLeft > 0)
        break;

      close(m_nFileDesc);
      m_nFileDesc = -1;
      if (nBytesLeft == 0) // File transfer done perfectly
      {
        gLog.info(tr("File Transfer: %s received."), myFileName.c_str());
      }
      else // nBytesLeft < 0
      {
        // Received too many bytes for the given size of the current file
        gLog.warning(tr("File Transfer: %s received %d too many bytes."),
            myFileName.c_str(), -nBytesLeft);
      }
      // Notify Plugin
      PushFileTransferEvent(new IcqFileTransferEvent(Licq::FT_DONExFILE, myPathName));

      // Now wait for a disconnect or another file
      m_nState = FT_STATE_WAITxFORxFILExINFO;
      break;
    }


    // Client States

    case FT_STATE_WAITxFORxSERVERxINIT:
    {
      b.unpackUInt16LE(); // Packet length
      char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.info(tr("File Transfer: Speed set to %ld%%."), nSpeed);
        break;
      }
      if (nCmd != 0x01)
      {
        b.log(Log::Error, "File Transfer: Invalid server init packet");
        m_nResult = Licq::FT_ERRORxHANDSHAKE;
        return false;
      }
      m_nSpeed = b.UnpackUnsignedLong();
      myRemoteName = b.unpackShortStringLE();

      // Send file info packet
      CPFile_Info p(*myPathNameIter);
      if (!p.IsValid())
      {
        gLog.warning(tr("File Transfer: Read error for %s:\n%s"),
            myPathNameIter->c_str(), p.ErrorStr());
        m_nResult = Licq::FT_ERRORxFILE;
        return false;
      }
      if (!SendPacket(&p))
      {
        m_nResult = Licq::FT_ERRORxCLOSED;
        return false;
      }

      m_nFileSize = p.GetFileSize();
      myFileName = p.fileName();

      m_nBatchStartTime = time(NULL);
      m_nBatchBytesTransfered = m_nBatchPos = 0;

      PushFileTransferEvent(Licq::FT_STARTxBATCH);

      m_nState = FT_STATE_WAITxFORxSTART;
      break;
    }

    case FT_STATE_WAITxFORxSTART:
    {
      // contains the seek value
      b.unpackUInt16LE(); // Packet length
      char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.info(tr("File Transfer: Speed set to %ld%%."), nSpeed);
        break;
      }
      if (nCmd != 0x03)
      {
        b.log(Log::Error, "File Transfer: Invalid start packet");
        m_nResult = Licq::FT_ERRORxCLOSED;
        return false;
      }

      m_nBytesTransfered = 0;
      m_nCurrentFile++;

      m_nFilePos = b.UnpackUnsignedLong();

      m_nFileDesc = open(myPathNameIter->c_str(), O_RDONLY);
      if (m_nFileDesc == -1)
      {
        gLog.error(tr("File Transfer: Read error '%s': %s."),
            myPathNameIter->c_str(), strerror(errno));
        m_nResult = Licq::FT_ERRORxFILE;
        return false;
      }

      if (lseek(m_nFileDesc, m_nFilePos, SEEK_SET) == -1)
      {
        gLog.error(tr("File Transfer: Seek error '%s': %s."),
            myFileName.c_str(), strerror(errno));
        m_nResult = Licq::FT_ERRORxFILE;
        return false;
      }

      m_nState = FT_STATE_SENDINGxFILE;
      break;
    }

    case FT_STATE_SENDINGxFILE:
    {
      b.unpackUInt16LE(); // Packet length
      char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.info(tr("File Transfer: Speed set to %ld%%."), nSpeed);
        break;
      }
      b.log(Log::Unknown, tr("File Transfer: Unknown packet received during file send"));
      break;
    }


    default:
    {
      gLog.error(tr("Internal error: FileTransferManager::ProcessPacket(), invalid state (%d)."),
          m_nState);
      break;
    }

  } // switch

  mySock.ClearRecvBuffer();

  return true;
}

// This function gives a callback opportunity for the plugin, just before
// the actual transfer begins
bool CFileTransferManager::startReceivingFile(const string& fileName)
{
  gLog.info(tr("File Transfer: Received plugin confirmation."));

  if (m_nState != FT_STATE_CONFIRMINGxFILE)
  {
     gLog.warning(tr("File Transfer: StartReceivingFile called without a pending confirmation."));
     return false;
  }

  // If a different filename was specified, use it
  if (fileName.empty())
    myFileName = fileName;

  // Get the local filename and set the file offset (for resume)
  struct stat buf;
  m_nFileDesc = -1;
  myPathName = myDirectory + '/' + myFileName;
  while (m_nFileDesc == -1)
  {
    if (stat(myPathName.c_str(), &buf) != -1)
    {
      if ((unsigned long)buf.st_size >= m_nFileSize)
      {
        char buf[20];
        snprintf(buf, sizeof(buf), ".%lu", (unsigned long)time(NULL));
        myPathName += buf;
      }
      m_nFileDesc = open(myPathName.c_str(), O_WRONLY | O_CREAT | O_APPEND, 00600);
      m_nFilePos = buf.st_size;
    }
    else
    {
      m_nFileDesc = open(myPathName.c_str(), O_WRONLY | O_CREAT, 00600);
      m_nFilePos = 0;
    }
    if (m_nFileDesc == -1)
    {
      gLog.error(tr("File Transfer: Unable to open %s for writing: %s."),
          myPathName.c_str(), strerror(errno));
      m_nResult = Licq::FT_ERRORxFILE;
      return false;
    }
  }

  // Send response
  CPFile_Start p(m_nFilePos, m_nCurrentFile);
  if (!SendPacket(&p))
  {
    gLog.error(tr("File Transfer: Unable to send file receive start packet."));
    m_nResult = Licq::FT_ERRORxCLOSED;
    return false;
  }

  m_nState = FT_STATE_RECEIVINGxFILE;
  return true;
}

//-----CFileTransferManager::SendFilePacket----------------------------------
bool CFileTransferManager::SendFilePacket()
{
  static char pSendBuf[2048];

  if (m_nBytesTransfered == 0)
  {
    m_nStartTime = time(NULL);
    m_nBatchPos += m_nFilePos;
    gLog.info(tr("File Transfer: Sending %s (%ld bytes)."),
        myPathName.c_str(), m_nFileSize);
    PushFileTransferEvent(new IcqFileTransferEvent(Licq::FT_STARTxFILE, myPathName));
    gettimeofday(&tv_lastupdate, NULL);
  }

  int nBytesToSend = m_nFileSize - m_nFilePos;
  if (nBytesToSend > 2048) nBytesToSend = 2048;
  if (read(m_nFileDesc, pSendBuf, nBytesToSend) != nBytesToSend)
  {
    gLog.error(tr("File Transfer: Error reading from %s: %s."),
        myPathName.c_str(), strerror(errno));
    m_nResult = Licq::FT_ERRORxFILE;
    return false;
  }
  CBuffer xSendBuf(nBytesToSend + 1);
  xSendBuf.PackChar(0x06);
  xSendBuf.Pack(pSendBuf, nBytesToSend);
  if (!SendBuffer(&xSendBuf))
  {
    m_nResult = Licq::FT_ERRORxCLOSED;
    return false;
  }

  m_nFilePos += nBytesToSend;
  m_nBytesTransfered += nBytesToSend;

  m_nBatchPos += nBytesToSend;
  m_nBatchBytesTransfered += nBytesToSend;

  // Check if we need to send an update notification
  if (m_nUpdatesEnabled)
  {
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    if (tv_now.tv_sec >= tv_lastupdate.tv_sec + m_nUpdatesEnabled)
    {
      PushFileTransferEvent(Licq::FT_UPDATE);
      tv_lastupdate = tv_now;
    }
  }

  int nBytesLeft = m_nFileSize - m_nFilePos;
  if (nBytesLeft > 0)
  {
    // More bytes to send so go away until the socket is free again
    return true;
  }

  // Only get here if we are done
  close(m_nFileDesc);
  m_nFileDesc = -1;

  if (nBytesLeft == 0)
  {
    gLog.info(tr("File Transfer: Sent %s."), myFileName.c_str());
  }
  else // nBytesLeft < 0
  {
    gLog.info(tr("File Transfer: Sent %s, %d too many bytes."),
        myFileName.c_str(), -nBytesLeft);
  }
  PushFileTransferEvent(new IcqFileTransferEvent(Licq::FT_DONExFILE, myPathName));

  // Go to the next file, if no more then close connections
  myPathNameIter++;
  if (myPathNameIter == myPathNames.end())
  {
    m_nResult = Licq::FT_DONExBATCH;
    return false;
  }
  else
  {
    // Send file info packet
    CPFile_Info p(*myPathNameIter);
    if (!p.IsValid())
    {
      gLog.warning(tr("File Transfer: Read error for %s: %s"),
          myPathNameIter->c_str(), p.ErrorStr());
      m_nResult = Licq::FT_ERRORxFILE;
      return false;
    }
    if (!SendPacket(&p))
    {
      m_nResult = Licq::FT_ERRORxCLOSED;
      return false;
    }

    m_nFileSize = p.GetFileSize();
    myFileName = p.fileName();
    myPathName = *myPathNameIter;

    m_nState = FT_STATE_WAITxFORxSTART;
  }

  return true;
}



//-----CFileTransferManager::PopFileTransferEvent------------------------------
IcqFileTransferEvent *CFileTransferManager::PopFileTransferEvent()
{
  if (ftEvents.empty()) return NULL;

  IcqFileTransferEvent* e = ftEvents.front();
  ftEvents.pop_front();

  return e;
}


//-----CFileTransferManager::PushFileTransferEvent-----------------------------
void CFileTransferManager::PushFileTransferEvent(unsigned char t)
{
  PushFileTransferEvent(new IcqFileTransferEvent(t));
}

void CFileTransferManager::PushFileTransferEvent(IcqFileTransferEvent *e)
{
  ftEvents.push_back(e);
  myEventsPipe.putChar('*');
}


unsigned short FileTransferManager::LocalPort() const
{
  return ftServer.getLocalPort();
}

int FileTransferManager::Pipe()
{
  return myEventsPipe.getReadFd();
}

//-----CFileTransferManager::SendPacket----------------------------------------------
bool CFileTransferManager::SendPacket(CPacket *p)
{
  return SendBuffer(p->getBuffer());
}


//-----CFileTransferManager::SendBuffer----------------------------------------------
bool CFileTransferManager::SendBuffer(CBuffer *b)
{
  if (!mySock.send(*b))
  {
    gLog.warning(tr("File Transfer: Send error: %s"), mySock.errorStr().c_str());
    return false;
  }
  return true;
}


void CFileTransferManager::ChangeSpeed(unsigned short nSpeed)
{
  if (nSpeed > 100)
  {
    gLog.warning(tr("Invalid file transfer speed: %d%%."), nSpeed);
    return;
  }

  //CPFile_ChangeSpeed p(nSpeed);
  //SendPacket(&p);
  m_nSpeed = nSpeed;
}



//----CFileTransferManager::CloseFileTransfer--------------------------------
void CFileTransferManager::CloseFileTransfer()
{
  // Close the thread
  myThreadPipe.putChar('X');
  if (m_bThreadCreated)
    pthread_join(thread_ft, NULL);
  m_bThreadCreated = false;

  CloseConnection();
}


//----CFileTransferManager::CloseConnection----------------------------------
void CFileTransferManager::CloseConnection()
{
  sockman.CloseSocket(ftServer.Descriptor(), false, false);
  sockman.CloseSocket(mySock.Descriptor(), false, false);
  m_nState = FT_STATE_DISCONNECTED;

  if (m_nFileDesc != -1)
  {
    close(m_nFileDesc);
    m_nFileDesc = -1;
  }
}



void* LicqIcq::FileTransferManager_tep(void* arg)
{
  CFileTransferManager *ftman = (CFileTransferManager *)arg;

  fd_set f_recv, f_send;
  struct timeval *tv;
  struct timeval tv_updates = { 2, 0 };
  int l, nSocketsAvailable, nCurrentSocket;

  if (!ftman->isReceiver())
  {
    if (!ftman->ConnectToFileServer(ftman->m_nPort))
    {
      ftman->PushFileTransferEvent(Licq::FT_ERRORxCONNECT);
      return NULL;
    }
  }
  else if (!ftman->isReceiver())
    return NULL;

  while (true)
  {
    f_recv = ftman->sockman.socketSet();
    l = ftman->sockman.LargestSocket() + 1;

    // Add the new socket pipe descriptor
    FD_SET(ftman->myThreadPipe.getReadFd(), &f_recv);
    if (ftman->myThreadPipe.getReadFd() >= l)
      l = ftman->myThreadPipe.getReadFd() + 1;

    // Set up the send descriptor
    FD_ZERO(&f_send);
    if (ftman->m_nState == FT_STATE_SENDINGxFILE)
    {
      FD_SET(ftman->mySock.Descriptor(), &f_send);
      // No need to check "l" as mySock is already in the read list
    }

    // Prepare max timeout if necessary
    if (ftman->m_nUpdatesEnabled &&
        (ftman->m_nState == FT_STATE_SENDINGxFILE ||
         ftman->m_nState == FT_STATE_RECEIVINGxFILE) )
    {
      tv_updates.tv_sec = ftman->m_nUpdatesEnabled;
      tv_updates.tv_usec = 0;
      tv = &tv_updates;
    }
    else
    {
      tv = NULL;
    }

    nSocketsAvailable = select(l, &f_recv, &f_send, NULL, tv);

    if (nSocketsAvailable == -1)
    {
      // Something is very wrong, most likely we've lost control of a file
      //   descriptor and select will continue to fail causing this thread to
      //   spin so better to just give up and exit.

      gLog.warning(tr("File Transfer: select failed, aborting thread: %s"), strerror(errno));
      pthread_exit(NULL);
    }

    // Check if we timed out
    if (tv != NULL && nSocketsAvailable == 0)
    {
      ftman->PushFileTransferEvent(Licq::FT_UPDATE);
      gettimeofday(&ftman->tv_lastupdate, NULL);
    }

    nCurrentSocket = 0;
    while (nSocketsAvailable > 0 && nCurrentSocket < l)
    {
      if (FD_ISSET(nCurrentSocket, &f_recv))
      {
        // New socket event ----------------------------------------------------
        if (nCurrentSocket == ftman->myThreadPipe.getReadFd())
        {
          char buf = ftman->myThreadPipe.getChar();
          if (buf == 'R')
          {
            DEBUG_THREADS("[FileTransferManager_tep] Reloading socket info.\n");
          }
          else if (buf == 'X')
          {
            DEBUG_THREADS("[FileTransferManager_tep] Exiting.\n");
            pthread_exit(NULL);
          }
        }

        // Connection on the server port ---------------------------------------
        else if (nCurrentSocket == ftman->ftServer.Descriptor())
        {
          if (ftman->mySock.Descriptor() != -1)
          {
            gLog.warning(tr("File Transfer: Receiving repeat incoming connection."));

            // Dump the extra connection to clear the listen socket queue
            Licq::TCPSocket ts;
            if (ftman->ftServer.RecvConnection(ts))
              ts.CloseConnection();
          }
          else
          {
            if (ftman->ftServer.RecvConnection(ftman->mySock))
            {
              ftman->sockman.AddSocket(&ftman->mySock);
              ftman->sockman.DropSocket(&ftman->mySock);

              ftman->m_nState = FT_STATE_HANDSHAKE;
              gLog.info(tr("File Transfer: Received connection."));
            }
            else
            {
              gLog.error(tr("File Transfer: Unable to receive new connection."));
            }
          }
        }

        // Message from connected socket----------------------------------------
        else if (nCurrentSocket == ftman->mySock.Descriptor())
        {
          ftman->mySock.Lock();
          bool ok = ftman->ProcessPacket();
          ftman->mySock.Unlock();
          if (!ok)
          {
            ftman->CloseConnection();
            ftman->PushFileTransferEvent(ftman->m_nResult);
          }
        }

        else
        {
          gLog.warning(tr("File Transfer: No such socket."));
        }

        nSocketsAvailable--;
      }
      else if (FD_ISSET(nCurrentSocket, &f_send))
      {
        if (nCurrentSocket == ftman->mySock.Descriptor())
        {
          ftman->mySock.Lock();
          bool ok = ftman->SendFilePacket();
          ftman->mySock.Unlock();
          if (!ok)
          {
            ftman->CloseConnection();
            ftman->PushFileTransferEvent(ftman->m_nResult);
          }
        }
        nSocketsAvailable--;
      }

      nCurrentSocket++;
    }
  }
  return NULL;
}

void* LicqIcq::FileWaitForSignal_tep(void* arg)
{
  pthread_detach(pthread_self());

  unsigned short nPort;
  struct SFileReverseConnectInfo *rc = (struct SFileReverseConnectInfo *)arg;
  pthread_mutex_t *cancel_mutex = &CFileTransferManager::thread_cancel_mutex;

  pthread_mutex_lock(cancel_mutex);
  pthread_cleanup_push(FileWaitForSignal_cleanup, arg);
    pthread_testcancel();
  pthread_cleanup_pop(0);
  Licq::UserId userId = rc->m->userId();
  nPort = rc->m->m_nPort;
  pthread_mutex_unlock(cancel_mutex);

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  gLog.info(tr("File Transfer: Waiting for reverse connection.\n"));
  bool bConnected = gIcqProtocol.waitForReverseConnection(rc->nId, userId);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  pthread_mutex_lock(cancel_mutex);
  pthread_cleanup_push(FileWaitForSignal_cleanup, arg);
    pthread_testcancel();
  pthread_cleanup_pop(0);

  if (bConnected || !rc->bTryDirect)
  {
    if (!bConnected)
      rc->m->PushFileTransferEvent(Licq::FT_ERRORxCONNECT);

    rc->m->m_bThreadRunning = false;
    pthread_mutex_unlock(cancel_mutex);

    delete rc;
    pthread_exit(NULL);
  }

  pthread_mutex_unlock(cancel_mutex);

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  gLog.info(tr("File Transfer: Reverse connection failed, trying direct."));
  Licq::TCPSocket s;
  bConnected = gIcqProtocol.openConnectionToUser(userId, &s, nPort);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  pthread_mutex_lock(cancel_mutex);
  pthread_cleanup_push(FileWaitForSignal_cleanup, arg);
    pthread_testcancel();
  pthread_cleanup_pop(0);

  if (bConnected)
  {
    if (rc->m->mySock.Descriptor() != -1)
    {
      gLog.warning(tr("File Transfer: Attempted connection when already connected."));
    }
    else
    {
      rc->m->mySock.TransferConnectionFrom(s);
      bConnected = rc->m->SendFileHandshake();
      rc->m->myThreadPipe.putChar('R');
    }
  }

  if (!bConnected)
    rc->m->PushFileTransferEvent(Licq::FT_ERRORxCONNECT);

  rc->m->m_bThreadRunning = false;

  pthread_mutex_unlock(cancel_mutex);
  delete rc;
  pthread_exit(NULL);
}

void LicqIcq::FileWaitForSignal_cleanup(void* arg)
{
  struct SFileReverseConnectInfo *rc = (struct SFileReverseConnectInfo *)arg;
  delete rc;

  pthread_mutex_unlock(&CFileTransferManager::thread_cancel_mutex);
}

CFileTransferManager *CFileTransferManager::FindByPort(unsigned short p)
{
  FileTransferManagerList::iterator iter;
  for (iter = ftmList.begin(); iter != ftmList.end(); ++iter)
  {
    if ( (*iter)->LocalPort() == p ) return *iter;
  }
  return NULL;
}


FileTransferManager::~FileTransferManager()
{
  // cancel the waiting thread first
  pthread_mutex_lock(&thread_cancel_mutex);
  if (m_bThreadRunning)
    pthread_cancel(m_tThread);
  pthread_mutex_unlock(&thread_cancel_mutex);

  CloseFileTransfer();

  // Delete any pending events
  IcqFileTransferEvent *e = NULL;
  while (ftEvents.size() > 0)
  {
    e = ftEvents.front();
    delete e;
    ftEvents.pop_front();
  }

  FileTransferManagerList::iterator fiter;
  for (fiter = ftmList.begin(); fiter != ftmList.end(); ++fiter)
  {
    if (*fiter == this) break;
  }
  if (fiter != ftmList.end()) ftmList.erase(fiter);
}

