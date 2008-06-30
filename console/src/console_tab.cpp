#include "console.h"

#include <ctype.h>


int StrMatchLen(const char *_sz1, const char *_sz2, unsigned short _nStartPos)
{
  int n = _nStartPos;
  while (_sz1[n] != '\0' && _sz2[n] != '\0'
         && tolower(_sz1[n]) == tolower(_sz2[n]))
    n++;
  return n;
}



/*---------------------------------------------------------------------------
 * CLicqConsole::TabCommand
 *-------------------------------------------------------------------------*/
void CLicqConsole::TabCommand(char *_szPartialMatch,
                              struct STabCompletion &_sTabCompletion)
{
  char *szMatch = 0;
  unsigned short nLen = strlen(_szPartialMatch);
  for (unsigned short i = 0; i < NUM_COMMANDS; i++)
  {
    char szTempCmd[20];
    snprintf(szTempCmd, 20, "%c%s", m_szCommandChar[0], aCommands[i].szName);
    if (strncasecmp(_szPartialMatch, szTempCmd, nLen) == 0)
    {
      if (szMatch == 0)
        szMatch = strdup(szTempCmd);
      else
        szMatch[StrMatchLen(szMatch, szTempCmd, nLen)] = '\0';
      _sTabCompletion.vszPartialMatch.push_back(strdup(szTempCmd));
    }
  }
  if (nLen == 0)
  {
    free(szMatch);
    _sTabCompletion.szPartialMatch = strdup("");
  }
  else
    _sTabCompletion.szPartialMatch = szMatch;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::TabUser
 *-------------------------------------------------------------------------*/
void CLicqConsole::TabUser(char *_szPartialMatch,
                           struct STabCompletion &_sTabCompletion)
{
  char *szMatch = 0;
  unsigned short nLen;
/*  char *szSubCmd = NULL;

  if (_szPartialMatch[0] == '"')
  {
    _szPartialMatch++;
    szSubCmd = strchr(_szPartialMatch, '"');
    if (szSubCmd != NULL) szSubCmd++;
  }
  else
  {
    szSubCmd = strchr(_szPartialMatch, ' ');
  }

  if (szSubCmd == NULL)*/
  {
    nLen = strlen(_szPartialMatch);
    FOR_EACH_USER_START(LOCK_R)
    {
      // Ignored users and users not in the current group are unwanted
      if ((!pUser->GetInGroup(m_nGroupType, m_nCurrentGroup) &&
          (m_nGroupType != GROUPS_USER || m_nCurrentGroup != 0)) ||
          (pUser->IgnoreList() && m_nGroupType != GROUPS_SYSTEM && m_nCurrentGroup != GROUP_IGNORE_LIST) )
        FOR_EACH_USER_CONTINUE

      if (nLen == 0 || strncasecmp(_szPartialMatch, pUser->GetAlias(), nLen) == 0)
      {
        if (szMatch == 0)
          szMatch = strdup(pUser->GetAlias());
        else
          szMatch[StrMatchLen(szMatch, pUser->GetAlias(), nLen)] = '\0';
        _sTabCompletion.vszPartialMatch.push_back(strdup(pUser->GetAlias()));
      }
      else if (strncasecmp(_szPartialMatch, pUser->IdString(), nLen) == 0)
      {
        if (szMatch == 0)
          szMatch = strdup(pUser->IdString());
        else
          szMatch[StrMatchLen(szMatch, pUser->IdString(), nLen)] = '\0';
        _sTabCompletion.vszPartialMatch.push_back(strdup(pUser->IdString()));
      }
    }
    FOR_EACH_USER_END

    if (nLen == 0)
    {
      free(szMatch);
      _sTabCompletion.szPartialMatch = strdup("");
    }
    else
      _sTabCompletion.szPartialMatch = szMatch;

  }
/*  else // Sub command time
  {
    // Remove any leading spaces
    while(isspace(*szSubCmd) && szSubCmd[0] != '\0') szSubCmd++;
    nLen = strlen(szSubCmd);
    for (unsigned short i = 0; i < NUM_USER_COMMANDS; i++)
    {
      if (strncasecmp(szSubCmd, aUserCommands[i].szName, nLen) == 0)
      {
        if (szMatch[0] == '\0')
          strcpy(szMatch, aUserCommands[i].szName);
        else
          szMatch[StrMatchLen(szMatch, aUserCommands[i].szName, nLen)] = '\0';
        _sTabCompletion.vszPartialMatch.push_back(strdup(aUserCommands[i].szName));
      }
    }
    if (nLen == 0)
      _sTabCompletion.szPartialMatch[0] = '\0';
    else
      sprintf(_sTabCompletion.szPartialMatch, "%s%s", _szPartialMatch, &szMatch[nLen]);
  }*/
}




/*---------------------------------------------------------------------------
 * CLicqConsole::TabStatus
 *-------------------------------------------------------------------------*/
void CLicqConsole::TabStatus(char *_szPartialMatch,
                             struct STabCompletion &_sTabCompletion)
{
  char *szMatch = 0;
  unsigned short nLen = strlen(_szPartialMatch);
  for (unsigned short i = 0; i < NUM_STATUS; i++)
  {
    if (strncasecmp(_szPartialMatch, aStatus[i].szName, nLen) == 0)
    {
      if (szMatch == 0)
        szMatch = strdup(aStatus[i].szName);
      else
        szMatch[StrMatchLen(szMatch, aStatus[i].szName, nLen)] = '\0';
      _sTabCompletion.vszPartialMatch.push_back(strdup(aStatus[i].szName));
    }
  }
  if (nLen == 0)
  {
    free(szMatch);
    _sTabCompletion.szPartialMatch = strdup("");
  }
  else
    _sTabCompletion.szPartialMatch = szMatch;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::TabSet
 *-------------------------------------------------------------------------*/
void CLicqConsole::TabSet(char *_szPartialMatch,
                          struct STabCompletion &_sTabCompletion)
{
  char *szMatch = 0;
  unsigned short nLen = strlen(_szPartialMatch);
  for (unsigned short i = 0; i < NUM_VARIABLES; i++)
  {
    if (strncasecmp(_szPartialMatch, aVariables[i].szName, nLen) == 0)
    {
      if (szMatch == 0)
        szMatch = strdup(aVariables[i].szName);
      else
        szMatch[StrMatchLen(szMatch, aVariables[i].szName, nLen)] = '\0';
      _sTabCompletion.vszPartialMatch.push_back(strdup(aVariables[i].szName));
    }
  }
  if (nLen == 0)
  {
    free(szMatch);
    _sTabCompletion.szPartialMatch = strdup("");
  }
  else
    _sTabCompletion.szPartialMatch = szMatch;
}

