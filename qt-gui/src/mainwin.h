#ifndef MAINWIN_H
#define MAINWIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include <kmenubar.h>
#else
#include <qmenubar.h>
#endif

#if QT_VERSION < 300
  #include <qlist.h>
#else
  #include <qptrlist.h>
#endif
#include <qwidget.h>
#include <qtimer.h>
#include <qbitmap.h>
#include <qdialog.h>
#include <vector>

#include "userbox.h"
#include "licq_user.h"
#include "licqdialog.h"
#include "gui-defines.h"

#define MSN_PPID 0x4D534E5F

class QStyle;

class QTextView;
class CSignalManager;
class CQtLogWindow;
class CSkin;
class CEmoticons;
class CICQDaemon;
class ICQEvent;
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

class CICQSignal;
class UserInfoDlg;
#if QT_VERSION >= 300
class UserEventTabDlg;
#endif

#ifdef USE_KDE
class LicqKIMIface;
#endif

#if QT_VERSION < 300
  typedef QList<UserViewEvent> UserViewEventList;
  typedef QList<UserInfoDlg> UserInfoList;
  typedef QList<UserSendCommon> UserSendEventList;
#else
  typedef QPtrList<UserViewEvent> UserViewEventList;
  typedef QPtrList<UserInfoDlg> UserInfoList;
  typedef QPtrList<UserSendCommon> UserSendEventList;
#endif

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

using std::vector;

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
  bool RemoveUserFromList(unsigned long, QWidget *);
  bool RemoveUserFromGroup(GroupType gtype, unsigned long group, unsigned long, QWidget *);
  UserEventCommon *callFunction(int fcn, const char *, unsigned long, int = -1);
  bool RemoveUserFromList(const char *, unsigned long, QWidget *);
  bool RemoveUserFromGroup(GroupType, unsigned long, const char *,
    unsigned long, QWidget *);

  void ApplySkin(const char *, bool = false);
  void ApplyIcons(const char *, bool = false);
  void ApplyExtendedIcons(const char *, bool = false);
  CUserView *UserView()  { return userView; }
  QPopupMenu *UserMenu() { return mnuUser; }
  void SetUserMenuUin(unsigned long n) { m_nUserMenuUin = n; } // deprecated, use SetUserMenuUser instead!
  void SetUserMenuUser(const char *_szId, unsigned long _nPPID)
  {
    if (m_szUserMenuId)  free(m_szUserMenuId);
    m_szUserMenuId = strdup(_szId);
    m_nUserMenuPPID = _nPPID;
    m_nUserMenuUin = strtoul(_szId, (char **)NULL, 10); // deprecated, use m_nUserMenuId instead!
  }
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
       m_bPopEmail,
       m_bPopPhone,
       m_bPopFax,
       m_bPopCellular,
       m_bPopIP,
       m_bPopLastOnline,
       m_bPopOnlineSince,
       m_bPopIdleTime,
       m_bPopID,
       m_bShowAllEncodings,
       m_bTabbedChatting,
       m_bShowHistory,
       m_bDisableDockIcon,
       m_bSortColumnAscending,
       m_bAppendLineBreak;

  QString m_MsgAutopopupKey;
  QString m_DefaultEncoding;

  ColumnInfos colInfo;
  FlashType m_nFlash;
  CSkin *skin;
  CEmoticons *emoticons;

  unsigned long m_nCurrentGroup, m_nGroupStates;
  unsigned short m_nSortByStatus,
                 m_nSortColumn,
                 m_nMsgStyle;
                 
  QColor m_colorRcvHistory,
         m_colorSntHistory,
         m_colorRcv,
         m_colorSnt,
         m_colorTab,
         m_colorTabTyping,
         m_colorChatBkg;
                   
  GroupType m_nGroupType;
  QString usprintfHelp;

public slots:
  void callInfoTab(int, const char *, unsigned long, bool = false, bool =false);

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
#if QT_VERSION >= 300
  UserEventTabDlg *userEventTabDlg;
#endif

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
          pmSms, pmAuthorize, pmSMS, pmSecureOn, pmSecureOff, pmHistory, 
          pmInfo, pmRemove, pmEncoding, pmBirthday, pmPhone, pmCellular, 
          pmInvisible, pmTyping, pmCustomAR, pmCollapsed, pmExpanded, pmSearch,
          pmICQphoneActive, pmICQphoneBusy, pmPhoneFollowMeActive,
          pmPhoneFollowMeBusy, pmSharedFiles, pmMSNOnline, pmMSNOffline,
          pmMSNOccupied, pmMSNPrivate, pmMSNAway, pmAIMOnline, pmAIMAway,
          pmAIMOffline, pmGPGKey, pmGPGKeyDisabled;
  unsigned long m_nUserMenuUin;
  unsigned int positionChanges;
  int m_nProtoNum;
  char *m_szUserMenuId;
  unsigned long m_nUserMenuPPID;
  vector<unsigned long> m_lnProtMenu;
  
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
  void CreateUserFloaty(unsigned long nUin, unsigned short x = 0,
     unsigned short y = 0, unsigned short w = 0);
  void CreateUserFloaty(const char *szId, unsigned long nPPID,
    unsigned short x = 0, unsigned short y = 0, unsigned short w = 0);
  void initMenu();
  bool show_user(ICQUser *);

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
  void slot_updatedList(CICQSignal *);
  void slot_updatedUser(CICQSignal *);
  void slot_viewurl(QWidget *, QString);

protected slots:
  void slot_hints();
  void UserGroupToggled(int);
  void ServerGroupChanged(int);
  void FillUserGroup();
  void FillServerGroup();
  void saveAllUsers();
  void updateEvents();
  void updateStatus(CICQSignal * = NULL);
  void updateGroups();
  void changeStatus(int index, unsigned long nPPID = 0xFFFFFFFF); //all
  void changeStatusManualProtocol(int index);
  void changeStatusManual(int index);
  void changePFMStatus(int index);
  void setCurrentGroupMenu(int id);
  void setCurrentGroup(int);
  void callDefaultFunction(const char *, unsigned long);
  void callDefaultFunction(QListViewItem *);
  void callOwnerFunction(int, unsigned long = LICQ_PPID);
  void callMsgFunction();
  void callFileFunction (const char *);
  void callUrlFunction (const char *);
  void callUserFunction(int);
  void slot_socket(const char *, unsigned long, unsigned long);
  void slot_convoJoin(const char *, unsigned long, unsigned long);
  void slot_convoLeave(const char *, unsigned long, unsigned long);
  //TODO
  //void callUserFunction(const char *, unsigned long);
  void slot_userfinished(const char *, unsigned long);
  void slot_sendfinished(const char *, unsigned long);
  void slot_ui_message(const char *, unsigned long);
  void slot_usermenu();
  void slot_logon();
  //void slot_ui_viewevent(unsigned long);
  void slot_ui_viewevent(const char *);
  void slot_protocolPlugin(unsigned long);
  void slot_eventTag(const char *, unsigned long, unsigned long);
  void slot_doneplugindlg();
  void slot_doneOptions();
  void slot_doneOwnerManager();
  void slot_doneOwnerFcn(ICQEvent *);
  void slot_doneAwayMsgDlg();
  void slot_stats();
  void showAddUserDlg();
  void showEditGrpDlg();
  void showSkinBrowser();
  void showPluginDlg();
  void showGPGKeyManager();
  void slot_AwayMsgDlg();
  void showAwayMsgDlg(unsigned short status);
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
  void UserInfoDlg_finished(const char *, unsigned long);
  void slot_doneUserEventTabDlg();
  void slot_pluginUnloaded(unsigned long);

  void sendMsg(const char* szId, unsigned long nPPID, const QString& message);
  void sendFileTransfer(const char* szId, unsigned long nPPID,
                        const QString& filename, const QString& description);
  void sendChatRequest(const char* szId, unsigned long nPPID);
  void addUser(const char* szId, unsigned long nPPID);

signals:
  void changeDockStatus(unsigned short);
  void signal_sentevent(ICQEvent *e);
  void signal_doneRegisterUser(ICQEvent *e);

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
