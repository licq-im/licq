#include "console.h"
#include "licq_countrycodes.h"
#include "licq_languagecodes.h"
#include "licq_filetransfer.h"

#include <string.h>
#include <time.h>

//======Utilities============================================================
char *EncodeFileSize(unsigned long nSize)
{
  char szUnit[6];

  if(nSize >= (1024 * 1024))
  {
    nSize /= (1024 * 1024) / 10;
    strcpy(szUnit, "MB");
  }
  else if(nSize >= 1024)
  {
    nSize /= (1024 / 10);
    strcpy(szUnit, "KB");
  }
  else if(nSize != 1)
  {
    nSize *= 10;
    strcpy(szUnit, "Bytes");
  }
  else
  {
    nSize *= 10;
    strcpy(szUnit, "Byte");
  }

  char buf[16];
  sprintf(buf, "%ld.%ld %s", (nSize / 10), (nSize % 10), szUnit);
  return strdup(buf);
}

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
void CLicqConsole::PrintBoxLeft()
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
void CLicqConsole::PrintPrompt()
{
  werase(winPrompt->Win());
  winPrompt->wprintf("%C> ", COLOR_CYAN);
  winPrompt->RefreshWin();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintStatus
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintStatus()
{
  static char szMsgStr[16];
  static char szLastUser[32];

  werase(winStatus->Win());

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short nNumOwnerEvents = o->NewMessages();
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
    {
      strcpy(szLastUser, u->GetAlias());
      gUserManager.DropUser(u);
    }
  }
  else
    strcpy(szLastUser, "<None>");

  o = gUserManager.FetchOwner(LOCK_R);
  wbkgdset(winStatus->Win(), COLOR_PAIR(COLOR_WHITE));
  mvwhline(winStatus->Win(), 0, 0, ACS_HLINE, COLS);
  mvwaddch(winStatus->Win(), 0, COLS - USER_WIN_WIDTH - 1, ACS_BTEE);
  wmove(winStatus->Win(), 1, 0);

  wbkgdset(winStatus->Win(), COLOR_PAIR(COLOR_YELLOW_BLUE));
  winStatus->wprintf("%C%A[ %C%s %C(%C%ld%C) - S: %C%s %C- G: %C%s %C- M: %C%s %C- L: %C%s %C]", COLOR_YELLOW_BLUE,
                     A_BOLD, COLOR_WHITE_BLUE, o->GetAlias(), COLOR_YELLOW_BLUE,
                     COLOR_WHITE_BLUE, o->Uin(), COLOR_YELLOW_BLUE,
                     COLOR_CYAN_BLUE, o->StatusStr(), COLOR_YELLOW_BLUE,
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
void CLicqConsole::PrintGroups()
{
  unsigned short j = 1, k;

  PrintBoxTop("Groups", COLOR_WHITE, 26);

  PrintBoxLeft();
  winMain->wprintf("%A%C%3d. %-19s",
      m_cColorGroupList->nAttr,
      m_cColorGroupList->nColor, 0, "All Users");
  PrintBoxRight(26);
  waddch(winMain->Win(), ACS_LTEE);
  for (k = 0; k < 24; k++) waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), ACS_RTEE);
  waddch(winMain->Win(), '\n');

  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  for (GroupList::iterator i = g->begin(); i != g->end(); i++, j++)
  {
    PrintBoxLeft();
    winMain->wprintf("%A%C%3d. %-19s",
        m_cColorGroupList->nAttr,
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
 * CLicqConsole::CreateUserList
 *-------------------------------------------------------------------------*/
void CLicqConsole::CreateUserList()
{
  unsigned short i = 0;
  struct SUser *s = NULL;
  list <SUser *>::iterator it;

  // Clear the list
  for (it = m_lUsers.begin(); it != m_lUsers.end(); it++)
  {
    delete (*it);
  }
#undef clear
  m_lUsers.clear();

  FOR_EACH_USER_START(LOCK_R)
  {
    // Only show users on the current group and not on the ignore list
    if (!pUser->GetInGroup(m_nGroupType, m_nCurrentGroup) ||
        (pUser->IgnoreList() && m_nGroupType != GROUPS_SYSTEM && m_nCurrentGroup != GROUP_IGNORE_LIST) )
      FOR_EACH_USER_CONTINUE

    if (!m_bShowOffline && pUser->StatusOffline() )
    {
      FOR_EACH_USER_CONTINUE;
    }

    s = new SUser;
    sprintf(s->szKey, "%05u%010lu", pUser->Status(), pUser->Touched() ^ 0xFFFFFFFF);
    s->nUin = pUser->Uin();
    s->bOffline = pUser->StatusOffline();

    unsigned long iStatus = pUser->StatusFull();

    if(iStatus & ICQ_STATUS_FxPRIVATE)
    {
      pUser->usprintf(&s->szLine[1], m_szOtherOnlineFormat);
      s->color = m_cColorOnline;
    }

    if((unsigned short)iStatus == ICQ_STATUS_OFFLINE)
    {
      pUser->usprintf(&s->szLine[1], m_szOfflineFormat);
      s->color = m_cColorOffline;
    }
    else if( (unsigned short) iStatus != ICQ_STATUS_OFFLINE && 
        ((iStatus & ICQ_STATUS_DND) || (iStatus & ICQ_STATUS_OCCUPIED) ||
        (iStatus & ICQ_STATUS_NA) || (iStatus & ICQ_STATUS_AWAY)))
    {
      pUser->usprintf(&s->szLine[1], m_szAwayFormat);
      s->color = m_cColorAway;
    }
    else if((unsigned short)iStatus == ICQ_STATUS_FREEFORCHAT)
    {
      pUser->usprintf(&s->szLine[1], m_szOtherOnlineFormat);
      s->color = m_cColorOnline;
    }
    else if((unsigned short)iStatus == ICQ_STATUS_ONLINE)
    {
      pUser->usprintf(&s->szLine[1], m_szOnlineFormat);
      s->color = m_cColorOnline;
    }

    if (pUser->NewUser())
      s->color = m_cColorNew;
    s->szLine[0] = pUser->NewMessages() > 0 ? '*' : ' ';

    // Insert into the list
    bool found = false;
    for (it = m_lUsers.begin(); it != m_lUsers.end(); it++)
    {
      if ( strcmp(s->szKey, (*it)->szKey) <= 0)
      {
        m_lUsers.insert(it, s);
        found = true;
        break;
      }
    }
    if (!found)
      m_lUsers.push_back(s);

    i++;
  }
  FOR_EACH_USER_END

}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintUsers
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintUsers()
{
  unsigned short i = 0, j;

  werase(winUsers->Win());
  mvwvline(winBar->Win(), 0, 0, ACS_VLINE, LINES - 5);

  bool bOfflineUsers = false;
  for (list<SUser *>::iterator it = m_lUsers.begin();
       it != m_lUsers.end();
       it++)
  {
    if (i == 0 && m_bShowDividers && !(*it)->bOffline)
    {
      int yp, xp;
      getyx(winUsers->Win(), yp, xp);
      mvwaddch(winBar->Win(), yp, 0, ACS_LTEE);
      for (j = 0; j < 10; j++) waddch(winUsers->Win(), ACS_HLINE);
      winUsers->wprintf("%A%C Online ",
            m_cColorOnline->nAttr,
            m_cColorOnline->nColor);
      for (j = 19; j < USER_WIN_WIDTH; j++) waddch(winUsers->Win(), ACS_HLINE);
      waddch(winUsers->Win(), '\n');
    }

    if (!bOfflineUsers && (*it)->bOffline)
    {
      if (m_bShowDividers)
      {
        int yp, xp;
        getyx(winUsers->Win(), yp, xp);
        mvwaddch(winBar->Win(), yp, 0, ACS_LTEE);
        for (j = 0; j < 10; j++) waddch(winUsers->Win(), ACS_HLINE);
        winUsers->wprintf("%A%C Offline ",
            m_cColorOffline->nAttr,
            m_cColorOffline->nColor);
        for (j = 20; j < USER_WIN_WIDTH; j++) waddch(winUsers->Win(), ACS_HLINE);
        waddch(winUsers->Win(), '\n');
      }
      bOfflineUsers = true;
    }
    winUsers->wprintf("%A%C%s\n",
                      (*it)->color->nAttr,
                      (*it)->color->nColor,
                      (*it)->szLine);

    if (i >= winUsers->Rows() - 3) break;
    i++;
  }

  winBar->RefreshWin();
  winUsers->RefreshWin();

}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintHelp
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintHelp()
{
  PrintBoxTop("Menu", COLOR_WHITE, 48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%cc%Z%s", A_BOLD, m_szCommandChar[0], A_BOLD, "ontacts");
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%cg%Zroup [ %A#%Z%s", A_BOLD, m_szCommandChar[0],
    A_BOLD, A_BOLD, A_BOLD, " ]");
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%ca%Zdd %A<uin>%Z [ alert ]", A_BOLD, m_szCommandChar[0],
    A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" [ %A%cu%Zser ] %A<alias>%Z <info | view | message |",
                   A_BOLD, m_szCommandChar[0], A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("                    sendfile | url |");
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("                    history | remove |");
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("                    auto-response>");
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%co%Zwner <view | info | history <n[,n]>",
                   A_BOLD, m_szCommandChar[0], A_BOLD);
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("         auto-response>");
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%cl%Zast [ %A<user-command>%Z ]",
                   A_BOLD, m_szCommandChar[0], A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%cf%Zilestat", A_BOLD, m_szCommandChar[0], A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%cst%Zatus [*]<online | away | na | dnd |", A_BOLD,
    m_szCommandChar[0], A_BOLD);
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("             occupied | ffc | offline>");
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%cse%Z%s [ %A<variable>%Z [ = %A<value>%Z ] ]",
                   A_BOLD, m_szCommandChar[0], A_BOLD, "t", A_BOLD, A_BOLD,
                   A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%ch%Zelp [ %A<command>%Z ]", A_BOLD, m_szCommandChar[0],
                   A_BOLD, A_BOLD, A_BOLD);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %A%cq%Zuit", A_BOLD, m_szCommandChar[0], A_BOLD);
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
    winMain->wprintf("%A%C[%d of %d] %s %s %s (%s) [%c%c%c]:\n%Z%s\n", A_BOLD,
                     COLOR_WHITE, n + 1, lHistory.size(), (*it)->Description(),
                     (*it)->Direction() == D_RECEIVER ? "from" : "to", szFrom,
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


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintInfo_General
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintInfo_General(unsigned long nUin)
{
  // Print the users info to the main window
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;

  // Some IP, Real IP and last seen stuff
  char buf[32];
  char szRealIp[32];
  const unsigned long nRealIp = u->RealIp();
  strcpy(szRealIp, inet_ntoa_r(*(struct in_addr *)&nRealIp, buf));
  time_t nLast = u->LastOnline();

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  wattroff(winMain->Win(), A_BOLD);

  winMain->wprintf("%s %A(%Z%ld%A) General Info - %Z%s\n", u->GetAlias(), A_BOLD,
                   A_BOLD, u->Uin(), A_BOLD, A_BOLD, u->StatusStr());

  winMain->wprintf("%C%AName: %Z%s %s\n", COLOR_WHITE, A_BOLD, A_BOLD,
                   u->GetFirstName(), u->GetLastName());
  winMain->wprintf("%C%AIp: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD,
                   u->IpPortStr(buf));
  winMain->wprintf("%C%AReal Ip: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD,
                   szRealIp);
  winMain->wprintf("%C%AEmail 1: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetEmailPrimary());
  winMain->wprintf("%C%AEmail 2: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetEmailSecondary());
  winMain->wprintf("%C%ACity: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCity());
  winMain->wprintf("%C%AState: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetState());
  winMain->wprintf("%C%AAddress: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetAddress());
  winMain->wprintf("%C%APhone Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetPhoneNumber());
  winMain->wprintf("%C%AFax Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetFaxNumber());
  winMain->wprintf("%C%ACellular Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCellularNumber());
  winMain->wprintf("%C%AZipcode: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetZipCode());
  winMain->wprintf("%C%ACountry: ", COLOR_WHITE, A_BOLD);
  if (u->GetCountryCode() == COUNTRY_UNSPECIFIED)
    winMain->wprintf("%CUnspecified\n", COLOR_WHITE);
  else
  {
    const SCountry *c = GetCountryByCode(u->GetCountryCode());
    if (c == NULL)
      winMain->wprintf("%CUnknown (%d)\n", COLOR_WHITE, u->GetCountryCode());
    else  // known
      winMain->wprintf("%C%s\n", COLOR_WHITE, c->szName);
  }
  winMain->wprintf("%C%ATimezone: %ZGMT%c%02d%s\n", COLOR_WHITE, A_BOLD, A_BOLD,
                   u->GetTimezone() > 0 ? '-' : '+',
                   u->GetTimezone() / 2,
                   u->GetTimezone() % 2 ? "30" : "00");
  winMain->wprintf("%C%ALast Seen: %Z%s", COLOR_WHITE, A_BOLD, A_BOLD,
    ctime(&nLast));

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  winMain->RefreshWin();
  wattroff(winMain->Win(), A_BOLD);

  gUserManager.DropUser(u);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintInfo_More
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintInfo_More(unsigned long nUin)
{
  // Print the users info to the main window
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  wattroff(winMain->Win(), A_BOLD);

  winMain->wprintf("%s %A(%Z%ld%A) More Info - %Z%s\n", u->GetAlias(), A_BOLD,
                   A_BOLD, u->Uin(), A_BOLD, A_BOLD, u->StatusStr());

  if (u->GetAge() == AGE_UNSPECIFIED)
    winMain->wprintf("%C%AAge: %ZUnspecified\n", COLOR_WHITE, A_BOLD, A_BOLD);
  else
    winMain->wprintf("%C%AAge: %Z%d\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetAge());
  winMain->wprintf("%C%AGender: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetGender() == GENDER_MALE ? "Male" : u->GetGender() == GENDER_FEMALE ? "Female" : "Unspecified");
  winMain->wprintf("%C%AHomepage: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetHomepage());
  winMain->wprintf("%C%ABirthday: %Z%d/%d/%d\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetBirthDay(), u->GetBirthMonth(), u->GetBirthYear() + 1900);
  for (unsigned short i = 0; i < 3; i++)
  {
    winMain->wprintf("%C%ALanguage %d: ", COLOR_WHITE, A_BOLD, i + 1);
    const SLanguage *l = GetLanguageByCode(u->GetLanguage(i));
    if (l == NULL)
      winMain->wprintf("%CUnknown (%d)\n", COLOR_WHITE, u->GetLanguage(i));
    else  // known
      winMain->wprintf("%C%s\n", COLOR_WHITE, l->szName);
  }

  gUserManager.DropUser(u);

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  winMain->RefreshWin();
  wattroff(winMain->Win(), A_BOLD);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintInfo_Work
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintInfo_Work(unsigned long nUin)
{
  // Print the users info to the main window
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL) return;

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  wattroff(winMain->Win(), A_BOLD);

  winMain->wprintf("%s %A(%Z%ld%A) Work Info - %Z%s\n", u->GetAlias(), A_BOLD,
                   A_BOLD, u->Uin(), A_BOLD, A_BOLD, u->StatusStr());

  winMain->wprintf("%C%ACompany Name: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCompanyName());
  winMain->wprintf("%C%ACompany City: %A%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCompanyCity());
  winMain->wprintf("%C%ACompany State: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCompanyState());
  winMain->wprintf("%C%ACompany Phone Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCompanyPhoneNumber());
  winMain->wprintf("%C%ACompany Fax Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCompanyFaxNumber());
  winMain->wprintf("%C%ACompany Address: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCompanyAddress());
  winMain->wprintf("%C%ACompany Department: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCompanyDepartment());
  winMain->wprintf("%C%ACompany Position: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCompanyPosition());
  winMain->wprintf("%C%ACompany Homepage: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->GetCompanyHomepage());

  gUserManager.DropUser(u);

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  winMain->RefreshWin();
  wattroff(winMain->Win(), A_BOLD);
}

/*----------------------------------------------------------------------------
 * CLicqConsole::PrintInfo_About
 *--------------------------------------------------------------------------*/
void CLicqConsole::PrintInfo_About(unsigned long nUin)
{
  // Print the user's about info to the main window
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  if (u == NULL)  return;

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  wattroff(winMain->Win(), A_BOLD);

  winMain->wprintf("%s %A(%Z%ld%A) About Info - %Z%s\n", u->GetAlias(), A_BOLD,
                    A_BOLD, u->Uin(), A_BOLD, A_BOLD, u->StatusStr());

  winMain->wprintf("%s\n", u->GetAbout());

  gUserManager.DropUser(u);

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  winMain->RefreshWin();
  wattroff(winMain->Win(), A_BOLD); 
}

/*---------------------------------------------------------------------------- 
 * CLicqConsole::PrintFileStat
 *--------------------------------------------------------------------------*/
void CLicqConsole::PrintFileStat(CFileTransferManager *ftman)
{
  // Get the user's name
  ICQUser *u = gUserManager.FetchUser(ftman->Uin(), LOCK_R);
  const char *szAlias = u->GetAlias();
  gUserManager.DropUser(u);
  
  // Make the title
  char szTitle[30];
  ftman->Direction() == D_RECEIVER ? strcpy(szTitle, "File from ") :
    strcpy(szTitle, "File to ");
  strcat(szTitle, szAlias);

  // Current file name and Current File # slash Total Batch Files
  PrintBoxTop(szTitle, COLOR_WHITE, 48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("%ACurrent File: %Z", A_BOLD, A_BOLD);
  winMain->wprintf(const_cast<char *>(ftman->FileName()));
  PrintBoxRight(48);
 
  // Current progress, current file xferred slash total current file size
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("%ACurrent Progress: %Z", A_BOLD, A_BOLD);
  unsigned long fCurPercent = (100 * ftman->FilePos()) / ftman->FileSize();
  winMain->wprintf("%02ld Percent", fCurPercent);
  PrintBoxRight(48);

  // Batch progress, current batch xferred slash total batch size
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("%ABatch Progress: %Z", A_BOLD, A_BOLD);
  unsigned long fBatchPercent = (100 * ftman->BatchPos()) / ftman->BatchSize();
  winMain->wprintf("%02ld Percent", fBatchPercent);
  PrintBoxRight(48);

  // Time, ETA, BPS
  waddch(winMain->Win(), ACS_VLINE);
  time_t Time = time(NULL) - ftman->StartTime();
  winMain->wprintf("%ATime: %Z%02ld:%02ld:%02ld   ", A_BOLD, A_BOLD,
    Time / 3600, (Time % 3600)/ 60, (Time % 60));

  if(ftman->BytesTransfered() == 0 || Time == 0)
  {
    winMain->wprintf("%AETA: %Z--:--:--:   ", A_BOLD, A_BOLD);
    winMain->wprintf("%ABPS: %Z---", A_BOLD, A_BOLD);
  }
  else
  {
    unsigned long nBytesLeft = ftman->FileSize() - ftman->FilePos();
    time_t nETA = (time_t)(nBytesLeft / (time_t)(ftman->BytesTransfered() / Time));
    winMain->wprintf("%AETA: %Z%02ld:%02ld:%02ld   ", A_BOLD, A_BOLD,
      nETA / 3600, (nETA % 3600)/60, (nETA % 60));
    winMain->wprintf("%ABPS: %Z%s", A_BOLD, A_BOLD, EncodeFileSize(
      ftman->BytesTransfered() / Time));
  }

  PrintBoxRight(48);

  // Close this box
  PrintBoxBottom(48);
  winMain->RefreshWin();
}
