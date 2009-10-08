/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2003-2009 Licq developers
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

#include <QDialog>

#include <licq_filetransfer.h>

class QListWidget;
class QPushButton;

namespace LicqQtGui
{

class EditFileListDlg : public QDialog
{
  Q_OBJECT

public:
  EditFileListDlg(ConstFileList* fileList, QWidget* parent = 0);

signals:
  void fileDeleted(unsigned);

private:
  QListWidget* lstFiles;
  ConstFileList* myFileList;
  QPushButton* btnDone;
  QPushButton* btnUp;
  QPushButton* btnDown;
  QPushButton* btnDelete;

  void refreshList();
  void moveCurrentItem(bool up = true);

private slots:
  void currentChanged(int newCurrent);
  void up();
  void down();
  void remove();
};

} // namespace LicqQtGui

#endif
