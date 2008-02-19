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

#ifndef SHOWAWAYMSGDLG_H
#define SHOWAWAYMSGDLG_H

#include <QDialog>

class QCheckBox;

class ICQEvent;

namespace LicqQtGui
{
class MLEdit;

class ShowAwayMsgDlg : public QDialog
{
  Q_OBJECT
public:
  ShowAwayMsgDlg(QString id, unsigned long ppid,
      bool fetch = false, QWidget* parent = 0);
  ~ShowAwayMsgDlg();

private:
  QString myId;
  unsigned long myPpid;
  unsigned long icqEventTag;
  MLEdit* mleAwayMsg;
  QCheckBox* chkShowAgain;

private slots:
  void doneEvent(ICQEvent* e);
};

} // namespace LicqQtGui

#endif
