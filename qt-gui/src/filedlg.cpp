#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef USE_KDE
#include <kfiledialog.h>
#else
#include <qfiledialog.h>
#endif
#include <qdir.h>
#include <qgroupbox.h>

#include "filedlg.h"
#include "icqpacket.h"
#include "log.h"
#include "translate.h"
#include "ewidgets.h"

extern int errno;

// State table defines
#define STATE_RECVxHANDSHAKE 1
#define STATE_RECVxCLIENTxINIT 2
#define STATE_RECVxFILExINFO 3
#define STATE_RECVxFILE 4
#define STATE_RECVxSERVERxINIT 5
#define STATE_RECVxSTART 6
#define STATE_SENDxFILE 7


//-----Constructor--------------------------------------------------------------
CFileDlg::CFileDlg(unsigned long _nUin,
                   const char *_szTransferFileName, unsigned long _nFileSize,
                   bool _bServer, unsigned short _nPort,
                   QWidget *parent, char *name)
   : QWidget(parent, name)
{
   // If we are the server, then we are receiving a file
   char t[64];
   m_nUin = _nUin;
   m_nPort = _nPort;
   m_bServer = _bServer;
   m_nFileSize = _nFileSize;
   m_nCurrentFile = 0;
   m_nFileDesc = 0;
   m_nBatchSize = m_nFileSize;
   m_nTotalFiles = 1;
   ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
   m_szLocalName = strdup(o->GetAlias());
   gUserManager.DropOwner();
   m_szRemoteName = NULL;
   m_snSend = snFile = snFileServer = NULL;

   setCaption(tr("ICQ file transfer"));
   setGeometry(100, 100, 500, 325);

   lblTransferFileName = new QLabel(tr("Current file:"), this);
   lblTransferFileName->setGeometry(10, 15, 100, 20);
   nfoTransferFileName = new CInfoField(this, true);
   nfoTransferFileName->setGeometry(120, 15, 290, 20);
   nfoTransferFileName->setText(_szTransferFileName);
   nfoTotalFiles = new CInfoField(this, true);
   nfoTotalFiles->setGeometry(410, 15, 60, 20);

   lblLocalFileName = new QLabel(tr("Local file name:"), this);
   lblLocalFileName->setGeometry(10, 40, 100, 20);
   nfoLocalFileName = new CInfoField(this, true);
   nfoLocalFileName->setGeometry(120, 40, 355, 20);
   nfoLocalFileName->setText(IsServer() ? tr("Unset") : tr("N/A"));
   nfoLocalFileName->setEnabled(IsServer());

   // Information stuff about the current file
   QGroupBox *boxCurrent = new QGroupBox(tr("Current File"), this);
   boxCurrent->setGeometry(10, 80, 220, 170);
   lblFileSize = new QLabel(tr("Size:"), boxCurrent);
   lblFileSize->setGeometry(10, 15, 35, 20);
   nfoFileSize = new CInfoField(boxCurrent, true);
   nfoFileSize->setGeometry(50, 15, 150, 20);
   sprintf(t, "%ld bytes", m_nFileSize);
   nfoFileSize->setText(t);
#if 0
   nfoTrans = new CInfoField(10, 40, 35, 150, tr("Trans:"), true, boxCurrent);
   nfoTime = new CInfoField(10, 65, 35, 150, tr("Time:"), true, boxCurrent);
   nfoBPS = new CInfoField(10, 90, 35, 150, tr("BPS:"), true, boxCurrent);
   nfoETA = new CInfoField(10, 115, 35, 150, tr("ETA:"), true, boxCurrent);
   barTransfer = new QProgressBar(boxCurrent);
   barTransfer->setGeometry(10, 140, 200, 20);

   QGroupBox *boxBatch = new QGroupBox(tr("Batch"), this);
   boxBatch->setGeometry(250, 80, 220, 170);
   nfoBatchSize = new CInfoField(10, 15, 35, 150, tr("Size:"), true, boxBatch);
   nfoBatchTrans = new CInfoField(10, 40, 35, 150, tr("Trans:"), true, boxBatch);
   nfoBatchTime = new CInfoField(10, 65, 35, 150, tr("Time:"), true, boxBatch);
   nfoBatchBPS = new CInfoField(10, 90, 35, 150, tr("BPS:"), true, boxBatch);
   nfoBatchETA = new CInfoField(10, 115, 35, 150, tr("ETA:"), true, boxBatch);
   barBatchTransfer = new QProgressBar(boxBatch);
   barBatchTransfer->setGeometry(10, 140, 200, 20);
#endif
   lblStatus = new QLabel(this);
   lblStatus->setFrameStyle(QFrame::Box | QFrame::Raised);

   btnCancel = new QPushButton(tr("&Cancel Transfer"), this);
   connect(btnCancel, SIGNAL(clicked()), this, SLOT(hide()));
   connect(&m_tUpdate, SIGNAL(timeout()), this, SLOT(fileUpdate()));

   // now either connect to the remote host or start up a server
   if (IsServer())
   {
      sprintf(t, "%d / ?", m_nCurrentFile + 1);
      nfoTotalFiles->setText(t);
      nfoBatchSize->setText(tr("Unknown"));
      show();
      if (!startAsServer()) setPort(0);
   }
   else
   {
      sprintf(t, "%d / %ld", m_nCurrentFile + 1, m_nTotalFiles);
      nfoTotalFiles->setText(t);
      sprintf(t, tr("%ld bytes"), m_nBatchSize);
      nfoBatchSize->setText(t);
      show();
      if (!startAsClient()) setPort(0);
   }
}


//-----Destructor---------------------------------------------------------------
CFileDlg::~CFileDlg(void)
{
  if (m_szLocalName != NULL) free(m_szLocalName);
  if (m_szRemoteName != NULL) delete[] m_szRemoteName;
  if (m_nFileDesc > 0) ::close(m_nFileDesc);
  if (m_snSend != NULL) delete m_snSend;
  if (snFile != NULL) delete snFile;
  if (snFileServer != NULL) delete snFileServer;
}


//-----GetLocalFileName---------------------------------------------------------
// Sets the local filename and opens the file
// returns false if the user hits cancel
bool CFileDlg::GetLocalFileName(void)
{
  QString f;
  bool bValid = false;
  // Get the local filename and open it, loop until valid or cancel
  while(!bValid)
  {
#ifdef USE_KDE
    f = KFileDialog::getSaveFileName(
          QString(QDir::homeDirPath() + "/" + m_sFileInfo.szName),
          QString::null, this);
#else
    f = QFileDialog::getSaveFileName(
          QString(QDir::homeDirPath() + "/" + m_sFileInfo.szName),
          QString::null, this);
#endif
    if (f.isNull()) return (false);
    struct stat buf;
    int nFlags = O_WRONLY;
    m_nFilePos = 0;
    if (stat(f, &buf) == 0)  // file already exists
    {
      if ((unsigned long)buf.st_size >= m_sFileInfo.nSize)
      {
        if(QueryUser(this, tr("File already exists and is at least as big as the incoming file."), tr("Overwrite"), tr("Cancel")))
          nFlags |= O_TRUNC;
        else
          return (false);
      }
      else
      {
        if (QueryUser(this, tr("File already exists and appears incomplete."), tr("Overwrite"), tr("Resume")))
          nFlags |= O_TRUNC;
        else
        {
          nFlags |= O_APPEND;
          m_nFilePos = buf.st_size;
        }
      }
    }
    else
    {
      nFlags |= O_CREAT;
    }

    m_nFileDesc = open (f, nFlags, 00664);
    if (m_nFileDesc < 0)
    {
      if (!QueryUser(this, tr("Open error - unable to open file for writing."), tr("Retry"), tr("Cancel")))
        return (false);
    }
    else
      bValid = true;
  }

  nfoLocalFileName->setText(f);
  m_nBytesTransfered = 0;
  barTransfer->setTotalSteps(m_sFileInfo.nSize);
  barTransfer->setProgress(0);
  return(true);
}


//-----fileCancel---------------------------------------------------------------
void CFileDlg::fileCancel()
{
  // close the local file and other stuff
  if (m_snSend != NULL) m_snSend->setEnabled(false);
  if (snFile != NULL) snFile->setEnabled(false);
  if (snFileServer != NULL) snFileServer->setEnabled(false);
  m_xSocketFileServer.CloseConnection();
  m_xSocketFile.CloseConnection();
  lblStatus->setText(tr("File transfer cancelled."));
  btnCancel->setText(tr("Done"));
}


//-----fileUpdate---------------------------------------------------------------
void CFileDlg::fileUpdate()
{
  //update time, BPS and eta
  static char sz[16];

  // Current File

  // Transfered
  if (m_nFilePos > 1024)
    sprintf(sz, tr("%0.2f kb"), m_nFilePos / 1024.0);
  else
    sprintf(sz, tr("%ld b"), m_nFilePos);
  nfoTrans->setText(sz);

  // Time
  time_t nTime = time(NULL) - m_nStartTime;
  sprintf(sz, "%02ld:%02ld:%02ld", nTime / 3600, (nTime % 3600) / 60, (nTime % 60));
  nfoTime->setText(sz);
  if (nTime == 0 || m_nBytesTransfered == 0)
  {
    nfoBPS->setText("---");
    nfoETA->setText("---");
    return;
  }

  // BPS
  float fBPS = m_nBytesTransfered / nTime;
  if (fBPS > 1024)
    sprintf(sz, tr("%.2f k"), fBPS / 1024);
  else
    sprintf(sz, "%.2f", fBPS);
  nfoBPS->setText(sz);

  // ETA
  int nBytesLeft = m_sFileInfo.nSize - m_nFilePos;
  time_t nETA = (time_t)(nBytesLeft / fBPS);
  sprintf(sz, "%02ld:%02ld:%02ld", nETA / 3600, (nETA % 3600) / 60, (nETA % 60));
  nfoETA->setText(sz);


  // Batch

  // Transfered
  if (m_nBatchPos > 1024)
    sprintf(sz, tr("%0.2f kb"), m_nBatchPos / 1024.0);
  else
    sprintf(sz, tr("%ld b"), m_nBatchPos);
  nfoBatchTrans->setText(sz);

  // Time
  time_t nBatchTime = time(NULL) - m_nBatchStartTime;
  sprintf(sz, "%02ld:%02ld:%02ld", nBatchTime / 3600, (nBatchTime % 3600) / 60,
         (nBatchTime % 60));
  nfoBatchTime->setText(sz);
  if (nBatchTime == 0 || m_nBatchBytesTransfered == 0)
  {
    nfoBatchBPS->setText("---");
    nfoBatchETA->setText("---");
    return;
  }

  // BPS
  float fBatchBPS = m_nBatchBytesTransfered / nBatchTime;
  if (fBatchBPS > 1024)
    sprintf(sz, tr("%.2f k"), fBatchBPS / 1024);
  else
    sprintf(sz, "%.2f", fBatchBPS);
  nfoBatchBPS->setText(sz);

  // ETA
  int nBatchBytesLeft = m_nBatchSize - m_nBatchPos;
  time_t nBatchETA = (time_t)(nBatchBytesLeft / fBatchBPS);
  sprintf(sz, "%02ld:%02ld:%02ld", nBatchETA / 3600, (nBatchETA % 3600) / 60,
          (nBatchETA % 60));
  nfoBatchETA->setText(sz);

}



//=====Server Side==============================================================

//-----startAsServer------------------------------------------------------------
bool CFileDlg::startAsServer(void)
{
   gLog.Info("%sStarting file server on port %d.\n", L_TCPxSTR, getPort());
   if (!(m_xSocketFileServer.StartServer(getPort())))
   {
     gLog.Error("%sFile transfer - error creating local socket:\n%s%s\n", L_ERRORxSTR,
                L_BLANKxSTR, m_xSocketFileServer.ErrorStr(buf, 128));
     return false;
   }

   setPort(m_xSocketFileServer.LocalPort());
   snFileServer = new QSocketNotifier(m_xSocketFileServer.Descriptor(), QSocketNotifier::Read);
   connect(snFileServer, SIGNAL(activated(int)), this, SLOT(fileRecvConnection()));

   lblStatus->setText(tr("Waiting for connection..."));

   return true;
}


//-----fileRecvConnection-------------------------------------------------------
void CFileDlg::fileRecvConnection()
{
   m_xSocketFileServer.RecvConnection(m_xSocketFile);
   disconnect(snFileServer, SIGNAL(activated(int)), this, SLOT(fileRecvConnection()));
   m_nState = STATE_RECVxHANDSHAKE;
   snFile = new QSocketNotifier(m_xSocketFile.Descriptor(), QSocketNotifier::Read);
   connect(snFile, SIGNAL(activated(int)), this, SLOT(StateServer()));
}


//----StateServer----------------------------------------------------------------
void CFileDlg::StateServer()
{
  // get the handshake packet
  if (!m_xSocketFile.RecvPacket())
  {
    fileCancel();
    if (m_xSocketFile.Error() == 0)
      InformUser(this, tr("Remote end disconnected."));
    else
      gLog.Error("%sFile transfer receive error - lost remote end:\n%s%s\n", L_ERRORxSTR,
                 L_BLANKxSTR, m_xSocketFile.ErrorStr(buf, 128));
    return;
  }
  if (!m_xSocketFile.RecvBufferFull()) return;

  switch (m_nState)
  {
  case STATE_RECVxHANDSHAKE:
  {
    char cHandshake;
    m_xSocketFile.RecvBuffer() >> cHandshake;
    if ((unsigned short)cHandshake != ICQ_CMDxTCP_HANDSHAKE)
    {
      gLog.Error("%sReceive error - bad handshake (%04X).\n", L_ERRORxSTR, cHandshake);
      fileCancel();
      return;
    }
    m_nState = STATE_RECVxCLIENTxINIT;
    break;
  }

  case STATE_RECVxCLIENTxINIT:
  {
    // Process init packet
    char cJunk, t[64];
    unsigned long nJunkLong;
    unsigned short nRemoteNameLen;
    m_xSocketFile.RecvBuffer() >> cJunk;
    if (cJunk != 0x00)
    {
      char *pbuf;
      gLog.Error("%sError receiving data: invalid client init packet:\n%s%s\n",
                 L_ERRORxSTR, L_BLANKxSTR, m_xSocketFile.RecvBuffer().print(pbuf));
      delete [] pbuf;
      fileCancel();
      return;
    }
    m_xSocketFile.RecvBuffer() >> nJunkLong
                               >> m_nTotalFiles
                               >> m_nBatchSize
                               >> nJunkLong
                               >> nRemoteNameLen;
    m_szRemoteName = new char[nRemoteNameLen];
    for (int i = 0; i < nRemoteNameLen; i++)
       m_xSocketFile.RecvBuffer() >> m_szRemoteName[i];
    sprintf(t, "%d / %ld", m_nCurrentFile + 1, m_nTotalFiles);
    nfoTotalFiles->setText(t);
    sprintf(t, "%ld bytes", m_nBatchSize);
    nfoBatchSize->setText(t);
    m_nBatchStartTime = time(NULL);
    m_nBatchBytesTransfered = m_nBatchPos = 0;
    barBatchTransfer->setTotalSteps(m_nBatchSize);
    barBatchTransfer->setProgress(0);
    setCaption(tr("ICQ file transfer %1 %2").arg(IsServer() ? tr("from") : tr("to")).arg(m_szRemoteName));

    // Send response
    CPFile_InitServer p(m_szLocalName);
    m_xSocketFile.SendPacket(p.getBuffer());

    lblStatus->setText(tr("Received init, waiting for batch info..."));
    m_nState = STATE_RECVxFILExINFO;
    break;
  }

  case STATE_RECVxFILExINFO:
  {
    // Process file packet
    unsigned short nLen, nTest;
    char t[64], cJunk;
    m_xSocketFile.RecvBuffer() >> nTest;
    if (nTest != 0x0002)
    {
      char *pbuf;
      gLog.Error("%sError receiving data: invalid file info packet:\n%s%s\n",
                 L_ERRORxSTR, L_BLANKxSTR, m_xSocketFile.RecvBuffer().print(pbuf));
      delete [] pbuf;
      fileCancel();
      return;
    }
    m_xSocketFile.RecvBuffer() >> nLen;
    for (int j = 0; j < nLen; j++)
      m_xSocketFile.RecvBuffer() >> m_sFileInfo.szName[j];
    m_xSocketFile.RecvBuffer() >> nLen;
    m_xSocketFile.RecvBuffer() >> cJunk;
    m_xSocketFile.RecvBuffer() >> m_sFileInfo.nSize;

    m_nCurrentFile++;
    nfoTransferFileName->setText(m_sFileInfo.szName);
    sprintf(t, tr("%ld bytes"), m_sFileInfo.nSize);
    nfoFileSize->setText(t);
    barTransfer->setTotalSteps(m_sFileInfo.nSize);

    // Get the local filename and set the file offset (for resume)
    if (!GetLocalFileName())
    {
      fileCancel();
      return;
    }

    // Send response
    CPFile_Start p(m_nFilePos);
    m_xSocketFile.SendPacket(p.getBuffer());
    lblStatus->setText("Starting transfer...");

    // Update the status every 2 seconds
    m_tUpdate.start(2000);

    disconnect(snFile, SIGNAL(activated(int)), this, SLOT(StateServer()));
    connect(snFile, SIGNAL(activated(int)), this, SLOT(fileRecvFile()));
    m_nState = STATE_RECVxFILE;
    break;
  }

  }  // switch

  m_xSocketFile.ClearRecvBuffer();
}


//-----fileRecvFile-------------------------------------------------------------
void CFileDlg::fileRecvFile()
{
  if (!m_xSocketFile.RecvPacket())
  {
    fileCancel();
    if (m_xSocketFile.Error() == 0)
      gLog.Error("%sFile receive error, remote end disconnected.\n", L_ERRORxSTR);
    else
      gLog.Error("%sFile receive error:\n%s%s\n", L_ERRORxSTR, L_BLANKxSTR,
                 m_xSocketFile.ErrorStr(buf, 128));
    return;
  }
  if (!m_xSocketFile.RecvBufferFull()) return;

  // if this is the first call to this function...
  if (m_nBytesTransfered == 0)
  {
    m_nStartTime = time(NULL);
    m_nBatchPos += m_nFilePos;
    lblStatus->setText(tr("Receiving file..."));
  }

  // Write the new data to the file and empty the buffer
  CBuffer &b = m_xSocketFile.RecvBuffer();
  char cTest;
  b >> cTest;
  if (cTest != 0x06)
  {
    gLog.Error("%sFile receive error, invalid data (%c).  Ignoring packet.\n", cTest);
               //, L_BLANKxSTR, m_xSocketFile.RecvBuffer().print());
    m_xSocketFile.ClearRecvBuffer();
    return;
  }
  errno = 0;
  size_t nBytesWritten = write(m_nFileDesc, b.getDataPosRead(), b.getDataSize() - 1);
  if (nBytesWritten != b.getDataSize() - 1)
  {
    gLog.Error("%sFile write error:\n%s%s.\n", L_ERRORxSTR, L_BLANKxSTR,
               errno == 0 ? "Disk full (?)" : strerror(errno));
    fileCancel();
    return;
  }

  m_nFilePos += nBytesWritten;
  m_nBytesTransfered += nBytesWritten;
  barTransfer->setProgress(m_nFilePos);

  m_nBatchPos += nBytesWritten;
  m_nBatchBytesTransfered += nBytesWritten;
  barBatchTransfer->setProgress(m_nBatchPos);

  m_xSocketFile.ClearRecvBuffer();

  int nBytesLeft = m_sFileInfo.nSize - m_nFilePos;
  if (nBytesLeft > 0)
  {
    // More bytes to come so go away and wait for them
    return;
  }
  else if (nBytesLeft == 0)
  {
    // File transfer done perfectly
    ::close(m_nFileDesc);
    m_nFileDesc = 0;
    /*char msg[1024];
    sprintf(msg, tr("%sFile transfer of\n'%s'\nfrom %s completed successfully.\n"),
            L_TCPxSTR, m_sFileInfo.szName, m_szRemoteName);
    InformUser(this, msg);*/
    QString msg = QString(tr("File '%1' from %2 received successfully."))
                          .arg(m_sFileInfo.szName).arg(m_szRemoteName);
    lblStatus->setText(msg);

  }
  else // nBytesLeft < 0
  {
    // Received too many bytes for the given size of the current file
    ::close(m_nFileDesc);
    m_nFileDesc = 0;
    gLog.Error("%sFile transfer of\n'%s'\nfrom %s received %d too many bytes.\n%sClosing file, recommend check for errors.\n",
               L_TCPxSTR, m_sFileInfo.szName, m_szRemoteName, -nBytesLeft, L_BLANKxSTR);
  }
  // Only get here if the current file is done
  m_nCurrentFile++;
  m_tUpdate.stop();
  btnCancel->setText(tr("Ok"));
  lblStatus->setText(tr("File received."));
  disconnect(snFile, SIGNAL(activated(int)), this, SLOT(fileRecvFile()));

  // Now wait for a disconnect or another file
  m_nState = STATE_RECVxFILExINFO;
  connect(snFile, SIGNAL(activated(int)), this, SLOT(StateServer()));
}



//=====Client Side==============================================================

//-----startAsClient------------------------------------------------------------
bool CFileDlg::startAsClient(void)
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  unsigned long nIp = u->Ip();
  gUserManager.DropUser(u);
  gLog.Info("%sFile transfer - connecting to %ld:%d.\n", L_TCPxSTR,
             inet_ntoa_r(*(struct in_addr *)&nIp, buf), getPort());
   lblStatus->setText(tr("Connecting to remote..."));
   m_xSocketFile.SetRemoteAddr(nIp, getPort());
   if (!m_xSocketFile.OpenConnection())
   {
     gLog.Error("%sUnable to connect to remote file server:\n%s%s.\n", L_ERRORxSTR,
                L_BLANKxSTR, m_xSocketFile.ErrorStr(buf, 128));
     return false;
   }

   lblStatus->setText(tr("Connected, shaking hands..."));

   // Send handshake packet:
   CPacketTcp_Handshake p_handshake(getLocalPort());
   m_xSocketFile.SendPacket(p_handshake.getBuffer());

   // Send init packet:
   CPFile_InitClient p(m_szLocalName, 1, m_nFileSize);
   m_xSocketFile.SendPacket(p.getBuffer());

   lblStatus->setText(tr("Connected, waiting for response..."));
   m_nState = STATE_RECVxSERVERxINIT;
   snFile = new QSocketNotifier(m_xSocketFile.Descriptor(), QSocketNotifier::Read);
   connect(snFile, SIGNAL(activated(int)), this, SLOT(StateClient()));

   return true;
}

//-----StateClient--------------------------------------------------------------
void CFileDlg::StateClient()
{
  if (!m_xSocketFile.RecvPacket())
  {
    fileCancel();
    if (m_xSocketFile.Error() == 0)
      InformUser(this, tr("Remote end disconnected."));
    else
      gLog.Error("%sFile transfer receive error - lost remote end:\n%s%s\n", L_ERRORxSTR,
                L_BLANKxSTR, m_xSocketFile.ErrorStr(buf, 128));
    return;
  }
  if (!m_xSocketFile.RecvBufferFull()) return;

  switch(m_nState)
  {
  case STATE_RECVxSERVERxINIT:
  {
    // Process init packet
    char cJunk;
    unsigned long nJunkLong;
    unsigned short nRemoteNameLen;
    m_xSocketFile.RecvBuffer() >> cJunk >> nJunkLong >> nRemoteNameLen;
    // Cheap hack to avoid icq99a screwing us up, the next packet should good
    if (cJunk == 0x05)
    {
      // set our speed, for now fuckem and go as fast as possible
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
    setCaption(tr("ICQ file transfer %1 %2").arg(IsServer() ? tr("from") : tr("to")).arg(m_szRemoteName));

    // Send file info packet
    CPFile_Info p(nfoTransferFileName->text());
    if (!p.IsValid())
    {
      gLog.Error("%sFile read error '%s':\n%s%s\n.", L_ERRORxSTR, (const char *)nfoTransferFileName->text(),
                 L_BLANKxSTR, p.ErrorStr());
      fileCancel();
      return;
    }
    m_xSocketFile.SendPacket(p.getBuffer());
    lblStatus->setText(tr("Sent batch info, waiting for ack..."));

    // Set up local batch info
    strcpy(m_sFileInfo.szName, nfoTransferFileName->text());
    m_sFileInfo.nSize = p.GetFileSize();
    m_nCurrentFile++;

    m_nBatchStartTime = time(NULL);
    m_nBatchBytesTransfered = m_nBatchPos = 0;
    barBatchTransfer->setTotalSteps(m_nBatchSize);
    barBatchTransfer->setProgress(0);

    m_nState = STATE_RECVxSTART;
    break;
  }

  case STATE_RECVxSTART:
  {
    // Process batch ack packet, it contains nothing useful so just start the transfer
    lblStatus->setText(tr("Starting transfer..."));

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

    m_nFileDesc = open(m_sFileInfo.szName, O_RDONLY);
    if (m_nFileDesc < 0)
    {
      gLog.Error("%sFile read error '%s':\n%s%s\n.", L_ERRORxSTR,
                 m_sFileInfo.szName, L_BLANKxSTR, strerror(errno));
      fileCancel();
      return;
    }

    if (lseek(m_nFileDesc, m_nFilePos, SEEK_SET) < 0)
    {
      gLog.Error("%sFile seek error '%s':\n%s%s\n.", L_ERRORxSTR,
                 m_sFileInfo.szName, L_BLANKxSTR, strerror(errno));
      fileCancel();
      return;
    }

    m_snSend = new QSocketNotifier(m_xSocketFile.Descriptor(), QSocketNotifier::Write);
    connect(m_snSend, SIGNAL(activated(int)), this, SLOT(fileSendFile()));

    m_nBytesTransfered = 0;
    barTransfer->setTotalSteps(m_sFileInfo.nSize);
    barTransfer->setProgress(0);

    // Update the status every 2 seconds
    m_tUpdate.start(2000);

    m_nState = STATE_SENDxFILE;
    break;
  }

  case STATE_SENDxFILE:
    // I don't know what this would be...
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
    lblStatus->setText(tr("Sending file..."));
  }

  int nBytesToSend = m_sFileInfo.nSize - m_nFilePos;
  if (nBytesToSend > 2048) nBytesToSend = 2048;
  if (read(m_nFileDesc, pSendBuf, nBytesToSend) != nBytesToSend)
  {
    gLog.Error("%sError reading from %s:\n%s%s.\n", L_ERRORxSTR,
               m_sFileInfo.szName, L_BLANKxSTR, strerror(errno));
    fileCancel();
    return;
  }
  CBuffer xSendBuf(nBytesToSend + 1);
  xSendBuf.add((char)0x06);
  xSendBuf.add(pSendBuf, nBytesToSend);
  if (!m_xSocketFile.SendPacket(&xSendBuf))
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

  int nBytesLeft = m_sFileInfo.nSize - m_nFilePos;
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
  btnCancel->setText(tr("Ok"));
  lblStatus->setText(tr("File sent."));

  if (nBytesLeft == 0)
  {
    // File transfer done perfectly
    /*char msg[1024];
    sprintf(msg, tr("%sFile transfer of\n'%s'\nto %s completed successfully.\n"),
            L_TCPxSTR, m_sFileInfo.szName, m_szRemoteName);
    InformUser(this, msg);*/
    QString msg = QString(tr("Sending of file '%1' to %2 completed successfully."))
      .arg(m_sFileInfo.szName).arg(m_szRemoteName);
    lblStatus->setText(msg);
  }
  else // nBytesLeft < 0
  {
    // Sent too many bytes for the given size of the current file, can't really happen
    gLog.Error("%sFile transfer of\n'%s'\n to %s received %d too many bytes.\n%sClosing file, recommend check for errors.\n", 
               L_TCPxSTR, m_sFileInfo.szName, m_szRemoteName, -nBytesLeft,
               L_BLANKxSTR);
  }


  m_xSocketFileServer.CloseConnection();
  m_xSocketFile.CloseConnection();
  if (m_snSend != NULL) m_snSend->setEnabled(false);
  lblStatus->setText(tr("File transfer complete."));
  btnCancel->setText(tr("Done"));
}


//=====Other Stuff==============================================================

//-----hide---------------------------------------------------------------------
void CFileDlg::hide()
{
   QWidget::hide();
   fileCancel();
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

#include "moc/moc_filedlg.h"
