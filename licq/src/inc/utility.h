#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "user.h"
#include "icq-defines.h"

//=====CUtilityUserField=========================================================
class CUtilityUserField
{
public:
  CUtilityUserField(const char *_szTitle, const char *_szDefault)
  {
    m_szTitle = strdup(_szTitle);
    m_szDefault = strdup(_szDefault);
  }
  ~CUtilityUserField(void)
  {
    free (m_szTitle);
    free (m_szDefault);
  }
  const char *Title(void)  { return m_szTitle; }
  const char *Default(void)  { return m_szDefault; }
  const char *FullDefault(void)  { return m_szFullDefault; }
  bool SetFields(ICQUser *);
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
  const char *Name(void)  { return m_szName; }
  const char *Command(ICQUser *u) { return m_szCommand; }
  const char *Description(void)  { return m_szDescription; }
  EWinType WinType(void)  { return m_eWinType; }

  bool SetFields(unsigned long _nUin);
  void SetUserFields(vector<const char *> &_vszUserFields);
  void SetBackgroundTask(void)  { strcat(m_szFullCommand, " &"); }
  const char *FullCommand(void) { return m_szFullCommand; }

  unsigned short NumUserFields(void)  { return m_vxUserField.size(); }
  CUtilityUserField *UserField(unsigned short i)  { return m_vxUserField[i]; }

  bool Exception(void)  { return bException; }

protected:
  char *m_szName;
  char *m_szDescription;
  EWinType m_eWinType;
  char *m_szCommand;
  char *m_szFullCommand;
  vector <CUtilityUserField *> m_vxUserField;
  bool bException;
};


//=====CUtilityManager===========================================================
class CUtilityManager
{
public:
  CUtilityManager(void);
  unsigned short LoadUtilities(const char *_szDir);
  CUtility *Utility(unsigned short n)  { return m_vxUtilities[n]; }
  unsigned short NumUtilities(void)  { return m_vxUtilities.size(); }
protected:
  vector <CUtility *> m_vxUtilities;
};

extern CUtilityManager gUtilityManager;


#endif
