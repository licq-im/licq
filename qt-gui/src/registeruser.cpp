// -*- c-basic-offset: 2 -*-
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qvbox.h>
#include <qlayout.h>

#include "registeruser.h"
#include "ewidgets.h"
#include "licq_icqd.h"
#include "licq_user.h"

#ifdef USE_KDE
RegisterUserDlg::RegisterUserDlg(CICQDaemon *s, QWidget *parent)  : KWizard
#else
RegisterUserDlg::RegisterUserDlg(CICQDaemon *s, QWidget *parent)  : QWizard
#endif
           (parent, "RegisterUserDialog", false, WDestructiveClose)
{
  page1 = new QLabel(tr("Welcome to the Registration Wizard.\n\n"
                        "You can register a new user here, or configure "
                        "Licq to use an existing UIN.\n\n"
                        "If you are registering a new uin, choose "
                        "a password and click \"Finish\".\n"
                        "If you already have a uin, then toggle \"Register Existing User\",\n"
                        "enter your uin and your password, and click \"OK\"\n\n"
                        "Press \"Next\" to proceed."), this);

  addPage(page1, tr("UIN Registration"));
  setHelpEnabled(page1, false);

  server = s;
  page2 = new QVBox(this);

  grpInfo = new QGroupBox(2, Horizontal, page2);

  chkExistingUser = new QCheckBox(tr("&Register Existing User"), grpInfo);
  // dummy widget
  (void) new QWidget(grpInfo);

  (void) new QLabel(tr("Uin:"), grpInfo);

  nfoUin = new CInfoField(grpInfo, false);
  nfoUin->setValidator(new QIntValidator(10000, 2000000000, this));

  (void) new QLabel(tr("Password:"), grpInfo);

  nfoPassword1 = new CInfoField(grpInfo, false);
  nfoPassword1->setEchoMode(QLineEdit::Password);

  (void) new QLabel(tr("Verify:"), grpInfo);
  nfoPassword2 = new CInfoField(grpInfo, false);
  nfoPassword2->setEchoMode(QLineEdit::Password);

  chkSavePassword = new QCheckBox(tr("&Save Password"), grpInfo);

  addPage(page2, tr("UIN Registration - Step 2"));
  setHelpEnabled(page2, false);

  connect (cancelButton(), SIGNAL(clicked()), SLOT(hide()) );
  connect (chkExistingUser, SIGNAL(toggled(bool)), nfoUin, SLOT(setEnabled(bool)));
  connect (nfoPassword2, SIGNAL(textChanged(const QString&)), this, SLOT(dataChanged()));
  chkExistingUser->setChecked(false);
  chkSavePassword->setChecked(true);
  nfoUin->setEnabled(false);
  setNextEnabled(page2, false);

  connect(backButton(), SIGNAL(clicked()), this, SLOT(nextPage()));
  connect(nextButton(), SIGNAL(clicked()), this, SLOT(nextPage()));
  connect(cancelButton(), SIGNAL(clicked()), this, SLOT(close()));

  page3 = new QVBox(this);
  addPage(page3, tr("UIN Registration - Step 3"));
  setHelpEnabled(page3, false);

  lblInfo = new QLabel(page3);
  lblInfo2 = new QLabel(page3);

  setMinimumSize(300, 200);
  setCaption(tr("Licq User Registration"));

  show();
}

RegisterUserDlg::~RegisterUserDlg()
{
  emit signal_done();
}

void RegisterUserDlg::dataChanged()
{
  setNextEnabled(page2, true);
}

void RegisterUserDlg::nextPage()
{
  if(currentPage() == page3) {
    bool errorOccured = false;
    if(chkExistingUser->isChecked() && nfoUin->text().toULong() < 10000)
    {
      lblInfo->setText(tr("You need to enter a valid UIN when you "
                           "try to register an existing user. "));
      errorOccured = true;
    }
    else if(nfoPassword1->text().length() > 8 || nfoPassword2->text().length() > 8)
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
  if (chkExistingUser->isChecked())
  {
    unsigned long nUin = nfoUin->text().toULong();
    // Validate uin
    if (nUin == 0)
    {
      InformUser (this, tr("Invalid UIN.  Try again."));
      return;
    }
    gUserManager.SetOwnerUin(nUin);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->SetSavePassword(chkSavePassword->isChecked());
    o->SetPassword(nfoPassword1->text().latin1());
    gUserManager.DropOwner();
    InformUser (this, tr("Registered succesfully.  Now log on and update your personal info."));
    hide();
  }
  else
  {
    setCaption(tr("User Registration in Progress..."));
    server->icqRegister(nfoPassword1->text().latin1());
    finishButton()->setEnabled(false);
    cancelButton()->setEnabled(false);
    nfoUin->setEnabled(false);
    nfoPassword1->setEnabled(false);
    nfoPassword2->setEnabled(false);
    chkExistingUser->setEnabled(false);
  }
  server->SaveConf();
  close(true);
}

#include "registeruser.moc"
