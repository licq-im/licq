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

#ifndef SECURITYDLG_H
#define SECURITYDLG_H

#include <QDialog>

class QCheckBox;
class QPushButton;

class LicqEvent;

namespace LicqQtGui
{
class SecurityDlg : public QDialog
{
  Q_OBJECT

public:
  SecurityDlg(QWidget* parent = NULL);

private:
  QPushButton* btnUpdate;

  QCheckBox* chkWebAware;
  QCheckBox* chkAuthorization;
  QCheckBox* chkHideIp;

  QString title;

  unsigned long eSecurityInfo;

private slots:
  void ok();
  void doneUserFcn(const LicqEvent* e);
};

} // namespace LicqQtGui

#endif
