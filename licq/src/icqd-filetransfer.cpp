// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2009 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <sys/time.h>

// Localization
#include "gettext.h"

#include "licq_filetransfer.h"
#include "licq_log.h"
#include "licq_constants.h"
#include "licq_icqd.h"
#include "licq_translate.h"
#include "licq_user.h"
#include "support.h"

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

using std::string;


//=====FILE==================================================================

//-----PacketFile---------------------------------------------------------------
CPacketFile::CPacketFile()
{
  // Empty
}

CPacketFile::~CPacketFile()
{
  // Empty
}

//-----FileInitClient-----------------------------------------------------------
CPFile_InitClient::CPFile_InitClient(char *_szLocalName,
                                    unsigned long _nNumFiles,
                                    unsigned long _nTotalSize)
{
  m_nSize = 20 + strlen(_szLocalName);
  InitBuffer();

  buffer->PackChar(0x00);
  buffer->PackUnsignedLong(0);
  buffer->PackUnsignedLong(_nNumFiles);
  buffer->PackUnsignedLong(_nTotalSize);
  buffer->PackUnsignedLong(0x64);
  buffer->PackString(_szLocalName);
}

CPFile_InitClient::~CPFile_InitClient()
{
  // Empty
}

//-----FileInitServer-----------------------------------------------------------
CPFile_InitServer::CPFile_InitServer(char *_szLocalName)
{
  m_nSize = 8 + strlen(_szLocalName);
  InitBuffer();

  buffer->PackChar(0x01);
  buffer->PackUnsignedLong(0x64);
  buffer->PackString(_szLocalName);
}

CPFile_InitServer::~CPFile_InitServer()
{
  // Empty
}

//-----FileBatch----------------------------------------------------------------
CPFile_Info::CPFile_Info(const char *_szFileName)
{
  m_bValid = true;
  m_nError = 0;

  const char* pcNoPath = NULL;
  struct stat buf;

  // Remove any path from the filename
  if ( (pcNoPath = strrchr(_szFileName, '/')) != NULL)
    m_szFileName = strdup(pcNoPath + 1);
  else
    m_szFileName = strdup(_szFileName);

  if (stat(_szFileName, &buf) < 0)
  {
    m_bValid = false;
    m_nError = errno;
    return;
  }
  m_nFileSize = buf.st_size;

  m_nSize = strlen(m_szFileName) + 21;
  InitBuffer();

  buffer->PackUnsignedShort(0x02);
  // Add all the file names
  buffer->PackString(m_szFileName);
  // Add the empty file name
  buffer->PackString("");
  //Add the file length
  buffer->PackUnsignedLong(m_nFileSize);
  buffer->PackUnsignedLong(0x00);
  buffer->PackUnsignedLong(0x64);
}


CPFile_Info::~CPFile_Info()
{
  free (m_szFileName);
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
CFileTransferEvent::CFileTransferEvent(unsigned char t, char *d)
{
  m_nCommand = t;
  if (d != NULL)
    m_szData = strdup(d);
  else
    m_szData = NULL;
}

CFileTransferEvent::~CFileTransferEvent()
{
  free(m_szData);
}


FileTransferManagerList CFileTransferManager::ftmList;

pthread_mutex_t CFileTransferManager::thread_cancel_mutex
                                                   = PTHREAD_MUTEX_INITIALIZER;

CFileTransferManager::CFileTransferManager(CICQDaemon* d, const char* accountId)
  : m_bThreadRunning(false)
{
  // Create the plugin notification pipe
  pipe(pipe_thread);
  pipe(pipe_events);

  if (myId != NULL)
    strcpy(myId, accountId);
  else
    myId[0] = '\0';
  m_nSession = rand();
  licqDaemon = d;

  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  strncpy(m_szLocalName, o->GetAlias(), sizeof(m_szLocalName) - 1);
  m_szLocalName[sizeof(m_szLocalName) - 1] = '\0';
  gUserManager.DropOwner(o);

  m_nCurrentFile = m_nBatchFiles = 0;
  m_nFileSize = m_nBatchSize = m_nFilePos = m_nBatchPos = 0;
  m_nBytesTransfered = m_nBatchBytesTransfered = 0;
  m_nStartTime = m_nBatchStartTime = 0;
  m_nFileDesc = -1;
  m_nState = FT_STATE_DISCONNECTED;
  m_bThreadCreated = false;

  m_szFileName[0] = m_szPathName[0] = '\0';
  strcpy(m_szRemoteName, myId);

  ftmList.push_back(this);
}


//-----CFileTransferManager::StartFileTransferServer-----------------------------------------
bool CFileTransferManager::StartFileTransferServer()
{
  if (licqDaemon->StartTCPServer(&ftServer) == -1)
  {
    gLog.Warn(tr("%sNo more ports available, add more or close open chat/file sessions.\n"), L_WARNxSTR);
    return false;
  }

  // Add the server to the sock manager
  sockman.AddSocket(&ftServer);
  sockman.DropSocket(&ftServer);

  return true;
}



bool CFileTransferManager::ReceiveFiles(const char *szDirectory)
{
  m_nDirection = D_RECEIVER;

  if (szDirectory == NULL)
  {
    snprintf(m_szDirectory, MAX_FILENAME_LEN, "%s/%s", BASE_DIR, myId);
    m_szDirectory[MAX_FILENAME_LEN - 1] = '\0';
    if (access(BASE_DIR, F_OK) < 0 && mkdir(m_szDirectory, 0700) == -1 &&
        errno != EEXIST)
    {
      gLog.Warn(tr("%sUnable to create directory %s for file transfer.\n"),
         L_WARNxSTR, m_szDirectory);
      strncpy(m_szDirectory, BASE_DIR, MAX_FILENAME_LEN - 1);
      m_szDirectory[MAX_FILENAME_LEN - 1] = '\0';
    }
  }
  else
  {
    strncpy(m_szDirectory, szDirectory, MAX_FILENAME_LEN - 1);
    m_szDirectory[MAX_FILENAME_LEN - 1] = '\0';
  }

  struct stat buf;
  stat(m_szDirectory, &buf);
  if (!S_ISDIR(buf.st_mode))
  {
    gLog.Warn(tr("%sPath %s is not a directory.\n"), L_WARNxSTR, m_szDirectory);
    return false;
  }

  if (!StartFileTransferServer())
  {
    PushFileTransferEvent(FT_ERRORxBIND);
    return false;
  }

  // Create the socket manager thread
  if (pthread_create(&thread_ft, NULL, &FileTransferManager_tep, this) == -1)
  {
    PushFileTransferEvent(FT_ERRORxRESOURCES);
    return false;
  }

  m_bThreadCreated = true;

  return true;
}


//-----CFileTransferManager::StartAsClient-------------------------------------------
void CFileTransferManager::SendFiles(ConstFileList lPathNames, unsigned short nPort)
{
  m_nDirection = D_SENDER;

  // Validate the pathnames
  if (lPathNames.size() == 0) return;

  struct stat buf;
  ConstFileList::iterator iter;
  for (iter = lPathNames.begin(); iter != lPathNames.end(); ++iter)
  {
    if (stat(*iter, &buf) == -1)
    {
      gLog.Warn(tr("%sFile Transfer: File access error %s:\n%s%s.\n"), L_WARNxSTR,
         *iter, L_BLANKxSTR, strerror(errno));
      continue;
    }
    m_lPathNames.push_back(const_cast<char*>(*iter));
    m_nBatchFiles++;
    m_nBatchSize += buf.st_size;
  }
  m_iPathName = m_lPathNames.begin();
  strcpy(m_szPathName, *m_iPathName);
  m_nPort = nPort;

  // start the server anyway, may need to do reverse connect
  if (!StartFileTransferServer())
  {
    PushFileTransferEvent(FT_ERRORxBIND);
    return;
  }

  // Create the socket manager thread
  if (pthread_create(&thread_ft, NULL, &FileTransferManager_tep, this) == -1)
  {
    PushFileTransferEvent(FT_ERRORxRESOURCES);
    return;
  }

  m_bThreadCreated = true;
}


//-----CFileTransferManager::ConnectToFileServer-----------------------------
bool CFileTransferManager::ConnectToFileServer(unsigned short nPort)
{
  const ICQUser* u = gUserManager.FetchUser(myId, LICQ_PPID, LOCK_R);
  if (u == NULL)
    return false;

  bool bTryDirect = u->Version() <= 6 || u->Mode() == MODE_DIRECT;
  bool bSendIntIp = u->SendIntIp();
  gUserManager.DropUser(u);

  bool bSuccess = false;
  if (bTryDirect)
  {
    gLog.Info("%sFile Transfer: Connecting to server.\n", L_TCPxSTR);
    bSuccess = licqDaemon->OpenConnectionToUser(myId, &ftSock, nPort);
   }

  bool bResult = false;
  if (!bSuccess)
  {
    const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
    unsigned long nIp = bSendIntIp ? o->IntIp() : o->Ip();
    gUserManager.DropOwner(o);

    // try reverse connect
    int nId = licqDaemon->RequestReverseConnection(myId, 0, nIp, LocalPort(),
                                                                        nPort);

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
  gLog.Info(tr("%sFile Transfer: Shaking hands.\n"), L_TCPxSTR);

  // Send handshake packet:
  const ICQUser* u = gUserManager.FetchUser(myId, LICQ_PPID, LOCK_R);
  unsigned short nVersion = u->ConnectionVersion();
  gUserManager.DropUser(u);
  if (!CICQDaemon::Handshake_Send(&ftSock, myId, LocalPort(), nVersion, false))
    return false;

  // Send init packet:
  CPFile_InitClient p(m_szLocalName, m_nBatchFiles, m_nBatchSize);
  if (!SendPacket(&p)) return false;

  gLog.Info(tr("%sFile Transfer: Waiting for server to respond.\n"), L_TCPxSTR);

  m_nState = FT_STATE_WAITxFORxSERVERxINIT;

  sockman.AddSocket(&ftSock);
  sockman.DropSocket(&ftSock);

  return true;
}


//-----CFileTransferManager::AcceptReverseConnection-------------------------
void CFileTransferManager::AcceptReverseConnection(TCPSocket *s)
{
  if (ftSock.Descriptor() != -1)
  {
    gLog.Warn(tr("%sFile Transfer: Attempted reverse connection when already connected.\n"),
       L_WARNxSTR);
    return;
  }

  ftSock.TransferConnectionFrom(*s);
  sockman.AddSocket(&ftSock);
  sockman.DropSocket(&ftSock);

  m_nState = FT_STATE_WAITxFORxCLIENTxINIT;

  // Reload socket info
  write(pipe_thread[PIPE_WRITE], "R", 1);

  gLog.Info(tr("%sFile Transfer: Received reverse connection.\n"), L_TCPxSTR);
}


//-----CFileTransferManager::ProcessPacket-------------------------------------------
bool CFileTransferManager::ProcessPacket()
{
  if (!ftSock.RecvPacket())
  {
    char buf[128];
    if (ftSock.Error() == 0)
      gLog.Info(tr("%sFile Transfer: Remote end disconnected.\n"), L_TCPxSTR);
    else
      gLog.Warn(tr("%sFile Transfer: Lost remote end:\n%s%s\n"), L_WARNxSTR,
                L_BLANKxSTR, ftSock.ErrorStr(buf, 128));
    if (m_nState == FT_STATE_WAITxFORxFILExINFO)
      m_nResult = FT_DONExBATCH;
    else
      m_nResult = FT_ERRORxCLOSED;
    return false;
  }

  if (!ftSock.RecvBufferFull()) return true;
  CBuffer &b = ftSock.RecvBuffer();

  switch(m_nState)
  {
    // Server States

    case FT_STATE_HANDSHAKE:
    {
      CBuffer tmp(b); // we need to save a copy for later
      
      if (!CICQDaemon::Handshake_Recv(&ftSock, LocalPort(), false)) break;
      gLog.Info(tr("%sFile Transfer: Received handshake.\n"), L_TCPxSTR);
      
      unsigned long nId = 0;
      if (ftSock.Version() == 7 || ftSock.Version() == 8)
      {
        CPacketTcp_Handshake_v7 hand(&tmp);
        nId = hand.Id();
      }

      if (nId != 0)
      {
        pthread_mutex_lock(&licqDaemon->mutex_reverseconnect);
        std::list<CReverseConnectToUserData *>::iterator iter;
        bool bFound = false;
        for (iter = licqDaemon->m_lReverseConnect.begin();
                          iter != licqDaemon->m_lReverseConnect.end();  ++iter)
        {
          if ((*iter)->nId == nId && (*iter)->myIdString == myId)
          {
            bFound = true;
            (*iter)->bSuccess = true;
            (*iter)->bFinished = true;
            pthread_cond_broadcast(&licqDaemon->cond_reverseconnect_done);
            break;
          }
        }
        pthread_mutex_unlock(&licqDaemon->mutex_reverseconnect);

        if (bFound)
        {
          // Send init packet:
          CPFile_InitClient p(m_szLocalName, m_nBatchFiles, m_nBatchSize);
          if (!SendPacket(&p))
          {
            m_nResult = FT_ERRORxCLOSED;
            return false;
          }

          gLog.Info("%sFile Transfer: Waiting for server to respond.\n",
            L_TCPxSTR);

          m_nState = FT_STATE_WAITxFORxSERVERxINIT;
          break;
        }
      }

      m_nState = FT_STATE_WAITxFORxCLIENTxINIT;
      break;
    }

    case FT_STATE_WAITxFORxCLIENTxINIT:
    {
      unsigned char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.Info(tr("%sFile Transfer: Speed set to %ld%%.\n"), L_TCPxSTR, nSpeed);
        break;
      }
      if (nCmd != 0x00)
      {
        char *pbuf;
        gLog.Error("%sFile Transfer: Invalid client init packet:\n%s%s\n",
                   L_ERRORxSTR, L_BLANKxSTR, b.print(pbuf));
        delete [] pbuf;
        m_nResult = FT_ERRORxHANDSHAKE;
        return false;
      }
      b.UnpackUnsignedLong();
      m_nBatchFiles = b.UnpackUnsignedLong();
      m_nBatchSize = b.UnpackUnsignedLong();
      m_nSpeed = b.UnpackUnsignedLong();
      b.UnpackString(m_szRemoteName, sizeof(m_szRemoteName));

      m_nBatchStartTime = time(TIME_NOW);
      m_nBatchBytesTransfered = m_nBatchPos = 0;

      PushFileTransferEvent(FT_STARTxBATCH);

      // Send speed response
      CPFile_SetSpeed p1(100);
      if (!SendPacket(&p1))
      {
        m_nResult = FT_ERRORxCLOSED;
        return false;
      }

      // Send response
      CPFile_InitServer p(m_szLocalName);
      if (!SendPacket(&p))
      {
        m_nResult = FT_ERRORxCLOSED;
        return false;
      }

      gLog.Info(tr("%sFile Transfer: Waiting for file info.\n"), L_TCPxSTR);
      m_nState = FT_STATE_WAITxFORxFILExINFO;
      break;
    }

    case FT_STATE_WAITxFORxFILExINFO:
    {
      unsigned char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.Info(tr("%sFile Transfer: Speed set to %ld%%.\n"), L_TCPxSTR, nSpeed);
        break;
      }
      if (nCmd == 0x06 && b.getDataSize() == 1)
      {
        gLog.Info(tr("%sFile Transfer: Ignoring a possible erroneous packet.\n"),
                  L_WARNxSTR);
        break;
      }
      if (nCmd != 0x02)
      {
        char *pbuf;
        gLog.Error("%sFile Transfer: Invalid file info packet:\n%s%s\n",
                   L_ERRORxSTR, L_BLANKxSTR, b.print(pbuf));
        delete [] pbuf;
        m_nResult = FT_ERRORxHANDSHAKE;
        return false;
      }
      b.UnpackChar();
      b.UnpackString(m_szFileName, sizeof(m_szFileName));

      // Remove any preceeding path info from the filename for security
      // reasons
      char *pTmp, *pNoPath;
      for (pTmp = m_szFileName + strlen(m_szFileName);
           pTmp >= m_szFileName && *pTmp != '/';
           pTmp--)
        ;
      if (pTmp >= m_szFileName && *pTmp == '/')
      {
        pNoPath = strdup(pTmp + 1);
        strcpy(m_szFileName, pNoPath);
        free(pNoPath);
      }
      
      b.UnpackUnsignedShort(); // 0 length string...?
      b.UnpackChar();
      m_nFileSize = b.UnpackUnsignedLong();
      b.UnpackUnsignedLong();
      m_nSpeed = b.UnpackUnsignedLong();

      m_nBytesTransfered = 0;
      m_nCurrentFile++;
      
      gLog.Info(tr("File Transfer: Waiting for plugin to confirm file receive.\n"));
      
      m_nState = FT_STATE_CONFIRMINGxFILE;
      PushFileTransferEvent(new CFileTransferEvent(FT_CONFIRMxFILE, m_szPathName));
      break;
    }
    
    case FT_STATE_CONFIRMINGxFILE:
    {
      // Still waiting for the plugin to confirm
      gLog.Warn(tr("File Transfer: Still waiting for the plugin to confirm file receive..."));
      break;
    }

    case FT_STATE_RECEIVINGxFILE:
    {
      // if this is the first call to this function...
      if (m_nBytesTransfered == 0)
      {
        m_nStartTime = time(TIME_NOW);
        m_nBatchPos += m_nFilePos;
        gLog.Info(tr("%sFile Transfer: Receiving %s (%ld bytes).\n"), L_TCPxSTR,
           m_szFileName, m_nFileSize);
        PushFileTransferEvent(new CFileTransferEvent(FT_STARTxFILE, m_szPathName));
        gettimeofday(&tv_lastupdate, NULL);
      }

      // Write the new data to the file and empty the buffer
      CBuffer &b = ftSock.RecvBuffer();
      char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.Info(tr("%sFile Transfer: Speed set to %ld%%.\n"), L_TCPxSTR, nSpeed);
        break;
      }

      if (nCmd != 0x06)
      {
        gLog.Unknown(tr("%sFile Transfer: Invalid data (%c) ignoring packet.\n"),
           L_UNKNOWNxSTR, nCmd);
        break;
      }

      errno = 0;
      size_t nBytesWritten = write(m_nFileDesc, b.getDataPosRead(), b.getDataSize() - 1);
      if (nBytesWritten != b.getDataSize() - 1)
      {
        gLog.Error("%sFile Transfer: Write error:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR,
           errno == 0 ? "Disk full (?)" : strerror(errno));
        m_nResult = FT_ERRORxFILE;
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
          PushFileTransferEvent(FT_UPDATE);
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
        gLog.Info(tr("%sFile Transfer: %s received.\n"), L_TCPxSTR, m_szFileName);
      }
      else // nBytesLeft < 0
      {
        // Received too many bytes for the given size of the current file
        gLog.Warn(tr("%sFile Transfer: %s received %d too many bytes.\n"), L_WARNxSTR,
           m_szFileName, -nBytesLeft);
      }
      // Notify Plugin
      PushFileTransferEvent(new CFileTransferEvent(FT_DONExFILE, m_szPathName));

      // Now wait for a disconnect or another file
      m_nState = FT_STATE_WAITxFORxFILExINFO;
      break;
    }


    // Client States

    case FT_STATE_WAITxFORxSERVERxINIT:
    {
      char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.Info(tr("%sFile Transfer: Speed set to %ld%%.\n"), L_TCPxSTR, nSpeed);
        break;
      }
      if (nCmd != 0x01)
      {
        char *pbuf;
        gLog.Error("%sFile Transfer: Invalid server init packet:\n%s%s\n",
                   L_ERRORxSTR, L_BLANKxSTR, b.print(pbuf));
        delete [] pbuf;
        m_nResult = FT_ERRORxHANDSHAKE;
        return false;
      }
      m_nSpeed = b.UnpackUnsignedLong();
      b.UnpackString(m_szRemoteName, sizeof(m_szRemoteName));

      // Send file info packet
      CPFile_Info p(*m_iPathName);
      if (!p.IsValid())
      {
        gLog.Warn(tr("%sFile Transfer: Read error for %s:\n%s\n"), L_WARNxSTR,
           *m_iPathName, p.ErrorStr());
        m_nResult = FT_ERRORxFILE;
        return false;
      }
      if (!SendPacket(&p))
      {
        m_nResult = FT_ERRORxCLOSED;
        return false;
      }

      m_nFileSize = p.GetFileSize();
      strcpy(m_szFileName, p.GetFileName());

      m_nBatchStartTime = time(TIME_NOW);
      m_nBatchBytesTransfered = m_nBatchPos = 0;

      PushFileTransferEvent(FT_STARTxBATCH);

      m_nState = FT_STATE_WAITxFORxSTART;
      break;
    }

    case FT_STATE_WAITxFORxSTART:
    {
      // contains the seek value
      char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.Info(tr("%sFile Transfer: Speed set to %ld%%.\n"), L_TCPxSTR, nSpeed);
        break;
      }
      if (nCmd != 0x03)
      {
        char *pbuf;
        gLog.Error("%sFile Transfer: Invalid start packet:\n%s%s\n",
                   L_ERRORxSTR, L_BLANKxSTR, b.print(pbuf));
        delete [] pbuf;
        m_nResult = FT_ERRORxCLOSED;
        return false;
      }

      m_nBytesTransfered = 0;
      m_nCurrentFile++;

      m_nFilePos = b.UnpackUnsignedLong();

      m_nFileDesc = open(*m_iPathName, O_RDONLY);
      if (m_nFileDesc == -1)
      {
        gLog.Error("%sFile Transfer: Read error '%s':\n%s%s\n.", L_ERRORxSTR,
           *m_iPathName, L_BLANKxSTR, strerror(errno));
        m_nResult = FT_ERRORxFILE;
        return false;
      }

      if (lseek(m_nFileDesc, m_nFilePos, SEEK_SET) == -1)
      {
        gLog.Error("%sFile Transfer: Seek error '%s':\n%s%s\n.", L_ERRORxSTR,
                   m_szFileName, L_BLANKxSTR, strerror(errno));
        m_nResult = FT_ERRORxFILE;
        return false;
      }

      m_nState = FT_STATE_SENDINGxFILE;
      break;
    }

    case FT_STATE_SENDINGxFILE:
    {
      char nCmd = b.UnpackChar();
      if (nCmd == 0x05)
      {
        unsigned long nSpeed = b.UnpackUnsignedLong();
        gLog.Info(tr("%sFile Transfer: Speed set to %ld%%.\n"), L_TCPxSTR, nSpeed);
        break;
      }
      char *p;
      gLog.Unknown(tr("%sFile Transfer: Unknown packet received during file send:\n%s\n"),
         L_UNKNOWNxSTR, b.print(p));
      delete [] p;
      break;
    }


    default:
    {
      gLog.Error("%sInternal error: FileTransferManager::ProcessPacket(), invalid state (%d).\n",
         L_ERRORxSTR, m_nState);
      break;
    }

  } // switch

  ftSock.ClearRecvBuffer();

  return true;
}

// This function gives a callback opportunity for the plugin, just before
// the actual transfer begins
bool CFileTransferManager::StartReceivingFile(char *szFileName)
{
  gLog.Info(tr("%sFile Transfer: Received plugin confirmation.\n"), L_TCPxSTR);
      
  if (m_nState != FT_STATE_CONFIRMINGxFILE)
  {
     gLog.Warn("%sFile Transfer: StartReceivingFile called without a pending confirmation.\n",
        L_WARNxSTR);
     return false;
  }

  // If a different filename was specified, use it
  if (szFileName != NULL)
  {
    strncpy(m_szFileName, szFileName, sizeof(m_szFileName)-1);
    m_szFileName[sizeof(m_szFileName)-1] = '\0';
  }

  // Get the local filename and set the file offset (for resume)
  struct stat buf;
  m_nFileDesc = -1;
  snprintf(m_szPathName, MAX_FILENAME_LEN, "%s/%s", m_szDirectory, m_szFileName);
  m_szPathName[MAX_FILENAME_LEN - 1] = '\0';
  while (m_nFileDesc == -1)
  {
    if (stat(m_szPathName, &buf) != -1)
    {
      if ((unsigned long)buf.st_size >= m_nFileSize)
      {
        snprintf(m_szPathName, MAX_FILENAME_LEN, "%s/%s.%lu", m_szDirectory, m_szFileName, (unsigned long)time(NULL));
        m_szPathName[MAX_FILENAME_LEN - 1] = '\0';
      }
      m_nFileDesc = open(m_szPathName, O_WRONLY | O_CREAT | O_APPEND, 00600);
      m_nFilePos = buf.st_size;
    }
    else
    {
      m_nFileDesc = open(m_szPathName, O_WRONLY | O_CREAT, 00600);
      m_nFilePos = 0;
    }
    if (m_nFileDesc == -1)
    {
      gLog.Error("%sFile Transfer: Unable to open %s for writing:\n%s%s.\n",
         L_ERRORxSTR, m_szPathName, L_BLANKxSTR, strerror(errno));
      m_nResult = FT_ERRORxFILE;
      return false;
    }
  }

  // Send response
  CPFile_Start p(m_nFilePos, m_nCurrentFile);
  if (!SendPacket(&p))
  {
    gLog.Error("%sFile Transfer: Unable to send file receive start packet.\n", L_ERRORxSTR);
    m_nResult = FT_ERRORxCLOSED;
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
    m_nStartTime = time(TIME_NOW);
    m_nBatchPos += m_nFilePos;
    gLog.Info(tr("%sFile Transfer: Sending %s (%ld bytes).\n"), L_TCPxSTR,
       m_szPathName, m_nFileSize);
    PushFileTransferEvent(new CFileTransferEvent(FT_STARTxFILE, m_szPathName));
    gettimeofday(&tv_lastupdate, NULL);
  }

  int nBytesToSend = m_nFileSize - m_nFilePos;
  if (nBytesToSend > 2048) nBytesToSend = 2048;
  if (read(m_nFileDesc, pSendBuf, nBytesToSend) != nBytesToSend)
  {
    gLog.Error("%sFile Transfer: Error reading from %s:\n%s%s.\n", L_ERRORxSTR,
       m_szPathName, L_BLANKxSTR, strerror(errno));
    m_nResult = FT_ERRORxFILE;
    return false;
  }
  CBuffer xSendBuf(nBytesToSend + 1);
  xSendBuf.PackChar(0x06);
  xSendBuf.Pack(pSendBuf, nBytesToSend);
  if (!SendBuffer(&xSendBuf))
  {
    m_nResult = FT_ERRORxCLOSED;
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
      PushFileTransferEvent(FT_UPDATE);
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
    gLog.Info(tr("%sFile Transfer: Sent %s.\n"), L_TCPxSTR, m_szFileName);
  }
  else // nBytesLeft < 0
  {
    gLog.Info(tr("%sFile Transfer: Sent %s, %d too many bytes.\n"), L_TCPxSTR,
       m_szFileName, -nBytesLeft);
  }
  PushFileTransferEvent(new CFileTransferEvent(FT_DONExFILE, m_szPathName));

  // Go to the next file, if no more then close connections
  m_iPathName++;
  if (m_iPathName == m_lPathNames.end())
  {
    m_nResult = FT_DONExBATCH;
    return false;
  }
  else
  {
    // Send file info packet
    CPFile_Info p(*m_iPathName);
    if (!p.IsValid())
    {
      gLog.Warn(tr("%sFile Transfer: Read error for %s:\n%s\n"), L_WARNxSTR,
         *m_iPathName, p.ErrorStr());
      m_nResult = FT_ERRORxFILE;
      return false;
    }
    if (!SendPacket(&p))
    {
      m_nResult = FT_ERRORxCLOSED;
      return false;
    }

    m_nFileSize = p.GetFileSize();
    strcpy(m_szFileName, p.GetFileName());
    strcpy(m_szPathName, *m_iPathName);

    m_nState = FT_STATE_WAITxFORxSTART;
  }

  return true;
}



//-----CFileTransferManager::PopFileTransferEvent------------------------------
CFileTransferEvent *CFileTransferManager::PopFileTransferEvent()
{
  if (ftEvents.size() == 0) return NULL;

  CFileTransferEvent *e = ftEvents.front();
  ftEvents.pop_front();

  return e;
}


//-----CFileTransferManager::PushFileTransferEvent-----------------------------
void CFileTransferManager::PushFileTransferEvent(unsigned char t)
{
  PushFileTransferEvent(new CFileTransferEvent(t));
}

void CFileTransferManager::PushFileTransferEvent(CFileTransferEvent *e)
{
  ftEvents.push_back(e);
  write(pipe_events[PIPE_WRITE], "*", 1);
}


//-----CFileTransferManager::SendPacket----------------------------------------------
bool CFileTransferManager::SendPacket(CPacket *p)
{
  return SendBuffer(p->getBuffer());
}


//-----CFileTransferManager::SendBuffer----------------------------------------------
bool CFileTransferManager::SendBuffer(CBuffer *b)
{
  if (!ftSock.SendPacket(b))
  {
    char buf[128];
    gLog.Warn(tr("%sFile Transfer: Send error:\n%s%s\n"), L_WARNxSTR, L_BLANKxSTR,
       ftSock.ErrorStr(buf, 128));
    return false;
  }
  return true;
}


void CFileTransferManager::ChangeSpeed(unsigned short nSpeed)
{
  if (nSpeed > 100)
  {
    gLog.Warn(tr("%sInvalid file transfer speed: %d%%.\n"), L_WARNxSTR, nSpeed);
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
  if (pipe_thread[PIPE_WRITE] != -1)
  {
    write(pipe_thread[PIPE_WRITE], "X", 1);
    if (m_bThreadCreated)
      pthread_join(thread_ft, NULL);
    m_bThreadCreated = false;
   
    close(pipe_thread[PIPE_READ]);
    close(pipe_thread[PIPE_WRITE]);

    pipe_thread[PIPE_READ] = pipe_thread[PIPE_WRITE] = -1;
  }

  CloseConnection();
}


//----CFileTransferManager::CloseConnection----------------------------------
void CFileTransferManager::CloseConnection()
{
  sockman.CloseSocket(ftServer.Descriptor(), false, false);
  sockman.CloseSocket(ftSock.Descriptor(), false, false);
  m_nState = FT_STATE_DISCONNECTED;

  if (m_nFileDesc != -1)
  {
    close(m_nFileDesc);
    m_nFileDesc = -1;
  }
}



void *FileTransferManager_tep(void *arg)
{
  CFileTransferManager *ftman = (CFileTransferManager *)arg;

  fd_set f_recv, f_send;
  struct timeval *tv;
  struct timeval tv_updates = { 2, 0 };
  int l, nSocketsAvailable, nCurrentSocket;
  char buf[2];

  if (ftman->m_nDirection == D_SENDER)
  {
    if (!ftman->ConnectToFileServer(ftman->m_nPort))
    {
      ftman->PushFileTransferEvent(FT_ERRORxCONNECT);
      return NULL;
    }
  }
  else if (ftman->m_nDirection != D_RECEIVER)
    return NULL;

  while (true)
  {
    f_recv = ftman->sockman.SocketSet();
    l = ftman->sockman.LargestSocket() + 1;

    // Add the new socket pipe descriptor
    FD_SET(ftman->pipe_thread[PIPE_READ], &f_recv);
    if (ftman->pipe_thread[PIPE_READ] >= l)
      l = ftman->pipe_thread[PIPE_READ] + 1;

    // Set up the send descriptor
    FD_ZERO(&f_send);
    if (ftman->m_nState == FT_STATE_SENDINGxFILE)
    {
      FD_SET(ftman->ftSock.Descriptor(), &f_send);
      // No need to check "l" as ftSock is already in the read list
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

      gLog.Warn(tr("%sFile Transfer: select failed, aborting thread:\n%s%s\n"),
          L_WARNxSTR, L_BLANKxSTR, strerror(errno));
      pthread_exit(NULL);
    }

    // Check if we timed out
    if (tv != NULL && nSocketsAvailable == 0)
    {
      ftman->PushFileTransferEvent(FT_UPDATE);
      gettimeofday(&ftman->tv_lastupdate, NULL);
    }

    nCurrentSocket = 0;
    while (nSocketsAvailable > 0 && nCurrentSocket < l)
    {
      if (FD_ISSET(nCurrentSocket, &f_recv))
      {
        // New socket event ----------------------------------------------------
        if (nCurrentSocket == ftman->pipe_thread[PIPE_READ])
        {
          read(ftman->pipe_thread[PIPE_READ], buf, 1);
          if (buf[0] == 'R')
          {
            DEBUG_THREADS("[FileTransferManager_tep] Reloading socket info.\n");
          }
          else if (buf[0] == 'X')
          {
            DEBUG_THREADS("[FileTransferManager_tep] Exiting.\n");
            pthread_exit(NULL);
          }
        }

        // Connection on the server port ---------------------------------------
        else if (nCurrentSocket == ftman->ftServer.Descriptor())
        {
          if (ftman->ftSock.Descriptor() != -1)
          {
            gLog.Warn(tr("%sFile Transfer: Receiving repeat incoming connection.\n"), L_WARNxSTR);

            // Dump the extra connection to clear the listen socket queue
            TCPSocket ts;
            if (ftman->ftServer.RecvConnection(ts))
              ts.CloseConnection();
          }
          else
          {
            if (ftman->ftServer.RecvConnection(ftman->ftSock))
            {
              ftman->sockman.AddSocket(&ftman->ftSock);
              ftman->sockman.DropSocket(&ftman->ftSock);

              ftman->m_nState = FT_STATE_HANDSHAKE;
              gLog.Info(tr("%sFile Transfer: Received connection.\n"), L_TCPxSTR);
            }
            else
            {
              gLog.Error(tr("%sFile Transfer: Unable to receive new connection.\n"), L_ERRORxSTR);
            }
          }
        }

        // Message from connected socket----------------------------------------
        else if (nCurrentSocket == ftman->ftSock.Descriptor())
        {
          ftman->ftSock.Lock();
          bool ok = ftman->ProcessPacket();
          ftman->ftSock.Unlock();
          if (!ok)
          {
            ftman->CloseConnection();
            ftman->PushFileTransferEvent(ftman->m_nResult);
          }
        }

        else
        {
          gLog.Warn(tr("%sFile Transfer: No such socket.\n"), L_WARNxSTR);
        }

        nSocketsAvailable--;
      }
      else if (FD_ISSET(nCurrentSocket, &f_send))
      {
        if (nCurrentSocket == ftman->ftSock.Descriptor())
        {
          ftman->ftSock.Lock();
          bool ok = ftman->SendFilePacket();
          ftman->ftSock.Unlock();
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

void *FileWaitForSignal_tep(void *arg)
{
  pthread_detach(pthread_self());

  CICQDaemon *d;
  unsigned short nPort;
  struct SFileReverseConnectInfo *rc = (struct SFileReverseConnectInfo *)arg;
  pthread_mutex_t *cancel_mutex = &CFileTransferManager::thread_cancel_mutex;

  pthread_mutex_lock(cancel_mutex);
  pthread_cleanup_push(FileWaitForSignal_cleanup, arg);
    pthread_testcancel();
  pthread_cleanup_pop(0);
  d = rc->m->licqDaemon;
  string id = rc->m->Id();
  nPort = rc->m->m_nPort;
  pthread_mutex_unlock(cancel_mutex);

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  gLog.Info("%sFile Transfer: Waiting for reverse connection.\n", L_TCPxSTR);
  bool bConnected = d->WaitForReverseConnection(rc->nId, id.c_str());
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  pthread_mutex_lock(cancel_mutex);
  pthread_cleanup_push(FileWaitForSignal_cleanup, arg);
    pthread_testcancel();
  pthread_cleanup_pop(0);

  if (bConnected || !rc->bTryDirect)
  {
    if (!bConnected)
      rc->m->PushFileTransferEvent(FT_ERRORxCONNECT);

    rc->m->m_bThreadRunning = false;
    pthread_mutex_unlock(cancel_mutex);

    delete rc;
    pthread_exit(NULL);
  }

  pthread_mutex_unlock(cancel_mutex);

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  gLog.Info("%sFile Transfer: Reverse connection failed, trying direct.\n",
                                                                    L_TCPxSTR);
  TCPSocket s;
  bConnected = d->OpenConnectionToUser(id.c_str(), &s, nPort);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

  pthread_mutex_lock(cancel_mutex);
  pthread_cleanup_push(FileWaitForSignal_cleanup, arg);
    pthread_testcancel();
  pthread_cleanup_pop(0);

  if (bConnected)
  {
    if (rc->m->ftSock.Descriptor() != -1)
    {
      gLog.Warn("%sFile Transfer: Attempted connection when already"
                " connected.\n", L_WARNxSTR);
    }
    else
    {
      rc->m->ftSock.TransferConnectionFrom(s);
      bConnected = rc->m->SendFileHandshake();
      write(rc->m->pipe_thread[PIPE_WRITE], "R", 1);
    }
  }

  if (!bConnected)
    rc->m->PushFileTransferEvent(FT_ERRORxCONNECT);

  rc->m->m_bThreadRunning = false;

  pthread_mutex_unlock(cancel_mutex);
  delete rc;
  pthread_exit(NULL);
}

void FileWaitForSignal_cleanup(void *arg)
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


CFileTransferManager::~CFileTransferManager()
{
  // cancel the waiting thread first
  pthread_mutex_lock(&thread_cancel_mutex);
  if (m_bThreadRunning)
    pthread_cancel(m_tThread);
  pthread_mutex_unlock(&thread_cancel_mutex);

  CloseFileTransfer();

  // Delete any pending events
  CFileTransferEvent *e = NULL;
  while (ftEvents.size() > 0)
  {
    e = ftEvents.front();
    delete e;
    ftEvents.pop_front();
  }

  FileList::iterator iter;
  for (iter = m_lPathNames.begin(); iter != m_lPathNames.end(); ++iter)
  {
    free(*iter);
  }

  FileTransferManagerList::iterator fiter;
  for (fiter = ftmList.begin(); fiter != ftmList.end(); ++fiter)
  {
    if (*fiter == this) break;
  }
  if (fiter != ftmList.end()) ftmList.erase(fiter);
}

