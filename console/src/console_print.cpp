/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2012 Licq developers <licq-dev@googlegroups.com>
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
#include <cstring>
#include <ctime>

#include <licq/contactlist/group.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/icq/codes.h>
#include <licq/icq/filetransfer.h>
#include <licq/socket.h> // For ip_ntoa
#include <licq/userevents.h>

using namespace std;
using Licq::User;

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
  string lastUser;

  werase(winStatus->Win());

  unsigned short nNumOwnerEvents = 0;

  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    if (o.isLocked())
      nNumOwnerEvents = o->NewMessages();
  }

  unsigned short nNumUserEvents = Licq::User::getNumUserEvents() - nNumOwnerEvents;
  if (nNumOwnerEvents > 0)
    sprintf (szMsgStr, "System Message");
  else if (nNumUserEvents > 0)
    sprintf (szMsgStr, "%d Message%c", nNumUserEvents, nNumUserEvents == 1 ? ' ' : 's');
  else
    strcpy(szMsgStr, "No Messages");

  if (winMain->sLastContact.isValid())
  {
    Licq::UserReadGuard u(winMain->sLastContact);
    if (!u.isLocked())
      lastUser = "<Removed>";
    else
      lastUser = u->getAlias();
  }
  else
    lastUser = "<None>";

  wbkgdset(winStatus->Win(), COLOR_PAIR(COLOR_WHITE));
  mvwhline(winStatus->Win(), 0, 0, ACS_HLINE, COLS);
  //mvwaddch(winStatus->Win(), 0, COLS - USER_WIN_WIDTH - 1, ACS_BTEE);
  wmove(winStatus->Win(), 1, 0);
  wbkgdset(winStatus->Win(), COLOR_PAIR(32));

  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    if (o.isLocked())
      winStatus->wprintf("%C%A[ %C%s %C(%C%s%C) - S: %C%s %C- G: %C%s %C- M: %C%s %C- L: %C%s %C]", 29,
          A_BOLD, 5,  o->getAlias().c_str(), 29,
        5, o->accountId().c_str(), 29,
        53, o->statusString().c_str(), 29,
                       53, CurrentGroupName(), 29,
                       53, szMsgStr, 29, 53,
          lastUser.c_str(), 29);
  }

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

  {
    Licq::GroupListGuard groups;
    BOOST_FOREACH(const Licq::Group* group, **groups)
    {
      PrintBoxLeft();
      winMain->wprintf("%A%C%3d. %-19s",
          m_cColorGroupList->nAttr,
          m_cColorGroupList->nColor, j, group->name().c_str());
      PrintBoxRight(26);
      ++j;
    }
  }

  waddch(winMain->Win(), ACS_LTEE);
  for (k = 0; k < 24; k++) waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), ACS_RTEE);
  waddch(winMain->Win(), '\n');

  for (int i = 1; i <= NumSystemGroups; i++)
  {
    PrintBoxLeft();
    winMain->wprintf("%A%C*%2d. %-19s",
        m_cColorGroupList->nAttr,
        m_cColorGroupList->nColor, i, GroupsSystemNames[i]);
    PrintBoxRight(26);
  }

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
    winMain->wprintf("\"%s\"\n", ((string*)aVariables[nVar].pData)->c_str());
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
  string sz;
  struct SUser *s = NULL;
  list <SUser *>::iterator it;

  // Clear the list
  for (it = m_lUsers.begin(); it != m_lUsers.end(); it++)
  {
    if ((*it)->szLine)
      delete [] (*it)->szLine;
    delete (*it);
  }
#undef clear
  m_lUsers.clear();

  Licq::UserListGuard users;
  BOOST_FOREACH(const Licq::User* pUser, **users)
  {
    // Only show users on the current group and not on the ignore list
    if ((!userIsInGroup(pUser, myCurrentGroup) && myCurrentGroup != AllUsersGroupId) ||
        (pUser->IgnoreList() && myCurrentGroup != IgnoreListGroupId) )
      continue;

    if (!m_bShowOffline && !pUser->isOnline())
      continue;

    s = new SUser;
    sprintf(s->szKey, "%05u%010lu", pUser->status(), pUser->Touched() ^ 0xFFFFFFFF);
    s->userId = pUser->id();
    s->bOffline = !pUser->isOnline();

    unsigned status = pUser->status();

    if (status & User::InvisibleStatus)
    {
      sz = pUser->usprintf(myOtherOnlineFormat);
      s->color = m_cColorOnline;
    }
    else if (status == User::OfflineStatus)
    {
      sz = pUser->usprintf(myOfflineFormat);
      s->color = m_cColorOffline;
    }
    else if (status & User::AwayStatuses)
    {
      sz = pUser->usprintf(myAwayFormat);
      s->color = m_cColorAway;
    }
    else if (status & User::FreeForChatStatus)
    {
      sz = pUser->usprintf(myOtherOnlineFormat);
      s->color = m_cColorOnline;
    }
    else
    {
      sz = pUser->usprintf(myOnlineFormat);
      s->color = m_cColorOnline;
    }

    if (pUser->NewUser() && myCurrentGroup != NewUsersGroupId)
      s->color = m_cColorNew;

    // Create the line to printout now
    if (pUser->NewMessages() > 0)
    {
      s->szLine = new char[sz.size() + 19];
      snprintf(s->szLine, sz.size() + 19, "</%d></K>%s<!K><!%d>", s->color->nColor - 6, sz.c_str(), s->color->nColor - 6);
      s->szLine[sz.size() + 18] = '\0';
    } else {
      s->szLine = new char[sz.size() + 11];
      snprintf(s->szLine, sz.size() + 11, "</%d>%s<!%d>", s->color->nColor, sz.c_str(), s->color->nColor);
      s->szLine[sz.size() + 10] = '\0';
    }

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
}

/*---------------------------------------------------------------------------
 * CLicqConsole::PrintUsers
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintUsers()
{
  const char* title = "<C></B/40>Contacts";
  const char* ulist[1024];
  int i = 0;
  struct SScrollUser *s = NULL;
  
  werase(winUsers->Win());
  m_lScrollUsers.clear();
  for (list<SUser *>::iterator it = m_lUsers.begin();
       it != m_lUsers.end();
       it++)
  {
    s = new SScrollUser;
    s->pos = i;
    s->userId = (*it)->userId;
        s->color = (*it)->color;

    m_lScrollUsers.push_back(s);
    ulist[i++] = copyChar((*it)->szLine);
  }

  // No users, so let's tell the user that
  if (i == 0)
  {
    ulist[i++] = "No users found";
  }

  if (!cdkUserList)
  {
    cdkUserList = newCDKScroll(winUsers->CDKScreen(), 0, 0, 
                               RIGHT, LINES - 5, USER_WIN_WIDTH, const_cast<char*>(title),
                               const_cast<char**>(ulist), i, FALSE, A_NORMAL, TRUE, TRUE);
  }
  else
  {
    setCDKScrollItems (cdkUserList, const_cast<char**>(ulist), i, FALSE);
  }
  
  bindCDKObject(vSCROLL, cdkUserList, SPACE, CLicqConsole::MenuPopupWrapper, this);
  bindCDKObject(vSCROLL, cdkUserList, KEY_UP, CLicqConsole::UserListCallback, this);
  bindCDKObject(vSCROLL, cdkUserList, KEY_DOWN, CLicqConsole::UserListCallback, this);
  UserListHighlight(A_NORMAL);
  drawCDKScroll(cdkUserList, true);
  winBar->RefreshWin();
  winUsers->RefreshWin();
}

/*---------------------------------------------------------------------------
 * CLicqConsole::UserListHighlight
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserListHighlight(chtype type, chtype input)
{
  // There has got to be a better way to do this...
  list <SScrollUser *>::iterator it;
  int down;
  if (input == KEY_DOWN)
    down = 1;
  else if (input == KEY_UP)
    down = -1;
  else
    down = 0;
  for (it = m_lScrollUsers.begin(); it != m_lScrollUsers.end(); it++)
  {
    if ((*it)->pos == (cdkUserList->currentItem + down))
    {
      Licq::UserReadGuard u((*it)->userId);
      if (u.isLocked() && u->NewMessages() > 0)
        setCDKScrollHighlight(cdkUserList, COLOR_PAIR((*it)->color->nColor - 6) | type);
      else
        setCDKScrollHighlight(cdkUserList, COLOR_PAIR((*it)->color->nColor) | type);
      break;
    }
  }
}

/*---------------------------------------------------------------------------
 * CLicqConsole::UserListCallback
 *-------------------------------------------------------------------------*/
int CLicqConsole::UserListCallback(EObjectType /* cdktype */, void* /* object */, void *clientData, chtype input)
{
  CLicqConsole *me = (CLicqConsole *)clientData; 
  me->UserListHighlight(A_REVERSE, input);
  return 0;
}

/*---------------------------------------------------------------------------
 * CLicqConsole::PrintContactPopup
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintContactPopup(const char* _szAlias)
{
  char title[256];
  snprintf(title, 256, "<C></B/40>%s", _szAlias);
  const char* list[10000];
  int i = 0;

  list[i++] = "Message";
  list[i++] = "View Event";

  cdkContactPopup = newCDKScroll(winMain->CDKScreen(), 0, 0,
                                 RIGHT, 10, 20, title,
                                 const_cast<char**>(list), i, FALSE, A_REVERSE, TRUE, TRUE);

  setCDKScrollBackgroundColor(cdkContactPopup, const_cast<char*>("</40>"));
  drawCDKScroll(cdkContactPopup, true);
  winMain->RefreshWin();
}

/*---------------------------------------------------------------------------
 * CLicqConsole::PrintHelp
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintHelp()
{
  PrintBoxTop("Menu", COLOR_WHITE, 48);

  for (unsigned short i = 0; i < NUM_COMMANDS; i++)
  {
    waddch(winMain->Win(), ACS_VLINE);
    winMain->wprintf(aCommands[i].szHelp, myCommandChar[0]);
    PrintBoxRight(48);
  }

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %BF10%b to activate the contact list");
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %BF(1-%d)%b to change between consoles", MAX_CON);
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %BF%d%b to see the log", MAX_CON + 1);
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" %B<user>%b can be alias, uin,");
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("   $ (last user) or # (owner)");
  PrintBoxRight(48);

  waddch(winMain->Win(), ACS_VLINE);
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf(" To end text use \".\" (accept),");
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("   \".d/s\" (force direct/server),");
  PrintBoxRight(48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("   \".u\" (urgent), or \",\" (abort)");
  PrintBoxRight(48);

  PrintBoxBottom(48);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintHistory
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintHistory(Licq::HistoryList& lHistory, unsigned short nStart,
                                unsigned short nEnd, const char *szFrom)
{
  Licq::HistoryList::iterator it = lHistory.begin();
  unsigned short n = 0;
  for (n = 0; n < nStart && it != lHistory.end(); n++, it++) ;
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
        COLOR_WHITE, n + 1, lHistory.size(), (*it)->description().c_str(),
        (*it)->isReceiver() ? "from" : "to", szFrom,
                     szTime, (*it)->IsDirect() ? 'D' : '-',
                     (*it)->IsMultiRec() ? 'M' : '-', (*it)->IsUrgent() ? 'U' : '-',
        A_BOLD, (*it)->textLoc().c_str());

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
void CLicqConsole::PrintInfo_General(const Licq::UserId& userId)
{
  // Print the users info to the main window
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  // Some IP, Real IP and last seen stuff
  char buf[32];
  char szRealIp[32];
  strcpy(szRealIp, Licq::ip_ntoa(u->RealIp(), buf));
  time_t nLast = u->LastOnline();
  time_t nOnSince = u->OnlineSince();

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  wattroff(winMain->Win(), A_BOLD);

  winMain->wprintf("%s %A(%Z%s%A) General Info - %Z%s\n", u->getAlias().c_str(), A_BOLD,
      A_BOLD, u->accountId().c_str(), A_BOLD, A_BOLD, u->statusString().c_str());

  winMain->wprintf("%C%AName: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getFullName().c_str());
  winMain->wprintf("%C%AIp: %Z%s:%s\n", COLOR_WHITE, A_BOLD, A_BOLD,
      u->ipToString().c_str(), u->portToString().c_str());
  winMain->wprintf("%C%AReal Ip: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD,
                   szRealIp);
  winMain->wprintf("%C%AEmail 1: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("Email1").c_str());
  winMain->wprintf("%C%AEmail 2: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("Email2").c_str());
  winMain->wprintf("%C%ACity: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("City").c_str());
  winMain->wprintf("%C%AState: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("State").c_str());
  winMain->wprintf("%C%AAddress: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("Address").c_str());
  winMain->wprintf("%C%APhone Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("PhoneNumber").c_str());
  winMain->wprintf("%C%AFax Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("FaxNumber").c_str());
  winMain->wprintf("%C%ACellular Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getCellularNumber().c_str());
  winMain->wprintf("%C%AZipcode: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("Zipcode").c_str());
  winMain->wprintf("%C%ACountry: ", COLOR_WHITE, A_BOLD);
  unsigned int countryCode = u->getUserInfoUint("Country");
  if (countryCode == COUNTRY_UNSPECIFIED)
    winMain->wprintf("%CUnspecified\n", COLOR_WHITE);
  else
  {
    const SCountry* c = GetCountryByCode(countryCode);
    if (c == NULL)
      winMain->wprintf("%CUnknown (%d)\n", COLOR_WHITE, countryCode);
    else  // known
      winMain->wprintf("%C%s\n", COLOR_WHITE, c->szName);
  }
  if (u->timezone() == Licq::User::TimezoneUnknown)
    winMain->wprintf("%C%ATimezone: %ZUnknown\n", COLOR_WHITE, A_BOLD, A_BOLD);
  else
    winMain->wprintf("%C%ATimezone: %ZGMT%c%d:%02d\n", COLOR_WHITE, A_BOLD, A_BOLD,
        (u->timezone() >= 0 ? '+' : '-'), abs(u->timezone() / 3600), abs(u->timezone() / 60) % 60);
  winMain->wprintf("%C%ALast Seen: %Z%s", COLOR_WHITE, A_BOLD, A_BOLD,
    ctime(&nLast));
  if (u->isOnline())
  {
    winMain->wprintf("%C%AOnline Since: %Z%s", COLOR_WHITE, A_BOLD, A_BOLD,
      (nOnSince ? ctime(&nOnSince) : "Unknown"));
  }

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  winMain->RefreshWin();
  wattroff(winMain->Win(), A_BOLD);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::PrintInfo_More
 *-------------------------------------------------------------------------*/
void CLicqConsole::PrintInfo_More(const Licq::UserId& userId)
{
  // Print the users info to the main window
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  wattroff(winMain->Win(), A_BOLD);

  winMain->wprintf("%s %A(%Z%s%A) More Info - %Z%s\n", u->getAlias().c_str(), A_BOLD,
      A_BOLD, u->accountId().c_str(), A_BOLD, A_BOLD, u->statusString().c_str());

  unsigned int age = u->getUserInfoUint("Age");
  if (age == Licq::User::AgeUnspecified)
    winMain->wprintf("%C%AAge: %ZUnspecified\n", COLOR_WHITE, A_BOLD, A_BOLD);
  else
    winMain->wprintf("%C%AAge: %Z%d\n", COLOR_WHITE, A_BOLD, A_BOLD, age);
  unsigned int gender = u->getUserInfoUint("Gender");
  winMain->wprintf("%C%AGender: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, gender == Licq::User::GenderMale ? "Male" : gender == Licq::User::GenderFemale ? "Female" : "Unspecified");
  winMain->wprintf("%C%AHomepage: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("Homepage").c_str());
  winMain->wprintf("%C%ABirthday: %Z%d/%d/%d\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoUint("BirthDay"), u->getUserInfoUint("BirthMonth"), u->getUserInfoUint("BirthYear"));
  for (unsigned short i = 0; i < 3; i++)
  {
    char langkey[16];
    sprintf(langkey, "Language%i", i);
    unsigned int language = u->getUserInfoUint(langkey);
    winMain->wprintf("%C%ALanguage %d: ", COLOR_WHITE, A_BOLD, i + 1);
    const SLanguage* l = GetLanguageByCode(language);
    if (l == NULL)
      winMain->wprintf("%CUnknown (%d)\n", COLOR_WHITE, language);
    else  // known
      winMain->wprintf("%C%s\n", COLOR_WHITE, l->szName);
  }

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
void CLicqConsole::PrintInfo_Work(const Licq::UserId& userId)
{
  // Print the users info to the main window
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  wattroff(winMain->Win(), A_BOLD);

  winMain->wprintf("%s %A(%Z%s%A) Work Info - %Z%s\n", u->getAlias().c_str(), A_BOLD,
      A_BOLD, u->accountId().c_str(), A_BOLD, A_BOLD, u->statusString().c_str());

  winMain->wprintf("%C%ACompany Name: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyName").c_str());
  winMain->wprintf("%C%ACompany Department: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyDepartment").c_str());
  winMain->wprintf("%C%ACompany Position: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyPosition").c_str());
  winMain->wprintf("%C%ACompany Phone Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyPhoneNumber").c_str());
  winMain->wprintf("%C%ACompany Fax Number: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyFaxNumber").c_str());
  winMain->wprintf("%C%ACompany City: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyCity").c_str());
  winMain->wprintf("%C%ACompany State: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyState").c_str());
  winMain->wprintf("%C%ACompany Address: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyAddress").c_str());
  winMain->wprintf("%C%ACompany Zip Code: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyZip").c_str());
  winMain->wprintf("%C%ACompany Country: ", COLOR_WHITE, A_BOLD);
  unsigned int companyCountry = u->getUserInfoUint("CompanyCountry");
  if (companyCountry == COUNTRY_UNSPECIFIED)
    winMain->wprintf("%CUnspecified\n", COLOR_WHITE);
  else
  {
    const SCountry* c = GetCountryByCode(companyCountry);
    if (c == NULL)
      winMain->wprintf("%CUnknown (%d)\n", COLOR_WHITE, companyCountry);
    else  // known
      winMain->wprintf("%C%s\n", COLOR_WHITE, c->szName);
  }
  winMain->wprintf("%C%ACompany Homepage: %Z%s\n", COLOR_WHITE, A_BOLD, A_BOLD, u->getUserInfoString("CompanyHomepage").c_str());

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
void CLicqConsole::PrintInfo_About(const Licq::UserId& userId)
{
  // Print the user's about info to the main window
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  wattron(winMain->Win(), A_BOLD);
  for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
    waddch(winMain->Win(), ACS_HLINE);
  waddch(winMain->Win(), '\n');
  wattroff(winMain->Win(), A_BOLD);

  winMain->wprintf("%s %A(%Z%s%A) About Info - %Z%s\n", u->getAlias().c_str(), A_BOLD,
      A_BOLD, u->accountId().c_str(), A_BOLD, A_BOLD, u->statusString().c_str());

  winMain->wprintf("%s\n", u->getUserInfoString("About").c_str());

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
  string title;
  {
    Licq::UserReadGuard u(ftman->userId());

    // Make the title
    title = (ftman->isReceiver() ? "File from " : "File to ");
    title += u->getAlias();
  }

  // Current file name and Current File # slash Total Batch Files
  PrintBoxTop(title.c_str(), COLOR_WHITE, 48);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("%ACurrent File: %Z", A_BOLD, A_BOLD);
  winMain->wprintf(ftman->fileName().c_str());
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


/*----------------------------------------------------------------------------
 * CLicqConsole::PrintMacros
 *--------------------------------------------------------------------------*/
void CLicqConsole::PrintMacros()
{
  MacroList::iterator iter;

  PrintBoxTop("Macros", COLOR_WHITE, 40);

  for (iter = listMacros.begin(); iter != listMacros.end(); iter++)
  {
    PrintBoxLeft();
    winMain->wprintf("%A%C%-10s %Z->%A %-19s",
        A_BOLD,
        COLOR_WHITE,
        (*iter)->macro.c_str(), A_BOLD, A_BOLD, (*iter)->command.c_str());
    PrintBoxRight(40);
  }

  PrintBoxBottom(40);
}


