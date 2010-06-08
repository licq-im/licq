#ifndef LICQDAEMON_CONTACTLIST_USERHISTORY_H
#define LICQDAEMON_CONTACTLIST_USERHISTORY_H

#include <string>

#include <licq/contactlist/user.h> // HistoryList

class CUserEvent;

namespace LicqDaemon
{

class UserHistory
{
public:
  UserHistory();
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
   * @return True if history was read
   */
  bool load(Licq::HistoryList& history) const;

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
  std::string myFilename;
  std::string myDescription;
};

} // namespace Licq

#endif



