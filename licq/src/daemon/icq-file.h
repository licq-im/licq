#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// State table defines
#define STATE_RECVxHANDSHAKE 1
#define STATE_RECVxCLIENTxINIT 2
#define STATE_RECVxBATCHxINFO 3
#define STATE_RECVxFILE 4
#define STATE_RECVxSERVERxINIT 5
#define STATE_RECVxSTART 6
#define STATE_SENDxFILE 7
#define STATE_DONExRECVxFILE 8
#define STATE_DONExSENDxFILE 9

struct SFileInfo
{
   char szName[MAX_FILENAME_LEN];
   unsigned long nSize;
};

struct SFileSession
{
  vector <struct SFileInfo> vfBatch;
  unsigned short nPort, nCurrentFile, nState;
  int nFileDesc;
  char *szLocalName, *szRemoteName;  
  TCPSocket sServer, sClient;
  pthread_t thread_filetransfer;
  CICQDaemon *daemon;
  
  // Batch statistics
  unsigned long nBatchSize, nTotalFiles, nBatchPos;
  time_t nBatchStartTime;
  unsigned long nBatchBytesTransfered;

  // Current file statistics
  unsigned long nFileSize, nFilePos;
  time_t nStartTime;
  unsigned long nBytesTransfered;
};


void FileDone(SFileSession *f, unsigned short _nState)
{
  f->sServer.CloseConnection();
  f->sClient.CloseConnection();
  f->nState = _nState;
  pthread_exit(NULL);
}

//=====Server Side==============================================================

//-----StartFileServer----------------------------------------------------------
bool CICQDaemon::StartFileServer(SFileSession *f)
{
  gLog.Info("%sStarting file server on port %d.\n", L_TCPxSTR, f->nPort);
  if (!(f->sServer.StartServer(f->nPort)))
  {
    gLog.Error("%sFile transfer - error creating local socket:\n%s%s\n", L_ERRORxSTR, 
               L_BLANKxSTR, f->sServer.ErrorStr(buf, 128));
    return false;
  }
  f->nPort = f->sServer.LocalPort());
  f->daemon = this;
  f->nState = STATE_WAITxCONNECTION;
  pthread_create(&f->thread_filetransfer, NULL, &FileServer_tep, f);

  return true;
}


void *FileServer_tep(void *p)
{
  pthread_detach(pthread_self());

  struct SFileSession *f = (struct SFileSession *)p;
  fd_set fdset;
  struct timeval tv;

  // Wait for a connection on the server socket
  while (f->nState == STATE_WAITINGxCONNECTION)
  {
    tv.tv_sec = MAX_WAIT_ACK;
    tv.tv_usec = 0;
    FD_CLEAR(&fdset);
    FD_SET(f->sServer.Descriptor(), &fdset);
    int n = select(f->sServer.Descriptor() + 1, &fdset, NULL, NULL, &tv);
    if (n == -1)
    {
      gLog.Error(...);
      FileDone(f, STATE_FAILED);
    }
    else if (n == 1)
    {
      f->sServer.RecvConnection(f->sClient);
      f->nState = STATE_RECVxHANDSHAKE;
    }
  }

  // State table for the client socket
  while (true)
  {
    // Receive data on the socket
    do
    {
      tv.tv_sec = MAX_WAIT_ACK;
      tv.tv_usec = 0;
      FD_CLEAR(&fdset);
      FD_SET(f->sServer.Descriptor(), &fdset);
      int n = select(f->sServer.Descriptor() + 1, &fdset, NULL, NULL, &tv);
      if (n == -1) 
      {
        gLog.Error(...);
        FileDone(f, STATE_FAILED);
      }
    } while (n != 1);
    
    if (!f->sClient.RecvPacket())
    {
      if (f->sClient.Error() == 0)
        gLog.Info("%sFile transfer - remote end disconnected.\n", L_ERRORxSTR);
      else 
        gLog.Error("%sFile transfer receive error - lost remote end:\n%s%s\n", L_ERRORxSTR, 
                   L_BLANKxSTR, f->sClient.ErrorStr(buf, 128));
      FileDone(f, STATE_FAILED);      
    }
    if (!f->sClient.RecvBufferFull()) continue;


    // Take action depending on our state
    switch(f->nState)
    {
    case STATE_RECVxHANDSHAKE:
    {
      unsigned char cHandshake;
      f->sClient.RecvBuffer() >> cHandshake;
      if (cHandshake != ICQ_CMDxTCP_HANDSHAKE)
      {
        gLog.Error("%sReceive error - bad handshake (%04X).\n", L_ERRORxSTR, 
                   cHandshake);
        FileDone(f, STATE_FAILED);
      }
      f->nState = STATE_RECVxCLIENTxINIT;
      break;
    }

    case STATE_RECVxCLIENTxINIT:
    {
      // Process init packet
      char cJunk, t[64];
      unsigned long nJunkLong;
      unsigned short nRemoteNameLen;
      f->sClient.RecvBuffer() >> cJunk;
      if (cJunk != 0x00)
      {
        char *pbuf;
        gLog.Error("%sError receiving data: invalid client init packet:\n%s%s\n", 
                   L_ERRORxSTR, L_BLANKxSTR, f->sClient.RecvBuffer().print(pbuf));
        delete [] pbuf;
        FileDone(f, STATE_FAILED);
      }
      f->sClient.RecvBuffer() >> nJunkLong
                              >> f->nTotalFiles
                              >> f->nBatchSize
                              >> nJunkLong
                              >> nRemoteNameLen;
      f->szRemoteName = new char[nRemoteNameLen];
      for (int i = 0; i < nRemoteNameLen; i++) 
         f->sClient.RecvBuffer() >> f->szRemoteName[i];

      // Send response
      CPFile_InitServer p(m_szLocalName);
      f->sClient.SendPacket(*p.getBuffer());

      f->nState = STATE_RECVxBATCHxINFO;
      break;
    }

    case STATE_RECVxBATCHxINFO:
    {
      // Process batch packet
      unsigned short nNumFiles, nLen;
      char t[64];
      f->sClient.RecvBuffer() >> nNumFiles;
      for (int i = 0; i < nNumFiles; i++) 
      {
         f->sClient.RecvBuffer() >> nLen;
         struct SFileInfo s;
         f->vfBatch.push_back(s);
         for (int j = 0; j < nLen; j++)
            f->sClient.RecvBuffer() >> f->vfBatch[i].szName[j];
      }
      for (int i = 0; i < nNumFiles; i++) 
         f->sClient.RecvBuffer() >> f->vfBatch[i].nSize;

      // Remove the last file as it is a blank one
      f->vfBatch.pop_back();
      f->nCurrentFile = 0;
      
//----------------------------
      // Get the local filename and set the file offset (for resume)
      if (!GetLocalFileName())
      {
        FileDone(f, STATE_FAILED);
      }
//----------------------------

      // Send response
      CPFile_Start p(m_nFilePos);
      m_xSocketFile.SendPacket(*p.getBuffer());
      lblStatus->setText("Starting transfer...");

      f->nBatchStartTime = time(NULL);
      f->nBatchBytesTransfered = f->nBatchPos = 0;
      f->nState = STATE_RECVxFILE;
      break;
    }

    case STATE_RECVxFILE:
    {
      // if this is the first call to this function for the current file
      if (f->nBytesTransfered == 0) 
      {
        f->nStartTime = time(NULL);
        f->nBatchPos += f->nFilePos;        
      }

      // Write the new data to the file and empty the buffer
      CBuffer &b = f->sClient.RecvBuffer();
      char cTest;
      b >> cTest;
      if (cTest != 0x06)
      {
        gLog.Error("%sFile receive error, invalid data (%c).  Ignoring packet.\n", cTest);
                   //, L_BLANKxSTR, m_xSocketFile.RecvBuffer().print());
        f->sClient.ClearRecvBuffer();  
        continue;
      }
      errno = 0;
      size_t nBytesWritten = write(f->nFileDesc, b.getDataPosRead(), b.getDataSize() - 1);
      if (nBytesWritten != b.getDataSize() - 1)
      {
        gLog.Error("%sFile write error:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR, 
                   errno == 0 ? "Disk full (?)" : strerror(errno));
        FileDone(f, STATE_FAILED);
      }

      f->nFilePos += nBytesWritten;
      f->nBytesTransfered += nBytesWritten;
      f->nBatchPos += nBytesWritten;
      f->nBatchBytesTransfered += nBytesWritten;
      
      f->sClient.ClearRecvBuffer();

      int nBytesLeft = f->vfBatch[f->nCurrentFile].nSize - f->nFilePos;
      if (nBytesLeft > 0) 
      {
        // More bytes to come so go away and wait for them
        continue;
      }
      else if (nBytesLeft == 0) 
      {
        // File transfer done perfectly
        ::close(f->nFileDesc);
        f->nFileDesc = 0;
        //gLog.Info(S_MSGBOX, "%sFile transfer of '%s' from %s completed successfully.\n", 
        //          L_TCPxSTR, f->vfBatch[f->nCurrentFile].szName, f->szRemoteName);
      }
      else // nBytesLeft < 0
      {
        // Received too many bytes for the given size of the current file
        ::close(f->nFileDesc);
        f->nFileDesc = 0;
        gLog.Error("%sFile transfer of '%s' from %s received %d too many bytes.\n%sClosing file, recommend check for errors.\n", 
                   L_TCPxSTR, f->vfBatch[f->nCurrentFile].szName, f->szRemoteName, 
                   -nBytesLeft, L_BLANKxSTR);
      }
      // Only get here if the current file is done
      f->nCurrentFile++;
      if (f->nCurrentFile == f->vfBatch.size())
      {
        FileDone(f, STATE_DONE);
      }
      else
      {
        // Prepare for another file
      }
      //m_nState = STATE_DONExRECVxFILE;      
    }

    /*case STATE_DONExRECVxFILE:
      // I don't know what this would be...
      break;*/

    }  // end-of-switch
    
    f->sClient.ClearRecvBuffer();
  }  // end-of-while

}



//=====Client Side==============================================================

//-----startAsClient------------------------------------------------------------
bool CFileDlg::startAsClient()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  unsigned long nIp = u->Ip();
  gUserManager.DropUser(u);
  gLog.Info("%sFile transfer - connecting to %ld:%d.\n", L_TCPxSTR,
             inet_ntoa_r(*(struct in_addr *)&nIp, buf), getPort());
   lblStatus->setText("Connecting to remote...");
   m_xSocketFile.SetRemoteAddr(nIp, getPort());
   if (!m_xSocketFile.OpenConnection()) 
   {
     gLog.Error("%sUnable to connect to remote file server:\n%s%s.", L_ERRORxSTR, 
                L_BLANKxSTR, m_xSocketFile.ErrorStr(buf, 128));
     return false;
   }
   
   lblStatus->setText("Connected, shaking hands...");

   // Send handshake packet:
   CPacketTcp_Handshake p_handshake(getLocalPort());
   m_xSocketFile.SendPacket(*p_handshake.getBuffer());

   // Send init packet:
   CPFile_InitClient p(m_szLocalName, 1, m_nFileSize);
   m_xSocketFile.SendPacket(*p.getBuffer());

   lblStatus->setText("Connected, waiting for response...");
   m_nState = STATE_RECVxSERVERxINIT;
   connect(snFile, SIGNAL(activated(int)), this, SLOT(StateClient()));
   
   return true;
}

//-----StateClient--------------------------------------------------------------
void CFileDlg::StateClient()
{
  if (!m_xSocketFile.RecvPacket())
  {
    if (m_xSocketFile.Error() == 0)
      gLog.Error("%sFile transfer - remote end disconnected.\n", L_ERRORxSTR);
    else 
      gLog.Error("%sFile transfer receive error - lost remote end:\n%s%s\n", L_ERRORxSTR, 
                L_BLANKxSTR, m_xSocketFile.ErrorStr(buf, 128));
    fileCancel();
    return;
  }
  if (!m_xSocketFile.RecvBufferFull()) return;

  switch(m_nState)
  {
  case STATE_RECVxSERVERxINIT:
  {
    // Process init packet
    char cJunk, t[64];
    unsigned long nJunkLong;
    unsigned short nRemoteNameLen;
    m_xSocketFile.RecvBuffer() >> cJunk >> nJunkLong >> nRemoteNameLen;
    // Cheap hack to avoid icq99a screwing us up, the next packet should good
    if (cJunk == 0x05) 
    {
      /*CBuffer b(5);
      b.add((char)ICQ_VERSION_TCP);
      b.add((unsigned long)0x64);
      m_xSocketFile.SendPacket(b);*/
      break;
    }
    if (cJunk != 0x01)
    {
      char *pbuf;
      gLog.Error("%sError receiving data: invalid server init packet:\n%s%s\n", 
                 L_ERRORxSTR, L_BLANKxSTR, m_xSocketFile.RecvBuffer().print(pbuf));
      delete [] pbuf;
      fileCancel();
      return;
    }
    m_szRemoteName = new char[nRemoteNameLen];
    for (int i = 0; i < nRemoteNameLen; i++) 
       m_xSocketFile.RecvBuffer() >> m_szRemoteName[i];
    sprintf(t, "ICQ file transfer %s %s", IsServer() ? "from" : "to", m_szRemoteName);
    setCaption(t);    
    
    // Send batch file info packet
    vector <const char *> vszFilenames(1);
    vszFilenames[0] = nfoTransferFileName->text();
    CPFile_Batch p(vszFilenames);
    if (!p.IsValid())
    {
      gLog.Error("%sFile read error '%s':\n%s%s\n.", L_ERRORxSTR, vszFilenames[0],
                 L_BLANKxSTR, p.ErrorStr());
      fileCancel();
      return;
    }
    m_xSocketFile.SendPacket(*p.getBuffer());
    lblStatus->setText("Sent batch info, waiting for ack...");
 
    // Set up local batch info
    struct SFileInfo s;
    strcpy(s.szName, vszFilenames[0]);
    s.nSize = p.GetFileSize(0);
    m_vsBatch.push_back(s);
    m_nCurrentFile = 0;
 
    m_nState = STATE_RECVxSTART;
    break;
  }

  case STATE_RECVxSTART:
  {
    // Process batch ack packet, it contains nothing useful so just start the transfer
    lblStatus->setText("Starting transfer...");

    // contains the seek value
    char cJunk;
    m_xSocketFile.RecvBuffer() >> cJunk >> m_nFilePos;
    if (cJunk != 0x03)
    {
      char *pbuf;
      gLog.Error("%sError receiving data: invalid start packet:\n%s%s\n", 
                 L_ERRORxSTR, L_BLANKxSTR, m_xSocketFile.RecvBuffer().print(pbuf));
      delete [] pbuf;
      fileCancel();
      return;
    }

    m_nFileDesc = open(m_vsBatch[m_nCurrentFile].szName, O_RDONLY);
    if (m_nFileDesc < 0)
    {
      gLog.Error("%sFile read error '%s':\n%s%s\n.", L_ERRORxSTR, 
                 m_vsBatch[m_nCurrentFile].szName, L_BLANKxSTR, strerror(errno));
      fileCancel();
      return;
    }

    if (lseek(m_nFileDesc, m_nFilePos, SEEK_SET) < 0)
    {
      gLog.Error("%sFile seek error '%s':\n%s%s\n.", L_ERRORxSTR, 
                 m_vsBatch[m_nCurrentFile].szName, L_BLANKxSTR, strerror(errno));
      fileCancel();
      return;
    }

    m_snSend = new QSocketNotifier(m_xSocketFile.Descriptor(), QSocketNotifier::Write);
    connect(m_snSend, SIGNAL(activated(int)), this, SLOT(fileSendFile()));

    m_nBytesTransfered = 0;
    barTransfer->setTotalSteps(m_vsBatch[m_nCurrentFile].nSize);
    barTransfer->setProgress(0);

    m_nBatchStartTime = time(NULL);
    m_nBatchBytesTransfered = m_nBatchPos = 0;
    barBatchTransfer->setTotalSteps(m_nBatchSize);
    barBatchTransfer->setProgress(0);
    
    // Update the status every 2 seconds
    m_tUpdate.start(2000);    
    
    m_nState = STATE_SENDxFILE;
    break;
  }

  case STATE_SENDxFILE:
    // I don't know what this would be...
    break;

  case STATE_DONExSENDxFILE:
    break;
  
  } // switch
  
  m_xSocketFile.ClearRecvBuffer();
}


//-----fileSendFile-------------------------------------------------------------
void CFileDlg::fileSendFile()
{
  static char pSendBuf[2048];
  
  if (m_nBytesTransfered == 0) 
  {
    m_nStartTime = time(NULL);
    m_nBatchPos += m_nFilePos;
    lblStatus->setText("Sending file...");
  }

  int nBytesToSend = m_vsBatch[m_nCurrentFile].nSize - m_nFilePos;
  if (nBytesToSend > 2048) nBytesToSend = 2048;
  if (read(m_nFileDesc, pSendBuf, nBytesToSend) != nBytesToSend)
  {
    gLog.Error("%sError reading from %s:\n%s%s.\n", L_ERRORxSTR,
               m_vsBatch[m_nCurrentFile].szName, L_BLANKxSTR, strerror(errno));
    fileCancel();
    return;
  }
  CBuffer xSendBuf(nBytesToSend + 1);
  xSendBuf.add((char)0x06);
  xSendBuf.add(pSendBuf, nBytesToSend);
  if (!m_xSocketFile.SendPacket(xSendBuf))
  {
    gLog.Error("%sFile send error:\n%s%s\n", L_ERRORxSTR, L_BLANKxSTR, 
               m_xSocketFile.ErrorStr(buf, 128));
    fileCancel();
    return;
  }

  m_nFilePos += nBytesToSend;
  m_nBytesTransfered += nBytesToSend;
  barTransfer->setProgress(m_nFilePos);
  
  m_nBatchPos += nBytesToSend;
  m_nBatchBytesTransfered += nBytesToSend;
  barBatchTransfer->setProgress(m_nBatchPos);
  
  int nBytesLeft = m_vsBatch[m_nCurrentFile].nSize - m_nFilePos;
  if (nBytesLeft > 0) 
  {
    // More bytes to send so go away until the socket is free again
    return;
  }
  
  // Only get here if we are done
  delete m_snSend;
  m_snSend = NULL;
  ::close(m_nFileDesc);
  m_nFileDesc = 0;
  m_tUpdate.stop();
  btnCancel->setText("Ok");
  lblStatus->setText("File sent.");
  
  if (nBytesLeft == 0) 
  {
    // File transfer done perfectly
    gLog.AddLogTypeToService(S_MSGBOX, L_INFO);
    gLog.Info("%sFile transfer of '%s' to %s completed successfully.\n", 
              L_TCPxSTR, m_vsBatch[m_nCurrentFile].szName, m_szRemoteName);
    gLog.RemoveLogTypeFromService(S_MSGBOX, L_INFO);
  }
  else // nBytesLeft < 0
  {
    // Sent too many bytes for the given size of the current file, can't really happen
    gLog.Error("%sFile transfer of '%s' to %s received %d too many bytes.\n%sClosing file, recommend check for errors.\n", 
               L_TCPxSTR, m_vsBatch[m_nCurrentFile].szName, m_szRemoteName, -nBytesLeft, 
               L_BLANKxSTR);
  }
  
  m_nCurrentFile++;
  m_nState = STATE_DONExSENDxFILE;
}


//=====Other Stuff==============================================================

//-----hide---------------------------------------------------------------------
void CFileDlg::hide()
{
   QWidget::hide();
   delete this;
}


//-----resizeEvent----------------------------------------------------
void CFileDlg::resizeEvent (QResizeEvent *)
{
   // resize / reposition all the widgets
   //barTransfer->setGeometry(10, height() - 90, (width() >> 1) - 15, 20);
   //barBatchTransfer->setGeometry((width() >> 1) + 5, height() - 90, (width() >> 1) - 15, 20);
   lblStatus->setGeometry(0, height() - 20, width(), 20);
   btnCancel->setGeometry((width() >> 1) - 100, height() - 60, 200, 30);
}
