// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#include <licq_message.h>

#include <QTimer>
#include <QVBoxLayout>

#ifdef USE_KDE
#include <KDE/KWindowSystem>
#endif

#include "config/chat.h"
#include "config/iconmanager.h"

#include "helpers/support.h"

#include "widgets/tabwidget.h"

#include "usereventcommon.h"
#include "usersendcommon.h"

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

  if (Config::Chat::instance()->dialogRect().isValid())
    setGeometry(Config::Chat::instance()->dialogRect());

  myTabs = new TabWidget();
  lay->addWidget(myTabs);

  connect(myTabs, SIGNAL(currentChanged(int)), SLOT(currentChanged(int)));
  connect(myTabs, SIGNAL(mouseMiddleClick(QWidget*)), SLOT(removeTab(QWidget*)));
}

UserEventTabDlg::~UserEventTabDlg()
{
  saveGeometry();
  emit signal_done();
}

void UserEventTabDlg::addTab(UserEventCommon* tab, int index)
{
  const ICQUser* u = gUserManager.FetchUser(tab->id().toLatin1(), tab->ppid(), LOCK_R);
  if (u == NULL)
    return;

  index = myTabs->insertTab(index, tab, QString::fromUtf8(u->GetAlias()));
  updateTabLabel(tab, u);
  gUserManager.DropUser(u);
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
  list<string> users = tab->convoUsers();
  list<string>::iterator it;
  QString newLabel = QString::null;

  for (it = users.begin(); it != users.end(); ++it)
  {
    const ICQUser* u = gUserManager.FetchUser((*it).c_str(), tab->ppid(), LOCK_R);

    if (!newLabel.isEmpty())
      newLabel += ", ";

    if (u == 0)
      newLabel += tr("[UNKNOWN_USER]");
    else
    {
      newLabel += QString::fromUtf8(u->GetAlias());
      gUserManager.DropUser(u);
    }
  }

  myTabs->setTabText(myTabs->indexOf(tab), newLabel);
}

void UserEventTabDlg::updateTabLabel(const ICQUser* u)
{
  if (u == NULL)
    return;

  for (int index = 0; index < myTabs->count(); index++)
  {
    UserEventCommon* tab = dynamic_cast<UserEventCommon*>(myTabs->widget(index));

    if (tab->ppid() == u->PPID() &&
        tab->isUserInConvo(u->IdString()))
      updateTabLabel(tab, u);
  }
}

void UserEventTabDlg::updateTabLabel(UserEventCommon* tab, const ICQUser* u)
{
  if (tab == NULL)
    return;

  bool fetched = false;
  if (u == NULL ||
      !tab->isUserInConvo(u->IdString()))
  {
    u = gUserManager.FetchUser(tab->id().toLatin1(), tab->ppid(), LOCK_R);
    if (u == NULL)
      return;
    fetched = true;
  }

  QIcon icon;

  if (u->NewMessages() > 0) // use an event icon
  {
    unsigned short SubCommand = ICQ_CMDxSUB_MSG;
    for (unsigned short i = 0; i < u->NewMessages(); i++)
      switch (u->EventPeek(i)->SubCommand())
      {
	case ICQ_CMDxSUB_FILE:
	  SubCommand = ICQ_CMDxSUB_FILE;
	  break;
	case ICQ_CMDxSUB_CHAT:
	  if (SubCommand != ICQ_CMDxSUB_FILE)
	    SubCommand = ICQ_CMDxSUB_CHAT;
	  break;
	case ICQ_CMDxSUB_URL:
	  if (SubCommand != ICQ_CMDxSUB_FILE &&
	      SubCommand != ICQ_CMDxSUB_CHAT)
	    SubCommand = ICQ_CMDxSUB_URL;
	  break;
	case ICQ_CMDxSUB_CONTACTxLIST:
	  if (SubCommand != ICQ_CMDxSUB_FILE &&
	      SubCommand != ICQ_CMDxSUB_CHAT &&
	      SubCommand != ICQ_CMDxSUB_URL)
	    SubCommand = ICQ_CMDxSUB_CONTACTxLIST;
	  break;
      }

    icon = IconManager::instance()->iconForEvent(SubCommand);
    myTabs->setTabColor(tab, QColor("blue"));

    // to clear it..
    tab->setTyping(u->GetTyping());
  }
  else // use status icon
  {
    icon = IconManager::instance()->iconForStatus(u->StatusFull(), u->IdString(), u->PPID());

    if (u->GetTyping() == ICQ_TYPING_ACTIVE)
      myTabs->setTabColor(tab, Config::Chat::instance()->tabTypingColor());
    else
      myTabs->setTabColor(tab, QColor("black"));
  }

  if (fetched)
    gUserManager.DropUser(u);

  int index = myTabs->indexOf(tab);
  myTabs->setTabIcon(index, icon);
  if (myTabs->currentIndex() == index)
    setWindowIcon(icon);
}

void UserEventTabDlg::setTyping(const ICQUser* u, int convoId)
{
  for (int index = 0; index < myTabs->count(); index++)
  {
    UserEventCommon* tab = dynamic_cast<UserEventCommon*>(myTabs->widget(index));

    if (tab->convoId() == static_cast<unsigned long>(convoId) &&
        tab->ppid() == u->PPID() &&
        tab->isUserInConvo(u->IdString()))
      tab->setTyping(u->GetTyping());
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

  QIcon icon = myTabs->tabIcon(myTabs->indexOf(tab));
  if (!icon.isNull())
    setWindowIcon(icon);
}

void UserEventTabDlg::clearEvents(QWidget* tab)
{
  if (!isActiveWindow())
    return;

  UserSendCommon* e = dynamic_cast<UserSendCommon*>(tab);
  QTimer::singleShot(e->clearDelay, e, SLOT(clearNewEvents()));
}

void UserEventTabDlg::saveGeometry()
{
  Config::Chat::instance()->setDialogRect(geometry());
}

void UserEventTabDlg::moveEvent(QMoveEvent* /* e */)
{
  saveGeometry();
}

void UserEventTabDlg::resizeEvent(QResizeEvent* /* e */)
{
  saveGeometry();
}
