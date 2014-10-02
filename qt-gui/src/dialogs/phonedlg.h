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

#ifndef PHONEDLG_H
#define PHONEDLG_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;

namespace Licq
{
struct PhoneBookEntry;
}

namespace LicqQtGui
{
class EditPhoneDlg : public QDialog
{
  Q_OBJECT

public:
  EditPhoneDlg(QWidget* parent, const struct Licq::PhoneBookEntry* pbe = 0,
      int nEntry = -1);

signals:
  void updated(struct Licq::PhoneBookEntry&, int);

private:
  QCheckBox* cbRemove0s;
  QComboBox* cmbType;
  QComboBox* cmbDescription;
  QComboBox* cmbCountry;
  QComboBox* cmbProvider;
  QLineEdit* leAreaCode;
  QLineEdit* leNumber;
  QLineEdit* leExtension;
  QLineEdit* leGateway;
  int m_nEntry;

private slots:
  void ok();
  void UpdateDlg(int);
  void ProviderChanged(int);
};

} // namespace LicqQtGui

#endif
