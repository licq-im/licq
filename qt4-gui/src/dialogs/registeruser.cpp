// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#include "registeruser.h"

#include "config.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QVariant>
#include <QVBoxLayout>
#include <QWizardPage>

#include <licq/contactlist/owner.h>
#include <licq/daemon.h>
#include <licq/icq.h>

#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::RegisterUserDlg */
/* TRANSLATOR LicqQtGui::VerifyDlg */

RegisterUserDlg::RegisterUserDlg(QWidget* parent)
  : QWizard(parent),
    myGotCaptcha(false),
    myGotOwner(false),
    mySuccess(false)
{
  Support::setWidgetProps(this, "RegisterUserDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Register Account"));

  QList<WizardButton> buttons;
  buttons << Stretch << NextButton << FinishButton << CancelButton;
  setButtonLayout(buttons);

  createIntroPage();
  createPasswordPage();
  createCaptchaPage();
  createResultPage();

  show();
}

RegisterUserDlg::~RegisterUserDlg()
{
  emit signal_done(mySuccess, myUserId);
}

void RegisterUserDlg::createIntroPage()
{
  // First page, just some welcome text
  //   in future this should be used to select protocol
  myIntroPage = new QWizardPage;

  myIntroPage->setTitle(tr("Introduction"));

  QVBoxLayout* layout = new QVBoxLayout(myIntroPage);

  QLabel* introText = new QLabel(tr("Welcome to the Registration Wizard.\n\n"
      "You can register a new ICQ account here.\n\n"
      "Press \"Next\" to proceed."));

  introText->setWordWrap(true);
  layout->addWidget(introText);

  addPage(myIntroPage);
}

void RegisterUserDlg::createPasswordPage()
{
  // Ask user for password information
  myPasswordPage = new QWizardPage;

  myPasswordPage->setTitle(tr("Select password"));
  myPasswordPage->setSubTitle(tr("Specify a password for your account.\nLength must be 1 to 8 characters."));

  QGridLayout* layout = new QGridLayout(myPasswordPage);

  QLabel* passwordLabel = new QLabel(tr("&Password:"));
  myPasswordField = new QLineEdit();
  myPasswordField->setMaxLength(8);
  myPasswordField->setEchoMode(QLineEdit::Password);
  passwordLabel->setBuddy(myPasswordField);
  layout->addWidget(passwordLabel, 0, 0);
  layout->addWidget(myPasswordField, 0, 1);

  QLabel* verifyLabel = new QLabel(tr("&Verify:"));
  myVerifyField = new QLineEdit();
  myVerifyField->setMaxLength(8);
  myVerifyField->setEchoMode(QLineEdit::Password);
  verifyLabel->setBuddy(myVerifyField);
  layout->addWidget(verifyLabel, 1, 0);
  layout->addWidget(myVerifyField, 1, 1);

  mySavePassword = new QCheckBox(tr("&Remember Password"));
  mySavePassword->setChecked(true);
  layout->addWidget(mySavePassword, 2, 0, 1, 2);

  addPage(myPasswordPage);
}

void RegisterUserDlg::createCaptchaPage()
{
  // Present captcha image and ask for user verification
  myCaptchaPage = new QWizardPage;

  myCaptchaPage->setTitle(tr("Account Verification"));
  myCaptchaPage->setSubTitle(tr("Retype the letters shown in the image."));

  QGridLayout* layout = new QGridLayout(myCaptchaPage);

  myCaptchaImage = new QLabel();
  layout->addWidget(myCaptchaImage, 0, 0, 1, 2, Qt::AlignHCenter);

  QLabel* captchaLabel = new QLabel(tr("&Verification:"));
  myCaptchaField = new QLineEdit();
  captchaLabel->setBuddy(myCaptchaField);
  layout->addWidget(captchaLabel, 1, 0);
  layout->addWidget(myCaptchaField, 1, 1);

  addPage(myCaptchaPage);
}

void RegisterUserDlg::createResultPage()
{
  // Last page, show information about the newly acquired account
  myResultPage = new QWizardPage;

  myResultPage->setTitle(tr("Registration Completed"));

  QGridLayout* layout = new QGridLayout(myResultPage);

  QLabel* finishText1 = new QLabel(tr("Account registration has been successfuly completed."));
  layout->addWidget(finishText1, 0, 0, 1, 3);

  // Show user id in a readonly input field to make it more visible and allow copying it
  QLabel* ownerIdLabel = new QLabel(tr("Your new user Id:"));
  myOwnerIdField = new QLineEdit();
  myOwnerIdField->setReadOnly(true);
  layout->addWidget(ownerIdLabel, 1, 0);
  layout->addWidget(myOwnerIdField, 1, 1);

  QLabel* finishText2 = new QLabel(tr("You are now being automatically logged on.\n"
      "Click Finish to edit your personal details.\n"
      "After you are online, you can send your personal details to the server."));
  layout->addWidget(finishText2, 2, 0, 1, 3);

  addPage(myResultPage);
}

bool RegisterUserDlg::validateCurrentPage()
{
  if (currentPage() == myPasswordPage && !myGotCaptcha)
  {
    if (myPasswordField->text().isEmpty())
      return false;

    if (myPasswordField->text() != myVerifyField->text())
    {
        WarnUser(this, tr("Passwords don't match."));
        return false;
    }

    // Disable dialog while we're waiting for server to respond
    setEnabled(false);
    button(CancelButton)->setEnabled(true);
    connect(gGuiSignalManager, SIGNAL(verifyImage(unsigned long)),
        SLOT(gotCaptcha(unsigned long)));
    gLicqDaemon->icqRegister(myPasswordField->text().toLatin1().data());
    return false;
  }

  if (currentPage() == myCaptchaPage && !myGotOwner)
  {
    if (myCaptchaField->text().isEmpty())
      return false;

    // Disable dialog while we're waiting for server to respond
    setEnabled(false);
    button(CancelButton)->setEnabled(true);
    connect(gGuiSignalManager, SIGNAL(newOwner(const Licq::UserId&)),
        SLOT(gotNewOwner(const Licq::UserId&)));
    gLicqDaemon->icqVerify(myCaptchaField->text().toLatin1().data());
    return false;
  }

  return true;
}

void RegisterUserDlg::gotCaptcha(unsigned long /* ppid */)
{
  // We got the image so reenable the dialog, set the image and go to next page
  disconnect(gGuiSignalManager, SIGNAL(verifyImage(unsigned long)),
      this, SLOT(gotCaptcha(unsigned long)));
  setEnabled(true);
  myCaptchaImage->setPixmap(QPixmap(QString(Licq::gDaemon.baseDir().c_str()) + "Licq_verify.jpg"));
  myGotCaptcha = true;
  next();
}

void RegisterUserDlg::gotNewOwner(const Licq::UserId& userId)
{
  // We got the new owner
  disconnect(gGuiSignalManager, SIGNAL(newOwner(const Licq::UserId& userId)),
      this, SLOT(gotNewOwner(const Licq::UserId& userId)));

  // Save "Remember password" setting
  {
    Licq::OwnerWriteGuard o(LICQ_PPID);
    if (o.isLocked())
      o->SetSavePassword(mySavePassword->isChecked());
  }
  Licq::gDaemon.SaveConf();

  // Mark that we have finished
  mySuccess = true;
  myUserId = userId;

  // Go to result page
  setEnabled(true);
  myGotOwner = true;
  myOwnerIdField->setText(myUserId.accountId().c_str());
  next();
}
