#include "console.h"

#include <ctype.h>

const unsigned short NUM_COMMANDS = 15;
const struct SCommand aCommands[NUM_COMMANDS] =
{
  { "contacts", &CLicqConsole::MenuContactList, NULL,
    "Force a refresh of the contact list." },
  { "group", &CLicqConsole::MenuGroup, NULL,
    "Prints the group list or changes to the given group number." },
  { "clear", &CLicqConsole::MenuClear, NULL,
    "Clears the current window." },
  { "filestat", &CLicqConsole::MenuFileStat, NULL,
    "Print out statistics on all current file transfers." },
  { "add", &CLicqConsole::MenuAdd, NULL,
    "Add a user to your list by uin." },
  { "user", &CLicqConsole::MenuUser, &CLicqConsole::TabUser,
    "User commands deal with indiviual users:\n"
    "info - print user information\n\n"
    "message - send a message to the user\n\n"
    "sendfile - send a file to the user\n\n"
    "url - send a url to the user\n\n"
    "view - view any new events from the user\n\n"
    "secure - open, close, or view the current secure channel status\n\n"
    "history - print the given range of events from the history.\n"
    "'$' represents the last message, and +/- can be used to specify "
    "an offset.  For example \"history $-5,$\" will print from the "
    "fifth-to-last event to the end.\n"
    "If only the start range is specified then one event will be printed.\n"
    "If the start range begins with +/- then the index of the last message "
    "printed will be modified by the operation.  If the end range begins "
    "with +/- then it will modify the start range value.\n"
    "To cycle through the last 10 events in the history\n"
    "try the following:\n"
    "user <> \"history $ - 10\"\n"
    "last \"history +1\"\n"
    "last \"history +1\"\n"
    "...\n" },
  { "owner", &CLicqConsole::MenuOwner, &CLicqConsole::TabOwner,
    "Commands dealing with yourself.  See /user help for details." },
  { "status", &CLicqConsole::MenuStatus, &CLicqConsole::TabStatus,
    "Set your status, prefix with \"*\" for invisible mode." },
  { "last", &CLicqConsole::MenuLast, &CLicqConsole::TabLast,
    "Perform the given command on the last user." },
  { "search", &CLicqConsole::MenuSearch, NULL,
    "Perform a search of the ICQ network." },
  { "set", &CLicqConsole::MenuSet, &CLicqConsole::TabSet,
    "Allows the setting and viewing of options.  With no arguments\n"
    "will print all current set'able values.  With one argument will\n"
    "print the value of the given argument.\n"
    "A boolean value can be yes/true/on or no/false/off.\n"
    "Color values can be red/blue/green/magenta/white/yellow or\n"
    "bright_<color> for bright colors." },
  { "plugins", &CLicqConsole::MenuPlugins, NULL,
    "List the currently loaded plugins." },
  { "define", &CLicqConsole::MenuDefine, NULL,
    "Define a new macro, enter macros by not using '/'.\n"
    "A macro can be any string of characters not containing\n"
    "a space.  The command can be any valid command, do not\n"
    "prepend the command character when defining.\n"
    "Example: \"/define r last message\" creates a macro \"r\"\n"
    "which replies to the last user you talked to."},
  { "help", &CLicqConsole::MenuHelp, NULL,
    "This help screen, can also be passed a command for detailed\n"
    "information about it." },
  { "quit", &CLicqConsole::MenuQuit, NULL,
    "Quit Licq." }
};

const unsigned short NUM_USER_COMMANDS = 9;
const struct SUserCommand aUserCommands[NUM_USER_COMMANDS] =
{
  { "info", &CLicqConsole::UserCommand_Info },
  { "view", &CLicqConsole::UserCommand_View },
  { "message", &CLicqConsole::UserCommand_Msg },
  { "sendfile", &CLicqConsole::UserCommand_SendFile },
  { "url", &CLicqConsole::UserCommand_Url },
  { "history", &CLicqConsole::UserCommand_History },
  { "auto-response", &CLicqConsole::UserCommand_FetchAutoResponse },
  { "remove", &CLicqConsole::UserCommand_Remove },
  { "secure", &CLicqConsole::UserCommand_Secure }
};

const unsigned short NUM_OWNER_COMMANDS = 4;
const struct SOwnerCommand aOwnerCommands[NUM_OWNER_COMMANDS] =
{
  { "info", &CLicqConsole::UserCommand_Info },
  { "view", &CLicqConsole::UserCommand_View },
  { "history", &CLicqConsole::UserCommand_History },
  { "auto-response", &CLicqConsole::UserCommand_SetAutoResponse }
};


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
    if (strncasecmp(_szArg, aCommands[i].szName, strlen(_szArg)) == 0)
      break;
  }
  if (i == NUM_COMMANDS)
  {
    PrintBadInput(_szArg);
    return;
  }
  winMain->wprintf("%AHelp on \"%Z%s%A\":%Z\n%s\n", A_BOLD, A_BOLD,
   aCommands[i].szName, A_BOLD, A_BOLD, aCommands[i].szHelp);

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
    winMain->wprintf("[%3d] %s v%s (%s %s) - %s", (*it)->Id(), (*it)->Name(),
                     (*it)->Version(), (*it)->BuildDate(),
                     (*it)->BuildTime(), (*it)->Status());
    PrintBoxRight(70);
  }
  PrintBoxBottom(70);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuDefine
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuDefine(char *szArg)
{
  MacroList::iterator iter;
  char *szCmd = szArg;

  if (szArg == NULL)
  {
    PrintMacros();
    return;
  }

  while (*szCmd != '\0' && *szCmd != ' ') szCmd++;

  // Check if we are undefining a macro
  if (*szCmd == '\0')
  {
    for (iter = listMacros.begin(); iter != listMacros.end(); iter++)
    {
      if (strcmp((*iter)->szMacro, szArg) == 0)
      {
        winMain->wprintf("%C%AErased macro \"%s -> %s\"\n",
         m_cColorInfo->nColor, m_cColorInfo->nAttr,
         (*iter)->szMacro, (*iter)->szCommand);
        delete *iter;
#undef erase
        listMacros.erase(iter);
        DoneOptions();
        return;
      }
    }
    winMain->wprintf("%CNo such macro \"%A%s%Z\"\n", COLOR_RED, A_BOLD,
      szArg, A_BOLD);
    return;
  }

  *szCmd++ = '\0';
  while (*szCmd == ' ') szCmd++;

  // See if this is a double macro definition
  for (iter = listMacros.begin(); iter != listMacros.end(); iter++)
  {
    if (strcmp((*iter)->szMacro, szArg) == 0)
    {
      delete *iter;
#undef erase
        listMacros.erase(iter);
      listMacros.erase(iter);
    }
  }

  // Set the macro
  SMacro *macro = new SMacro;
  strcpy(macro->szMacro, szArg);
  strcpy(macro->szCommand, szCmd);
  listMacros.push_back(macro);

  winMain->wprintf("%A%CAdded macro \"%s -> %s\"\n",
   m_cColorInfo->nAttr, m_cColorInfo->nColor,
   macro->szMacro, macro->szCommand);

  DoneOptions();
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

  GroupType nGroupType;
  unsigned short nCurrentGroup;

  // Try to change groups
  if (_szArg[0] == '*')
  {
    _szArg++;
    nGroupType = GROUPS_SYSTEM;
    nCurrentGroup = atol(_szArg);

    if (nCurrentGroup > NUM_GROUPS_SYSTEM || nCurrentGroup == 0)
    {
      winMain->wprintf("%CInvalid group number (0 - %d)\n", COLOR_RED,
                       NUM_GROUPS_SYSTEM);
      return;
    }
    m_nCurrentGroup = nCurrentGroup;
    m_nGroupType = nGroupType;
    winMain->wprintf("%C%ASwitching to group *%d (%s).\n",
                     m_cColorInfo->nColor, m_cColorInfo->nAttr,
                     m_nCurrentGroup,
                     GroupsSystemNames[m_nCurrentGroup]);
  }
  else
  {
    nGroupType = GROUPS_USER;
    nCurrentGroup = atol(_szArg);

    if (nCurrentGroup > gUserManager.NumGroups())
    {
      winMain->wprintf("%CInvalid group number (0 - %d)\n", COLOR_RED,
                       gUserManager.NumGroups());
      return;
    }
    m_nCurrentGroup = nCurrentGroup;
    m_nGroupType = nGroupType;
    GroupList *g = gUserManager.LockGroupList(LOCK_R);
    winMain->wprintf("%C%ASwitching to group %d (%s).\n",
                     m_cColorInfo->nColor, m_cColorInfo->nAttr,
                     m_nCurrentGroup,
                     m_nCurrentGroup == 0 ? "All Users" : (*g)[m_nCurrentGroup - 1]);
    gUserManager.UnlockGroupList();
  }

  PrintStatus();
  CreateUserList();
  PrintUsers();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuAdd
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuAdd(char *szArg)
{
  if (szArg == NULL)
  {
    winMain->wprintf("%CSpecify a UIN to add.\n", COLOR_RED);
    return;
  }

  // Try to change groups
  int nUin = atol(szArg);
  bool bAlert = false;

  while (*szArg != '\0' && *szArg != ' ') szArg++;
  if (*szArg == ' ')
  {
    while (*szArg == ' ') szArg++;
    if (strcasecmp(szArg, "alert") == 0)
      bAlert = true;
  }

  if (!licqDaemon->AddUserToList(nUin))
  {
    winMain->wprintf("%CAdding user %lu failed (duplicate user or invalid uin).\n",
     COLOR_RED, nUin);
    return;
  }

  winMain->wprintf("%C%AAdded user %ld.\n",
                     m_cColorInfo->nColor, m_cColorInfo->nAttr,
                     nUin);

  if (bAlert)
  {
    licqDaemon->icqAlertUser(nUin);
    winMain->wprintf("%C%AAlerted user %ld they were added.\n",
                     m_cColorInfo->nColor, m_cColorInfo->nAttr,
                     nUin);
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

  // call the right function
  bool b = o->StatusOffline();
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
  CreateUserList();
  PrintUsers();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuUser
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuUser(char *_szArg)
{
  char *szAlias, *szCmd, *szUserArg;
  unsigned long nUin = 0;
  unsigned short nCmd = 0;
  bool bCheckUin = true;

  if(_szArg == NULL) {
    winMain->wprintf("%CNo command given. See </help user> for details.\n",
                     COLOR_RED);
    return;
  }

  // Check if the alias is quoted
  if (_szArg[0] == '"')
  {
    bCheckUin = false;
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
    szUserArg = NULL;
  }
  else
  {
    *szCmd++ = '\0';
    STRIP(szCmd);
    // Find any command args
    szUserArg = strchr(szCmd, ' ');
    if (szUserArg != NULL)
    {
      *szUserArg++ = '\0';
      STRIP(szUserArg);
      if (*szUserArg == '\0') szUserArg = NULL;
    }
    unsigned short i;
    for (i = 0; i < NUM_USER_COMMANDS; i++)
    {
      if (strncasecmp(szCmd, aUserCommands[i].szName, strlen(szCmd)) == 0)
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
  // See if all the chars are digits
  if (bCheckUin)
  {
    char *sz = szAlias;
    while (isdigit(*sz)) sz++;
    if (*sz == '\0') nUin = atol(szAlias);
  }

  if (nUin == 0)
  {
    FOR_EACH_USER_START(LOCK_R)
    {
      if (strcasecmp(szAlias, pUser->GetAlias()) == 0)
      {
        nUin = pUser->Uin();
        FOR_EACH_USER_BREAK;
      }
    }
    FOR_EACH_USER_END
    if (nUin == 0)
    {
      winMain->wprintf("%CInvalid user: %A%s\n", COLOR_RED, A_BOLD, szAlias);
      return;
    }
  }
  else
  {
    if (!gUserManager.IsOnList(nUin))
    {
      winMain->wprintf("%CInvalid uin: %A%lu\n", COLOR_RED, A_BOLD, nUin);
      return;
    }
  }

  // Save this as the last user
  if (winMain->nLastUin != nUin)
  {
    winMain->nLastUin = nUin;
    PrintStatus();
  }
  // Run the command
  (this->*(aUserCommands[nCmd].fProcessCommand))(nUin, szUserArg);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuLast
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuLast(char *_szArg)
{
  unsigned short nCmd = 0;
  char *szUserArg;

  if (winMain->nLastUin == 0)
  {
    winMain->wprintf("%CNo last user.\n", COLOR_RED);
    return;
  }

  if (_szArg == NULL)
  {
    nCmd = 0;
    szUserArg = NULL;
  }
  else
  {
    // Find any command args
    szUserArg = strchr(_szArg, ' ');
    if (szUserArg != NULL)
    {
      *szUserArg++ = '\0';
      STRIP(szUserArg);
      if (*szUserArg == '\0') szUserArg = NULL;
    }
    for (nCmd = 0; nCmd < NUM_USER_COMMANDS; nCmd++)
    {
      if (strncasecmp(_szArg, aUserCommands[nCmd].szName, strlen(_szArg)) == 0)
        break;
    }
    if (nCmd == NUM_USER_COMMANDS)
    {
      winMain->wprintf("%CInvalid user command: %A%s\n", COLOR_RED, A_BOLD, _szArg);
      return;
    }
  }

  // Run the command
  (this->*(aUserCommands[nCmd].fProcessCommand))(winMain->nLastUin, szUserArg);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuOwner
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuOwner(char *_szArg)
{
  unsigned short nCmd = 0;
  char *szUserArg;

  if(_szArg == NULL) {
    winMain->wprintf("%COwner command required. See </help owner>.\n");
    return;
  }

  // Find any command args
  szUserArg = strchr(_szArg, ' ');
  if (szUserArg != NULL)
  {
    *szUserArg++ = '\0';
    STRIP(szUserArg);
    if (*szUserArg == '\0') szUserArg = NULL;
  }

  for (nCmd = 0; nCmd < NUM_OWNER_COMMANDS; nCmd++)
  {
    if (strncasecmp(_szArg, aOwnerCommands[nCmd].szName, strlen(_szArg)) == 0)
      break;
  }
  if (nCmd == NUM_OWNER_COMMANDS)
  {
    winMain->wprintf("%CInvalid owner command: %A%s\n", COLOR_RED, A_BOLD, _szArg);
    return;
  }

  // Run the command
  (this->*(aOwnerCommands[nCmd].fProcessCommand))(gUserManager.OwnerUin(), szUserArg);
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

    switch(nVariable)
    {
      case 2: m_nColorOnline = i;    break;
      case 3: m_nColorAway = i;      break;
      case 4: m_nColorOffline = i;   break;
      case 5: m_nColorNew = i;       break;
      case 6: m_nColorGroupList = i; break;
      case 7: m_nColorQuery = i;     break;
      case 8: m_nColorInfo = i;      break;
      case 9: m_nColorError = i;     break;
      default: break;
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
    strncpy((char *)aVariables[nVariable].pData, &szValue[1], 30);
    break;

  case INT:
    *(int *)aVariables[nVariable].pData = atoi(szValue);
    break;
  }

  // Save it all
  DoneOptions();
}

/*--------------------------------------------------------------------------
 * CLicqConsole::MenuFileStat
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuFileStat(char *sz)
{
  bool bNum = false;

  // Go through the list and print out the info on each file
  list<CFileTransferManager *>::iterator iter;
  for(iter = m_lFileStat.begin(); iter != m_lFileStat.end(); iter++)
  {
    bNum = true;
    PrintFileStat(*iter);
  }

  if(!bNum)
    winMain->wprintf("%C%ANo current file transfers.\n%C%Z",
      COLOR_RED, A_BOLD, COLOR_WHITE, A_BOLD);
}

/*-----------------------------------------------------------------------
 * CLicqConsole::MenuClear
 *---------------------------------------------------------------------*/
void CLicqConsole::MenuClear(char *)
{
  winMain->wprintf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}


/*-----------------------------------------------------------------------
 * CLicqConsole::MenuSearch
 *---------------------------------------------------------------------*/
void CLicqConsole::MenuSearch(char *)
{
  Command_Search();
}


