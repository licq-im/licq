#ifndef LICQCON_H
#define LICQCON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "icqd.h"
#include "log.h"
#include "window.h"

#define MAX_CON 8


class CLicqConsole
{
public:
  CLicqConsole(int, char **);
  ~CLicqConsole(void);
  int Run(CICQDaemon *);
  void Shutdown(void);
protected:
  int m_nPipe;
  bool m_bExit;

  bool m_bShowOffline, m_bShowDividers;
  unsigned short m_nCurrentGroup, m_nCon;
  GroupType m_nGroupType;

  CICQDaemon *licqDaemon;
  CWindow *winMain, *winStatus, *winPrompt, *winLog, *winCon[MAX_CON + 1],
          *winConStatus;
  CPluginLog *log;

public:
  void ProcessPipe(void);
  void ProcessSignal(CICQSignal *);
  void ProcessEvent(ICQEvent *);
  void ProcessStdin(void);
  void ProcessLog(void);
  char *CurrentGroupName(void);
  void SwitchToCon(unsigned short nCon);

  void InputCommand(int cIn);
  void InputMessage(int cIn);
  void InputUrl(int cIn);
  char *Input_MultiLine(char *sz, unsigned short &n, int cIn);
  char *Input_Line(char *sz, unsigned short &n, int cIn);

  void PrintPrompt(void);
  void PrintStatus(void);
  void PrintBadInput(const char *);
  void PrintGroups(void);
  void PrintBoxTop(const char *_szTitle, short _nColor, short _nLength);
  void PrintBoxBottom(short _nLength);
  void PrintBoxRight(short _nLength);
  void PrintBoxLeft(void);

  void MenuHelp(char *);
  void MenuContactList(char *);
  void MenuGroup(char *);
  void MenuQuit(char *);
  void MenuUser(char *);
  void MenuStatus(char *);
  void MenuPlugins(char *);

  void TabUser(char *, struct STabCompletion &);
  void TabCommand(char *, struct STabCompletion &);
  void TabStatus(char *, struct STabCompletion &);

  void UserCommand_Info(unsigned long nUin);
  void UserCommand_Msg(unsigned long nUin);
  void UserCommand_View(unsigned long nUin);
  void UserCommand_Url(unsigned long nUin);

  void Beep(void) { printf("\a"); fflush(stdout); }
};


struct STabCompletion
{
  vector <char *> vszPartialMatch;
  char szPartialMatch[32];
};

struct SCommand
{
  char szName[16];
  void (CLicqConsole::*fProcessCommand)(char *);
  void (CLicqConsole::*fProcessTab)(char *, struct STabCompletion &);
};
extern const unsigned short NUM_COMMANDS;
extern const struct SCommand aCommands[];
struct SStatus
{
  char szName[16];
  unsigned short nId;
};
extern const unsigned short NUM_STATUS;
extern const struct SStatus aStatus[];
struct SUserCommand
{
  char szName[16];
  void (CLicqConsole::*fProcessCommand)(unsigned long);
};
extern const unsigned short NUM_USER_COMMANDS;
extern const struct SUserCommand aUserCommands[];

#endif
