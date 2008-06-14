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

#ifndef EDITGRP_H
#define EDITGRP_H

#include <qvaluelist.h>
#include <qwidget.h>

class QListBox;
class QLineEdit;
class QGroupBox;
class QPushButton;

class CSignalManager;

class EditGrpDlg : public QWidget
{
  Q_OBJECT
public:
  EditGrpDlg(CSignalManager* signalManager, QWidget *parent = 0);
protected:
  QListBox *lstGroups;
  QGroupBox *grpGroups;
  QPushButton *btnAdd, *btnRemove, *btnUp, *btnDown, *btnDone, *btnEdit,
              *btnSave;

  QLineEdit *edtName;

  void RefreshList();

protected slots:
  void slot_add();
  void slot_remove();
  void slot_up();
  void slot_down();
  void slot_edit();
  void slot_editok();
  void slot_editcancel();
signals:
  void signal_updateGroups();

private slots:
  void listUpdated(CICQSignal* sig);

private:
  unsigned short currentGroupId() const;
  void setCurrentGroupId(unsigned short groupId);
  void moveGroup(int delta);

  QValueList<unsigned short> myGroupIds;
  unsigned short myEditGroupId;
};

#endif
