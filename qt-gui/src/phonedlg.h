/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2006 Licq developers
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

#include <qdialog.h>

#include "licq_user.h"

class QCheckBox;

class EditPhoneDlg : public QDialog
{
  Q_OBJECT
public:
  EditPhoneDlg(QWidget *parent, const struct PhoneBookEntry *pbe = 0,
               int nEntry = -1);

protected:
  QCheckBox *cbRemove0s;
  QComboBox *cmbType, *cmbDescription, *cmbCountry, *cmbProvider;
  QLineEdit *leAreaCode, *leNumber, *leExtension, *leGateway;
  int m_nEntry;

signals:
  void updated(struct PhoneBookEntry, int);

protected slots:
  void ok();
  void UpdateDlg(int);
  void ProviderChanged(int);
};

#endif
