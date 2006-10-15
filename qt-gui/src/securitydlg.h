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

#ifndef SECURITYDLG_H
#define SECURITYDLG_H

#include <qwidget.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "licqdialog.h"

class QPushButton;
class QCheckBox;
class ICQEvent;

class CICQDaemon;
class CSignalManager;

//TODO Maybe add support for other protocols.  Probably unnecessary.

class SecurityDlg : public LicqDialog
{
   Q_OBJECT
public:
  SecurityDlg (CICQDaemon *s, CSignalManager *, QWidget* parent = 0);
  ~SecurityDlg();

protected:
  CICQDaemon *server;
  CSignalManager *sigman;
  QPushButton *btnUpdate, *btnCancel;
  QCheckBox *chkWebAware, *chkAuthorization, *chkHideIp, *chkOnlyLocal;
  QLineEdit *edtUin, *edtFirst, *edtSecond;
  QLabel *lblUin, *lblPassword, *lblVerify;

  // Some returned events
  unsigned long eSecurityInfo, ePasswordChange;

  // For saving initial values
  bool initAuthorization, initWebAware, initHideIp;
  QString initEdtUin, initEdtFirst, initEdtSecond;

public slots:
  void slot_doneUserFcn(ICQEvent *);

protected slots:
  void ok();
  void slot_chkOnlyLocalToggled(bool b);
};


#endif
