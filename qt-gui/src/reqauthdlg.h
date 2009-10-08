/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2002-2009 Licq developers
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

#ifndef REQAUTHDLG_H
#define REQAUTHDLG_H

#include "licqdialog.h"

class QPushButton;
class QLabel;
class QLineEdit;
class QGroupBox;

class CICQDaemon;
class MLEditWrap;

class ReqAuthDlg : public LicqDialog
{
   Q_OBJECT
public:
   ReqAuthDlg(CICQDaemon *s, const char *szId, unsigned long nPPID, QWidget *parent = 0);

protected:
   CICQDaemon *server;
   QPushButton *btnOk, *btnCancel;
   QLabel *lblUin;
   QLineEdit *edtUin;
   QGroupBox *grpRequest;
   MLEditWrap *mleRequest;
protected slots:
   void ok();
};


#endif
