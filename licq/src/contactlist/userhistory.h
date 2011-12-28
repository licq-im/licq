/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers <licq-dev@googlegroups.com>
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

#ifndef LICQDAEMON_CONTACTLIST_USERHISTORY_H
#define LICQDAEMON_CONTACTLIST_USERHISTORY_H

#include <string>

#include <licq/contactlist/user.h> // HistoryList

namespace LicqDaemon
{

class UserHistory
{
public:
  explicit UserHistory(unsigned long ppid);
  ~UserHistory();

  /**
   * Sets name of the history file
   * Note: Should not be called by plugins
   *
   * @param filename "default", "none" or a filename
   * @param userId User id to use if default is requested
   */
  void setFile(const std::string& filename, const std::string& description);

  /**
   * Read history from file
   * Call clear to free up the memory used by the history list
   *
   * @param history List to put all history entries in
   * @param userEncoding Default encoding to use if unknown
   * @return True if history was read
   */
  bool load(Licq::HistoryList& history, const std::string& userEncoding) const;

  /**
   * Frees up memory used by a history list
   *
   * @param history List with history events to free
   */
  static void clear(Licq::HistoryList& history);

  /**
   * Write to the history file, creating it if necessary
   *
   * @param buf String with data to write
   * @param append True to append data or false to overwrite file
   */
  void write(const std::string& buf, bool append);

  void append(const std::string& buf) { write(buf, true); }
  void save(const std::string& buf) { write(buf, false); }

  const std::string& description() const { return myDescription; }
  const std::string& filename() const { return myFilename; }

protected:
  unsigned long myPpid;
  std::string myFilename;
  std::string myDescription;
};

} // namespace Licq

#endif



