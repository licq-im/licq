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
  : UserSendCommon(ET_URL, id, ppid, parent, "UserSendUrlEvent")
{
  mainWidget->addWidget(splView);
  mleSend->setFocus();

  QHBoxLayout* h_lay = new QHBoxLayout();
  mainWidget->addLayout(h_lay);
  lblItem = new QLabel(tr("URL : "));
  h_lay->addWidget(lblItem);
  edtItem = new InfoField(false);
  h_lay->addWidget(edtItem);
  edtItem->installEventFilter(this);

  myBaseTitle += tr(" - URL");

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
  grpSendType->actions().at(ET_URL)->setChecked(true);
}

UserSendUrlEvent::~UserSendUrlEvent()
{
  // Empty
}

bool UserSendUrlEvent::eventFilter(QObject* watched, QEvent* e)
{
  if (watched == edtItem)
  {
    if (e->type() == QEvent::KeyPress)
    {
      QKeyEvent* key = dynamic_cast<QKeyEvent*>(e);
      const bool isEnter = (key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return);
      if (isEnter && (Config::Chat::instance()->singleLineChatMode() || key->modifiers() & Qt::ControlModifier))
      {
        btnSend->animateClick();
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
  edtItem->setText(url);
  setText(description);
}

bool UserSendUrlEvent::sendDone(ICQEvent* e)
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

void UserSendUrlEvent::resetSettings()
{
  mleSend->clear();
  edtItem->clear();
  mleSend->setFocus();
  slotMassMessageToggled(false);
}

void UserSendUrlEvent::send()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  connect(mleSend, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, false, myConvoId);

  if (edtItem->text().trimmed().isEmpty())
  {
    InformUser(this, tr("No URL specified"));
    return;
  }

  if (!checkSecure())
    return;

  if (chkMass->isChecked())
  {
    MMSendDlg* m = new MMSendDlg(lstMultipleRecipients, this);
    int r = m->go_url(edtItem->text(), mleSend->toPlainText());
    delete m;
    if (r != QDialog::Accepted)
      return;
  }

  unsigned long icqEventTag;
  icqEventTag = gLicqDaemon->ProtoSendUrl(
      myUsers.front().c_str(),
      myPpid,
      edtItem->text().toLatin1(),
      myCodec->fromUnicode(mleSend->toPlainText()),
      chkSendServer->isChecked() ? false : true,
      chkUrgent->isChecked() ? ICQ_TCPxMSG_URGENT : ICQ_TCPxMSG_NORMAL,
      chkMass->isChecked(),
      &icqColor);

  myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}
