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

#include <licq/inifile.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

#include <licq/daemon.h>
#include <licq/logging/log.h>

#include "gettext.h"

using namespace std;
using Licq::IniFile;


IniFile::IniFile(const string& filename)
  : myConfigData("\n"),
    mySectionStart(string::npos),
    mySectionEnd(string::npos),
    myIsModified(true)
{
  if (!filename.empty())
    setFilename(filename);
}

IniFile::~IniFile()
{
  // Empty
}

void IniFile::setFilename(const std::string& filename)
{
  myFilename = filename;

  // If filename has changed we most likely want to allow write
  myIsModified = true;
}

bool IniFile::loadFile()
{
  if (myFilename.empty())
    return false;

  string filename;
  if (myFilename.size() > 0 && myFilename[0] != '/')
    filename = Licq::gDaemon.baseDir() + myFilename;
  else
    filename = myFilename;

  int fd = open(filename.c_str(), O_RDONLY);
  if (fd < 0)
  {
    // Open failed. If it was just a missing file it might be normal and we'll let caller handle it
    if (errno != ENOENT)
    {
      // Failure was not just a missing file, this is probably serious
      gLog.error(tr("%sIniFile: I/O error, failed to open file.\nFile: %s\nError code: %i\n"),
          L_ERRORxSTR, filename.c_str(), errno);
    }

    return false;
  }

  struct stat st;
  if (fstat(fd, &st) != 0)
  {
    gLog.error(tr("%sIniFile: I/O error, failed to get file size.\nFile: %s\nError code: %i\n"),
        L_ERRORxSTR, filename.c_str(), errno);
    close(fd);
    return false;
  }

  // Read entire file at once
  char* buffer = new char[st.st_size + 1];
  ssize_t numRead = read(fd, buffer, st.st_size);

  if (numRead < 0)
  {
    gLog.error(tr("%sIniFile: I/O error, failed to read file.\nFile: %s\nError code: %i\n"),
        L_ERRORxSTR, filename.c_str(), errno);
    close(fd);
    delete[] buffer;
    return false;
  }

  close(fd);

  // Null terminate buffer
  buffer[numRead] = '\0';

  loadRawConfiguration(buffer);
  delete [] buffer;
  return true;
}

void IniFile::loadRawConfiguration(const string& rawConfig)
{
  // Prepend buffer with an extra newline so setSection can assume every section starts with a newline
  myConfigData = '\n' + rawConfig;

  // Make sure data ends with a newline
  if (myConfigData[myConfigData.size() - 1] != '\n')
    myConfigData += '\n';

  // Reset section pointers to mark that no section is selected
  mySectionStart = string::npos;
  mySectionEnd = string::npos;

  // Old configuration files had spaces around equal sign, drop them

  // First character is always a newline so first line starts on second character
  string::size_type lineStart = 1;
  while (true)
  {
    // Find next line
    string::size_type lineEnd = myConfigData.find('\n', lineStart);
    if (lineEnd == string::npos)
      // Last character is always a new line so no next line means we're done
      break;

    // Ignore section headers and comments
    if (myConfigData[lineStart] == '[' || myConfigData[lineStart] == '#' || myConfigData[lineStart] == ';')
    {
      lineStart = lineEnd + 1;
      continue;
    }

    // Find equal sign between name and value
    string::size_type pos = myConfigData.find('=', lineStart);

    if (pos != string::npos && pos < lineEnd && pos < myConfigData.size() - 1)
    {
      // Equal sign found, check for spaces
      if (myConfigData[pos-1] == ' ' && myConfigData[pos+1] == ' ')
      {
        // Spaces found, drop them
        myConfigData.replace(pos-1, 3, "=");
        lineEnd -= 2;
      }
    }

    lineStart = lineEnd + 1;
  }

  // TODO: Validate the config data so we can reject broken config files

  // We currently have no changes to write
  myIsModified = false;
}

bool IniFile::writeFile(bool allowCreate)
{
  if (myFilename.empty())
    return false;

  string filename;
  if (myFilename.size() > 0 && myFilename[0] != '/')
    filename = Licq::gDaemon.baseDir() + myFilename;
  else
    filename = myFilename;

  // If data hasn't been modified there is no point in generating disk I/O
  if (!myIsModified)
    return true;

  // First get stats for old file
  struct stat st;
  if (stat(filename.c_str(), &st) != 0)
  {
    if (errno != ENOENT)
    {
      // Failure was not just a missing file, this is probably serious
      gLog.error(tr("%sIniFile: I/O error, failed to get file size.\nFile: %s\nError code: %i\n"),
          L_ERRORxSTR, filename.c_str(), errno);
      return false;
    }

    // Let caller decide if a missing file is an error and how to report it
    if (!allowCreate)
      return false;

    // File is missing but we're allowed to create it
    st.st_mode = S_IRUSR | S_IWUSR;
  }

  string tempFile = filename + ".new";
  int fd = open(tempFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);

  if (fd < 0)
  {
    gLog.error(tr("%sIniFile: I/O error, failed to create file.\nFile: %s\nError code: %i\n"),
        L_ERRORxSTR, tempFile.c_str(), errno);
    return false;
  }

  // Write entire configuration but skip our initial newline
  ssize_t numWritten = write(fd, myConfigData.c_str() + 1, myConfigData.size() - 1);

  if (numWritten != static_cast<ssize_t>(myConfigData.size() - 1))
  {
    // Write failed, remove temp file
    gLog.error(tr("%sIniFile: I/O error, failed to write file.\nFile: %s\nError code: %i\n"),
        L_ERRORxSTR, tempFile.c_str(), errno);
    close(fd);
    unlink(tempFile.c_str());
    return false;
  }

  // Replace real file with temp file
  if (close(fd) != 0 || rename(tempFile.c_str(), filename.c_str()) != 0)
  {
    // Close or rename file failed, data might not have made it to disk
    gLog.error(tr("%sIniFile: I/O error, failed to replace file.\nFile: %s\nError code: %i\n"),
        L_ERRORxSTR, filename.c_str(), errno);
    unlink(tempFile.c_str());
    return false;
  }

  // Changes are written, mark data as unchanged
  myIsModified = false;
  return true;
}

string IniFile::getRawConfiguration() const
{
  // First character is our newline, the rest is the real config
  return myConfigData.substr(1);
}

bool IniFile::setSection(const string& rawSection, bool allowAdd)
{
  string section(rawSection);

  // Restrict characters allowed in section name
  string::size_type p = 0;
  while ((p = section.find_first_not_of("-.1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz", p)) != string::npos)
    section.erase(p, 1);
  if (section.empty())
  {
    mySectionStart = string::npos;
    mySectionEnd = string::npos;
    return false;
  }

  // Find section
  string::size_type pos = myConfigData.find("\n[" + section + "]");

  if (pos != string::npos)
  {
    // Section found, find start of next line (probably first parameter)
    pos = myConfigData.find('\n', pos + 1);
    if (pos != string::npos)
    {
      // mySectionStart points to first character on first line after section header
      mySectionStart = pos + 1;
    }
    else
    {
      // Line break missing after section, add it
      myConfigData.append("\n");
      mySectionStart = myConfigData.size();
    }

    // Find end of section (i.e. start of next section)
    pos = myConfigData.find("\n[", mySectionStart-1);
    if (pos == string::npos)
      mySectionEnd = myConfigData.size();
    else
      mySectionEnd = pos + 1;

    // Don't include empty lines between sections as part of this section
    while (mySectionEnd > mySectionStart && myConfigData[mySectionEnd-2] == '\n')
      --mySectionEnd;

    // TODO: Don't include comments between sections as part of this section

    // mySectionEnd points to first character of line after section end

    return true;
  }

  if (!allowAdd)
  {
    // Section not found and not allowed to create, fail
    mySectionStart = string::npos;
    mySectionEnd = string::npos;
    return false;
  }

  // Section not found, create it
  if (myConfigData.size() > 1 &&
      (myConfigData[myConfigData.size()-1] != '\n' || myConfigData[myConfigData.size()-2] != '\n'))
    // Make sure we get an extra space between each section
    myConfigData.append("\n");
  myConfigData.append("[" + section + "]\n");
  mySectionStart = myConfigData.size();
  mySectionEnd = myConfigData.size();

  // We've added a section, mark data as changed
  myIsModified = true;

  return true;
}

void IniFile::removeSection(const string& section)
{
  // Find section to remove
  if (!setSection(section, false))
    return;

  // find section header (should be line before mySectionStart)
  string::size_type pos = myConfigData.rfind("\n[", mySectionStart-2);
  if (pos == string::npos)
    // Shouldn't happen, just give up
    return;

  // Also remove empty lines at end of section
  while (mySectionEnd < myConfigData.size() && myConfigData[mySectionEnd] == '\n')
    mySectionEnd++;

  myConfigData.erase(pos+1, mySectionEnd-pos-1);

  // Data has changed
  myIsModified = true;

  // We no longer have a valid section selected
  mySectionStart = string::npos;
  mySectionEnd = string::npos;
}

void IniFile::getSections(list<string>& ret, const string& prefix) const
{
  string needle = "\n[" + prefix;
  string::size_type pos = 0;
  while ((pos = myConfigData.find(needle, pos)) != string::npos)
  {
    // Skip newline and left bracket
    pos += 2;

    // Find end of section name and make sure it ends on the same line
    string::size_type secend = myConfigData.find(']', pos);
    string::size_type nl = myConfigData.find('\n', pos);
    if (secend == string::npos || nl == string::npos || nl < secend)
      continue;

    // Section name is valid, return it
    ret.push_back(myConfigData.substr(pos, secend - pos));
  }
}

void IniFile::getKeyList(list<string>& ret, const string& prefix) const
{
  if (mySectionStart == string::npos)
    return;

  string::size_type lineStart = mySectionStart;
  string::size_type nextLine;
  for (lineStart = mySectionStart; lineStart < mySectionEnd; lineStart = nextLine)
  {
    // Find start of next line
    nextLine = myConfigData.find('\n', lineStart);
    if (nextLine == string::npos)
      nextLine = myConfigData.size();
    else
      nextLine += 1;

    // Ignore comments
    if (myConfigData[lineStart] == '#' || myConfigData[lineStart] == ';')
      continue;

    // Check prefix
    if (myConfigData.compare(lineStart, prefix.size(), prefix) != 0)
      continue;

    // Check for delimiter
    string::size_type equalPos = myConfigData.find('=', lineStart);
    if (equalPos != string::npos && equalPos < nextLine)
      // Delimiter found on same line, add key to list
      ret.push_back(myConfigData.substr(lineStart, equalPos - lineStart));
  }
}

bool IniFile::get(const string& key, string& data, const string& defValue) const
{
  if (mySectionStart == string::npos)
  {
    data = defValue;
    return false;
  }

  // Find parameter
  string::size_type pos = myConfigData.find('\n' + key + '=', mySectionStart-1);
  if (pos == string::npos || pos >= mySectionEnd)
  {
    // Parameter not found or not within this section
    data = defValue;
    return false;
  }

  string::size_type start = pos + key.size() + 2;

  // Find end of parameter value
  pos = myConfigData.find('\n', start);
  string::size_type len = (pos == string::npos ? string::npos : pos - start);
  data = myConfigData.substr(start, len);

  // Convert special characters
  pos = 0;
  while ( (pos = data.find_first_of('\\', pos)) != string::npos)
  {
    if (pos == data.size() - 1)
      break;

    char c = '\0';
    switch (data[pos+1])
    {
      case 'n': c = '\n'; break;
      case '\\': c = '\\'; break;
    }
    if (c != '\0')
      data.replace(pos, 2, 1, c);

    // Don't check the character we just converted
    ++pos;
  }

  return true;
}

bool IniFile::get(const string& key, unsigned long& data, unsigned long defValue) const
{
  string strData;
  if (!get(key, strData))
  {
    data = defValue;
    return false;
  }
  data = strtoul(strData.c_str(), (char**)NULL, 10);
  return true;
}

bool IniFile::get(const string& key, int& data, int defValue) const
{
  string strData;
  if (!get(key, strData))
  {
    data = defValue;
    return false;
  }
  data = atoi(strData.c_str());
  return true;
}

bool IniFile::get(const string& key, unsigned& data, unsigned defValue) const
{
  string strData;
  if (!get(key, strData))
  {
    data = defValue;
    return false;
  }
  data = strtoul(strData.c_str(), (char**)NULL, 10);
  return true;
}

bool IniFile::get(const string& key, bool& data, bool defValue) const
{
  string strData;
  if (!get(key, strData))
  {
    data = defValue;
    return false;
  }
  data = (atoi(strData.c_str()) == 0 ? false : true);
  return true;
}

bool IniFile::get(const string& key, boost::any& data) const
{
  if (data.type() == typeid(string))
    return get(key, boost::any_cast<string&>(data));
  if (data.type() == typeid(unsigned))
    return get(key, boost::any_cast<unsigned&>(data));
  if (data.type() == typeid(int))
    return get(key, boost::any_cast<int&>(data));
  if (data.type() == typeid(bool))
    return get(key, boost::any_cast<bool&>(data));

  // Unhandled data type
  gLog.warning(tr("%sInternal Error: IniFile::get, key=%s, data.type=%s\n"),
      L_WARNxSTR, key.c_str(), data.type().name());
  return false;
}

bool IniFile::set(const string& key, const string& data)
{
  if (mySectionStart == string::npos)
    return false;

  // Restrict characters allowed in parameter name
  if (key.find_first_not_of("-.1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz") != string::npos)
    return false;

  // Convert special characters
  string safeData = data;
  string::size_type pos = 0;
  while ( (pos = safeData.find_first_of("\n\\", pos)) != string::npos)
  {
    char c = '\0';
    switch (safeData[pos])
    {
      case '\n': c = 'n'; break;
      case '\\': c = '\\'; break;
    }
    if (c != '\0')
    {
      safeData.replace(pos, 1, string("\\") + c);
      ++pos;
    }

    // Don't check the character we just converted
    ++pos;
  }

  pos = myConfigData.find('\n' + key + '=', mySectionStart-1);
  if (pos != string::npos && pos < mySectionEnd)
  {
    // Parameter already exists, replace value
    string::size_type start = pos + key.size() + 2;
    pos = myConfigData.find('\n', start);
    string::size_type len = (pos == string::npos ? string::npos : pos - start);

    if (safeData == myConfigData.substr(start, len))
      // New data is same as old, no point in continuing
      return true;

    myConfigData.replace(start, len, safeData);
    mySectionEnd += safeData.size() - len;
  }
  else
  {
    // Parameter not found, add it at end of section
    myConfigData.insert(mySectionEnd, key + '=' + safeData + '\n');
    mySectionEnd += key.size() + safeData.size() + 2;
  }

  // Data has changed
  myIsModified = true;

  return true;
}

bool IniFile::set(const string& key, unsigned long data)
{
  char strData[32];
  snprintf(strData, sizeof(strData), "%lu", data);
  return set(key, string(strData));
}

bool IniFile::set(const string& key, int data)
{
  char strData[32];
  snprintf(strData, sizeof(strData), "%d", data);
  return set(key, string(strData));
}

bool IniFile::set(const string& key, unsigned data)
{
  char strData[32];
  snprintf(strData, sizeof(strData), "%u", data);
  return set(key, string(strData));
}

bool IniFile::set(const string& key, bool data)
{
  return set(key, string(data ? "1" : "0"));
}

bool IniFile::set(const string& key, const boost::any& data)
{
  if (data.type() == typeid(string))
    return set(key, boost::any_cast<const string&>(data));
  if (data.type() == typeid(unsigned))
    return set(key, boost::any_cast<unsigned>(data));
  if (data.type() == typeid(int))
    return set(key, boost::any_cast<int>(data));
  if (data.type() == typeid(bool))
    return set(key, boost::any_cast<bool>(data));

  gLog.warning(tr("%sInternal Error: IniFile::set, key=%s, data.type=%s\n"),
      L_WARNxSTR, key.c_str(), data.type().name());
  return false;
}
