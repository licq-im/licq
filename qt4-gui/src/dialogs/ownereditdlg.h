/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef OWNEREDITDLG_H
#define OWNEREDITDLG_H

#include <QDialog>

#include <licq/userid.h>

class QCheckBox;
class QLineEdit;

namespace LicqQtGui
{
class SpecialSpinBox;

class OwnerEditDlg : public QDialog
{
  Q_OBJECT

public:
  OwnerEditDlg(unsigned long ppid, QWidget* parent = NULL);

  OwnerEditDlg(const Licq::UserId& ownerId, unsigned setStatus = 0,
      const QString& autoMessage = QString::null, QWidget* parent = NULL);

private slots:
  void slot_ok();

private:
  void init();

  QLineEdit* edtId;
  QLineEdit* edtPassword;
  QCheckBox* chkSave;
  QLineEdit* myHostEdit;
  SpecialSpinBox* myPortSpin;
  Licq::UserId myOwnerId;
  unsigned long myPpid;
  bool myNewOwner;
  unsigned mySetStatus;
  QString myAutoMessage;
};

} // namespace LicqQtGui

#endif // OWNEREDITDLG_H
