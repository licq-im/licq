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

#ifndef LICQDAEMON_ONEVENTMANAGER_H
#define LICQDAEMON_ONEVENTMANAGER_H

#include <licq/oneventmanager.h>

#include <licq/thread/mutex.h>

namespace LicqDaemon
{

class OnEventManager : public Licq::OnEventManager
{
public:
  OnEventManager();
  ~OnEventManager();

  /**
   * Initialize the on event manager
   */
  void initialize();

  // From Licq::OnEventManager
  void lock();
  void unlock(bool save = false);
  bool enabled() const;
  void setEnabled(bool enabled);
  bool alwaysOnlineNotify() const;
  void setAlwaysOnlineNotify(bool alwaysOnlineNotify);
  std::string command() const;
  void setCommand(const std::string& command);
  std::string parameter(OnEventType event) const;
  void setParameter(OnEventType event, const std::string& parameter);
  void performOnEvent(OnEventType event, const Licq::User* user);

private:
  bool myEnabled;
  std::string myCommand;
  std::string myParameters[NumOnEventTypes];
  bool myAlwaysOnlineNotify;
  Licq::Mutex myMutex;
};

extern OnEventManager gOnEventManager;

} // namespace Licq

#endif
