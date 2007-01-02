// -*- c-basic-offset: 2 -*-
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qpixmap.h>

#include "sigman.h"
#include "registeruser.h"
#include "ewidgets.h"
#include "licq_icqd.h"
#include "licq_user.h"

#ifdef USE_KDE
RegisterUserDlg::RegisterUserDlg(CICQDaemon *s, CSignalManager *m, QWidget *parent)  : KWizard
#else
RegisterUserDlg::RegisterUserDlg(CICQDaemon *s, CSignalManager *m, QWidget *parent)  : QWizard
#endif
           (parent, "RegisterUserDialog", false, WDestructiveClose)
{
  m_bSuccess = false;
  m_szId = 0;
  m_nPPID = 0;
  
  page1 = new QLabel(tr("Welcome to the Registration Wizard.\n\n"
                        "You can register a new user here.\n\n"
                        "Press \"Next\" to proceed."), this);

  addPage(page1, tr("Account Registration"));
  setHelpEnabled(page1, false);

  server = s;
  sigMan = m;
  
  page2 = new QVBox(this);

  (void) new QLabel(tr("Enter a password to protect your account."), page2);

  grpInfo = new QGroupBox(2, Horizontal, page2);

  (void) new QLabel(tr("Password:"), grpInfo);

  nfoPassword1 = new CInfoField(grpInfo, false);
  nfoPassword1->setEchoMode(QLineEdit::Password);

  (void) new QLabel(tr("Verify:"), grpInfo);
  nfoPassword2 = new CInfoField(grpInfo, false);
  nfoPassword2->setEchoMode(QLineEdit::Password);

  chkSavePassword = new QCheckBox(tr("&Remember Password"), grpInfo);

  addPage(page2, tr("Account Registration - Step 2"));
  setHelpEnabled(page2, false);

  connect (cancelButton(), SIGNAL(clicked()), SLOT(hide()) );
  connect (nfoPassword2, SIGNAL(textChanged(const QString&)), this, SLOT(dataChanged()));
  chkSavePassword->setChecked(true);
  setNextEnabled(page2, false);

  connect(backButton(), SIGNAL(clicked()), this, SLOT(nextPage()));
  connect(nextButton(), SIGNAL(clicked()), this, SLOT(nextPage()));
  connect(cancelButton(), SIGNAL(clicked()), this, SLOT(close()));

  page3 = new QVBox(this);
  addPage(page3, tr("Account Registration - Step 3"));
  setHelpEnabled(page3, false);

  lblInfo = new QLabel(page3);
  lblInfo2 = new QLabel(page3);
  
  setMinimumSize(300, 200);
  setCaption(tr("Licq Account Registration"));
  
  connect(sigMan, SIGNAL(signal_verifyImage(unsigned long)), this,
    SLOT(verifyImage(unsigned long)));
  connect(sigMan, SIGNAL(signal_newOwner(const char *, unsigned long)),
    this, SLOT(gotNewOwner(const char *, unsigned long)));
  show();
}

RegisterUserDlg::~RegisterUserDlg()
{
  emit signal_done(m_bSuccess, m_szId, m_nPPID);
}

void RegisterUserDlg::dataChanged()
{
  setNextEnabled(page2, true);
}

void RegisterUserDlg::nextPage()
{
  if(currentPage() == page3) {
    bool errorOccured = false;
    if(nfoPassword1->text().length() > 8 || nfoPassword2->text().length() > 8)
    {
      lblInfo->setText(tr("Invalid password, must be between 1 and 8 characters."));
      errorOccured = true;
    }
    else if(nfoPassword1->text().length() == 0)
    {
      lblInfo->setText(tr("Please enter your password in both input fields."));
      errorOccured = true;
    }
    else if(nfoPassword1->text() != nfoPassword2->text())
    {
      lblInfo->setText(tr("The passwords don't seem to match."));
      errorOccured = true;
    }

    if(errorOccured)
    {
      lblInfo2->setText(tr("Now please press the 'Back' button "
                          "and try again."));
    }
    else
    {
      lblInfo->clear();
      lblInfo2->setText(tr("Now please click 'Finish' to start "
                          "the registration process."));
    }
    setFinishEnabled(page3, !errorOccured);
  }
}



void RegisterUserDlg::accept()
{
  setCaption(tr("Account Registration in Progress..."));
  server->icqRegister(nfoPassword1->text().latin1());
  finishButton()->setEnabled(false);
  cancelButton()->setEnabled(false);
  nfoPassword1->setEnabled(false);
  nfoPassword2->setEnabled(false);

  server->SaveConf();
//  close(true);
}

void RegisterUserDlg::verifyImage(unsigned long /* nPPID */)
{
  // XXX Use nPPID
  (void) new VerifyDlg(server, this);
}

void RegisterUserDlg::gotNewOwner(const char *szId, unsigned long nPPID)
{
  QString strMsg(tr("Account registration has been successfuly completed.\n"
                    "Your new user id is %1.\n"
                    "You are now being automatically logged on.\n"
                    "Click OK to edit your personal details.\n"
                    "After you are online, you can send your personal details to the server.")
                    .arg(szId));
  InformUser(this, strMsg);
  
  m_bSuccess = true;
  m_szId = const_cast<char *>(szId);
  m_nPPID = nPPID;
  close(true);
}

VerifyDlg::VerifyDlg(CICQDaemon *s, QWidget *parent)
  : QDialog(parent, "VerifyDlg", true, WDestructiveClose)
{
  server = s;
  
  // XXX Assume ICQ protocol
  QString strFile = BASE_DIR;
  strFile += "/Licq_verify.jpg";
  QPixmap *pixVerify = new QPixmap(strFile);
  
  QGridLayout *lay = new QGridLayout(this, 3, 2, 10);
  QLabel *lblPix = new QLabel(".", this);
  lblPix->setPixmap(*pixVerify);
  lay->addMultiCellWidget(lblPix, 0, 0, 0, 1);
  
  lay->addWidget(new QLabel(tr("Retype the letters shown above:"), this), 1, 0);
  nfoVerify = new CInfoField(this, false);
  lay->addWidget(nfoVerify, 1, 1);
  
  QHBox *hBox = new QHBox(this);
  QPushButton *btnOk = new QPushButton(tr("&OK"), hBox);
  btnOk->setDefault(true);
  connect(btnOk, SIGNAL(clicked()), SLOT(ok()));
  lay->addMultiCellWidget(hBox, 2, 2, 0, 1);
  
  setCaption(tr("Licq - New Account Verification"));
  show();
}

void VerifyDlg::ok()
{
  QString strText = nfoVerify->text();
  server->icqVerify(strText.latin1());
  close();
}

#include "registeruser.moc"
