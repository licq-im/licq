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

#include <qwidget.h>
#include <qcombo.h>
#include <qpopmenu.h>
#include <qlistbox.h>
#include <qgrpbox.h>
#include <qtimer.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qwindowsstyle.h>

#include <vector.h>

#define QTGUI_DIR "qt-gui/"

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
class ColInfo;
class CELabel;
class CEButton;
class CEComboBox;
#ifdef USE_DOCK
class IconManager;
#endif


//=====CMainWindow==============================================================
class CMainWindow : public QWidget
{
  Q_OBJECT
public:
  CMainWindow(CICQDaemon *theServer, CSignalManager *theSigMan,
              CQtLogWindow *theLogWindow,
              const char *skinName, const char *iconsName,
              QWidget *parent = 0, const char *name = 0);
  virtual ~CMainWindow(void);
  virtual bool close(bool);

protected:
  // Command Tools
  CICQDaemon *licqDaemon;
  CSignalManager *licqSigMan;
  CQtLogWindow *licqLogWindow;
  CSkin *skin;
#ifdef USE_DOCK
  IconManager *licqIcon;
#endif

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
             *mnuUserAdm,
             *mnuStatus,
             *mnuDebug,
             *mnuRemove,
             *mnuUtilities;
  CELabel *lblStatus, *lblMsg;
  CEButton *btnSystem;
  CEComboBox *cmbUserGroups;

  QPixmap *pmBorder, *pmMask;
  QBitmap bmMask;

  // GUI Data
  char m_szCaption[128];
  QFont defaultFont;
  QWindowsStyle *winstyle;
  unsigned short m_nAutoLogon;
  char *m_szIconSet;
  QPixmap *pmOnline, *pmOffline, *pmAway, *pmDnd, *pmOccupied, *pmNa,
          *pmPrivate, *pmFFC, *pmMessage, *pmUrl, *pmChat, *pmFile;
  vector <class ColInfo> colInfo;

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
       showHeader,
       autoClose,
       m_bShowOffline,
       m_bShowDividers,
       m_bDockIcon48;
  int m_nCurrentGroup;

  int mouseX, mouseY;

  // Functions
  void ApplySkin(const char *, bool = false);
  void ApplyIcons(const char *, bool = false);
  void CreateUserView(void);
  void resizeEvent (QResizeEvent *);
  void initMenu(void);
  ICQFunctions *callFunction(int fcn, bool isUser, unsigned long _nUin = 0);

  virtual void mouseMoveEvent (QMouseEvent *);
  virtual void mousePressEvent (QMouseEvent *);
  virtual void closeEvent (QCloseEvent *);
  virtual void keyPressEvent(QKeyEvent *e);

friend class WharfIcon;
friend class IconManager;
friend class SkinBrowserDlg;
friend class OptionsDlg;

protected slots:
  void removeUserFromGroup();
  void removeUserFromList();
  void addUserToGroup(int);
  void saveAllUsers();
  void updateUserWin();
  void updateStatus();
  void updateGroups();
  void changeStatus(int index);
  void changeStatusManual(int index);
  void setCurrentGroupMenu(int id);
  void setCurrentGroup(int index);
  void callDefaultFunction();
  void callOwnerFunction();
  void callMsgFunction();
  void callFileFunction (const char *);
  void callUrlFunction (const char *);
  void callUserFunction(int fcn);
  void slot_logon();
  void slot_register();
  void slot_doneregister();
  void slot_doneOwnerFcn(ICQEvent *);
  void saveOptions();
  void showAddUserDlg();
  void showEditGrpDlg();
  void showSkinBrowser();
  void showOptionsDlg();
  void showAuthUserDlg();
  void showSearchUserDlg();
  void popupSystemMenu();
  void changeDebug(int);
  void slot_utility(int);
  void miniMode();
  void ToggleShowOffline();
  void nextServer();
  void autoAway();
  void aboutBox();
  void slot_updateContactList();
  void slot_updateAllUsers();
  void slot_shutdown();

signals:
  void changeDockStatus(unsigned short);

};


#endif
