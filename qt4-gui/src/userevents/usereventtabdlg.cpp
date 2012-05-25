/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2012 Licq developers <licq-dev@googlegroups.com>
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

#include "usereventtabdlg.h"

#include "config.h"

#include <licq/contactlist/user.h>
#include <licq/userevents.h>

#include <QAction>
#include <QActionGroup>
#include <QTimer>
#include <QVBoxLayout>

#ifdef USE_KDE
#include <KDE/KWindowSystem>
#endif

#include "config/chat.h"
#include "config/iconmanager.h"
#include "config/shortcuts.h"

#include "helpers/support.h"

#include "widgets/tabwidget.h"

#include "usereventcommon.h"
#include "usersendevent.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserEventTabDlg */

using std::list;
using std::string;

UserEventTabDlg::UserEventTabDlg(QWidget* parent, const char* name)
  : QWidget(parent)
{
  Support::setWidgetProps(this, name);
  setAttribute(Qt::WA_DeleteOnClose, true);

  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->setContentsMargins(0, 0, 0, 0);

  if (Config::Chat::instance()->tabDialogRect().isValid())
    setGeometry(Config::Chat::instance()->tabDialogRect());

  myTabs = new TabWidget();
  lay->addWidget(myTabs);

  connect(myTabs, SIGNAL(currentChanged(int)), SLOT(currentChanged(int)));
  connect(myTabs, SIGNAL(mouseMiddleClick(QWidget*)), SLOT(removeTab(QWidget*)));

  QActionGroup* tabActionGroup = new QActionGroup(this);
  connect(tabActionGroup, SIGNAL(triggered(QAction*)), SLOT(switchTab(QAction*)));

#define ADD_TABSHORTCUT(var, shortcut, index) \
  var = new QAction(tabActionGroup); \
  var->setData(index);

  ADD_TABSHORTCUT(myTabSwitch01Action, Config::Shortcuts::ChatTab01, 0);
  ADD_TABSHORTCUT(myTabSwitch02Action, Config::Shortcuts::ChatTab02, 1);
  ADD_TABSHORTCUT(myTabSwitch03Action, Config::Shortcuts::ChatTab03, 2);
  ADD_TABSHORTCUT(myTabSwitch04Action, Config::Shortcuts::ChatTab04, 3);
  ADD_TABSHORTCUT(myTabSwitch05Action, Config::Shortcuts::ChatTab05, 4);
  ADD_TABSHORTCUT(myTabSwitch06Action, Config::Shortcuts::ChatTab06, 5);
  ADD_TABSHORTCUT(myTabSwitch07Action, Config::Shortcuts::ChatTab07, 6);
  ADD_TABSHORTCUT(myTabSwitch08Action, Config::Shortcuts::ChatTab08, 7);
  ADD_TABSHORTCUT(myTabSwitch09Action, Config::Shortcuts::ChatTab09, 8);
  ADD_TABSHORTCUT(myTabSwitch10Action, Config::Shortcuts::ChatTab10, 9);

#undef ADD_TABSHORTCUT

  addActions(tabActionGroup->actions());
  updateShortcuts();
  connect(Config::Shortcuts::instance(), SIGNAL(shortcutsChanged()), SLOT(updateShortcuts()));
}

void UserEventTabDlg::updateShortcuts()
{
  const Config::Shortcuts* shortcuts = Config::Shortcuts::instance();
  myTabSwitch01Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab01));
  myTabSwitch02Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab02));
  myTabSwitch03Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab03));
  myTabSwitch04Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab04));
  myTabSwitch05Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab05));
  myTabSwitch06Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab06));
  myTabSwitch07Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab07));
  myTabSwitch08Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab08));
  myTabSwitch09Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab09));
  myTabSwitch10Action->setShortcut(shortcuts->getShortcut(Config::Shortcuts::ChatTab10));
}

UserEventTabDlg::~UserEventTabDlg()
{
  saveGeometry();
  emit signal_done();
}

void UserEventTabDlg::addTab(UserEventCommon* tab, int index)
{
  // Insert tab before we lock the user as our event() function will be called
  // when tab is inserted and will also need to fetch user.
  // Tab label will be set later by updateTabLabel()
  myTabs->insertTab(index, tab, QString());

  Licq::UserReadGuard u(tab->userId());
  if (!u.isLocked())
    return;

  updateTabLabel(tab, *u);
}

void UserEventTabDlg::switchTab(QAction* action)
{
  int index = action->data().toInt();

  myTabs->setCurrentIndex(index);
}

void UserEventTabDlg::selectTab(QWidget* tab)
{
  myTabs->setCurrentIndex(myTabs->indexOf(tab));
  updateTitle(tab);
}

void UserEventTabDlg::replaceTab(QWidget* oldTab, UserEventCommon* newTab)
{
  addTab(newTab, myTabs->indexOf(oldTab) + 1);
  removeTab(oldTab);
}

bool UserEventTabDlg::tabIsSelected(QWidget* tab)
{
  return (myTabs->currentIndex() == myTabs->indexOf(tab));
}

bool UserEventTabDlg::tabExists(QWidget* tab)
{
  return (myTabs->indexOf(tab) != -1);
}

void UserEventTabDlg::updateConvoLabel(UserEventCommon* tab)
{
  // Show the list of users in the conversation
  list<Licq::UserId> users = tab->convoUsers();
  list<Licq::UserId>::iterator it;
  QString newLabel = QString::null;

  for (it = users.begin(); it != users.end(); ++it)
  {
    if (!newLabel.isEmpty())
      newLabel += ", ";

    Licq::UserReadGuard u(*it);
    if (!u.isLocked())
      newLabel += tr("[UNKNOWN_USER]");
    else
      newLabel += QString::fromUtf8(u->getAlias().c_str());
  }

  myTabs->setTabText(myTabs->indexOf(tab), newLabel);
}

void UserEventTabDlg::updateTabLabel(const Licq::User* u)
{
  if (u == NULL)
    return;

  for (int index = 0; index < myTabs->count(); index++)
  {
    UserEventCommon* tab = dynamic_cast<UserEventCommon*>(myTabs->widget(index));

    if (tab->isUserInConvo(u->id()))
      updateTabLabel(tab, u);
  }
}

void UserEventTabDlg::updateTabLabel(UserEventCommon* tab, const Licq::User* u)
{
  if (tab == NULL)
    return;

  int index = myTabs->indexOf(tab);
  myTabs->setTabText(index, QString::fromUtf8(u->getAlias().c_str()));

  QIcon icon;

  if (u->NewMessages() > 0) // use an event icon
  {
    unsigned eventType = Licq::UserEvent::TypeMessage;
    for (unsigned short i = 0; i < u->NewMessages(); i++)
      switch (u->EventPeek(i)->eventType())
      {
        case Licq::UserEvent::TypeFile:
          eventType = Licq::UserEvent::TypeFile;
          break;
        case Licq::UserEvent::TypeChat:
          if (eventType != Licq::UserEvent::TypeFile)
            eventType = Licq::UserEvent::TypeChat;
          break;
        case Licq::UserEvent::TypeUrl:
          if (eventType != Licq::UserEvent::TypeFile &&
              eventType != Licq::UserEvent::TypeChat)
            eventType = Licq::UserEvent::TypeUrl;
          break;
        case Licq::UserEvent::TypeContactList:
          if (eventType != Licq::UserEvent::TypeFile &&
              eventType != Licq::UserEvent::TypeChat &&
              eventType != Licq::UserEvent::TypeUrl)
            eventType = Licq::UserEvent::TypeContactList;
          break;
      }

    icon = IconManager::instance()->iconForEvent(eventType);
    myTabs->setTabColor(tab, QColor("blue"));

    // to clear it..
    tab->setTyping(u->isTyping());
  }
  else // use status icon
  {
    icon = IconManager::instance()->iconForUser(u);

    if (u->isTyping())
      myTabs->setTabColor(tab, Config::Chat::instance()->tabTypingColor());
    else
      myTabs->setTabColor(tab, QColor());
  }

  myTabs->setTabIcon(index, icon);
  if (myTabs->currentIndex() == index)
    setWindowIcon(icon);
}

void UserEventTabDlg::setTyping(const Licq::User* u, int convoId)
{
  for (int index = 0; index < myTabs->count(); index++)
  {
    UserEventCommon* tab = dynamic_cast<UserEventCommon*>(myTabs->widget(index));

    if (tab->convoId() == static_cast<unsigned long>(convoId) &&
        tab->isUserInConvo(u->id()))
      tab->setTyping(u->isTyping());
  }
}

#ifdef USE_KDE
/* KDE 3.2 handles app-icon updates differently, since KDE 3.2 a simple setIcon() call
   does no longer update the icon in kicker anymore :(
   So we do it the "kde-way" here */
void UserEventTabDlg::setIcon(const QPixmap& icon)
{
  KWindowSystem::setIcons(winId(), icon, icon);
}
#endif

/*! This slot should get called when the current tab has
 *  changed.
 */
void UserEventTabDlg::currentChanged(int index)
{
  QWidget* tab = myTabs->widget(index);
  tab->setFocus();  // prevents users from accidentally typing in the wrong widget
  updateTitle(tab);
  clearEvents(tab);
}

void UserEventTabDlg::moveLeft()
{
  myTabs->setPreviousPage();
}

void UserEventTabDlg::moveRight()
{
  myTabs->setNextPage();
}

void UserEventTabDlg::removeTab(QWidget* tab)
{
  if (myTabs->count() > 1)
  {
    int index = myTabs->indexOf(tab);
    myTabs->removeTab(index);
    tab->close();
    tab->setEnabled(false);
    tab->deleteLater();
  }
  else
    close();
}

void UserEventTabDlg::setMsgWinSticky(bool sticky)
{
  Support::changeWinSticky(winId(), sticky);
}

void UserEventTabDlg::updateTitle(QWidget* tab)
{
  QString title = tab->windowTitle();
  if (!title.isEmpty())
    setWindowTitle(title);

  setWindowIconText(tab->windowIconText());

  QIcon icon = myTabs->tabIcon(myTabs->indexOf(tab));
  if (!icon.isNull())
    setWindowIcon(icon);
}

void UserEventTabDlg::clearEvents(QWidget* tab)
{
  if (!isActiveWindow())
    return;

  UserSendEvent* e = dynamic_cast<UserSendEvent*>(tab);
  QTimer::singleShot(e->clearDelay, e, SLOT(clearNewEvents()));
}

void UserEventTabDlg::saveGeometry()
{
  QRect geom = geometry();

  // When first showing window, we may get a rect without frame which will be
  // become wrong if used for a window with a frame
  if (geom == frameGeometry())
    return;

  // For setGeometry to get it right we need position including frame but
  // size without frame
  geom.moveTo(frameGeometry().topLeft());

  Config::Chat::instance()->setTabDialogRect(geom);
}

void UserEventTabDlg::moveEvent(QMoveEvent* event)
{
  saveGeometry();
  QWidget::moveEvent(event);
}

void UserEventTabDlg::resizeEvent(QResizeEvent* event)
{
  saveGeometry();
  QWidget::resizeEvent(event);
}
