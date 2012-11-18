/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_CONTACTLIST_GROUP_H
#define LICQDAEMON_CONTACTLIST_GROUP_H

#include <licq/contactlist/group.h>

#include <map>

namespace Licq
{
class IniFile;
}

namespace LicqDaemon
{
typedef std::map<Licq::UserId, unsigned long> ServerIdMap;

class Group : public Licq::Group
{
public:
  /**
   * Constructor, creates a new user group
   *
   * @param id Group id, must be unique
   * @param name Group name
   */
  Group(int id, const std::string& name);

  /**
   * Destructor
   */
  ~Group();

  /**
   * Save group to file
   *
   * @param file Open file to write group data to
   * @param num Number of group entry to write to file
   */
  void save(Licq::IniFile& file, int num) const;


  // From Licq::Group
  unsigned long serverId(const Licq::UserId& ownerId) const;
  void setServerId(const Licq::UserId& ownerId, unsigned long serverId);
  void unsetServerId(const Licq::UserId& ownerId);

private:
  ServerIdMap myServerIds;
};

/**
 * Helper function for sorting group list
 *
 * @param first Left hand group to compare
 * @param second Right hand group to compare
 * @return True if first has a lower sorting index than second
 */
bool compare_groups(const Licq::Group* first, const Licq::Group* second);


} // namespace LicqDaemon

#endif
