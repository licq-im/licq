/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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

#ifndef FORWARDDLG_H
#define FORWARDDLG_H

#include <licq_types.h>

#include "licqdialog.h"

class CMainWindow;
class QPushButton;
class CUserEvent;
class CInfoField;
class CICQDaemon;
class CSignalManager;

class CForwardDlg : public LicqDialog
{
Q_OBJECT
public:
  CForwardDlg(CSignalManager *sigMan, CUserEvent *e, QWidget *p = 0);
  ~CForwardDlg();
protected:
  virtual void dragEnterEvent(QDragEnterEvent * dee);
  virtual void dropEvent(QDropEvent * de);
  QString s1, s2;
  unsigned long m_nEventType;
  CSignalManager *sigman;
  CInfoField *edtUser;
  QPushButton *btnOk, *btnCancel;
  UserId myUserId;
protected slots:
  void slot_ok();
};

#endif
