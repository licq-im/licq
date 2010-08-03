/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#include "console.h"

#include <boost/foreach.hpp>
#include <ctype.h>

#include <licq/contactlist/usermanager.h>


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
    snprintf(szTempCmd, 20, "%c%s", myCommandChar[0], aCommands[i].szName);
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

    Licq::UserListGuard users;
    BOOST_FOREACH(const Licq::User* pUser, **users)
    {
      // Ignored users and users not in the current group are unwanted
      if ((!userIsInGroup(pUser, myCurrentGroup) && myCurrentGroup != AllUsersGroupId) ||
          (pUser->IgnoreList() && myCurrentGroup != IgnoreListGroupId) )
        continue;

      if (nLen == 0 || strncasecmp(_szPartialMatch, pUser->GetAlias(), nLen) == 0)
      {
        if (szMatch == 0)
          szMatch = strdup(pUser->GetAlias());
        else
          szMatch[StrMatchLen(szMatch, pUser->GetAlias(), nLen)] = '\0';
        _sTabCompletion.vszPartialMatch.push_back(strdup(pUser->GetAlias()));
      }
      else if (strncasecmp(_szPartialMatch, pUser->accountId().c_str(), nLen) == 0)
      {
        if (szMatch == 0)
          szMatch = strdup(pUser->accountId().c_str());
        else
          szMatch[StrMatchLen(szMatch, pUser->accountId().c_str(), nLen)] = '\0';
        _sTabCompletion.vszPartialMatch.push_back(strdup(pUser->accountId().c_str()));
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

