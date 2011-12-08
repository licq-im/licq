/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2011 Licq developers <licq-dev@googlegroups.com>
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

#ifndef AUTHDLG_H
#define AUTHDLG_H

#include <QDialog>

#include <licq/userid.h>

class QLineEdit;

namespace LicqQtGui
{
class MLEdit;
class ProtoComboBox;


class AuthDlg : public QDialog
{
   Q_OBJECT

public:
  enum AuthDlgType
  {
    RequestAuth,        // Request for authorization from user
    GrantAuth,          // Grant authorization for user
    RefuseAuth,         // Refuse authorization for user
  };

  /**
   * Constructor
   *
   * @param type Type of authorization dialog
   * @param userId User id to act on or invalid to let user enter
   * @param parent Parent widget
   */
  AuthDlg(enum AuthDlgType type, const Licq::UserId& userId = Licq::UserId(),
      QWidget* parent = 0);

private slots:
  /**
   * Send request/reply
   */
  void send();

private:
  enum AuthDlgType myType;
  Licq::UserId myUserId;
  ProtoComboBox* myProtocolCombo;
  QLineEdit* myAccountIdEdit;
  MLEdit* myMessageEdit;
};

} // namespace LicqQtGui

#endif
