#include "console.h"

#include <ctype.h>

const unsigned short NUM_COMMANDS = 8;
const struct SCommand aCommands[NUM_COMMANDS] =
{
  { "/contacts", &CLicqConsole::MenuContactList, NULL,
    "" },
  { "/group", &CLicqConsole::MenuGroup, NULL,
    "Prints the group list or changes to the given group number." },
  { "/user", &CLicqConsole::MenuUser, &CLicqConsole::TabUser,
    "User commands deal with indiviual users." },
  { "/status", &CLicqConsole::MenuStatus, &CLicqConsole::TabStatus,
    "Set your status, prefix with \"*\" for invisible mode." },
  { "/set", &CLicqConsole::MenuSet, &CLicqConsole::TabSet,
    "Allows the setting and viewing of options.  With no arguments\n"
    "will print all current set'able values.  With one argument will\n"
    "print the value of the given argument.\n"
    "A boolean value can be yes/true/on or no/false/off.\n"
    "Color values can be red/blue/green/magenta/white/yellow or\n"
    "bright_<color> for bright colors." },
  { "/plugins", &CLicqConsole::MenuPlugins, NULL,
    "List the currently loaded plugins." },
  { "/help", &CLicqConsole::MenuHelp, NULL,
    "This help screen, can also be passed a command for detailed\n"
    "information about it." },
  { "/quit", &CLicqConsole::MenuQuit, NULL,
    "Quit Licq." }
};

const unsigned short NUM_USER_COMMANDS = 4;
const struct SUserCommand aUserCommands[NUM_USER_COMMANDS] =
{
  { "info", &CLicqConsole::UserCommand_Info },
  { "view", &CLicqConsole::UserCommand_View },
  { "message", &CLicqConsole::UserCommand_Msg },
  { "url", &CLicqConsole::UserCommand_Url }
};

#define STRIP(x) while(isspace(*(x)) && *(x) != '\0') (x)++;

/*---------------------------------------------------------------------------
 * CLicqConsole::MenuHelp
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuHelp(char *_szArg)
{
  if (_szArg == NULL)
  {
    PrintHelp();
    return;
  }

  // Print help on a specific topic
  unsigned short i;
  for (i = 0; i < NUM_COMMANDS; i++)
  {
    if (strncasecmp(_szArg, &aCommands[i].szName[1], strlen(_szArg)) == 0)
      break;
  }
  if (i == NUM_COMMANDS)
  {
    PrintBadInput(_szArg);
    return;
  }
  winMain->wprintf("Help on %s:\n%s\n", aCommands[i].szName,
                   aCommands[i].szHelp);

}



/*---------------------------------------------------------------------------
 * CLicqConsole::MenuQuit
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuQuit(char *)
{
  licqDaemon->Shutdown();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuPlugins
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuPlugins(char *_szArg)
{
  PluginsList l;
  PluginsListIter it;
  licqDaemon->PluginList(l);
  PrintBoxTop("Plugins", COLOR_BLUE, 70);
  for (it = l.begin(); it != l.end(); it++)
  {
    PrintBoxLeft();
    winMain->wprintf("[%3d] %s v%s (%s)", (*it)->Id(), (*it)->Name(),
                     (*it)->Version(), (*it)->Status());
    PrintBoxRight(70);
  }
  PrintBoxBottom(70);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuGroup
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuGroup(char *_szArg)
{
  if (_szArg == NULL)
  {
    PrintGroups();
    return;
  }

  // Try to change groups
  int nCurrentGroup = atol(_szArg);
  if (nCurrentGroup > gUserManager.NumGroups())
  {
    winMain->wprintf("%CInvalid group number (0 - %d)\n", COLOR_RED,
                     gUserManager.NumGroups());
  }
  else
  {
    m_nCurrentGroup = nCurrentGroup;
    GroupList *g = gUserManager.LockGroupList(LOCK_R);
    winMain->wprintf("Switching to group %d (%s).\n", m_nCurrentGroup,
                     (*g)[m_nCurrentGroup - 1]);
    gUserManager.UnlockGroupList();
    PrintStatus();
  }
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuStatus
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuStatus(char *_szArg)
{
  unsigned short nStatus = ICQ_STATUS_ONLINE, i;
  bool bInvisible = false;

  if (_szArg == NULL)
  {
    winMain->wprintf("%CSpecify status.\n", COLOR_RED);
    return;
  }

  // Check if we are going invisible or not
  if (_szArg[0] == '*')
  {
    bInvisible = true;
    //_szArg++;
  }
  // Find the status
  for (i = 0; i < NUM_STATUS; i++)
  {
    if (strcasecmp(_szArg, aStatus[i].szName) == 0)
    {
      nStatus = aStatus[i].nId;
      break;
    }
  }
  // Check that we found it
  if (i == NUM_STATUS)
  {
    winMain->wprintf("%CInvalid status: %A%s\n", COLOR_RED, A_BOLD, _szArg);
    return;
  }

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  if (nStatus == ICQ_STATUS_OFFLINE)
  {
    gUserManager.DropOwner();
    licqDaemon->icqLogoff();
    return;
  }
  if (bInvisible)
    nStatus |= ICQ_STATUS_FxPRIVATE;

  // maintain the current extended status flags (we aren't changing any of
  // them in this function so it's ok)
  nStatus |= o->getStatusFlags();

  // disable combo box, flip pixmap...
  //lblStatus->setEnabled(false);

  // call the right function
  bool b = o->getStatusOffline();
  gUserManager.DropOwner();
  if (b)
     licqDaemon->icqLogon(nStatus);
  else
     licqDaemon->icqSetStatus(nStatus);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuContactList
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuContactList(char *)
{
  PrintUsers();
/*
  static char szStatusStr[32];
  unsigned short i = 0, j;

  PrintBoxTop(CurrentGroupName(), COLOR_WHITE, 40);

  bool bOfflineUsers = false;
  FOR_EACH_USER_START(LOCK_R)
  {
    // Only show users on the current group and not on the ignore list
    if (!pUser->GetInGroup(m_nGroupType, m_nCurrentGroup) ||
        (pUser->IgnoreList() && m_nGroupType != GROUPS_SYSTEM && m_nCurrentGroup != GROUP_IGNORE_LIST) )
      FOR_EACH_USER_CONTINUE

    if (i == 0 && m_bShowDividers && !pUser->getStatusOffline())
    {
      waddch(winMain->Win(), ACS_LTEE);
      for (j = 0; j < 11; j++) waddch(winMain->Win(), ACS_HLINE);
      winMain->wprintf("%A%C Online ",
            m_cColorOnline->bBright ? A_BOLD : A_NORMAL,
            m_cColorOnline->nColor);
      for (j = 0; j < 19; j++) waddch(winMain->Win(), ACS_HLINE);
      waddch(winMain->Win(), ACS_RTEE);
      waddch(winMain->Win(), '\n');
    }
    if (!bOfflineUsers && pUser->getStatusOffline())
    {
      if (!m_bShowOffline)
      {
        FOR_EACH_USER_BREAK;
      }
      if (m_bShowDividers)
      {
        waddch(winMain->Win(), ACS_LTEE);
        for (j = 0; j < 11; j++) waddch(winMain->Win(), ACS_HLINE);
        winMain->wprintf("%A%C Offline ",
            m_cColorOffline->bBright ? A_BOLD : A_NORMAL,
            m_cColorOffline->nColor);
        for (j = 0; j < 18; j++) waddch(winMain->Win(), ACS_HLINE);
        waddch(winMain->Win(), ACS_RTEE);
        waddch(winMain->Win(), '\n');
      }
      bOfflineUsers = true;
    }
    pUser->getStatusStr(szStatusStr);
    waddch(winMain->Win(), ACS_VLINE);
    if (bOfflineUsers)
    {
      winMain->wprintf("%A%C%c %-20s%16s",
                       m_cColorOffline->bBright ? A_BOLD : A_NORMAL,
                       m_cColorOffline->nColor,
                       pUser->getNumMessages() > 0 ? '*' : ' ',
                       pUser->getAlias(), szStatusStr);
    }
    else
    {
      winMain->wprintf("%A%C%c %-20s%16s",
                       m_cColorOnline->bBright ? A_BOLD : A_NORMAL,
                       m_cColorOnline->nColor,
                       pUser->getNumMessages() > 0 ? '*' : ' ',
                       pUser->getAlias(), szStatusStr);
    }
    PrintBoxRight(40);

    i++;
  }
  FOR_EACH_USER_END

  PrintBoxBottom(40);
*/
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuUser
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuUser(char *_szArg)
{
  char *szAlias, *szCmd;
  unsigned long nUin = 0;
  unsigned short nCmd = 0;

  // Check if the alias is quoted
  if (_szArg[0] == '"')
  {
    szAlias = &_szArg[1];
    szCmd = strchr(&_szArg[1], '"');
    if (szCmd == NULL)
    {
      winMain->wprintf("%CUnbalanced quotes.\n", COLOR_RED);
      return;
    }
    *szCmd++ = '\0';
    szCmd = strchr(szCmd, ' ');
  }
  else
  {
    szAlias = _szArg;
    szCmd = strchr(_szArg, ' ');
  }

  if (szCmd == NULL)
  {
    nCmd = 0;
  }
  else
  {
    *szCmd++ = '\0';
    STRIP(szCmd);
    unsigned short i;
    for (i = 0; i < NUM_USER_COMMANDS; i++)
    {
      if (strcasecmp(szCmd, aUserCommands[i].szName) == 0)
      {
        nCmd = i;
        break;
      }
    }
    if (i == NUM_USER_COMMANDS)
    {
      winMain->wprintf("%CInvalid user command: %A%s\n", COLOR_RED, A_BOLD, szCmd);
      return;
    }
  }

  // Find the user
  FOR_EACH_USER_START(LOCK_R)
  {
    if (strcasecmp(szAlias, pUser->getAlias()) == 0)
    {
      nUin = pUser->getUin();
      FOR_EACH_USER_BREAK;
    }
  }
  FOR_EACH_USER_END
  if (nUin == 0)
  {
    winMain->wprintf("%CInvalid user: %A%s\n", COLOR_RED, A_BOLD, szAlias);
    return;
  }

  // Run the command
  (this->*(aUserCommands[nCmd].fProcessCommand))(nUin);
}



/*---------------------------------------------------------------------------
 * CLicqConsole::MenuSet
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuSet(char *_szArg)
{
  char *szVariable, *szValue;
  unsigned short nVariable = 0, i;

  // If no argument then print all variables and return
  if (_szArg == NULL)
  {
    for (i = 0; i < NUM_VARIABLES; i++)
      PrintVariable(i);
    return;
  }

  szVariable = _szArg;
  szValue = strchr(_szArg, ' ');

  if (szValue != NULL)
  {
    *szValue++ = '\0';
    STRIP(szValue);
  }

  // Find the variable
  for (i = 0; i < NUM_VARIABLES; i++)
  {
    if (strcasecmp(szVariable, aVariables[i].szName) == 0)
    {
      nVariable = i;
      break;
    }
  }
  if (i == NUM_VARIABLES)
  {
    winMain->wprintf("%CNo such variable: %A%s\n", COLOR_RED, A_BOLD, szVariable);
    return;
  }

  // If there is no value then print the current one
  if (szValue == NULL)
  {
    PrintVariable(nVariable);
    return;
  }

  // Set the variable
  switch(aVariables[nVariable].nType)
  {
  case BOOL:
    *(bool *)aVariables[nVariable].pData =
      ( strcasecmp(szValue, "yes") == 0 ||
        strcasecmp(szValue, "on") == 0 ||
        strcasecmp(szValue, "true") == 0 );
    break;

  case COLOR:
    for (i = 0; i < NUM_COLORMAPS; i++)
    {
      if (strcasecmp(szValue, aColorMaps[i].szName) == 0)
        break;
    }
    if (i == NUM_COLORMAPS)
    {
      winMain->wprintf("%CNo such color: %A%s\n", COLOR_RED, A_BOLD, szValue);
      break;
    }
    *(const struct SColorMap **)aVariables[nVariable].pData = &aColorMaps[i];
    break;

  case STRING:
    if (szValue[0] != '"' || szValue[strlen(szValue) - 1] != '"')
    {
      winMain->wprintf("%CString values must be enclosed by double quotes (\").\n", COLOR_RED);
      return;
    }
    szValue[strlen(szValue) - 1] = '\0';
    strcpy((char *)aVariables[nVariable].pData, &szValue[1]);
    break;

  case INT:
    *(int *)aVariables[nVariable].pData = atoi(szValue);
    break;
  }


}
