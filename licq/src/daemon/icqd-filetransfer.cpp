#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "licq_filetransfer.h"
#include "licq_log.h"
#include "licq_constants.h"
#include "licq_icqd.h"
#include "licq_translate.h"
#include "licq_sighandler.h"
#include "support.h"

#define DEBUG_THREADS(x)

const unsigned short FT_STATE_DISCONNECTED = 0;
const unsigned short FT_STATE_HANDSHAKE = 1;
const unsigned short FT_STATE_ = 2;
//const unsigned short FT_STATE_WAITxFORx = 3;
//const unsigned short FT_STATE_WAITxFORx = 4;
const unsigned short FT_STATE_CONNECTED = 5;



//=====FILE==================================================================


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


//-----FileInitServer-----------------------------------------------------------
CPFile_InitServer::CPFile_InitServer(char *_szLocalName)
{
  m_nSize = 8 + strlen(_szLocalName);
  InitBuffer();

  buffer->PackChar(0x01);
  buffer->PackUnsignedLong(0x64);
  buffer->PackString(_szLocalName);
}


//-----FileBatch----------------------------------------------------------------
CPFile_Info::CPFile_Info(const char *_szFileName)
{
  m_bValid = true;
  m_nError = 0;

  char *pcNoPath = NULL;
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
CPFile_Start::CPFile_Start(unsigned long _nFilePos)
{
  m_nSize = 13;
  InitBuffer();

  buffer->PackChar(0x03);
  buffer->PackUnsignedLong(_nFilePos);
  buffer->PackUnsignedLong(0x00);
  buffer->PackUnsignedLong(0x64);
}


//=====FileTransferManager===========================================================
CFileTransferManager::CFileTransferManager(CICQDaemon *d, unsigned long nUin)
{
  // Create the plugin notification pipe
  pipe(pipe_thread);
  pipe(pipe_events);

  m_nUin = nUin;
  m_nSession = rand();
  licqDaemon = d;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  strcpy(m_szName, o->GetAlias());
  gUserManager.DropOwner();

  m_nCurrentFile = m_nTotalFiles = 0;
  m_nFileSize = m_nBatchSize = m_nFilePos = m_nBatchPos = 0;
  m_nBytesTransfered = m_nBatchBytesTransfered = 0;
  m_nStartTime = m_nBatchStartTime = 0;
  m_nFileDesc = -1;
}


//-----CFileTransferManager::StartFileTransferServer-----------------------------------------
bool CFileTransferManager::StartFileTransferServer()
{
  if (licqDaemon->StartTCPServer(&ftServer) == -1)
  {
    gLog.Warn("%sNo more ports available, add more or close open chat/file sessions.\n", L_WARNxSTR);
    return false;
  }

  // Add the server to the sock manager
  sockman.AddSocket(&ftServer);
  sockman.DropSocket(&ftServer);

  return true;
}



bool CFileTransferManager::StartAsServer()
{
  if (!StartFileTransferServer()) return false;

  // Create the socket manager thread
  if (pthread_create(&thread_ft, NULL, &FileTransferManager_tep, this) == -1)
    return false;

  return true;
}


//-----CFileTransferManager::StartAsClient-------------------------------------------
bool CFileTransferManager::StartAsClient(unsigned short nPort)
{
  //if (!StartFileTransferServer()) return false;

  if (!ConnectToFileServer(nPort)) return false;

  // Create the socket manager thread
  if (pthread_create(&thread_ft, NULL, &FileTransferManager_tep, this) == -1)
    return false;

  return true;
}


//-----CFileTransferManager::ConnectToFileServer-----------------------------
bool CFileTransferManager::ConnectToFileServer(unsigned short nPort)
{
  gLog.Info("%sFile Transfer: Connecting to server.\n", L_TCPxSTR);
  if (!licqDaemon->OpenConnectionToUser(m_nUin, &ftSock, nPort))
  {
    return false;
  }

  gLog.Info("%sFile Transfer: Shaking hands.\n", L_TCPxSTR);

  // Send handshake packet:
  CPacketTcp_Handshake p_handshake(ftSock.LocalPort());
  SendPacket(&p_handshake);

  // Send init packet:
  //CPFile_InitClient p(m_szName, 1, m_nFileSize);
  //SendPacket(&p);

  gLog.Info("%sFile Transfer: Waiting for ...\n", L_TCPxSTR);

  m_nState = FT_STATE_;

  sockman.AddSocket(&ftSock);
  sockman.DropSocket(&ftSock);

  return true;
}


//-----CFileTransferManager::ProcessPacket-------------------------------------------
bool CFileTransferManager::ProcessPacket()
{
  if (!ftSock.RecvPacket())
  {
    char buf[128];
    if (ftSock.Error() == 0)
      gLog.Info("%sFile Transfer: Remote end disconnected.\n", L_TCPxSTR);
    else
      gLog.Info("%sFile Transfer: Lost remote end:\n%s%s\n", L_TCPxSTR,
                L_BLANKxSTR, ftSock.ErrorStr(buf, 128));
    return false;
  }

  if (!ftSock.RecvBufferFull()) return true;
  CBuffer &b = ftSock.RecvBuffer();

  switch(m_nState)
  {
    case FT_STATE_HANDSHAKE:
    {
      unsigned char cHandshake = b.UnpackChar();
      if (cHandshake != ICQ_CMDxTCP_HANDSHAKE)
      {
        gLog.Warn("%sFile Transfer: Bad handshake (%04X).\n", L_WARNxSTR, cHandshake);
        break;
      }

      gLog.Info("%sFile Transfer: Received handshake.\n", L_TCPxSTR);
      m_nState = FT_STATE_;
      break;
    }

    case FT_STATE_CONNECTED:
    {
      break;
    }

    default:
      gLog.Error("%sInternal error: FileTransferManager::ProcessPacket(), invalid state (%d).\n",
         L_ERRORxSTR, m_nState);
      break;

  } // switch

  ftSock.ClearRecvBuffer();

  return true;
}


//-----CFileTransferManager::PopChatEvent--------------------------------------------
CFileTransferEvent *CFileTransferManager::PopFileTransferEvent()
{
  if (ftEvents.size() == 0) return NULL;

  CFileTransferEvent *e = ftEvents.front();
  ftEvents.pop_front();

  return e;
}


//-----CFileTransferManager::PushChatEvent-------------------------------------------
void CFileTransferManager::PushFileTransferEvent(CFileTransferEvent *e)
{
  ftEvents.push_back(e);
  write(pipe_events[PIPE_WRITE], "*", 1);
}


//-----CFileTransferManager::SendPacket----------------------------------------------
void CFileTransferManager::SendPacket(CPacket *p)
{
  SendBuffer(p->getBuffer());
}


//-----CFileTransferManager::SendBuffer----------------------------------------------
void CFileTransferManager::SendBuffer(CBuffer *b)
{
  if (!ftSock.SendPacket(b))
  {
    char buf[128];
    gLog.Warn("%sFile Transfer: Send error:\n%s%s\n", L_WARNxSTR, L_BLANKxSTR,
               ftSock.ErrorStr(buf, 128));
    //CloseFileTransfer(); FIXME
  }
}

/*
void CFileTransferManager::SendCharacter(char c)
{
  CBuffer buf(1);
  gTranslator.ClientToServer(c);
  buf.PackChar(c);
  SendBuffer(&buf);
}
*/

void CFileTransferManager::ChangeSpeed(unsigned short nSpeed)
{
  if (nSpeed > 100)
  {
    gLog.Warn("%sInvalid file transfer speed: %d%%.\n", L_WARNxSTR, nSpeed);
    return;
  }

  //CPFile_ChangeSpeed p(nSpeed);
  //SendPacket(&p);
  m_nSpeed = nSpeed;
}



//----CFileTransferManager::CloseFileTransfer--------------------------------
void CFileTransferManager::CloseFileTransfer()
{
  sockman.CloseSocket(ftSock.Descriptor(), false, false);
  sockman.CloseSocket(ftServer.Descriptor(), false, false);
  m_nState = FT_STATE_DISCONNECTED;

  // Close the chat thread
  if (pipe_thread[PIPE_WRITE] != -1)
  {
    write(pipe_thread[PIPE_WRITE], "X", 1);
    pthread_join(thread_ft, NULL);

    close(pipe_thread[PIPE_READ]);
    close(pipe_thread[PIPE_WRITE]);

    pipe_thread[PIPE_READ] = pipe_thread[PIPE_WRITE] = -1;
  }
}



void *FileTransferManager_tep(void *arg)
{
  CFileTransferManager *ftman = (CFileTransferManager *)arg;

  licq_segv_handler(&signal_handler_ftThread);

  fd_set f;
  int l, nSocketsAvailable, nCurrentSocket;
  char buf[2];

  while (true)
  {
    f = ftman->sockman.SocketSet();
    l = ftman->sockman.LargestSocket() + 1;

    // Add the new socket pipe descriptor
    FD_SET(ftman->pipe_thread[PIPE_READ], &f);
    if (ftman->pipe_thread[PIPE_READ] >= l)
      l = ftman->pipe_thread[PIPE_READ] + 1;

    nSocketsAvailable = select(l, &f, NULL, NULL, NULL);

    nCurrentSocket = 0;
    while (nSocketsAvailable > 0 && nCurrentSocket < l)
    {
      if (FD_ISSET(nCurrentSocket, &f))
      {
        // New socket event ----------------------------------------------------
        if (nCurrentSocket == ftman->pipe_thread[PIPE_READ])
        {
          read(ftman->pipe_thread[PIPE_READ], buf, 1);
          if (buf[0] == 'S')
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
            gLog.Warn("%sFile Transfer: Receiving repeat incoming connection.\n", L_WARNxSTR);
          }
          else
          {
            ftman->ftServer.RecvConnection(ftman->ftSock);
            ftman->sockman.AddSocket(&ftman->ftSock);
            ftman->sockman.DropSocket(&ftman->ftSock);

            ftman->m_nState = FT_STATE_HANDSHAKE;
            gLog.Info("%sFile Transfer: Received connection.\n", L_TCPxSTR);
          }
        }

        // Message from connected socket----------------------------------------
        else if (nCurrentSocket == ftman->ftSock.Descriptor())
        {
          ftman->ftSock.Lock();
          /*bool ok =*/ ftman->ProcessPacket();
          ftman->ftSock.Unlock();
          //if (!ok) ftman->CloseClient(u); FIXME
        }

        else
        {
          gLog.Warn("%sFile Transfer: No such socket.\n", L_WARNxSTR);
        }

        nSocketsAvailable--;
      }
      nCurrentSocket++;
    }
  }
  return NULL;
}



CFileTransferManager::~CFileTransferManager()
{
  CloseFileTransfer();

  // Delete any pending events
  CFileTransferEvent *e = NULL;
  while (ftEvents.size() > 0)
  {
    e = ftEvents.front();
    delete e;
    ftEvents.pop_front();
  }
}

