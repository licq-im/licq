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

#ifndef ADDUSERDLG_H
#define ADDUSERDLG_H

#include "licqdialog.h"

class QPushButton;
class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;

class CICQDaemon;

class AddUserDlg : public LicqDialog
{
   Q_OBJECT
public:
   AddUserDlg (CICQDaemon *s, const char* szId = 0, unsigned long PPID = 0,
               QWidget *parent = 0);
protected:
   CICQDaemon *server;
   QPushButton *btnOk, *btnCancel;
   QLabel *lblUin, *lblProtocol;
   QLineEdit *edtUin;
   QCheckBox *chkAlert;
   QComboBox *cmbProtocol;
public slots:
   virtual void show();
protected slots:
   void ok();
};


#endif
