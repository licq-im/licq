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

#include "usersendsmsevent.h"

#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <QTextCodec>
#include <QTimer>

#include <licq_icqd.h>

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"

#include "widgets/infofield.h"
#include "widgets/mledit.h"

#include "usereventtabdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendSmsEvent */

UserSendSmsEvent::UserSendSmsEvent(QString id, unsigned long ppid, QWidget* parent)
  : UserSendCommon(SmsEvent, id, ppid, parent, "UserSendSmsEvent")
{
  mySendServerCheck->setChecked(true);
  mySendServerCheck->setEnabled(false);
  myUrgentCheck->setChecked(false);
  myUrgentCheck->setEnabled(false);
  myMassMessageCheck->setChecked(false);
  myMassMessageCheck->setEnabled(false);
  myForeColor->setEnabled(false);
  myBackColor->setEnabled(false);
  myEncoding->setEnabled(false); // SMSs are always UTF-8

  myMainWidget->addWidget(myViewSplitter);
  myMessageEdit->setFocus();

  QHBoxLayout* h_lay = new QHBoxLayout();
  myMainWidget->addLayout(h_lay);
  myNumberLabel = new QLabel(tr("Phone : "));
  h_lay->addWidget(myNumberLabel);
  myNumberField = new InfoField(false);
  h_lay->addWidget(myNumberField);
  myNumberField->setFixedWidth(qMax(140, myNumberField->sizeHint().width()));
  h_lay->addStretch(1);
  myCountLabel = new QLabel(tr("Chars left : "));
  h_lay->addWidget(myCountLabel);
  myCountField = new InfoField(false);
  h_lay->addWidget(myCountField);
  myCountField->setFixedWidth(40);
  myCountField->setAlignment(Qt::AlignCenter);
  count();
  connect(myMessageEdit, SIGNAL(textChanged()), SLOT(count()));

  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_W);
  if (u != NULL)
  {
    myNumberField->setText(myCodec->toUnicode(u->GetCellularNumber()));
    gUserManager.DropUser(u);
  }

  myBaseTitle += tr(" - SMS");

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
  myEventTypeGroup->actions().at(SmsEvent)->setChecked(true);
}

UserSendSmsEvent::~UserSendSmsEvent()
{
  // Empty
}

bool UserSendSmsEvent::sendDone(ICQEvent* /* e */)
{
  return true;
}

void UserSendSmsEvent::resetSettings()
{
  myMessageEdit->clear();
  myMessageEdit->setFocus();
  massMessageToggled(false);
}

void UserSendSmsEvent::send()
{
  // Take care of typing notification now
  mySendTypingTimer->stop();
  connect(myMessageEdit, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, false, myConvoId);

  unsigned long icqEventTag = 0;
  if (myEventTag.size())
    icqEventTag = myEventTag.front();

  // do nothing if a command is already being processed
  if (icqEventTag != 0)
    return;

  if (!myMessageEdit->document()->isModified() &&
      !QueryYesNo(this, tr("You didn't edit the SMS.\nDo you really want to send it?")))
    return;

  // don't let the user send empty messages
  if (myMessageEdit->toPlainText().trimmed().isEmpty())
    return;

  //TODO in daemon
  icqEventTag = gLicqDaemon->icqSendSms(myUsers.front().c_str(), LICQ_PPID,
      myNumberField->text().toLatin1(),
      myMessageEdit->toPlainText().toUtf8().data());
  myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}

void UserSendSmsEvent::count()
{
  int len = 160 - strlen(myMessageEdit->toPlainText().toUtf8().data());
  myCountField->setText((len >= 0) ? len : 0);
}
