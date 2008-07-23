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

#include "usersendurlevent.h"

#include <QAction>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QPushButton>
#include <QSplitter>
#include <QTextCodec>
#include <QTimer>
#include <QVBoxLayout>

#include <licq_icqd.h>

#include "config/chat.h"

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"

#include "dialogs/mmsenddlg.h"
#include "dialogs/showawaymsgdlg.h"

#include "widgets/infofield.h"
#include "widgets/mledit.h"

#include "usereventtabdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendUrlEvent */

UserSendUrlEvent::UserSendUrlEvent(QString id, unsigned long ppid, QWidget* parent)
  : UserSendCommon(UrlEvent, id, ppid, parent, "UserSendUrlEvent")
{
  myMainWidget->addWidget(myViewSplitter);
  myMessageEdit->setFocus();

  QHBoxLayout* h_lay = new QHBoxLayout();
  myMainWidget->addLayout(h_lay);
  myUrlLabel = new QLabel(tr("URL : "));
  h_lay->addWidget(myUrlLabel);
  myUrlEdit = new InfoField(false);
  h_lay->addWidget(myUrlEdit);
  myUrlEdit->installEventFilter(this);

  myBaseTitle += tr(" - URL");

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
  myEventTypeGroup->actions().at(UrlEvent)->setChecked(true);
}

UserSendUrlEvent::~UserSendUrlEvent()
{
  // Empty
}

bool UserSendUrlEvent::eventFilter(QObject* watched, QEvent* e)
{
  if (watched == myUrlEdit)
  {
    if (e->type() == QEvent::KeyPress)
    {
      QKeyEvent* key = dynamic_cast<QKeyEvent*>(e);
      const bool isEnter = (key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return);
      if (isEnter && (Config::Chat::instance()->singleLineChatMode() || key->modifiers() & Qt::ControlModifier))
      {
        mySendButton->animateClick();
        return true; // filter the event out
      }
    }
    return false;
  }
  else
    return UserSendCommon::eventFilter(watched, e);
}

void UserSendUrlEvent::setUrl(const QString& url, const QString& description)
{
  myUrlEdit->setText(url);
  setText(description);
}

bool UserSendUrlEvent::sendDone(ICQEvent* e)
{
  if (e->Command() != ICQ_CMDxTCP_START)
    return true;

  bool showAwayDlg = false;
  const ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_R);
  if (u != NULL)
  {
    showAwayDlg = u->Away() && u->ShowAwayMsg();
    gUserManager.DropUser(u);
  }

  if (showAwayDlg && Config::Chat::instance()->popupAutoResponse())
    new ShowAwayMsgDlg(myUsers.front().c_str(), myPpid);

  return true;
}

void UserSendUrlEvent::resetSettings()
{
  myMessageEdit->clear();
  myUrlEdit->clear();
  myMessageEdit->setFocus();
  massMessageToggled(false);
}

void UserSendUrlEvent::send()
{
  // Take care of typing notification now
  mySendTypingTimer->stop();
  connect(myMessageEdit, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, false, myConvoId);

  if (myUrlEdit->text().trimmed().isEmpty())
  {
    InformUser(this, tr("No URL specified"));
    return;
  }

  if (!checkSecure())
    return;

  if (myMassMessageCheck->isChecked())
  {
    MMSendDlg* m = new MMSendDlg(myMassMessageList, this);
    int r = m->go_url(myUrlEdit->text(), myMessageEdit->toPlainText());
    delete m;
    if (r != QDialog::Accepted)
      return;
  }

  unsigned long icqEventTag;
  icqEventTag = gLicqDaemon->ProtoSendUrl(
      myUsers.front().c_str(),
      myPpid,
      myUrlEdit->text().toLatin1(),
      myCodec->fromUnicode(myMessageEdit->toPlainText()),
      mySendServerCheck->isChecked() ? false : true,
      myUrgentCheck->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
      myMassMessageCheck->isChecked(),
      &myIcqColor);

  myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}
