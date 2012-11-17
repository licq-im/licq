/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2010-2012 Licq Developers <licq-dev@googlegroups.com>
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

#ifndef LICQ_INIFILE_H
#define LICQ_INIFILE_H

#include <ctime>
#include <list>
#include <string>
#include <boost/any.hpp>

namespace Licq
{

/**
 * This class provides access to ini style configuration files
 */
class IniFile
{
public:
  /**
   * Constructor
   *
   * @param filename A filename, either absolute path or relative to base dir
   */
  IniFile(const std::string& filename = "");

  /**
   * Set filename
   *
   * @param filename A filename, either absolute path or relative to base dir
   */
  void setFilename(const std::string& filename);

  /**
   * Get filename
   *
   * @return Filename
   */
  const std::string& filename() const
  { return myFilename; }

  /**
   * Destructor
   */
  virtual ~IniFile();

  /**
   * Load configuration from file
   * If load fails, existing config is unchanged
   *
   * @return True if file was successfully read
   */
  bool loadFile();

  /**
   * Load configuration from a string
   * This function can be used to load a default configuration all at once
   *
   * @param rawConfig Complete configuration, as it would appear in the file
   */
  void loadRawConfiguration(const std::string& rawConfig);

  /**
   * Write current data to file
   *
   * @param allowCreate True to create file if missing
   * @return True if file was successfully written
   */
  bool writeFile(bool allowCreate = true);

  /**
   * Get the entire configuration as a string
   *
   * @return Complete configuration as it would appear in the file
   */
  std::string getRawConfiguration() const;

  /**
   * Set active section to use when getting and setting parameters
   *
   * @param section Name of section
   * @param allowCreate True if section should be created if it's missing
   * @return True if section was found or created
   */
  bool setSection(const std::string& section, bool allowAdd = true);

  /**
   * Remove a section, including all values
   *
   * @param section Name of section to remove
   */
  void removeSection(const std::string& section);

  /**
   * Get a list of sections
   *
   * @param ret List to return section names in
   * @param prefix Prefix that sections must match to be returned
   */
  void getSections(std::list<std::string>& ret, const std::string& prefix = "") const;

  /**
   * Get a list of keys from the current section
   *
   * @param ret List to return keys in
   * @param prefix Prefix that keys must match to be returned
   */
  void getKeyList(std::list<std::string>& ret, const std::string& prefix = "") const;

  /**
   * Get a string value from the configuration
   *
   * @param key Key of value to get
   * @param data String to put value in
   * @param defValue Default value to set if key doesn't exist
   * @return True if value was found or false if default value was used
   */
  bool get(const std::string& key, std::string& data,
      const std::string& defValue = "") const;

  /**
   * Get an unsigned long value from the configuration
   *
   * @param key Key of value to get
   * @param data Integer to put value in
   * @param defValue Default value to set if key doesn't exist
   * @return True if value was found or false if default value was used
   */
  bool get(const std::string& key, unsigned long& data, unsigned long defValue = 0) const;

  /**
   * Get a signed value from the configuration
   *
   * @param key Key of value to get
   * @param data Integer to put value in
   * @param defValue Default value to set if key doesn't exist
   * @return True if value was found or false if default value was used
   */
  bool get(const std::string& key, int& data, int defValue = 0) const;

  /**
   * Get an unsigned value from the configuration
   *
   * @param key Key of value to get
   * @param data Integer to put value in
   * @param defValue Default value to set if key doesn't exist
   * @return True if value was found or false if default value was used
   */
  bool get(const std::string& key, unsigned& data, unsigned defValue = 0) const;

  /**
   * Get a boolean value from the configuration
   *
   * @param key Key of value to get
   * @param data Boolean to put value in
   * @param defValue Default value to set if key doesn't exist
   * @return True if value was found or false if default value was used
   */
  bool get(const std::string& key, bool& data, bool defValue = false) const;

  /**
   * Get a value from the configuration
   *
   * @param key Key of value to read
   * @param data variable to put value in
   * @return True if value was found and read
   */
  bool get(const std::string& key, boost::any& data) const;

  /**
   * Get a hex encoded value from the configuration
   *
   * @param key Key of the value to read
   * @param data Raw data converted from hex
   * @param defValue Default value to set if key doesn't exist
   * @return True if value was found or false if default value was used
   */
  bool getHex(const std::string& key, std::string& data,
      const std::string& defValue = "") const;

  /**
   * Set a string value in the configuration
   *
   * @param key Key of value to set
   * @param data Value to set
   * @return True if value was set, false on error
   */
  bool set(const std::string& key, const std::string& data);

  /**
   * Set a string value in the configuration
   * Convenience function to make sure char pointers are used as string rather
   * than being cast to int.
   *
   * @param key Key of value to set
   * @param data Value to set
   * @return True if value was set, false on error
   */
  bool set(const char* key, const char* data)
  { return set(key, std::string(data)); }

  /**
   * Set an unsigned long value in the configuration
   *
   * @param key Key of value to set
   * @param data Value to set
   * @return True if value was set, false on error
   */
  bool set(const std::string& key, unsigned long data);

  /**
   * Set a signed value in the configuration
   *
   * @param key Key of value to set
   * @param data Value to set
   * @return True if value was set, false on error
   */
  bool set(const std::string& key, int data);

  /**
   * Set an unsigned value in the configuration
   *
   * @param key Key of value to set
   * @param data Value to set
   * @return True if value was set, false on error
   */
  bool set(const std::string& key, unsigned data);

  /**
   * Set a boolean value in the configuration
   *
   * @param key Key of value to set
   * @param data Value to set
   * @return True if value was set, false on error
   */
  bool set(const std::string& key, bool data);

  /**
   * Set a value in the configuration
   *
   * @param key Key of value to set
   * @param data Value to set
   * @return True if value was set, false on error
   */
  bool set(const std::string& key, const boost::any& data);

  /**
   * Set a hex encoded value in the configuration
   *
   * @param key Key of value to set
   * @param data Raw data to convert to hex and set
   * @return True if value was set, false on error
   */
  bool setHex(const std::string& key, const std::string& data);

  /**
   * Remove a value from the configuration
   *
   * @param key Key of value to remove
   * @return True if key existed
   */
  bool unset(const std::string& key);

private:
  std::string myConfigData;
  std::string::size_type mySectionStart;
  std::string::size_type mySectionEnd;
  std::string myFilename;
  bool myIsModified;
  time_t myLastTimestamp;
};

} // namespace Licq

#endif
