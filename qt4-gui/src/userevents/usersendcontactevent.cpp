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

#include "usersendcontactevent.h"

#include <QAction>
#include <QLabel>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>

#include <licq_user.h>
#include <licq_icqd.h>

#include "core/licqgui.h"

#include "dialogs/mmsenddlg.h"
#include "dialogs/showawaymsgdlg.h"

#include "views/mmuserview.h"

#include "widgets/mledit.h"

#include "usereventtabdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendContactEvent */

UserSendContactEvent::UserSendContactEvent(QString id, unsigned long ppid, QWidget* parent)
  : UserSendCommon(ET_CONTACT, id, ppid, parent, "UserSendContactEvent")
{
  chkMass->setChecked(false);
  chkMass->setEnabled(false);
  myForeColor->setEnabled(false);
  myBackColor->setEnabled(false);
  myEmoticon->setEnabled(false);

  mainWidget->addWidget(splView);

  QSplitter* bottom = dynamic_cast<QSplitter*>(mleSend->parentWidget());
  int ind = bottom->indexOf(mleSend);
  delete mleSend;
  mleSend = NULL;

  QWidget* w = new QWidget();
  bottom->insertWidget(ind, w);
  QVBoxLayout* lay = new QVBoxLayout(w);
  lay->setContentsMargins(0, 0, 0, 0);

  w->setToolTip(tr("Drag Users Here - Right Click for Options"));

  lstContacts = new MMUserView(myUsers.front().c_str(), myPpid,
      LicqGui::instance()->contactList());
  lay->addWidget(lstContacts);

  myBaseTitle += tr(" - Contact List");

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
  grpSendType->actions().at(ET_CONTACT)->setChecked(true);
}

UserSendContactEvent::~UserSendContactEvent()
{
  // Empty
}

void UserSendContactEvent::setContact(QString id, unsigned long ppid)
{
  ICQUser* u = gUserManager.FetchUser(id.toLatin1(), ppid, LOCK_R);

  if (u != NULL)
  {
    lstContacts->add(id, ppid);
    gUserManager.DropUser(u);
  }
}

bool UserSendContactEvent::sendDone(ICQEvent* e)
{
  if (e->Command() != ICQ_CMDxTCP_START)
    return true;

  bool showAwayDlg = false;
  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
  if (u != NULL)
  {
    showAwayDlg = u->Away() && u->ShowAwayMsg();
    gUserManager.DropUser(u);
  }

  if (showAwayDlg)
    new ShowAwayMsgDlg(myUsers.front().c_str(), myPpid);

  return true;
}

void UserSendContactEvent::resetSettings()
{
  lstContacts->clear();
  slotMassMessageToggled(false);
}

//TODO Fix this for new protocol plugin
void UserSendContactEvent::send()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, false, myConvoId);

  UserStringList users;

  QPair<QString, unsigned long> i;
  foreach (i, lstContacts->contacts())
  {
    users.push_back(i.first.toLatin1().data());
  }

  if (users.size() == 0)
    return;

  if (!checkSecure())
    return;

  if (chkMass->isChecked())
  {
    MMSendDlg* m = new MMSendDlg(lstMultipleRecipients, this);
    int r = m->go_contact(users);
    delete m;
    if (r != QDialog::Accepted) return;
  }

  unsigned long icqEventTag;
  icqEventTag = gLicqDaemon->icqSendContactList(
      myUsers.front().c_str(),
      users,
      chkSendServer->isChecked() ? false : true,
      chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
      chkMass->isChecked(),
      &icqColor);
  myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}
