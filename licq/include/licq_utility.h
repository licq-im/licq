#ifndef UTILITY_H
#define UTILITY_H

#include <cstdio>
#include <vector>

class ICQUser;

// Define for marking functions as deprecated
#ifndef LICQ_DEPRECATED
# if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (__GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2))
#  define LICQ_DEPRECATED __attribute__ ((__deprecated__))
# elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#  define LICQ_DEPRECATED __declspec(deprecated)
# else
#  define LICQ_DEPRECATED
# endif
#endif


//=====CUtilityInternalWindow================================================
class CUtilityInternalWindow
{
public:
  CUtilityInternalWindow();
  ~CUtilityInternalWindow();

  bool POpen(const char *cmd);
  int PClose();

  bool Running() { return fStdOut != NULL; }

  FILE *StdOut()  { return fStdOut; }
  FILE *StdErr()  { return fStdErr; }
protected:
  int pid;
  FILE *fStdOut;
  FILE *fStdErr;
};


//=====CUtilityUserField=========================================================
class CUtilityUserField
{
public:
  CUtilityUserField(const char *_szTitle, const char *_szDefault);
  ~CUtilityUserField();

  const char *Title()  { return m_szTitle; }
  const char *Default()  { return m_szDefault; }
  const char *FullDefault()  { return m_szFullDefault; }
  bool SetFields(const ICQUser* u);
protected:
  char *m_szTitle;
  char *m_szDefault;
  char *m_szFullDefault;
};


//=====CUtility==================================================================
enum EWinType { UtilityWinLicq, UtilityWinTerm, UtilityWinGui };

class CUtility
{
public:
  CUtility(const char *_szFileName);
  ~CUtility();

  const char *Name()  { return m_szName; }
  const char *Command(ICQUser *) { return m_szCommand; }
  const char *Description()  { return m_szDescription; }
  EWinType WinType()  { return m_eWinType; }

  bool SetFields(const char *szId, unsigned long nPPID);
  void SetUserFields(const std::vector<const char *> &_vszUserFields);
  void SetBackgroundTask()  { strcat(m_szFullCommand, " &"); }
  const char *FullCommand() { return m_szFullCommand; }

  unsigned short NumUserFields()  { return m_vxUserField.size(); }
  CUtilityUserField *UserField(unsigned short i)  { return m_vxUserField[i]; }

  bool Exception()  { return bException; }

  // Deprecated functions, to be removed
  LICQ_DEPRECATED bool SetFields(unsigned long _nUin);

protected:
  char *m_szName;
  char *m_szDescription;
  EWinType m_eWinType;
  char *m_szCommand;
  char *m_szFullCommand;
  std::vector <CUtilityUserField *> m_vxUserField;
  bool bException;
};


//=====CUtilityManager===========================================================
class CUtilityManager
{
public:
  CUtilityManager();
  ~CUtilityManager();

  unsigned short LoadUtilities(const char *_szDir);
  CUtility *Utility(unsigned short n)  { return m_vxUtilities[n]; }
  unsigned short NumUtilities()  { return m_vxUtilities.size(); }
protected:
  std::vector <CUtility *> m_vxUtilities;
};

extern CUtilityManager gUtilityManager;


#endif
