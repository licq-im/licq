/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#ifndef FILEDLG_H
#define FILEDLG_H

#include <qwidget.h>
#include <qtimer.h>

#include "ewidgets.h"
#include "licq_filetransfer.h"

class QPushButton;
class QProgressBar;
class QLabel;
class QSocketNotifier;
class CICQDaemon;
class CFileTransferManager;
class MLEditWrap;

class CFileDlg : public QWidget
{
   Q_OBJECT
public:
  CFileDlg(const char *szId, unsigned long nPPID, CICQDaemon *daemon,
    QWidget *parent = 0);
  virtual ~CFileDlg();

  bool SendFiles(ConstFileList filelist, unsigned short nPort);
  bool ReceiveFiles();

  unsigned short LocalPort();
  char *Id()  { return m_szId; }
  unsigned long PPID()  { return m_nPPID; }

protected:
   QLabel *lblTransferFileName, *lblLocalFileName, *lblFileSize,
	  *lblTrans, *lblBatch, *lblTime, *lblBPS, *lblETA;
   CInfoField *nfoFileSize,
              *nfoTransferFileName,
              *nfoTotalFiles,
              *nfoBatchSize,
              *nfoLocalFileName,
              *nfoBPS,
              *nfoETA,
              *nfoTime;
   QPushButton *btnCancel, *btnOpen, *btnOpenDir;
   MLEditWrap *mleStatus;
   QProgressBar *barTransfer, *barBatchTransfer;

   CFileTransferManager *ftman;

   char *m_szId;
   unsigned long m_nPPID;
   CICQDaemon *licqDaemon;
   QSocketNotifier *sn;

   QTimer m_tUpdate;

   QString encodeFSize(unsigned long size);

protected slots:
  void slot_ft();
  void slot_update();
  void slot_cancel();
  void slot_open();
  void slot_opendir();
};

#endif
