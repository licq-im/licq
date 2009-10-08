// -*- c-basic-offset: 2 -*-
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>
#include <qtoolbutton.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "ewidgets.h"
#include "securitydlg.h"
#include "sigman.h"

#include "licq_icqd.h"
#include "licq_events.h"
#include "licq_user.h"

SecurityDlg::SecurityDlg(CICQDaemon *s, CSignalManager *_sigman,
                         QWidget *parent)
   : LicqDialog(parent, "SecurityDialog", false, WStyle_ContextHelp | WDestructiveClose )
{
  server = s;
  sigman = _sigman;
  eSecurityInfo   = 0;
  ePasswordChange = 0;

  QString strUin = gUserManager.OwnerId(LICQ_PPID);;

  QVBoxLayout *lay = new QVBoxLayout(this, 8);
  QGroupBox *box = new QGroupBox(1, QGroupBox::Horizontal, tr("Options"), this);
  lay->addWidget(box);

  QGroupBox *passwordBox = new QGroupBox(2, QGroupBox::Horizontal,
    tr("Password/UIN settings"), this);

  box->setInsideSpacing(1);
  passwordBox->setInsideSpacing(1);

  lay->addWidget(passwordBox);

  // Password boxes
  lblUin = new QLabel(tr("&Uin:"), passwordBox);
  edtUin = new QLineEdit(passwordBox);
  QWhatsThis::add(edtUin, tr("Enter the UIN which you want to use.  "
                             "Only available if \"Local changes only\" is "
                             "checked."));
  lblPassword = new QLabel(tr("&Password:"), passwordBox);
  edtFirst = new QLineEdit(passwordBox);
  QWhatsThis::add(edtFirst, tr("Enter your ICQ password here."));
  lblVerify = new QLabel(tr("&Verify:"), passwordBox);
  edtSecond = new QLineEdit(passwordBox);
  QWhatsThis::add(edtSecond, tr("Verify your ICQ password here."));
  chkOnlyLocal = new QCheckBox(tr("&Local changes only"), passwordBox);
  QWhatsThis::add(chkOnlyLocal, tr("If checked, password/UIN changes will apply"
                                   " only on your local computer.  Useful if "
                                   "your password is incorrectly saved in Licq."));

  edtUin->setEnabled(false);
  edtFirst->setEchoMode(QLineEdit::Password);
  edtSecond->setEchoMode(QLineEdit::Password);
  lblUin->setBuddy(edtUin);
  lblPassword->setBuddy(edtFirst);
  lblVerify->setBuddy(edtSecond);

  // UIN
  edtUin->setValidator(new QIntValidator(10000, 2147483647, edtUin));
  edtUin->setText(strUin);

  // Owner password
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o != NULL)
  {
    edtFirst->setText(o->Password());
    edtSecond->setText(o->Password());
  }
  else
  {
    edtFirst->setEnabled(false);
    edtSecond->setEnabled(false);
  }

  QVBoxLayout *blay = new QVBoxLayout;
  chkAuthorization = new QCheckBox(tr("Authorization Required"), box);
  QWhatsThis::add(chkAuthorization, tr("Determines whether regular ICQ clients "
                                       "require your authorization to add you to "
                                       "their contact list."));
  chkWebAware = new QCheckBox(tr("Web Presence"), box);
  QWhatsThis::add(chkWebAware, tr("Web Presence allows users to see if you are online "
                                  "through your web indicator."));
  chkHideIp = new QCheckBox(tr("Hide IP"), box);
  QWhatsThis::add(chkHideIp, tr("Hide IP stops users from seeing your IP address. It doesn't guarantee it will be hidden though."));
  blay->addWidget(chkAuthorization);
  blay->addWidget(chkWebAware);
  blay->addWidget(chkHideIp);

  btnUpdate = new QPushButton(tr("&Update"), this);
  btnUpdate->setDefault(true);
  btnUpdate->setMinimumWidth(75);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  btnCancel->setMinimumWidth(75);
  QHBoxLayout *hlay = new QHBoxLayout;
  hlay->addWidget(QWhatsThis::whatsThisButton(this), 0, AlignLeft);
  hlay->addSpacing(20);
  hlay->addStretch(1);
  hlay->addWidget(btnUpdate, 0, AlignRight);
  hlay->addSpacing(20);
  hlay->addWidget(btnCancel, 0, AlignLeft);
  lay->addLayout(hlay);

  connect (btnUpdate, SIGNAL(clicked()), SLOT(ok()) );
  connect (btnCancel, SIGNAL(clicked()), SLOT(close()) );
  connect (chkOnlyLocal, SIGNAL(toggled(bool)), SLOT(slot_chkOnlyLocalToggled(bool)));

  // do some magic ;)
  // if we are offline, we enable the checkbox "Only local changes"
  // this saves one click :)
  if (o != NULL)  // Make sure we exist
  {
    slot_chkOnlyLocalToggled( (o->Status() == ICQ_STATUS_OFFLINE) );
    chkAuthorization->setChecked(o->GetAuthorization());
    chkWebAware->setChecked(o->WebAware());
    chkHideIp->setChecked(o->HideIp());
    gUserManager.DropOwner(o);
  }
  else
  {
    slot_chkOnlyLocalToggled(true);
    chkOnlyLocal->setEnabled(false);
    chkAuthorization->setChecked(false);
    chkWebAware->setChecked(false);
    chkHideIp->setChecked(false);
  }


  setCaption(tr("ICQ Security Options"));

  // remember the initial values
  // later we use these to apply only what has been changed by the user
  initAuthorization = chkAuthorization->isChecked();
  initWebAware      = chkWebAware->isChecked();
  initHideIp        = chkHideIp->isChecked();
  initEdtUin        = edtUin->text();
  initEdtFirst      = edtFirst->text();
  initEdtSecond     = edtSecond->text();

	// Set Tab Order
	setTabOrder(chkAuthorization, chkWebAware);
	setTabOrder(chkWebAware, chkHideIp);
	setTabOrder(chkHideIp, edtUin);
	setTabOrder(edtUin, edtFirst);
	setTabOrder(edtFirst, edtSecond);
	setTabOrder(edtSecond, chkOnlyLocal);
	setTabOrder(chkOnlyLocal, btnUpdate);
	setTabOrder(btnUpdate, btnCancel);

  show();
}


SecurityDlg::~SecurityDlg()
{
}


void SecurityDlg::ok()
{
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if(o == 0) return;
  unsigned short status = o->Status();
  gUserManager.DropOwner(o);

  // validate password
  if ((edtFirst->text().isEmpty() && !chkOnlyLocal->isChecked()) ||
       edtFirst->text().length() > 8)
  {
    InformUser(this, tr("Invalid password, must be between 1 and 8 characters."));
    return;
  }
  if (edtFirst->text() != edtSecond->text())
  {
    InformUser(this, tr("Passwords do not match, try again."));
    return;
  }

  if(status == ICQ_STATUS_OFFLINE && !chkOnlyLocal->isChecked()) {
    InformUser(this, tr("You need to be connected to the\n"
                        "ICQ Network to change the settings."));
    return;
  }

  // test if we really need to update something
  bool secUpdateNeeded = false;
  bool pasUpdateNeeded = false;

  if ((chkAuthorization->isChecked() != initAuthorization) ||
      (chkWebAware->isChecked() != initWebAware) ||
      (chkHideIp->isChecked() != initHideIp))
  {
    secUpdateNeeded = true;
    initAuthorization = chkAuthorization->isChecked();
    initWebAware = chkWebAware->isChecked();
    initHideIp = chkHideIp->isChecked();
  }
  if ((edtUin->text() != initEdtUin) || (edtFirst->text() != initEdtFirst) ||
      (edtSecond->text() != initEdtSecond))
  {
    pasUpdateNeeded = true;
    initEdtUin = edtUin->text();
    initEdtFirst = edtFirst->text();
    initEdtSecond = edtSecond->text();
  }

  if (secUpdateNeeded || pasUpdateNeeded)
  {
    btnUpdate->setEnabled(false);
    connect(sigman, SIGNAL(signal_doneUserFcn(LicqEvent*)), SLOT(slot_doneUserFcn(LicqEvent*)));

    if (chkOnlyLocal->isChecked())
    {
      gUserManager.AddOwner(edtUin->text(), LICQ_PPID);
      o = gUserManager.FetchOwner(LICQ_PPID, LOCK_W);
      o->SetPassword(edtFirst->text().latin1());
      gUserManager.DropOwner(o);
      close();
    }
    else
    {
      // eSecurityInfo and ePasswordChange contain the event numbers.
      // These are used in slot_doneUserFcn to compare if we reveived
      // what we expected :)
      if (secUpdateNeeded)
        eSecurityInfo = server->icqSetSecurityInfo(chkAuthorization->isChecked(),
          chkHideIp->isChecked(), chkWebAware->isChecked());
      if (pasUpdateNeeded)
        ePasswordChange = server->icqSetPassword(edtFirst->text().local8Bit());
      setCaption(tr("ICQ Security Options [Setting...]"));
    }
  }
  else
    close();
}


void SecurityDlg::slot_doneUserFcn(ICQEvent *e)
{
  // Checking if the received events correspond to one of
  // our sent events.
  bool bSec  = e->Equals(eSecurityInfo);
  bool bPass = e->Equals(ePasswordChange);

  if (!bSec && !bPass) return;

  QString result;
  switch (e->Result())
  {
  case EVENT_FAILED:
    result = tr("failed");
    if (bSec) InformUser(this, tr("Setting security options failed."));
    else if (bPass) InformUser(this, tr("Changing password failed."));
    break;
  case EVENT_TIMEDOUT:
    result = tr("timed out");
    if (bSec) InformUser(this, tr("Timeout while setting security options."));
    else if (bPass) InformUser(this, tr("Timeout while changing password."));
    break;
  case EVENT_ERROR:
    result = tr("error");
    if (bSec) InformUser(this, tr("Internal error while setting security options."));
    else if (bPass) InformUser(this, tr("Internal error while changing password."));
    break;
  default:
    break;
  }

  if (bSec) eSecurityInfo = 0;
  else if (bPass) ePasswordChange = 0;

  if ((eSecurityInfo == 0) && (ePasswordChange == 0)) btnUpdate->setEnabled(true);

  if(!result.isEmpty())
    setCaption(tr("ICQ Security Options [Setting...") + result + "]");
  else
  {
    if ((eSecurityInfo == 0) && (ePasswordChange == 0))
    {
      setCaption(tr("ICQ Security Options"));
      close();
    }
  }
}

void SecurityDlg::slot_chkOnlyLocalToggled(bool b)
{
  edtUin->setEnabled(b);
  chkOnlyLocal->setChecked(b);
  chkAuthorization->setEnabled(!b);
  chkWebAware->setEnabled(!b);
  chkHideIp->setEnabled(!b);
}


#include "securitydlg.moc"
