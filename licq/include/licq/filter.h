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

#ifndef LICQ_FILTER_H
#define LICQ_FILTER_H

#include <boost/noncopyable.hpp>
#include <list>
#include <string>

namespace Licq
{

/**
 * Data for a filter rule
 */
struct FilterRule
{
  // Rule will be ignore unless this is set
  bool isEnabled;

  // Protocol id to use rule for or zero for all protocols
  unsigned long protocolId;

  // Event types to apply this rule for
  unsigned long eventMask;

  // Regular expression to match with
  std::string expression;

  // Action to perform if rule matches
  enum ActionType
  {
    ActionAccept        = 1,
    ActionSilent        = 2,
    ActionIgnore        = 3,
  };
  int action;
};

typedef std::list<FilterRule> FilterRules;

/**
 * Manager for event filter
 *
 * Incoming events are matched againts the list of filter rules before being
 * handled by Licq or forwarded to any plugins.
 * Events from contacts in list are always accepted
 * The first rule that matches is used
 * If no rule matches, the default action is to accept all events
 */
class FilterManager : private boost::noncopyable
{
public:
  /**
   * Get the list of filter rules
   *
   * @return The currently active filter rules
   */
  virtual FilterRules getRules() = 0;

  /**
   * Set filter rules
   *
   * @param newRules List of filter rules to make active
   */
  virtual void setRules(const FilterRules& newRules) = 0;

protected:
  virtual ~FilterManager() { /* Empty */ }
};

extern FilterManager& gFilterManager;

} // namespace Licq

#endif
