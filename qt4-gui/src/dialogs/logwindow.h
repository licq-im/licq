/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QDialog>

#include <licq/logging/pluginlogsink.h>

class QShowEvent;
class QSocketNotifier;

namespace LicqQtGui
{
class MLEdit;

class LogWindow : public QDialog
{
  Q_OBJECT

public:
  LogWindow(QWidget* parent = 0);
  ~LogWindow();

  Licq::PluginLogSink::Ptr pluginLogSink() { return myLogSink; }

private:
  MLEdit* outputBox;
  QSocketNotifier* sn;
  Licq::PluginLogSink::Ptr myLogSink;

private slots:
  void log(int fd);
  void save();
};

} // namespace LicqQtGui

#endif
