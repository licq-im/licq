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

#include <QTimer>
#include <QWidget>

#include <licq_filetransfer.h>

class CFileTransferManager;
class QLabel;
class QProgressBar;
class QPushButton;
class QSocketNotifier;

namespace LicqQtGui
{
class InfoField;
class MLEdit;

class FileDlg : public QWidget
{
   Q_OBJECT
public:
  FileDlg(const char* szId, unsigned long nPPID, QWidget* parent = 0);
  virtual ~FileDlg();

  bool SendFiles(ConstFileList filelist, unsigned short nPort);
  bool ReceiveFiles();

  unsigned short LocalPort();
  QString Id()  { return myId; }
  unsigned long PPID()  { return m_nPPID; }

private:
   QLabel* lblTransferFileName;
   QLabel* lblLocalFileName;
   QLabel* lblFileSize;
   QLabel* lblTrans;
   QLabel* lblBatch;
   QLabel* lblTime;
   QLabel* lblBPS;
   QLabel* lblETA;
   InfoField* nfoFileSize;
   InfoField* nfoTransferFileName;
   InfoField* nfoTotalFiles;
   InfoField* nfoBatchSize;
   InfoField* nfoLocalFileName;
   InfoField* nfoBPS;
   InfoField* nfoETA;
   InfoField* nfoTime;
   QPushButton* btnCancel;
   QPushButton* btnOpen;
   QPushButton* btnOpenDir;
   MLEdit* mleStatus;
   QProgressBar* barTransfer;
   QProgressBar* barBatchTransfer;

   CFileTransferManager* ftman;

   QString myId;
   unsigned long m_nPPID;
   QSocketNotifier* sn;

   QTimer m_tUpdate;

   QString encodeFSize(unsigned long size);

private slots:
  void slot_ft();
  void slot_update();
  void slot_cancel();
  void slot_open();
  void slot_opendir();
};

} // namespace LicqQtGui

#endif
