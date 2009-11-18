/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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

#ifndef MMSENDDLG_H
#define MMSENDDLG_H

#include "config.h"

#include <QDialog>
#include <QString>

#include <licq_icqd.h>

class QGroupBox;
class QProgressBar;
class QPushButton;

class LicqEvent;

namespace LicqQtGui
{
class MMUserView;

class MMSendDlg : public QDialog
{
  Q_OBJECT
public:
  MMSendDlg(MMUserView* _mmv, QWidget* parent = 0);
  ~MMSendDlg();

  int go_message(const QString& msg);
  int go_url(const QString& url, const QString& desc);
  int go_contact(StringList& users);

signals:
  /**
   * Since daemon doesn't notify us when an event is sent we'll have to handle
   * it ourselfs. This signal is sent to notify other windows about the event
   * that was sent.
   *
   * @param event Event object that was sent
   */
  void eventSent(const LicqEvent* event);

private:
  QString s1, s2;
  StringList* myUsers;

  unsigned long m_nEventType;
  QGroupBox* grpSending;
  QPushButton* btnCancel;
  QProgressBar* barSend;
  MMUserView* mmv;
  unsigned long icqEventTag;

  void SendNext();
private slots:
  void slot_done(const LicqEvent* event);
  void slot_cancel();
};

} // namespace LicqQtGui

#endif
