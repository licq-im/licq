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
  enum WinType { WinLicq, WinTerm, WinGui };

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
