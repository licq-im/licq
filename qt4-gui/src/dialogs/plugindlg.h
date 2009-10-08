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

#ifndef PLUGINDLG_H
#define PLUGINDLG_H

#include <QMap>
#include <QDialog>

class QTableWidget;
class QTableWidgetItem;

namespace LicqQtGui
{
class PluginDlg : public QDialog
{
  Q_OBJECT

public:
  /**
   * Create and show plugin dialog or raise it if already exists
   */
  static void showPluginDlg();

private:
  static PluginDlg* myInstance;

  PluginDlg();
  virtual ~PluginDlg();

  QTableWidget* tblStandard;
  QTableWidget* tblProtocol;

  QMap<QTableWidgetItem*,bool> mapCheckboxCache;

private slots:
  void slot_standard(QTableWidgetItem* item);
  void slot_stdConfig(int, int);
  void slot_protocol(QTableWidgetItem* item);
  void slot_refresh();
};

} // namespace LicqQtGui

#endif
