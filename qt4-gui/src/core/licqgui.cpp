// -*- c-basic-offset: 2; -*-
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

#include "licqgui.h"

#include "config.h"

#include <list>
#include <stdio.h> // for snprintf
#include <unistd.h> // for getopt

#include <QByteArray>
#include <QClipboard>
#include <QDesktopWidget>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QMimeData>
#include <QSessionManager>
#include <QStyle>
#include <QTranslator>
#include <QUrl>

#ifdef USE_KDE
#include <KDE/KStandardDirs>
#include <KDE/KToolInvocation>
#include <KDE/KUrl>
#else
# include <QDesktopServices>
#endif

#if defined(Q_WS_X11)
#include <QX11Info>

extern "C"
{
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef USE_SCRNSAVER
#include <X11/extensions/scrnsaver.h>
#endif
}
#undef Bool
#undef None
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef Status

#endif /* defined(Q_WS_X11) */

#include <licq_events.h>
#include <licq_file.h>
#include <licq_icq.h>
#include <licq_icqd.h>
#include <licq_sar.h>
#include <licq_user.h>

#include "config/chat.h"
#include "config/contactlist.h"
#include "config/emoticons.h"
#include "config/general.h"
#include "config/iconmanager.h"
#include "config/shortcuts.h"
#include "config/skin.h"

#include "contactlist/contactlist.h"

#include "dialogs/logwindow.h"

#include "dockicons/dockicon.h"
#include "dockicons/systemtrayicon.h"
#ifndef USE_KDE
#include "dockicons/defaultdockicon.h"
#include "dockicons/themeddockicon.h"
#endif

#include "helpers/support.h"

#include "userdlg/userdlg.h"

#include "userevents/usereventcommon.h"
#include "userevents/usereventtabdlg.h"
#include "userevents/usersendchatevent.h"
#include "userevents/usersendcontactevent.h"
#include "userevents/usersendfileevent.h"
#include "userevents/usersendmsgevent.h"
#include "userevents/usersendsmsevent.h"
#include "userevents/usersendurlevent.h"
#include "userevents/userviewevent.h"

#include "views/floatyview.h"

#include "xpm/licq.xpm"

#include "groupmenu.h"
#include "gui-defines.h"
#include "mainwin.h"
#include "messagebox.h"
#include "signalmanager.h"
#include "systemmenu.h"
#include "usermenu.h"

using namespace std;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::LicqGui */

#if defined(USE_SCRNSAVER)
static XErrorHandler old_handler = 0;
static int licq_xerrhandler(Display* dpy, XErrorEvent* err)
{
  // XScreenSaverQueryInfo produces a BadDrawable error
  // if it cannot connect to the extension. This happens i.e. when
  // client runs on a 64bit machine and the server on a 32bit one.
  // We need to catch that here and tell the Xlib that we
  // ignore it, otherwise Qt's handler will terminate us. :-(
  if (err->error_code == BadDrawable)
    return 0;

  return (*old_handler)(dpy, err);
}
#endif /* defined(USE_SCRNSAVER) */


LicqGui* LicqGui::myInstance = NULL;

LicqGui::LicqGui(int& argc, char** argv) :
#ifdef USE_KDE
  KApplication(true),
#else
  QApplication(argc, argv),
#endif
  myStartHidden(false),
  myDisableDockIcon(false),
  myUserEventTabDlg(NULL),
  grabKeysym(0)
{
  myInstance = this;

  int i = 1;

  setQuitOnLastWindowClosed(false);
  setWindowIcon(QPixmap(licq_xpm));

  // store command line arguments for session management
  myCmdLineParams << argv[0] << "-p" << PLUGIN_NAME << "--";
  for (; i < argc; i++)
    myCmdLineParams << argv[i];

  // parse command line for arguments
#ifdef USE_KDE
  while ((i = getopt(argc, argv, "hs:i:e:dD")) > 0)
#else
  while ((i = getopt(argc, argv, "hs:i:e:g:dD")) > 0)
#endif
  {
    switch (i)
    {
      case 's':   // skin name
        mySkin = optarg;
        break;

      case 'i':   // icons name
        myIcons = optarg;
        break;

      case 'e':   // extended icons name
        myExtendedIcons = optarg;
        break;

      case 'd':   // dock icon
        if (!myDisableDockIcon)
          myStartHidden = true;
        break;

      case 'D': // disable dock icon
        myStartHidden = false; // discard any -d
        myDisableDockIcon = true;
    }
  }

  // Since Licq daemon blocks SIGCHLD and Qt never receives it,
  // QProcess hangs. By this we avoid Qt's attempts to be
  // conformant to desktop settings in Cleanlooks style.
  if (style()->objectName().compare("Cleanlooks", Qt::CaseInsensitive) == 0)
    setDesktopSettingsAware(false);

  // QLocale::system() uses LC_ALL, LC_NUMERIC and LANG (in that
  // order) when determines system locale. We want LC_MESSAGES instead
  // of LC_NUMERIC so we have to do it ourselves.
#ifdef Q_OS_UNIX
  QByteArray lang = qgetenv("LC_ALL");
  if (lang.isNull())
    lang = qgetenv("LC_MESSAGES");
  if (lang.isNull())
    lang = qgetenv("LANG");
  if (lang.isNull())
    lang = "C";
  QString locale = QLocale(lang).name();
#else
  QString locale = QLocale::system().name();
#endif
  // Try to load a translation
  gLog.Info("%sAttempting to load Qt4 and Licq Qt4-GUI translations for %s locale.\n",
      L_INITxSTR, qPrintable(locale));

  QTranslator* qtTranslator = new QTranslator(this);
  qtTranslator->load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  installTranslator(qtTranslator);

  QTranslator* licqTranslator = new QTranslator(this);
  licqTranslator->load(locale, QString(SHARE_DIR) + QTGUI_DIR + "locale");
  installTranslator(licqTranslator);
}

LicqGui::~LicqGui()
{
  saveConfig();
  delete mySignalManager;
  delete myLogWindow;
  delete myMainWindow;
  delete myContactList;
  delete myGroupMenu;
  delete myUserMenu;
  delete myDockIcon;
}

void LicqGui::loadGuiConfig()
{
  char szTemp[MAX_FILENAME_LEN];

  gLog.Info("%s%s configuration.\n", L_INITxSTR, PLUGIN_NAME);
  snprintf(szTemp, MAX_FILENAME_LEN, "%s%s", BASE_DIR, QTGUI_CONFIGFILE);
  szTemp[MAX_FILENAME_LEN - 1] = '\0';
  CIniFile licqConf;
  licqConf.SetFlags(INI_FxALLOWxCREATE);
  if (!licqConf.LoadFile(szTemp))
  {
    // File doesn't exist so define sections and write them now
    // so saving won't generate warnings later
    licqConf.ReloadFile();
    licqConf.CreateSection("appearance");
    licqConf.CreateSection("functions");
    licqConf.CreateSection("startup");
    licqConf.CreateSection("locale");
    licqConf.CreateSection("floaties");
    licqConf.CreateSection("geometry");
    licqConf.FlushFile();

    // Now try to load the old config file, set the original config file back
    // in case of error or if user doesn't want to load it.
    snprintf(szTemp, MAX_FILENAME_LEN, "%s%s", BASE_DIR, "licq_qt-gui.conf");
    szTemp[MAX_FILENAME_LEN - 1] = '\0';
    if (!licqConf.LoadFile(szTemp) ||
        !QueryYesNo(NULL,
          tr("There was an error loading the default configuration file.\n"
            "Would you like to try loading the old one?")))
    {
      snprintf(szTemp, MAX_FILENAME_LEN, "%s%s", BASE_DIR, QTGUI_CONFIGFILE);
      szTemp[MAX_FILENAME_LEN - 1] = '\0';
      licqConf.LoadFile(szTemp);
    }
  }

  // Let configuration classes load their settings
  Config::General::instance()->loadConfiguration(licqConf);
  Config::Chat::instance()->loadConfiguration(licqConf);
  Config::ContactList::instance()->loadConfiguration(licqConf);
  Config::Shortcuts::instance()->loadConfiguration(licqConf);

  // Load icons
  licqConf.SetSection("appearance");

  if (myIcons.isEmpty())
  {
    licqConf.ReadStr("Icons", szTemp, "ami");
    myIcons = szTemp;
  }
  if (myExtendedIcons.isEmpty())
  {
    licqConf.ReadStr("ExtendedIcons", szTemp, "basic");
    myExtendedIcons = szTemp;
  }
  IconManager::createInstance(myIcons, myExtendedIcons, this);

  // Load Emoticons
  licqConf.ReadStr("Emoticons", szTemp, Emoticons::DEFAULT_THEME.toLatin1());
  QStringList emoticonsDirs;
  emoticonsDirs += QString::fromLocal8Bit(SHARE_DIR) + QTGUI_DIR + EMOTICONS_DIR;
  emoticonsDirs += QString::fromLocal8Bit(BASE_DIR) + QTGUI_DIR + EMOTICONS_DIR;
#ifdef USE_KDE
  // emoticons resource added in KDE 3.4
  emoticonsDirs += KGlobal::dirs()->findDirs("emoticons", "");
#endif
  Emoticons::self()->setBasedirs(emoticonsDirs);
  if (!Emoticons::self()->setTheme(Emoticons::translateThemeName(szTemp)))
    gLog.Error("%sLoading emoticons theme '%s'\n", L_ERRORxSTR, szTemp);

  // Load skin
  if (mySkin.isEmpty())
  {
    licqConf.ReadStr("Skin", szTemp, "basic");
    mySkin = szTemp;
  }
  bool skinFrameTransparent;
  licqConf.ReadBool("Transparent", skinFrameTransparent, false);
  unsigned short skinFrameStyle;
  licqConf.ReadNum("FrameStyle", skinFrameStyle, 51);

  Config::Skin::createInstance(mySkin, this);
  Config::Skin::active()->setFrameStyle(skinFrameStyle);
  Config::Skin::active()->setFrameTransparent(skinFrameTransparent);
}

void LicqGui::loadFloatiesConfig()
{
  char szTemp[MAX_FILENAME_LEN];
  snprintf(szTemp, MAX_FILENAME_LEN, "%s%s", BASE_DIR, QTGUI_CONFIGFILE);
  szTemp[MAX_FILENAME_LEN - 1] = '\0';
  CIniFile licqConf;
  licqConf.LoadFile(szTemp);

  char key[16];
  unsigned short nFloaties = 0, xPosF, yPosF, wValF;
  licqConf.SetSection("floaties");
  licqConf.ReadNum("Num", nFloaties, 0);
  for (unsigned short i = 0; i < nFloaties; i++)
  {
    sprintf(key, "Floaty%d.Ppid", i);
    unsigned long ppid;
    licqConf.ReadNum(key, ppid, LICQ_PPID);
    sprintf(key, "Floaty%d.Uin", i);
    licqConf.ReadStr(key, szTemp, "");
    if (szTemp[0] == '\0')
      continue;
    UserId userId = LicqUser::makeUserId(szTemp, ppid);

    sprintf(key, "Floaty%d.X", i);
    licqConf.ReadNum(key, xPosF, 0);
    sprintf(key, "Floaty%d.Y", i);
    licqConf.ReadNum(key, yPosF, 0);
    sprintf(key, "Floaty%d.W", i);
    licqConf.ReadNum(key, wValF, 80);

    if (USERID_ISVALID(userId))
      createFloaty(userId, xPosF, yPosF, wValF);
  }
}

void LicqGui::saveConfig()
{
  // Tell the daemon to save its options
  myLicqDaemon->SaveConf();

  // Save all our options
  char filename[MAX_FILENAME_LEN];
  snprintf(filename, MAX_FILENAME_LEN, "%s%s", BASE_DIR, QTGUI_CONFIGFILE);
  filename[MAX_FILENAME_LEN - 1] = '\0';
  CIniFile licqConf(INI_FxERROR | INI_FxALLOWxCREATE);

  if (!licqConf.LoadFile(filename))
    return;

  Config::General::instance()->saveConfiguration(licqConf);
  Config::Chat::instance()->saveConfiguration(licqConf);
  Config::ContactList::instance()->saveConfiguration(licqConf);
  Config::Shortcuts::instance()->saveConfiguration(licqConf);

  licqConf.SetSection("appearance");
  licqConf.WriteStr("Skin", Config::Skin::active()->skinName().toLocal8Bit());
  licqConf.WriteStr("Icons", IconManager::instance()->iconSet().toLocal8Bit());
  licqConf.WriteStr("ExtendedIcons", IconManager::instance()->extendedIconSet().toLocal8Bit());
  licqConf.WriteStr("Emoticons", Emoticons::untranslateThemeName(Emoticons::self()->theme()).toLatin1());

  licqConf.WriteBool("Transparent", Config::Skin::active()->frame.transparent);
  licqConf.WriteNum("FrameStyle", Config::Skin::active()->frame.frameStyle);

  char key[32];
  licqConf.SetSection("floaties");
  licqConf.WriteNum("Num", (unsigned short)FloatyView::floaties.size());
  for (unsigned short i = 0; i < FloatyView::floaties.size(); i++)
  {
    FloatyView* iter = FloatyView::floaties.at(i);
    sprintf(key, "Floaty%d.Ppid", i);
    licqConf.WriteNum(key, LicqUser::getUserProtocolId(iter->userId()));
    sprintf(key, "Floaty%d.Uin", i);
    licqConf.writeString(key, LicqUser::getUserAccountId(iter->userId()));
    sprintf(key, "Floaty%d.X", i);
    licqConf.WriteNum(key, (unsigned short)(iter->x() > 0 ? iter->x() : 0));
    sprintf(key, "Floaty%d.Y", i);
    licqConf.WriteNum(key, (unsigned short)(iter->y() > 0 ? iter->y() : 0));
    sprintf(key, "Floaty%d.W", i);
    licqConf.WriteNum(key, (unsigned short)iter->width());
  }

  licqConf.FlushFile();
}

int LicqGui::Run(CICQDaemon* daemon)
{
  myLicqDaemon = daemon;

  // Register with the daemon, we want to receive all signals
  int pipe = daemon->RegisterPlugin(SIGNAL_ALL);

  // Create the configuration handlers
  Config::General::createInstance(this);
  Config::ContactList::createInstance(this);
  Config::Chat::createInstance(this);
  Config::Shortcuts::createInstance(this);

#ifdef Q_WS_X11
  connect(Config::General::instance(),
      SIGNAL(msgPopupKeyChanged(QString)), SLOT(grabKey(QString)));
#endif

  // Create the main widgets
  mySignalManager = new SignalManager(pipe);
  myLogWindow = new LogWindow;

  gLog.AddService(new CLogService_Plugin(myLogWindow, L_MOST));

  // Check for qt-gui directory in current base dir
  if (!QDir(QString("%1/%2").arg(BASE_DIR).arg(QTGUI_DIR)).exists())
  {
    QDir d;
    d.mkdir(QString("%1/%2").arg(BASE_DIR).arg(QTGUI_DIR));
  }

  loadGuiConfig();

  // Contact list model
  myContactList = new ContactListModel(this);
  connect(mySignalManager, SIGNAL(updatedList(unsigned long, int, const UserId&)),
      myContactList, SLOT(listUpdated(unsigned long, int, const UserId&)));
  connect(mySignalManager, SIGNAL(updatedUser(const UserId&, unsigned long, int, unsigned long)),
      myContactList, SLOT(userUpdated(const UserId&, unsigned long, int)));

  connect(mySignalManager, SIGNAL(updatedList(unsigned long, int, const UserId&)),
      SLOT(listUpdated(unsigned long, int, const UserId&)));
  connect(mySignalManager, SIGNAL(updatedUser(const UserId&, unsigned long, int, unsigned long)),
      SLOT(userUpdated(const UserId&, unsigned long, int, unsigned long)));
  connect(mySignalManager, SIGNAL(socket(const UserId&, unsigned long)),
      SLOT(convoSet(const UserId&, unsigned long)));
  connect(mySignalManager, SIGNAL(convoJoin(const UserId&, unsigned long, unsigned long)),
      SLOT(convoJoin(const UserId&, unsigned long, unsigned long)));
  connect(mySignalManager, SIGNAL(convoLeave(const UserId&, unsigned long, unsigned long)),
      SLOT(convoLeave(const UserId&, unsigned long, unsigned long)));
  connect(mySignalManager, SIGNAL(ui_message(const UserId&)),
      SLOT(showMessageDialog(const UserId&)));
  connect(mySignalManager, SIGNAL(ui_viewevent(const UserId&)),
      SLOT(showNextEvent(const UserId&)));

  myUserMenu = new UserMenu();
  myGroupMenu = new GroupMenu();
  myDockIcon = NULL;

  // Overwrite Qt's event handler
#if defined(USE_SCRNSAVER)
  old_handler = XSetErrorHandler(licq_xerrhandler);
#endif /* defined(USE_SCRNSAVER) */

  myMainWindow = new MainWindow(myStartHidden);

  loadFloatiesConfig();

  if (!myDisableDockIcon)
  {
    updateDockIcon();
    connect(Config::General::instance(), SIGNAL(dockModeChanged()), SLOT(updateDockIcon()));
  }

  myAutoAwayTimer.start(10000);  // start the inactivity timer for auto away
  connect(&myAutoAwayTimer, SIGNAL(timeout()), SLOT(autoAway()));

  // automatically logon if requested in conf file
  unsigned short autoLogon = Config::General::instance()->autoLogon();
  if (autoLogon > 0)
  {
    bool invisible = (autoLogon >= 10);
    switch (autoLogon % 10)
    {
      case 0: break;
      case 1: changeStatus(ICQ_STATUS_ONLINE, invisible); break;
      case 2: changeStatus(ICQ_STATUS_AWAY, invisible); break;
      case 3: changeStatus(ICQ_STATUS_NA, invisible); break;
      case 4: changeStatus(ICQ_STATUS_OCCUPIED, invisible); break;
      case 5: changeStatus(ICQ_STATUS_DND, invisible); break;
      case 6: changeStatus(ICQ_STATUS_FREEFORCHAT, invisible); break;
      default: gLog.Warn("%sInvalid auto online id: %d.\n", L_WARNxSTR, autoLogon);
    }
  }

  int r = exec();

  daemon->UnregisterPlugin();

  gLog.Info("%sShutting down gui.\n", L_ENDxSTR);
  gLog.ModifyService(S_PLUGIN, 0);

  return r;
}

void LicqGui::commitData(QSessionManager& sm)
{
  if (sm.allowsInteraction())
  {
    saveConfig();
    sm.release();
  }

  QStringList restartCmd;
  restartCmd  = myCmdLineParams;
  restartCmd += QString("-session");
  restartCmd += sm.sessionId();
  sm.setRestartCommand(restartCmd);
}

void LicqGui::saveState(QSessionManager& sm)
{
  sm.setRestartHint(QSessionManager::RestartIfRunning);
}

#if defined(Q_WS_X11)
bool LicqGui::x11EventFilter(XEvent* event)
{
  if (event->type == KeyPress && grabKeysym)
  {
    Display* dsp = QX11Info::display();
    unsigned int mod = event->xkey.state & (ControlMask | ShiftMask | Mod1Mask);
    unsigned int keysym = XKeycodeToKeysym(dsp, event->xkey.keycode, 0);

    if (keysym == Support::keyToXSym(grabKeysym) &&
        mod == Support::keyToXMod(grabKeysym))
      showNextEvent();

    if (!QWidget::keyboardGrabber())
    {
      XAllowEvents(dsp, AsyncKeyboard, CurrentTime);
      XUngrabKeyboard(dsp, CurrentTime);
      XSync(dsp, false);
    }
  }
#ifdef USE_KDE
  return KApplication::x11EventFilter(event);
#else
  return QApplication::x11EventFilter(event);
#endif
}

void LicqGui::grabKey(const QString& key)
{
  Display* dsp = QX11Info::display();
  Qt::HANDLE rootWin = QX11Info::appRootWindow();

  // Stop grabbing old key
  if (grabKeysym != 0)
  {
    XGrabKey(dsp, XKeysymToKeycode(dsp, Support::keyToXSym(grabKeysym)),
        Support::keyToXMod(grabKeysym), rootWin, false,
        GrabModeAsync, GrabModeSync);
    grabKeysym = 0;
  }

  if (key.isEmpty())
    return;

  grabKeysym = QKeySequence(key);

  if (grabKeysym == 0)
  {
    gLog.Error("%sUnknown popup key: %s\n", L_INITxSTR, key.toLatin1().data());
    return;
  }

  XGrabKey(dsp, XKeysymToKeycode(dsp, Support::keyToXSym(grabKeysym)),
      Support::keyToXMod(grabKeysym), rootWin, true,
      GrabModeAsync, GrabModeSync);
}
#endif /* defined(Q_WS_X11) */

void LicqGui::changeStatus(unsigned long status, bool invisible)
{
  FOR_EACH_PROTO_PLUGIN_START(myLicqDaemon)
  {
    unsigned long ppid = (*_ppit)->PPID();

    // Keep invisible mode on protocols when changing global status
    bool protoInvisible = invisible;
    if (status != ICQ_STATUS_FxPRIVATE && myMainWindow->systemMenu()->getInvisibleStatus(ppid))
      protoInvisible = true;

    changeStatus(status, ppid, protoInvisible);
  }
  FOR_EACH_PROTO_PLUGIN_END
}

void LicqGui::changeStatus(unsigned long status, unsigned long ppid, bool invisible)
{
  const ICQOwner* o = gUserManager.FetchOwner(ppid, LOCK_R);
  if (o == NULL)
    return;

  if (status == ICQ_STATUS_FxPRIVATE)
  {
    if (o->StatusOffline())
    {
      gUserManager.DropOwner(o);
      return;
    }

    status = o->StatusFull();

    if (invisible)
      status |= ICQ_STATUS_FxPRIVATE;
    else
      status &= (~ICQ_STATUS_FxPRIVATE);
  }
  else if(status != ICQ_STATUS_OFFLINE)
  {
    if (o->StatusInvisible() || invisible)
      status |= ICQ_STATUS_FxPRIVATE;
  }

  UserId ownerId = o->id();
  gUserManager.DropOwner(o);

  myLicqDaemon->protoSetStatus(ownerId, status);
}

bool LicqGui::removeUserFromList(const UserId& userId, QWidget* parent)
{
  if (parent == NULL)
    parent = myMainWindow;

  const LicqUser* u = gUserManager.fetchUser(userId, LOCK_R);
  if (u == NULL)
    return true;
  QString warning(tr("Are you sure you want to remove\n%1 (%2)\nfrom your contact list?")
      .arg(QString::fromUtf8(u->GetAlias()))
      .arg(u->IdString()));
  gUserManager.DropUser(u);
  if (QueryYesNo(parent, warning))
  {
    gUserManager.removeUser(userId);
    return true;
  }
  return false;
}

void LicqGui::showInfoDialog(int /* fcn */, const UserId& userId, bool toggle, bool updateNow)
{
  if (!USERID_ISVALID(userId))
    return;

  UserDlg* f = NULL;

  for (int i = 0; i < myUserDlgList.size(); ++i)
  {
    UserDlg* item = myUserDlgList.at(i);
    if (item->userId() == userId)
    {
      f = item;
      break;
    }
  }

  UserDlg::UserPage tab = UserDlg::GeneralPage;

  if (f != NULL)
  {
    if (toggle && f->currentPage() == tab)
    {
      delete f; // will notify us about deletion
      return;
    }
    else
    {
      f->show();
      f->raise();
    }
  }
  else
  {
    f = new UserDlg(userId);
    connect(f, SIGNAL(finished(UserDlg*)), SLOT(userDlgFinished(UserDlg*)));
    f->show();
    myUserDlgList.append(f);
  }

  f->showPage(tab);
  f->show();
  f->raise();
  if (updateNow)
    f->retrieveSettings();
}

UserViewEvent* LicqGui::showViewEventDialog(const UserId& userId)
{
  if (!USERID_ISVALID(userId))
    return NULL;

  for (int i = 0; i < myUserViewList.size(); ++i)
  {
    UserViewEvent* e = myUserViewList.at(i);
    if (e->userId() == userId)
    {
      e->show();
      if (Config::Chat::instance()->autoFocus() &&
          (!qApp->activeWindow() || !qApp->activeWindow()->inherits("UserEventCommon")))
      {
        e->raise();
        e->activateWindow();
      }
      return e;
    }
  }

  UserViewEvent* e = new UserViewEvent(userId);

  e->show();
  userEventFinished(userId);
  connect(e, SIGNAL(finished(const UserId&)), SLOT(userEventFinished(const UserId&)));
  myUserViewList.append(e);

  return e;
}

UserEventCommon* LicqGui::showEventDialog(int fcn, const UserId& userId, int convoId, bool autoPopup)
{
  if (!USERID_ISVALID(userId))
    return NULL;

  LicqUser* user = gUserManager.fetchUser(userId);
  if (user == NULL)
    return NULL;
  QString id = user->accountId().c_str();
  unsigned long ppid = user->ppid();
  gUserManager.DropUser(user);

  // Find out what's supported for this protocol
  unsigned long sendFuncs = 0xFFFFFFFF;
  if (ppid != LICQ_PPID)
  {
    FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
    {
      if ((*_ppit)->PPID() == ppid)
      {
        sendFuncs = (*_ppit)->SendFunctions();
        break;
      }
    }
    FOR_EACH_PROTO_PLUGIN_END
  }

  // Check if the protocol for this contact support the function we want to open
  if ((fcn == MessageEvent && !(sendFuncs & PP_SEND_MSG)) ||
      (fcn == UrlEvent && !(sendFuncs & PP_SEND_URL)) ||
      (fcn == ChatEvent && !(sendFuncs & PP_SEND_CHAT)) ||
      (fcn == FileEvent && !(sendFuncs & PP_SEND_FILE )) ||
      (fcn == ContactEvent && !(sendFuncs & PP_SEND_CONTACT)) ||
      (fcn == SmsEvent && !(sendFuncs & PP_SEND_SMS)))
    return NULL;


  // Focus the new window/tab after showing it
  bool activateMsgwin = true;

  if (autoPopup)
  {
    // Message is triggered by auto-popup (and not by user) so check if it
    // really should be activated
    activateMsgwin = Config::Chat::instance()->autoFocus();

    // Don't change focus if another message window is already active
    const QWidget* activeWin = QApplication::activeWindow();
    if (activeWin != NULL && ((qobject_cast<const UserEventCommon*>(activeWin)) != NULL ||
        (qobject_cast<const UserEventTabDlg*>(activeWin)) != NULL))
      activateMsgwin = false;
  }

  if (Config::Chat::instance()->msgChatView())
  {
    for (int i = 0; i < myUserSendList.size(); ++i)
    {
      UserSendCommon* e = myUserSendList.at(i);

      // Protocols (MSN only atm) that support convo ids are differentiated from
      // the icq protocol because the convo id will be the server socket.. which does
      // not meet the requirement that convo ids must be unique for each conversation.
      if ( (ppid == MSN_PPID && (e->isUserInConvo(userId) ||
              (e->convoId() == (unsigned long)(convoId) && e->convoId() != (unsigned long)(-1)))) ||
          e->isUserInConvo(userId))
      {
        //if (!e->FindUserInConvo(id))
        //  e->convoJoin(id);

        QWidget* msgWindow = e;
        if (myUserEventTabDlg != NULL && myUserEventTabDlg->tabExists(e))
        {
          msgWindow = myUserEventTabDlg;
          if (activateMsgwin)
            myUserEventTabDlg->selectTab(e);
        }

        msgWindow->show();
        msgWindow->setWindowState(msgWindow->windowState() & ~Qt::WindowMinimized);
        if (activateMsgwin)
        {
          // FIXME: When restoring from minimized, the window isn't activated (at least not on KDE)
          msgWindow->raise();
          msgWindow->activateWindow();
        }

        // Make the existing event dialog change to the new event type
        e = e->changeEventType(fcn);
        return e;
      }
    }
  }

  bool newtabw = false;
  QWidget* parent = NULL;
  if (Config::Chat::instance()->tabbedChatting())
  {
    if (myUserEventTabDlg == NULL)
    {
      // create the tab dialog if it does not exist
      myUserEventTabDlg = new UserEventTabDlg(0, "UserEventTabbedDialog");
      connect(myUserEventTabDlg, SIGNAL(signal_done()), SLOT(userEventTabDlgDone()));
      newtabw = true;
    }
    parent = myUserEventTabDlg;
  }

  UserEventCommon* e = NULL;

  switch (fcn)
  {
    case MessageEvent:
      e = new UserSendMsgEvent(userId, parent);
      break;

    case UrlEvent:
      e = new UserSendUrlEvent(userId, parent);
      break;

    case ChatEvent:
      e = new UserSendChatEvent(userId, parent);
      break;

    case FileEvent:
      e = new UserSendFileEvent(userId, parent);
      break;

    case ContactEvent:
      e = new UserSendContactEvent(userId, parent);
      break;

    case SmsEvent:
      e = new UserSendSmsEvent(userId, parent);
      break;

    default:
      gLog.Warn("%sunknown callFunction() fcn: %d\n", L_WARNxSTR, fcn);
  }
  if (e == NULL) return NULL;

  QWidget* msgWindow = e;
  if (Config::Chat::instance()->tabbedChatting())
  {
    msgWindow = myUserEventTabDlg;

    myUserEventTabDlg->addTab(e);
    if (activateMsgwin)
      myUserEventTabDlg->selectTab(e);

    // Check if we want the window sticky
    if (Config::Chat::instance()->msgWinSticky())
      QTimer::singleShot(100, myUserEventTabDlg, SLOT(setMsgWinSticky()));
  }

  // FIXME: New windows always become active stealing focus from other event windows (at least on KDE)
  msgWindow->show();
  msgWindow->setWindowState(msgWindow->windowState() & ~Qt::WindowMinimized);
  if (activateMsgwin)
  {
    // FIXME: When restoring from minimized, the window isn't activated (at least not on KDE)
    msgWindow->raise();
    msgWindow->activateWindow();
  }

  // Since daemon doesn't notify us when an event is sent we need to take care of it ourselfs
  // Get signals from event dialog and forward it to anyone who needs it
  connect(e, SIGNAL(eventSent(const LicqEvent*)), SIGNAL(eventSent(const LicqEvent*)));

  // there might be more than one send window open
  // make sure we only remember one, or it will get complicated
  sendEventFinished(userId);
  connect(e, SIGNAL(finished(const UserId&)), SLOT(sendEventFinished(const UserId&)));
  myUserSendList.append(static_cast<UserSendCommon*>(e));

  return e;
}

void LicqGui::replaceEventDialog(UserSendCommon* oldDialog, UserSendCommon* newDialog, const UserId& userId)
{
  disconnect(oldDialog, SIGNAL(finished(const UserId&)), this, SLOT(sendEventFinished(const UserId&)));
  sendEventFinished(userId);
  connect(newDialog, SIGNAL(eventSent(const LicqEvent*)), SIGNAL(eventSent(const LicqEvent*)));
  connect(newDialog, SIGNAL(finished(const UserId&)), SLOT(sendEventFinished(const UserId&)));
    myUserSendList.append(newDialog);
}

void LicqGui::showMessageDialog(const UserId& userId)
{
  showEventDialog(MessageEvent, userId);
}

void LicqGui::sendMsg(const UserId& userId, const QString& message)
{
  UserSendCommon* event = dynamic_cast<UserSendCommon*>(showEventDialog(MessageEvent, userId));
  if (event == 0)
    return;

  event->setText(message);
}

void LicqGui::sendFileTransfer(const UserId& userId, const QString& filename, const QString& description)
{
  UserSendFileEvent* event = dynamic_cast<UserSendFileEvent*>(showEventDialog(FileEvent, userId));
  if (event == 0)
    return;

  event->setFile(filename, description);
}

void LicqGui::sendChatRequest(const UserId& userId)
{
  UserSendCommon* event = dynamic_cast<UserSendCommon*>(showEventDialog(ChatEvent, userId));
  if (event == 0)
    return;
}

bool LicqGui::userDropEvent(const UserId& userId, const QMimeData& mimeData)
{
  if (mimeData.hasUrls())
  {
    QList<QUrl> urlList = mimeData.urls();
    QListIterator<QUrl> urlIter(urlList);
    QString text;
    QUrl firstUrl = urlIter.next();

    if (!(text = firstUrl.toLocalFile()).isEmpty())
    {
      // Local file(s), open send file dialog
      UserEventCommon* x = showEventDialog(FileEvent, userId);
      UserSendFileEvent* sendFile = dynamic_cast<UserSendFileEvent*>(x);
      if (!sendFile)
        return false;

      sendFile->setFile(text, QString::null);

      // Add all the files
      while (urlIter.hasNext())
      {
        if (!(text = urlIter.next().toLocalFile()).isEmpty())
          sendFile->addFile(text);
      }

      sendFile->show();
    }
    else
    {
      // Not local file, open URL dialog
      UserSendUrlEvent* sendUrl = dynamic_cast<UserSendUrlEvent*>(showEventDialog(UrlEvent, userId));
      if (!sendUrl)
        return false;

      sendUrl->setUrl(firstUrl.toString(), QString::null);
      sendUrl->show();
    }
  }
  else if (mimeData.hasText())
  {
    // Text might be a user id

    QString text = mimeData.text();

    unsigned long dropPpid = 0;
    FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
    {
      if (text.startsWith(PPIDSTRING((*_ppit)->PPID())))
      {
        dropPpid = (*_ppit)->PPID();
        break;
      }
    }
    FOR_EACH_PROTO_PLUGIN_END

    if (dropPpid != 0 && text.length() > 4)
    {
      QString dropId = text.mid(4);
      UserId dropUserId = LicqUser::makeUserId(dropId.toLatin1().data(), dropPpid);
      if (!USERID_ISVALID(dropUserId) || userId == dropUserId)
        return false;

      UserSendContactEvent* sendContact = dynamic_cast<UserSendContactEvent*>(showEventDialog(ContactEvent, userId));
      if (!sendContact)
        return false;

      sendContact->setContact(dropUserId);
      sendContact->show();
    }
    else
    {
      UserSendMsgEvent* sendMsg = dynamic_cast<UserSendMsgEvent*>(showEventDialog(MessageEvent, userId));
      if (!sendMsg)
        return false;

      sendMsg->setText(text);
      sendMsg->show();
    }
  }

  return true;
}

void LicqGui::viewUrl(const QString& url)
{
#ifdef USE_KDE
  if (url.startsWith("mailto:"))
    KToolInvocation::invokeMailer(KUrl(url));
  else
    KToolInvocation::invokeBrowser(url);
#else
  bool useCustomUrlBrowser(Config::Chat::instance()->useCustomUrlBrowser());

  if (useCustomUrlBrowser || (!useCustomUrlBrowser && !QDesktopServices::openUrl(QUrl(url))))
  {
    if (!myLicqDaemon->getUrlViewer())
      myLicqDaemon->setUrlViewer(DEFAULT_URL_VIEWER);
    if (!myLicqDaemon->ViewUrl(url.toLocal8Bit().data()))
      WarnUser(NULL, tr("Licq is unable to start your browser and open the URL.\n"
            "You will need to start the browser and open the URL manually."));
  }
#endif
}

void LicqGui::userDlgFinished(UserDlg* dialog)
{
  if (myUserDlgList.removeAll(dialog) > 0)
    return;

  gLog.Warn("%sUser Info finished signal for user with no window (%s)!\n",
      L_WARNxSTR, USERID_TOSTR(dialog->userId()));
}

void LicqGui::userEventTabDlgDone()
{
  myUserEventTabDlg = NULL;
}

void LicqGui::userEventFinished(const UserId& userId)
{
  for (int i = 0; i < myUserViewList.size(); ++i)
  {
    UserViewEvent* item = myUserViewList.at(i);
    if (item->userId() == userId)
    {
      myUserViewList.removeAll(item);
      return;
    }
  }
}

void LicqGui::sendEventFinished(const UserId& userId)
{
  // go through the whole list, since there might be more than one hit
  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->userId() == userId)
      myUserSendList.removeAll(item);
  }
}

void LicqGui::showDefaultEventDialog(const UserId& userId)
{
  if (!USERID_ISVALID(userId))
    return;

  const LicqUser* u = gUserManager.fetchUser(userId, LOCK_R);

  if (u == NULL)
    return;

  QString id = u->accountId().c_str();
  unsigned long ppid = u->ppid();

  // For multi user conversations (i.e. in MSN)
  int convoId = -1;

  // set default function to read or send depending on whether or not
  // there are new messages
  bool send = (u->NewMessages() == 0);
  if (!send && Config::Chat::instance()->msgChatView())
  {
    // if one of the new events is a msg in chatview mode,
    // change def function to send
    for (unsigned short i = 0; i < u->NewMessages(); i++)
      if (u->EventPeek(i)->SubCommand() == ICQ_CMDxSUB_MSG ||
          u->EventPeek(i)->SubCommand() == ICQ_CMDxSUB_URL)
      {
        convoId = u->EventPeek(i)->ConvoId();
        send = true;
        break;
      }
  }
  gUserManager.DropUser(u);

  if (!send)
  {
    // Messages pending and not open in chatview mode so open view event dialog
    showViewEventDialog(userId);
    return;
  }

  // See if the clipboard contains a url or file
  if (Config::Chat::instance()->sendFromClipboard())
  {
    QClipboard* clip = QApplication::clipboard();
    QClipboard::Mode mode = QClipboard::Clipboard;

    QString c = clip->text(mode);

    if (c.isEmpty() && clip->supportsSelection())
    {
      mode = QClipboard::Selection;
      c = clip->text(mode);
    }

    // Check which message types are supported for this protocol
    unsigned long sendFuncs = 0xFFFFFFFF;
    if (ppid != LICQ_PPID)
    {
      FOR_EACH_PROTO_PLUGIN_START(gLicqDaemon)
      {
        if ((*_ppit)->PPID() == ppid)
        {
          sendFuncs = (*_ppit)->SendFunctions();
          break;
        }
      }
      FOR_EACH_PROTO_PLUGIN_END
    }

    if (sendFuncs & PP_SEND_URL && (c.left(5) == "http:" || c.left(4) == "ftp:" || c.left(6) == "https:"))
    {
      UserEventCommon* ec = showEventDialog(UrlEvent, userId);
      if (!ec || ec->objectName() != "UserSendUrlEvent")
        return;
      UserSendUrlEvent* e = dynamic_cast<UserSendUrlEvent*>(ec);
      // Set the url
      e->setUrl(c, "");
      // Clear the buffer now
      clip->clear(mode);
      return;
    }
    else if (sendFuncs & PP_SEND_FILE && (c.left(5) == "file:" || c.left(1) == "/"))
    {
      UserEventCommon* ec = showEventDialog(FileEvent, userId);
      if (!ec || ec->objectName() != "UserSendFileEvent")
        return;
      UserSendFileEvent* e = dynamic_cast<UserSendFileEvent*>(ec);
      // Set the file
      if(c.left(5) == "file:")
        c.remove(0, 5);
      while (c[0] == '/')
        c.remove(0, 1);
      c.prepend('/');
      e->setFile(c, "");
      // Clear the buffer now
      clip->clear(mode);
      return;
    }
  }

  showEventDialog(MessageEvent, userId, convoId);
}

void LicqGui::showAllOwnerEvents()
{
  FOR_EACH_PROTO_PLUGIN_START(myLicqDaemon)
  {
    const ICQOwner* o = gUserManager.FetchOwner((*_ppit)->PPID(), LOCK_R);
    if (o == NULL)
      continue;
    unsigned short nNumMsg = o->NewMessages();
    UserId userId = o->id();
    gUserManager.DropOwner(o);

    if (nNumMsg > 0)
      showViewEventDialog(userId);
  }
  FOR_EACH_PROTO_PLUGIN_END
}

void LicqGui::showNextEvent(const UserId& uid)
{
  // Do nothing if there are no events pending
  if (LicqUser::getNumUserEvents() == 0)
    return;

  UserId userId = uid;

  if (!USERID_ISVALID(userId))
  {
    // Do system messages first
    FOR_EACH_PROTO_PLUGIN_START(myLicqDaemon)
    {
      const ICQOwner* o = gUserManager.FetchOwner((*_ppit)->PPID(), LOCK_R);
      if (o == NULL)
        continue;
      unsigned short nNumMsg = o->NewMessages();
      gUserManager.DropOwner(o);
      if (nNumMsg > 0)
      {
        showAllOwnerEvents();
        return;
      }
    }
    FOR_EACH_PROTO_PLUGIN_END

    time_t t = time(NULL);
    FOR_EACH_USER_START(LOCK_R)
    {
      if (pUser->NewMessages() > 0 && pUser->Touched() <= t)
      {
        userId = pUser->id();
        t = pUser->Touched();
      }
    }
    FOR_EACH_USER_END
  }

  if (USERID_ISVALID(userId))
  {
    if (Config::Chat::instance()->msgChatView())
    {
      const LicqUser* u = gUserManager.fetchUser(userId);
      if (u == NULL)
        return;

      for (unsigned short i = 0; i < u->NewMessages(); i++)
      {
        if (u->EventPeek(i)->SubCommand() == ICQ_CMDxSUB_MSG ||
            u->EventPeek(i)->SubCommand() == ICQ_CMDxSUB_URL)
        {
          gUserManager.DropUser(u);
          showEventDialog(MessageEvent, userId, u->EventPeek(i)->ConvoId());
          return;
        }
      }
      gUserManager.DropUser(u);
    }

    showViewEventDialog(userId);
  }
}

void LicqGui::showAllEvents()
{
  // Do nothing if there are no events pending
  if (LicqUser::getNumUserEvents() == 0)
    return;

  // Do system messages first
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  unsigned short numMsg = 0;
  if (o != NULL)
  {
    numMsg = o->NewMessages();
    gUserManager.DropOwner(o);
  }

  if (numMsg > 0)
    showAllOwnerEvents();

  list<UserId> users;
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->NewMessages() > 0)
      users.push_back(pUser->id());
  }
  FOR_EACH_USER_END

  list<UserId>::iterator iter;
  for (iter = users.begin(); iter != users.end(); iter++)
    showDefaultEventDialog(*iter);
}

void LicqGui::toggleFloaty(const UserId& userId)
{
  FloatyView* v = FloatyView::findFloaty(userId);
  if (v == NULL)
    createFloaty(userId);
  else
    delete v;
}

void LicqGui::createFloaty(const UserId& userId,
   unsigned short x, unsigned short y, unsigned short w)
{
  if (!USERID_ISVALID(userId) || !gUserManager.userExists(userId))
    return;

  FloatyView* f = new FloatyView(myContactList, userId);

  connect(f, SIGNAL(userDoubleClicked(const UserId&)),
      SLOT(showDefaultEventDialog(const UserId&)));

  // not so good, we should allow for multiple guys in one box...
  // perhaps use the viewport sizeHint
  f->setFixedHeight(f->sizeHintForRow(0) + f->frameWidth() * 2);

  if (w != 0)
  {
    if (y > QApplication::desktop()->height() - 16) y = 0;
    if (x > QApplication::desktop()->width() - 16) x = 0;
    f->setGeometry(x, y, w, f->height());
  }

  f->show();
}

void LicqGui::listUpdated(unsigned long subSignal, int /* argument */, const UserId& userId)
{
  switch (subSignal)
  {
    case LIST_REMOVE:
    {
      // If their floaty is enabled, remove it
      FloatyView* f = FloatyView::findFloaty(userId);
      if (f)
        delete f;

      // If their view box is open, kill it
      for (int i = 0; i < myUserViewList.size(); ++i)
      {
        UserViewEvent* item = myUserViewList.at(i);
        if (item->userId() == userId)
        {
          item->close();
          myUserViewList.removeAll(item);
          break;
        }
      }
      // if their info box is open, kill it
      for (int i = 0; i < myUserDlgList.size(); ++i)
      {
        UserDlg* item = myUserDlgList.at(i);
        if (item->userId() == userId)
        {
          item->close();
          myUserDlgList.removeAll(item);
          break;
        }
      }
      // if their send box is open, kill it
      for (int i = 0; i < myUserSendList.size(); ++i)
      {
        UserSendCommon* item = myUserSendList.at(i);
        if (item->userId() == userId)
        {
          if (myUserEventTabDlg && myUserEventTabDlg->tabExists(item))
            myUserEventTabDlg->removeTab(item);
          else
            item->close();
          myUserSendList.removeAll(item);
          break;
        }
      }

      break;
    }

    case LIST_ALL:
    case LIST_GROUP_ADDED:
    case LIST_GROUP_REMOVED:
    case LIST_GROUP_CHANGED:
      myMainWindow->updateGroups();
      myUserMenu->updateGroups();
      myGroupMenu->updateGroups();
      break;
  }
}

void LicqGui::userUpdated(const UserId& userId, unsigned long subSignal, int argument, unsigned long cid)
{
  const LicqUser* u = gUserManager.fetchUser(userId, LOCK_R);
  if (u == NULL)
  {
    gLog.Warn("%sLicqGui::userUpdated(): Invalid user received: %s\n",
        L_ERRORxSTR, USERID_TOSTR(userId));
    return;
  }
  QString id = u->accountId().c_str();
  unsigned long ppid = u->ppid();
  gUserManager.DropUser(u);

  switch (subSignal)
  {
    case USER_EVENTS:
    {
      // Skip all this if it was just an away message check
      if (argument == 0)
        break;

      if (argument > 0)
      {
        unsigned short popCheck = 99;

        const ICQOwner* o = gUserManager.FetchOwner(ppid, LOCK_R);
        if (o != NULL)
        {
          switch (o->Status())
          {
            case ICQ_STATUS_ONLINE:
            case ICQ_STATUS_FREEFORCHAT:
              popCheck = 1;
              break;
            case ICQ_STATUS_AWAY:
              popCheck = 2;
              break;
            case ICQ_STATUS_NA:
              popCheck = 3;
              break;
            case ICQ_STATUS_OCCUPIED:
              popCheck = 4;
              break;
            case ICQ_STATUS_DND:
              popCheck = 5;
              break;
          }
          gUserManager.DropOwner(o);
        }

        if (Config::Chat::instance()->autoPopup() >= popCheck)
        {
          const LicqUser* u = gUserManager.fetchUser(userId, LOCK_R);
          if (u != NULL)
          {
            bool bCallUserView = false, bCallSendMsg = false;

            if (u->NewMessages() > 0)
            {
              if (Config::Chat::instance()->msgChatView())
              {
                for (unsigned short i = 0; i < u->NewMessages(); i++)
                {
                  switch (u->EventPeek(i)->SubCommand())
                  {
                    case ICQ_CMDxSUB_MSG:
                    case ICQ_CMDxSUB_URL:
                      bCallSendMsg = true;
                      break;
                    default:
                      bCallUserView = true;
                  }
                  if (bCallSendMsg && bCallUserView)
                    break;
                }
              }
              else
                bCallUserView = true;
            }

            gUserManager.DropUser(u);

            if (bCallSendMsg)
              showEventDialog(MessageEvent, userId, cid, true);
            if (bCallUserView)
              showViewEventDialog(userId);
          }
        }
      }
      // Fall through
    }
    case USER_STATUS:
    case USER_BASIC:
    case USER_GENERAL:
    case USER_EXT:
    case USER_SECURITY:
    case USER_TYPING:
    {
      const LicqUser* u = gUserManager.fetchUser(userId, LOCK_R);
      if (u == NULL)
        break;

      // update the tab icon of this user
      if (Config::Chat::instance()->tabbedChatting() && myUserEventTabDlg != NULL)
      {
        if (subSignal == USER_TYPING)
          myUserEventTabDlg->setTyping(u, argument);
        myUserEventTabDlg->updateTabLabel(u);
      }
      else if (subSignal == USER_TYPING)
      {
        // First, update the window if available
        for (int i = 0; i < myUserSendList.size(); ++i)
        {
          UserSendCommon* item = myUserSendList.at(i);

          if (item->ppid() == MSN_PPID)
          {
            // For protocols that use the convo id
            if (item->convoId() == (unsigned long)(argument) && item->ppid() == ppid)
              item->setTyping(u->GetTyping());
          }
          else
          {
            // For protocols that don't use a convo id
            if (item->userId() == userId)
              item->setTyping(u->GetTyping());
          }
        }
      }

      gUserManager.DropUser(u);

      break;
    }
  }
}

void LicqGui::updateUserData(const UserId& userId)
{
  myContactList->updateUser(userId);
}

void LicqGui::convoSet(const UserId& userId, unsigned long convoId)
{
  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->userId() == userId)
    {
      item->setConvoId(convoId);
      break;
    }
  }
}

void LicqGui::convoJoin(const UserId& userId, unsigned long ppid, unsigned long convoId)
{
  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->ppid() == ppid && item->convoId() == convoId)
    {
      item->convoJoin(userId);
      break;
    }
  }
}

void LicqGui::convoLeave(const UserId& userId, unsigned long ppid, unsigned long convoId)
{
  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->ppid() == ppid && item->convoId() == convoId &&
        item->isUserInConvo(userId))
    {
      item->convoLeave(userId);
      break;
    }
  }
}

struct SAutoAwayInfo
{
  SAutoAwayInfo() : isAutoAway(false) {}
  bool isAutoAway;

  unsigned short preAutoAwayStatus;
  unsigned short setAutoAwayStatus;
};

void LicqGui::autoAway()
{
#ifndef USE_SCRNSAVER
  myAutoAwayTimer.stop();
#else
  static XScreenSaverInfo* mit_info = NULL;

  if (mit_info == NULL)
  {
    int event_base, error_base;
    if (XScreenSaverQueryExtension(myMainWindow->x11Info().display(), &event_base, &error_base))
      mit_info = XScreenSaverAllocInfo ();
    else
    {
      gLog.Warn("%sNo XScreenSaver extension found on current XServer, disabling auto-away.\n",
                L_WARNxSTR);
      myAutoAwayTimer.stop();
      return;
    }
  }

  if (!XScreenSaverQueryInfo(myMainWindow->x11Info().display(), QX11Info::appRootWindow(), mit_info))
  {
    gLog.Warn("%sXScreenSaverQueryInfo failed, disabling auto-away.\n",
              L_WARNxSTR);
    myAutoAwayTimer.stop();
    return;
  }

  const unsigned long idleTime = mit_info->idle;
  static std::map<unsigned long, SAutoAwayInfo> autoAwayInfo;

  Config::General* generalConfig = Config::General::instance();

  // Go through each protocol, as the statuses may differ
  FOR_EACH_PROTO_PLUGIN_START(myLicqDaemon)
  {
    const unsigned long nPPID = (*_ppit)->PPID();

    // Fetch current status
    unsigned short status = ICQ_STATUS_OFFLINE;
    const ICQOwner* o = gUserManager.FetchOwner(nPPID, LOCK_R);
    if (o != NULL)
    {
      status = o->Status();
      gUserManager.DropOwner(o);
    }

    SAutoAwayInfo& info = autoAwayInfo[nPPID];

    // Check no one changed the status behind our back
    if (info.isAutoAway && info.setAutoAwayStatus != status)
    {
      gLog.Warn("%sSomeone changed the status behind our back (%u != %u; PPID: 0x%lx).\n",
                L_WARNxSTR, info.setAutoAwayStatus, status, nPPID);
      info.isAutoAway = false;
      continue;
    }

    // If we are offline, and it isn't auto offline, we shouldn't do anything
    if (status == ICQ_STATUS_OFFLINE && !info.isAutoAway)
      continue;

    bool returnFromAutoAway = false;
    unsigned short wantedStatus;
    if (generalConfig->autoOfflineTime() > 0 &&
        idleTime > (unsigned long)(generalConfig->autoOfflineTime() * 60000))
      wantedStatus = ICQ_STATUS_OFFLINE;
    else if (generalConfig->autoNaTime() > 0 &&
        idleTime > (unsigned long)(generalConfig->autoNaTime() * 60000))
      wantedStatus = ICQ_STATUS_NA;
    else if (generalConfig->autoAwayTime() > 0 &&
        idleTime > (unsigned long)(generalConfig->autoAwayTime() * 60000))
      wantedStatus = ICQ_STATUS_AWAY;
    else
    {
      // The user is active and we're not auto away
      if (!info.isAutoAway)
        continue;

      returnFromAutoAway = true;
      wantedStatus = info.preAutoAwayStatus;
    }

    // MSN does not support NA
    if (nPPID == MSN_PPID && wantedStatus == ICQ_STATUS_NA)
      wantedStatus = ICQ_STATUS_AWAY;

    // Never change from NA to away unless we are returning from auto away
    if (status == ICQ_STATUS_NA && wantedStatus == ICQ_STATUS_AWAY && !returnFromAutoAway)
      continue;

    if (status == wantedStatus)
      continue;

    // If we're not auto away, save current status
    if (!info.isAutoAway)
    {
      info.isAutoAway = true;
      info.preAutoAwayStatus = status;
    }
    else if (returnFromAutoAway)
      info.isAutoAway = false;

    // Set auto response
    if (wantedStatus == ICQ_STATUS_NA && generalConfig->autoNaMess())
    {
      SARList& sar = gSARManager.Fetch(SAR_NA);
      ICQOwner* o = gUserManager.FetchOwner(nPPID, LOCK_W);
      if (o != NULL)
      {
        o->SetAutoResponse(QString(sar[generalConfig->autoNaMess() - 1]->AutoResponse()).toLocal8Bit());
        gUserManager.DropOwner(o);
      }
      gSARManager.Drop();
    }
    else if (wantedStatus == ICQ_STATUS_AWAY && generalConfig->autoAwayMess())
    {
      SARList& sar = gSARManager.Fetch(SAR_AWAY);
      ICQOwner* o = gUserManager.FetchOwner(nPPID, LOCK_W);
      if (o != NULL)
      {
        o->SetAutoResponse(QString(sar[generalConfig->autoAwayMess() - 1]->AutoResponse()).toLocal8Bit());
        gUserManager.DropOwner(o);
      }
      gSARManager.Drop();
    }

    //gLog.Info("%sAuto-away changing status to %u (from %u, PPID 0x%lx).\n",
    //          L_SRVxSTR, wantedStatus, status, nPPID);

    // Change status
    info.setAutoAwayStatus = wantedStatus;
    changeStatus(wantedStatus, nPPID);
  }
  FOR_EACH_PROTO_PLUGIN_END
#endif // USE_SCRNSAVER
}

void LicqGui::updateDockIcon()
{
  // Delete old dock icon
  if (myDockIcon != NULL)
  {
    delete myDockIcon;
    myDockIcon = NULL;
  }

  switch (Config::General::instance()->dockMode())
  {
#ifndef USE_KDE
    case Config::General::DockDefault:
      myDockIcon = new DefaultDockIcon(myMainWindow->systemMenu());
      break;

    case Config::General::DockThemed:
      myDockIcon = new ThemedDockIcon(myMainWindow->systemMenu());
      break;
#endif

    case Config::General::DockTray:
      myDockIcon = new SystemTrayIcon(myMainWindow->systemMenu());
      break;

    case Config::General::DockNone:
    default:
      // Ensure that main window is visible when the dock gets disabled:
      myMainWindow->show();
      return;
  }

  connect(myDockIcon, SIGNAL(clicked()), myMainWindow, SLOT(trayIconClicked()));
  connect(myDockIcon, SIGNAL(middleClicked()), SLOT(showNextEvent()));
}

