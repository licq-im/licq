/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008-2013 Licq developers <licq-dev@googlegroups.com>
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

#include "userdlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/pluginsignal.h>

#include "core/signalmanager.h"
#include "core/usermenu.h"
#include "helpers/support.h"
#include "widgets/treepager.h"

#include "info.h"
#include "owner.h"
#include "settings.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserDlg */

QMap<Licq::UserId, UserDlg*> UserDlg::myDialogs;

void UserDlg::showDialog(const Licq::UserId& userId, UserPage page,
    bool updateNow)
{
  if (!userId.isValid())
    return;

  UserDlg* dialog = myDialogs.value(userId);
  if (dialog == NULL)
  {
    dialog = new UserDlg(userId);
    myDialogs.insert(userId, dialog);
  }

  if (page != UnknownPage)
    dialog->showPage(page);

  dialog->show();
  dialog->raise();
  dialog->activateWindow();

  if (updateNow)
    dialog->retrieve();
}

UserDlg::UserDlg(const Licq::UserId& userId, QWidget* parent)
  : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    myUserId(userId),
    myIcqEventTag(0)
{
  Support::setWidgetProps(this, "UserDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  myIsOwner = myUserId.isOwner();

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  myPager = new TreePager(this);
  connect(myPager, SIGNAL(currentPageChanged(QWidget*)), SLOT(pageChanged(QWidget*)));
  top_lay->addWidget(myPager);

  // Dialog buttons
  QHBoxLayout* buttonsLayout = new QHBoxLayout();
  if (!myIsOwner)
  {
    QPushButton* myUserMenuButton = new QPushButton(tr("Menu"));
    connect(myUserMenuButton, SIGNAL(pressed()), SLOT(showUserMenu()));
    myUserMenuButton->setMenu(gUserMenu);
    buttonsLayout->addWidget(myUserMenuButton);
  }

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel |
      QDialogButtonBox::Apply);

  connect(buttons, SIGNAL(accepted()), SLOT(ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));
  connect(buttons->button(QDialogButtonBox::Apply),
      SIGNAL(clicked()), SLOT(apply()));

  if (myIsOwner)
  {
    // Button to send user info to server
    mySendButton = new QPushButton(tr("Send"));
    buttons->addButton(mySendButton, QDialogButtonBox::ActionRole);
    connect(mySendButton, SIGNAL(clicked()), SLOT(send()));
  }

  // Button to retrieve user info from server
  myRetrieveButton = new QPushButton(myIsOwner ? tr("Retrieve") : tr("Update"));
  buttons->addButton(myRetrieveButton, QDialogButtonBox::ActionRole);
  connect(myRetrieveButton, SIGNAL(clicked()), SLOT(retrieve()));

  buttonsLayout->addWidget(buttons);
  top_lay->addLayout(buttonsLayout);

  myUserInfo = new UserPages::Info(myIsOwner, myUserId.protocolId(), this);
  if (!myIsOwner)
  {
    myUserSettings = new UserPages::Settings(this);
    myOwnerSettings = NULL;
  }
  else
  {
    myUserSettings = NULL;
    myOwnerSettings = new UserPages::Owner(myUserId.protocolId(), this);
  }

  {
     Licq::UserReadGuard user(myUserId);
    if (user.isLocked())
    {
      myUserInfo->load(*user);
      if (!myIsOwner)
        myUserSettings->load(*user);
      else
        myOwnerSettings->load(*user);
    }
    setBasicTitle(*user);
  }
  resetCaption();

  connect(gGuiSignalManager,
      SIGNAL(updatedUser(const Licq::UserId&, unsigned long, int, unsigned long)),
      SLOT(userUpdated(const Licq::UserId&, unsigned long)));

  QDialog::show();
}

UserDlg::~UserDlg()
{
  myDialogs.remove(myUserId);
}

void UserDlg::addPage(UserPage page, QWidget* widget, const QString& title, UserPage parent)
{
  myPages.insert(page, widget);
  myPager->addPage(widget, title, (parent == UnknownPage ? NULL : myPages.value(parent)));
}

UserDlg::UserPage UserDlg::currentPage() const
{
  return myPages.key(myPager->currentPage());
}

void UserDlg::showPage(UserPage page)
{
  if (myPages.contains(page))
    myPager->showPage(myPages.value(page));
}

void UserDlg::pageChanged(QWidget* widget)
{
  UserPage page = myPages.key(widget);
  if (page == UnknownPage)
    return;

  // Only enable retrieve and send buttons for info pages
  bool infoPage = (page == GeneralPage || page == MorePage ||
      page == More2Page || page == WorkPage || page == AboutPage ||
      page == PhonePage || page == PicturePage);

  myRetrieveButton->setEnabled(infoPage);
  if (myIsOwner)
    mySendButton->setEnabled(infoPage ||
        page == OwnerSecurityPage || page == OwnerChatGroupPage);
}

void UserDlg::retrieve()
{
  myIcqEventTag = myUserInfo->retrieve(currentPage());

  if (myIcqEventTag != 0)
  {
    setCursor(Qt::WaitCursor);
    myProgressMsg = tr("Updating...");
    connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
        SLOT(doneFunction(const Licq::Event*)));
    setWindowTitle(myBasicTitle + " [" + myProgressMsg + "]");
  }
}

void UserDlg::send()
{
  switch (currentPage())
  {
    case SettingsPage:
    case StatusPage:
    case OnEventPage:
    case GroupsPage:
    case OwnerPage:
      return;

    case OwnerSecurityPage:
    case OwnerChatGroupPage:
      myIcqEventTag = myOwnerSettings->send(currentPage());
      break;

    default:
      myIcqEventTag = myUserInfo->send(currentPage());
  }

  if (myIcqEventTag != 0)
  {
    myProgressMsg = tr("Updating server...");
    setCursor(Qt::WaitCursor);
    connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
        SLOT(doneFunction(const Licq::Event*)));
    setWindowTitle(myBasicTitle + " [" + myProgressMsg +"]");
  }
}

void UserDlg::ok()
{
  apply();
  close();
}

void UserDlg::apply()
{
  {
    Licq::UserWriteGuard user(myUserId);
    if (!user.isLocked())
      return;

    user->SetEnableSave(false);

    myUserInfo->apply(*user);
    if (!myIsOwner)
      myUserSettings->apply(*user);
    else
      myOwnerSettings->apply(*user);

    user->SetEnableSave(true);
    user->save(Licq::User::SaveAll);
  }

  // Special stuff that must be called without holding lock
  myUserInfo->apply2(myUserId);
  if (!myIsOwner)
    myUserSettings->apply2(myUserId);

  // Notify all plugins (including ourselves)
  Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserBasic);
  Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserGroups);
  Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserInfo);
  Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserSettings);
}

void UserDlg::userUpdated(const Licq::UserId& userId, unsigned long subSignal)
{
  if (userId != myUserId)
    return;

  Licq::UserReadGuard user(myUserId);
  if (!user.isLocked())
    return;

  if (subSignal == Licq::PluginSignal::UserBasic)
    setBasicTitle(*user);

  myUserInfo->userUpdated(*user, subSignal);
  if (!myIsOwner)
    myUserSettings->userUpdated(*user, subSignal);
  else
    myOwnerSettings->userUpdated(*user, subSignal);
}

void UserDlg::listUpdated(unsigned long subSignal, int /* argument */,
    const Licq::UserId& userId)
{
  if (userId != myUserId)
    return;

  switch (subSignal)
  {
    case Licq::PluginSignal::ListInvalidate:
      if (Licq::gUserManager.userExists(myUserId))
        break;
      // User no longer exists, fall through to handle the removed user

    case Licq::PluginSignal::ListUserRemoved:
    case Licq::PluginSignal::ListOwnerRemoved:
      close();
      break;
  }
}

void UserDlg::setBasicTitle(const Licq::User* user)
{
  bool wasBasicTitle = (windowTitle() == myBasicTitle);

  QString name;
  if (user == NULL)
  {
    name = tr("INVALID USER");
  }
  else
  {
    name = QString::fromUtf8(user->getFullName().c_str());
    if (!name.isEmpty())
      name = " (" + name + ")";
    name.prepend(QString::fromUtf8(user->getAlias().c_str()));
  }

  myBasicTitle = tr("Licq - Info ") + name;
  if (wasBasicTitle)
    resetCaption();
}

void UserDlg::doneFunction(const Licq::Event* event)
{
  if (!event->Equals(myIcqEventTag))
    return;

  QString title, result;
  if (event == NULL)
    result = tr("error");
  else
  {
    switch (event->Result())
    {
      case Licq::Event::ResultAcked:
      case Licq::Event::ResultSuccess:
      result = tr("done");
        break;
      case Licq::Event::ResultFailed:
      case Licq::Event::ResultUnsupported:
      result = tr("failed");
        break;
      case Licq::Event::ResultTimedout:
      result = tr("timed out");
        break;
      case Licq::Event::ResultError:
      result = tr("error");
      break;
    default:
      break;
    }
  }

  setWindowTitle(myBasicTitle + " [" + myProgressMsg + result + "]");
  QTimer::singleShot(5000, this, SLOT(resetCaption()));
  setCursor(Qt::ArrowCursor);
  myIcqEventTag = 0;
  disconnect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
      this, SLOT(doneFunction(const Licq::Event*)));
}

void UserDlg::showUserMenu()
{
  gUserMenu->setUser(myUserId);
}

void UserDlg::resetCaption()
{
  setWindowTitle(myBasicTitle);
}
