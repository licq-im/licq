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

#include "usersendchatevent.h"

#include <QAction>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QTextCodec>

#include <licq_icqd.h>

#include "config/chat.h"

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"

#include "dialogs/chatdlg.h"
#include "dialogs/joinchatdlg.h"

#include "widgets/infofield.h"
#include "widgets/mledit.h"

#include "usereventtabdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendChatEvent */

UserSendChatEvent::UserSendChatEvent(QString id, unsigned long ppid, QWidget* parent)
  : UserSendCommon(ChatEvent, id, ppid, parent, "UserSendChatEvent")
{
  myChatPort = 0;
  chkMass->setChecked(false);
  chkMass->setEnabled(false);
  myForeColor->setEnabled(false);
  myBackColor->setEnabled(false);

  mainWidget->addWidget(splView);

  if (!Config::Chat::instance()->msgChatView())
    mleSend->setMinimumHeight(150);

  QHBoxLayout* h_lay = new QHBoxLayout();
  mainWidget->addLayout(h_lay);
  lblItem = new QLabel(tr("Multiparty: "));
  h_lay->addWidget(lblItem);

  edtItem = new InfoField(false);
  h_lay->addWidget(edtItem);

  btnBrowse = new QPushButton(tr("Invite"));
  connect(btnBrowse, SIGNAL(clicked()), SLOT(inviteUser()));
  h_lay->addWidget(btnBrowse);

  myBaseTitle += tr(" - Chat Request");

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
  grpSendType->actions().at(ChatEvent)->setChecked(true);
}

UserSendChatEvent::~UserSendChatEvent()
{
  // Empty
}

bool UserSendChatEvent::sendDone(ICQEvent* e)
{
  if (!e->ExtendedAck() || !e->ExtendedAck()->Accepted())
  {
    ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
    QString s = !e->ExtendedAck() ?
      tr("No reason provided") :
      myCodec->toUnicode(e->ExtendedAck()->Response());
    QString result = tr("Chat with %1 refused:\n%2")
      .arg(u == NULL ? QString(myUsers.front().c_str()) : QString::fromUtf8(u->GetAlias()))
      .arg(s);
    if (u != NULL)
      gUserManager.DropUser(u);
    InformUser(this, result);
  }
  else
  {
    CEventChat* c = dynamic_cast<CEventChat*>(e->UserEvent());
    if (c->Port() == 0)  // If we requested a join, no need to do anything
    {
      ChatDlg* chatDlg = new ChatDlg(myUsers.front().c_str(), myPpid);
      chatDlg->StartAsClient(e->ExtendedAck()->Port());
    }
  }

  return true;
}

void UserSendChatEvent::resetSettings()
{
  mleSend->clear();
  edtItem->clear();
  mleSend->setFocus();
  slotMassMessageToggled(false);
}

void UserSendChatEvent::inviteUser()
{
  if (myChatPort == 0)
  {
    if (ChatDlg::chatDlgs.size() > 0)
    {
      ChatDlg* chatDlg = NULL;
      JoinChatDlg* j = new JoinChatDlg(true, this);
      if (j->exec() && (chatDlg = j->JoinedChat()) != NULL)
      {
        edtItem->setText(j->ChatClients());
        myChatPort = chatDlg->LocalPort();
        myChatClients = chatDlg->ChatName() + ", " + chatDlg->ChatClients();
      }
      delete j;
      btnBrowse->setText(tr("Clear"));
    }
  }
  else
  {
    myChatPort = 0;
    myChatClients = "";
    edtItem->setText("");
    btnBrowse->setText(tr("Invite"));
  }
}

void UserSendChatEvent::send()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  connect(mleSend, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, false, myConvoId);

  unsigned long icqEventTag;

  if (myChatPort == 0)
    //TODO in daemon
    icqEventTag = gLicqDaemon->icqChatRequest(
        strtoul(myUsers.front().c_str(), NULL, 10),
        myCodec->fromUnicode(mleSend->toPlainText()),
        chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
        chkSendServer->isChecked());
  else
    icqEventTag = gLicqDaemon->icqMultiPartyChatRequest(
        strtoul(myUsers.front().c_str(), NULL, 10),
        myCodec->fromUnicode(mleSend->toPlainText()),
        myCodec->fromUnicode(myChatClients),
        myChatPort,
        chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
        chkSendServer->isChecked());

  myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}
