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

#ifndef RCDLG_H
#define RCDLG_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licqdialog.h"

class CMainWindow;
class CICQDaemon;
class CSignalManager;
class QListBox;
class QPushButton;
class LicqEvent;


class CRandomChatDlg : public LicqDialog
{
Q_OBJECT
public:
  CRandomChatDlg(CMainWindow *_mainwin, CICQDaemon *s,
                 CSignalManager *_sigman, QWidget* p = 0);
  ~CRandomChatDlg();
protected:
  CMainWindow *mainwin;
  unsigned long tag;
  CICQDaemon *server;
  CSignalManager *sigman;
  QListBox *lstGroups;
  QPushButton *btnOk, *btnCancel;
protected slots:
  void slot_ok();
  void slot_doneUserFcn(LicqEvent*);
};


class CSetRandomChatGroupDlg : public LicqDialog
{
Q_OBJECT
public:
  CSetRandomChatGroupDlg(CICQDaemon *s, CSignalManager *_sigman,
                 QWidget *p = 0);
  ~CSetRandomChatGroupDlg();
protected:
  CMainWindow *mainwin;
  unsigned long tag;
  CICQDaemon *server;
  CSignalManager *sigman;
  QListBox *lstGroups;
  QPushButton *btnOk, *btnCancel;
protected slots:
  void slot_ok();
  void slot_doneUserFcn(LicqEvent*);
};


#endif
