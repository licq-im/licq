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

#ifndef REGISTERUSER_H
#define REGISTERUSER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include "kwizard.h"
#define QWizard KWizard
#else
#include <qwizard.h>
#endif


class QGroupBox;
class QCheckBox;
class QWidget;
class QHideEvent;
class QLabel;

class CICQDaemon;
class CSignalManager;
class CInfoField;

class RegisterUserDlg : public QWizard
{
  Q_OBJECT
public:
  RegisterUserDlg (CICQDaemon *s, CSignalManager *sigMan, QWidget *parent = 0 );
  ~RegisterUserDlg();

signals:
  void signal_done(bool, char *, unsigned long);


    
protected:
  CICQDaemon *server;
  CSignalManager *sigMan;
  QGroupBox *grpInfo;
  CInfoField *nfoPassword1, *nfoPassword2;
  QCheckBox *chkSavePassword;
  QWidget* page1, *page2, *page3;
  QLabel* lblInfo, *lblInfo2;
  bool m_bSuccess;
  char *m_szId;
  unsigned long m_nPPID;
  
protected slots:
  void accept();
  void dataChanged();
  void nextPage();
  void verifyImage(unsigned long);
  void gotNewOwner(const char *, unsigned long);
};

class VerifyDlg : public QDialog
{
  Q_OBJECT
public:
  VerifyDlg(CICQDaemon *s, QWidget *parent = 0);

protected slots:
  void ok();
  
protected:
  CICQDaemon *server;
  CInfoField *nfoVerify;
};

#endif
