/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2003-2006 Licq developers
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

#ifndef EDITFILEDLG_H
#define EDITFILEDLG_H 

#include <qwidget.h>
#include <qlistbox.h>
#include <qpushbutton.h>

#include "licqdialog.h"
#include "licq_filetransfer.h"

class QPushButton;
class QListView;

class CEditFileListDlg : public LicqDialog
{
   Q_OBJECT
public:
  CEditFileListDlg(ConstFileList *_lFileList, QWidget *parent = 0);
  virtual ~CEditFileListDlg();

protected:
   QPushButton *btnDone, *btnUp, *btnDown, *btnDel;
   QListBox *lstFiles;
   ConstFileList *m_lFileList;
   
  void RefreshList();

protected slots:
  void slot_selectionChanged(QListBoxItem *item);
  void slot_done();
  void slot_up();
  void slot_down();
  void slot_del();

signals:
  void file_deleted(unsigned);
};

#endif
