/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#ifndef KEYREQUESTDLG_H
#define KEYREQUESTDLG_H

#include <QDialog>

class QLabel;
class QPushButton;

class ICQEvent;

namespace LicqQtGui
{
class KeyRequestDlg : public QDialog
{
  Q_OBJECT

public:
  KeyRequestDlg(QString id, unsigned long ppid, QWidget* parent = 0);
  ~KeyRequestDlg();

private:
  QString myId;
  unsigned long myPpid;
  unsigned long myIcqEventTag;
  bool myOpen;

  QPushButton* btnSend;
  QLabel* lblStatus;

private slots:
  void startSend();
  void doneEvent(ICQEvent* e);
  void openConnection();
  void closeConnection();
};

} // namespace LicqQtGui

#endif
