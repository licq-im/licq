#ifndef MAINWIN_H
#define MAINWIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector.h>

#ifdef USE_KDE
#include <kmenubar.h>
#else
#include <qmenubar.h>
#endif

#include <qwidget.h>
#include <qtimer.h>
#include <qbitmap.h>

#include "userbox.h"
#include "licq_user.h"
#include "gui-defines.h"

class QStyle;

class ICQFunctions;
class CSignalManager;
class CQtLogWindow;
class CSkin;
class CICQDaemon;
class ICQEvent;
class OptionsDlg;
class AwayMsgDlg;
class RegisterUserDlg;
class CUserView;
class CELabel;
class CEButton;
class CEComboBox;
class QListViewItem;
#ifdef USE_DOCK
class IconManager;
#endif

class CICQSignal;

typedef list <ICQFunctions *> UserDataList;
typedef list <ICQFunctions *>::iterator UserDataListIter;


//=====CMainWindow==============================================================
class CMainWindow : public QWidget
{
  Q_OBJECT
public:
  CMainWindow(CICQDaemon *theServer, CSignalManager *theSigMan,
              CQtLogWindow *theLogWindow, bool bStartHidden,
              const char *skinName, const char *iconsName,
              QWidget *parent = 0, const char *name = 0);
  virtual ~CMainWindow();
  ICQFunctions *callFunction(int fcn, unsigned long _nUin);
  bool RemoveUserFromList(unsigned long, QWidget *);
  bool RemoveUserFromGroup(unsigned long, QWidget *);
  void ApplySkin(const char *, bool = false);
  void ApplyIcons(const char *, bool = false);
  CUserView *UserView()  { return userView; }

protected:
  // Command Tools
  CICQDaemon *licqDaemon;
  CSignalManager *licqSigMan;
  CQtLogWindow *licqLogWindow;
  CSkin *skin;
#ifdef USE_DOCK
  IconManager *licqIcon;
#endif
  UserDataList licqUserData;

  // Dialog boxes
  AwayMsgDlg *awayMsgDlg;
  OptionsDlg *optionsDlg;
  RegisterUserDlg *registerUserDlg;

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
             *mnuOwnerAdm,
             *mnuUserAdm,
             *mnuStatus,
             *mnuDebug,
             *mnuRemove,
             *mnuUtilities,
             *mnuAwayModes,
             *mnuSend;
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
  char *m_szIconSet;
  QPixmap pmOnline, pmOffline, pmAway, pmDnd, pmOccupied, pmNa,
          pmPrivate, pmFFC, pmMessage, pmUrl, pmChat, pmFile, pmAuthorize;
  ColumnInfos colInfo;

  // AutoAway
  QTimer autoAwayTimer;
  unsigned short autoAwayTime,
                 autoNATime,
                 manualAway,
                 m_nRealHeight;

  // Toggles
  bool inMiniMode,
       gridLines,
       m_bFontStyles,
       m_bShowGroupIfNoMsg,
       showHeader,
       autoClose,
       m_bAutoPopup,
       m_bShowOffline,
       m_bSortByStatus,
       m_bShowDividers,
       m_bAutoRaise,
       m_bBoldOnMsg;
  FlashType m_nFlash;
  unsigned long m_nCurrentGroup;
  DockMode m_nDockMode;
  GroupType m_nGroupType;

  int mouseX, mouseY;

  // Functions
  void CreateUserView();
  void CreateUserFloaty(unsigned long nUin, unsigned short x = 0,
     unsigned short y = 0, unsigned short w = 0);
  void initMenu();

  void resizeEvent (QResizeEvent *);
  virtual void mouseMoveEvent (QMouseEvent *);
  virtual void mousePressEvent (QMouseEvent *);
  virtual void closeEvent (QCloseEvent *);
  virtual void keyPressEvent(QKeyEvent *e);

friend class WharfIcon;
friend class IconManager_Default;
friend class OptionsDlg;
friend class ICQFunctions;

protected slots:
  void slot_removeUserFromGroup();
  void slot_removeUserFromList();
  void slot_hints();
  void addUserToGroup(int);
  void saveAllUsers();
  void updateUserWin();
  void updateEvents();
  void updateStatus();
  void updateGroups();
  void changeStatus(int index);
  void changeStatusManual(int index);
  void setCurrentGroupMenu(int id);
  void setCurrentGroup(int);
  void callDefaultFunction(QListViewItem *);
  void callOwnerFunction(int);
  void callMsgFunction();
  void callFileFunction (const char *);
  void callUrlFunction (const char *);
  void callUserFunction(int);
  void slot_userfinished(unsigned long);
  void slot_usermenu();
  void slot_logon();
  void slot_register();
  void slot_doneregister();
  void slot_doneOptions();
  void slot_doneOwnerFcn(ICQEvent *);
  void slot_doneAwayMsgDlg();
  void saveOptions();
  void showAddUserDlg();
  void showEditGrpDlg();
  void showSkinBrowser();
  void showPluginDlg();
  void slot_AwayMsgDlg();
  void showAwayMsgDlg(unsigned short status);
  void showOptionsDlg(int);
  void popupOptionsDlg() { emit showOptionsDlg(0); };
  void showAuthUserDlg();
  void showSearchUserDlg();
  void popupSystemMenu();
  void changeDebug(int);
  void slot_utility(int);
  void slot_awaymodes(int);
  void slot_randomchatsearch();
  void miniMode();
  void ToggleShowOffline();
  void nextServer();
  void autoAway();
  void aboutBox();
  void slot_updatedList(CICQSignal *);
  void slot_updatedUser(CICQSignal *);
  void slot_updateContactList();
  void slot_updateAllUsers();
  void slot_popupall();
  void slot_shutdown();

signals:
  void changeDockStatus(unsigned short);

};


#endif
