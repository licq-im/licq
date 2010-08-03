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
#include <cctype>
#include <cstring>
#include <string>

#include "event_data.h"
#include <licq/contactlist/group.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/icq.h>
#include <licq/pluginmanager.h>
#include <licq/protocolmanager.h>

using namespace std;
using Licq::gPluginManager;
using Licq::gProtocolManager;

const unsigned short NUM_COMMANDS = 24;
const struct SCommand aCommands[NUM_COMMANDS] =
{
  { "contacts", &CLicqConsole::MenuContactList, NULL,
    " %B%cco%bntacts",
    "Force a refresh of the contact list." },
  { "console", &CLicqConsole::MenuSwitchConsole, NULL,
    " %B%ccons%bole <num>",
    "Switch to a console." },
  { "group", &CLicqConsole::MenuGroup, NULL,
    " %B%cg%broup [ %B#%b ]",
    "Prints the group list or changes to the given group number." },
  { "clear", &CLicqConsole::MenuClear, NULL,
    " %B%ccl%bear",
    "Clears the current window." },
  { "add", &CLicqConsole::MenuAdd, NULL,
    " %B%cad%bd %B<uin>%b [ alert ]",
    "Add a user to your list by uin." },
  { "authorize", &CLicqConsole::MenuAuthorize, NULL,
    " %B%cau%bthorize <grant | refuse> %B<uin>",
    "Authorize grant or refuse  the given user." },
  { "history", &CLicqConsole::MenuHistory, &CLicqConsole::TabUser,
    " %B%chi%bstory %B<user>[.protocol]%b [ %B#%b,%B#%b ]",
    "Print the given range of events from the history.\n"
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
  { "message", &CLicqConsole::MenuMessage, &CLicqConsole::TabUser,
    " %B%cm%bessage %B<user>[.protocol]%b",
    "Send a message to a user." },
  { "url", &CLicqConsole::MenuUrl, &CLicqConsole::TabUser,
    " %B%cur%bl %B<user>%b",
    "Send a URL to a user." },
  { "file", &CLicqConsole::MenuFile, &CLicqConsole::TabUser,
    " %B%cf%bile [ %B<user>%b ]",
    "Send a file to a user or display file transfer stats." },
  { "sms", &CLicqConsole::MenuSms, &CLicqConsole::TabUser,
    " %B%csms%b %B<user>%b",
    " Send an SMS to a user."},
  { "info", &CLicqConsole::MenuInfo, &CLicqConsole::TabUser,
    " %B%ci%bnfo %B<user>%b",
    "Display user information." },
  { "view", &CLicqConsole::MenuView, &CLicqConsole::TabUser,
    " %B%cv%biew [ %B<user>[.protocol]%b ]",
    "View an incoming event." },
  { "secure", &CLicqConsole::MenuSecure, &CLicqConsole::TabUser,
    " %B%cs%becure %B<user>%b [ open | close ]",
    "Establish a secure connection to a user." },
  { "auto-response", &CLicqConsole::MenuAutoResponse, &CLicqConsole::TabUser,
    " %B%ca%buto-response [ %B<user>%b ]",
    "View a user's auto-reponse or set your own (use #)." },
  { "remove", &CLicqConsole::MenuRemove, &CLicqConsole::TabUser,
    " %B%cr%bemove %B<user>[.protocol]%b",
    "Remove a user from your contact list." },
  { "status", &CLicqConsole::MenuStatus, &CLicqConsole::TabStatus,
    " %B%cst%batus [*]<online | away | na | dnd | occupied | ffc | offline>",
    "Set your status, prefix with \"*\" for invisible mode." },
  { "search", &CLicqConsole::MenuSearch, NULL,
    " %B%csea%brch",
    "Perform a search of the ICQ network." },
  { "uins", &CLicqConsole::MenuUins, NULL,
    " %B%cui%bns",
    "Print out the uins of the users in the current group.\n"
    "Useful if the user has odd characters in their alias." },
  { "set", &CLicqConsole::MenuSet, &CLicqConsole::TabSet,
    " %B%cset%b [ %B<variable>%b [ %B<value>%b ] ]",
    "Allows the setting and viewing of options.  With no arguments\n"
    "will print all current set'able values.  With one argument will\n"
    "print the value of the given argument.\n"
    "A boolean value can be yes/true/on or no/false/off.\n"
    "Color values can be red/blue/green/cyan/magenta/white/yellow\n"
    "or bright_<color> for bright colors." },
  { "plugins", &CLicqConsole::MenuPlugins, NULL,
    " %B%cp%blugins",
    "List the currently loaded plugins." },
  { "define", &CLicqConsole::MenuDefine, NULL,
    " %B%cd%befine [ %B<macro>%b [ %B<command>%b ] ]",
    "Define a new macro, enter macros by not using '/'.\n"
    "A macro can be any string of characters not containing\n"
    "a space.  The command can be any valid command, do not\n"
    "prepend the command character when defining.\n"
    "Example: \"/define r message $\" creates a macro \"r\"\n"
    "which replies to the last user you talked to."},
  { "help", &CLicqConsole::MenuHelp, NULL,
    " %B%che%blp [ %B<command>%b ]",
    "This help screen, can also be passed a command for detailed\n"
    "information about it." },
  { "quit", &CLicqConsole::MenuQuit, NULL,
    " %B%cq%buit",
    "Quit Licq." }
};

/*---------------------------------------------------------------------------
 * CLicqConsole::MenuPopupWrapper
 *
 * Callback function for when SPACE is pressed on the cdkUserList
 *-------------------------------------------------------------------------*/
int CLicqConsole::MenuPopupWrapper(EObjectType /* cdktype */, void* /* object */, void* clientData, chtype /* key */)
{
  CLicqConsole *me = (CLicqConsole *)clientData;
  me->MenuPopup(me->cdkUserList->currentItem);
  return 1;
}

/*---------------------------------------------------------------------------
 * CLicqConsole::MenuPopup
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuPopup(int userSelected) {
  list <SScrollUser *>::iterator it;
  for (it = m_lScrollUsers.begin(); it != m_lScrollUsers.end(); it++)
  {
    if ((*it)->pos == userSelected)
    {
      int choice;
      {
        Licq::UserReadGuard u((*it)->userId);
        if (!u.isLocked())
          return;

        PrintContactPopup(u->GetAlias());
      }
      nl();
      choice = activateCDKScroll(cdkContactPopup, NULL);
      eraseCDKScroll(cdkContactPopup);
      destroyCDKScroll(cdkContactPopup);
      winMain->RefreshWin();
      if (cdkContactPopup->exitType == vNORMAL)
      {
        nonl();
        switch (choice)
        {
          case 0:
            UserCommand_Msg((*it)->userId, NULL);
            break;
          case 1:
            UserCommand_View((*it)->userId, NULL);
            break;
        }
      }
      SaveLastUser((*it)->userId);
      break;
    }
  }
}

/*---------------------------------------------------------------------------
 * CLicqConsole::MenuSwitchConsole
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuSwitchConsole(char *_szArg)
{
  int i;
  if (!_szArg)
    return;
  i = atoi(_szArg);
  if (i <= 0 || i > 9)
    winMain->wprintf("Invalid console number - valid numbers are 1-9\n");
  else
    if (i == 9)
      SwitchToCon(0); 
    else
      SwitchToCon(i);
}

/*---------------------------------------------------------------------------
 * CLicqConsole::MenuList
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuList(char* /* _szArg */)
{
  int userSelected;
  UserListHighlight(A_REVERSE);
  nl();
  userSelected = activateCDKScroll(cdkUserList, NULL);
  nonl();
  UserListHighlight(A_NORMAL);
  drawCDKScroll(cdkUserList, TRUE);
  if (cdkUserList->exitType == vNORMAL)
  {
    list <SScrollUser *>::iterator it;
    for (it = m_lScrollUsers.begin(); it != m_lScrollUsers.end(); it++)
    {
      if ((*it)->pos == userSelected)
      {
        bool newMessages;
        {
          Licq::UserReadGuard u((*it)->userId);
          if (!u.isLocked())
            return;
          newMessages = (u->NewMessages() > 0);
        }
        if (newMessages)
          UserCommand_View((*it)->userId, NULL);
        else
          UserCommand_Msg((*it)->userId, NULL);
        SaveLastUser((*it)->userId);
        break;
      }
    }
  }
}


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
   aCommands[i].szName, A_BOLD, A_BOLD, aCommands[i].szDescription);

}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuQuit
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuQuit(char *)
{
  Licq::gDaemon.Shutdown();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuPlugins
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuPlugins(char* /* _szArg */)
{
  Licq::GeneralPluginsList plugins;
  gPluginManager.getGeneralPluginsList(plugins);
  Licq::ProtocolPluginsList protocols;
  gPluginManager.getProtocolPluginsList(protocols);

  PrintBoxTop("Plugins", COLOR_BLUE, 70);
  BOOST_FOREACH(Licq::GeneralPlugin::Ptr plugin, plugins)
  {
    PrintBoxLeft();
    winMain->wprintf("[%3d] %s v%s (%s %s) - %s",
                     plugin->getId(), plugin->getName(),
                     plugin->getVersion(), plugin->getBuildDate(),
                     plugin->getBuildTime(), plugin->getStatus());
    PrintBoxRight(70);
  }

  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr protocol, protocols)
  {
    PrintBoxLeft();
    winMain->wprintf("[%3d] %s v%s",
                     protocol->getId(), protocol->getName(),
                     protocol->getVersion());
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
      if ((*iter)->macro == szArg)
      {
        winMain->wprintf("%C%AErased macro \"%s -> %s\"\n",
         m_cColorInfo->nColor, m_cColorInfo->nAttr,
         (*iter)->macro.c_str(), (*iter)->command.c_str());
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
    if ((*iter)->macro == szArg)
    {
      delete *iter;
#undef erase
      listMacros.erase(iter);
      break;
    }
  }

  // Set the macro
  SMacro *macro = new SMacro;
  macro->macro = szArg;
  macro->command = szCmd;
  listMacros.push_back(macro);

  winMain->wprintf("%A%CAdded macro \"%s -> %s\"\n",
   m_cColorInfo->nAttr, m_cColorInfo->nColor,
      macro->macro.c_str(), macro->command.c_str());

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

  unsigned short nCurrentGroup;

  // Try to change groups
  if (_szArg[0] == '*')
  {
    _szArg++;
    nCurrentGroup = atol(_szArg);

    if (nCurrentGroup > NumSystemGroups || nCurrentGroup == 0)
    {
      winMain->wprintf("%CInvalid group number (1 - %d)\n", COLOR_RED,
                       NumSystemGroups);
      return;
    }
    myCurrentGroup = nCurrentGroup + SystemGroupOffset;
    winMain->wprintf("%C%ASwitching to group *%d (%s).\n",
                     m_cColorInfo->nColor, m_cColorInfo->nAttr,
                     myCurrentGroup,
                     GroupsSystemNames[myCurrentGroup - SystemGroupOffset]);
  }
  else
  {
    nCurrentGroup = atol(_szArg);
    Licq::GroupReadGuard group(nCurrentGroup);

    if (nCurrentGroup != 0 && !group.isLocked())
    {
      winMain->wprintf("%CInvalid group number\n", COLOR_RED);
      return;
    }
    myCurrentGroup = nCurrentGroup;
    winMain->wprintf("%C%ASwitching to group %d (%s).\n",
                     m_cColorInfo->nColor, m_cColorInfo->nAttr,
                     myCurrentGroup,
                     myCurrentGroup == 0 ? "All Users" : group->name().c_str());
  }

  PrintStatus();
  CreateUserList();
  PrintUsers();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuAdd
 *-------------------------------------------------------------------------*/
//TODO: fix this for other protocols
void CLicqConsole::MenuAdd(char *szArg)
{
  char* accountId = NULL;
  char* param = NULL;

  if (szArg != NULL)
  {
    char* tokptr;
    accountId = strtok_r(szArg, " ", &tokptr);
    param = strtok_r(NULL, " ", &tokptr);
  }

  if (accountId == NULL)
  {
    winMain->wprintf("%CSpecify a UIN to add.\n", COLOR_RED);
    return;
  }

  // Try to change groups
  bool bAlert = false;
  if (param != NULL && strcasecmp(param, "alert") == 0)
    bAlert = true;

  Licq::UserId userId(accountId, LICQ_PPID);

  if (!Licq::gUserManager.addUser(userId))
  {
    winMain->wprintf("%CAdding user %s failed (duplicate user or invalid uin).\n",
        COLOR_RED, userId.toString().c_str());
    return;
  }

  winMain->wprintf("%C%AAdded user %s.\n",
      m_cColorInfo->nColor, m_cColorInfo->nAttr, userId.toString().c_str());

  if (bAlert)
  {
    gLicqDaemon->icqAlertUser(userId);
    winMain->wprintf("%C%AAlerted user %s they were added.\n",
        m_cColorInfo->nColor, m_cColorInfo->nAttr, userId.toString().c_str());
  }

}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuAuthorize
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuAuthorize(char *szArg)
{
  if (szArg == NULL)
  {
    winMain->wprintf("%CSpecify \"grant/refuse\" and a UIN/Screen Name to authorize.\n", COLOR_RED);
    return;
  }

  bool bGrant = true;

  if (strncasecmp(szArg, "grant", 5) == 0)
  {
    bGrant = true;
    szArg += 5;
  }
  else if (strncasecmp(szArg, "refuse", 6) == 0)
  {
    bGrant = false;
    szArg += 6;
  }
  Licq::UserId userId(szArg, LICQ_PPID);

  // Get the input now
  winMain->fProcessInput = &CLicqConsole::InputAuthorize;
  winMain->state = STATE_MLE;
  DataMsg *data = new DataMsg(userId);
  data->bUrgent = bGrant;
  winMain->data = data;

  winMain->wprintf("%A%CEnter authorization message:\n",
                   m_cColorQuery->nAttr, m_cColorQuery->nColor);

  return;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuStatus
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuStatus(char *_szArg)
{
  unsigned status = Licq::User::OnlineStatus;
  unsigned long i;

  if (_szArg == NULL)
  {
    winMain->wprintf("%CSpecify status.\n", COLOR_RED);
    return;
  }

  // Find the status
  for (i = 0; i < NUM_STATUS; i++)
  {
    if (strcasecmp(_szArg, aStatus[i].szName) == 0)
    {
      status = aStatus[i].nId;
      break;
    }
  }
  // Check that we found it
  if (i == NUM_STATUS)
  {
    winMain->wprintf("%CInvalid status: %A%s\n", COLOR_RED, A_BOLD, _szArg);
    return;
  }

  //set same status for all protocols for now
  Licq::ProtocolPluginsList protocols;
  gPluginManager.getProtocolPluginsList(protocols);
  BOOST_FOREACH(Licq::ProtocolPlugin::Ptr protocol, protocols)
  {
    unsigned long nPPID = protocol->getProtocolId();
    gProtocolManager.setStatus(Licq::gUserManager.ownerUserId(nPPID), status);
  }
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
 * CLicqConsole::MenuUins
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuUins(char *)
{
  list <SUser *>::iterator it;

  for (it = m_lUsers.begin(); it != m_lUsers.end(); it++)
  {
    Licq::UserReadGuard u((*it)->userId);
    winMain->wprintf("%s %A-%Z %s\n", u->getAlias().c_str(), A_BOLD, A_BOLD, u->accountId().c_str());
  }
}

/*---------------------------------------------------------------------------
 * CLicqConsole::GetContactFromArg
 *-------------------------------------------------------------------------*/
bool CLicqConsole::GetContactFromArg(char **p_szArg, Licq::UserId& userId)
{
  char *szAlias, *szCmd;
  char *szArg = *p_szArg;
  unsigned long nPPID = 0;
  userId = Licq::UserId();

  if (szArg == NULL) {
    return true;
  }
  string strArg(szArg);
  string::size_type nPos = strArg.find_last_of(".");
  if (nPos != string::npos)
  {
    string::size_type s = strArg.find_last_of(" ");
    string strProtocol(strArg, nPos + 1, (s == string::npos) ? strArg.size() : s - nPos - 1);

    Licq::ProtocolPluginsList protocols;
    gPluginManager.getProtocolPluginsList(protocols);
    BOOST_FOREACH(Licq::ProtocolPlugin::Ptr protocol, protocols)
    {
      if (strcasecmp(protocol->getName(), strProtocol.c_str()) == 0)
      {
        nPPID = protocol->getProtocolId();
        szArg[strArg.find_last_of(".")] = '\0';
        string tmp(strArg, 0, nPos);
        tmp.append(strArg, s, strArg.size());
        szArg = (char *)tmp.c_str();
        break;
      }
    }
  }

  // Check if the alias is quoted
  if (szArg[0] == '"')
  {
    szAlias = &szArg[1];
    szCmd = strchr(&szArg[1], '"');
    if (szCmd == NULL)
    {
      winMain->wprintf("%CUnbalanced quotes.\n", COLOR_RED);
      return false;
    }
    *szCmd++ = '\0';
    szCmd = strchr(szCmd, ' ');
  }
  else if (szArg[0] == '#')
  {
    *p_szArg = NULL;
    userId = Licq::gUserManager.ownerUserId(LICQ_PPID);
    return true;
  }
  else if (szArg[0] == '$')
  {
    *p_szArg = NULL;
    userId = winMain->sLastContact;
    return true;
  }
  else
  {
    szAlias = szArg;
    szCmd = strchr(szArg, ' ');
  }

  if (szCmd != NULL)
  {
    *szCmd++ = '\0';
    STRIP(szCmd);
  }
  *p_szArg = szCmd;

  {
    Licq::UserListGuard userList;
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard u(user);
      if ((nPPID && u->protocolId() == nPPID && strcasecmp(szAlias, u->getAlias().c_str()) == 0) ||
          (!nPPID && strcasecmp(szAlias, u->GetAlias()) == 0))
      {
        userId = u->id();
        break;
      }
      else if ((nPPID && u->protocolId() == nPPID && strcasecmp(szAlias, u->accountId().c_str()) == 0) ||
          (!nPPID && strcasecmp(szAlias, u->accountId().c_str()) == 0))
      {
        userId = u->id();
        break;
      }
    }
  }

  if (!userId.isValid())
  {
    winMain->wprintf("%CInvalid user: %A%s\n", COLOR_RED, A_BOLD, szAlias);
    userId = Licq::UserId();
    return false;
  }
  SaveLastUser(userId);
  return true;
}



/*---------------------------------------------------------------------------
 * CLicqConsole::MenuMessage
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuMessage(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (!userId.isValid())
    winMain->wprintf("%CYou must specify a user to send a message to.\n", COLOR_RED);
  else
    UserCommand_Msg(userId, sz);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuInfo
 *-------------------------------------------------------------------------*/
//TODO: fix this for other protocols
void CLicqConsole::MenuInfo(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (Licq::gUserManager.isOwner(userId))
    UserCommand_Info(Licq::gUserManager.ownerUserId(LICQ_PPID), sz);
  else
    UserCommand_Info(userId, sz);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuUrl
 *-------------------------------------------------------------------------*/
//TODO: fix this for other protocols
void CLicqConsole::MenuUrl(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (Licq::gUserManager.isOwner(userId))
    winMain->wprintf("%CYou can't send URLs to yourself!\n", COLOR_RED);
  else if (!userId.isValid())
    winMain->wprintf("%CYou must specify a user to send a URL to.\n", COLOR_RED);
  else
    UserCommand_Url(userId, sz);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuSms
 *-------------------------------------------------------------------------*/
//TODO: fix this for other protocols
void CLicqConsole::MenuSms(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (!userId.isValid())
    winMain->wprintf("%CInvalid user\n", COLOR_RED);
  else
    UserCommand_Sms(userId, sz);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuView
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuView(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (userId.isValid())
  {
    UserCommand_View(userId, sz);
    return;
  }

    // Do nothing if there are no events pending
  if (Licq::User::getNumUserEvents() == 0)
    return;

    // Do icq system messages first
  unsigned short nNumMsg;
  {
    Licq::OwnerReadGuard o(LICQ_PPID);
    nNumMsg = o->NewMessages();
  }
    if (nNumMsg > 0)
    {
      //TODO which owner?
    UserCommand_View(Licq::gUserManager.ownerUserId(LICQ_PPID), NULL);
      return;
    }

  {
    time_t t = time(NULL);
    Licq::UserListGuard users;
    BOOST_FOREACH(const Licq::User* pUser, **users)
    {
      if (pUser->NewMessages() > 0 && pUser->Touched() <= t)
      {
        userId = pUser->id();
        t = pUser->Touched();
      }
    }
  }

  if (userId.isValid())
    UserCommand_View(userId, NULL);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuSecure
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuSecure(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (Licq::gUserManager.isOwner(userId))
    winMain->wprintf("%CYou can't establish a secure connection to yourself!\n", COLOR_RED);
  else if (!userId.isValid())
    winMain->wprintf("%CYou must specify a user to talk to.\n", COLOR_RED);
  else
    UserCommand_Secure(userId, sz);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuFile
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuFile(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (Licq::gUserManager.isOwner(userId))
    winMain->wprintf("%CYou can't send files to yourself!\n", COLOR_RED);
  else if (!userId.isValid())
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
    {
      winMain->wprintf("%A%CNo current file transfers.\n",
       m_cColorInfo->nAttr,
       m_cColorInfo->nColor);
    }
  }
  else
    UserCommand_SendFile(userId, sz);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuAutoResponse
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuAutoResponse(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (Licq::gUserManager.isOwner(userId))
  {
    wattron(winMain->Win(), A_BOLD);
    for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
      waddch(winMain->Win(), ACS_HLINE);
    waddch(winMain->Win(), '\n');
    {
      Licq::OwnerReadGuard o(LICQ_PPID);
      winMain->wprintf("%B%CAuto response:\n%b%s\n",
          COLOR_WHITE, o->autoResponse().c_str());
    }
    wattron(winMain->Win(), A_BOLD);
    for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
      waddch(winMain->Win(), ACS_HLINE);
    waddch(winMain->Win(), '\n');
    winMain->RefreshWin();
    wattroff(winMain->Win(), A_BOLD);
  }
  else if (!userId.isValid())
    UserCommand_SetAutoResponse(Licq::UserId(), sz);
  else
    UserCommand_FetchAutoResponse(Licq::UserId(), sz);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuRemove
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuRemove(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (Licq::gUserManager.isOwner(userId))
    winMain->wprintf("%CYou can't remove yourself!\n", COLOR_RED);
  else if (!userId.isValid())
    winMain->wprintf("%CYou must specify a user to remove.\n", COLOR_RED);
  else
    UserCommand_Remove(userId, sz);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::MenuHistory
 *-------------------------------------------------------------------------*/
void CLicqConsole::MenuHistory(char *szArg)
{
  char *sz = szArg;
  Licq::UserId userId;
  if (!GetContactFromArg(&sz, userId))
    return;

  if (!userId.isValid())
    winMain->wprintf("%CYou must specify a user to view history.\n", COLOR_RED);
  else
    UserCommand_History(userId, sz);
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
        strcasecmp(szValue, "1") == 0 ||
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
    *((string*)aVariables[nVariable].pData) = &szValue[1];
    break;

  case INT:
    *(int *)aVariables[nVariable].pData = atoi(szValue);
    break;
  }

  // Save it all
  DoneOptions();
}


/*-----------------------------------------------------------------------
 * CLicqConsole::MenuClear
 *---------------------------------------------------------------------*/
void CLicqConsole::MenuClear(char *)
{
  for (unsigned short i = 0; i < winMain->Rows(); i++)
    winMain->wprintf("\n");
}


/*-----------------------------------------------------------------------
 * CLicqConsole::MenuSearch
 *---------------------------------------------------------------------*/
void CLicqConsole::MenuSearch(char *)
{
  Command_Search();
}


