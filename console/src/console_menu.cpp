#include "console.h"

#include <ctype.h>

const unsigned short NUM_COMMANDS = 7;
const struct SCommand aCommands[NUM_COMMANDS] =
{
  { "/contacts", &CLicqConsole::MenuContactList, NULL },
  { "/group", &CLicqConsole::MenuGroup, NULL },
  { "/user", &CLicqConsole::MenuUser, &CLicqConsole::TabUser },
  { "/status", &CLicqConsole::MenuStatus, &CLicqConsole::TabStatus },
  { "/plugins", &CLicqConsole::MenuPlugins, NULL },
  { "/help", &CLicqConsole::MenuHelp, NULL },
  { "/quit", &CLicqConsole::MenuQuit, NULL }
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
void CLicqConsole::MenuHelp(char *)
{
  PrintBoxTop("Menu", COLOR_WHITE, 40);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/c%Z%s", A_BOLD, A_BOLD, "ontacts");
  PrintBoxRight(40);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/g%Zroup [%A#%Z%s", A_BOLD, A_BOLD, A_BOLD, A_BOLD, "]");
  PrintBoxRight(40);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" [%A/u%Zser] %A<alias>%Z %Ai%Znfo|%Av%Ziew|%Am%Zessage|%Au%Zrl",
                   A_BOLD, A_BOLD, A_BOLD, A_BOLD, A_BOLD, A_BOLD, A_BOLD,
                   A_BOLD, A_BOLD, A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(40);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/s%Z%s", A_BOLD, A_BOLD, "tatus");
  PrintBoxRight(40);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/h%Z%s", A_BOLD, A_BOLD, "elp");
  PrintBoxRight(40);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/q%Z%s", A_BOLD, A_BOLD, "uit");
  PrintBoxRight(40);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %AF(1-%d)%Z to change between consoles", A_BOLD, MAX_CON,
                   A_BOLD);
  PrintBoxRight(40);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %AF%d%Z to see the log", A_BOLD, MAX_CON + 1, A_BOLD);
  PrintBoxRight(40);
  PrintBoxBottom(40);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuQuit
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuQuit(char *)
{
  m_bExit = true;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuPlugins
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuPlugins(char *_szArg)
{
  PluginsList l;
  PluginsListIter it;
  licqDaemon->PluginList(l);
  PrintBoxTop("Plugins", COLOR_BLUE, 40);
  for (it = l.begin(); it != l.end(); it++)
  {
    PrintBoxLeft();
    winMain->wprintf("[%3d] %s v%s", (*it)->Id(), (*it)->Name(), (*it)->Version());
    PrintBoxRight(40);
  }
  PrintBoxBottom(40);
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
    _szArg++;
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
      winMain->wprintf("%C Online ", COLOR_BLUE);
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
        winMain->wprintf("%C Offline ", COLOR_RED);
        for (j = 0; j < 18; j++) waddch(winMain->Win(), ACS_HLINE);
        waddch(winMain->Win(), ACS_RTEE);
        waddch(winMain->Win(), '\n');
      }
      bOfflineUsers = true;
    }
    pUser->getStatusStr(szStatusStr);
    waddch(winMain->Win(), ACS_VLINE);
    winMain->wprintf("%A%C%-20s%16s",
                     pUser->getNumMessages() > 0 ? A_BOLD : A_NORMAL,
                     bOfflineUsers ? COLOR_RED : COLOR_BLUE,
                     pUser->getAlias(), szStatusStr);
    PrintBoxRight(40);

    i++;
  }
  FOR_EACH_USER_END

  PrintBoxBottom(40);

}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuUser
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuUser(char *_szArg)
{
  char *szAlias, *szCmd;
  unsigned long nUin = 0;
  unsigned short nCmd = 0;

  // Find the command
  szCmd = strchr(_szArg, ' ');
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
  szAlias = _szArg;
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


