/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#include <qwidget.h>


class QTable;
class CICQDaemon;

class PluginDlg : public QWidget
{
  Q_OBJECT
public:
  PluginDlg();
  virtual ~PluginDlg();

protected:
  QTable *tblStandard;
  QTable *tblProtocol;

signals:
  void signal_done();
  void pluginUnloaded(unsigned long);

protected slots:
  void slot_standard(int, int);
  void slot_stdConfig(int, int, int, const QPoint &);
  void slot_protocol(int, int);
  void slot_details();
  void slot_refresh();
};

#endif
