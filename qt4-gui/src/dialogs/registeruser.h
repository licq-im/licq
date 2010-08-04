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

#ifndef REGISTERUSER_H
#define REGISTERUSER_H

#include "config.h"

#include <QWizard>

#include <licq/userid.h>

class QCheckBox;
class QLabel;
class QLineEdit;
class QWizardPage;

namespace LicqQtGui
{
class RegisterUserDlg : public QWizard
{
  Q_OBJECT

public:
  RegisterUserDlg(QWidget* parent = NULL);
  ~RegisterUserDlg();

  virtual bool validateCurrentPage();

signals:
  void signal_done(bool success, const Licq::UserId& userId);

private:
  void createIntroPage();
  void createPasswordPage();
  void createCaptchaPage();
  void createResultPage();

  bool myGotCaptcha;
  bool myGotOwner;

  bool mySuccess;
  Licq::UserId myUserId;

  QWizardPage* myIntroPage;
  QWizardPage* myPasswordPage;
  QWizardPage* myCaptchaPage;
  QWizardPage* myResultPage;

  QLineEdit* myPasswordField;
  QLineEdit* myVerifyField;
  QCheckBox* mySavePassword;
  QLabel* myCaptchaImage;
  QLineEdit* myCaptchaField;
  QLineEdit* myOwnerIdField;

private slots:
  void gotCaptcha(unsigned long ppid);
  void gotNewOwner(const Licq::UserId& userId);
};

} // namespace LicqQtGui

#endif
