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

#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>

class QSocketNotifier;

class CICQSignal;
class ICQEvent;

namespace LicqQtGui
{

class SignalManager: public QObject
{
  Q_OBJECT

public:
  SignalManager(int pipe);
  ~SignalManager();

signals:
  void updatedList(CICQSignal* sig);
  void updatedUser(CICQSignal* sig);
  void updatedStatus(CICQSignal* sig);
  void doneOwnerFcn(ICQEvent* ev);
  void doneUserFcn(ICQEvent* ev);
  void searchResult(ICQEvent* ev);
  void logon();
  void logoff();
  void ui_viewevent(QString);
  void ui_message(QString, unsigned long);
  void protocolPlugin(unsigned long);
  void eventTag(QString, unsigned long, unsigned long);
  void socket(QString, unsigned long, unsigned long);
  void convoJoin(QString, unsigned long, unsigned long);
  void convoLeave(QString, unsigned long, unsigned long);
  void verifyImage(unsigned long);
  void newOwner(QString, unsigned long);

private:
  int myPipe;
  QSocketNotifier* sn;

  void ProcessSignal(CICQSignal* sig);
  void ProcessEvent(ICQEvent* ev);

private slots:
  void process();
};

} // namespace LicqQtGui

#endif
