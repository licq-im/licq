#include "console.h"
#include "eventdesc.h"


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintBadInput
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintBadInput(const char *_szIn)
{
  winMain->wprintf("%CInvalid command [%A%s%Z].  Type \"help\" for help.\n",
                   COLOR_RED, A_BOLD, _szIn, A_BOLD);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintBoxTop
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintBoxTop(const char *_szTitle, short _nColor, short _nLength)
{
  unsigned short i, j;
  wattrset(winMain->Win(), COLOR_PAIR(COLOR_WHITE));
  waddch(winMain->Win(), '\n');
  waddch(winMain->Win(), ACS_ULCORNER);
  for (i = 0; i < 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), ACS_RTEE);
  winMain->wprintf("%C %s ", _nColor, _szTitle);
  waddch(winMain->Win(), ACS_LTEE);
  j = _nLength - 16 - strlen(_szTitle);
  for (i = 0; i < j; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), ACS_URCORNER);
  waddch(winMain->Win(), '\n');
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintBoxLeft
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintBoxLeft(void)
{
  waddch(winMain->Win(), ACS_VLINE);
  waddch(winMain->Win(), ' ');
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintBoxRight
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintBoxRight(short _nLength)
{
  int y, x;
  getyx(winMain->Win(), y, x);
  mvwaddch(winMain->Win(), y, _nLength - 1, ACS_VLINE);
  waddch(winMain->Win(), '\n');
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintBoxBottom
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintBoxBottom(short _nLength)
{
  unsigned short i;
  waddch(winMain->Win(), ACS_LLCORNER);
  for (i = 0; i < _nLength - 2; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), ACS_LRCORNER);
  waddch(winMain->Win(), '\n');

  winMain->RefreshWin();
  wattrset(winMain->Win(), COLOR_PAIR(COLOR_WHITE));
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintPrompt
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintPrompt(void)
{
  werase(winPrompt->Win());
  winPrompt->wprintf("%C> ", COLOR_CYAN);
  winPrompt->RefreshWin();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintStatus
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintStatus(void)
{
  static char szStatusStr[32];
  static char szMsgStr[16];
  static char szLastUser[32];

  werase(winStatus->Win());

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nNumOwnerEvents = o->getNumMessages();
  gUserManager.DropOwner();
  unsigned short nNumUserEvents = ICQUser::getNumUserEvents() - nNumOwnerEvents;
  if (nNumOwnerEvents > 0)
    sprintf (szMsgStr, "System Message");
  else if (nNumUserEvents > 0)
    sprintf (szMsgStr, "%d Message%c", nNumUserEvents, nNumUserEvents == 1 ? ' ' : 's');
  else
    strcpy(szMsgStr, "No Messages");

  if (winMain->nLastUin != 0)
  {
    ICQUser *u = gUserManager.FetchUser(winMain->nLastUin, LOCK_R);
    if (u == NULL)
      strcpy(szLastUser, "<Removed>");
    else
      strcpy(szLastUser, u->getAlias());
    gUserManager.DropUser(u);
  }
  else
    strcpy(szLastUser, "<None>");

  o = gUserManager.FetchOwner(LOCK_R);
  o->getStatusStr(szStatusStr);
  wbkgdset(winStatus->Win(), COLOR_PAIR(COLOR_WHITE));
  mvwhline(winStatus->Win(), 0, 0, ACS_HLINE, COLS);
  mvwaddch(winStatus->Win(), 0, COLS - USER_WIN_WIDTH - 1, ACS_BTEE);
  wmove(winStatus->Win(), 1, 0);

  wbkgdset(winStatus->Win(), COLOR_PAIR(COLOR_YELLOW_BLUE));
  winStatus->wprintf("%C%A[ %C%s %C(%C%ld%C) - S: %C%s %C- G: %C%s %C- M: %C%s %C- L: %C%s %C]", COLOR_YELLOW_BLUE,
                     A_BOLD, COLOR_WHITE_BLUE, o->getAlias(), COLOR_YELLOW_BLUE,
                     COLOR_WHITE_BLUE, o->getUin(), COLOR_YELLOW_BLUE,
                     COLOR_CYAN_BLUE, szStatusStr, COLOR_YELLOW_BLUE,
                     COLOR_CYAN_BLUE, CurrentGroupName(), COLOR_YELLOW_BLUE,
                     COLOR_CYAN_BLUE, szMsgStr, COLOR_YELLOW_BLUE, COLOR_CYAN_BLUE,
                     szLastUser, COLOR_YELLOW_BLUE);
  gUserManager.DropOwner();
  wclrtoeol(winStatus->Win());
  winStatus->RefreshWin();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintGroups
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintGroups(void)
{
  unsigned short j = 1, k;

  PrintBoxTop("Groups", COLOR_WHITE, 26);

  PrintBoxLeft();
  winMain->wprintf("%A%C%3d. %-19s",
      m_cColorGroupList->bBright ? A_BOLD : A_NORMAL,
      m_cColorGroupList->nColor, 0, "All Users");
  PrintBoxRight(26);
  waddch(winMain->Win(), ACS_LTEE);
  for (k = 0; k < 24; k++) waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), ACS_RTEE);
  waddch(winMain->Win(), '\n');

  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  for (GroupListIter i = g->begin(); i != g->end(); i++, j++)
  {
    PrintBoxLeft();
    winMain->wprintf("%A%C%3d. %-19s",
        m_cColorGroupList->bBright ? A_BOLD : A_NORMAL,
        m_cColorGroupList->nColor, j, *i);
    PrintBoxRight(26);
  }
  gUserManager.UnlockGroupList();
  PrintBoxBottom(26);

}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintVariable
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintVariable(unsigned short nVar)
{
  winMain->wprintf("%s = ", aVariables[nVar].szName);

  switch(aVariables[nVar].nType)
  {
  case INT:
    winMain->wprintf("%d\n", *(int *)aVariables[nVar].pData);
    break;

  case BOOL:
    winMain->wprintf("%s\n", *(bool *)aVariables[nVar].pData == true
        ? "<YES>" : "<NO>");
    break;

  case STRING:
    winMain->wprintf("\"%s\"\n", (char *)aVariables[nVar].pData);
    break;

  case COLOR:
    winMain->wprintf("[%s]\n",
        (*(struct SColorMap **)aVariables[nVar].pData)->szName );
    break;
  }
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintUsers
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintUsers(void)
{
  static char szLine[256];
  unsigned short i = 0, j;

  werase(winUsers->Win());
  mvwvline(winBar->Win(), 0, 0, ACS_VLINE, LINES - 5);

  bool bOfflineUsers = false;
  FOR_EACH_USER_START(LOCK_R)
  {
    // Only show users on the current group and not on the ignore list
    if (!pUser->GetInGroup(m_nGroupType, m_nCurrentGroup) ||
        (pUser->IgnoreList() && m_nGroupType != GROUPS_SYSTEM && m_nCurrentGroup != GROUP_IGNORE_LIST) )
      FOR_EACH_USER_CONTINUE

    if (i == 0 && m_bShowDividers && !pUser->getStatusOffline())
    {
      int yp, xp;
      getyx(winUsers->Win(), yp, xp);
      mvwaddch(winBar->Win(), yp, 0, ACS_LTEE);
      for (j = 0; j < 10; j++) waddch(winUsers->Win(), ACS_HLINE);
      winUsers->wprintf("%A%C Online ",
            m_cColorOnline->bBright ? A_BOLD : A_NORMAL,
            m_cColorOnline->nColor);
      for (j = 19; j < USER_WIN_WIDTH; j++) waddch(winUsers->Win(), ACS_HLINE);
      waddch(winUsers->Win(), '\n');
    }
    if (!bOfflineUsers && pUser->getStatusOffline())
    {
      if (!m_bShowOffline)
      {
        FOR_EACH_USER_BREAK;
      }
      if (m_bShowDividers)
      {
        int yp, xp;
        getyx(winUsers->Win(), yp, xp);
        mvwaddch(winBar->Win(), yp, 0, ACS_LTEE);
        for (j = 0; j < 10; j++) waddch(winUsers->Win(), ACS_HLINE);
        winUsers->wprintf("%A%C Offline ",
            m_cColorOffline->bBright ? A_BOLD : A_NORMAL,
            m_cColorOffline->nColor);
        for (j = 20; j < USER_WIN_WIDTH; j++) waddch(winUsers->Win(), ACS_HLINE);
        waddch(winUsers->Win(), '\n');
      }
      bOfflineUsers = true;
    }
    switch(pUser->getStatus())
    {
    case ICQ_STATUS_ONLINE:
      pUser->usprintf(szLine, m_szOnlineFormat);
      winUsers->wprintf("%A%C%c%s\n",
                       m_cColorOnline->bBright ? A_BOLD : A_NORMAL,
                       m_cColorOnline->nColor,
                       pUser->getNumMessages() > 0 ? '*' : ' ',
                       szLine);
      break;
    case ICQ_STATUS_OFFLINE:
      pUser->usprintf(szLine, m_szOfflineFormat);
      winUsers->wprintf("%A%C%c%s\n",
                       m_cColorOffline->bBright ? A_BOLD : A_NORMAL,
                       m_cColorOffline->nColor,
                       pUser->getNumMessages() > 0 ? '*' : ' ',
                       szLine);
      break;
    default:
      pUser->usprintf(szLine, m_szAwayFormat);
      winUsers->wprintf("%A%C%c%s\n",
                       m_cColorAway->bBright ? A_BOLD : A_NORMAL,
                       m_cColorAway->nColor,
                       pUser->getNumMessages() > 0 ? '*' : ' ',
                       szLine);
      break;
    }

    if (i >= winUsers->Rows() - 3) FOR_EACH_USER_BREAK;
    i++;
  }
  FOR_EACH_USER_END

  winBar->RefreshWin();
  winUsers->RefreshWin();

}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintHelp
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintHelp(void)
{
  PrintBoxTop("Menu", COLOR_WHITE, 48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/c%Z%s", A_BOLD, A_BOLD, "ontacts");
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/g%Zroup [ %A#%Z%s", A_BOLD, A_BOLD, A_BOLD, A_BOLD, " ]");
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" [ %A/u%Zser ] %A<alias>%Z <info | view | message |",
                   A_BOLD, A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("                    url | history>",
                   A_BOLD, A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/o%Zwner [ %Av%Ziew|%Ai%Znfo ]",
                   A_BOLD, A_BOLD, A_BOLD, A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/l%Zast [ %A<user-command>%Z ]",
                   A_BOLD, A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/st%Zatus [*]<online | away | na | dnd |"
                   "              occupied | ffc | offline>", A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/se%Z%s [ %A<variable>%Z [ = %A<value>%Z ] ]",
                   A_BOLD, A_BOLD, "t", A_BOLD, A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/h%Zelp [ %A<command>%Z ]", A_BOLD, A_BOLD,
                   A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A/q%Zuit", A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %AF(1-%d)%Z to change between consoles", A_BOLD, MAX_CON,
                   A_BOLD);
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %AF%d%Z to see the log", A_BOLD, MAX_CON + 1, A_BOLD);
  PrintBoxRight(48);
  PrintBoxBottom(48);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintHistory
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintHistory(HistoryList &lHistory, unsigned short nStart,
                                unsigned short nEnd, const char *szFrom)
{
  HistoryListIter it = lHistory.begin();
  unsigned short n = 0;
  for (n = 0; n < nStart && it != lHistory.end(); n++, it++);
  while (n <= nEnd && it != lHistory.end())
  {
    wattron(winMain->Win(), A_BOLD);
    for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
      waddch(winMain->Win(), ACS_HLINE);
    waddch(winMain->Win(), '\n');
    time_t t = (*it)->Time();
    char *szTime = ctime(&t);
    szTime[16] = '\0';
    winMain->wprintf("%A%C[%d of %d] %s from %s (%s) [%c%c%c]:\n%Z%s\n", A_BOLD,
                     COLOR_WHITE, n + 1, lHistory.size(), EventDescription(*it),
                     szFrom,
                     szTime, (*it)->IsDirect() ? 'D' : '-',
                     (*it)->IsMultiRec() ? 'M' : '-', (*it)->IsUrgent() ? 'U' : '-',
                     A_BOLD, (*it)->Text());

    n++;
    it++;
  }
  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  winMain->RefreshWin();
  wattroff(winMain->Win(), A_BOLD);
}


