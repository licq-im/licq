/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2011 Licq developers
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

#ifndef LICQCON_H
#define LICQCON_H

#include <licq/plugin/generalplugin.h>

#include <list>
#include <string>
#include <vector>

#include <licq/contactlist/user.h>
#include <licq/logging/pluginlogsink.h>
#include <licq/userid.h>

#include "window.h"

class CFileTransferManager;

namespace Licq
{
class Event;
class PluginSignal;
class UserEvent;
}

#define MAX_CON 8
#define MAX_CMD_HISTORY 100
#define SCROLLBACK_BUFFER 20
const unsigned short USER_WIN_WIDTH = 30;
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
  Licq::UserId userId;
  char *szLine;
  bool bOffline;
  const struct SColorMap *color;
};


// for keeping track of where a user is
// in the cdkUserList
struct SScrollUser
{
  int pos;
  Licq::UserId userId;
  const struct SColorMap *color;
};

struct SMacro
{
  std::string macro;
  std::string command;
};
typedef std::list<SMacro*> MacroList;


class CLicqConsole : public Licq::GeneralPlugin
{
public:
  CLicqConsole(Params& p);
  CLicqConsole(int, char **);
  ~CLicqConsole();

  // From Licq::GeneralPlugin
  std::string name() const;
  std::string version() const;
  std::string description() const;
  std::string usage() const;
  std::string configFile() const;

  static const int SystemGroupOffset = 10000;
  static const int AllUsersGroupId = 0;
  static const int OnlineNotifyGroupId = SystemGroupOffset + 1;
  static const int VisibleListGroupId = SystemGroupOffset + 2;
  static const int InvisibleListGroupId = SystemGroupOffset + 3;
  static const int IgnoreListGroupId = SystemGroupOffset + 4;
  static const int NewUsersGroupId = SystemGroupOffset + 5;
  static const int NumSystemGroups = 5;
  static const char* GroupsSystemNames[NumSystemGroups+1];

  bool userIsInGroup(const Licq::User* user, int groupId);

protected:
  // From Licq::GeneralPlugin
  bool init(int argc, char** argv);
  int run();
  void destructor();

  int m_nPipe;
  bool m_bExit;
  fd_set fdSet;
  std::list<SUser*> m_lUsers;
  std::list<SScrollUser*> m_lScrollUsers;

  // Set'able variables
  bool m_bShowOffline, m_bShowDividers;
  unsigned m_nColorOnline, m_nColorOffline, m_nColorAway, m_nColorNew,
    m_nColorGroupList, m_nColorQuery, m_nColorInfo, m_nColorError;
  const struct SColorMap *m_cColorOnline, *m_cColorOffline,
                   *m_cColorAway, *m_cColorGroupList, *m_cColorNew,
                   *m_cColorQuery, *m_cColorInfo, *m_cColorError;
  std::string myOnlineFormat;
  std::string myOtherOnlineFormat;
  std::string myAwayFormat;
  std::string myOfflineFormat;
  std::string myCommandChar;
  int m_nBackspace;

  int myCurrentGroup;
  unsigned short m_nCon;
  std::list<char*> m_lCmdHistory;
  std::list<char*>::iterator m_lCmdHistoryIter;
  std::list<CFileTransferManager*> m_lFileStat;
  MacroList listMacros;

  CWindow *winMain, *winStatus, *winPrompt, *winLog, *winCon[MAX_CON + 1],
          *winConStatus, *winUsers, *winBar;
  CDKSCROLL *cdkUserList;
  CDKSCROLL *cdkContactPopup;
  Licq::PluginLogSink::Ptr myLogSink;

	
public:
  void DoneOptions();
  void ProcessPipe();
  void ProcessSignal(Licq::PluginSignal* s);
  void ProcessEvent(Licq::Event*);
  void ProcessDoneEvent(Licq::Event* e);
  void ProcessDoneSearch(Licq::Event* e);
  void ProcessStdin();
  void ProcessLog();
  bool ProcessFile(CFileTransferManager *);
  char *CurrentGroupName();
  void SwitchToCon(unsigned short nCon);
  void CreateUserList();

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
  void PrintHistory(Licq::HistoryList&, unsigned short, unsigned short, const char *);
  void PrintInfo_General(const Licq::UserId& userId);
  void PrintInfo_More(const Licq::UserId& userId);
  void PrintInfo_Work(const Licq::UserId& userId);
  void PrintInfo_About(const Licq::UserId& userId);
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

  void UserCommand_Info(const Licq::UserId& userId, char *);
  void UserCommand_Msg(const Licq::UserId& userId, char *);
  void UserCommand_View(const Licq::UserId& userId, char *);
  void UserCommand_SendFile(const Licq::UserId&  userId, char *);
  void UserCommand_Url(const Licq::UserId& userId, char *);
  void UserCommand_Sms(const Licq::UserId& userId, char *);
  void UserCommand_History(const Licq::UserId&  userId, char *);
  void UserCommand_Remove(const Licq::UserId& userId, char *);
  void UserCommand_FetchAutoResponse(const Licq::UserId& userId, char *);
  void UserCommand_SetAutoResponse(const Licq::UserId& userId, char *);
  void UserCommand_Secure(const Licq::UserId& userId, char *);
  void Command_Search();

  void Beep() { printf("\a"); fflush(stdout); }
  void FileChatOffer(Licq::UserEvent*, const Licq::UserId& userId);
  void RegistrationWizard();
  void InputRegistrationWizard(int cIn);
  void UserSelect();
  void InputUserSelect(int cIn);
  bool ParseMacro(char *);
  void SaveLastUser(const Licq::UserId& userId);
  bool GetContactFromArg(char **, Licq::UserId& userId);
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
