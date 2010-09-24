// -*- c-basic-offset: 2; -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#include <boost/foreach.hpp>
#include <list>
#include <map>
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
#include <QTextCodec>
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

#include <licq/inifile.h>
#include <licq/logging/log.h>
#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/icqdefines.h>
#include <licq/pluginmanager.h>
#include <licq/pluginsignal.h>
#include <licq/protocolmanager.h>
#include <licq/sarmanager.h>
#include <licq/userevents.h>

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
#include "helpers/usercodec.h"

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
using Licq::SarManager;
using Licq::User;
using Licq::gLog;
using Licq::gSarManager;
using Licq::gPluginManager;
using Licq::gProtocolManager;

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


LicqGui* LicqQtGui::gLicqGui = NULL;

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
  assert(gLicqGui == NULL);
  gLicqGui = this;

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
  gLog.info("Attempting to load Qt4 and Licq " DISPLAY_PLUGIN_NAME " translations for %s locale",
      qPrintable(locale));

  QTranslator* qtTranslator = new QTranslator(this);
  qtTranslator->load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  installTranslator(qtTranslator);

  QTranslator* licqTranslator = new QTranslator(this);
  licqTranslator->load(locale, QString(Licq::gDaemon.shareDir().c_str()) + QTGUI_DIR + "locale");
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
  gLog.info("Loading %s configuration", PLUGIN_NAME);
  Licq::IniFile guiConf(QTGUI_CONFIGFILE);
  if (!guiConf.loadFile())
  {
    // No config file, try to load the old config file
    guiConf.setFilename("licq_qt-gui.conf");
    if (!guiConf.loadFile() ||
        !QueryYesNo(NULL,
          tr("There was an error loading the default configuration file.\n"
            "Would you like to try loading the old one?")))
    {
      // No old file either, or user didn't want to load it, revert to newer file
      guiConf.setFilename(QTGUI_CONFIGFILE);
    }
  }

  // Let configuration classes load their settings
  Config::General::instance()->loadConfiguration(guiConf);
  Config::Chat::instance()->loadConfiguration(guiConf);
  Config::ContactList::instance()->loadConfiguration(guiConf);
  Config::Shortcuts::instance()->loadConfiguration(guiConf);

  // Load icons
  guiConf.setSection("appearance");
  std::string s;

  if (myIcons.isEmpty())
  {
    guiConf.get("Icons", s, "ami");
    myIcons = s.c_str();
  }
  if (myExtendedIcons.isEmpty())
  {
    guiConf.get("ExtendedIcons", s, "basic");
    myExtendedIcons = s.c_str();
  }
  IconManager::createInstance(myIcons, myExtendedIcons, this);

  // Load Emoticons
  guiConf.get("Emoticons", s, Emoticons::DEFAULT_THEME.toLatin1().data());
  QStringList emoticonsDirs;
  emoticonsDirs += QString::fromLocal8Bit(Licq::gDaemon.shareDir().c_str()) + QTGUI_DIR + EMOTICONS_DIR;
  emoticonsDirs += QString::fromLocal8Bit(Licq::gDaemon.baseDir().c_str()) + QTGUI_DIR + EMOTICONS_DIR;
#ifdef USE_KDE
  // emoticons resource added in KDE 3.4
  emoticonsDirs += KGlobal::dirs()->findDirs("emoticons", "");
#endif
  Emoticons::self()->setBasedirs(emoticonsDirs);
  if (!Emoticons::self()->setTheme(Emoticons::translateThemeName(s.c_str())))
    gLog.error("Failed to load emoticons theme '%s'", s.c_str());

  // Load skin
  if (mySkin.isEmpty())
  {
    guiConf.get("Skin", s, "basic-ncb");
    mySkin = s.c_str();
  }
  bool skinFrameTransparent;
  guiConf.get("Transparent", skinFrameTransparent, false);
  unsigned skinFrameStyle;
  guiConf.get("FrameStyle", skinFrameStyle, 51);

  Config::Skin::createInstance(mySkin, this);
  Config::Skin::active()->setFrameStyle(skinFrameStyle);
  Config::Skin::active()->setFrameTransparent(skinFrameTransparent);
}

void LicqGui::loadFloatiesConfig()
{
  Licq::IniFile guiConf(QTGUI_CONFIGFILE);
  if (!guiConf.loadFile())
    return;

  std::string s;
  char key[16];
  int nFloaties = 0, xPosF, yPosF, wValF;
  guiConf.setSection("floaties");
  guiConf.get("Num", nFloaties, 0);
  for (int i = 0; i < nFloaties; i++)
  {
    sprintf(key, "Floaty%d.Ppid", i);
    unsigned long ppid;
    guiConf.get(key, ppid, LICQ_PPID);
    sprintf(key, "Floaty%d.Uin", i);
    guiConf.get(key, s, "");
    if (s.empty())
      continue;
    Licq::UserId userId(s, ppid);

    sprintf(key, "Floaty%d.X", i);
    guiConf.get(key, xPosF, 0);
    sprintf(key, "Floaty%d.Y", i);
    guiConf.get(key, yPosF, 0);
    sprintf(key, "Floaty%d.W", i);
    guiConf.get(key, wValF, 80);

    if (userId.isValid())
      createFloaty(userId, xPosF, yPosF, wValF);
  }
}

void LicqGui::saveConfig()
{
  // Tell the daemon to save its options
  Licq::gDaemon.SaveConf();

  // Save all our options
  Licq::IniFile guiConf(QTGUI_CONFIGFILE);
  guiConf.loadFile();

  Config::General::instance()->saveConfiguration(guiConf);
  Config::Chat::instance()->saveConfiguration(guiConf);
  Config::ContactList::instance()->saveConfiguration(guiConf);
  Config::Shortcuts::instance()->saveConfiguration(guiConf);

  guiConf.setSection("appearance");
  guiConf.set("Skin", Config::Skin::active()->skinName().toLocal8Bit());
  guiConf.set("Icons", IconManager::instance()->iconSet().toLocal8Bit());
  guiConf.set("ExtendedIcons", IconManager::instance()->extendedIconSet().toLocal8Bit());
  guiConf.set("Emoticons", Emoticons::untranslateThemeName(Emoticons::self()->theme()).toLatin1());

  guiConf.set("Transparent", Config::Skin::active()->frame.transparent);
  guiConf.set("FrameStyle", Config::Skin::active()->frame.frameStyle);

  char key[32];
  guiConf.setSection("floaties");
  guiConf.set("Num", FloatyView::floaties.size());
  for (int i = 0; i < FloatyView::floaties.size(); i++)
  {
    FloatyView* iter = FloatyView::floaties.at(i);
    sprintf(key, "Floaty%d.Ppid", i);
    guiConf.set(key, iter->userId().protocolId());
    sprintf(key, "Floaty%d.Uin", i);
    guiConf.set(key, iter->userId().accountId());
    sprintf(key, "Floaty%d.X", i);
    guiConf.set(key, (iter->x() > 0 ? iter->x() : 0));
    sprintf(key, "Floaty%d.Y", i);
    guiConf.set(key, (iter->y() > 0 ? iter->y() : 0));
    sprintf(key, "Floaty%d.W", i);
    guiConf.set(key, iter->width());
  }

  guiConf.writeFile();
}

int LicqGui::Run()
{
  // Register with the daemon, we want to receive all signals
  int pipe = gPluginManager.registerGeneralPlugin(Licq::PluginSignal::SignalAll);

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

  using Licq::Log;
  myLogWindow->pluginLogSink()->setLogLevel(Log::Unknown, true);
  myLogWindow->pluginLogSink()->setLogLevel(Log::Info, true);
  myLogWindow->pluginLogSink()->setLogLevel(Log::Warning, true);
  myLogWindow->pluginLogSink()->setLogLevel(Log::Error, true);

  // Check for qt-gui directory in current base dir
  if (!QDir(QString("%1%2").arg(Licq::gDaemon.baseDir().c_str()).arg(QTGUI_DIR)).exists())
  {
    QDir d;
    d.mkdir(QString("%1%2").arg(Licq::gDaemon.baseDir().c_str()).arg(QTGUI_DIR));
  }

  loadGuiConfig();

  // Contact list model
  myContactList = new ContactListModel(this);
  connect(mySignalManager, SIGNAL(updatedList(unsigned long, int, const Licq::UserId&)),
      myContactList, SLOT(listUpdated(unsigned long, int, const Licq::UserId&)));
  connect(mySignalManager, SIGNAL(updatedUser(const Licq::UserId&, unsigned long, int, unsigned long)),
      myContactList, SLOT(userUpdated(const Licq::UserId&, unsigned long, int)));

  connect(mySignalManager, SIGNAL(updatedList(unsigned long, int, const Licq::UserId&)),
      SLOT(listUpdated(unsigned long, int, const Licq::UserId&)));
  connect(mySignalManager, SIGNAL(updatedUser(const Licq::UserId&, unsigned long, int, unsigned long)),
      SLOT(userUpdated(const Licq::UserId&, unsigned long, int, unsigned long)));
  connect(mySignalManager, SIGNAL(socket(const Licq::UserId&, unsigned long)),
      SLOT(convoSet(const Licq::UserId&, unsigned long)));
  connect(mySignalManager, SIGNAL(convoJoin(const Licq::UserId&, unsigned long, unsigned long)),
      SLOT(convoJoin(const Licq::UserId&, unsigned long, unsigned long)));
  connect(mySignalManager, SIGNAL(convoLeave(const Licq::UserId&, unsigned long, unsigned long)),
      SLOT(convoLeave(const Licq::UserId&, unsigned long, unsigned long)));
  connect(mySignalManager, SIGNAL(ui_message(const Licq::UserId&)),
      SLOT(showMessageDialog(const Licq::UserId&)));
  connect(mySignalManager, SIGNAL(ui_viewevent(const Licq::UserId&)),
      SLOT(showNextEvent(const Licq::UserId&)));

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

  int r = exec();

  gPluginManager.unregisterGeneralPlugin();

  gLog.info("Shutting down gui");

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
    gLog.error("Unknown popup key: %s", key.toLatin1().data());
    return;
  }

  XGrabKey(dsp, XKeysymToKeycode(dsp, Support::keyToXSym(grabKeysym)),
      Support::keyToXMod(grabKeysym), rootWin, true,
      GrabModeAsync, GrabModeSync);
}
#endif /* defined(Q_WS_X11) */

void LicqGui::changeStatus(unsigned status, bool invisible, const QString& autoMessage)
{
  // Get a list of owners first since we can't call changeStatus with list locked
  list<Licq::UserId> owners;
  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* o, **ownerList)
      owners.push_back(o->id());
  }

  BOOST_FOREACH(const Licq::UserId& userId, owners)
    changeStatus(status, userId, invisible, autoMessage);
}

void LicqGui::changeStatus(unsigned status, const Licq::UserId& userId, bool invisible, const QString& autoMessage)
{
  unsigned oldStatus;

  {
    Licq::OwnerReadGuard o(userId);
    if (!o.isLocked())
      return;

    oldStatus = o->status();
  }

  if (status == User::InvisibleStatus)
  {
    // Don't try to toggle invisible if we're offline
    if (oldStatus == User::OfflineStatus)
      return;

    // Just set invisible status, keep the rest
    status = oldStatus;
    if (invisible)
      status |= User::InvisibleStatus;
    else
      status &= ~User::InvisibleStatus;
  }
  else if (status != User::OfflineStatus)
  {
    // Normal status change, keep flags
    status |= User::OnlineStatus;
    if (invisible || oldStatus & User::InvisibleStatus)
      status |= User::InvisibleStatus;
    if (oldStatus & User::IdleStatus)
      status |= User::IdleStatus;

    if (oldStatus == User::OfflineStatus)
    {
      // When going online, keep oldinvisible flag from status menu
      if (myMainWindow->systemMenu()->getInvisibleStatus(userId))
        status |= User::InvisibleStatus;
    }
  }

  const QTextCodec* codec = UserCodec::defaultEncoding();
  gProtocolManager.setStatus(userId, status,
      (autoMessage.isNull() ? gProtocolManager.KeepAutoResponse : codec->fromUnicode(autoMessage).data()));
}

bool LicqGui::removeUserFromList(const Licq::UserId& userId, QWidget* parent)
{
  if (parent == NULL)
    parent = myMainWindow;

  QString warning;
  bool notInList;
  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return true;
    notInList = u->NotInList();
    warning = tr("Are you sure you want to remove\n%1 (%2)\nfrom your contact list?")
        .arg(QString::fromUtf8(u->GetAlias()))
        .arg(u->accountId().c_str());
  }

  if (notInList || QueryYesNo(parent, warning))
  {
    Licq::gUserManager.removeUser(userId);
    return true;
  }
  return false;
}

void LicqGui::showInfoDialog(int /* fcn */, const Licq::UserId& userId, bool toggle, bool updateNow)
{
  if (!userId.isValid())
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

UserViewEvent* LicqGui::showViewEventDialog(const Licq::UserId& userId)
{
  if (!userId.isValid())
    return NULL;

  for (int i = 0; i < myUserViewList.size(); ++i)
  {
    UserViewEvent* e = myUserViewList.at(i);
    if (e->userId() == userId)
    {
      e->show();
      if (Config::Chat::instance()->autoFocus())
      {
        // Don't change focus if another message window is already active
        const QWidget* activeWin = QApplication::activeWindow();
        if (activeWin == NULL || ((qobject_cast<const UserEventCommon*>(activeWin)) == NULL &&
            (qobject_cast<const UserEventTabDlg*>(activeWin)) == NULL))
        {
          e->raise();
          e->activateWindow();
        }
      }
      return e;
    }
  }

  UserViewEvent* e = new UserViewEvent(userId);

  e->show();
  userEventFinished(userId);
  connect(e, SIGNAL(finished(const Licq::UserId&)), SLOT(userEventFinished(const Licq::UserId&)));
  myUserViewList.append(e);

  return e;
}

UserEventCommon* LicqGui::showEventDialog(int fcn, const Licq::UserId& userId, int convoId, bool autoPopup)
{
  if (!userId.isValid())
    return NULL;

  QString id = userId.accountId().c_str();
  unsigned long ppid = userId.protocolId();

  // Find out what's supported for this protocol
  unsigned long sendFuncs = 0;
  Licq::ProtocolPlugin::Ptr protocol = gPluginManager.getProtocolPlugin(ppid);
  if (protocol.get() != NULL)
    sendFuncs = protocol->getSendFunctions();

  // Check if the protocol for this contact support the function we want to open
  if ((fcn == MessageEvent && !(sendFuncs & Licq::ProtocolPlugin::CanSendMsg)) ||
      (fcn == UrlEvent && !(sendFuncs & Licq::ProtocolPlugin::CanSendUrl)) ||
      (fcn == ChatEvent && !(sendFuncs & Licq::ProtocolPlugin::CanSendChat)) ||
      (fcn == FileEvent && !(sendFuncs & Licq::ProtocolPlugin::CanSendFile )) ||
      (fcn == ContactEvent && !(sendFuncs & Licq::ProtocolPlugin::CanSendContact)) ||
      (fcn == SmsEvent && !(sendFuncs & Licq::ProtocolPlugin::CanSendSms)))
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

  QWidget* parent = NULL;
  if (Config::Chat::instance()->tabbedChatting())
  {
    if (myUserEventTabDlg == NULL)
    {
      // create the tab dialog if it does not exist
      myUserEventTabDlg = new UserEventTabDlg(0, "UserEventTabbedDialog");
      connect(myUserEventTabDlg, SIGNAL(signal_done()), SLOT(userEventTabDlgDone()));
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
      gLog.warning("Unknown callFunction() fcn: %d", fcn);
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
  connect(e, SIGNAL(eventSent(const Licq::Event*)), SIGNAL(eventSent(const Licq::Event*)));

  // there might be more than one send window open
  // make sure we only remember one, or it will get complicated
  sendEventFinished(userId);
  connect(e, SIGNAL(finished(const Licq::UserId&)), SLOT(sendEventFinished(const Licq::UserId&)));
  myUserSendList.append(static_cast<UserSendCommon*>(e));

  return e;
}

void LicqGui::replaceEventDialog(UserSendCommon* oldDialog, UserSendCommon* newDialog, const Licq::UserId& userId)
{
  disconnect(oldDialog, SIGNAL(finished(const Licq::UserId&)), this, SLOT(sendEventFinished(const Licq::UserId&)));
  sendEventFinished(userId);
  connect(newDialog, SIGNAL(eventSent(const Licq::Event*)), SIGNAL(eventSent(const Licq::Event*)));
  connect(newDialog, SIGNAL(finished(const Licq::UserId&)), SLOT(sendEventFinished(const Licq::UserId&)));
    myUserSendList.append(newDialog);
}

void LicqGui::showMessageDialog(const Licq::UserId& userId)
{
  showEventDialog(MessageEvent, userId);
}

void LicqGui::sendMsg(const Licq::UserId& userId, const QString& message)
{
  UserSendCommon* event = dynamic_cast<UserSendCommon*>(showEventDialog(MessageEvent, userId));
  if (event == 0)
    return;

  event->setText(message);
}

void LicqGui::sendFileTransfer(const Licq::UserId& userId, const QString& filename, const QString& description)
{
  UserSendFileEvent* event = dynamic_cast<UserSendFileEvent*>(showEventDialog(FileEvent, userId));
  if (event == 0)
    return;

  event->setFile(filename, description);
}

void LicqGui::sendChatRequest(const Licq::UserId& userId)
{
  UserSendCommon* event = dynamic_cast<UserSendCommon*>(showEventDialog(ChatEvent, userId));
  if (event == 0)
    return;
}

bool LicqGui::userDropEvent(const Licq::UserId& userId, const QMimeData& mimeData)
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

    {
      Licq::OwnerListGuard ownerList;
      BOOST_FOREACH(Licq::Owner* owner, **ownerList)
      {
        unsigned long ppid = owner->ppid();
        char ppidStr[5];
        Licq::protocolId_toStr(ppidStr, ppid);
        if (text.startsWith(ppidStr))
        {
          dropPpid = ppid;
          break;
        }
      }
    }

    if (dropPpid != 0 && text.length() > 4)
    {
      QString dropId = text.mid(4);
      Licq::UserId dropUserId(dropId.toLatin1().data(), dropPpid);
      if (!dropUserId.isValid() || userId == dropUserId)
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
  QDesktopServices::openUrl(QUrl(url));
#endif
}

void LicqGui::userDlgFinished(UserDlg* dialog)
{
  if (myUserDlgList.removeAll(dialog) > 0)
    return;

  gLog.warning("User Info finished signal for user with no window (%s)",
      dialog->userId().toString().c_str());
}

void LicqGui::userEventTabDlgDone()
{
  myUserEventTabDlg = NULL;
}

void LicqGui::userEventFinished(const Licq::UserId& userId)
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

void LicqGui::sendEventFinished(const Licq::UserId& userId)
{
  // go through the whole list, since there might be more than one hit
  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->userId() == userId)
      myUserSendList.removeAll(item);
  }
}

void LicqGui::showDefaultEventDialog(const Licq::UserId& userId)
{
  if (!userId.isValid())
    return;

  QString id = userId.accountId().c_str();
  unsigned long ppid = userId.protocolId();

  bool send;
  int convoId = -1;
  {
    Licq::UserReadGuard u(userId);
    if (!u.isLocked())
      return;

    // set default function to read or send depending on whether or not
    // there are new messages
    send = (u->NewMessages() == 0);
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
  }

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
    unsigned long sendFuncs = 0;
    Licq::ProtocolPlugin::Ptr protocol = gPluginManager.getProtocolPlugin(ppid);
    if (protocol.get() != NULL)
      sendFuncs = protocol->getSendFunctions();

    if (sendFuncs & Licq::ProtocolPlugin::CanSendUrl &&
        (c.left(5) == "http:" || c.left(4) == "ftp:" || c.left(6) == "https:"))
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
    else if (sendFuncs & Licq::ProtocolPlugin::CanSendFile &&
        (c.left(5) == "file:" || c.left(1) == "/"))
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

bool LicqGui::showAllOwnerEvents()
{
  bool foundEvents = false;

  // Get a list of owners first so we can unlock list before calling showViewEventDialog
  list<Licq::UserId> users;

  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(Licq::Owner* owner, **ownerList)
    {
      Licq::OwnerReadGuard o(owner);
      if (o->NewMessages() > 0)
      {
        users.push_back(o->id());
        foundEvents = true;
      }
    }
  }

  BOOST_FOREACH(Licq::UserId& userId, users)
    showViewEventDialog(userId);

  return foundEvents;
}

void LicqGui::showNextEvent(const Licq::UserId& uid)
{
  // Do nothing if there are no events pending
  if (Licq::User::getNumUserEvents() == 0)
    return;

  Licq::UserId userId = uid;

  if (!userId.isValid())
  {
    // Do system messages first
    if (showAllOwnerEvents())
      return;

    time_t t = time(NULL);
    Licq::UserListGuard userList;
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard pUser(user);
      if (pUser->NewMessages() > 0 && pUser->Touched() <= t)
      {
        userId = pUser->id();
        t = pUser->Touched();
      }
    }
  }

  if (userId.isValid())
  {
    if (Config::Chat::instance()->msgChatView())
    {
      Licq::UserReadGuard u(userId);
      if (!u.isLocked())
        return;

      for (unsigned short i = 0; i < u->NewMessages(); i++)
      {
        if (u->EventPeek(i)->SubCommand() == ICQ_CMDxSUB_MSG ||
            u->EventPeek(i)->SubCommand() == ICQ_CMDxSUB_URL)
        {
          int convoId = u->EventPeek(i)->ConvoId();
          u.unlock();
          showEventDialog(MessageEvent, userId, convoId);
          return;
        }
      }
    }

    showViewEventDialog(userId);
  }
}

void LicqGui::showAllEvents()
{
  // Do nothing if there are no events pending
  if (Licq::User::getNumUserEvents() == 0)
    return;

  // Do system messages first
  showAllOwnerEvents();

  list<Licq::UserId> users;
  {
    Licq::UserListGuard userList;
    BOOST_FOREACH(const Licq::User* user, **userList)
    {
      Licq::UserReadGuard pUser(user);
      if (pUser->NewMessages() > 0)
        users.push_back(pUser->id());
    }
  }

  list<Licq::UserId>::iterator iter;
  for (iter = users.begin(); iter != users.end(); iter++)
    showDefaultEventDialog(*iter);
}

void LicqGui::toggleFloaty(const Licq::UserId& userId)
{
  FloatyView* v = FloatyView::findFloaty(userId);
  if (v == NULL)
    createFloaty(userId);
  else
    delete v;
}

void LicqGui::createFloaty(const Licq::UserId& userId,
   int x, int y, int w)
{
  if (!userId.isValid() || !Licq::gUserManager.userExists(userId))
    return;

  FloatyView* f = new FloatyView(myContactList, userId);

  connect(f, SIGNAL(userDoubleClicked(const Licq::UserId&)),
      SLOT(showDefaultEventDialog(const Licq::UserId&)));

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

void LicqGui::listUpdated(unsigned long subSignal, int /* argument */, const Licq::UserId& userId)
{
  switch (subSignal)
  {
    case Licq::PluginSignal::ListUserRemoved:
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

    case Licq::PluginSignal::ListInvalidate:
    case Licq::PluginSignal::ListGroupAdded:
    case Licq::PluginSignal::ListGroupRemoved:
    case Licq::PluginSignal::ListGroupChanged:
      myMainWindow->updateGroups();
      myUserMenu->updateGroups();
      myGroupMenu->updateGroups();
      break;
  }
}

void LicqGui::userUpdated(const Licq::UserId& userId, unsigned long subSignal, int argument, unsigned long cid)
{
  QString id = userId.accountId().c_str();
  unsigned long ppid = userId.protocolId();

  switch (subSignal)
  {
    case Licq::PluginSignal::UserEvents:
    {
      // Skip all this if it was just an away message check
      if (argument == 0)
        break;

      if (argument > 0)
      {
        unsigned short popCheck = 99;

        {
          Licq::OwnerReadGuard o(ppid);
          if (o.isLocked())
          {
            unsigned status = o->status();
            if (status & User::DoNotDisturbStatus)
              popCheck = 5;
            else if (status & User::OccupiedStatus)
              popCheck = 4;
            else if (status & User::NotAvailableStatus)
              popCheck = 3;
            else if (status & User::AwayStatus)
              popCheck = 2;
            else if (status & User::OnlineStatus)
              popCheck = 1;
          }
        }

        if (Config::Chat::instance()->autoPopup() >= popCheck)
        {
          bool bCallUserView = false, bCallSendMsg = false;
          {
            Licq::UserReadGuard u(userId);
            if (u.isLocked() && u->NewMessages() > 0)
            {
              bool urgent = false;
              for (unsigned short i = 0; i < u->NewMessages(); i++)
              {
                const Licq::UserEvent* event = u->EventPeek(i);

                if (event->IsUrgent())
                  urgent = true;

                switch (event->SubCommand())
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

              if (!urgent && Config::Chat::instance()->autoPopupUrgentOnly())
              {
                bCallSendMsg = false;
                bCallUserView = false;
              }

              if (bCallSendMsg && !Config::Chat::instance()->msgChatView())
                bCallUserView = true;
            }
          }

          if (bCallSendMsg)
            showEventDialog(MessageEvent, userId, cid, true);
          if (bCallUserView)
            showViewEventDialog(userId);
        }
      }
      // Fall through
    }
    case Licq::PluginSignal::UserStatus:
    case Licq::PluginSignal::UserBasic:
    case Licq::PluginSignal::UserSecurity:
    case Licq::PluginSignal::UserTyping:
    {
      Licq::UserReadGuard u(userId);
      if (!u.isLocked())
        break;

      // update the tab icon of this user
      if (Config::Chat::instance()->tabbedChatting() && myUserEventTabDlg != NULL)
      {
        if (subSignal == Licq::PluginSignal::UserTyping)
          myUserEventTabDlg->setTyping(*u, argument);
        myUserEventTabDlg->updateTabLabel(*u);
      }
      else if (subSignal == Licq::PluginSignal::UserTyping)
      {
        // First, update the window if available
        for (int i = 0; i < myUserSendList.size(); ++i)
        {
          UserSendCommon* item = myUserSendList.at(i);

          if (item->ppid() == MSN_PPID)
          {
            // For protocols that use the convo id
            if (item->convoId() == (unsigned long)(argument) && item->ppid() == ppid)
              item->setTyping(u->isTyping());
          }
          else
          {
            // For protocols that don't use a convo id
            if (item->userId() == userId)
              item->setTyping(u->isTyping());
          }
        }
      }

      break;
    }
  }
}

void LicqGui::convoSet(const Licq::UserId& userId, unsigned long convoId)
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

void LicqGui::convoJoin(const Licq::UserId& userId, unsigned long ppid, unsigned long convoId)
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

void LicqGui::convoLeave(const Licq::UserId& userId, unsigned long ppid, unsigned long convoId)
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

  unsigned preAutoAwayStatus;
  unsigned setAutoAwayStatus;
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
      gLog.warning("No XScreenSaver extension found on current XServer, disabling auto-away");
      myAutoAwayTimer.stop();
      return;
    }
  }

  if (!XScreenSaverQueryInfo(myMainWindow->x11Info().display(), QX11Info::appRootWindow(), mit_info))
  {
    gLog.warning("XScreenSaverQueryInfo failed, disabling auto-away");
    myAutoAwayTimer.stop();
    return;
  }

  const unsigned long idleTime = mit_info->idle;
  static std::map<Licq::UserId, SAutoAwayInfo> autoAwayInfo;

  Config::General* generalConfig = Config::General::instance();

  // Check every owner as the statuses may differ
  map<Licq::UserId, unsigned> newStatuses;

  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(Licq::Owner* owner, **ownerList)
    {
      // Fetch current status
      Licq::UserId userId;
      unsigned status;
      {
        Licq::OwnerReadGuard o(owner);
        userId = o->id();
        status = o->status();
      }

      SAutoAwayInfo& info = autoAwayInfo[userId];

      // Check no one changed the status behind our back
      if (info.isAutoAway && info.setAutoAwayStatus != status)
      {
        gLog.warning("Someone changed the status behind our back (%4x != %4x; account: %s)",
            info.setAutoAwayStatus, status, userId.toString().c_str());
        info.isAutoAway = false;
        continue;
      }

      // If we are offline, and it isn't auto offline, we shouldn't do anything
      if (status == User::OfflineStatus && !info.isAutoAway)
        continue;

      bool returnFromAutoAway = false;
      unsigned wantedStatus;
      if (generalConfig->autoOfflineTime() > 0 &&
          idleTime > (unsigned long)(generalConfig->autoOfflineTime() * 60000))
        wantedStatus = User::OfflineStatus;
      else if (generalConfig->autoNaTime() > 0 &&
          idleTime > (unsigned long)(generalConfig->autoNaTime() * 60000))
        wantedStatus = User::OnlineStatus | User::NotAvailableStatus;
      else if (generalConfig->autoAwayTime() > 0 &&
          idleTime > (unsigned long)(generalConfig->autoAwayTime() * 60000))
        wantedStatus = User::OnlineStatus | User::AwayStatus;
      else
      {
        // The user is active and we're not auto away
        if (!info.isAutoAway)
          continue;

        returnFromAutoAway = true;
        wantedStatus = info.preAutoAwayStatus;
      }

      // MSN does not support NA
      if (userId.protocolId() == MSN_PPID && wantedStatus & User::NotAvailableStatus)
        wantedStatus = User::OnlineStatus | User::AwayStatus;

      // Never change from NA to away unless we are returning from auto away
      if (status & User::NotAvailableStatus && wantedStatus & User::AwayStatus && !returnFromAutoAway)
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
      QString autoResponse;
      if (wantedStatus & User::NotAvailableStatus && generalConfig->autoNaMess())
      {
        const Licq::SarList& sars(gSarManager.getList(SarManager::NotAvailableList));
        autoResponse = QString::fromLocal8Bit(sars.begin()->text.c_str());
        gSarManager.releaseList();
      }
      else if (wantedStatus & User::AwayStatus && generalConfig->autoAwayMess())
      {
        const Licq::SarList& sars(gSarManager.getList(SarManager::AwayList));
        autoResponse = QString::fromLocal8Bit(sars.begin()->text.c_str());
        gSarManager.releaseList();
      }
      if (!autoResponse.isNull())
      {
        Licq::OwnerWriteGuard o(owner);
        o->setAutoResponse(autoResponse.toLocal8Bit().data());
      }

      //gLog.info("Auto-away changing status to %u (from %u, PPID 0x%lx)",
      //    wantedStatus, status, userId.protocolId());

      // Change status
      info.setAutoAwayStatus = wantedStatus;
      newStatuses[userId] = wantedStatus;
    }
  }

  // Do the actual status change here, after we've released the lock on owner list
  map<Licq::UserId, unsigned>::const_iterator iter;
  for (iter = newStatuses.begin(); iter != newStatuses.end(); ++iter)
    changeStatus(iter->second, iter->first);

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

void LicqGui::setUserInGroup(const Licq::UserId& userId, int groupId, bool inGroup, bool updateServer)
{
  // Normal user group
  if (groupId < ContactListModel::SystemGroupOffset)
    return Licq::gUserManager.setUserInGroup(userId, groupId, inGroup, updateServer);

  // Groups that require server update
  if (groupId == ContactListModel::VisibleListGroupId)
    return gProtocolManager.visibleListSet(userId, inGroup);
  if (groupId == ContactListModel::InvisibleListGroupId)
    return gProtocolManager.invisibleListSet(userId, inGroup);
  if (groupId == ContactListModel::IgnoreListGroupId)
    return gProtocolManager.ignoreListSet(userId, inGroup);

  // If we got here, the "group" is just a flag for local user

  {
    Licq::UserWriteGuard u(userId);
    if (!u.isLocked())
      return;

    switch (groupId)
    {
      case ContactListModel::OnlineNotifyGroupId:
        u->SetOnlineNotify(inGroup);
        break;
      case ContactListModel::NewUsersGroupId:
        u->SetNewUser(inGroup);
        break;
      default:
        // Invalid group
        return;
    }
  }

  // Notify everyone of the change
  Licq::gUserManager.notifyUserUpdated(userId, Licq::PluginSignal::UserSettings);
}
