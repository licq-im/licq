/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "filedlg.h"

#include "config.h"

#include <unistd.h>

#ifdef USE_KDE
#include <kfiledialog.h>
#include <krun.h>
#else
#include <QFileDialog>
#endif

#include <QDir>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSocketNotifier>

#include <licq/icq/filetransfer.h>
#include <licq/icq/icq.h>
#include <licq/logging/log.h>
#include <licq/plugin/pluginmanager.h>

#include "core/messagebox.h"

#include "widgets/infofield.h"
#include "widgets/mledit.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::FileDlg */

FileDlg::FileDlg(const Licq::UserId& userId, QWidget* parent)
  : QWidget(parent),
    myUserId(userId)
{
  myId = userId.accountId().c_str();
  m_nPPID = userId.protocolId();

  setObjectName("FileDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Licq - File Transfer (%1)").arg(myId));

  unsigned short CR = 0;
  QGridLayout* lay = new QGridLayout(this);
  lay->setColumnStretch(1, 2);

  lblTransferFileName = new QLabel(tr("Current:"));
  lay->addWidget(lblTransferFileName, CR, 0);
  nfoTransferFileName = new InfoField(true);
  nfoTransferFileName->setMinimumWidth(nfoTransferFileName->sizeHint().width()*2);
  lay->addWidget(nfoTransferFileName, CR, 1);
  nfoTotalFiles = new InfoField(true);
  nfoTotalFiles->setMinimumWidth((nfoTotalFiles->sizeHint().width()*3)/2);
  lay->addWidget(nfoTotalFiles, CR, 2);

  lblLocalFileName = new QLabel(tr("File name:"));
  lay->addWidget(lblLocalFileName, ++CR, 0);
  nfoLocalFileName = new InfoField(true);
  lay->addWidget(nfoLocalFileName, CR, 1, 1, 2);

  lay->addItem(new QSpacerItem(0, 10), ++CR, 0);

  // Information stuff about the current file
  lblTrans = new QLabel(tr("File:"));
  lay->addWidget(lblTrans, ++CR, 0);
  barTransfer = new QProgressBar();
  lay->addWidget(barTransfer, CR, 1);
  nfoFileSize = new InfoField(true);
  lay->addWidget(nfoFileSize, CR, 2);

  // Information about the batch file transfer
  lblBatch = new QLabel(tr("Batch:"));
  lay->addWidget(lblBatch, ++CR, 0);
  barBatchTransfer = new QProgressBar();
  lay->addWidget(barBatchTransfer, CR, 1);
  nfoBatchSize = new InfoField(true);
  lay->addWidget(nfoBatchSize, CR, 2);

  lblTime = new QLabel(tr("Time:"));
  lay->addWidget(lblTime, ++CR, 0);

  QHBoxLayout* hbox = new QHBoxLayout();
  hbox->setSpacing(8);
  lay->addLayout(hbox, CR, 0, 1, 2);
  nfoTime = new InfoField(true);
  hbox->addWidget(nfoTime);
  nfoBPS = new InfoField(true);
  hbox->addWidget(nfoBPS);
  lblETA = new QLabel(tr("ETA:"));
  hbox->addWidget(lblETA);

  nfoETA = new InfoField(true);
  lay->addWidget(nfoETA, CR++, 2);
  lay->addItem(new QSpacerItem(0, 10), ++CR, 0);

  mleStatus = new MLEdit(true);
  mleStatus->setReadOnly(true);
  ++CR;
  lay->addWidget(mleStatus, CR, 0, 1, 3);
  mleStatus->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

  lay->addItem(new QSpacerItem(0, 3), ++CR, 0);

  hbox = new QHBoxLayout();
  hbox->setSpacing(8);
  lay->addLayout(hbox, CR, 0, 1, 3);

  btnCancel = new QPushButton(tr("&Cancel Transfer"));
  btnCancel->setMinimumWidth(75);
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));
  hbox->addWidget(btnCancel);

  Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
      Licq::gPluginManager.getProtocolInstance(myUserId.ownerId()));
  if (icq)
  {
    //TODO fix this
    ftman = icq->createFileTransferManager(myUserId);
    ftman->SetUpdatesEnabled(2);
    sn = new QSocketNotifier(ftman->Pipe(), QSocketNotifier::Read);
    connect(sn, SIGNAL(activated(int)), SLOT(slot_ft()));
  }
  else
  {
    // ICQ protocol isn't loaded
    ftman = NULL;
    sn = NULL;
    close();
  }

  #ifdef USE_KDE
    btnOpen = new QPushButton(tr("&Open"));
    btnOpenDir = new QPushButton(tr("O&pen Dir"));
    btnOpen->hide();
    btnOpenDir->hide();
    connect(btnOpen, SIGNAL(clicked()), SLOT(slot_open()));
    connect(btnOpenDir, SIGNAL(clicked()), SLOT(slot_opendir()));
  hbox->addWidget(btnOpen);
  hbox->addWidget(btnOpenDir);
  #else
    btnOpen = 0;
    btnOpenDir = 0;
  #endif
}


//-----Destructor---------------------------------------------------------------
FileDlg::~FileDlg()
{
  delete sn;
  delete ftman;
}


unsigned short FileDlg::LocalPort()
{
  return ftman->LocalPort();
}


//-----fileCancel---------------------------------------------------------------
void FileDlg::slot_cancel()
{
  // close the local file and other stuff
  if (sn != NULL) sn->setEnabled(false);
  mleStatus->append(tr("File transfer cancelled."));
  btnCancel->setText(tr("Close"));
  ftman->CloseFileTransfer();
}

//-----open button--------------------------------------------------------------
void FileDlg::slot_open()
{
  #ifdef USE_KDE
    new KRun ("file:" + nfoLocalFileName->text(), 0, true, true);
  #endif
}

//-----open directory-----------------------------------------------------------
void FileDlg::slot_opendir()
{
  #ifdef USE_KDE
    new KRun ("file:" + nfoLocalFileName->text().replace(QRegExp("/[^/]*$"),""),
        0, true, true);
  #endif
}

//-----fileUpdate---------------------------------------------------------------
void FileDlg::slot_update()
{
  //update time, BPS and eta
  static char sz[16];

  // Current File

  // Transfered
  nfoFileSize->setText(QString("%1/%2")
      .arg(encodeFSize(ftman->FilePos()))
      .arg(encodeFSize(ftman->FileSize())));

  // Time
  time_t nTime = time(NULL) - ftman->StartTime();
  unsigned long nBytesTransfered = ftman->BytesTransfered();
  sprintf(sz, "%02lu:%02lu:%02lu", (unsigned long)(nTime / 3600), (unsigned long)((nTime % 3600) / 60), (unsigned long)((nTime % 60)));
  nfoTime->setText(sz);
  if (nTime == 0 || nBytesTransfered == 0)
  {
    nfoBPS->setText("---");
    nfoETA->setText("---");
    return;
  }

  // BPS
  nfoBPS->setText(QString("%1/s").arg(encodeFSize(nBytesTransfered / nTime)));

  // ETA
  int nBytesLeft = ftman->FileSize() - ftman->FilePos();
  time_t nETA = (time_t)(nBytesLeft / (nBytesTransfered / nTime));
  sprintf(sz, "%02lu:%02lu:%02lu", (unsigned long)(nETA / 3600), (unsigned long)((nETA % 3600) / 60), (unsigned long)((nETA % 60)));
  nfoETA->setText(sz);

  // Batch

  // Transfered
  nfoBatchSize->setText(QString(tr("%1/%2"))
     .arg(encodeFSize(ftman->BatchPos())).arg(encodeFSize(ftman->BatchSize())));


  barTransfer->setValue(ftman->FilePos() / 1024);
  barBatchTransfer->setValue(ftman->BatchPos() / 1024);
}




//-----ReceiveFiles----------------------------------------------------------
bool FileDlg::ReceiveFiles()
{
  // Get the local directory to save to
  QString d;
#ifdef USE_KDE
  d = KFileDialog::getExistingDirectory(QString(QDir::homePath()), this);
#else
  d = QFileDialog::getExistingDirectory(this, QString(), QDir::homePath());
#endif
  if (d.isNull()) return false;

  // Cut trailing slash
  if (d.right(1) == "/") d.truncate(d.length()-1);

  if (!ftman->receiveFiles(QFile::encodeName(d).data()))
    return false;

  mleStatus->append(tr("Waiting for connection..."));
  show();
  return true;
}


//-----FileDlg::slot_ft-----------------------------------------------------
void FileDlg::slot_ft()
{
  // Read out any pending events
  char buf[32];
  read(ftman->Pipe(), buf, 32);

  Licq::IcqFileTransferEvent* e = NULL;
  while ( (e = ftman->PopFileTransferEvent()) != NULL)
  {
    switch(e->Command())
    {
      case Licq::FT_STARTxBATCH:
      {
        setWindowTitle(QString(tr("Licq - File Transfer (%1)")).arg(QString::fromUtf8(ftman->remoteName().c_str())));
        nfoTotalFiles->setText(QString("%1 / %2").arg(1).arg(ftman->BatchFiles()));
        nfoBatchSize->setText(encodeFSize(ftman->BatchSize()));
        barBatchTransfer->setMaximum(ftman->BatchSize() / 1024);
        barBatchTransfer->setValue(0);
        break;
      }

      case Licq::FT_CONFIRMxFILE:
      {
        // Use this opportunity to encode the filename
        ftman->startReceivingFile(ftman->fileName());
        break;
      }

      case Licq::FT_STARTxFILE:
      {
        nfoTotalFiles->setText(QString("%1 / %2").arg(ftman->CurrentFile()).arg(ftman->BatchFiles()));
        nfoTransferFileName->setText(QFile::decodeName(ftman->fileName().c_str()));
        nfoLocalFileName->setText(QFile::decodeName(ftman->pathName().c_str()));
        nfoFileSize->setText(encodeFSize(ftman->FileSize()));
        barTransfer->setMaximum(ftman->FileSize() / 1024);
        if (ftman->isReceiver())
          mleStatus->append(tr("Receiving file..."));
        else
          mleStatus->append(tr("Sending file..."));
        break;
      }

      case Licq::FT_UPDATE:
      {
        slot_update();
        break;
      }

      case Licq::FT_DONExFILE:
      {
        slot_update();
        if (ftman->isReceiver())
          mleStatus->append(tr("Received %1 from %2 successfully.")
              .arg(QFile::decodeName(e->fileName().c_str()))
              .arg(QString::fromUtf8(ftman->remoteName().c_str())));
        else
          mleStatus->append(tr("Sent %1 to %2 successfully.")
              .arg(QFile::decodeName(e->fileName().c_str()))
              .arg(QString::fromUtf8(ftman->remoteName().c_str())));
        break;
      }

      case Licq::FT_DONExBATCH:
      {
        mleStatus->append(tr("File transfer complete."));
        btnCancel->setText(tr("OK"));
        if (btnOpen && btnOpenDir && ftman->isReceiver())
        {
          btnOpen->show();
          btnOpenDir->show();
        }

        ftman->CloseFileTransfer();
        break;
      }

      case Licq::FT_ERRORxCLOSED:
      {
        btnCancel->setText(tr("Close"));
        mleStatus->append(tr("Remote side disconnected."));
        ftman->CloseFileTransfer();
        WarnUser(this, tr("Remote side disconnected."));
        break;
      }

      case Licq::FT_ERRORxFILE:
      {
        btnCancel->setText(tr("Close"));
        mleStatus->append(tr("File I/O error: %1.").arg(QFile::decodeName(ftman->pathName().c_str())));
        ftman->CloseFileTransfer();
        WarnUser(this, tr("File I/O Error:\n%1\n\nSee Network Window for details.")
           .arg(QFile::decodeName(ftman->pathName().c_str())));
        break;
      }

      case Licq::FT_ERRORxHANDSHAKE:
      {
        btnCancel->setText(tr("Close"));
        mleStatus->appendNoNewLine(tr("Handshaking error.\n"));
        ftman->CloseFileTransfer();
        WarnUser(this, tr("Handshake Error.\nSee Network Window for details."));
        break;
      }

      case Licq::FT_ERRORxCONNECT:
      {
        btnCancel->setText(tr("Close"));
        mleStatus->appendNoNewLine(tr("Connection error.\n"));
        ftman->CloseFileTransfer();
        WarnUser(this,
          tr("Unable to reach remote host.\nSee Network Window for details."));
        break;
      }

      case Licq::FT_ERRORxBIND:
      {
        btnCancel->setText(tr("Close"));
        mleStatus->appendNoNewLine(tr("Bind error.\n"));
        ftman->CloseFileTransfer();
        WarnUser(this,
          tr("Unable to bind to a port.\nSee Network Window for details."));
        break;
      }

      case Licq::FT_ERRORxRESOURCES:
      {
        btnCancel->setText(tr("Close"));
        mleStatus->appendNoNewLine(tr("Not enough resources.\n"));
        ftman->CloseFileTransfer();
        WarnUser(this,
          tr("Unable to create a thread.\nSee Network Window for details."));
        break;
      }
    }

    delete e;
  }
}



//-----FileDlg::SendFiles---------------------------------------------------
bool FileDlg::SendFiles(const std::list<std::string>& fl, unsigned short nPort)
{
  ftman->sendFiles(fl, nPort);

  mleStatus->append(tr("Connecting to remote..."));
  show();
  return true;
}


// -----------------------------------------------------------------------------

QString FileDlg::encodeFSize(unsigned long size)
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
  else if(size != 1) {
    size *= 10;
    unit = tr("Bytes");
  }
  else {
    size *= 10;
    unit = tr("Byte");
  }

  return QString("%1.%2 %3").arg(size/10).arg(size%10).arg(unit);
}

#if 0
//-----GetLocalFileName---------------------------------------------------------
// Sets the local filename and opens the file
// returns false if the user hits cancel
bool FileDlg::GetLocalFileName()
{
  QString f;
  bool bValid = false;

  const QTextCodec* codec = UserCodec::codecForUserId(myUserId);

  // Get the local filename and open it, loop until valid or cancel
  while(!bValid)
  {
#ifdef USE_KDE
    KURL u = KFileDialog::getSaveURL(
          QString(QDir::homeDirPath() + "/" + codec->toUnicode(m_sFileInfo.szName)),
          QString::null, this);
    f = u.path();
#else
    f = QFileDialog::getSaveFileName(0, QString(),
          QString(QDir::homeDirPath() + "/" + codec->toUnicode(m_sFileInfo.szName)),
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
  barTransfer->setTotalSteps(m_sFileInfo.nSize / 1024);
  barTransfer->setProgress(0);
  return(true);
}
#endif

