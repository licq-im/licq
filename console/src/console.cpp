#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <ctype.h>
#include <vector.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno
#endif

#include "console.h"
#include "log.h"
#include "icqd.h"
#include "event_data.h"
#include "eventdesc.h"

// Undefine what stupid ncurses defines as wclear(WINDOW *)
#undef clear()

extern "C" const char *LP_Version();

const unsigned short NUM_STATUS = 13;
const struct SStatus aStatus[NUM_STATUS] =
{
  { "online", ICQ_STATUS_ONLINE },
  { "away", ICQ_STATUS_AWAY },
  { "na", ICQ_STATUS_NA },
  { "dnd", ICQ_STATUS_DND },
  { "occupied", ICQ_STATUS_OCCUPIED },
  { "ffc", ICQ_STATUS_FREEFORCHAT },
  { "offline", ICQ_STATUS_OFFLINE },
  { "*online", ICQ_STATUS_ONLINE },
  { "*away", ICQ_STATUS_AWAY },
  { "*na", ICQ_STATUS_NA },
  { "*dnd", ICQ_STATUS_DND },
  { "*occupied", ICQ_STATUS_OCCUPIED },
  { "*ffc", ICQ_STATUS_FREEFORCHAT }
};

const unsigned short NUM_VARIABLES = 12;
struct SVariable aVariables[NUM_VARIABLES] =
{
  { "show_offline_users", BOOL, NULL },
  { "show_dividers", BOOL, NULL },
  { "color_online", COLOR, NULL },
  { "color_away", COLOR, NULL },
  { "color_offline", COLOR, NULL },
  { "color_new", COLOR, NULL },
  { "color_group_list", COLOR, NULL },
  { "color_query", COLOR, NULL },
  { "color_info", COLOR, NULL },
  { "user_online_format", STRING, NULL },
  { "user_away_format", STRING, NULL },
  { "user_offline_format", STRING, NULL }
};

const unsigned short NUM_COLORMAPS = 15;
const struct SColorMap aColorMaps[NUM_COLORMAPS] =
{
  { "green", COLOR_GREEN, A_NORMAL },          // 0
  { "red", COLOR_RED, A_NORMAL },              // 1
  { "cyan", COLOR_CYAN, A_NORMAL },            // 2
  { "white", COLOR_WHITE, A_NORMAL },          // 3
  { "magenta", COLOR_MAGENTA, A_NORMAL },      // 4
  { "blue", COLOR_BLUE, A_NORMAL },            // 5
  { "yellow", COLOR_YELLOW, A_NORMAL },        // 6
  { "black", COLOR_BLACK, A_NORMAL },          // 7
  { "bright_green", COLOR_GREEN, A_BOLD },     // 8
  { "bright_red", COLOR_RED, A_BOLD },         // 9
  { "bright_cyan", COLOR_CYAN, A_BOLD },       // 10
  { "bright_white", COLOR_WHITE, A_BOLD },     // 11
  { "bright_magenta", COLOR_MAGENTA, A_BOLD }, // 12
  { "bright_blue", COLOR_BLUE, A_BOLD },       // 13
  { "bright_yellow", COLOR_YELLOW, A_BOLD }    // 14
};

const char MLE_HELP[] =
  "[ '.' send | '.s' send server | '.u' send urgent | ',' abort ]";



/*---------------------------------------------------------------------------
 * CLicqConsole::Constructor
 *-------------------------------------------------------------------------*/
CLicqConsole::CLicqConsole(int argc, char **argv)
{
  CWindow::StartScreen();

  m_bShowOffline = true;
  m_bShowDividers = true;
  m_nCurrentGroup = 2;
  m_nGroupType = GROUPS_USER;
  m_cColorOnline = &aColorMaps[5];
  m_cColorAway = &aColorMaps[0];
  m_cColorOffline = &aColorMaps[1];
  m_cColorNew = &aColorMaps[14];
  m_cColorGroupList = &aColorMaps[13];
  m_cColorQuery = &aColorMaps[8];
  m_cColorInfo = &aColorMaps[13];
  m_cColorError = &aColorMaps[9];
  strcpy(m_szOnlineFormat, "%-20a");
  strcpy(m_szAwayFormat, "%-20a[%6S]");
  strcpy(m_szOfflineFormat, "%-20a");
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
  aVariables[i++].pData = m_szOnlineFormat;
  aVariables[i++].pData = m_szAwayFormat;
  aVariables[i++].pData = m_szOfflineFormat;

  m_bExit = false;
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
  gLog.Info("%sShutting down console.\n", L_CONSOLExSTR);
  gLog.ModifyService(S_PLUGIN, 0);
  licqDaemon->UnregisterPlugin();
}


/*---------------------------------------------------------------------------
 * CLicqConsole::Run
 *-------------------------------------------------------------------------*/
int CLicqConsole::Run(CICQDaemon *_licqDaemon)
{
  // Register with the daemon, we want to receive all signals
  m_nPipe = _licqDaemon->RegisterPlugin(SIGNAL_ALL);
  m_bExit = false;
  licqDaemon = _licqDaemon;

  // Create the windows
  for (unsigned short i = 0; i <= MAX_CON; i++)
  {
    winCon[i] = new CWindow(LINES - 5, COLS - USER_WIN_WIDTH - 1, 2, 0,
                            SCROLLBACK_BUFFER);
    scrollok(winCon[i]->Win(), true);
    winCon[i]->fProcessInput = &CLicqConsole::InputCommand;
  }
  winCon[0]->fProcessInput = &CLicqConsole::InputLogWindow;
  winStatus = new CWindow(2, COLS, LINES - 3, 0, false);
  winPrompt = new CWindow(1, COLS, LINES - 1, 0, false);
  winConStatus = new CWindow(2, COLS, 0, 0, false);
  winStatus->SetActive(true);
  winPrompt->SetActive(true);
  winConStatus->SetActive(true);
  winBar = new CWindow(LINES - 5, 1, 2, COLS - USER_WIN_WIDTH - 1, false);
  winUsers = new CWindow(LINES - 5, USER_WIN_WIDTH, 2, COLS - USER_WIN_WIDTH, false);
  winBar->SetActive(true);
  winUsers->SetActive(true);

  log = new CPluginLog;
  gLog.AddService(new CLogService_Plugin(log, gLog.ServiceLogTypes(S_STDOUT)));
  gLog.ModifyService(S_STDOUT, L_NONE);

  winMain = winCon[1];
  winLog = winCon[0];
  SwitchToCon(1);

  PrintStatus();
  PrintPrompt();
  CreateUserList();
  PrintUsers();

  if (gUserManager.OwnerUin() == 0)
  {
    winMain->wprintf("You have not yet registered a uin.\n"
                     "Use the Qt-GUI plugin to do so.\n");
  }

  fd_set fdSet;
  int nResult;

  while (!m_bExit)
  {
    FD_ZERO(&fdSet);
    FD_SET(STDIN_FILENO, &fdSet);
    FD_SET(m_nPipe, &fdSet);
    FD_SET(log->Pipe(), &fdSet);

    nResult = select(log->Pipe() + 1, &fdSet, NULL, NULL, NULL);
    if (nResult == -1)
    {
      gLog.Error("Error in select(): %s.\n", strerror(errno));
      m_bExit = true;
    }
    else
    {
      if (FD_ISSET(STDIN_FILENO, &fdSet))
        ProcessStdin();
      else if (FD_ISSET(m_nPipe, &fdSet))
        ProcessPipe();
      else if (FD_ISSET(log->Pipe(), &fdSet))
        ProcessLog();
    }
  }

  winMain->wprintf("Exiting\n\n");
  return 0;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessLog
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessLog()
{
  static char buf[2];
  read(log->Pipe(), buf, 1);

  short cp;
  switch (log->NextLogType())
  {
  case L_WARN:
    cp = COLOR_YELLOW;
    break;
  case L_ERROR:
    cp = COLOR_RED;
    break;
  case L_PACKET:
    cp = COLOR_BLUE;
    break;
  case L_UNKNOWN:
    cp = COLOR_MAGENTA;
    break;
  case L_INFO:
  default:
    cp = COLOR_WHITE;
    break;
  }
  char *p = log->NextLogMsg();
  char *l = &p[LOG_PREFIX_OFFSET];
  p[LOG_PREFIX_OFFSET - 1] = '\0';
  winLog->wprintf("%C%s %C%s", COLOR_GREEN, p, cp, l);
  if (log->NextLogType() == L_ERROR)
  {
    winMain->wprintf("%C%s %C%s", COLOR_GREEN, p, cp, l);
    winMain->RefreshWin();
  }
  log->ClearLog();
  winLog->RefreshWin();
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
  case 'S':  // A signal is pending
  {
    CICQSignal *s = licqDaemon->PopPluginSignal();
    ProcessSignal(s);
    break;
  }

  case 'E':  // An event is pending
  {
    ICQEvent *e = licqDaemon->PopPluginEvent();
    ProcessEvent(e);
    break;
  }

  case 'X':  // Shutdown
  {
    gLog.Info("%sExiting console.\n", L_CONSOLExSTR);
    m_bExit = true;
    break;
  }

  case '0':
  case '1':
    break;

  default:
    gLog.Warn("%sUnknown notification type from daemon: %c.\n", L_WARNxSTR, buf[0]);
  }
}


/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessSignal
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessSignal(CICQSignal *s)
{
  switch (s->Signal())
  {
  case SIGNAL_UPDATExLIST:
    PrintStatus();
    CreateUserList();
    PrintUsers();
    break;
  case SIGNAL_UPDATExUSER:
  {
    if (s->Uin() == gUserManager.OwnerUin() && s->SubSignal() == USER_STATUS
        || s->SubSignal() == USER_EVENTS)
      PrintStatus();
    ICQUser *u = gUserManager.FetchUser(s->Uin(), LOCK_R);
    if (u != NULL)
    {
      if (u->GetInGroup(GROUPS_USER, m_nCurrentGroup))
      {
        CreateUserList();
        PrintUsers();
      }
      gUserManager.DropUser(u);
    }
    break;
  }
  case SIGNAL_LOGON:
    PrintStatus();
    break;
  default:
    gLog.Warn("%sInternal error: CLicqConsole::ProcessSignal(): Unknown signal command received from daemon: %d.\n",
              L_WARNxSTR, s->Signal());
    break;
  }
  delete s;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessEvent
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessEvent(ICQEvent *e)
{
  switch (e->m_nCommand)
  {
  // Event commands for a user
  case ICQ_CMDxTCP_START:
  case ICQ_CMDxSND_THRUxSERVER:
  case ICQ_CMDxSND_USERxGETINFO:
  case ICQ_CMDxSND_USERxGETDETAILS:
  case ICQ_CMDxSND_UPDATExDETAIL:
  case ICQ_CMDxSND_UPDATExBASIC:
  {
    unsigned short i;
    for (i = 1; i <= MAX_CON; i++)
    {
      if (winCon[i]->event->Equals(e))
      {
        ProcessDoneEvent(winCon[i], e);
        break;
      }
    }
    if (i > MAX_CON)
      gLog.Warn("%sInternal error: CLicqConsole::ProcessEvent(): Unknown event from daemon: %d.\n",
                L_WARNxSTR, e->m_nCommand);
    break;
  }

  // Commands related to the basic operation
  case ICQ_CMDxSND_LOGON:
    if (e->m_eResult != EVENT_SUCCESS)
      winMain->wprintf("%CLogon failed.  See the log console for details.\n", COLOR_RED);
    break;

  case ICQ_CMDxSND_REGISTERxUSER:
    // Needs to be better dealt with...
    break;

  case ICQ_CMDxSND_SETxSTATUS:
  case ICQ_CMDxSND_AUTHORIZE:
  case ICQ_CMDxSND_USERxLIST:
  case ICQ_CMDxSND_VISIBLExLIST:
  case ICQ_CMDxSND_INVISIBLExLIST:
  case ICQ_CMDxSND_PING:
  case ICQ_CMDxSND_USERxADD:
  case ICQ_CMDxSND_SYSxMSGxREQ:
  case ICQ_CMDxSND_SYSxMSGxDONExACK:
    break;

  case ICQ_CMDxSND_SEARCHxINFO:
  case ICQ_CMDxSND_SEARCHxUIN:
    break;

  default:
    gLog.Warn("%sInternal error: CLicqConsole::ProcessEvent(): Unknown event command received from daemon: %d.\n",
              L_WARNxSTR, e->m_nCommand);
    break;
  }
  delete e;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::ProcessDoneEvent
 *-------------------------------------------------------------------------*/
void CLicqConsole::ProcessDoneEvent(CWindow *win, ICQEvent *e)
{
  bool isOk = (e != NULL && (e->m_eResult == EVENT_ACKED || e->m_eResult == EVENT_SUCCESS));

  if (e == NULL)
  {
    win->wprintf("%A%Cerror\n", A_BOLD, COLOR_RED);
  }
  else
  {
    switch (e->m_eResult)
    {
    case EVENT_ACKED:
    case EVENT_SUCCESS:
      win->wprintf("%A%Cdone\n", m_cColorInfo->nAttr, m_cColorInfo->nColor);
      break;
    case EVENT_TIMEDOUT:
      win->wprintf("%A%Ctimed out\n", m_cColorError->nAttr, m_cColorError->nColor);
      break;
    case EVENT_FAILED:
      win->wprintf("%A%Cfailed\n", m_cColorError->nAttr, m_cColorError->nColor);
      break;
    case EVENT_ERROR:
      win->wprintf("%A%Cerror\n", m_cColorError->nAttr, m_cColorError->nColor);
      break;
    case EVENT_CANCELLED:
      win->wprintf("%A%Ccancelled\n", m_cColorInfo->nAttr, m_cColorInfo->nColor);
      break;
    }
  }
  delete win->event;
  win->event = NULL;
  if (e == NULL) return;

  if (!isOk)
  {
    if (e->m_nCommand == ICQ_CMDxTCP_START &&
        (e->m_nSubCommand == ICQ_CMDxSUB_MSG ||
         e->m_nSubCommand == ICQ_CMDxSUB_URL) )
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
    switch(e->m_nCommand)
    {
    case ICQ_CMDxTCP_START:
    {
      ICQUser *u = NULL;
      CUserEvent *ue = e->m_xUserEvent;
      if (e->m_nSubResult == ICQ_TCPxACK_RETURN)
      {
        u = gUserManager.FetchUser(e->m_nDestinationUin, LOCK_R);
        win->wprintf("%s is in %s mode:\n%s\n[Send \"urgent\" ('.u') to ignore]\n",
                     u->GetAlias(), u->StatusStr(), u->AutoResponse());
        gUserManager.DropUser(u);
      }
      else if (e->m_nSubResult == ICQ_TCPxACK_REFUSE)
      {
        u = gUserManager.FetchUser(e->m_nDestinationUin, LOCK_R);
        win->wprintf("%s refused %s.\n",
                     u->GetAlias(), EventDescription(ue));
        gUserManager.DropUser(u);
      }
      /*else if (e->m_nSubCommand == ICQ_CMDxSUB_CHAT || e->m_nSubCommand == ICQ_CMDxSUB_FILE)
      {
        struct SExtendedAck *ea = e->m_sExtendedAck;
        if (ea == NULL || ue == NULL)
        {
          gLog.Error("%sInternal error: ICQFunctions::doneFcn(): chat or file request acknowledgement without extended result.\n", L_ERRORxSTR);
          return;
        }
        if (!ea->bAccepted)
        {
           u = gUserManager.FetchUser(m_nUin, LOCK_R);
           QString result;
           result.sprintf(tr("%s%1 with %2 refused:\n%s%3"), L_TCPxSTR, L_BLANKxSTR);
           result.arg(EventDescription(ue)).arg(u->GetAlias()).arg(ea->szResponse);
           gUserManager.DropUser(u);
           InformUser(this, result);
        }
        else
        {
          switch (e->m_nSubCommand)
          {
          case ICQ_CMDxSUB_CHAT:
          {
            ChatDlg *chatDlg = new ChatDlg(m_nUin, false, ea->nPort);
            chatDlg->show();
            break;
          }
          case ICQ_CMDxSUB_FILE:
          {
            CFileDlg *fileDlg = new CFileDlg(m_nUin,
                                             ((CEventFile *)ue)->Filename(),
                                             ((CEventFile *)ue)->FileSize(),
                                             false, ea->nPort);
            fileDlg->show();
            break;
          }
          default:
            break;
          } // case
        } // if accepted
      } // if file or chat*/
      else
      {
        u = gUserManager.FetchUser(e->m_nDestinationUin, LOCK_R);
        if (u != NULL && u->Away() && u->ShowAwayMsg())
        {
          win->wprintf("%s\n", u->AutoResponse());
        }
        gUserManager.DropUser(u);
      }

      break;
    } // case

    case ICQ_CMDxSND_THRUxSERVER:
    case ICQ_CMDxSND_USERxGETINFO:
    case ICQ_CMDxSND_USERxGETDETAILS:
    case ICQ_CMDxSND_UPDATExBASIC:
    case ICQ_CMDxSND_UPDATExDETAIL:
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
 * CLicqConsole::SwitchToCon
 *-------------------------------------------------------------------------*/
void CLicqConsole::SwitchToCon(unsigned short nCon)
{
  winMain->SetActive(false);
  winMain = winCon[nCon];
  winMain->SetActive(true);
  m_nCon = nCon;

  // Print the header
  wbkgdset(winConStatus->Win(), COLOR_PAIR(COLOR_YELLOW_BLUE));
  werase(winConStatus->Win());
  winConStatus->wprintf("%A[ %CLicq Console Plugin v%C%s%C (",
                        A_BOLD, COLOR_WHITE_BLUE,
                        COLOR_CYAN_BLUE, LP_Version(), COLOR_YELLOW_BLUE);
  if (m_nCon != 0)
    winConStatus->wprintf("%A%Cconsole %C%d", A_BOLD, COLOR_WHITE_BLUE,
                          COLOR_CYAN_BLUE, m_nCon);
  else
    winConStatus->wprintf("%A%Clog console", A_BOLD, COLOR_WHITE_BLUE);
  winConStatus->wprintf("%A%C) ]", A_BOLD, COLOR_YELLOW_BLUE);
  wclrtoeol(winConStatus->Win());
  wbkgdset(winConStatus->Win(), COLOR_PAIR(COLOR_WHITE));
  mvwhline(winConStatus->Win(), 1, 0, ACS_HLINE, COLS);
  mvwaddch(winConStatus->Win(), 1, COLS - USER_WIN_WIDTH - 1, ACS_TTEE);
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
  {
    if (nPos == 0) return;
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

    szIn[nPos] = '\0';
    char *szArg = strchr(szIn, ' ');
    unsigned short nArgPos = 0;
    if (szIn[0] == '/' && szArg == NULL)
    { // Command completion
      TabCommand(szIn, sTabCompletion);
    }
    else if (szIn[0] != '/')
    { // User completion
      szArg = NULL;
      nArgPos = 0;
      TabUser(szIn, sTabCompletion);
    }
    else
    { // Argument completion
      *szArg = '\0';
      szArg++;
      while (*szArg == ' ') szArg++;
      nArgPos = szArg - szIn;
      // Figure out which command we have
      bool bTab = false;
      for (unsigned short i = 0; i < NUM_COMMANDS; i++)
      {
        if (strncasecmp(szIn, aCommands[i].szName, strlen(szIn)) == 0)
        {
          if (aCommands[i].fProcessTab == NULL)
            break;;
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
    if (sTabCompletion.vszPartialMatch.size() == 0)
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
    { // Only one match
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
    { // Multiple matches
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
    winPrompt->RefreshWin();
    break;
  }

  case '\r':
  {
    if (nPos == 0) break;
    szIn[nPos] = '\0';
    // Strip trailing spaces
    while (nPos != 0 && szIn[nPos - 1] == ' ')
    {
      szIn[nPos - 1] = '\0';
      nPos--;
    }
    // Save the command in the history
    if (m_lCmdHistory.size() == 0 || strcmp(m_lCmdHistory.back(), szIn) != 0)
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

    if (szIn[0] != '/')
    { // User command
      MenuUser(szIn);
    }
    else
    { // Regular command
      char *szArg = strchr(szIn, ' ');
      if (szArg != NULL)
      {
        *szArg = '\0';
        szArg++;
        // Remove any leading spaces
        while (*szArg == ' ') szArg++;
        if (*szArg == '\0') szArg = NULL;
      }
      unsigned short i;
      for (i = 0; i < NUM_COMMANDS; i++)
      {
        if (strncasecmp(szIn, aCommands[i].szName, strlen(szIn)) == 0)
        {
          (this->*(aCommands[i].fProcessCommand))(szArg);
          break;
        }
      }
      if (i == NUM_COMMANDS) PrintBadInput(szIn);
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
      szIn[nPos++] = (char)cIn;
      *winPrompt << (char)cIn;
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
  if (nTabs > 0) nTabs--;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::CurrentGroupName
 *-------------------------------------------------------------------------*/
char *CLicqConsole::CurrentGroupName()
{
  static char szGroupName[64];

  if (m_nCurrentGroup == 0)
    strcpy(szGroupName, "All Users");
  else
  {
    GroupList *g = gUserManager.LockGroupList(LOCK_R);
    strcpy(szGroupName, (*g)[m_nCurrentGroup - 1]);
    gUserManager.UnlockGroupList();
  }
  return szGroupName;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Info
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Info(unsigned long nUin, char *)
{
  // Print the users info to the main window
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);

  PrintBoxTop("User Info", COLOR_WHITE, 50);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("Alias: %s (%ld)", u->GetAlias(), u->Uin());
  PrintBoxRight(50);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("Status: %s", u->StatusStr());
  PrintBoxRight(50);
  PrintBoxBottom(50);

  gUserManager.DropUser(u);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_View
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_View(unsigned long nUin, char *)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);
  if (u->NewUser()) u->SetNewUser(false);

  if (u->NewMessages() > 0)
  {
    // Fetch the most recent event
    CUserEvent *e = u->GetEvent(0);
    wattron(winMain->Win(), A_BOLD);
    for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
      waddch(winMain->Win(), ACS_HLINE);
    waddch(winMain->Win(), '\n');
    time_t t = e->Time();
    char *szTime = ctime(&t);
    szTime[16] = '\0';
    winMain->wprintf("%A%C%s from %s (%s) [%c%c%c]:\n%Z%s\n", A_BOLD,
                     COLOR_WHITE, EventDescription(e),
                     u->User() ? u->GetAlias() : "Server",
                     szTime, e->IsDirect() ? 'D' : '-',
                     e->IsMultiRec() ? 'M' : '-', e->IsUrgent() ? 'U' : '-',
                     A_BOLD, e->Text());
    wattron(winMain->Win(), A_BOLD);
    for (unsigned short i = 0; i < winMain->Cols() - 10; i++)
      waddch(winMain->Win(), ACS_HLINE);
    waddch(winMain->Win(), '\n');
    winMain->RefreshWin();
    wattroff(winMain->Win(), A_BOLD);
    u->ClearEvent(0);
    gUserManager.DropUser(u);
    //PrintUsers();
    //PrintStatus();
    ProcessSignal(new CICQSignal(SIGNAL_UPDATExUSER, USER_EVENTS, nUin));
  }
  else
  {
    gUserManager.DropUser(u);
    winMain->wprintf("No new events.\n");
  }

}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Remove
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Remove(unsigned long nUin, char *)
{
  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputRemove;
  winMain->state = STATE_QUERY;
  winMain->data = new CData(nUin);

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  winMain->wprintf("%C%ARemove %s (%ld) from contact list (y/N)? %C%Z",
                   m_cColorQuery->nColor, m_cColorQuery->nAttr,
                   u->GetAlias(), nUin, COLOR_WHITE, A_BOLD);
  winMain->RefreshWin();
  gUserManager.DropUser(u);
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
      licqDaemon->RemoveUserFromList(data->nUin);
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
void CLicqConsole::UserCommand_FetchAutoResponse(unsigned long nUin, char *)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  winMain->wprintf("%C%AFetching auto-response for %s (%ld)...",
                   m_cColorInfo->nColor, m_cColorInfo->nAttr,
                   u->GetAlias(), nUin);
  winMain->RefreshWin();
  gUserManager.DropUser(u);

  winMain->event = licqDaemon->icqFetchAutoResponse(nUin);
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
    while (isdigit(*sz)) sz++;
  }
  STRIP(sz);

  if (*sz == '+')
  {
    sz++;
    STRIP(sz);
    n += atoi(sz);
    while (isdigit(*sz)) sz++;
  }
  else if (*sz == '-')
  {
    sz++;
    STRIP(sz);
    n -= atoi(sz);
    while (isdigit(*sz)) sz++;
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
void CLicqConsole::UserCommand_History(unsigned long nUin, char *szArg)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  HistoryList lHistory;
  if (!u->GetHistory(lHistory))
  {
    winMain->wprintf("Error loading history.\n");
    gUserManager.DropUser(u);
    return;
  }
  char szFrom[32];
  if (gUserManager.OwnerUin() == nUin)
    strcpy(szFrom, "Server");
  else
    strcpy(szFrom, u->GetAlias());
  gUserManager.DropUser(u);

  unsigned short nLast = lHistory.size();

  // Process the argument
  char *szStart = szArg;
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
    return;
  }
  else if (nStart > nLast || nStart < 1)
  {
    winMain->wprintf("%CStart value out of range, history contains %d events.\n",
                     COLOR_RED, nLast);
    return;
  }

  if (szEnd != NULL)
  {
    nEnd = StrToRange(szEnd, nLast, nStart);
    if (nEnd == -1)
    {
      winMain->wprintf("%CInvalid end range: %A%s\n", COLOR_RED,
                       A_BOLD, szEnd);
      return;
    }
    else if (nEnd > nLast || nEnd < 1)
    {
      winMain->wprintf("%CEnd value out of range, history contains %d events.\n",
                       COLOR_RED, nLast);
      return;
    }
  }
  else
  {
    nEnd = nStart;
  }

  winMain->nLastHistory = nEnd;
  PrintHistory(lHistory, nStart - 1, nEnd - 1, szFrom);
}

/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Msg
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Msg(unsigned long nUin, char *)
{
  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputMessage;
  winMain->state = STATE_MLE;
  winMain->data = new DataMsg(nUin);

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  winMain->wprintf("%AEnter message to %s (%ld):\n%s\n", A_BOLD, u->GetAlias(),
                   nUin, MLE_HELP);
  winMain->RefreshWin();
  gUserManager.DropUser(u);
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
      licqDaemon->CancelEvent(winMain->event);
    return;

  case STATE_MLE:
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
    *sz = '\0';
    sz++;
    winMain->wprintf("%C%ASending message %s...", m_cColorInfo->nColor,
                     m_cColorInfo->nAttr,
                     *sz == 's' ? "through the server" : "direct");
    winMain->event = licqDaemon->icqSendMessage(data->nUin, data->szMsg,
                                                *sz != 's', *sz == 'u');
    winMain->state = STATE_PENDING;
    break;

  // If we are here then direct failed and we asked if send through server
  case STATE_QUERY:
    if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
      return;
    // The input is done
    if (strncasecmp(data->szQuery, "yes", strlen(data->szQuery)) == 0)
    {
      winMain->wprintf("%C%ASending message through the server...",
                       m_cColorInfo->nColor, m_cColorInfo->nAttr);
      winMain->event = licqDaemon->icqSendMessage(data->nUin, data->szMsg,
                                                  false, false);
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
 * CLicqConsole::UserCommand_SetAutoResponse
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_SetAutoResponse(unsigned long nUin, char *)
{
  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputAutoResponse;
  winMain->state = STATE_MLE;
  winMain->data = new DataAutoResponse();

  winMain->wprintf("%A%CEnter auto response:\n", A_BOLD, COLOR_WHITE);
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
      ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
      o->SetAutoResponse(data->szRsp);
      gUserManager.DropOwner();
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
void CLicqConsole::UserCommand_Url(unsigned long nUin, char *)
{
  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputUrl;
  winMain->state = STATE_LE;
  winMain->data = new DataUrl(nUin);

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  winMain->wprintf("%A%CEnter URL to %s (%ld): ", A_BOLD, COLOR_WHITE,
                   u->GetAlias(), nUin);
  winMain->RefreshWin();
  gUserManager.DropUser(u);
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
      licqDaemon->CancelEvent(winMain->event);
    return;

  case STATE_LE:
    // Process the character as a single-line edit window
    // If we get NULL back then we aren't done yet
    if ((sz = Input_Line(data->szUrl, data->nPos, cIn)) == NULL)
      return;
    // The input is done
    winMain->wprintf("%A%CEnter description:\n%s\n", A_BOLD, COLOR_WHITE,
                     MLE_HELP);
    winMain->state = STATE_MLE;
    data->nPos = 0;
    break;

  case STATE_MLE:
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
    winMain->wprintf("%C%ASending URL %s...",
                       m_cColorInfo->nColor, m_cColorInfo->nAttr,
                     *sz == 's' ? "through the server" : "direct");
    winMain->event = licqDaemon->icqSendUrl(data->nUin, data->szUrl,
                                            data->szDesc,
                                            *sz != 's', *sz == 'u');
    winMain->state = STATE_PENDING;
    break;

  // If we are here then direct failed and we asked if send through server
  case STATE_QUERY:
    if ((sz = Input_Line(data->szQuery, data->nPos, cIn)) == NULL)
      return;
    // The input is done
    if (strncasecmp(data->szQuery, "yes", strlen(data->szQuery)) == 0)
    {
      winMain->wprintf("%C%ASending URL through the server...",
                       m_cColorInfo->nColor, m_cColorInfo->nAttr);
      winMain->event = licqDaemon->icqSendUrl(data->nUin, data->szUrl,
                                              data->szDesc, false, false);
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
 * CLicqConsole::Input_Line
 *-------------------------------------------------------------------------*/
char *CLicqConsole::Input_Line(char *sz, unsigned short &n, int cIn)
{
  // Now check for keys
  switch (cIn)
  {
  case KEY_PPAGE:
  case KEY_NPAGE:
  case '\t':
    break;

  case KEY_BACKSPACE:
  {
    if (n == 0) break;
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
    sz[n++] = (char)cIn;
    *winMain << (char)cIn;

  } // switch

  return NULL;

}


/*---------------------------------------------------------------------------
 * CLicqConsole::Input_MultiLine
 *-------------------------------------------------------------------------*/
char *CLicqConsole::Input_MultiLine(char *sz, unsigned short &n, int cIn)
{
  static int nLinePos[MAX_CON + 1] = {0};

  // Now check for keys
  switch (cIn)
  {
  case KEY_PPAGE:
  case KEY_NPAGE:
  case '\t':
    break;

  case KEY_BACKSPACE:
  {
    if (n == 0) break;
    int yp, xp;
    getyx(winMain->Win(), yp, xp);
    if (xp == 0)
    {
      mvwdelch(winMain->Win(), yp - 1, nLinePos[m_nCon]);
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
    int yp;
    getyx(winMain->Win(), yp, nLinePos[m_nCon]);
    *winMain << '\n';
    // Check if the line started with a '.'
    sz[n] = '\0';
    char *szNL = strrchr(sz, '\n');
    if (szNL == NULL)
      szNL = &sz[0];
    else
      szNL++;
    if ( *szNL == '.' || *szNL == ',' )
      return szNL;

    sz[n++] = '\n';
    break;
  }

  default:
    sz[n++] = (char)cIn;
    *winMain << (char)cIn;
    break;

  } // switch

  return NULL;

}



