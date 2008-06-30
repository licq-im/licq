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

#ifndef SHOWAWAYMSGDLG_H
#define SHOWAWAYMSGDLG_H

#include "licqdialog.h"

//#include "user.h"

class MLEditWrap;
class CSignalManager;
class CICQDaemon;
class ICQEvent;

class QPushButton;
class QCheckBox;


class ShowAwayMsgDlg : public LicqDialog
{
  Q_OBJECT
public:
  ShowAwayMsgDlg(CICQDaemon *_server, CSignalManager *_sigman,
                 const char *szId, unsigned long nPPID,
                 QWidget *parent = 0);
  virtual ~ShowAwayMsgDlg();

protected:
  char *m_szId;
  unsigned long m_nPPID;
  MLEditWrap *mleAwayMsg;
  QCheckBox *chkShowAgain;
  QPushButton *btnOk;

  CSignalManager *sigman;
  CICQDaemon *server;
  unsigned long icqEventTag;

protected slots:
  virtual void accept();
  void doneEvent(ICQEvent *);
};


#endif
