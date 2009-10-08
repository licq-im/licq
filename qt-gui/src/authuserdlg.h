/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#ifndef AUTHUSERDLG_H
#define AUTHUSERDLG_H

#include <licq_types.h>

#include "licqdialog.h"

class QPushButton;
class QLabel;
class QGroupBox;
class QLineEdit;

class CICQDaemon;
class MLEditWrap;

class AuthUserDlg : public LicqDialog
{
   Q_OBJECT
public:
  AuthUserDlg(CICQDaemon* s, const UserId& userId,
      bool bGrant, QWidget *parent = 0);

protected:
   CICQDaemon *server;
   QPushButton *btnOk, *btnCancel;
   QLabel *lblUin;
   QGroupBox *grpResponse;
   QLineEdit *edtUin;
   MLEditWrap* mleResponse;

  UserId myUserId;
   bool m_bGrant;
protected slots:
   void ok();
};


#endif
