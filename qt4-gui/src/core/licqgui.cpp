// -*- c-basic-offset: 2; -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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
#include <QSessionManager>
#include <QTranslator>

#ifdef USE_KDE
#include <KDE/KStandardDirs>
#include <KDE/KToolInvocation>
#include <KDE/KUrl>
#else
# include <QDesktopServices>
# include <QStyle>
# include <QStyleFactory>
# include <QUrl>
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

#ifndef USE_KDE
  char styleName[32] = "";
#endif
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

#ifndef USE_KDE
      case 'g':   // gui style
        strncpy(styleName, optarg, sizeof(styleName));
        styleName[sizeof(styleName) - 1] = '\0';
        break;
#endif

      case 'd':   // dock icon
        if (!myDisableDockIcon)
          myStartHidden = true;
        break;

      case 'D': // disable dock icon
        myStartHidden = false; // discard any -d
        myDisableDockIcon = true;
    }
  }

#ifndef USE_KDE
  char* ptr;
  char buf[MAX_FILENAME_LEN];
  snprintf(buf, MAX_FILENAME_LEN, "%s%sstyle.ini", BASE_DIR, QTGUI_DIR);
  buf[MAX_FILENAME_LEN - 1] = '\0';

  if (strcmp(styleName, "default") == 0)
  {
    unlink(buf);
    styleName[0]='\0';
  }

  QStyle* newStyle = createStyle(styleName);

  // Write out the style if not NULL
  if (newStyle != NULL)
  {
    FILE* f = fopen(buf, "w");
    if (f != NULL)
    {
      fprintf(f, "%s\n", styleName);
      fclose(f);
    }
  }
  // Otherwise try and load it from the file
  else
  {
    FILE* f = fopen(buf, "r");
    if (f != NULL)
    {
      if (fgets(styleName, 32, f) != NULL)
      {
        ptr = strrchr(styleName, '\n');
        if (ptr != NULL)
          *ptr = '\0';
        newStyle = createStyle(styleName);
      }
      fclose(f);
    }
  }

  if (newStyle != NULL)
    setStyle(newStyle);
#endif

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
  if (!licqConf.LoadFile(szTemp))
  {
    // File doesn't exist so define sections and write them now
    // so saving won't generate warnings later
    licqConf.SetFlags(INI_FxALLOWxCREATE);
    licqConf.ReloadFile();
    licqConf.CreateSection("appearance");
    licqConf.CreateSection("functions");
    licqConf.CreateSection("startup");
    licqConf.CreateSection("locale");
    licqConf.CreateSection("floaties");
    licqConf.CreateSection("geometry");
    licqConf.FlushFile();

    // Don't allow anymore writes
    licqConf.SetFlags(0);

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
  unsigned long ppid;
  licqConf.SetSection("floaties");
  licqConf.ReadNum("Num", nFloaties, 0);
  for (unsigned short i = 0; i < nFloaties; i++)
  {
    sprintf(key, "Floaty%d.Ppid", i);
    licqConf.ReadNum(key, ppid, LICQ_PPID);
    sprintf(key, "Floaty%d.Uin", i);
    licqConf.ReadStr(key, szTemp, "");
    sprintf(key, "Floaty%d.X", i);
    licqConf.ReadNum(key, xPosF, 0);
    sprintf(key, "Floaty%d.Y", i);
    licqConf.ReadNum(key, yPosF, 0);
    sprintf(key, "Floaty%d.W", i);
    licqConf.ReadNum(key, wValF, 80);

    if (szTemp[0] != 0)
      createFloaty(szTemp, ppid, xPosF, yPosF, wValF);
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
    licqConf.WriteNum(key, iter->ppid());
    sprintf(key, "Floaty%d.Uin", i);
    licqConf.WriteStr(key, iter->id().toLocal8Bit());
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
  connect(mySignalManager, SIGNAL(updatedList(CICQSignal*)),
      myContactList, SLOT(listUpdated(CICQSignal*)));
  connect(mySignalManager, SIGNAL(updatedUser(CICQSignal*)),
      myContactList, SLOT(userUpdated(CICQSignal*)));

  connect(mySignalManager, SIGNAL(updatedList(CICQSignal*)),
      SLOT(listUpdated(CICQSignal*)));
  connect(mySignalManager, SIGNAL(updatedUser(CICQSignal*)),
      SLOT(userUpdated(CICQSignal*)));
  connect(mySignalManager, SIGNAL(socket(QString, unsigned long, unsigned long)),
      SLOT(convoSet(QString, unsigned long, unsigned long)));
  connect(mySignalManager, SIGNAL(convoJoin(QString, unsigned long, unsigned long)),
      SLOT(convoJoin(QString, unsigned long, unsigned long)));
  connect(mySignalManager, SIGNAL(convoLeave(QString, unsigned long, unsigned long)),
      SLOT(convoLeave(QString, unsigned long, unsigned long)));
  connect(mySignalManager, SIGNAL(ui_message(QString, unsigned long)),
      SLOT(showMessageDialog(QString, unsigned long)));
  connect(mySignalManager, SIGNAL(ui_viewevent(QString)),
      SLOT(showNextEvent(QString)));
  connect(mySignalManager, SIGNAL(eventTag(QString, unsigned long, unsigned long)),
      SLOT(addEventTag(QString, unsigned long, unsigned long)));

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

void LicqGui::grabKey(QString key)
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

#ifndef USE_KDE
QStyle* LicqGui::createStyle(const char* name) const
{
  QStyle* s = NULL;

  if (name != NULL && name[0] != '\0' &&
      QStyleFactory::keys().contains(name, Qt::CaseInsensitive))
    s = QStyleFactory::create(name);

  return s;
}
#endif

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
  if (status == ICQ_STATUS_OFFLINE)
  {
    myLicqDaemon->ProtoLogoff(ppid);
    return;
  }

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
  else
  {
    if (o->StatusInvisible() || invisible)
      status |= ICQ_STATUS_FxPRIVATE;
  }

  bool b = o->StatusOffline();
  gUserManager.DropOwner(o);
  if (b)
    myLicqDaemon->ProtoLogon(ppid, status);
  else
    myLicqDaemon->ProtoSetStatus(ppid, status);
}

bool LicqGui::removeUserFromList(QString id, unsigned long ppid, QWidget* parent)
{
  if (parent == NULL)
    parent = myMainWindow;

  const ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);
  if (u == NULL)
    return true;
  QString warning(tr("Are you sure you want to remove\n%1 (%2)\nfrom your contact list?")
      .arg(QString::fromUtf8(u->GetAlias()))
      .arg(u->IdString()));
  gUserManager.DropUser(u);
  if (QueryYesNo(parent, warning))
  {
    myLicqDaemon->RemoveUserFromList(id.toLatin1(), ppid);
    return true;
  }
  return false;
}

void LicqGui::showInfoDialog(int /* fcn */, QString id, unsigned long ppid,
  bool toggle, bool updateNow)
{
  if (id.isEmpty() || ppid == 0) return;

  UserDlg* f = NULL;

  for (int i = 0; i < myUserDlgList.size(); ++i)
  {
    UserDlg* item = myUserDlgList.at(i);
    if (item->id() == id && item->ppid() == ppid)
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
    f = new UserDlg(id, ppid);
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

UserViewEvent* LicqGui::showViewEventDialog(QString id, unsigned long ppid)
{
  if (id.isEmpty() || ppid == 0)
    return NULL;

  for (int i = 0; i < myUserViewList.size(); ++i)
  {
    UserViewEvent* e = myUserViewList.at(i);
    if (id.compare(e->id(), Qt::CaseInsensitive) == 0 && e->ppid() == ppid)
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

  UserViewEvent* e = new UserViewEvent(id, ppid);

  e->show();
  userEventFinished(id, ppid);
  connect(e, SIGNAL(finished(QString, unsigned long)), SLOT(userEventFinished(QString, unsigned long)));
  myUserViewList.append(e);

  return e;
}

UserEventCommon* LicqGui::showEventDialog(int fcn, QString id, unsigned long ppid, int convoId, bool autoPopup)
{
  if (id.isEmpty() || ppid == 0)
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
      if ( ((ppid == MSN_PPID && e->ppid() == MSN_PPID) && (e->isUserInConvo(id) ||
              (e->convoId() == (unsigned long)(convoId) && e->convoId() != (unsigned long)(-1)))) ||
          (e->isUserInConvo(id) && e->ppid() == ppid))
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
        e->changeEventType(fcn);
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
      e = new UserSendMsgEvent(id, ppid, parent);
      break;

    case UrlEvent:
      e = new UserSendUrlEvent(id, ppid, parent);
      break;

    case ChatEvent:
      e = new UserSendChatEvent(id, ppid, parent);
      break;

    case FileEvent:
      e = new UserSendFileEvent(id, ppid, parent);
      break;

    case ContactEvent:
      e = new UserSendContactEvent(id, ppid, parent);
      break;

    case SmsEvent:
      e = new UserSendSmsEvent(id, ppid, parent);
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
  connect(e, SIGNAL(eventSent(const ICQEvent*)), SIGNAL(eventSent(const ICQEvent*)));

  // there might be more than one send window open
  // make sure we only remember one, or it will get complicated
  sendEventFinished(id, ppid);
  connect(e, SIGNAL(finished(QString, unsigned long)), SLOT(sendEventFinished(QString, unsigned long)));
  myUserSendList.append(static_cast<UserSendCommon*>(e));

  return e;
}

void LicqGui::replaceEventDialog(UserSendCommon* oldDialog, UserSendCommon* newDialog, QString id, unsigned long ppid)
{
    disconnect(oldDialog, SIGNAL(finished(QString, unsigned long)), this, SLOT(sendEventFinished(QString, unsigned long)));
    sendEventFinished(id, ppid);
  connect(newDialog, SIGNAL(eventSent(const ICQEvent*)), SIGNAL(eventSent(const ICQEvent*)));
    connect(newDialog, SIGNAL(finished(QString, unsigned long)), SLOT(sendEventFinished(QString, unsigned long)));
    myUserSendList.append(newDialog);
}

void LicqGui::showMessageDialog(QString id, unsigned long ppid)
{
  showEventDialog(MessageEvent, id, ppid);
}

void LicqGui::sendMsg(QString id, unsigned long ppid, const QString& message)
{
  UserSendCommon* event = dynamic_cast<UserSendCommon*>(showEventDialog(MessageEvent, id, ppid));
  if (event == 0)
    return;

  event->setText(message);
}

void LicqGui::sendFileTransfer(QString id, unsigned long ppid, const QString& filename, const QString& description)
{
  UserSendFileEvent* event = dynamic_cast<UserSendFileEvent*>(showEventDialog(FileEvent, id, ppid));
  if (event == 0)
    return;

  event->setFile(filename, description);
}

void LicqGui::sendChatRequest(QString id, unsigned long ppid)
{
  UserSendCommon* event = dynamic_cast<UserSendCommon*>(showEventDialog(ChatEvent, id, ppid));
  if (event == 0)
    return;
}

void LicqGui::viewUrl(QString url)
{
#ifdef USE_KDE
  if (url.startsWith("mailto:"))
    KToolInvocation::invokeMailer(KUrl(url));
  else
    KToolInvocation::invokeBrowser(url);
#else
  if (!QDesktopServices::openUrl(QUrl(url)))
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
      L_WARNxSTR, dialog->id().toLatin1().data());
}

void LicqGui::userEventTabDlgDone()
{
  myUserEventTabDlg = NULL;
}

void LicqGui::userEventFinished(QString id, unsigned long ppid)
{
  for (int i = 0; i < myUserViewList.size(); ++i)
  {
    UserViewEvent* item = myUserViewList.at(i);
    if (item->ppid() == ppid && item->id() == id)
    {
      myUserViewList.removeAll(item);
      return;
    }
  }
}

void LicqGui::sendEventFinished(QString id, unsigned long ppid)
{
  // go through the whole list, since there might be more than one hit
  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->ppid() == ppid && item->id() == id)
      myUserSendList.removeAll(item);
  }
}

void LicqGui::showDefaultEventDialog(QString id, unsigned long ppid)
{
  if (id.isEmpty() || ppid == 0)
    return;

  const ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);

  if (u == NULL)
    return;

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
    showViewEventDialog(id, ppid);
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
      UserEventCommon* ec = showEventDialog(UrlEvent, id, ppid);
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
      UserEventCommon* ec = showEventDialog(FileEvent, id, ppid);
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

  showEventDialog(MessageEvent, id, ppid, convoId);
}

void LicqGui::showAllOwnerEvents()
{
  FOR_EACH_PROTO_PLUGIN_START(myLicqDaemon)
  {
    const ICQOwner* o = gUserManager.FetchOwner((*_ppit)->PPID(), LOCK_R);
    if (o == NULL)
      continue;
    QString id = o->IdString();
    unsigned short nNumMsg = o->NewMessages();
    gUserManager.DropOwner(o);

    if (nNumMsg > 0)
      showViewEventDialog(id, (*_ppit)->PPID());
  }
  FOR_EACH_PROTO_PLUGIN_END
}

void LicqGui::showNextEvent(QString id)
{
  // Do nothing if there are no events pending
  if (ICQUser::getNumUserEvents() == 0 || id.isEmpty())
    return;

  unsigned long ppid = 0;

  if (id == "0")
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
        id = pUser->IdString();
        ppid = pUser->PPID();
        t = pUser->Touched();
      }
    }
    FOR_EACH_USER_END
  }

  if (!id.isEmpty())
  {
    if (Config::Chat::instance()->msgChatView())
    {
      const ICQUser* u = NULL;
      if (ppid == 0)
      {
        FOR_EACH_PROTO_PLUGIN_START(myLicqDaemon)
        {
          u = gUserManager.FetchUser(id.toLatin1(), (*_ppit)->PPID(), LOCK_R);
          if (u == NULL)
            continue;

          if (u->NewMessages())
          {
            ppid = (*_ppit)->PPID();
            break;
          }
          else
          {
            gUserManager.DropUser(u);
            u = NULL;
          }
        }
        FOR_EACH_PROTO_PLUGIN_END
      }
      else
        u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);

      if (u == NULL)
        return;

      for (unsigned short i = 0; i < u->NewMessages(); i++)
      {
        if (u->EventPeek(i)->SubCommand() == ICQ_CMDxSUB_MSG ||
            u->EventPeek(i)->SubCommand() == ICQ_CMDxSUB_URL)
        {
          gUserManager.DropUser(u);
          showEventDialog(MessageEvent, id, ppid, u->EventPeek(i)->ConvoId());
          return;
        }
      }
      gUserManager.DropUser(u);
    }

    showViewEventDialog(id, ppid);
  }
}

void LicqGui::showAllEvents()
{
  // Do nothing if there are no events pending
  if (ICQUser::getNumUserEvents() == 0)
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

  list<pair<QString, unsigned long> > users;
  FOR_EACH_USER_START(LOCK_R)
  {
    if (pUser->NewMessages() > 0)
      users.push_back(pair<QString, unsigned long>(pUser->IdString(), pUser->PPID()));
  }
  FOR_EACH_USER_END

  list<pair<QString, unsigned long> >::iterator iter;
  for (iter = users.begin(); iter != users.end(); iter++)
    showDefaultEventDialog(iter->first, iter->second);
}

void LicqGui::toggleFloaty(QString id, unsigned long ppid)
{
  FloatyView* v = FloatyView::findFloaty(id, ppid);
  if (v == NULL)
    createFloaty(id, ppid);
  else
    delete v;
}

void LicqGui::createFloaty(QString id, unsigned long ppid,
   unsigned short x, unsigned short y, unsigned short w)
{
  if (id.isEmpty() || ppid == 0)
    return;
  const ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);
  if (u == NULL)
    return;

  FloatyView* f = new FloatyView(myContactList, u);

  connect(f, SIGNAL(userDoubleClicked(QString, unsigned long)),
      SLOT(showDefaultEventDialog(QString, unsigned long)));

  gUserManager.DropUser(u);

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

void LicqGui::listUpdated(CICQSignal* sig)
{
  switch (sig->SubSignal())
  {
    case LIST_REMOVE:
    {
      // If their floaty is enabled, remove it
      FloatyView* f = FloatyView::findFloaty(sig->Id(), sig->PPID());
      if (f)
        delete f;

      // If their view box is open, kill it
      for (int i = 0; i < myUserViewList.size(); ++i)
      {
        UserViewEvent* item = myUserViewList.at(i);
        if (item->id() == sig->Id() && item->ppid() == sig->PPID())
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
        if (item->id() == sig->Id() && item->ppid() == sig->PPID())
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
        if (item->id() == sig->Id() && item->ppid() == sig->PPID())
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

void LicqGui::userUpdated(CICQSignal* sig)
{
  QString id = sig->Id();
  unsigned long ppid = sig->PPID();

  const ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);
  if (u == NULL)
  {
    char* ppidString = PPIDSTRING(ppid);
    gLog.Warn("%sLicqGui::userUpdated(): Invalid user received: %s (%s)\n",
      L_ERRORxSTR, id.toLatin1().data(), ppidString);
    delete[] ppidString;
    return;
  }
  else
    gUserManager.DropUser(u);

  switch (sig->SubSignal())
  {
    case USER_EVENTS:
    {
      // Skip all this if it was just an away message check
      if (sig->Argument() == 0)
        break;

      if (sig->Argument() > 0)
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
          const ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);
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
              showEventDialog(MessageEvent, id, ppid, sig->CID(), true);
            if (bCallUserView)
              showViewEventDialog(id, ppid);
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
      const ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);
      if (u == NULL)
        break;

      // update the tab icon of this user
      if (Config::Chat::instance()->tabbedChatting() && myUserEventTabDlg != NULL)
      {
        if (sig->SubSignal() == USER_TYPING)
          myUserEventTabDlg->setTyping(u, sig->Argument());
        myUserEventTabDlg->updateTabLabel(u);
      }
      else if (sig->SubSignal() == USER_TYPING)
      {
        // First, update the window if available
        for (int i = 0; i < myUserSendList.size(); ++i)
        {
          UserSendCommon* item = myUserSendList.at(i);

          if (item->ppid() == MSN_PPID)
          {
            // For protocols that use the convo id
            if (item->convoId() == (unsigned long)(sig->Argument()) && item->ppid() == ppid)
              item->setTyping(u->GetTyping());
          }
          else
          {
            // For protocols that don't use a convo id
            if (id.compare(item->id(), Qt::CaseInsensitive) == 0 && item->ppid() == ppid)
              item->setTyping(u->GetTyping());
          }
        }
      }

      gUserManager.DropUser(u);

      break;
    }
  }
}

void LicqGui::updateUserData(QString id, unsigned long ppid)
{
  myContactList->updateUser(id, ppid);
}

void LicqGui::convoSet(QString id, unsigned long ppid, unsigned long convoId)
{
  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->id() == id && item->ppid() == ppid)
    {
      item->setConvoId(convoId);
      break;
    }
  }
}

void LicqGui::convoJoin(QString id, unsigned long ppid, unsigned long convoId)
{
  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->ppid() == ppid && item->convoId() == convoId)
    {
      item->convoJoin(id, convoId);
      break;
    }
  }
}

void LicqGui::convoLeave(QString id, unsigned long ppid, unsigned long convoId)
{
  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->ppid() == ppid && item->convoId() == convoId &&
        item->isUserInConvo(id))
    {
      item->convoLeave(id, convoId);
      break;
    }
  }
}

void LicqGui::addEventTag(QString id, unsigned long ppid, unsigned long eventTag)
{
  if (id.isEmpty() || ppid == 0 || eventTag == 0)
    return;

  for (int i = 0; i < myUserSendList.size(); ++i)
  {
    UserSendCommon* item = myUserSendList.at(i);
    if (item->id() == id && item->ppid() == ppid)
    {
      item->addEventTag(eventTag);
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

