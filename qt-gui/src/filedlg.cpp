/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

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
#include <qhbox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qprogressbar.h>
#include <qsocketnotifier.h>

#include "filedlg.h"
#include "licq_packets.h"
#include "licq_log.h"
#include "licq_translate.h"
#include "ewidgets.h"
#include "licq_icqd.h"

extern int errno;

// State table defines
#define STATE_RECVxHANDSHAKE 1
#define STATE_RECVxCLIENTxINIT 2
#define STATE_RECVxFILExINFO 3
#define STATE_RECVxFILE 4
#define STATE_RECVxSERVERxINIT 5
#define STATE_RECVxSTART 6
#define STATE_SENDxFILE 7


//-----Constructor------------------------------------------------------------
CFileDlg::CFileDlg(unsigned long _nUin, const char *_szTransferFileName,
                   unsigned long _nFileSize, CICQDaemon *daemon,
                   QWidget *parent, char *name)
  : QDialog(parent, name)
{
  // If we are the server, then we are receiving a file
  char t[64];
  m_nUin = _nUin;
  m_nPort = 0;
  m_nFileSize = _nFileSize;
  m_nCurrentFile = 0;
  m_nFileDesc = 0;
  m_nBatchSize = m_nFileSize;
  m_nTotalFiles = 1;
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  m_szLocalName = strdup(o->GetAlias());
  gUserManager.DropOwner();
  m_szRemoteName = NULL;
  licqDaemon = daemon;
  m_snSend = snFile = snFileServer = NULL;

  setCaption(tr("ICQ file transfer"));

  unsigned short CR = 0;
  QGridLayout* lay = new QGridLayout(this, 8, 3, 8, 8);
  lay->setColStretch(1, 2);

  lblTransferFileName = new QLabel(tr("Current:"), this);
  lay->addWidget(lblTransferFileName, CR, 0);
  nfoTransferFileName = new CInfoField(this, true);
  nfoTransferFileName->setMinimumWidth(nfoTransferFileName->sizeHint().width()*2);
  lay->addWidget(nfoTransferFileName, CR, 1);
  nfoTransferFileName->setText(_szTransferFileName);
  nfoTotalFiles = new CInfoField(this, true);
  nfoTotalFiles->setMinimumWidth((nfoTotalFiles->sizeHint().width()*3)/2);
  lay->addWidget(nfoTotalFiles, CR, 2);

  lblLocalFileName = new QLabel(tr("File name:"), this);
  lay->addWidget(lblLocalFileName, ++CR, 0);
  nfoLocalFileName = new CInfoField(this, true);
  lay->addMultiCellWidget(nfoLocalFileName, CR, CR, 1, 2);

  lay->addRowSpacing(++CR, 10);

  // Information stuff about the current file
  lblTrans = new QLabel(tr("File:"), this);
  lay->addWidget(lblTrans, ++CR, 0);
  barTransfer = new QProgressBar(this);
  lay->addWidget(barTransfer, CR, 1);
  nfoFileSize = new CInfoField(this, true);
  lay->addWidget(nfoFileSize, CR, 2);
  nfoFileSize->setText(encodeFSize(m_nFileSize));

  // Information about the batch file transfer
  lblBatch = new QLabel(tr("Batch:"), this);
  lay->addWidget(lblBatch, ++CR, 0);
  barBatchTransfer = new QProgressBar(this);
  lay->addWidget(barBatchTransfer, CR, 1);
  nfoBatchSize = new CInfoField(this, true);
  lay->addWidget(nfoBatchSize, CR, 2);
  sprintf(t, "%ld bytes", m_nBatchSize);
  nfoBatchSize->setText(t);

  lblTime = new QLabel(tr("Time:"), this);
  lay->addWidget(lblTime, ++CR, 0);

  QHBox* hbox = new QHBox(this);
  hbox->setSpacing(8);
  lay->addMultiCellWidget(hbox, CR, CR, 0, 1);
  nfoTime = new CInfoField(hbox, true);
  nfoBPS = new CInfoField(hbox, true);
  lblETA = new QLabel(tr("ETA:"), hbox);
  nfoETA = new CInfoField(this, true);
  lay->addWidget(nfoETA, CR++, 2);
  lay->addRowSpacing(++CR, 10);

  lblStatus = new QLabel(this);
  ++CR;
  lay->addMultiCellWidget(lblStatus, CR, CR, 0, 2);
  lblStatus->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

  lay->setRowStretch(++CR, 3);

  btnCancel = new QPushButton(tr("&Cancel Transfer"), this);
  btnCancel->setMinimumWidth(75);
  lay->addMultiCellWidget(btnCancel, CR, CR, 1, 2);
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(hide()));
  connect(&m_tUpdate, SIGNAL(timeout()), this, SLOT(fileUpdate()));
}


//-----Destructor---------------------------------------------------------------
CFileDlg::~CFileDlg()
{
  if (m_szLocalName != NULL) free(m_szLocalName);
  if (m_szRemoteName != NULL) delete[] m_szRemoteName;
  if (m_nFileDesc > 0) ::close(m_nFileDesc);
  delete m_snSend;
  delete snFile;
  delete snFileServer;
}


//-----GetLocalFileName---------------------------------------------------------
// Sets the local filename and opens the file
// returns false if the user hits cancel
bool CFileDlg::GetLocalFileName()
{
  QString f;
  bool bValid = false;
  // Get the local filename and open it, loop until valid or cancel
  while(!bValid)
  {
#ifdef USE_KDE
    KURL u = KFileDialog::getSaveURL(
          QString(QDir::homeDirPath() + "/" + m_sFileInfo.szName),
          QString::null, this);
    f = u.path();
#else
    f = QFileDialog::getSaveFileName(
          QString(QDir::homeDirPath() + "/" + m_sFileInfo.szName),
          QString::null, this);
#endif
    if (f.isNull()) return (false);
    struct stat buf;
    int nFlags = O_WRONLY;
    m_nFilePos = 0;
    if (stat(QFile::encodeName(f), &buf) == 0)  // file already exists
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

    m_nFileDesc = open (QFile::encodeName(f), nFlags, 00664);
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
  nfoFileSize->setText(QString(tr("%1/%2"))
                       .arg(encodeFSize(m_nFilePos))
                       .arg(encodeFSize(m_sFileInfo.nSize)));

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
  nfoBPS->setText(QString("%1/s").arg(encodeFSize(m_nBytesTransfered / nTime)));

  // ETA
  int nBytesLeft = m_sFileInfo.nSize - m_nFilePos;
  time_t nETA = (time_t)(nBytesLeft / (m_nBytesTransfered / nTime));
  sprintf(sz, "%02ld:%02ld:%02ld", nETA / 3600, (nETA % 3600) / 60, (nETA % 60));
  nfoETA->setText(sz);

  // Batch

  // Transfered
  nfoBatchSize->setText(QString(tr("%1/%2"))
                        .arg(encodeFSize(m_nBatchPos))
                        .arg(encodeFSize(m_nBatchSize)));
}



//=====Server Side==============================================================

//-----startAsServer------------------------------------------------------------
bool CFileDlg::StartAsServer()
{
  m_bServer = true;
  if (licqDaemon->StartTCPServer(&m_xSocketFileServer) == -1)
  {
    WarnUser(this, tr("No more ports available, add more\nor close open chat/file sessions."));
    return false;
  }
  m_nPort = m_xSocketFileServer.LocalPort();

  nfoTotalFiles->setText(QString("%1 / ?").arg(m_nCurrentFile + 1));
  nfoBatchSize->setText(tr("Unknown"));
  nfoLocalFileName->setText(tr("Unset"));

  snFileServer = new QSocketNotifier(m_xSocketFileServer.Descriptor(), QSocketNotifier::Read);
  connect(snFileServer, SIGNAL(activated(int)), this, SLOT(fileRecvConnection()));

  lblStatus->setText(tr("Waiting for connection..."));

  show();

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
    unsigned char cHandshake;
    m_xSocketFile.RecvBuffer() >> cHandshake;
    if (cHandshake != ICQ_CMDxTCP_HANDSHAKE)
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
    nfoBatchSize->setText(encodeFSize(m_nBatchSize));
    m_nBatchStartTime = time(NULL);
    m_nBatchBytesTransfered = m_nBatchPos = 0;
    barBatchTransfer->setTotalSteps(m_nBatchSize);
    barBatchTransfer->setProgress(0);
    setCaption(tr("ICQ file transfer %1 %2").arg(m_bServer ? tr("from") : tr("to")).arg(m_szRemoteName));

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
    unsigned short nLen;
    char cJunk;
    m_xSocketFile.RecvBuffer() >> cJunk;
    if (cJunk == 0x05)
    {
      // set our speed, for now fuckem and go as fast as possible
      break;
    }
    if (cJunk != 0x02)
    {
      char *pbuf;
      gLog.Error("%sError receiving data: invalid file info packet:\n%s%s\n",
                 L_ERRORxSTR, L_BLANKxSTR, m_xSocketFile.RecvBuffer().print(pbuf));
      delete [] pbuf;
      fileCancel();
      return;
    }
    m_xSocketFile.RecvBuffer() >> cJunk >> nLen;
    for (int j = 0; j < nLen; j++)
      m_xSocketFile.RecvBuffer() >> m_sFileInfo.szName[j];
    m_xSocketFile.RecvBuffer() >> nLen;
    m_xSocketFile.RecvBuffer() >> cJunk;
    m_xSocketFile.RecvBuffer() >> m_sFileInfo.nSize;

    m_nCurrentFile++;
    nfoTransferFileName->setText(m_sFileInfo.szName);
    nfoFileSize->setText(encodeFSize(m_sFileInfo.nSize));
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

    // Update the status every 0.5 seconds
    m_tUpdate.start(1000);

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
    if (cTest != 0x05)
    {
      gLog.Unknown("%sFile receive error, invalid data (%c).  Ignoring packet.\n",
                   L_UNKNOWNxSTR, cTest);
    }
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

  QString msg;
  if (nBytesLeft == 0)
  {
    // File transfer done perfectly
    ::close(m_nFileDesc);
    m_nFileDesc = 0;
    msg = QString(tr("File '%1' from %2 received successfully."))
            .arg(m_sFileInfo.szName).arg(m_szRemoteName);
  }
  else // nBytesLeft < 0
  {
    // Received too many bytes for the given size of the current file
    ::close(m_nFileDesc);
    m_nFileDesc = 0;
    msg = QString(tr("File transfer of\n'%1'\nfrom %2 received %3 too many bytes.\n%sClosing file, recommend check for errors.")
            .arg(m_sFileInfo.szName).arg(m_szRemoteName).arg(-nBytesLeft));
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

  InformUser(this, msg);
}



//=====Client Side==============================================================

//-----startAsClient------------------------------------------------------------
bool CFileDlg::StartAsClient(unsigned short nPort)
{
  m_bServer = false;
  m_nPort = nPort;

  if (!licqDaemon->OpenConnectionToUser(m_nUin, &m_xSocketFile, m_nPort))
  {
    WarnUser(this, tr("Unable to connect to remote file server.\n"
                      "See the network log for details."));
    return false;
  }

  lblStatus->setText(tr("Connected, shaking hands..."));

  nfoLocalFileName->setText(tr("N/A"));
  nfoLocalFileName->setEnabled(false);
  nfoTotalFiles->setText(QString("%1 / %2").arg(m_nCurrentFile + 1).arg(m_nTotalFiles));
  nfoBatchSize->setText(QString("(%1)").arg(encodeFSize(m_nBatchSize)));
  show();

  // Send handshake packet:
  CPacketTcp_Handshake p_handshake(m_xSocketFile.LocalPort());
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
      WarnUser(this, tr("File transfer receive error - lost remote end:\n%1\n")
               .arg(m_xSocketFile.ErrorStr(buf, 128)));
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
    setCaption(tr("ICQ file transfer %1 %2").arg(m_bServer ? tr("from") : tr("to")).arg(m_szRemoteName));

    // Send file info packet
    CPFile_Info p(nfoTransferFileName->text().local8Bit());
    if (!p.IsValid())
    {
      fileCancel();
      InformUser(this, tr("File read error '%1':\n%2.")
         .arg(nfoTransferFileName->text().local8Bit())
         .arg(p.ErrorStr()));
      return;
    }
    m_xSocketFile.SendPacket(p.getBuffer());
    lblStatus->setText(tr("Sent batch info, waiting for ack..."));

    // Set up local batch info
    strcpy(m_sFileInfo.szName, nfoTransferFileName->text().local8Bit());
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
    if (cJunk == 0x05)
    {
      // set our speed, for now fuckem and go as fast as possible
      break;
    }
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

    // Update the status every 1 seconds
    m_tUpdate.start(1000);

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
  xSendBuf.PackChar(0x06);
  xSendBuf.Pack(pSendBuf, nBytesToSend);
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
  if (nBytesLeft > 0) {
    // More bytes to send so go away until the socket is free again
    return;
  }

  // Only get here if we are done
  delete m_snSend;
  m_snSend = NULL;
  ::close(m_nFileDesc);
  m_nFileDesc = 0;
  m_tUpdate.stop();

  QString msg;
  if (nBytesLeft == 0)
  {
    // File transfer done perfectly
    msg = QString(tr("Sending of file '%1' to %2 completed successfully."))
      .arg(m_sFileInfo.szName).arg(m_szRemoteName);
  }
  else // nBytesLeft < 0
  {
    // Sent too many bytes for the given size of the current file, can't really happen
    msg = QString(tr("File transfer of\n'%1'\n to %2 received %3 too many bytes.\n%sClosing file, recommend check for errors.\n")
            .arg(m_sFileInfo.szName).arg(m_szRemoteName).arg(-nBytesLeft));
  }

  m_xSocketFileServer.CloseConnection();
  m_xSocketFile.CloseConnection();
  if (m_snSend != NULL) m_snSend->setEnabled(false);

  lblStatus->setText(tr("File transfer complete."));
  btnCancel->setText(tr("Done"));

  InformUser(this, msg);
}


//=====Other Stuff==============================================================

//-----hide--------------------------------------------------------------------
void CFileDlg::hide()
{
   QWidget::hide();
   fileCancel();
   delete this;
}

// -----------------------------------------------------------------------------

QString CFileDlg::encodeFSize(unsigned long size)
{
  QString unit;
  if(size >= 1024*1024) {
    size /= (1024*1024)/10;
    unit = tr("MB");
  }
  else if(size >= 1024) {
    size /= 1024/10;
    unit = tr("KB");
  }
  else if(size != 1)
    unit = tr("Bytes");
  else
    unit = tr("Byte");

  return QString("%1.%2 %3").arg(size/10).arg(size%10).arg(unit);
}

#include "filedlg.moc"
