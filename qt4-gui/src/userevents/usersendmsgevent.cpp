// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2009 Licq developers
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

#include <licq_icqd.h>
#include <licq_translate.h>

#include "config/chat.h"

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"

#include "dialogs/mmsenddlg.h"
#include "dialogs/showawaymsgdlg.h"

#include "widgets/mledit.h"

#include "usereventcommon.h"
#include "usereventtabdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendMsgEvent */

UserSendMsgEvent::UserSendMsgEvent(const UserId& userId, QWidget* parent)
  : UserSendCommon(MessageEvent, userId, parent, "UserSendMsgEvent")
{
  myMainWidget->addWidget(myViewSplitter);
  myMessageEdit->setFocus();
  if (!Config::Chat::instance()->msgChatView())
    myMessageEdit->setMinimumHeight(150);

  myBaseTitle += tr(" - Message");

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
  myEventTypeGroup->actions().at(MessageEvent)->setChecked(true);
}

UserSendMsgEvent::~UserSendMsgEvent()
{
  // Empty
}

bool UserSendMsgEvent::sendDone(const LicqEvent* /* e */)
{
  myMessageEdit->setText(QString::null);

  bool showAwayDlg = false;
  const LicqUser* u = gUserManager.fetchUser(myUsers.front());
  if (u != NULL)
  {
    showAwayDlg = u->Away() && u->ShowAwayMsg();
    gUserManager.DropUser(u);
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
  gLicqDaemon->sendTypingNotification(myUsers.front(), false, myConvoId);

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

  const LicqUser* u = gUserManager.fetchUser(myUsers.front());
  bool userOffline = true;
  if (u != NULL)
  {
    userOffline = u->StatusOffline();
    gUserManager.DropUser(u);
  }

  // create initial strings (implicit copying, no allocation impact :)
  char* tmp = gTranslator.NToRN(myCodec->fromUnicode(myMessageEdit->toPlainText()));
  QByteArray wholeMessageRaw(tmp);
  delete [] tmp;
  int wholeMessagePos = 0;

  bool needsSplitting = false;
  // If we send through server (= have message limit), and we've crossed the limit
  unsigned short maxSize = userOffline ? MAX_OFFLINE_MESSAGE_SIZE : MAX_MESSAGE_SIZE;
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
      messageRaw = wholeMessageRaw.mid(wholeMessagePos, maxSize);
      tmp = gTranslator.RNToN(messageRaw);
      messageRaw = tmp;
      delete [] tmp;
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
      connect(m, SIGNAL(eventSent(const LicqEvent*)), SIGNAL(eventSent(const LicqEvent*)));
      m->go_message(message);
    }

    icqEventTag = gLicqDaemon->sendMessage(
        myUsers.front(),
        messageRaw.data(),
        mySendServerCheck->isChecked(),
        myUrgentCheck->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
        myMassMessageCheck->isChecked(),
        &myIcqColor,
        myConvoId);
    if (LicqUser::getUserProtocolId(myUsers.front()) == LICQ_PPID)
      myEventTag.push_back(icqEventTag);

    tmp = gTranslator.NToRN(messageRaw);
    wholeMessagePos += strlen(tmp);
    delete [] tmp;
  }

  UserSendCommon::send();
}
