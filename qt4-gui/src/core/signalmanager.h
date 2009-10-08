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

#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>

#include <licq_types.h>

class QSocketNotifier;

class LicqSignal;
class LicqEvent;

namespace LicqQtGui
{

class SignalManager: public QObject
{
  Q_OBJECT

public:
  SignalManager(int pipe);
  ~SignalManager();

signals:
  /**
   * Contact list has changed
   *
   * @param subSignal Sub signal telling what the change was
   * @param argument Additional data, usage depend on sub signal type
   * @param userId Id for affected user, if applicable
   */
  void updatedList(unsigned long subSignal, int argument, const UserId& userId);

  /**
   * Data for a user has changed
   *
   * @param userId Id for affected user
   * @param subSignal Sub signal telling what the change was
   * @param argument Additional data, usage depend on sub signal type
   * @param cid Conversation id
   */
  void updatedUser(const UserId& userId, unsigned long subSignal, int argument, unsigned long cid);

  /**
   * Status has changed
   *
   * @param ppid Protocol instance id for owner that changed status
   */
  void updatedStatus(unsigned long ppid);

  void doneOwnerFcn(const LicqEvent* ev);
  void doneUserFcn(const LicqEvent* ev);
  void searchResult(const LicqEvent* ev);
  void logon();
  void logoff();

  /**
   * Open event dialog to show next event for a user
   * Triggered from fifo or other plugin
   *
   * @param userId User to show event for
   */
  void ui_viewevent(const UserId& userId);

  /**
   * Open a message dialog for a user
   * Triggered from fifo or other plugin
   *
   * @param userId User to open dialog for
   */
  void ui_message(const UserId& userId);
  void protocolPlugin(unsigned long);

  /**
   * A new event is ongoing for a user
   *
   * @param userId User event is sent for
   * @param eventTag Id of event
   */
  void eventTag(const UserId& userId, unsigned long eventTag);

  /**
   * A conversation id has been associated with a user
   *
   * @param userId User id to associate conversation with
   * @param convoId Conversation id
   */
  void socket(const UserId& userId, unsigned long convoId);

  /**
   * Someone joined an ongoing conversation
   *
   * @param userId User that joined conversation
   * @param ppid Protocol of conversation
   * @param convoId Id of conversation
   */
  void convoJoin(const UserId& userId, unsigned long ppid, unsigned long convoId);

  /**
   * Someone left an ongoing conversation
   *
   * @param userId User that left conversation
   * @param ppid Protocol of conversation
   * @param convoId Id of conversation
   */
  void convoLeave(const UserId& userId, unsigned long ppid, unsigned long convoId);
  void verifyImage(unsigned long);
  void newOwner(const QString& accountId, unsigned long ppid);

private:
  int myPipe;
  QSocketNotifier* sn;

  void ProcessSignal(LicqSignal* sig);
  void ProcessEvent(LicqEvent* ev);

private slots:
  void process();
};

} // namespace LicqQtGui

#endif
