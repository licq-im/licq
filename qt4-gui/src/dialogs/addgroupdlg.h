/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010 Licq developers
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

#ifndef ADDGROUPDLG_H
#define ADDGROUPDLG_H

#include <QDialog>

class QLineEdit;

namespace LicqQtGui
{
class GroupComboBox;

class AddGroupDlg : public QDialog
{
   Q_OBJECT

public:
   AddGroupDlg(QWidget* parent = 0);

private:
   QLineEdit* myNameEdit;
   GroupComboBox* myPositionCombo;

private slots:
   void ok();
};

} // namespace LicqQtGui

#endif

