/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#ifndef MAINWIN_H
#define MAINWIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>

#ifdef USE_KDE
#include <kmenubar.h>
#else
#include <qmenubar.h>
#endif

#include <qptrlist.h>
#include <qwidget.h>
#include <qtimer.h>
#include <qbitmap.h>
#include <qdialog.h>
#include <qvaluelist.h>

#include <licq_user.h>

#include "gui-defines.h"
#include "licqdialog.h"
#include "support.h"
#include "userbox.h"

class QStyle;

class QTextView;
class CSignalManager;
class CQtLogWindow;
class CSkin;
class CICQDaemon;
class LicqEvent;
class OptionsDlg;
class AwayMsgDlg;
class RegisterUserDlg;
class OwnerManagerDlg;
class PluginDlg;
class CUserView;
class CELabel;
class CEButton;
class CEComboBox;
class QListViewItem;
class UserViewEvent;
class UserEventCommon;
class UserSendCommon;
class IconManager;

class UserInfoDlg;
class UserEventTabDlg;

#ifdef USE_KDE
class LicqKIMIface;
#endif

typedef QPtrList<UserViewEvent> UserViewEventList;
typedef QPtrList<UserInfoDlg> UserInfoList;
typedef QPtrList<UserSendCommon> UserSendEventList;

//=====CMainWindow==============================================================
class CMainWindow : public QWidget
{
  Q_OBJECT
  friend class CLicqGui;

public:
  CMainWindow(CICQDaemon *theServer, CSignalManager *theSigMan,
              CQtLogWindow *theLogWindow, bool bStartHidden,
              const char *skinName, const char *iconsName,
              const char *extendedIconsName, bool bDisableDockIcon,
              QWidget *parent = 0);
  virtual ~CMainWindow();
  UserEventCommon* callFunction(int fcn, const UserId& userId, int = -1);
  bool RemoveUserFromList(const UserId& userId, QWidget *);
  bool RemoveUserFromGroup(GroupType gtype, int group, const UserId& userId, QWidget* parent);

  void ApplySkin(const char *, bool = false);
  void ApplyIcons(const char *, bool = false);
  void ApplyExtendedIcons(const char *, bool = false);
  CUserView *UserView()  { return userView; }
  QPopupMenu *UserMenu() { return mnuUser; }
  void SetUserMenuUser(const UserId& userId) { myMenuUserId = userId; }
  static QPixmap &iconForStatus(unsigned long FullStatus, const char *szId = "0",
    unsigned long nPPID = LICQ_PPID);
  static QPixmap &iconForEvent(unsigned short SubCommand);

  // global configuration data
  // Toggles
  bool m_bInMiniMode,
       m_bGridLines,
       m_bFontStyles,
       m_bShowGroupIfNoMsg,
       m_bShowHeader,
       m_bAutoClose,
       m_bAutoPopup,
       m_bShowOffline,
       m_bAlwaysShowONU,
       m_bShowDividers,
       m_bAutoRaise,
       m_bHidden,
       m_bBoldOnMsg,
       m_bManualNewUser,
       m_bThreadView,
       m_bScrollBar,
       m_bShowExtendedIcons,
       m_bSystemBackground,
       m_bSendFromClipboard,
       m_bMsgChatView,
       m_bAutoPosReplyWin,
       m_bAutoSendThroughServer,
       m_bEnableMainwinMouseMovement,
       m_bPopPicture,
       m_bPopAlias,
       m_bPopName,
       m_bPopEmail,
       m_bPopPhone,
       m_bPopFax,
       m_bPopCellular,
       m_bPopIP,
       m_bPopLastOnline,
       m_bPopOnlineSince,
       m_bPopIdleTime,
       m_bPopLocalTime,
       m_bPopID,
       m_bShowAllEncodings,
       m_bTabbedChatting,
       m_bShowHistory,
       m_showNotices,
       m_bDisableDockIcon,
       m_bSortColumnAscending,
       m_chatVertSpacing,
       m_chatAppendLineBreak,
       m_histVertSpacing,
       m_bFlashTaskbar,
       m_bMainWinSticky,
       m_bMsgWinSticky,
       m_bSingleLineChatMode,
       m_bCheckSpellingEnabled,
       m_bShowUserIcons;

  QString m_MsgAutopopupKey;
  QString m_DefaultEncoding;

  ColumnInfos colInfo;
  FlashType m_nFlash;
  CSkin *skin;

  int m_nCurrentGroup;
  unsigned long m_nGroupStates;
  unsigned short m_nSortByStatus,
                 m_nSortColumn,
                 m_chatMsgStyle,
                 m_histMsgStyle;

  QString m_chatDateFormat,
          m_histDateFormat;

  QColor m_colorRcvHistory,
         m_colorSntHistory,
         m_colorRcv,
         m_colorSnt,
         m_colorNotice,
         m_colorTab,
         m_colorTabTyping,
         m_colorChatBkg;
                   
  GroupType m_nGroupType;
  QString usprintfHelp;

public slots:
  void callInfoTab(int, const UserId& userId, bool = false, bool =false);

public:
  // Command Tools
  CICQDaemon *licqDaemon;
  CSignalManager *licqSigMan;
  CQtLogWindow *licqLogWindow;
  IconManager *licqIcon;
  UserViewEventList licqUserView;
  UserInfoList licqUserInfo;
  UserSendEventList licqUserSend;

  // Dialog boxes
  AwayMsgDlg *awayMsgDlg;
  OptionsDlg *optionsDlg;
  OwnerManagerDlg *ownerManagerDlg;
  PluginDlg *pluginDlg;
  UserEventTabDlg *userEventTabDlg;

  // Widgets
  CUserView *userView;
#ifdef USE_KDE
  KMenuBar *menu;
#else
  QMenuBar *menu;
#endif
  QPopupMenu *mnuSystem,
             *mnuUser,
             *mnuUserGroups,
             *mnuGroup,
             *mnuServerGroup,
             *mnuOwnerAdm,
             *mnuUserAdm,
             *mnuStatus,
             *mnuPFM,
             *mnuDebug,
             *mnuUtilities,
             *mnuMiscModes,
             *mnuSend,
             *mnuProtocolStatus[16],
             *mnuProtocolOwnerAdm[16];
  CELabel *lblStatus, *lblMsg;
  CEButton *btnSystem;
  CEComboBox *cmbUserGroups;

  QPixmap *pmBorder, *pmMask;
  QBitmap bmMask;

  // GUI Data
  QString m_szCaption;
  QFont defaultFont;
  QStyle *style;
  unsigned short m_nAutoLogon;
  char *m_szIconSet,
       *m_szExtendedIconSet;
  QPixmap pmOnline, pmOffline, pmAway, pmDnd, pmOccupied, pmNa,
          pmPrivate, pmFFC, pmMessage, pmUrl, pmChat, pmFile, pmContact, 
          pmSms, pmAuthorize, pmReqAuthorize, pmSMS, pmSecureOn, pmSecureOff, pmHistory, 
          pmInfo, pmRemove, pmEncoding, pmBirthday, pmPhone, pmCellular, 
          pmInvisible, pmTyping, pmCustomAR, pmCollapsed, pmExpanded, pmSearch,
          pmICQphoneActive, pmICQphoneBusy, pmPhoneFollowMeActive,
          pmPhoneFollowMeBusy, pmSharedFiles, pmMSNOnline, pmMSNOffline,
          pmMSNOccupied, pmMSNPrivate, pmMSNAway, pmAIMOnline, pmAIMAway,
          pmAIMOffline, pmGPGKey, pmGPGKeyEnabled, pmGPGKeyDisabled;
  bool positionChanges;
  int m_nProtoNum;
  UserId myMenuUserId;
  std::vector<unsigned long> m_lnProtMenu;

  // AutoAway
  QTimer autoAwayTimer;
  unsigned short autoAwayTime,
                 autoNATime,
                 autoOfflineTime,
                 manualAway,
                 m_nRealHeight,
                 autoAwayMess,
                 autoNAMess;

  DockMode m_nDockMode;
  int mouseX, mouseY;

#ifdef USE_KDE
    LicqKIMIface* kdeIMInterface;
#endif
  
  // Functions
  void CreateUserView();
  void CreateUserFloaty(const UserId& userId,
    unsigned short x = 0, unsigned short y = 0, unsigned short w = 0);
  void initMenu();
  bool show_user(const LicqUser* u) const;
  void changeMainWinSticky(bool _bStick);

  virtual void resizeEvent (QResizeEvent *);
  virtual void moveEvent(QMoveEvent*);
  virtual void mouseMoveEvent (QMouseEvent *);
  virtual void mousePressEvent (QMouseEvent *);
  virtual void closeEvent (QCloseEvent *);
  virtual void keyPressEvent(QKeyEvent *e);

  friend class WharfIcon;
  friend class IconManager;
  friend class IconManager_Default;
  friend class OptionsDlg;
  friend class CUserViewItem;
  friend class UserSendCommon;
  
public slots:
  void updateUserWin();
  void slot_shutdown();
  void saveOptions();
  void slot_updatedList(unsigned long subSignal, int argument, const UserId& userId);
  void slot_updatedUser(const UserId& userId, unsigned long subSignal, int argument=0, unsigned long cid=0);
  void slot_viewurl(QWidget *, QString);

protected slots:
  void slot_hints();
  void UserGroupToggled(int);
  void ServerGroupChanged(int);
  void FillUserGroup();
  void FillServerGroup();
  void saveAllUsers();
  void updateEvents();
  void updateStatus(unsigned long ppid = 0);
  void updateGroups();
  void changeStatus(int index, unsigned long nPPID = 0xFFFFFFFF,
                    bool _bAutoLogon = false); //all
  void changeStatusManualProtocol(int index);
  void changeStatusManual(int index);
  void changePFMStatus(int index);
  void setCurrentGroupMenu(int id);
  void setCurrentGroup(int);
  void callDefaultFunction(const UserId& userId);
  void callDefaultFunction(QListViewItem *);
  void callOwnerFunction(int, unsigned long = LICQ_PPID);
  void callMsgFunction();
  void callUserFunction(int);
  void slot_socket(const UserId& userId, unsigned long convoId);
  void slot_convoJoin(const UserId& userId, unsigned long ppid, unsigned long convoId);
  void slot_convoLeave(const UserId& userId, unsigned long ppid, unsigned long convoId);
  //TODO
  //void callUserFunction(const char *, unsigned long);
  void slot_userfinished(const UserId& userId);
  void slot_sendfinished(const UserId& userId);
  void slot_ui_message(const UserId& userId);
  void slot_usermenu();
  void slot_logon();
  void slot_ui_viewevent(const UserId& userId);
  void slot_protocolPlugin(unsigned long);
  void slot_eventTag(const UserId& userId, unsigned long);
  void slot_doneplugindlg();
  void slot_doneOptions();
  void slot_doneOwnerManager();
  void slot_doneOwnerFcn(LicqEvent*);
  void slot_doneAwayMsgDlg();
  void slot_stats();
  void showAddUserDlg();
  void showEditGrpDlg();
  void showSkinBrowser();
  void showPluginDlg();
  void showGPGKeyManager();
  void slot_AwayMsgDlg();
  void showAwayMsgDlg(unsigned short status, bool autoclose = false);
  void showOptionsDlg(int);
  void popupOptionsDlg() { emit showOptionsDlg(0); };
  void showAuthUserDlg();
  void showReqAuthDlg(int);
  void showReqAuthDlg(const char *, unsigned long);
  void showSearchUserDlg();
  void showOwnerManagerDlg();
  void popupSystemMenu();
  void changeDebug(int);
  void slot_utility(int);
  void slot_miscmodes(int);
  void slot_randomchatsearch();
  void ToggleMiniMode();
  void ToggleShowOffline();
  void ToggleThreadView();
  void autoAway();
  void aboutBox();
  void slot_updateContactList();
  void slot_updateAllUsers();
  void slot_updateAllUsersInGroup();
  void slot_popupall();
  void slot_aboutToQuit();
  void UserInfoDlg_finished(const UserId& userId);
  void slot_doneUserEventTabDlg();
  void slot_pluginUnloaded(unsigned long);

  void sendMsg(const char* szId, unsigned long nPPID, const QString& message);
  void sendFileTransfer(const char* szId, unsigned long nPPID,
                        const QString& filename, const QString& description);
  void sendChatRequest(const char* szId, unsigned long nPPID);
  void addUser(const char* szId, unsigned long nPPID);
  void setMainWinSticky();

signals:
  void changeDockStatus(unsigned short);
  void signal_sentevent(LicqEvent* e);
  void signal_doneRegisterUser(LicqEvent* e);

private:
  enum { /* ID's for the menu items in mnuUserAdm.
            Needed to reference individual menu items,
            i.e. when changing icons or status of an item.
        */
        MNU_USER_ADM_ADD_USER = 0,
        MNU_USER_ADM_SEARCH_USER = 1,
        MNU_USER_ADM_AUTHORIZE_USER = 2,
        MNU_USER_ADM_REQUEST_AUTH = 3,
        MNU_USER_ADM_RANDOM_CHAT = 4,
        MNU_USER_ADM_POPUP_ALL_MSG = 5,
        MNU_USER_ADM_EDIT_GROUPS = 6,
        MNU_USER_ADM_UPDATE_ALL_USERS = 7,
        MNU_USER_ADM_UPDATE_CURRENT_GROUP = 8,
        MNU_USER_ADM_REDRAW_USER_WIN = 9,
        MNU_USER_ADM_SAVE_ALL_USERS = 10,
        MNU_USER_ADM_REGISTER_USER = 11
  };
  
  enum { /* ID's for the menu items in mnuSystem.
            Needed to reference individual menu items,
            i.e. when changing icons or status of an item.
        */
        MNU_SYS_SYSTEM_FUNCTIONS = 0,
        MNU_SYS_USER_FUNCTIONS = 1,
        MNU_SYS_STATUS = 2,
        MNU_SYS_GROUP= 3,
        MNU_SYS_AUTO_RESPONSE = 4,
        MNU_SYS_NETWORKWIN = 5,
        MNU_SYS_MINI_MODE = 6,
        MNU_SYS_SHOW_OFFLINE = 7,
        MNU_SYS_THREAD_VIEW = 8,
        MNU_SYS_OPTIONS = 9,
        MNU_SYS_SKINBROWSER = 10,
        MNU_SYS_PLUGINS = 11,
        MNU_SYS_GPG = 12
  };

  QValueList<int> myGroupIds;
};

// -----------------------------------------------------------------------------

class HintsDlg : public LicqDialog
{
  Q_OBJECT

public:
  HintsDlg(QString &);

private:
  QTextView *txtView;
  QPushButton *btnClose;
};

// -----------------------------------------------------------------------------

extern CMainWindow* gMainWindow;

// HintsDlg class

#endif
