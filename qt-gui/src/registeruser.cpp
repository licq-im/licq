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

RegisterUserDlg::RegisterUserDlg(CICQDaemon *s, QWidget *parent, const char *name)
  : QWizard(parent, name, false)
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

  (void) new QLabel(tr("Password:"), grpInfo);

  nfoPassword1 = new CInfoField(grpInfo, false);
  nfoPassword1->setEchoMode(QLineEdit::Password);
  nfoPassword1->setMaxLength(8);

  (void) new QLabel(tr("Verify:"), grpInfo);
  nfoPassword2 = new CInfoField(grpInfo, false);
  nfoPassword2->setEchoMode(QLineEdit::Password);
  nfoPassword2->setMaxLength(8);

  addPage(page2, tr("UIN Registration - Step 2"));
  setHelpEnabled(page2, false);

  connect (cancelButton(), SIGNAL(clicked()), SLOT(hide()) );
  connect (chkExistingUser, SIGNAL(toggled(bool)), nfoUin, SLOT(setEnabled(bool)));
  connect (nfoPassword2, SIGNAL(textChanged(const QString&)), this, SLOT(dataChanged()));
  chkExistingUser->setChecked(false);
  nfoUin->setEnabled(false);

  page3 = new QVBox(this);

  (void) new QLabel(tr("Now click Finish to start the registration process."), page3);

  addPage(page3, tr("UIN Registration - Step 3"));
  setHelpEnabled(page3, false);

  setMinimumSize(300, 200);
  setCaption(tr("Licq User Registration"));

  show();
}


void RegisterUserDlg::dataChanged()
{
  // This is a HACK. It should validate the user Input first and only
  // set finish button enabled when the data is ok.
  setFinishEnabled(page3, true);
}

void RegisterUserDlg::accept()
{
  const char *szPassword = nfoPassword1->text().local8Bit();
  const char *szPassword2 = nfoPassword2->text().local8Bit();
  // Validate password
  if (szPassword == NULL || strlen(szPassword) > 8)
  {
    InformUser (this, tr("Invalid password, must be 8 characters or less."));
    return;
  }
  if (szPassword2 == NULL || strcmp(szPassword, szPassword2) != 0)
  {
    InformUser (this, tr("Passwords do not match, try again."));
    return;
  }

  if (chkExistingUser->isChecked())
  {
    unsigned long nUin = nfoUin->text().toULong();
    // Validate uin
    if (nUin <= 0)
    {
      InformUser (this, tr("Invalid UIN.  Try again."));
      return;
    }
    gUserManager.SetOwnerUin(nUin);
    ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
    o->SetPassword(szPassword);
    gUserManager.DropOwner();
    InformUser (this, tr("Registered succesfully.  Now log on and update your personal info."));
    hide();
  }
  else
  {
    setCaption(tr("User Registration in Progress..."));
    server->icqRegister(szPassword);
    finishButton()->setEnabled(false);
    cancelButton()->setEnabled(false);
    nfoUin->setEnabled(false);
    nfoPassword1->setEnabled(false);
    nfoPassword2->setEnabled(false);
    chkExistingUser->setEnabled(false);
  }
}

#include "registeruser.moc"
