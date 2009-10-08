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

#ifndef ADDUSERDLG_H
#define ADDUSERDLG_H

#include <QDialog>

#include <licq_types.h>

class QCheckBox;
class QLineEdit;

namespace LicqQtGui
{
class GroupComboBox;
class ProtoComboBox;

class AddUserDlg : public QDialog
{
   Q_OBJECT

public:
   AddUserDlg(const UserId& userId = USERID_NONE, QWidget* parent = 0);

private:
   QCheckBox* myNotify;
   QLineEdit* myId;
   GroupComboBox* myGroup;
   ProtoComboBox* myProtocol;

private slots:
   void ok();
};

} // namespace LicqQtGui

#endif
