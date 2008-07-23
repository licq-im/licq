#ifndef LICQCON_H
#define LICQCON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <list>
#include <string>
#include <vector>

#include <licq_icqd.h>
#include <licq_log.h>
#include <licq_user.h>

#include "window.h"

class CFileTransferManager;

#define MAX_CON 8
#define MAX_CMD_HISTORY 100
#define SCROLLBACK_BUFFER 20
const unsigned short USER_WIN_WIDTH = 30;
const char L_CONSOLExSTR[] = "[CON] ";
const char CANCEL_KEY = 'C';

struct SColorMap
{
  char szName[16];
  int nColor;
  int nAttr;
};

struct SUser
{
  char szKey[256];
  char szId[256];
  unsigned long nPPID;
  char *szLine;
  bool bOffline;
  const struct SColorMap *color;
};


// for keeping track of where a user is
// in the cdkUserList
struct SScrollUser
{
  int pos;
  unsigned long nPPID;
  char szId[256];
  const struct SColorMap *color;
};

struct SMacro
{
  char szMacro[32];
  char szCommand[128];
};
typedef std::list<SMacro*> MacroList;


class CLicqConsole
{
public:
  CLicqConsole(int, char **);
  ~CLicqConsole();
  int Run(CICQDaemon *);
  void Shutdown();

protected:
  int m_nPipe;
  bool m_bExit;
  fd_set fdSet;
  std::list<SUser*> m_lUsers;
  std::list<SScrollUser*> m_lScrollUsers;

  // Set'able variables
  bool m_bShowOffline, m_bShowDividers;
  unsigned short m_nColorOnline, m_nColorOffline, m_nColorAway, m_nColorNew,
    m_nColorGroupList, m_nColorQuery, m_nColorInfo, m_nColorError;
  const struct SColorMap *m_cColorOnline, *m_cColorOffline,
                   *m_cColorAway, *m_cColorGroupList, *m_cColorNew,
                   *m_cColorQuery, *m_cColorInfo, *m_cColorError;
  char m_szOnlineFormat[30];
  char m_szOtherOnlineFormat[30];
  char m_szAwayFormat[30];
  char m_szOfflineFormat[30];
  char m_szCommandChar[30];
  short m_nBackspace;

  unsigned short m_nCurrentGroup, m_nCon;
  GroupType m_nGroupType;
  std::list<char*> m_lCmdHistory;
  std::list<char*>::iterator m_lCmdHistoryIter;
  std::list<CFileTransferManager*> m_lFileStat;
  MacroList listMacros;

  CICQDaemon *licqDaemon;
  CWindow *winMain, *winStatus, *winPrompt, *winLog, *winCon[MAX_CON + 1],
          *winConStatus, *winUsers, *winBar;
  CDKSCROLL *cdkUserList;
  CDKSCROLL *cdkContactPopup;
  CPluginLog *log;

	
public:
  void DoneOptions();
  void ProcessPipe();
  void ProcessSignal(CICQSignal *);
  void ProcessEvent(ICQEvent *);
  void ProcessDoneEvent(ICQEvent *e);
  void ProcessDoneSearch(ICQEvent *e);
  void ProcessStdin();
  void ProcessLog();
  bool ProcessFile(CFileTransferManager *);
  char *CurrentGroupName();
  void SwitchToCon(unsigned short nCon);
  void CreateUserList();
  void AddEventTag(const char* _szId, unsigned long _nPPID,
      unsigned long _nEventTag);

  void InputCommand(int cIn);
  void InputLogWindow(int cIn);
  void InputMessage(int cIn);
  void InputUrl(int cIn);
  void InputSms(int cIn);
  void InputSendFile(int cIn);
  void InputRemove(int cIn);
  void InputInfo(int cIn);
  void InputAutoResponse(int cIn);
  void InputFileChatOffer(int cIn);
  void InputSearch(int cIn);
  void InputAuthorize(int cIn);
  char *Input_MultiLine(char *sz, unsigned short &n, int cIn);
  char *Input_Line(char *sz, unsigned short &n, int cIn, bool bEcho = true);

  void PrintPrompt();
  void PrintStatus();
  void PrintBadInput(const char *);
  void PrintGroups();
  void PrintBoxTop(const char *_szTitle, short _nColor, short _nLength);
  void PrintBoxBottom(short _nLength);
  void PrintBoxRight(short _nLength);
  void PrintBoxLeft();
  void PrintVariable(unsigned short);
  void PrintUsers();
  void PrintHelp();
  void PrintHistory(HistoryList &, unsigned short, unsigned short, const char *);
  void PrintInfo_General(const char *, unsigned long);
  void PrintInfo_More(const char *, unsigned long);
  void PrintInfo_Work(const char *, unsigned long);
  void PrintInfo_About(const char *, unsigned long);
  void PrintFileStat(CFileTransferManager *);
  void PrintMacros();
  void PrintContactPopup(const char* alias);

  static int UserListCallback(EObjectType, void *, void *, chtype);
  static int MenuPopupWrapper(EObjectType, void *, void *, chtype);
  void UserListHighlight(chtype, chtype = 'a');
  void MenuPopup(int);
  void MenuSwitchConsole(char *);
  void MenuList(char *);
  void MenuHelp(char *);
  void MenuContactList(char *);
  void MenuUins(char *);
  void MenuGroup(char *);
  void MenuQuit(char *);
  void MenuUser(char *);
  void MenuOwner(char *);
  void MenuStatus(char *);
  void MenuPlugins(char *);
  void MenuSet(char *);
  void MenuLast(char *);
  void MenuFile(char *);
  void MenuClear(char *);
  void MenuAdd(char *);
  void MenuDefine(char *);
  void MenuSearch(char *);
  void MenuAuthorize(char *);
  void MenuMessage(char *);
  void MenuUrl(char *);
  void MenuSms(char *);
  void MenuInfo(char *);
  void MenuSecure(char *);
  void MenuAutoResponse(char *);
  void MenuView(char *);
  void MenuHistory(char *);
  void MenuRemove(char *);

  void TabUser(char *, struct STabCompletion &);
  void TabCommand(char *, struct STabCompletion &);
  void TabStatus(char *, struct STabCompletion &);
  void TabSet(char *, struct STabCompletion &);

  void UserCommand_Info(const char *szId, unsigned long nPPID, char *);
  void UserCommand_Msg(const char *szId, unsigned long nPPID, char *);
  void UserCommand_View(const char *szId, unsigned long nPPID, char *);
  void UserCommand_SendFile(const char *szId, unsigned long nPPID, char *);
  void UserCommand_Url(const char *szId, unsigned long nPPID, char *);
  void UserCommand_Sms(const char *szId, unsigned long nPPID, char *);
  void UserCommand_History(const char *szId, unsigned long nPPID, char *);
  void UserCommand_Remove(const char *szId, unsigned long nPPID, char *);
  void UserCommand_FetchAutoResponse(const char *szId, unsigned long nPPID, char *);
  void UserCommand_SetAutoResponse(const char *szId, unsigned long nPPID, char *);
  void UserCommand_Secure(const char *szId, unsigned long nPPID, char *);
  void Command_Search();

  void Beep() { printf("\a"); fflush(stdout); }
  void FileChatOffer(CUserEvent *, const char *, unsigned long);
  void RegistrationWizard();
  void InputRegistrationWizard(int cIn);
  void UserSelect();
  void InputUserSelect(int cIn);
  bool ParseMacro(char *);
  std::string GetUserFromArg(char** p_szArg);
  void SaveLastUser(const std::string& id, unsigned long ppid);
  struct SContact GetContactFromArg(char **);
};


struct STabCompletion
{
  std::vector<char*> vszPartialMatch;
  char *szPartialMatch;
};

struct SCommand
{
  const char* szName;
  void (CLicqConsole::*fProcessCommand)(char *);
  void (CLicqConsole::*fProcessTab)(char *, struct STabCompletion &);
  const char* szHelp;
  const char* szDescription;
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
  void (CLicqConsole::*fProcessCommand)(unsigned long, char *);
};
extern const unsigned short NUM_USER_COMMANDS;
extern const struct SUserCommand aUserCommands[];

struct SOwnerCommand
{
  char szName[16];
  void (CLicqConsole::*fProcessCommand)(unsigned long, char *);
};
extern const unsigned short NUM_OWNER_COMMANDS;
extern const struct SOwnerCommand aOwnerCommands[];

typedef enum { INT, BOOL, STRING, COLOR } VarType;
struct SVariable
{
  char szName[32];
  VarType nType;
  void *pData;
};

extern const unsigned short NUM_COLORMAPS;
extern const struct SColorMap aColorMaps[];

extern const unsigned short NUM_VARIABLES;
extern struct SVariable aVariables[];

#define STRIP(x) while(isspace(*(x)) && *(x) != '\0') (x)++;

#endif
