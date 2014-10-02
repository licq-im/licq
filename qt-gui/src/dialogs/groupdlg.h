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

#ifndef GROUPDLG_H
#define GROUPDLG_H

#include <QDialog>

namespace LicqQtGui
{
class OnEventBox;

class GroupDlg : public QDialog
{
  Q_OBJECT

public:
  /**
   * Constructor
   *
   * @param groupId Group id
   */
  GroupDlg(int groupId, QWidget* parent = NULL);

private slots:
  /**
   * Save settings and close dialog
   */
  void ok();

  /**
   * Save settings
   */
  void apply();

private:
  int myGroupId;
  OnEventBox* myOnEventBox;
};

} // namespace LicqQtGui

#endif
