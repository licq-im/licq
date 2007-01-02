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

#ifndef USERSELECTDLG_H
#define USERSELECTDLG_H

#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qframe.h>

#include "licqdialog.h"

class QPushButton;
class QCheckBox;
class QComboBox;

class CICQDaemon;

//TODO for other protocols

class UserSelectDlg : public LicqDialog
{
  Q_OBJECT
public:
  UserSelectDlg(CICQDaemon *s, QWidget *parent = 0);
  ~UserSelectDlg();

protected:
  CICQDaemon *server;
  QPushButton *btnOk, *btnCancel;
  QCheckBox *chkSavePassword;
  QComboBox *cmbUser;
  QLineEdit *edtPassword;
  QLabel *lblUser, *lblPassword;
  QFrame *frmUser, *frmPassword, *frmButtons;
  
protected slots:
  void slot_ok();
};

#endif
