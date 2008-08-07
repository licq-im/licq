// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2008 Licq developers
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
#include <QTextCodec>
#include <QTimer>
#include <QVBoxLayout>

#include <licq_events.h>
#include <licq_icqd.h>
#include <licq_user.h>

#include "core/licqgui.h"
#include "core/signalmanager.h"
#include "core/usermenu.h"
#include "helpers/support.h"
#include "helpers/usercodec.h"
#include "widgets/treepager.h"

#include "info.h"
#include "settings.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserDlg */

UserDlg::UserDlg(const QString& id, unsigned long ppid, QWidget* parent)
  : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
    myId(id),
    myPpid(ppid),
    myIcqEventTag(0)
{
  Support::setWidgetProps(this, "UserDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  myIsOwner = (gUserManager.FindOwner(myId.toLatin1(), myPpid) != NULL);

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
    myUserMenuButton->setMenu(LicqGui::instance()->userMenu());
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

  myUserInfo = new UserPages::Info(myIsOwner, this);
  myUserSettings = new UserPages::Settings(myIsOwner, this);

  const ICQUser* user = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_R);
  if (user != NULL)
  {
    QTextCodec* codec = UserCodec::codecForICQUser(user);
    QString name = codec->toUnicode(user->GetFirstName());
    QString lastname = codec->toUnicode(user->GetLastName());
    if ((!name.isEmpty()) && (!lastname.isEmpty()))
      name += " ";
    name += lastname;
    if (!name.isEmpty())
      name = " (" + name + ")";
    myBasicTitle = tr("Licq - Info ") + QString::fromUtf8(user->GetAlias()) + name;

    myUserInfo->load(user);
    myUserSettings->load(user);

    gUserManager.DropUser(user);
  }
  else
  {
    myBasicTitle = tr("Licq - Info ") + tr("INVALID USER");
  }
  resetCaption();

  connect(LicqGui::instance()->signalManager(),
      SIGNAL(updatedUser(CICQSignal*)), SLOT(userUpdated(CICQSignal*)));

  QDialog::show();
}

UserDlg::~UserDlg()
{
  emit finished(this);
}

void UserDlg::addPage(UserPage page, QWidget* widget, QString title, UserPage parent)
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
    mySendButton->setEnabled(infoPage);
}

void UserDlg::retrieve()
{
  myIcqEventTag = myUserInfo->retrieve(currentPage());

  if (myIcqEventTag != 0)
  {
    setCursor(Qt::WaitCursor);
    myProgressMsg = tr("Updating...");
    connect(LicqGui::instance()->signalManager(),
        SIGNAL(doneUserFcn(ICQEvent*)), SLOT(doneFunction(ICQEvent*)));
    setWindowTitle(myBasicTitle + " [" + myProgressMsg + "]");
  }
}

void UserDlg::send()
{
  myIcqEventTag = myUserInfo->send(currentPage());

  if (myIcqEventTag != 0)
  {
    myProgressMsg = tr("Updating server...");
    setCursor(Qt::WaitCursor);
    connect(LicqGui::instance()->signalManager(),
        SIGNAL(doneUserFcn(ICQEvent*)), SLOT(doneFunction(ICQEvent*)));
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
  ICQUser* user = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_W);
  if (user == NULL)
    return;

  user->SetEnableSave(false);

  myUserInfo->apply(user);
  myUserSettings->apply(user);

  user->SetEnableSave(true);
  user->saveAll();

  gUserManager.DropUser(user);

  // Special stuff that must be called without holding lock
  myUserInfo->apply2(myId, myPpid);
  myUserSettings->apply2(myId, myPpid);

  // Make sure GUI is updated
  LicqGui::instance()->updateUserData(myId, myPpid);
}

void UserDlg::userUpdated(CICQSignal* sig)
{
  if (sig->PPID() != myPpid || sig->Id() != myId)
    return;

  const ICQUser* user = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_R);
  if (user == NULL)
    return;

  myUserInfo->userUpdated(sig, user);
  myUserSettings->userUpdated(sig, user);

  gUserManager.DropUser(user);
}

void UserDlg::doneFunction(ICQEvent* event)
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
    case EVENT_ACKED:
    case EVENT_SUCCESS:
      result = tr("done");
      break;
    case EVENT_FAILED:
      result = tr("failed");
      break;
    case EVENT_TIMEDOUT:
      result = tr("timed out");
      break;
    case EVENT_ERROR:
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
  disconnect(LicqGui::instance()->signalManager(),
      SIGNAL(doneUserFcn(ICQEvent*)), this, SLOT(doneFunction(ICQEvent*)));
}

void UserDlg::showUserMenu()
{
  LicqGui::instance()->userMenu()->setUser(myId, myPpid);
}

void UserDlg::resetCaption()
{
  setWindowTitle(myBasicTitle);
}
