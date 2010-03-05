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

#ifndef LICQ_SARMANAGER_H
#define LICQ_SARMANAGER_H

#include <boost/noncopyable.hpp>
#include <vector>
#include <string>

namespace Licq
{

struct SavedAutoResponse
{
  std::string name;
  std::string text;
};

typedef std::vector<SavedAutoResponse> SarList;

class SarManager : private boost::noncopyable
{
public:
  enum List
  {
    AwayList = 0,
    NotAvailableList = 1,
    OccupiedList= 2,
    DoNotDisturbList = 3,
    FreeForChatList = 4,
  };

  static const int NumLists = 5;

  /**
   * Get an auto response list
   * This call will also lock the list to stop other threads from accessing it
   *
   * @param list Which auto response list to get
   * @return Reference to the list
   */
  virtual SarList& getList(List list) = 0;

  /**
   * Release a previously locked list
   *
   * @param save True if list was changed and needs to be saved to disk
   */
  virtual void releaseList(bool save = false) = 0;

protected:
  virtual ~SarManager() { /* Empty */ }
};

extern SarManager& gSarManager;

} // namespace Licq

#endif
