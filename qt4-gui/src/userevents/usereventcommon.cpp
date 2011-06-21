// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2011 Licq developers
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

#include "usereventcommon.h"

#include "config.h"

#include <QApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QMenu>
#include <QTextCodec>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/icqdefines.h>
#include <licq/pluginmanager.h>
#include <licq/pluginsignal.h>

#include "config/chat.h"
#include "config/iconmanager.h"
#include "config/shortcuts.h"

#include "core/licqgui.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"
#include "core/usermenu.h"

#include "dialogs/historydlg.h"
#include "dialogs/keyrequestdlg.h"

#include "helpers/support.h"
#include "helpers/usercodec.h"

#include "widgets/infofield.h"

#include "usereventtabdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserEventCommon */

using std::list;
using std::string;

UserEventCommon::UserEventCommon(const Licq::UserId& userId, QWidget* parent, const char* name)
  : QWidget(parent),
    myHighestEventId(-1)
{
  Support::setWidgetProps(this, name);
  setAttribute(Qt::WA_DeleteOnClose, true);

  myUsers.push_back(userId);
  {
    Licq::UserReadGuard user(userId);
    if (user.isLocked())
    {
      myId = user->realAccountId().c_str();
      myPpid = user->ppid();
    }
  }

  // Find out what's supported for the protocol
  mySendFuncs = 0;
  Licq::ProtocolPlugin::Ptr protocol = Licq::gPluginManager.getProtocolPlugin(myPpid);
  if (protocol.get() != NULL)
    mySendFuncs = protocol->capabilities();

  myCodec = QTextCodec::codecForLocale();
  myIsOwner = Licq::gUserManager.isOwner(myUsers.front());
  myDeleteUser = false;
  myConvoId = 0;

  myTophLayout = new QHBoxLayout(this);
  myTopLayout = new QVBoxLayout();
  myTophLayout->addLayout(myTopLayout);
  myTophLayout->setStretchFactor(myTopLayout, 1);

  QHBoxLayout* layt = new QHBoxLayout();
  myTopLayout->addLayout(layt);

  myToolBar = new QToolBar();
  myToolBar->setIconSize(QSize(16, 16));
  layt->addWidget(myToolBar);

  layt->addStretch(1);

  myTimezone = new InfoField(true);
  myTimezone->setToolTip(tr("User's current local time"));
  int timezoneWidth = 
    qMax(myTimezone->fontMetrics().width("88:88:88"),
         myTimezone->fontMetrics().width(tr("Unknown")))
         + 10;
  myTimezone->setFixedWidth(timezoneWidth);
  myTimezone->setAlignment(Qt::AlignCenter);
  myTimezone->setFocusPolicy(Qt::ClickFocus);
  layt->addWidget(myTimezone);

  myMenu = myToolBar->addAction(tr("Menu"), this, SLOT(showUserMenu()));
  myMenu->setMenu(gUserMenu);
  if (myIsOwner)
    myMenu->setEnabled(false);

  myHistory = myToolBar->addAction(tr("History..."), this, SLOT(showHistory()));
  myInfo = myToolBar->addAction(tr("User Info..."), this, SLOT(showUserInfo()));

  myEncodingsMenu = new QMenu(this);
  myEncoding = myToolBar->addAction(tr("Encoding"), this, SLOT(showEncodingsMenu()));
  myEncoding->setMenu(myEncodingsMenu);

  myToolBar->addSeparator();

  mySecure = myToolBar->addAction(tr("Secure Channel"), this, SLOT(switchSecurity()));
  if (!(mySendFuncs & Licq::ProtocolPlugin::CanSendSecure))
    mySecure->setEnabled(false);

  myTimeTimer = NULL;
  myTypingTimer = NULL;

  {
    Licq::UserReadGuard u(myUsers.front());
    if (u.isLocked())
    {
      if (u->NewMessages() == 0)
        setWindowIcon(IconManager::instance()->iconForUser(*u));
      else
      {
        setWindowIcon(IconManager::instance()->iconForEvent(ICQ_CMDxSUB_MSG));
        flashTaskbar();
      }

      updateWidgetInfo(*u);

      // restore prefered encoding
      myCodec = UserCodec::codecForUser(*u);

      setTyping(u->isTyping());
    }
  }

  myEncodingsGroup = new QActionGroup(this);
  connect(myEncodingsGroup, SIGNAL(triggered(QAction*)), SLOT(setEncoding(QAction*)));

  QString codec_name = QString::fromLatin1(myCodec->name()).toLower();

  // populate the popup menu
  for (UserCodec::encoding_t* it = &UserCodec::m_encodings[0]; it->encoding != NULL; ++it)
  {
    // Use check_codec since the QTextCodec name will be different from the
    // user codec. But QTextCodec will recognize both, so let's make it standard
    // for the purpose of checking for the same string.
    QTextCodec* check_codec = QTextCodec::codecForName(it->encoding);

    bool currentCodec = check_codec != NULL && QString::fromLatin1(check_codec->name()).toLower() == codec_name;

    if (!currentCodec && !Config::Chat::instance()->showAllEncodings() && !it->isMinimal)
      continue;

    QAction* a = new QAction(UserCodec::nameForEncoding(it->encoding), myEncodingsGroup);
    a->setCheckable(true);
    a->setData(it->mib);

    if (currentCodec)
      a->setChecked(true);

    if (currentCodec && !Config::Chat::instance()->showAllEncodings() && !it->isMinimal)
    {
      // if the current encoding does not appear in the minimal list
      myEncodingsMenu->insertSeparator(myEncodingsMenu->actions()[0]);
      myEncodingsMenu->insertAction(myEncodingsMenu->actions()[0], a);
    }
    else
    {
      myEncodingsMenu->addAction(a);
    }
  }

  myPopupNextMessage = new QAction("Popup Next Message", this);
  addAction(myPopupNextMessage);
  connect(myPopupNextMessage, SIGNAL(triggered()), gLicqGui, SLOT(showNextEvent()));

  // We might be called from a slot so connect the signal only after all the
  // existing signals are handled.
  QTimer::singleShot(0, this, SLOT(connectSignal()));

  myMainWidget = new QVBoxLayout();
  myMainWidget->setContentsMargins(0, 0, 0, 0);
  myTopLayout->addLayout(myMainWidget);

  updateIcons();
  updateShortcuts();
  connect(IconManager::instance(), SIGNAL(generalIconsChanged()), SLOT(updateIcons()));
  connect(Config::Shortcuts::instance(), SIGNAL(shortcutsChanged()), SLOT(updateShortcuts()));

  // Check if we want the window sticky
  if (!Config::Chat::instance()->tabbedChatting() &&
      Config::Chat::instance()->msgWinSticky())
    QTimer::singleShot(100, this, SLOT(setMsgWinSticky()));
}

UserEventCommon::~UserEventCommon()
{
  emit finished(myUsers.front());

  if (myDeleteUser && !myIsOwner)
    gLicqGui->removeUserFromList(myUsers.front(), this);

  myUsers.clear();
}

void UserEventCommon::updateIcons()
{
  IconManager* iconman = IconManager::instance();

  myMenu->setIcon(iconman->getIcon(IconManager::MenuIcon));
  myHistory->setIcon(iconman->getIcon(IconManager::HistoryIcon));
  myInfo->setIcon(iconman->getIcon(IconManager::InfoIcon));
  myEncoding->setIcon(iconman->getIcon(IconManager::EncodingIcon));
}

void UserEventCommon::updateShortcuts()
{
  Config::Shortcuts* shortcuts = Config::Shortcuts::instance();

  myPopupNextMessage->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatPopupNextMessage));

  myMenu->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatUserMenu));
  myHistory->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatHistory));
  myInfo->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatUserInfo));
  myEncoding->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatEncodingMenu));
  mySecure->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatToggleSecure));

  // Tooltips include shortcut so update them here as well
  pushToolTip(myMenu, tr("Open user menu"));
  pushToolTip(myHistory, tr("Show user history"));
  pushToolTip(myInfo, tr("Show user information"));
  pushToolTip(myEncoding, tr("Select the text encoding used for outgoing messages."));
  pushToolTip(mySecure, tr("Open / Close secure channel"));
}

bool UserEventCommon::isUserInConvo(const Licq::UserId& userId) const
{
  bool found = (std::find(myUsers.begin(), myUsers.end(), userId) != myUsers.end());
  return found;
}

void UserEventCommon::setTyping(bool isTyping)
{
  if (isTyping)
  {
    if (myTypingTimer->isActive())
      myTypingTimer->stop();
    myTypingTimer->setSingleShot(true);
    myTypingTimer->start(10000);

    QPalette p = myTimezone->palette();
    p.setColor(myTimezone->backgroundRole(), Config::Chat::instance()->tabTypingColor());
    myTimezone->setPalette(p);
  }
  else
  {
    myTimezone->setPalette(QPalette());
  }
}

void UserEventCommon::flashTaskbar()
{
  if (Config::Chat::instance()->flashTaskbar())
    QApplication::alert(this);
}

void UserEventCommon::updateWidgetInfo(const Licq::User* u)
{
  const QTextCodec* codec = UserCodec::codecForUser(u);

  if (u->GetTimezone() == Licq::User::TimezoneUnknown)
    myTimezone->setText(tr("Unknown"));
  else
  {
    myRemoteTimeOffset = u->LocalTimeOffset();
    updateTime();

    if (myTimeTimer == NULL)
    {
      myTimeTimer = new QTimer(this);
      connect(myTimeTimer, SIGNAL(timeout()), SLOT(updateTime()));
      myTimeTimer->start(3000);
    }
  }

  if (myTypingTimer == NULL)
  {
    myTypingTimer = new QTimer(this);
    connect(myTypingTimer, SIGNAL(timeout()), SLOT(updateTyping()));
  }

  if (u->Secure())
    mySecure->setIcon(IconManager::instance()->getIcon(IconManager::SecureOnIcon));
  else
    mySecure->setIcon(IconManager::instance()->getIcon(IconManager::SecureOffIcon));

  QString tmp = codec->toUnicode(u->getFullName().c_str());
  if (!tmp.isEmpty())
    tmp = " (" + tmp + ")";
  myBaseTitle = QString::fromUtf8(u->getAlias().c_str()) + tmp;

  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
  {
    tabDlg->setWindowTitle(myBaseTitle);
    tabDlg->setWindowIconText(QString::fromUtf8(u->getAlias().c_str()));
  }
  else
  {
    setWindowTitle(myBaseTitle);
    setWindowIconText(QString::fromUtf8(u->getAlias().c_str()));
  }
}

void UserEventCommon::pushToolTip(QAction* action, const QString& tooltip)
{
  if (action == 0 || tooltip.isEmpty())
    return;

  QString newtip = tooltip;

  if (!action->shortcut().isEmpty())
    newtip += " (" + action->shortcut().toString(QKeySequence::NativeText) + ")";

  action->setToolTip(newtip);
}

void UserEventCommon::connectSignal()
{
  connect(gGuiSignalManager,
      SIGNAL(updatedUser(const Licq::UserId&, unsigned long, int, unsigned long)),
      SLOT(updatedUser(const Licq::UserId&, unsigned long, int, unsigned long)));
}

void UserEventCommon::setEncoding(QAction* action)
{
  int encodingMib = action->data().toUInt();

  /* initialize a codec according to the encoding menu item id */
  QString encoding(UserCodec::encodingForMib(encodingMib));

  if (!encoding.isNull())
  {
    QTextCodec* codec = QTextCodec::codecForName(encoding.toLatin1());
    if (codec == NULL)
    {
      WarnUser(this, tr("Unable to load encoding <b>%1</b>.<br>"
            "Message contents may appear garbled.").arg(encoding));
      return;
    }
    myCodec = codec;

    /* save preferred character set */
    {
      Licq::UserWriteGuard u(myUsers.front());
      if (u.isLocked())
      {
        u->SetEnableSave(false);
        u->setUserEncoding(encoding.toLatin1().data());
        u->SetEnableSave(true);
        u->SaveLicqInfo();
      }
    }

    emit encodingChanged();
  }
}

void UserEventCommon::setMsgWinSticky(bool sticky)
{
  Support::changeWinSticky(winId(), sticky);
}

void UserEventCommon::showHistory()
{
  new HistoryDlg(myUsers.front());
}

void UserEventCommon::showUserInfo()
{
  gLicqGui->showInfoDialog(mnuUserGeneral, myUsers.front(), true);
}

void UserEventCommon::switchSecurity()
{
  new KeyRequestDlg(myUsers.front());
}

void UserEventCommon::updateTime()
{
  QDateTime t;
  t.setTime_t(time(NULL) + myRemoteTimeOffset);
  myTimezone->setText(t.time().toString());
}

void UserEventCommon::updateTyping()
{
  // MSN needs this, ICQ/AIM doesn't send additional packets
  // This does need to be verified with the official AIM client, there is a
  // packet for it, but ICQ isn't using it apparently.
  if (myPpid == LICQ_PPID || myUsers.empty())
    return;

  //FIXME Which user?
  Licq::UserWriteGuard u(myUsers.front());
  u->setIsTyping(false);
  myTimezone->setPalette(QPalette());
  UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
  if (Config::Chat::instance()->tabbedChatting() && tabDlg != NULL)
    tabDlg->updateTabLabel(*u);
}

void UserEventCommon::showUserMenu()
{
  // Tell menu which contact to use and show it immediately.
  // Menu is normally delayed but if we use InstantPopup mode we won't get
  //   this signal so we can't tell menu which contact to use.
  gUserMenu->setUser(myUsers.front());
  dynamic_cast<QToolButton*>(myToolBar->widgetForAction(myMenu))->showMenu();
}

void UserEventCommon::showEncodingsMenu()
{
  // Menu is normally delayed but if we use InstantPopup mode shortcut won't work
  dynamic_cast<QToolButton*>(myToolBar->widgetForAction(myEncoding))->showMenu();
}

void UserEventCommon::updatedUser(const Licq::UserId& userId, unsigned long subSignal, int argument, unsigned long cid)
{
  if (!isUserInConvo(userId))
  {
    if (myConvoId != 0 && cid == myConvoId)
    {
      myUsers.push_back(userId);

      // Now update the tab label
      UserEventTabDlg* tabDlg = gLicqGui->userEventTabDlg();
      if (tabDlg != NULL)
        tabDlg->updateConvoLabel(this);
    }
    else
    {
      return;
    }
  }

  Licq::UserReadGuard u(userId);
  if (!u.isLocked())
    return;

  switch (subSignal)
  {
    case Licq::PluginSignal::UserStatus:
      if (u->NewMessages() == 0)
        setWindowIcon(IconManager::instance()->iconForUser(*u));
      break;

    case Licq::PluginSignal::UserBasic:
    case Licq::PluginSignal::UserInfo: // For time zone
    case Licq::PluginSignal::UserSecurity:
      updateWidgetInfo(*u);
      break;

    case Licq::PluginSignal::UserEvents:
      if (u->NewMessages() == 0)
        setWindowIcon(IconManager::instance()->iconForUser(*u));
      else
      {
        setWindowIcon(IconManager::instance()->iconForEvent(ICQ_CMDxSUB_MSG));
        flashTaskbar();
      }

      break;
  }

  u.unlock();

  // Call the event specific function now
  userUpdated(userId, subSignal, argument, cid);
}

void UserEventCommon::focusChanged(bool gotFocus)
{
  // Check if we should block on events, but always unblock in case we might leave a user blocked
  if (gotFocus && !Config::Chat::instance()->noSoundInActiveChat())
    return;

  Licq::UserWriteGuard user(userId());
  if (user.isLocked())
    user->setOnEventsBlocked(gotFocus);
}

bool UserEventCommon::event(QEvent* event)
{
  // Mark/unmark user as active user when we get/loose focus
  if (event->type() == QEvent::WindowActivate || event->type() == QEvent::ShowToParent)
    focusChanged(true);
  if (event->type() == QEvent::WindowDeactivate || event->type() == QEvent::HideToParent)
    focusChanged(false);

  return QWidget::event(event);
}
