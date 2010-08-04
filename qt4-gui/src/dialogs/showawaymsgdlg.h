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

#ifndef SHOWAWAYMSGDLG_H
#define SHOWAWAYMSGDLG_H

#include <QDialog>

#include <licq/userid.h>

class QCheckBox;

namespace Licq
{
class Event;
}

namespace LicqQtGui
{
class MLEdit;

class ShowAwayMsgDlg : public QDialog
{
  Q_OBJECT
public:
  /**
   * Constructor, create and show away response dialog
   *
   * @param userId User to show away response for
   * @param fetch True if away response should be fetch, false to use cached message
   * @param parent Parent widget
   */
  ShowAwayMsgDlg(const Licq::UserId& userId, bool fetch = false, QWidget* parent = 0);
  ~ShowAwayMsgDlg();

private:
  Licq::UserId myUserId;
  unsigned long icqEventTag;
  MLEdit* mleAwayMsg;
  QCheckBox* chkShowAgain;

private slots:
  void doneEvent(const Licq::Event* e);
};

} // namespace LicqQtGui

#endif
