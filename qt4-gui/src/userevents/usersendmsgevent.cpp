// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#include "usersendmsgevent.h"

#include <QAction>
#include <QActionGroup>
#include <QSplitter>
#include <QTextCodec>
#include <QTimer>
#include <QVBoxLayout>

#include <licq/contactlist/user.h>
#include <licq/icq.h>
#include <licq/icqdefines.h>
#include <licq/protocolmanager.h>
#include <licq/translator.h>

#include "config/chat.h"

#include "core/gui-defines.h"
#include "core/messagebox.h"

#include "dialogs/mmsenddlg.h"
#include "dialogs/showawaymsgdlg.h"

#include "widgets/mledit.h"

using Licq::gProtocolManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendMsgEvent */

UserSendMsgEvent::UserSendMsgEvent(const Licq::UserId& userId, QWidget* parent)
  : UserSendCommon(MessageEvent, userId, parent, "UserSendMsgEvent")
{
  myMainWidget->addWidget(myViewSplitter);
  myMessageEdit->setFocus();
  if (!Config::Chat::instance()->msgChatView())
    myMessageEdit->setMinimumHeight(150);

  myBaseTitle += tr(" - Message");

  setWindowTitle(myBaseTitle);
  myEventTypeGroup->actions().at(MessageEvent)->setChecked(true);
}

UserSendMsgEvent::~UserSendMsgEvent()
{
  // Empty
}

bool UserSendMsgEvent::sendDone(const Licq::Event* /* e */)
{
  myMessageEdit->setText(QString::null);

  bool showAwayDlg = false;
  {
    Licq::UserReadGuard u(myUsers.front());
    if (u.isLocked())
      showAwayDlg = u->Away() && u->ShowAwayMsg();
  }

  if (showAwayDlg && Config::Chat::instance()->popupAutoResponse())
    new ShowAwayMsgDlg(myUsers.front());

  return true;
}

void UserSendMsgEvent::resetSettings()
{
  myMessageEdit->clear();
  myMessageEdit->setFocus();

  // Makes the cursor blink so that the user sees that the text edit has focus.
  myMessageEdit->moveCursor(QTextCursor::Start);

  massMessageToggled(false);
}

void UserSendMsgEvent::send()
{
  // Take care of typing notification now
  if (mySendTypingTimer->isActive())
    mySendTypingTimer->stop();
  connect(myMessageEdit, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  gProtocolManager.sendTypingNotification(myUsers.front(), false, myConvoId);

  // do nothing if a command is already being processed
  unsigned long icqEventTag = 0;
  if (myEventTag.size())
    icqEventTag = myEventTag.front();

  if (icqEventTag != 0)
    return;

  if (!myMessageEdit->document()->isModified() &&
      !QueryYesNo(this, tr("You didn't edit the message.\nDo you really want to send it?")))
    return;

  // don't let the user send empty messages
  if (myMessageEdit->toPlainText().trimmed().isEmpty())
    return;

  if (!checkSecure())
    return;

  bool userOffline = true;
  {
    Licq::UserReadGuard u(myUsers.front());
    if (u.isLocked())
      userOffline = !u->isOnline();
  }

  // create initial strings (implicit copying, no allocation impact :)
  QByteArray wholeMessageRaw(Licq::gTranslator.returnToDos(myCodec->fromUnicode(myMessageEdit->toPlainText()).data()).c_str());
  int wholeMessagePos = 0;

  bool needsSplitting = false;
  // If we send through server (= have message limit), and we've crossed the limit
  unsigned short maxSize = userOffline ? CICQDaemon::MaxOfflineMessageSize : CICQDaemon::MaxMessageSize;
  if (mySendServerCheck->isChecked() && ((wholeMessageRaw.length() - wholeMessagePos) > maxSize))
    needsSplitting = true;

  QString message;
  QByteArray messageRaw;

  while (wholeMessageRaw.length() > wholeMessagePos)
  {
    if (needsSplitting)
    {
      // This is a bit ugly but adds safety. We don't simply search
      // for a whitespace to cut at in the encoded text (since we don't
      // really know how spaces are represented in its encoding), so
      // we take the maximum length, then convert back to a Unicode string
      // and then search for Unicode whitespaces.
      messageRaw = Licq::gTranslator.returnToUnix(wholeMessageRaw.mid(wholeMessagePos, maxSize).data()).c_str();
      message = myCodec->toUnicode(messageRaw);

      if (wholeMessageRaw.length() - wholeMessagePos > maxSize)
      {
        // We try to find the optimal place to cut
        // (according to our narrow-minded Latin1 idea of optimal :)
        // prefer keeping sentences intact 1st
        int foundIndex = message.lastIndexOf(QRegExp("[\\.\\n]"));
        // slicing at 0 position would be useless
        if (foundIndex <= 0)
          foundIndex = message.lastIndexOf(QRegExp("\\s"));

        if (foundIndex > 0)
        {
          message.truncate(foundIndex + 1);
          messageRaw = myCodec->fromUnicode(message);
        }
      }
    }
    else
    {
      message = myMessageEdit->toPlainText();
      messageRaw = myCodec->fromUnicode(message);
    }

    if (myMassMessageCheck->isChecked())
    {
      MMSendDlg* m = new MMSendDlg(myMassMessageList, this);
      connect(m, SIGNAL(eventSent(const Licq::Event*)), SIGNAL(eventSent(const Licq::Event*)));
      m->go_message(message);
    }

    icqEventTag = gProtocolManager.sendMessage(
        myUsers.front(),
        messageRaw.data(),
        mySendServerCheck->isChecked(),
        myUrgentCheck->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
        myMassMessageCheck->isChecked(),
        &myIcqColor,
        myConvoId);
    if (icqEventTag != 0)
      myEventTag.push_back(icqEventTag);

    wholeMessagePos += Licq::gTranslator.returnToDos(messageRaw.data()).size();
  }

  UserSendCommon::send();
}
