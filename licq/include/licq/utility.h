/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
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

#ifndef LICQ_UTILITY_H
#define LICQ_UTILITY_H

#include <cstdio>
#include <string>
#include <vector>

namespace Licq
{
class User;
class UserId;


class UtilityInternalWindow
{
public:
  UtilityInternalWindow();
  ~UtilityInternalWindow();

  bool POpen(const std::string& command);
  int PClose();

  bool Running() { return fStdOut != NULL; }

  FILE *StdOut()  { return fStdOut; }
  FILE *StdErr()  { return fStdErr; }

protected:
  int pid;
  FILE *fStdOut;
  FILE *fStdErr;
};


class UtilityUserField
{
public:
  UtilityUserField(const std::string& title, const std::string& defaultValue);
  ~UtilityUserField();

  const std::string& title() const { return myTitle; }
  const std::string& defaultValue() const { return myDefaultValue; }
  const std::string& fullDefault() const { return myFullDefault; }
  bool setFields(const Licq::User* u);

protected:
  std::string myTitle;
  std::string myDefaultValue;
  std::string myFullDefault;
};


class Utility
{
public:
  enum WinType
  {
    WinLicq     = 1,
    WinTerm     = 2,
    WinGui      = 3,
  };

  Utility(const std::string& filename);
  ~Utility();

  const std::string& name() const { return myName; }
  const std::string& command() const { return myCommand; }
  const std::string& description() const { return myDescription; }
  WinType winType()  { return myWinType; }

  /**
   * Set user specific data for utility
   *
   * @param userId Id of user to populate fields from
   * @return True if user data was sucessfully read
   */
  bool setFields(const Licq::UserId& userId);

  void setUserFields(const std::vector<std::string>& userFields);
  void setBackgroundTask()  { myFullCommand += " &"; }
  const std::string& fullCommand() const { return myFullCommand; }

  int numUserFields() const { return myUserFields.size(); }
  const UtilityUserField* userField(int i) const { return myUserFields[i]; }

  bool isFailed() const { return myIsFailed; }

protected:
  std::string myName;
  std::string myDescription;
  WinType myWinType;
  std::string myCommand;
  std::string myFullCommand;
  std::vector<UtilityUserField*> myUserFields;
  bool myIsFailed;
};


class UtilityManager
{
public:
  UtilityManager();
  ~UtilityManager();

  int loadUtilities(const std::string& dir);
  Utility* utility(int n) { return myUtilities[n]; }
  int numUtilities() const { return myUtilities.size(); }

protected:
  std::vector<Utility*> myUtilities;
};

extern UtilityManager gUtilityManager;

} // namespace Licq

#endif
