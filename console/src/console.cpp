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

extern "C" const char *LP_Version(void);

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

const unsigned short NUM_VARIABLES = 7;
struct SVariable aVariables[NUM_VARIABLES] =
{
  { "show_offline_users", BOOL, NULL },
  { "show_dividers", BOOL, NULL },
  { "color_online", COLOR, NULL },
  { "color_away", COLOR, NULL },
  { "color_offline", COLOR, NULL },
  { "color_group_list", COLOR, NULL },
  { "user_format", STRING, NULL }
};

const unsigned short NUM_COLORMAPS = 15;
const struct SColorMap aColorMaps[NUM_COLORMAPS] =
{
  { "green", COLOR_GREEN, false },
  { "red", COLOR_RED, false },
  { "cyan", COLOR_CYAN, false },
  { "white", COLOR_WHITE, false },
  { "magenta", COLOR_MAGENTA, false },
  { "blue", COLOR_BLUE, false },
  { "yellow", COLOR_YELLOW, false },
  { "black", COLOR_BLACK, false },
  { "bright_green", COLOR_GREEN, true },
  { "bright_red", COLOR_RED, true },
  { "bright_cyan", COLOR_CYAN, true },
  { "bright_white", COLOR_WHITE, true },
  { "bright_magenta", COLOR_MAGENTA, true },
  { "bright_blue", COLOR_BLUE, true },
  { "bright_yellow", COLOR_YELLOW, true }
};



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
  m_cColorGroupList = &aColorMaps[13];
  strcpy(m_szUserFormat, "%-20a%18s");

  // Set the variable data pointers
  aVariables[0].pData = &m_bShowOffline;
  aVariables[1].pData = &m_bShowDividers;
  aVariables[2].pData = &m_cColorOnline;
  aVariables[3].pData = &m_cColorAway;
  aVariables[4].pData = &m_cColorOffline;
  aVariables[5].pData = &m_cColorGroupList;
  aVariables[6].pData = m_szUserFormat;

  m_bExit = false;
}


/*---------------------------------------------------------------------------
 * CLicqConsole::Destructor
 *-------------------------------------------------------------------------*/
CLicqConsole::~CLicqConsole(void)
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
void CLicqConsole::Shutdown(void)
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
    winCon[i] = new CWindow(LINES - 5, COLS - USER_WIN_WIDTH - 1, 2, 0, true);
    scrollok(winCon[i]->Win(), true);
    winCon[i]->fProcessInput = &CLicqConsole::InputCommand;
  }
  winCon[0]->fProcessInput = NULL;
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
  PrintUsers();

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
void CLicqConsole::ProcessLog(void)
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
void CLicqConsole::ProcessPipe(void)
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
    PrintUsers();
    break;
  case SIGNAL_UPDATExUSER:
    if (s->Uin() == gUserManager.OwnerUin() && s->SubSignal() == USER_STATUS
        || s->SubSignal() == USER_EVENTS)
      PrintStatus();
    PrintUsers();
    break;
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
      if (e == winCon[i]->event)
      {
        switch (winCon[i]->event->m_eResult)
        {
        case EVENT_ACKED:
        case EVENT_SUCCESS:
          winCon[i]->wprintf("%A%Cdone\n", A_BOLD, COLOR_BLUE);
          break;
        case EVENT_TIMEDOUT:
          winCon[i]->wprintf("%A%Ctimed out\n", A_BOLD, COLOR_RED);
          break;
        case EVENT_FAILED:
          winCon[i]->wprintf("%A%Cfailed\n", A_BOLD, COLOR_RED);
          break;
        case EVENT_ERROR:
          winCon[i]->wprintf("%A%Cerror\n", A_BOLD, COLOR_RED);
          break;
        case EVENT_CANCELLED:
          winCon[i]->wprintf("%A%Ccancelled\n", A_BOLD, COLOR_GREEN);
          break;
        }
        winCon[i]->fProcessInput = &CLicqConsole::InputCommand;
        if (winCon[i]->data != NULL) delete winCon[i]->data;
        winCon[i]->state = STATE_COMMAND;
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

  case ICQ_CMDxSND_SEARCHxSTART:
    break;

  default:
    gLog.Warn("%sInternal error: CLicqConsole::ProcessEvent(): Unknown event command received from daemon: %d.\n",
              L_WARNxSTR, e->m_nCommand);
    break;
  }
  delete e;
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
void CLicqConsole::ProcessStdin(void)
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

  // Don't accept any input from the log window
  if (winMain->fProcessInput == NULL) return;

  (this->*(winMain->fProcessInput))(cIn);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::InputCommand
 *-------------------------------------------------------------------------*/
void CLicqConsole::InputCommand(int cIn)
{
  static char szIn[128];
  static int nPos = 0;
  static int nTabs = 0;
  static struct STabCompletion sTabCompletion;

  // Now check for keys
  switch (cIn)
  {
  case KEY_PPAGE:
    wscrl(winMain->Win(), 10);
    break;

  case KEY_NPAGE:
    wscrl(winMain->Win(), -(LINES >> 1));
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
char *CLicqConsole::CurrentGroupName(void)
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
void CLicqConsole::UserCommand_Info(unsigned long nUin)
{
  // Print the users info to the main window
  struct UserBasicInfo ubi;
  struct UserExtInfo uei;
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  u->getBasicInfo(ubi);
  u->getExtInfo(uei);
  gUserManager.DropUser(u);

  PrintBoxTop("User Info", COLOR_WHITE, 50);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("Alias: %s (%s)", ubi.alias, ubi.uin);
  PrintBoxRight(50);
  waddch(winMain->Win(), ACS_VLINE);
  winMain->wprintf("Status: %s", ubi.status);
  PrintBoxRight(50);
  PrintBoxBottom(50);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_View
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_View(unsigned long nUin)
{
  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_W);

  if (u->getNumMessages() > 0)
  {
    // Fetch the most recent event
    CUserEvent *e = u->GetEvent(0);
    wattron(winMain->Win(), A_BOLD);
    for (unsigned short i = 0; i < 60; i++)
      waddch(winMain->Win(), ACS_HLINE);
    waddch(winMain->Win(), '\n');
    winMain->wprintf("%A%C%s from %s (%s) [%c%c%c]:\n%Z%s\n", A_BOLD,
                     COLOR_WHITE, EventDescription(e),
                     u->User() ? u->getAlias() : "Server",
                     e->Time(), e->IsDirect() ? 'D' : '-',
                     e->IsMultiRec() ? 'M' : '-', e->IsUrgent() ? 'U' : '-',
                     A_BOLD, e->Text());
    wattron(winMain->Win(), A_BOLD);
    for (unsigned short i = 0; i < 60; i++)
      waddch(winMain->Win(), ACS_HLINE);
    waddch(winMain->Win(), '\n');
    winMain->RefreshWin();
    wattroff(winMain->Win(), A_BOLD);
    u->ClearEvent(0);
    PrintUsers();
    PrintStatus();
  }
  else
  {
    winMain->wprintf("No new events.\n");
  }

  gUserManager.DropUser(u);
}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Msg
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Msg(unsigned long nUin)
{
  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputMessage;
  winMain->state = STATE_MLE;
  winMain->data = new DataMsg(nUin);

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  winMain->wprintf("%AEnter message to %s (%ld):\n", A_BOLD, u->getAlias(),
                   nUin);
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
    return;

  case STATE_MLE:
    // Process the character as a multi-line edit window
    // If we get NULL back then we aren't done yet
    if ((sz = Input_MultiLine(data->szMsg, data->nPos, cIn)) == NULL)
      return;

    // The input is done, so process it, sz points to the '.'
    *sz = '\0';
    sz++;
    winMain->wprintf("%C%ASending message %s...", COLOR_BLUE, A_BOLD,
                     *sz == 's' ? "through the server" : "direct");
    winMain->event = licqDaemon->icqSendMessage(data->nUin, data->szMsg,
                                                *sz != 's', *sz == 'u');
    winMain->state = STATE_PENDING;
    break;

  default:
    winMain->wprintf("%CInvalid state: %A%d%Z.\n", COLOR_RED, A_BOLD, A_BOLD);
  }

}


/*---------------------------------------------------------------------------
 * CLicqConsole::UserCommand_Url
 *-------------------------------------------------------------------------*/
void CLicqConsole::UserCommand_Url(unsigned long nUin)
{
  // First put this console into edit mode
  winMain->fProcessInput = &CLicqConsole::InputUrl;
  winMain->state = STATE_LE;
  winMain->data = new DataUrl(nUin);

  ICQUser *u = gUserManager.FetchUser(nUin, LOCK_R);
  winMain->wprintf("%AEnter URL to %s (%ld): ", A_BOLD, u->getAlias(),
                   nUin);
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
    return;

  case STATE_LE:
    // Process the character as a single-line edit window
    // If we get NULL back then we aren't done yet
    if ((sz = Input_Line(data->szUrl, data->nPos, cIn)) == NULL)
      return;
    // The input is done
    winMain->wprintf("%AEnter description:\n", A_BOLD);
    winMain->state = STATE_MLE;
    data->nPos = 0;
    break;

  case STATE_MLE:
    // Process the character as a multi-line edit window
    // If we get NULL back then we aren't done yet
    if ((sz = Input_MultiLine(data->szDesc, data->nPos, cIn)) == NULL)
      return;

    // The input is done, so process it, sz points to the '.'
    *sz = '\0';
    sz++;
    winMain->wprintf("%C%ASending URL %s...", COLOR_BLUE, A_BOLD,
                     *sz == 's' ? "through the server" : "direct");
    winMain->event = licqDaemon->icqSendUrl(data->nUin, data->szUrl,
                                            data->szDesc,
                                            *sz != 's', *sz == 'u');
    winMain->state = STATE_PENDING;
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
    char *szNL = strrchr(sz, '\n');
    if (szNL == NULL || *(szNL + 1) != '.')
    {
      sz[n++] = '\n';
      break;
    }
    // It was a dot so we are done
    sz[n] = '\0';
    return ++szNL;
    break;
  }

  default:
    sz[n++] = (char)cIn;
    *winMain << (char)cIn;

  } // switch

  return NULL;

}



