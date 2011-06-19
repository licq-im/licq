/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2011 Licq developers
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

#ifndef LICQDAEMON_FILTER_H
#define LICQDAEMON_FILTER_H

#include <licq/filter.h>

#include <licq/thread/mutex.h>

namespace Licq
{
class User;
class UserEvent;
}

namespace LicqDaemon
{

class FilterManager : public Licq::FilterManager
{
public:
  FilterManager();
  ~FilterManager();

  /**
   * Initialize the filter manager
   */
  void initialize();

  /**
   * Check an event against the current set of rules
   *
   * @param user User or owner event originates from
   * @param event Event to test
   * @return Action to take (from FilterRule::ActionType)
   */
  int filterEvent(const Licq::User* user, const Licq::UserEvent* event);

  // From Licq::FilterManager
  Licq::FilterRules getRules();
  void setRules(const Licq::FilterRules& newRules);

private:
  /**
   * Save the current set of rules to file
   */
  void saveRules();


  Licq::FilterRules myRules;
  Licq::Mutex myDataMutex;
};

extern FilterManager gFilterManager;

} // namespace LicqDaemon

#endif
