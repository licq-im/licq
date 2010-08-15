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

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <cctype>
#include <vector>
#include <list>
#include <fstream>
#include <cerrno>

#include "console.h"
#include <licq/contactlist/group.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/icq.h>
#include <licq/icqcodes.h>
#include <licq/icqdefines.h>
#include <licq/icqfiletransfer.h>
#include <licq/inifile.h>
#include <licq/logging/logservice.h>
#include <licq/logging/logutils.h>
#include <licq/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/userevents.h>

#include "event_data.h"


// Undefine what stupid ncurses defines as wclear(WINDOW *)
#undef clear

using namespace std;
using Licq::User;
using Licq::gLog;
using Licq::gPluginManager;
using Licq::gProtocolManager;

extern "C" const char *LP_Version();

const char* CLicqConsole::GroupsSystemNames[NumSystemGroups+1] = {
  "All Users",
  "Online Notify",
  "Visible List",
  "Invisible List",
  "Ignore List",
  "New Users"
};

const unsigned short NUM_STATUS = 13;
const struct SStatus aStatus[NUM_STATUS] =
  {
    { "online",     User::OnlineStatus },
    { "away",       User::OnlineStatus | User::AwayStatus },
    { "na",         User::OnlineStatus | User::NotAvailableStatus },
    { "dnd",        User::OnlineStatus | User::DoNotDisturbStatus },
    { "occupied",   User::OnlineStatus | User::OccupiedStatus },
    { "ffc",        User::OnlineStatus | User::FreeForChatStatus },
    { "offline",    User::OfflineStatus },
    { "*online",    User::OnlineStatus | User::InvisibleStatus },
    { "*away",      User::OnlineStatus | User::InvisibleStatus | User::AwayStatus },
    { "*na",        User::OnlineStatus | User::InvisibleStatus | User::NotAvailableStatus },
    { "*dnd",       User::OnlineStatus | User::InvisibleStatus | User::DoNotDisturbStatus },
    { "*occupied",  User::OnlineStatus | User::InvisibleStatus | User::OccupiedStatus },
    { "*ffc",       User::OnlineStatus | User::InvisibleStatus | User::FreeForChatStatus }
  };

const unsigned short NUM_VARIABLES = 15;
struct SVariable aVariables[NUM_VARIABLES] =
  {
    { "show_offline_users", BOOL, NULL }
    ,           // 0
    { "show_dividers", BOOL, NULL }
    ,                // 1
    { "color_online", COLOR, NULL }
    ,                // 2
    { "color_away", COLOR, NULL }
    ,                  // 3
    { "color_offline", COLOR, NULL }
    ,               // 4
    { "color_new", COLOR, NULL }
    ,                   // 5
    { "color_group_list", COLOR, NULL }
    ,            // 6
    { "color_query", COLOR, NULL }
    ,                 // 7
    { "color_info", COLOR, NULL }
    ,                  // 8
    { "color_error", COLOR, NULL }
    ,                 // 9
    { "user_online_format", STRING, NULL }
    ,         // 10
    { "user_other_online_format", STRING, NULL }
    ,   // 11
    { "user_away_format", STRING, NULL }
    ,           // 12
    { "user_offline_format", STRING, NULL }
    ,        // 13
    { "command_character", STRING, NULL }           // 14
  };

const unsigned short NUM_COLORMAPS = 15;
const struct SColorMap aColorMaps[NUM_COLORMAPS] =
  {
    { "green", COLOR_GREEN, A_NORMAL }
    ,          // 0
    { "red", COLOR_RED, A_NORMAL }
    ,              // 1
    { "cyan", COLOR_CYAN, A_NORMAL }
    ,            // 2
    { "white", COLOR_WHITE, A_NORMAL }
    ,          // 3
    { "magenta", COLOR_MAGENTA, A_NORMAL }
    ,      // 4
    { "blue", COLOR_BLUE, A_NORMAL }
    ,            // 5
    { "yellow", COLOR_YELLOW, A_NORMAL }
    ,        // 6
    { "black", COLOR_BLACK, A_NORMAL }
    ,          // 7
    { "bright_green", COLOR_GREEN, A_BOLD }
    ,     // 8
    { "bright_red", COLOR_RED, A_BOLD }
    ,         // 9
    { "bright_cyan", COLOR_CYAN, A_BOLD }
    ,       // 10
    { "bright_white", COLOR_WHITE, A_BOLD }
    ,     // 11
    { "bright_magenta", COLOR_MAGENTA, A_BOLD }
    , // 12
    { "bright_blue", COLOR_BLUE, A_BOLD }
    ,       // 13
    { "bright_yellow", COLOR_YELLOW, A_BOLD }    // 14
  };

const char MLE_HELP[] =
  "[ '.' send | '.d/s' force direct/server | '.u' send urgent | ',' abort ]";

/*---------------------------------------------------------------------------
 * CLicqConsole::Constructor
 *-------------------------------------------------------------------------*/
CLicqConsole::CLicqConsole(int /* argc */, char** /* argv */)
{
  // oh yeah, add in a variable for the
  // status window text and colors.. that'd be cool
  Licq::IniFile conf("licq_console.conf");
  conf.loadFile();
  conf.setSection("appearance");

  conf.get("ShowOfflineUsers", m_bShowOffline, true);
  conf.get("ShowDividers", m_bShowDividers, true);
  unsigned currentGroup, groupType;
  conf.get("CurrentGroup", currentGroup, 0);
  conf.get("GroupType", groupType, 0);
  myCurrentGroup = currentGroup + groupType * SystemGroupOffset;
  conf.get("ColorOnline", m_nColorOnline, 5);
  conf.get("ColorAway", m_nColorAway, 0);
  conf.get("ColorOffline", m_nColorOffline, 1);
  conf.get("ColorNew", m_nColorNew, 14);
  conf.get("ColorGroupList", m_nColorGroupList, 13);
  conf.get("ColorQuery", m_nColorQuery, 8);
  conf.get("ColorInfo", m_nColorInfo, 13);
  conf.get("ColorError", m_nColorError, 9);
  conf.get("OnlineFormat", myOnlineFormat, "%a");
  conf.get("OtherOnlineFormat", myOtherOnlineFormat, "%a [%S]");
  conf.get("AwayFormat", myAwayFormat, "%a [%S]");
  conf.get("OfflineFormat", myOfflineFormat, "%a");
  conf.get("CommandCharacter", myCommandChar, "/");
  conf.get("Backspace", m_nBackspace, KEY_BACKSPACE);

  if (conf.setSection("macros", false))
  {
    char sz[32];
    unsigned n = 0;
    conf.get("NumMacros", n, 0);
    for (unsigned short i = 1; i <= n; i++)
    {
      SMacro *mac = new SMacro;
      sprintf(sz, "Macro.%d", i);
      conf.get(sz, mac->macro);
      sprintf(sz, "Command.%d", i);
      conf.get(sz, mac->command);
      listMacros.push_back(mac);
    }
  }

  // Set the colors
  m_cColorOnline    = &aColorMaps[m_nColorOnline];
  m_cColorAway      = &aColorMaps[m_nColorAway];
  m_cColorOffline   = &aColorMaps[m_nColorOffline];
  m_cColorNew       = &aColorMaps[m_nColorNew];
  m_cColorGroupList = &aColorMaps[m_nColorGroupList];
  m_cColorQuery     = &aColorMaps[m_nColorQuery];
  m_cColorInfo      = &aColorMaps[m_nColorInfo];
  m_cColorError     = &aColorMaps[m_nColorError];

  m_lCmdHistoryIter = m_lCmdHistory.end();

  // Set the variable data pointers
  unsigned short i = 0;
  aVariables[i++].pData = &m_bShowOffline;
  aVariables[i++].pData = &m_bShowDividers;
  aVariables[i++].pData = &m_cColorOnline;
  aVariables[i++].pData = &m_cColorAway;
  aVariables[i++].pData = &m_cColorOffline;
  aVariables[i++].pData = &m_cColorNew;
  aVariables[i++].pData = &m_cColorGroupList;
  aVariables[i++].pData = &m_cColorQuery;
  aVariables[i++].pData = &m_cColorInfo;
  aVariables[i++].pData = &m_cColorError;
  aVariables[i++].pData = &myOnlineFormat;
  aVariables[i++].pData = &myOtherOnlineFormat;
  aVariables[i++].pData = &myAwayFormat;
  aVariables[i++].pData = &myOfflineFormat;
  aVariables[i++].pData = &myCommandChar;

  m_bExit = false;
  cdkUserList = 0;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::Destructor
 *-------------------------------------------------------------------------*/
CLicqConsole::~CLicqConsole()
{
  for (unsigned short i = 0; i <= MAX_CON; i++)
    delete winCon[i];
  delete winConStatus;
  delete winStatus;
  delete winPrompt;
  CWindow::EndScreen();
}

/*---------------------------------------------------------------------------
 * CLicqConsole::Shutdown
 *-------------------------------------------------------------------------*/
void CLicqConsole::Shutdown()
{
  gLog.info("Shutting down console");
  Licq::gDaemon.getLogService().unregisterLogSink(myLogSink);
  gPluginManager.unregisterGeneralPlugin();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::Run
 *-------------------------------------------------------------------------*/
int CLicqConsole::Run()
{
  CWindow::StartScreen();

  // Register with the daemon, we want to receive all signals
  m_nPipe = gPluginManager.registerGeneralPlugin(Licq::PluginSignal::SignalAll);
  m_bExit = false;

  // Create the windows
  for (unsigned short i = 0; i <= MAX_CON; i++)
  {
    winCon[i] = new CWindow(LINES - 5, COLS - USER_WIN_WIDTH - 1, 2, USER_WIN_WIDTH + 1,
                            SCROLLBACK_BUFFER, true);
    if (winCon[i]->CDKScreen() == NULL)
      return 0;
    scrollok(winCon[i]->Win(), true);
    winCon[i]->fProcessInput = &CLicqConsole::InputCommand;
    winCon[i]->data = NULL;
  }
  winCon[0]->fProcessInput = &CLicqConsole::InputLogWindow;
  winStatus = new CWindow(2, COLS, LINES - 3, 0, false);
  winPrompt = new CWindow(1, COLS, LINES - 1, 0, false);
  winConStatus = new CWindow(2, COLS, 0, 0, false);
  winStatus->SetActive(true);
  winPrompt->SetActive(true);
  winConStatus->SetActive(true);
  winBar = new CWindow(LINES - 5, 1, 2, COLS - USER_WIN_WIDTH - 1, false);
  winUsers = new CWindow(LINES - 5, USER_WIN_WIDTH, 2, 0, false, true);
  if (winUsers->CDKScreen() == NULL)
    return 0;
  winBar->SetActive(true);
  winUsers->SetActive(true);

  Licq::LogService& logService = Licq::gDaemon.getLogService();

  myLogSink.reset(new Licq::PluginLogSink);
  myLogSink->setLogLevelsFromBitmask(
      logService.getDefaultLogSink()->getLogLevelsBitmask());
  myLogSink->setLogLevel(Licq::Log::Unknown, true);
  myLogSink->setLogLevel(Licq::Log::Info, true);
  myLogSink->setLogLevel(Licq::Log::Warning, true);
  myLogSink->setLogLevel(Licq::Log::Error, true);
  logService.registerLogSink(myLogSink);

  // Disable default log sink to stop it from messing with the console
  logService.getDefaultLogSink()->setLogLevelsFromBitmask(0);

  winMain = winCon[1];
  winLog = winCon[0];
  SwitchToCon(1);

  PrintStatus();
  PrintPrompt();
  CreateUserList();
  PrintUsers();

  if (Licq::gUserManager.NumOwners() == 0)
  {
    RegistrationWizard();
  }
  else
  {
    bool pwdEmpty;
    {
      Licq::OwnerReadGuard o(LICQ_PPID);
      pwdEmpty = o->password().empty();
    }

    if (pwdEmpty)
      UserSelect();
  }

  //fd_set fdSet;
  int nResult;

  while (!m_bExit)
  {
    FD_ZERO(&fdSet);
    FD_SET(STDIN_FILENO, &fdSet);
    int maxFd = STDIN_FILENO;

    FD_SET(m_nPipe, &fdSet);
    maxFd = std::max(maxFd, m_nPipe);

    FD_SET(myLogSink->getReadPipe(), &fdSet);
    maxFd = std::max(maxFd, myLogSink->getReadPipe());

    // Check to see if we want to add in the file xfer manager..
    list<CFileTransferManager *>::iterator iter;
    for (iter = m_lFileStat.begin(); iter != m_lFileStat.end(); iter++)
    {
      FD_SET((*iter)->Pipe(), &fdSet);
      maxFd = std::max(maxFd, (*iter)->Pipe());
    }

    nResult = select(maxFd + 1, &fdSet, NULL, NULL, NULL);
    if (nResult == -1)
    {
      if (errno != EINTR)
      {
        gLog.error("Error in select(): %s", strerror(errno));
        m_bExit = true;
      }
    }
    else
    {
      if (FD_ISSET(STDIN_FILENO, &fdSet))
      {
        ProcessStdin();
        continue;
      }
      else if (FD_ISSET(m_nPipe, &fdSet))
      {
        ProcessPipe();
        continue;
      }
      else if (FD_ISSET(myLogSink->getReadPipe(), &fdSet))
      {
        ProcessLog();
        continue;
      }

      list<CFileTransferManager *>::iterator iter;
      for (iter = m_lFileStat.begin(); iter != m_lFileStat.end(); iter++)
      {
        if (FD_ISSET((*iter)->Pipe(), &fdSet))
        {
          if (!ProcessFile(*iter))
          {
            delete *iter;
#undef erase
            m_lFileStat.erase(iter);
          }

          break;
        }
      }
    }
  }

  winMain->wprintf("Exiting\n\n");
  return 0;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::DoneOptions
 *-------------------------------------------------------------------------*/
void CLicqConsole::DoneOptions()
{
  Licq::IniFile conf("licq_console.conf");
  conf.loadFile();

  conf.setSection("appearance");
  conf.set("ShowOfflineUsers", m_bShowOffline);
  conf.set("ShowDividers", m_bShowDividers);
  conf.set("CurrentGroup", myCurrentGroup % SystemGroupOffset);
  conf.set("GroupType", myCurrentGroup / SystemGroupOffset);
  conf.set("ColorOnline", m_nColorOnline);
  conf.set("ColorAway", m_nColorAway);
  conf.set("ColorOffline", m_nColorOffline);
  conf.set("ColorNew", m_nColorNew);
  conf.set("ColorGroupList", m_nColorGroupList);
  conf.set("ColorQuery", m_nColorQuery);
  conf.set("ColorInfo", m_nColorInfo);
  conf.set("ColorError", m_nColorError);
  conf.set("OnlineFormat", myOnlineFormat);
  conf.set("OtherOnlineFormat", myOtherOnlineFormat);
  conf.set("AwayFormat", myAwayFormat);
  conf.set("OfflineFormat", myOfflineFormat);
  conf.set("CommandCharacter", myCommandChar);
  conf.set("Backspace", m_nBackspace);

  conf.setSection("macros");
  char sz[32];
  unsigned short i = 1;
  conf.set("NumMacros", (unsigned short)listMacros.size());
  for (MacroList::iterator iter = listMacros.begin(); iter != listMacros.end(); iter++, i++)
  {
    sprintf(sz, "Macro.%d", i);
    conf.set(sz, (*iter)->macro);
    sprintf(sz, "Command.%d", i);
    conf.set(sz, (*iter)->command);
  }

  conf.writeFile();
}

/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessLog
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessLog()
{
  Licq::LogSink::Message::Ptr message = myLogSink->popMessage();

  short cp;
  switch (message->level)
  {
  case Licq::Log::Warning:
    cp = COLOR_YELLOW;
    break;
  case Licq::Log::Error:
    cp = COLOR_RED;
    break;
  case Licq::Log::Unknown:
    cp = COLOR_MAGENTA;
    break;
  default:
    cp = COLOR_WHITE;
    break;
  }

  using namespace Licq::LogUtils;
  const char* level = levelToShortString(message->level);
  std::string time = timeToString(message->time);

  CWindow* wins[2] = { winLog, NULL };
  if (message->level == Licq::Log::Error)
    wins[1] = winMain;

  size_t pos = message->text.find_last_not_of('\n');
  pos = (pos == std::string::npos ? 0 : pos + 1);
  std::string text = message->text.substr(0, pos);

  for (size_t i = 0; i < 2 && wins[i] != NULL; ++i)
  {
    wins[i]->wprintf("%C%s %C[%s] %s: %s\n",
                     COLOR_GREEN, time.c_str(), cp, level,
                     message->sender.c_str(), text.c_str());
    if (myLogSink->isLoggingPackets()
        && !message->packet.empty()
        && wins[i] == winLog)
    {
      wins[i]->wprintf("%C%s\n", COLOR_BLUE, packetToString(message).c_str());
    }
    wins[i]->RefreshWin();
  }
}


/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessPipe
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessPipe()
{
  char buf[16];
  read(m_nPipe, buf, 1);
  switch (buf[0])
  {
    case Licq::GeneralPlugin::PipeSignal:
    {
      Licq::PluginSignal* s = Licq::gDaemon.popPluginSignal();
      ProcessSignal(s);
      break;
    }

    case Licq::GeneralPlugin::PipeEvent:
    {
      Licq::Event* e = Licq::gDaemon.PopPluginEvent();
      ProcessEvent(e);
      break;
    }

    case Licq::GeneralPlugin::PipeShutdown:
    {
      gLog.info("Exiting console");
      m_bExit = true;
      break;
    }

    case Licq::GeneralPlugin::PipeDisable:
    case Licq::GeneralPlugin::PipeEnable:
    break;

  default:
    gLog.warning("Unknown notification type from daemon: %c", buf[0]);
  }
}


/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessSignal
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessSignal(Licq::PluginSignal* s)
{
  switch (s->signal())
  {
    case Licq::PluginSignal::SignalList:
      if (s->subSignal() == Licq::PluginSignal::ListUserRemoved)
      {
      for (unsigned short i = 0; i < MAX_CON; i++)
      {
        if (s->userId() == winCon[i]->sLastContact)
            winCon[i]->sLastContact = Licq::UserId();
      }
    }
    PrintStatus();
    CreateUserList();
    PrintUsers();
      break;
    case Licq::PluginSignal::SignalUser:
    {
      if ((Licq::gUserManager.isOwner(s->userId()) && s->subSignal() == Licq::PluginSignal::UserStatus)
          || s->subSignal() == Licq::PluginSignal::UserEvents)
        PrintStatus();
      bool isInGroup = false;
      {
        Licq::UserReadGuard u(s->userId());
        if (u.isLocked())
          isInGroup = userIsInGroup(*u, myCurrentGroup);
      }
      if (isInGroup || myCurrentGroup == 0)
      {
        CreateUserList();
        PrintUsers();
      }

      break;
    }
    case Licq::PluginSignal::SignalLogon:
    case Licq::PluginSignal::SignalLogoff:
    PrintStatus();
      break;
    case Licq::PluginSignal::SignalAddedToServer:
      gProtocolManager.updateUserAlias(s->userId());
      break;
    case Licq::PluginSignal::SignalNewProtocol:
    //ignore for now
    break;
    default:
      gLog.warning("Internal error: CLicqConsole::ProcessSignal(): Unknown "
                   "signal command received from daemon: %d", s->signal());
      break;
  }
  delete s;
}

/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessEvent
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessEvent(Licq::Event* e)
{
  if (e->Command() == ICQ_CMDxTCP_START) // direct connection check
  {
    ProcessDoneEvent(e);
    delete e;
    return;
  }

  if (e->SNAC() == 0) {
    // Not from ICQ
    ProcessDoneEvent(e); //FIXME
    return;
  }
  
  switch (e->SNAC())
  {
    // Event commands for a user
  case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxMESSAGE):
  case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SERVERxREPLYxMSG):
  case MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER):
    {
      ProcessDoneEvent(e);
      break;
    }

    // The all being meta snac
  case MAKESNAC(ICQ_SNACxFAM_VARIOUS, ICQ_SNACxMETA):
    {
      if (e->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxLAST_USER ||
          e->SubCommand() == ICQ_CMDxMETA_SEARCHxWPxFOUND)
        ProcessDoneSearch(e);
      else
        ProcessDoneEvent(e);
      break;
    }

    // Commands related to the basic operation
  case MAKESNAC(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_SETxSTATUS):
  case MAKESNAC(ICQ_SNACxFAM_BUDDY, ICQ_SNACxBDY_ADDxTOxLIST):
  case ICQ_CMDxSND_LOGON:
      if (e->Result() != Licq::Event::ResultSuccess)
      winMain->wprintf("%CLogon failed.  See the log console for details.\n", COLOR_RED);
    break;

  case ICQ_CMDxSND_REGISTERxUSER:
    // Needs to be better dealt with...
    // How's this then?
      winMain->wprintf("Registration complete!\nYour UIN is %s\n",
          Licq::gUserManager.ownerUserId(LICQ_PPID).accountId().c_str());
    winMain->fProcessInput = &CLicqConsole::InputCommand;
    PrintStatus();
    break;

  default:
    gLog.warning("Internal error: CLicqConsole::ProcessEvent(): Unknown event "
                 "SNAC received from daemon: 0x%08lX", e->SNAC());
    break;
  }
  delete e;
}

/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessFile
 *-------------------------------------------------------------------------*/
bool CLicqConsole::ProcessFile(CFileTransferManager *ftman)
{
  char buf[32];
  bool bCloseFT = false;
  read(ftman->Pipe(), buf, 32);

  CFileTransferEvent *e = NULL;

  while ((e = ftman->PopFileTransferEvent()) != NULL)
  {
    switch(e->Command())
    {
    case FT_ERRORxCONNECT:
      winMain->wprintf("%C%AFile transfer could not connect.  See network "
                       "window for details.%C%Z\n",
                       COLOR_RED, A_BOLD, COLOR_WHITE, A_BOLD);
      bCloseFT = true;
      break;

    case FT_ERRORxBIND:
      winMain->wprintf("%C%AFile transfer could not bind to a port.  See "
                       "network window for details.%C%Z\n",
                       COLOR_RED, A_BOLD, COLOR_WHITE, A_BOLD);
      bCloseFT = true;
      break;

    case FT_ERRORxRESOURCES:
      winMain->wprintf("%C%AFile transfer unable to create new thread.  See "
                       "network window for details.%C%Z\n",
                       COLOR_RED, A_BOLD, COLOR_WHITE, A_BOLD);
      bCloseFT = true;
      break;
           
    case FT_DONExFILE:
      break;

    case FT_DONExBATCH:
      winMain->wprintf("%C%AFile transfer successfuly finished.%C%Z\n",
                       COLOR_GREEN, A_BOLD, COLOR_WHITE, A_BOLD);
      bCloseFT = true;
      break;

    case FT_ERRORxCLOSED:
      winMain->wprintf("%C%AFile transfer closed.%C%Z\n",
                       COLOR_RED, A_BOLD, COLOR_WHITE, A_BOLD);
      bCloseFT = true;
      break;

    case FT_ERRORxFILE:
      winMain->wprintf("%C%AFile transfer I/O error.%C%Z\n",
                       COLOR_RED, A_BOLD, COLOR_WHITE, A_BOLD);
      bCloseFT = true;
      break;

    case FT_ERRORxHANDSHAKE:
      winMain->wprintf("%C%AFile transfer handshake error.%C%Z\n",
                       COLOR_RED, A_BOLD, COLOR_WHITE, A_BOLD);
      bCloseFT = true;
      break;

    case FT_CONFIRMxFILE:
      ftman->startReceivingFile(ftman->fileName());
      break;
    }

    if (bCloseFT)
    {
      ftman->CloseFileTransfer();
      delete e;
      return false;
    }

    delete e;
  }

  return true;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessDoneEvent
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessDoneEvent(Licq::Event* e)
{
  CWindow *win = NULL;

  unsigned short i;
  for (i = 1; i <= MAX_CON; i++)
  {
    if (winCon[i]->event != 0 && e->Equals(winCon[i]->event))
    {
      win = winCon[i];
      break;
    }
  }
  if (win == NULL)
  {
    gLog.warning("Internal error: CLicqConsole::ProcessDoneEvent(): Unknown "
                 "event from daemon: %d", e->SubCommand());
    return;
  }

  bool isOk = (e != NULL && (e->Result() == Licq::Event::ResultAcked || e->Result() == Licq::Event::ResultSuccess));

  if (e == NULL)
  {
    win->wprintf("%A%Cerror\n", A_BOLD, COLOR_RED);
  }
  else
  {
    switch (e->Result())
    {
      case Licq::Event::ResultAcked:
      case Licq::Event::ResultSuccess:
      win->wprintf("%A%Cdone\n", m_cColorInfo->nAttr, m_cColorInfo->nColor);
        break;
      case Licq::Event::ResultTimedout:
      win->wprintf("%A%Ctimed out\n", m_cColorError->nAttr, m_cColorError->nColor);
        break;
      case Licq::Event::ResultFailed:
      win->wprintf("%A%Cfailed\n", m_cColorError->nAttr, m_cColorError->nColor);
        break;
      case Licq::Event::ResultError:
      win->wprintf("%A%Cerror\n", m_cColorError->nAttr, m_cColorError->nColor);
        break;
      case Licq::Event::ResultCancelled:
      win->wprintf("%A%Ccancelled\n", m_cColorInfo->nAttr, m_cColorInfo->nColor);
      break;
    }
  }
  win->event = 0;
  if (e == NULL)
    return;

  if (!isOk)
  {
    if (e->Command() == ICQ_CMDxTCP_START &&
        (e->SubCommand() == ICQ_CMDxSUB_MSG ||
         e->SubCommand() == ICQ_CMDxSUB_URL ||
         e->SubCommand() == ICQ_CMDxSUB_FILE) )
    {
      win->wprintf("%C%ADirect send failed, send through server (y/N)? %C%Z",
                   m_cColorQuery->nColor, m_cColorQuery->nAttr, COLOR_WHITE,
                   A_BOLD);
      win->state = STATE_QUERY;
      win->data->nPos = 0;
      return;
    }
  }
  else
  {
    switch(e->Command())
    {
    case ICQ_CMDxTCP_START:
      {
        const Licq::UserEvent* ue = e->userEvent();
        if (e->SubResult() == ICQ_TCPxACK_RETURN)
        {
          Licq::UserReadGuard u(e->userId());
          win->wprintf("%s is in %s mode:\n%s\n[Send \"urgent\" ('.u') to ignore]\n",
              u->getAlias().c_str(), u->statusString().c_str(), u->autoResponse().c_str());
        }
        else if (e->SubResult() == ICQ_TCPxACK_REFUSE)
        {
          Licq::UserReadGuard u(e->userId());
          win->wprintf("%s refused %s.\n",
              u->getAlias().c_str(), ue->description().c_str());
        }
        else if(e->SubCommand() == ICQ_CMDxSUB_FILE)
        {
          const Licq::ExtendedData* ea = e->ExtendedAck();

          if (ea == NULL || ue == NULL)
          {
            gLog.error("Internal error: file request acknowledgement "
                       "without extended result");
            return;
          }

          if(!ea->accepted())
          {
            Licq::UserReadGuard u(e->userId());
            win->wprintf("%s refused file: %s\n", u->getAlias().c_str(),
                ea->response().c_str());
          }

          else
          {
            // For now don't check for a chat subcommand..
            // Invoke a file transfer manager here
            const Licq::EventFile* f = dynamic_cast<const Licq::EventFile*>(ue);
            CFileTransferManager* ftman = new CFileTransferManager(e->userId());
            m_lFileStat.push_back(ftman);

            // Now watch the file pipe
            ftman->SetUpdatesEnabled(1);
            FD_SET(ftman->Pipe(), &fdSet);

            list<string> fl;
            fl.push_back(f->filename());
            ftman->sendFiles(fl, ea->port());
          }
        }
        else
        {
          Licq::UserReadGuard u(e->userId());
          if (u.isLocked() && u->Away() && u->ShowAwayMsg())
            win->wprintf("%s\n", u->autoResponse().c_str());
        }

        break;
      } // case

    case ICQ_CMDxSND_THRUxSERVER:
    case ICQ_CMDxSND_USERxGETINFO:
    case ICQ_CMDxSND_USERxGETDETAILS:
    case ICQ_CMDxSND_UPDATExBASIC:
    case ICQ_CMDxSND_UPDATExDETAIL:
    case ICQ_CMDxSND_META:
    default:
      break;

    }
  }

  win->fProcessInput = &CLicqConsole::InputCommand;
  if (win->data != NULL)
  {
    delete win->data;
    win->data = NULL;
  }
  win->state = STATE_COMMAND;

}


/*---------------------------------------------------------------------------
* CLicqConsole::ProcessDoneSearch
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessDoneSearch(Licq::Event* e)
{
  CWindow *win = NULL;

  unsigned short i;
  for (i = 1; i <= MAX_CON; i++)
  {
    if (winCon[i]->event != 0 && e->Equals(winCon[i]->event))
    {
      win = winCon[i];
      break;
    }
  }
  if (win == NULL)
  {
    gLog.warning("Internal error: CLicqConsole::ProcessEvent(): Unknown "
                 "event from daemon: %d", e->Command());
    return;
  }

  if (e->SearchAck() != NULL && e->SearchAck()->userId().isValid())
  {
    win->wprintf("%C%s%A,%Z %s %s %A(%Z%s%A) -%Z %s %A(%Z%s%A)\n",
                 COLOR_WHITE,
        e->SearchAck()->alias().c_str(),
                 A_BOLD, A_BOLD,
        e->SearchAck()->firstName().c_str(),
        e->SearchAck()->lastName().c_str(),
                 A_BOLD, A_BOLD,
        e->SearchAck()->email().c_str(),
                 A_BOLD, A_BOLD,
        e->SearchAck()->userId().accountId().c_str(),
                 A_BOLD, A_BOLD,
        e->SearchAck()->status() == Licq::SearchData::StatusOnline ? "online" :
        e->SearchAck()->status() == Licq::SearchData::StatusOffline ? "offline" :
                 "disabled",
                 A_BOLD);
  }

  if (e->Result() == Licq::Event::ResultAcked)
    return;

  if (e->Result() == Licq::Event::ResultSuccess)
  {
    if (e->SearchAck() == NULL || e->SearchAck()->more() == 0)
    {
      win->wprintf("%A%CSearch complete.\n", m_cColorInfo->nAttr, m_cColorInfo->nColor);
    }
    else if (static_cast<long>(e->SearchAck()->more()) == -1)
    {
      win->wprintf("%A%CSearch complete.  More users found, narrow search.\n",
                   m_cColorInfo->nAttr, m_cColorInfo->nColor);
    }
    else if (e->SearchAck()->more() > 0)
    {
      win->wprintf("%A%CSearch complete.  %d more users found, narrow search.\n",
          m_cColorInfo->nAttr, m_cColorInfo->nColor, e->SearchAck()->more());
    }
  }
  else
  {
    win->wprintf("%CSearch failed.\n", COLOR_RED);
  }

  win->fProcessInput = &CLicqConsole::InputCommand;
  if (win->data != NULL)
  {
    delete win->data;
    win->data = NULL;
  }
  win->state = STATE_COMMAND;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::SwitchToCon
 *-------------------------------------------------------------------------*/
void CLicqConsole::SwitchToCon(unsigned short nCon)
{
  winMain->SetActive(false);
  winMain = winCon[nCon];
  winMain->SetActive(true);
  m_nCon = nCon;

  // Print the header
  wbkgdset(winConStatus->Win(), COLOR_PAIR(29));
  werase(winConStatus->Win());
  winConStatus->wprintf("%A[ %CLicq Console Plugin v%C%s%C (",
                        A_BOLD, 5,
                        53, LP_Version(), 29);
  if (m_nCon != 0)
    winConStatus->wprintf("%A%Cconsole %C%d", A_BOLD, 5,
                          53, m_nCon);
  else
    winConStatus->wprintf("%A%Clog console", A_BOLD, 5);
  winConStatus->wprintf("%A%C) ]", A_BOLD, 29);
  wclrtoeol(winConStatus->Win());
  wbkgdset(winConStatus->Win(), COLOR_PAIR(COLOR_WHITE));
  mvwhline(winConStatus->Win(), 1, 0, ACS_HLINE, COLS);
  //mvwaddch(winConStatus->Win(), 1, COLS - USER_WIN_WIDTH - 1, ACS_TTEE);
  winConStatus->RefreshWin();

  // Refresh the last user field
  PrintStatus();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessStdin
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessStdin()
{
  int cIn = wgetch(winPrompt->Win());

  // First check for a function key
  for (unsigned short i = 1; i <= MAX_CON; i++)
  {
    if (cIn == KEY_F(i))
    {
      SwitchToCon(i);
      return;
    }
  }
  if (cIn == KEY_F(MAX_CON + 1))
  {
    SwitchToCon(0);
    return;
  }
  if (cIn == KEY_F(MAX_CON + 2))
  {
    MenuList((char *)NULL);
    return;
  }

  // Some operating systems don't have a consistent set of settings for
  // what key should erase characters.  In particular, FC3 sets the terminal
  // erase character (kbs) for xterm to ^H instead of 127, even though the
  // rest of the operating system follows the Debian conventions.
  // This allows the user to override whatever the OS has chosen.
  if (cIn == m_nBackspace)
    cIn = KEY_BACKSPACE;

  (this->*(winMain->fProcessInput))(cIn);
}

/*---------------------------------------------------------------------------
 * CLicqConsole::InputLogWindow
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputLogWindow(int cIn)
{
  // Check for keys
  switch (cIn)
  {
  case KEY_PPAGE:
    winMain->ScrollUp();
    break;

  case KEY_NPAGE:
    winMain->ScrollDown();
    break;

  default:
    Beep();
    break;
  }
}


/*---------------------------------------------------------------------------
 * CLicqConsole::InputCommand
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputCommand(int cIn)
{
  static char szIn[1024];
  static int nPos = 0;
  static int nTabs = 0;
  static struct STabCompletion sTabCompletion;

  // Now check for keys
  switch (cIn)
  {
  case KEY_PPAGE:
    winMain->ScrollUp();
    break;

  case KEY_NPAGE:
    winMain->ScrollDown();
    break;

  case KEY_CLEAR:
  case '\014':
    {
      wrefresh(curscr);
      break;
    }


  case KEY_DOWN:
    if (m_lCmdHistoryIter == m_lCmdHistory.end())
    {
      Beep();
      break;
    }
    // Erase the old command
    while (nPos > 0)
    {
      int yp, xp;
      getyx(winPrompt->Win(), yp, xp);
      mvwdelch(winPrompt->Win(), yp, xp - 1);
      nPos--;
    }
    m_lCmdHistoryIter++;
    if (m_lCmdHistoryIter == m_lCmdHistory.end())
      szIn[0] = '\0';
    else
      strcpy(szIn, *m_lCmdHistoryIter);
    nPos = strlen(szIn);
    *winPrompt << szIn;
    break;

  case KEY_UP:
    if (m_lCmdHistoryIter == m_lCmdHistory.begin())
    {
      Beep();
      break;
    }
    // Erase the old command
    while (nPos > 0)
    {
      int yp, xp;
      getyx(winPrompt->Win(), yp, xp);
      mvwdelch(winPrompt->Win(), yp, xp - 1);
      nPos--;
    }
    m_lCmdHistoryIter--;
    strcpy(szIn, *m_lCmdHistoryIter);
    nPos = strlen(szIn);
    *winPrompt << szIn;
    break;

  case KEY_BACKSPACE:
  case KEY_DC:
  case KEY_LEFT:
  case '\b':
    {
      if (nPos == 0)
        return;
      int yp, xp;
      getyx(winPrompt->Win(), yp, xp);
      mvwdelch(winPrompt->Win(), yp, xp - 1);
      winPrompt->RefreshWin();
      nPos--;
      break;
    }

  case '\t':
    {
      // Check if this is the second tab in a row
      if (nTabs > 0)
      {
        unsigned short nCol = 0;
        vector<char *>::iterator iter;
        for (iter = sTabCompletion.vszPartialMatch.begin();
             iter != sTabCompletion.vszPartialMatch.end(); iter++)
        {
          // Add a newline if necessary
          if (nCol + strlen(*iter) + 5 >= (unsigned short)COLS)
          {
            *winMain << '\n';
            nCol = 0;
          }
          // Print out the partial matches
          winMain->wprintf("[ %A%s %Z] ", A_BOLD, *iter, A_BOLD);
          nCol += strlen(*iter) + 5;
        }
        *winMain << '\n';
        break;
      }

      // If we haven't typed anything, why should we tab?
      if (!nPos)
        break;

      szIn[nPos] = '\0';
      char *szArg = strchr(szIn, ' ');
      unsigned short nArgPos = 0;
      if (szIn[0] == myCommandChar[0] && szArg == NULL)
      { // Command completion
        TabCommand(szIn, sTabCompletion);
      }
      else if (szIn[0] != myCommandChar[0])
      { // User completion
        szArg = NULL;
        nArgPos = 0;
        TabUser(szIn, sTabCompletion);
      }
      else
      { // Argument completion
        *szArg = '\0';
        szArg++;
        while (*szArg == ' ')
          szArg++;
        nArgPos = szArg - szIn;
        // Figure out which command we have
        bool bTab = false;
        for (unsigned short i = 0; i < NUM_COMMANDS; i++)
        {
          char szTempCmd[20];
          snprintf(szTempCmd, 20, "%c%s", myCommandChar[0], aCommands[i].szName);
          if (strncasecmp(szIn, szTempCmd, strlen(szIn)) == 0)
          {
            if (!aCommands[i].fProcessTab)
              break;
            ;
            (this->*(aCommands[i].fProcessTab))(szArg, sTabCompletion);
            bTab = true;
            break;
          }
        }
        // Restore the space
        szIn[strlen(szIn)] = ' ';
        if (bTab == false)
        {
          Beep();
          break;
        }
      }

      // Process the tab results
      if (sTabCompletion.vszPartialMatch.empty())
      { // No matches
        Beep();
        winMain->wprintf("%CNo matches.\n", COLOR_RED);
        wprintw(winPrompt->Win(), "");
        break;
      }

      // Erase the old argument
      int yp, xp;
      while (nPos > nArgPos)
      {
        getyx(winPrompt->Win(), yp, xp);
        mvwdelch(winPrompt->Win(), yp, xp - 1);
        nPos--;
      }
      szIn[nPos] = '\0';
      char *szMatch = sTabCompletion.szPartialMatch;

      if (sTabCompletion.vszPartialMatch.size() == 1)
      {
        // Only one match
        // Check if there is a space in the match
        if (strchr(szMatch, ' ') != NULL)
        {
          wprintw(winPrompt->Win(), "\"%s\" ", szMatch);
          sprintf(&szIn[nPos], "\"%s\" ", szMatch);
          nPos += strlen(szMatch) + 3;
        }
        else
        {
          wprintw(winPrompt->Win(), "%s ", szMatch);
          sprintf(&szIn[nPos], "%s ", szMatch);
          nPos += strlen(szMatch) + 1;
        }
        // Set nTabs to 1 so the array is cleared
        nTabs = 1;
      }
      else
      { 
        if (szMatch == 0) break;
        // Multiple matches
        Beep();
        // Check if there is a space in the match
        if (strchr(szMatch, ' ') != NULL)
        {
          wprintw(winPrompt->Win(), "\"%s", szMatch);
          sprintf(&szIn[nPos], "\"%s", szMatch);
          nPos += strlen(szMatch) + 1;
        }
        else
        {
          wprintw(winPrompt->Win(), "%s", szMatch);
          sprintf(&szIn[nPos], "%s", szMatch);
          nPos += strlen(szMatch);
        }
        // Set nTabs to 2 so the array is saved for the next tab hit
        nTabs = 2;
      }
      free(szMatch);
      winPrompt->RefreshWin();
      break;
    }

  case '\r':
    {
      if (nPos == 0)
        break;
      szIn[nPos] = '\0';
      // Strip trailing spaces
      while (nPos != 0 && szIn[nPos - 1] == ' ')
      {
        szIn[nPos - 1] = '\0';
        nPos--;
      }
      // Save the command in the history
      if (m_lCmdHistory.empty() || strcmp(m_lCmdHistory.back(), szIn) != 0)
      {
        m_lCmdHistory.push_back(strdup(szIn));
        while(m_lCmdHistory.size() > MAX_CMD_HISTORY)
        {
          char *sz = m_lCmdHistory.front();
          free(sz);
          m_lCmdHistory.pop_front();
        }
      }
      m_lCmdHistoryIter = m_lCmdHistory.end();

      bool valid = true;
      if (szIn[0] != myCommandChar[0])
      {
        valid = ParseMacro(szIn);
      }

      if (valid)
      { // Regular command
        char *szArg = strchr(szIn, ' ');
        if (szArg != NULL)
        {
          *szArg = '\0';
          szArg++;
          // Remove any leading spaces
          while (*szArg == ' ')
            szArg++;
          if (*szArg == '\0')
            szArg = NULL;
        }
        unsigned short i;
        for (i = 0; i < NUM_COMMANDS; i++)
        {
          char szTempCmd[20];
          snprintf(szTempCmd, 20, "%c%s", myCommandChar[0], aCommands[i].szName);

          if (strncasecmp(szIn, szTempCmd, strlen(szIn)) == 0)
          {
            (this->*(aCommands[i].fProcessCommand))(szArg);
            break;
          }
        }
        if (i == NUM_COMMANDS)
          PrintBadInput(szIn);
      }

      // We are done with the current command line
      szIn[0] = '\0';
      nPos = 0;
      PrintPrompt();
      break;
    }

  case ' ':
    {
      if (nPos == 0)
      {
        Beep();
        break;
      }
      // Fall through
    }

  default:
    if (isprint(cIn))
    {
      szIn[nPos++] = (unsigned char)cIn;
      *winPrompt << (unsigned char)cIn;
    }
    else
      Beep();

  } // switch

  // Erase the sTabCompletion structure if no longer needed
  if (nTabs == 1)
  {
    vector<char *>::iterator iter;
    for (iter = sTabCompletion.vszPartialMatch.begin();
         iter != sTabCompletion.vszPartialMatch.end();
         iter++)
      free(*iter);
    sTabCompletion.vszPartialMatch.clear();
  }
  if (nTabs > 0)
    nTabs--;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::ParseMacro
 *-------------------------------------------------------------------------*/
bool CLicqConsole::ParseMacro(char *szMacro)
{
  MacroList::iterator iter;
  for (iter = listMacros.begin(); iter != listMacros.end(); iter++)
  {
    if ((*iter)->macro == szMacro)
    {
      sprintf(szMacro, "%c%s", myCommandChar[0], (*iter)->command.c_str());
      break;
    }
  }

  if (iter == listMacros.end())
  {
    winMain->wprintf("%CNo such macro \"%A%s%Z\"\n", COLOR_RED, A_BOLD,
                     szMacro, A_BOLD);
    szMacro[0] = '\0';
    return false;
  }

  return true;
}

/*---------------------------------------------------------------------------
 * CLicqConsole::SaveLastUser
 *-------------------------------------------------------------------------*/
void CLicqConsole::SaveLastUser(const Licq::UserId& userId)
{
  // Save this as the last user
  if (!winMain->sLastContact.isValid() || userId != winMain->sLastContact)
  {
    winMain->sLastContact = userId;
    PrintStatus();
  }
}

/*---------------------------------------------------------------------------
 * CLicqConsole::CurrentGroupName
 *-------------------------------------------------------------------------*/
char *CLicqConsole::CurrentGroupName()
{
  static char szGroupName[64];

  if (myCurrentGroup < SystemGroupOffset)
  {
    if (myCurrentGroup == 0)
      strcpy(szGroupName, "All Users");
    else
    {
      Licq::GroupReadGuard group(myCurrentGroup);
      if (!group.isLocked())
        strcpy(szGroupName, "Invalid Group");
      else
        strcpy(szGroupName, group->name().c_str());
    }
  }
  else
  {
    if (myCurrentGroup <= NumSystemGroups)
      strcpy(szGroupName, GroupsSystemNames[myCurrentGroup - SystemGroupOffset]);
    else
      strcpy(szGroupName, "Invalid Group");
  }
  return szGroupName;
}

bool CLicqConsole::userIsInGroup(const User* user, int groupId)
{
  if (groupId < SystemGroupOffset)
    return user->isInGroup(groupId);

  if (groupId == OnlineNotifyGroupId)
    return user->OnlineNotify();
  if (groupId == VisibleListGroupId)
    return user->VisibleList();
  if (groupId == InvisibleListGroupId)
    return user->InvisibleList();
  if (groupId == IgnoreListGroupId)
    return user->IgnoreList();
  if (groupId == NewUsersGroupId)
    return user->NewUser();

  return false;
}

/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Info
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Info(const Licq::UserId& userId, char *)
{
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputInfo;
  winMain->state = STATE_QUERY;
  winMain->data = new CData(userId);

  winMain->wprintf("%C%A"
                   "(G)eneral Info\n"
                   "(M)ore Info\n"
                   "(W)ork Info\n"
                   "(A)bout Info\n"
                   "(U)pdate Info\n"
                   "for %s (%s)? %C%Z",
                   m_cColorQuery->nColor, m_cColorQuery->nAttr,
      u->GetAlias(), u->accountId().c_str(), COLOR_WHITE, A_BOLD);
  winMain->RefreshWin();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::InputInfo
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputInfo(int cIn)
{
  CData *data = (CData *)winMain->data;

  winMain->wprintf("\n");

  switch(winMain->state)
  {
  case STATE_QUERY:
    // The input is done
    switch(tolower(cIn))
    {
    case 'g':
          PrintInfo_General(data->userId);
      break;
    case 'm':
          PrintInfo_More(data->userId);
      break;
    case 'w':
          PrintInfo_Work(data->userId);
      break;
    case 'a':
          PrintInfo_About(data->userId);
      break;
    case 'u':
        {
      winMain->wprintf("%C%AUpdate info...", m_cColorInfo->nColor,
                       m_cColorInfo->nAttr);
          winMain->event = gProtocolManager.requestUserInfo(data->userId);
      winMain->state = STATE_PENDING;
      return;
        }
    case '\r':
      break;
    default:
      winMain->wprintf("%CInvalid key.\n", COLOR_RED);
    }

    winMain->fProcessInput = &CLicqConsole::InputCommand;
    if (winMain->data != NULL)
    {
      delete winMain->data;
      winMain->data = NULL;
    }
    winMain->state = STATE_COMMAND;
    break;

  case STATE_PENDING:
    break;

  default:
    winMain->wprintf("%CInvalid state: %A%d%Z.\n", COLOR_RED, A_BOLD, A_BOLD);
  }

}



/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_View
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_View(const Licq::UserId& userId, char *)
{
  Licq::UserWriteGuard u(userId);
  if (!u.isLocked())
    return;

  if (u->NewMessages() > 0)
  {
    // Fetch the most recent event
    Licq::UserEvent* e = u->EventPop();
    wattron(winMain->Win(), A_BOLD);
    for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
      waddch(winMain->Win(), ACS_HLINE);
    waddch(winMain->Win(), '\n');
    time_t t = e->Time();
    char *szTime = ctime(&t);
    szTime[16] = '\0';
    winMain->wprintf("%B%s from %b%s%B (%b%s%B) [%b%c%c%c%B]:\n%b%s\n",
        e->description().c_str(),
                     u->isUser() ? u->GetAlias() : "Server",
                     szTime, e->IsDirect() ? 'D' : '-',
                     e->IsMultiRec() ? 'M' : '-', e->IsUrgent() ? 'U' : '-',
        e->text().c_str());
    wattron(winMain->Win(), A_BOLD);
    for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
      waddch(winMain->Win(), ACS_HLINE);
    waddch(winMain->Win(), '\n');
    winMain->RefreshWin();
    wattroff(winMain->Win(), A_BOLD);

    // Do we want to accept the file transfer?
    if (e->SubCommand() == ICQ_CMDxSUB_FILE)
      FileChatOffer(e, userId);

    delete e;
    u.unlock();
    //PrintUsers();
    //PrintStatus();
    ProcessSignal(new Licq::PluginSignal(Licq::PluginSignal::SignalUser,
        Licq::PluginSignal::UserEvents, userId));
  }
  else
  {
    winMain->wprintf("No new events.\n");
  }

}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Remove
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Remove(const Licq::UserId& userId, char *)
{
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputRemove;
  winMain->state = STATE_QUERY;
  winMain->data = new CData(userId);

  winMain->wprintf("%C%ARemove %s (%s) from contact list (y/N)? %C%Z",
                   m_cColorQuery->nColor, m_cColorQuery->nAttr,
      u->GetAlias(), u->accountId().c_str(), COLOR_WHITE, A_BOLD);
  winMain->RefreshWin();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::InputRemove
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputRemove(int cIn)
{
  CData *data = (CData *)winMain->data;
  char *sz;

  switch(winMain->state)
  {
  case STATE_QUERY:
    if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
      return;
    // The input is done
    if (strncasecmp(data->szQuery, "yes", strlen(data->szQuery)) == 0)
    {
        Licq::gUserManager.removeUser(data->userId);
      winMain->wprintf("%C%AUser removed.\n", m_cColorInfo->nColor,
                       m_cColorInfo->nAttr);
    }
    else
    {
      winMain->wprintf("%C%ARemoval aborted.\n", m_cColorInfo->nColor,
                       m_cColorInfo->nAttr);
    }
    winMain->fProcessInput = &CLicqConsole::InputCommand;
    if (winMain->data != NULL)
    {
      delete winMain->data;
      winMain->data = NULL;
    }
    winMain->state = STATE_COMMAND;
    break;

  default:
    winMain->wprintf("%CInvalid state: %A%d%Z.\n", COLOR_RED, A_BOLD, A_BOLD);
  }

}



/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_FetchAutoResponse
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_FetchAutoResponse(const Licq::UserId& userId, char *)
{
  {
    Licq::UserReadGuard u(userId);
    winMain->wprintf("%C%AFetching auto-response for %s (%s)...",
                   m_cColorInfo->nColor, m_cColorInfo->nAttr,
      u->GetAlias(), u->accountId().c_str());
    winMain->RefreshWin();
  }

  winMain->event = gLicqDaemon->icqFetchAutoResponse(userId);
  // InputMessage just to catch the cancel key
  winMain->fProcessInput = &CLicqConsole::InputMessage;
  winMain->data = NULL;
  winMain->state = STATE_PENDING;
}





int StrToRange(char *sz, int nLast, int nStart)
{
  int n;
  if (*sz == '$')
  {
    n = nLast;
    sz++;
  }
  else if (*sz == '+' || *sz == '-')
  {
    n = nStart;
  }
  else
  {
    n = atoi(sz);
    while (isdigit(*sz))
      sz++;
  }
  STRIP(sz);

  if (*sz == '+')
  {
    sz++;
    STRIP(sz);
    n += atoi(sz);
    while (isdigit(*sz))
      sz++;
  }
  else if (*sz == '-')
  {
    sz++;
    STRIP(sz);
    n -= atoi(sz);
    while (isdigit(*sz))
      sz++;
  }
  STRIP(sz);
  if (*sz != '\0')
  {
    return -1;
  }
  return n;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_History
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_History(const Licq::UserId& userId, char *szArg)
{
  Licq::HistoryList lHistory;
  char *szFrom;
  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return;
    if (!u->GetHistory(lHistory))
    {
      winMain->wprintf("Error loading history.\n");
      return;
    }
    if (Licq::gUserManager.isOwner(userId))
      szFrom = strdup("Server");
    else
      szFrom = strdup(u->GetAlias());
  }

  unsigned short nLast = lHistory.size();

  // Process the argument
  char *szStart = szArg;
  if (szStart == NULL)
  {
    if (nLast > 0 )
    {
      winMain->wprintf("%CYou must specify an event number. (1-%d)\n",
                       COLOR_RED, nLast);
    }
    else
    {
      winMain->wprintf("%CNo System Events.\n", COLOR_WHITE);
    }
    free(szFrom);
    return;
  }

  char *szEnd = strchr(szStart, ',');
  int nStart, nEnd;

  if (szEnd != NULL)
  {
    *szEnd++ = '\0';
    STRIP(szEnd);
  }
  nStart = StrToRange(szStart, nLast, winMain->nLastHistory);
  if (nStart == -1)
  {
    winMain->wprintf("%CInvalid start range: %A%s\n", COLOR_RED,
                     A_BOLD, szStart);
    free(szFrom);
    return;
  }
  else if (nStart > nLast || nStart < 1)
  {
    winMain->wprintf("%CStart value out of range, history contains %d events.\n",
                     COLOR_RED, nLast);
    free(szFrom);
    return;
  }

  if (szEnd != NULL)
  {
    nEnd = StrToRange(szEnd, nLast, nStart);
    if (nEnd == -1)
    {
      winMain->wprintf("%CInvalid end range: %A%s\n", COLOR_RED,
                       A_BOLD, szEnd);
      free(szFrom);
      return;
    }
    else if (nEnd > nLast || nEnd < 1)
    {
      winMain->wprintf("%CEnd value out of range, history contains %d events.\n",
                       COLOR_RED, nLast);
      free(szFrom);
      return;
    }
  }
  else
  {
    nEnd = nStart;
  }

  winMain->nLastHistory = nEnd;
  PrintHistory(lHistory, nStart - 1, nEnd - 1, szFrom);
  free(szFrom);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Msg
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Msg(const Licq::UserId& userId, char *)
{
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputMessage;
  winMain->state = STATE_MLE;
  winMain->data = new DataMsg(userId);

  winMain->wprintf("%BEnter message to %b%s%B (%b%s%B):\n", u->GetAlias(),
      u->accountId().c_str());
  winMain->RefreshWin();
}


/*---------------------------------------------------------------------------
 * SendDirect
 *-------------------------------------------------------------------------*/
static bool SendDirect(const Licq::UserId& userId, char c)
{
  bool bDirect = (c != 's');
  Licq::UserReadGuard u(userId);
  if (u.isLocked())
  {
    if (u->normalSocketDesc() == -1 &&
        (u->Ip() == 0 || u->Port() == 0 || !u->isOnline()))
      bDirect = false;
    else if (u->SendServer() && c != 'd' && c != 'u')
      bDirect = false;
  }
  else
  {
    bDirect = false;
  }
  return bDirect;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::InputMessage
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputMessage(int cIn)
{
  DataMsg *data = (DataMsg *)winMain->data;
  char *sz;

  switch(winMain->state)
  {
  case STATE_PENDING:
    if (cIn == CANCEL_KEY)
        Licq::gDaemon.cancelEvent(winMain->event);
    return;

  case STATE_MLE:
    {
      // Process the character as a multi-line edit window
      // If we get NULL back then we aren't done yet
      if ((sz = Input_MultiLine(data->szMsg, data->nPos, cIn)) == NULL)
        return;

      // The input is done, so process it, sz points to the '.'
      if (*sz == ',')
      {
        winMain->fProcessInput = &CLicqConsole::InputCommand;
        if (winMain->data != NULL)
        {
          delete winMain->data;
          winMain->data = NULL;
        }
        winMain->state = STATE_COMMAND;
        winMain->wprintf("%C%AMessage aborted.\n", m_cColorInfo->nColor,
                         m_cColorInfo->nAttr);
        return;
      }
      sz--;
      *sz = '\0';
      sz += 2;
      bool bDirect = SendDirect(data->userId, *sz);
      winMain->wprintf("%C%ASending message %s...", m_cColorInfo->nColor,
                       m_cColorInfo->nAttr,
                       !bDirect ? "through the server" : "direct");
      winMain->event = gProtocolManager.sendMessage(data->userId, data->szMsg,
          !bDirect, *sz == 'u');
      winMain->state = STATE_PENDING;
      break;
    }

    // If we are here then direct failed and we asked if send through server
  case STATE_QUERY:
    if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
      return;
    // The input is done
    if (strncasecmp(data->szQuery, "yes", strlen(data->szQuery)) == 0)
    {
      winMain->wprintf("%C%ASending message through the server...",
                       m_cColorInfo->nColor, m_cColorInfo->nAttr);
        winMain->event = gProtocolManager.sendMessage(data->userId, data->szMsg,
            true, false);
      winMain->state = STATE_PENDING;
    }
    else
    {
      winMain->fProcessInput = &CLicqConsole::InputCommand;
      delete winMain->data;
      winMain->data = NULL;
      winMain->state = STATE_COMMAND;
    }
    break;

  default:
    winMain->wprintf("%CInvalid state: %A%d%Z.\n", COLOR_RED, A_BOLD, A_BOLD);
  }

}

/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_SendFile
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_SendFile(const Licq::UserId& userId, char *)
{
  // Get the file name
  winMain->fProcessInput = &CLicqConsole::InputSendFile;
  winMain->state = STATE_LE;
  winMain->data = new DataSendFile(userId);

  Licq::UserReadGuard u(userId);
  winMain->wprintf("%BEnter file to send to %b%s%B (%b%s%B):\n",
      u->GetAlias(), u->accountId().c_str());
  winMain->RefreshWin();
}

/*---------------------------------------------------------------------------
 * CLicqConsole::InputSendFile
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputSendFile(int cIn)
{
  DataSendFile *data = (DataSendFile *)winMain->data;
  char *sz;

  switch(winMain->state)
  {
  case STATE_PENDING:
    if(cIn == CANCEL_KEY)
        Licq::gDaemon.cancelEvent(winMain->event);
    return;

  case STATE_LE:
    {
      // If we get NULL back, then we're not done yet
      if((sz = Input_Line(data->szFileName, data->nPos, cIn)) == NULL)
        return;

      // Check to make sure the file exists, if it doesn't then tell the
      // user it doesn't and quit sending the file.
      ifstream check_file(data->szFileName);

      if(!check_file)
      {
        winMain->fProcessInput = &CLicqConsole::InputCommand;

        if(winMain->data != NULL)
        {
          delete winMain->data;
          winMain->data = NULL;
        }

        winMain->state = STATE_COMMAND;
        winMain->wprintf("%C%AFile Transfer aborted: No such file\n",
                         m_cColorInfo->nColor, m_cColorInfo->nAttr);
        return;
      }

      check_file.close();

      // The input is done
      winMain->wprintf("%BEnter description:\n");
      winMain->state = STATE_MLE;
      data->nPos = 0;
      break;
    }
  case STATE_MLE:
    {
      // If we get NULL back, then we're not odne yet
      if((sz = Input_MultiLine(data->szDescription, data->nPos, cIn)) == NULL)
        return;

      // The input is done, so process it, sz points to '.'
      if(*sz == ',')
      {
        winMain->fProcessInput = &CLicqConsole::InputCommand;

        if(winMain->data != NULL)
        {
          delete winMain->data;
          winMain->data = NULL;
        }

        winMain->state = STATE_COMMAND;
        winMain->wprintf("%C%AFile Transfer aborted.\n",
                         m_cColorInfo->nColor, m_cColorInfo->nAttr);
        return;
      }

      *sz = '\0';
      sz++;
      bool bDirect = SendDirect(data->userId, *sz);
      bDirect = true; // XXX hack
      winMain->wprintf("%C%ASending File %s...",
                       m_cColorInfo->nColor, m_cColorInfo->nAttr,
                       !bDirect ? "trhough the server" : "direct");

      list<string> lFileList;
      lFileList.push_back(data->szFileName);

      winMain->event = gProtocolManager.fileTransferPropose(data->userId,
              data->szFileName, data->szDescription, lFileList, ICQ_TCPxMSG_NORMAL,
                       !bDirect);
      break;
    }
  case STATE_QUERY:
    break;
  case STATE_COMMAND:
    break;
  }
}

/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_SetAutoResponse
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_SetAutoResponse(const Licq::UserId& /* userId */, char *)
{
  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputAutoResponse;
  winMain->state = STATE_MLE;
  winMain->data = new DataAutoResponse();

  winMain->wprintf("%BEnter auto response:\n");
  winMain->RefreshWin();
}

/*---------------------------------------------------------------------------
 * CLicqConsole::InputAutoResponse
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputAutoResponse(int cIn)
{
  DataAutoResponse *data = (DataAutoResponse *)winMain->data;
  char *sz;

  switch(winMain->state)
  {
  case STATE_MLE:
    // Process the character as a multi-line edit window
    // If we get NULL back then we aren't done yet
    if ((sz = Input_MultiLine(data->szRsp, data->nPos, cIn)) == NULL)
      return;

    // The input is done, so process it, sz points to the '.'
    if (*sz == ',')
    {
      winMain->wprintf("%C%AAuto-response set aborted.\n",
                       m_cColorInfo->nColor, m_cColorInfo->nAttr);
    }
    else
    {
      *sz = '\0';
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
        o->setAutoResponse(data->szRsp);
      }
      winMain->wprintf("%C%AAuto-response set.\n",
                       m_cColorInfo->nColor, m_cColorInfo->nAttr);
    }
    delete winMain->data;
    winMain->data = NULL;
    winMain->fProcessInput = &CLicqConsole::InputCommand;
    winMain->state = STATE_COMMAND;
    break;

  default:
    winMain->wprintf("%CInvalid state: %A%d%Z.\n", COLOR_RED, A_BOLD, A_BOLD);
  }

}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Url
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Url(const Licq::UserId& userId, char *)
{
  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputUrl;
  winMain->state = STATE_LE;
  winMain->data = new DataUrl(userId);

  Licq::UserReadGuard u(userId);
  winMain->wprintf("%BEnter URL to %b%s%B (%b%ld%B): ",
      u->GetAlias(), u->accountId().c_str());
  winMain->RefreshWin();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::InputUrl
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputUrl(int cIn)
{
  DataUrl *data = (DataUrl *)winMain->data;
  char *sz;

  switch(winMain->state)
  {
  case STATE_PENDING:
    if (cIn == CANCEL_KEY)
        Licq::gDaemon.cancelEvent(winMain->event);
    return;

  case STATE_LE:
    // Process the character as a single-line edit window
    // If we get NULL back then we aren't done yet
    if ((sz = Input_Line(data->szUrl, data->nPos, cIn)) == NULL)
      return;
    // The input is done
    winMain->wprintf("%BEnter description:\n");
    winMain->state = STATE_MLE;
    data->nPos = 0;
    break;

  case STATE_MLE:
    {
      // Process the character as a multi-line edit window
      // If we get NULL back then we aren't done yet
      if ((sz = Input_MultiLine(data->szDesc, data->nPos, cIn)) == NULL)
        return;

      // The input is done, so process it, sz points to the '.'
      if (*sz == ',')
      {
        winMain->fProcessInput = &CLicqConsole::InputCommand;
        if (winMain->data != NULL)
        {
          delete winMain->data;
          winMain->data = NULL;
        }
        winMain->state = STATE_COMMAND;
        winMain->wprintf("%C%AURL aborted.\n",
                         m_cColorInfo->nColor, m_cColorInfo->nAttr);
        return;
      }
      *sz = '\0';
      sz++;
      bool bDirect = SendDirect(data->userId, *sz);
      winMain->wprintf("%C%ASending URL %s...",
                       m_cColorInfo->nColor, m_cColorInfo->nAttr,
                       !bDirect ? "through the server" : "direct");
      winMain->event = gProtocolManager.sendUrl(data->userId, data->szUrl,
          data->szDesc, !bDirect, *sz == 'u');
      winMain->state = STATE_PENDING;
      break;
    }

    // If we are here then direct failed and we asked if send through server
  case STATE_QUERY:
    if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
      return;
    // The input is done
    if (strncasecmp(data->szQuery, "yes", strlen(data->szQuery)) == 0)
    {
      winMain->wprintf("%C%ASending URL through the server...",
                       m_cColorInfo->nColor, m_cColorInfo->nAttr);
        winMain->event = gProtocolManager.sendUrl(data->userId, data->szUrl,
            data->szDesc, true, false);
      winMain->state = STATE_PENDING;
    }
    else
    {
      winMain->fProcessInput = &CLicqConsole::InputCommand;
      delete winMain->data;
      winMain->data = NULL;
      winMain->state = STATE_COMMAND;
    }
    break;

  default:
    winMain->wprintf("%CInvalid state: %A%d%Z.\n", COLOR_RED, A_BOLD, A_BOLD);
  }

}

/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Sms
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Sms(const Licq::UserId& userId, char *)
{
  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputSms;
  winMain->state = STATE_MLE;
  winMain->data = new DataSms(userId);
  winMain->wprintf("%BEnter SMS to %b%s%B (%b%s%B):\n", u->GetAlias(),
      u->getCellularNumber().c_str());
  winMain->RefreshWin();
}

/*---------------------------------------------------------------------------
 * CLicqConsole::InputSms
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputSms(int cIn)
{
  DataSms *data = (DataSms *)winMain->data;
  char *sz;

  switch(winMain->state)
  {
  case STATE_PENDING:
    if (cIn == CANCEL_KEY)
        Licq::gDaemon.cancelEvent(winMain->event);
    return;

  case STATE_MLE:
    {
      // Process the character as a multi-line edit window
      // If we get NULL back then we aren't done yet
      if ((sz = Input_MultiLine(data->szMsg, data->nPos, cIn)) == NULL)
        return;

      // The input is done, so process it, sz points to the '.'
      if (*sz == ',')
      {
        winMain->fProcessInput = &CLicqConsole::InputCommand;
        if (winMain->data != NULL)
        {
          delete winMain->data;
          winMain->data = NULL;
        }
        winMain->state = STATE_COMMAND;
        winMain->wprintf("%C%ASMS aborted.\n", m_cColorInfo->nColor,
                         m_cColorInfo->nAttr);
        return;
      }
      *sz = '\0';
      sz++;
      Licq::UserReadGuard u(data->userId);
      winMain->wprintf("%C%ASending SMS to %s ...", m_cColorInfo->nColor,
          m_cColorInfo->nAttr, u->getCellularNumber().c_str());
      winMain->event = gLicqDaemon->icqSendSms(data->userId,
          u->getCellularNumber().c_str(), data->szMsg);
      winMain->state = STATE_PENDING;
      break;
    }

  default:
    winMain->wprintf("%CInvalid state: %A%d%Z.\n", COLOR_RED, A_BOLD, A_BOLD);
  }

}


/*---------------------------------------------------------------------------
 * CLicqConsole::InputAuthorize
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputAuthorize(int cIn)
{
  DataMsg *data = (DataMsg *)winMain->data;
  char *sz;

  switch(winMain->state)
  {
  case STATE_MLE:
    {
      // Process the character as a multi-line edit window
      // If we get NULL back then we aren't done yet
      if ((sz = Input_MultiLine(data->szMsg, data->nPos, cIn)) == NULL)
        return;

      // The input is done, so process it, sz points to the '.'
      if (*sz == ',')
      {
        winMain->fProcessInput = &CLicqConsole::InputCommand;
        if (winMain->data != NULL)
        {
          delete winMain->data;
          winMain->data = NULL;
        }
        winMain->state = STATE_COMMAND;
        winMain->wprintf("%C%AAuthorization aborted.\n", m_cColorInfo->nColor,
                         m_cColorInfo->nAttr);
        return;
      }
      *sz = '\0';
      if (data->bUrgent)
      {
        winMain->wprintf("%C%AGranting authorizing to %s...", m_cColorInfo->nColor,
            m_cColorInfo->nAttr, data->userId.toString().c_str());
      }
      else
      {
        winMain->wprintf("%C%ARefusing authorizing to %s...", m_cColorInfo->nColor,
            m_cColorInfo->nAttr, data->userId.toString().c_str());
      }
      winMain->event = gProtocolManager.authorizeReply(data->userId, data->bUrgent, data->szMsg);

      winMain->fProcessInput = &CLicqConsole::InputCommand;
      if (winMain->data != NULL)                           
      {
        delete winMain->data;
        winMain->data = NULL;
      }
      winMain->state = STATE_COMMAND;
      break;
    }

  default:
    winMain->wprintf("%CInvalid state: %A%d%Z.\n", COLOR_RED, A_BOLD, A_BOLD);
  }

}



/*---------------------------------------------------------------------------
 * CLicqConsole::Input_Line
 *-------------------------------------------------------------------------*/
char *CLicqConsole::Input_Line(char *sz, unsigned short &n, int cIn,
                               bool bEcho)
{
  // Now check for keys
  switch (cIn)
  {
  case KEY_PPAGE:
  case KEY_NPAGE:
  case '\t':
    break;

  case KEY_BACKSPACE:
  case KEY_DC:
  case KEY_LEFT:
    {
      if (n == 0)
        break;
      int yp, xp;
      getyx(winMain->Win(), yp, xp);
      mvwdelch(winMain->Win(), yp, xp - 1);
      winMain->RefreshWin();
      n--;
      break;
    }

  case '\r':
    {
      // Print the new line
      *winMain << '\n';
      sz[n] = '\0';
      return sz;
      break;
    }

  default:
    if (isprint(cIn))
    {
      sz[n++] = (unsigned char)cIn;
      if (bEcho)
        *winMain << (unsigned char)cIn;
    }
    else
    {
      Beep();
    }

  } // switch

  return NULL;

}


/*---------------------------------------------------------------------------
 * CLicqConsole::Input_MultiLine
 *-------------------------------------------------------------------------*/
char *CLicqConsole::Input_MultiLine(char *sz, unsigned short &n, int cIn)
{
  // Now check for keys
  switch (cIn)
  {
  case KEY_PPAGE:
  case KEY_NPAGE:
  case '\t':
    break;

  case KEY_BACKSPACE:
  case KEY_DC:
  case KEY_LEFT:
    {
      if (n == 0)
        break;
      int yp, xp;
      getyx(winMain->Win(), yp, xp);

      if (xp == 0)
      {
        int newX = 0;
        int xmax = getmaxx(winMain->Win());

        // If the previous char is a new line we need to calculate the length
        // of the line to determine where to put the cursor.
        if (sz[n - 1] == '\n')
        {
          if (n >= 2)
          {
            // Find previous new line
            int pos;
            for (pos = n - 2; pos >= 0; --pos)
            {
              if (sz[pos] == '\n')
                break;
            }

            // The length of the string between the new lines
            int length = n - 2 - pos;

            newX = length % xmax;
          }
        }
        else
        {
          // Word-wrap, put pointer at end of line, removing the last char.
          newX = xmax - 1;
        }

        mvwdelch(winMain->Win(), yp - 1, newX);
      }
      else
        mvwdelch(winMain->Win(), yp, xp - 1);
      winMain->RefreshWin();
      n--;
      break;
    }

  case '\r':
    {
      // Print the new line
      *winMain << '\n';
      // Check if the line only contains a '.'
      sz[n] = '\0';
      char *szNL = strrchr(sz, '\n');
      if (szNL == NULL)
        szNL = &sz[0];
      else
        szNL++;
      if ( (( *szNL == '.' || *szNL == ',' ) && strlen( szNL ) == 1) ||
           !strcmp(szNL, ".s") || !strcmp(szNL, ".d") || !strcmp(szNL, ".u"))
        return szNL;

      sz[n++] = '\n';
      break;
    }

  default:
    if (isprint(cIn))
    {
      sz[n++] = (unsigned char)cIn;
      *winMain << (unsigned char)cIn;
    }
    else
      Beep();
    break;

  } // switch

  return NULL;

}


/*---------------------------------------------------------------------------
 * CLicqConsole::Command_Search
 *-------------------------------------------------------------------------*/
void CLicqConsole::Command_Search()
{
  // Get the input now
  winMain->fProcessInput = &CLicqConsole::InputSearch;
  winMain->state = STATE_LE;
  winMain->data = new DataSearch();

  winMain->wprintf("%A%CSearch for User (leave field blank if unknown)\n"
                   "Enter uin: ",
                   m_cColorQuery->nAttr, m_cColorQuery->nColor);

  return;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::InputSearch
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputSearch(int cIn)
{
  DataSearch *data = (DataSearch *)winMain->data;
  char *sz;

  switch(winMain->state)
  {
  case STATE_PENDING:
    {
      if (cIn == CANCEL_KEY)
      {
        if (winMain->event != 0)
          Licq::gDaemon.cancelEvent(winMain->event);
      }
      return;
    }

  case STATE_LE:
    {
      switch (data->nState)
      {
        // UIN
      case 0:
        {
          // If we get NULL back, then we're not done yet
          if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
            return;

          // Back to 0 for you!
          data->nPos = 0;

          data->szId = sz;

          if (data->szId != 0)
          {
            winMain->wprintf("%C%ASearching:\n",
                             m_cColorInfo->nColor, m_cColorInfo->nAttr);

            winMain->event = gLicqDaemon->icqSearchByUin(strtoul(data->szId, (char **)NULL, 10));
            winMain->state = STATE_PENDING;

            return;
          }

          winMain->wprintf("%A%CAlias: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 1;

          return;
        }

      case 1:
        {
          // If we get NULL back, then we're not done yet
          if ((sz = Input_Line(data->szAlias, data->nPos, cIn)) == NULL)
            return;

          // Back to 0 for you!
          data->nPos = 0;

          winMain->wprintf("%A%CFirst Name: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 2;

          return;
        }

        // First Name
      case 2:
        {
          // If we get NULL back, then we're not done yet
          if ((sz = Input_Line(data->szFirstName, data->nPos, cIn)) == NULL)
            return;

          // Back to 0 for you!
          data->nPos = 0;

          winMain->wprintf("%A%CLast Name: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 3;

          return;
        }

        // Last Name
      case 3:
        {
          // If we get NULL back, then we're not done yet
          if ((sz = Input_Line(data->szLastName, data->nPos, cIn)) == NULL)
            return;

          // Back to 0 for you!
          data->nPos = 0;

          winMain->wprintf("%A%CEmail: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 4;

          return;
        }

        // Email
      case 4:
        {
          if ((sz = Input_Line(data->szEmail, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          winMain->wprintf("%A%CMinimum Age: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 5;
          return;
        }

        // Min age
      case 5:
        {
          if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          data->nMinAge = atol(data->szQuery);
          winMain->wprintf("%A%CMaximum Age: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 6;
          return;
        }

        // Max age
      case 6:
        {
          if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          data->nMaxAge = atol(data->szQuery);
          winMain->wprintf("%A%CGender [?]: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 7;
          return;
        }

      case 7:
        {
          if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          if (data->szQuery[0] == '?')
          {
            winMain->wprintf("%C0 %A-%Z Unknown\n"
                             "1 %A-%Z Female\n"
                             "2 %A-%Z Male\n"
                             "%A%CGender: ",
                             COLOR_WHITE, A_BOLD, A_BOLD, A_BOLD, A_BOLD,
                             A_BOLD, A_BOLD, m_cColorQuery->nAttr, m_cColorQuery->nColor);
            return;
          }
          data->nGender = atol(data->szQuery);
          winMain->wprintf("%A%CLanguage [?]: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 8;
          return;
        }

      case 8:
        {
          if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          const SLanguage *l = NULL;
          if (data->szQuery[0] != '?' && data->szQuery[0] != '\0')
          {
            if (isdigit(data->szQuery[0]))
              l = GetLanguageByCode(atol(data->szQuery));
            else
              l = GetLanguageByName(data->szQuery);
          }

          // Print out list of languages
          if (l == NULL && data->szQuery[0] != '\0')
          {
            for (unsigned short i = 0; i < NUM_LANGUAGES; i++)
            {
              winMain->wprintf("%C%s %A(%Z%d%A)%s%Z",
                               COLOR_WHITE, gLanguages[i].szName,
                               A_BOLD, A_BOLD, gLanguages[i].nCode,
                               A_BOLD,
                               i == NUM_LANGUAGES - 1 ? "\n" : ", ",
                               A_BOLD);
            }
            winMain->wprintf("%A%CLanguage [?]: ",
                             m_cColorQuery->nAttr, m_cColorQuery->nColor);
            return;
          }

          data->nLanguage = (l == NULL ? LANGUAGE_UNSPECIFIED : l->nCode);
          winMain->wprintf("%A%CCity: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 9;
          return;
        }

      case 9:
        {
          if ((sz = Input_Line(data->szCity, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          winMain->wprintf("%A%CState: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 10;
          return;
        }

      case 10:
        {
          if ((sz = Input_Line(data->szState, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          winMain->wprintf("%A%CCountry [?]: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 11;
          return;
        }

      case 11:
        {
          if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          const SCountry
          *c = NULL;
          if (data->szQuery[0] != '?' && data->szQuery[0] != '\0')
          {
            if (isdigit(data->szQuery[0]))
              c = GetCountryByCode(atol(data->szQuery));
            else
              c = GetCountryByName(data->szQuery);
          }

          // Print out list of countries
          if (c == NULL && data->szQuery[0] != '\0')
          {
            for (unsigned short i = 0; i < NUM_COUNTRIES; i++)
            {
              winMain->wprintf("%C%s %A(%Z%d%A)%s%Z",
                               COLOR_WHITE, gCountries[i].szName,
                               A_BOLD, A_BOLD, gCountries[i].nCode,
                               A_BOLD,
                               i == NUM_COUNTRIES - 1 ? "\n" : ", ",
                               A_BOLD);
            }
            winMain->wprintf("%A%CCountry [?]: ",
                             m_cColorQuery->nAttr, m_cColorQuery->nColor);
            return;
          }

          data->nCountryCode = (c == NULL ? COUNTRY_UNSPECIFIED : c->nCode);
          winMain->wprintf("%A%CCompany Name: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 12;
          return;
        }

      case 12:
        {
          if ((sz = Input_Line(data->szCoName, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          winMain->wprintf("%A%CCompany Department: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 13;
          return;
        }

      case 13:
        {
          if ((sz = Input_Line(data->szCoDept, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;
          winMain->wprintf("%A%CCompany Position: ", m_cColorQuery->nAttr, m_cColorQuery->nColor);
          data->nState = 14;
          return;
        }

      case 14:
        {
          // If we get NULL back, then we're not done yet
          if ((sz = Input_Line(data->szCoPos, data->nPos, cIn)) == NULL)
            return;

          // Back to 0 for you!
          data->nPos = 0;

          winMain->wprintf("%A%COnline Only [y/n]: ", m_cColorQuery->nAttr,
                           m_cColorQuery->nColor);
          data->nState = 15;
          return;
        }

      case 15:
        {
          if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
            return;
          data->nPos = 0;

          if (data->szQuery[0] == 'y' || data->szQuery[0] == 'Y')
            data->bOnlineOnly = true;

          /*if (data->szAlias[0] == '\0' && data->szFirstName[0] == '\0' &&
              data->szLastName[0] =='\0' && data->szEmail[0] == '\0')
          {
            winMain->fProcessInput = &CLicqConsole::InputCommand;
            if (winMain->data != NULL)
            {
              delete winMain->data;
              winMain->data = NULL;
            }
            winMain->state = STATE_COMMAND;
            winMain->wprintf("%C%ASearch aborted.\n",
                             m_cColorInfo->nColor, m_cColorInfo->nAttr);
            return;
        }*/

          winMain->wprintf("%C%ASearching:\n",
                           m_cColorInfo->nColor, m_cColorInfo->nAttr);

          /*winMain->event = gLicqDaemon->icqSearchByInfo(data->szAlias, data->szFirstName,
           data->szLastName, data->szEmail);*/
          winMain->event = gLicqDaemon->icqSearchWhitePages(data->szFirstName,
                           data->szLastName, data->szAlias, data->szEmail,
                           data->nMinAge, data->nMaxAge, data->nGender, data->nLanguage,
                           data->szCity, data->szState, data->nCountryCode,
                           data->szCoName, data->szCoDept, data->szCoPos, "", data->bOnlineOnly);
          winMain->state = STATE_PENDING;

          return;
        }

      }
    }

  default:
    break;
  }
}


/*---------------------------------------------------------------------------
 * CLicqConsole::RegistrationWizard
 *-------------------------------------------------------------------------*/
void CLicqConsole::RegistrationWizard()
{

  // Get the input now
  winMain->fProcessInput = &CLicqConsole::InputRegistrationWizard;
  winMain->state = STATE_LE;
  winMain->data = new DataRegWizard();

  winMain->wprintf("%A%CWelcome to the Licq Console Registration Wizard\n\nPress 1 to register a new UIN\nPress 2 if you have a UIN and password\n\n",
                   A_BOLD, COLOR_GREEN);

  return;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::InputRegistrationWizard
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputRegistrationWizard(int cIn)
{
  DataRegWizard *data = (DataRegWizard *)winMain->data;
  char *sz;

  switch(winMain->state)
  {
  case STATE_PENDING:
    {
      if(cIn == CANCEL_KEY)
      {
        if(winMain->event != 0)
          Licq::gDaemon.cancelEvent(winMain->event);
      }
      return;
    }

  case STATE_LE:
    {
      // Make sure we need to get this
      if (data->nState == 0)
      {
        // If we get NULL back, then we're not done yet
        if ((sz = Input_Line(data->szOption, data->nPos, cIn)) == NULL)
          return;

        // Back to 0 for you!
        data->nPos = 0;
        if (data->szOption[0] == '1')
        {
          winMain->wprintf("Please enter your password: ");
          data->nState = 1;
        }
        else if(data->szOption[0] == '2')
        {
          winMain->wprintf("Please enter your UIN: ");
          data->nState = 10;
        }

        return;
      }

      // The option to register a new UIN or use an existing is in szOption now
      switch(data->szOption[0])
      {
      case '1':
        {
          // Register a new UIN
          if (data->nState == 1)
          {
            if ((sz = Input_Line(data->szPassword1, data->nPos, cIn, false)) == NULL)
              return;

            // Time to go on to the next state
            data->nState = 2;
            data->nPos = 0;

            winMain->wprintf("Verify Password: ");
            break;
          }

          if(data->nState == 2)
          {
            if((sz = Input_Line(data->szPassword2, data->nPos, cIn, false))
                == NULL)
              return;

            if(strcasecmp(data->szPassword1, data->szPassword2) != 0)
            {
              winMain->wprintf("Passwords do not match!\nPlease enter your password: ");
              data->nState = 1;
              data->nPos = 0;
              return;
            }

            winMain->state = STATE_QUERY;
            winMain->wprintf("\nSave password? (y/N) ");
          }
          break;
        }

      case '2':
        {
          // Use an existing
          if (data->nState == 10)
          {
            if ((sz = Input_Line(data->szUin, data->nPos, cIn)) == NULL)
              return;

            data->nState = 11;
            data->nPos = 0;

            winMain->wprintf("Enter your password: ");
            return;
          }

          if(data->nState == 11)
          {
            if ((sz = Input_Line(data->szPassword1, data->nPos, cIn, false)) == NULL)
              return;

            data->nState = 12;
            data->nPos = 0;

            winMain->wprintf("Verify your password: ");
            return;
          }

          if (data->nState == 12)
          {
            if ((sz = Input_Line(data->szPassword2, data->nPos, cIn, false)) == NULL)
              return;

            // Compare the 2 passwords
            if (strcasecmp(data->szPassword1, data->szPassword2) != 0)
            {
              winMain->wprintf("Passwords do not match!\nPlease enter your password: ");
              data->nState = 11;
              data->nPos = 0;
              return;
            }

            // Passwords match if we are this far, now set up the new user
            Licq::UserId ownerId(data->szUin, LICQ_PPID);
            winMain->wprintf("Registration complete for user %s\n",data->szUin);
            Licq::gUserManager.addOwner(ownerId);
            {
              Licq::OwnerWriteGuard o(LICQ_PPID);
              o->setPassword(data->szPassword1);
            }
            Licq::gDaemon.SaveConf();

            winMain->wprintf("Save password? (y/N) ");
            winMain->state = STATE_QUERY;
          }
          break;
        }

      default:
        winMain->wprintf("Invalid option: %c\n", data->szOption[0]);
      }
      break;
    }

  case STATE_QUERY:
  {
    {
      Licq::OwnerWriteGuard o(LICQ_PPID);
      o->SetSavePassword(tolower(cIn) == 'y');
    }

    if (data->szOption[0] == '1')
    {
      winMain->wprintf("\nRegistering you as a new user...\n");
      gLicqDaemon->icqRegister(data->szPassword1);
      winMain->state = STATE_PENDING;
    }
    else
    {
      winMain->wprintf("\n%ADone. Awaiting commands.%Z\n", A_BOLD, A_BOLD);
      winMain->state = STATE_COMMAND;
      winMain->fProcessInput = &CLicqConsole::InputCommand;
      PrintStatus();
    }
  }
  
  default:
    break;
  }
}

/*---------------------------------------------------------------------------
 * CLicqConsole::FileChatOffer
 *-------------------------------------------------------------------------*/
void CLicqConsole::FileChatOffer(Licq::UserEvent* e, const Licq::UserId& userId)
{
  Licq::EventFile* f = dynamic_cast<Licq::EventFile*>(e);
  // Get y or n
  winMain->fProcessInput = &CLicqConsole::InputFileChatOffer;
  winMain->state = STATE_QUERY;
  winMain->data = new DataFileChatOffer(f, userId);
  winMain->wprintf("%C%ADo you wish to accept this request? (y/N) %C%Z",
                   m_cColorQuery->nColor, m_cColorQuery->nAttr, COLOR_WHITE, A_BOLD);
  winMain->RefreshWin();

  return;
}

/*--------------------------------------------------------------------------
 * CLicqConsole::InputFileChatOffer
 *------------------------------------------------------------------------*/
void CLicqConsole::InputFileChatOffer(int cIn)
{
  DataFileChatOffer *data = (DataFileChatOffer *)winMain->data;
  char *sz;
  Licq::EventFile* f = data->f;

  string szId = data->userId.accountId();

  switch(winMain->state)
  {
  case STATE_QUERY:
    {
      switch(tolower(cIn))
      {
      case 'y':
        {
          winMain->wprintf("%C%A\nAccepting file\n", COLOR_GREEN, A_BOLD);

          // Make the ftman
          CFileTransferManager *ftman = new CFileTransferManager(data->userId);
          ftman->SetUpdatesEnabled(1);
          m_lFileStat.push_back(ftman);

          // Now watch the file pipe
          FD_SET(ftman->Pipe(), &fdSet);


          // Accept the file
          const char *home = getenv("HOME");
          ftman->receiveFiles(home);
          gProtocolManager.fileTransferAccept(data->userId,
              ftman->LocalPort(), f->Sequence(), f->MessageID()[0],
              f->MessageID()[1], f->fileDescription(), f->filename(),
              f->FileSize(), !f->IsDirect());
          winMain->fProcessInput = &CLicqConsole::InputCommand;

          if(winMain->data)
            delete winMain->data;

          break;
        }

      case 'n':
      default:
        {
          winMain->state = STATE_MLE;
          winMain->wprintf("\n%BEnter a refusal reason:\n");
          return;
        }
      }

      break;
    }

  case STATE_MLE:
    {
      if((sz = Input_MultiLine(data->szReason, data->nPos, cIn)) == NULL)
        return;

      // Don't send the "."
      data->szReason[data->nPos - 1] = '\0';

      // XXX hack
      gProtocolManager.fileTransferRefuse(data->userId,
          data->szReason, f->Sequence(), 0, 0, false);

      // We are done now
      winMain->wprintf("%ARefusing file from %s with reason: %Z%s\n",
          A_BOLD, data->userId.toString().c_str(), A_BOLD, data->szReason);
      winMain->fProcessInput = &CLicqConsole::InputCommand;

      if(winMain->data)
        delete winMain->data;

      break;
    }

  default:
    break;
  }
}

/*-------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Secure
 *-----------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Secure(const Licq::UserId& userId, char *szStatus)
{
  Licq::UserReadGuard u(userId);

  if (!Licq::gDaemon.haveCryptoSupport())
  {
    winMain->wprintf("%CYou need to recompile Licq with OpenSSL for this "
                     "feature to work!\n", COLOR_RED);
    return;
  }

  if(u->SecureChannelSupport() != Licq::SECURE_CHANNEL_SUPPORTED)
  {
    winMain->wprintf("%CThe remote end is not using a supported client.  "
                     "This may not work!\n", COLOR_RED);
  }

  bool bOpen = u->Secure();

  if(szStatus == NULL)
  {
    winMain->wprintf("%ASecure channel is %s to %s\n", A_BOLD,
                     bOpen ? "open" : "closed", u->GetAlias());
  }
  else if(strcasecmp(szStatus, "open") == 0 && bOpen)
  {
    winMain->wprintf("%ASecure channel already open to %s\n", A_BOLD,
                     u->GetAlias());
  }
  else if(strcasecmp(szStatus, "close") == 0 && !bOpen)
  {
    winMain->wprintf("%ASecure channel already closed to %s\n", A_BOLD,
                     u->GetAlias());
  }
  else if(strcasecmp(szStatus, "open") == 0)
  {
    winMain->wprintf("%ARequest secure channel with %s ... ", A_BOLD,
                     u->GetAlias());
    u.unlock();
    winMain->event = gProtocolManager.secureChannelOpen(userId);
  }
  else if(strcasecmp(szStatus, "close") == 0)
  {
    winMain->wprintf("%AClose secure channel with %s ... ", A_BOLD,
                     u->GetAlias());
    u.unlock();
    winMain->event = gProtocolManager.secureChannelClose(userId);
  }
  else
  {
    winMain->wprintf("%C<user> secure <open | close | (blank)>\n", COLOR_RED);
  }
}

/*------------------------------------------------------------------------
 * CLicqConsole::UserSelect
 *----------------------------------------------------------------------*/
void CLicqConsole::UserSelect()
{
  // Get the input now
  winMain->fProcessInput = &CLicqConsole::InputUserSelect;
  winMain->state = STATE_LE;

  //TODO which owner
  winMain->data = new DataUserSelect(Licq::gUserManager.ownerUserId(LICQ_PPID));

  Licq::OwnerReadGuard o(LICQ_PPID);
  winMain->wprintf("%A%CEnter your password for %s (%s):%C%Z\n", A_BOLD,
      COLOR_GREEN, o->getAlias().c_str(), o->accountId().c_str(), COLOR_WHITE, A_BOLD);
}

/*------------------------------------------------------------------------
 * CLicqConsole::InputUserSelect
 *----------------------------------------------------------------------*/
void CLicqConsole::InputUserSelect(int cIn)
{
  DataUserSelect *data = (DataUserSelect *)winMain->data;
  char *sz;

  switch (winMain->state)
  {
    case STATE_LE:
      if ((sz = Input_Line(data->szPassword, data->nPos, cIn, false)) == NULL)
        return;

      // Go back to the beginning
      data->nPos = 0;

      // Next stage is saving the password possibly
      winMain->wprintf("%C%ASave password? (y/N) %C%Z", COLOR_GREEN, A_BOLD,
                       COLOR_WHITE, A_BOLD);
      winMain->state = STATE_QUERY;
      break;
    
    case STATE_QUERY:
    {
      {
        Licq::OwnerWriteGuard o(LICQ_PPID);
        o->SetSavePassword(tolower(cIn) == 'y');
        o->setPassword(data->szPassword);
      }

      if (winMain->data)
      {
        delete winMain->data;
        winMain->data = 0;
      }

      winMain->wprintf("%A\nDone. Awaiting commands.%A\n", A_BOLD, A_BOLD);
      winMain->fProcessInput = &CLicqConsole::InputCommand;
      winMain->state = STATE_COMMAND;
      break;
    }

    case STATE_PENDING:
    case STATE_COMMAND:
    case STATE_MLE:
    default:
      break;
  }
}
