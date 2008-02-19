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

#include "core/licqgui.h"
#include "core/messagebox.h"

#include "widgets/infofield.h"
#include "widgets/mledit.h"

#include "usereventtabdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::UserSendSmsEvent */

UserSendSmsEvent::UserSendSmsEvent(QString id, unsigned long ppid, QWidget* parent)
  : UserSendCommon(ET_SMS, id, ppid, parent, "UserSendSmsEvent")
{
  chkSendServer->setChecked(true);
  chkSendServer->setEnabled(false);
  chkUrgent->setChecked(false);
  chkUrgent->setEnabled(false);
  chkMass->setChecked(false);
  chkMass->setEnabled(false);
  myForeColor->setEnabled(false);
  myBackColor->setEnabled(false);
  myEncoding->setEnabled(false); // SMSs are always UTF-8

  mainWidget->addWidget(splView);
  mleSend->setFocus();

  QHBoxLayout* h_lay = new QHBoxLayout();
  mainWidget->addLayout(h_lay);
  lblNumber = new QLabel(tr("Phone : "));
  h_lay->addWidget(lblNumber);
  nfoNumber = new InfoField(false);
  h_lay->addWidget(nfoNumber);
  nfoNumber->setFixedWidth(qMax(140, nfoNumber->sizeHint().width()));
  h_lay->addStretch(1);
  lblCount = new QLabel(tr("Chars left : "));
  h_lay->addWidget(lblCount);
  nfoCount = new InfoField(false);
  h_lay->addWidget(nfoCount);
  nfoCount->setFixedWidth(40);
  nfoCount->setAlignment(Qt::AlignCenter);
  count();
  connect(mleSend, SIGNAL(textChanged()), SLOT(count()));

  ICQUser* u = gUserManager.FetchUser(myUsers.front().c_str(), myPpid, LOCK_W);
  if (u != NULL)
  {
    nfoNumber->setText(myCodec->toUnicode(u->GetCellularNumber()));
    gUserManager.DropUser(u);
  }

  myBaseTitle += tr(" - SMS");

  UserEventTabDlg* tabDlg = LicqGui::instance()->userEventTabDlg();
  if (tabDlg != NULL && tabDlg->tabIsSelected(this))
    tabDlg->setWindowTitle(myBaseTitle);

  setWindowTitle(myBaseTitle);
  grpSendType->actions().at(ET_SMS)->setChecked(true);
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
  mleSend->clear();
  mleSend->setFocus();
  slotMassMessageToggled(false);
}

void UserSendSmsEvent::send()
{
  // Take care of typing notification now
  tmrSendTyping->stop();
  connect(mleSend, SIGNAL(textChanged()), SLOT(messageTextChanged()));
  gLicqDaemon->ProtoTypingNotification(myUsers.front().c_str(), myPpid, false, myConvoId);

  unsigned long icqEventTag = 0;
  if (myEventTag.size())
    icqEventTag = myEventTag.front();

  // do nothing if a command is already being processed
  if (icqEventTag != 0)
    return;

  if (!mleSend->document()->isModified() &&
      !QueryYesNo(this, tr("You didn't edit the SMS.\nDo you really want to send it?")))
    return;

  // don't let the user send empty messages
  if (mleSend->toPlainText().trimmed().isEmpty())
    return;

  //TODO in daemon
  icqEventTag = gLicqDaemon->icqSendSms(
      nfoNumber->text().toLatin1(),
      mleSend->toPlainText().toUtf8().data(),
      strtoul(myUsers.front().c_str(), NULL, 10));
  myEventTag.push_back(icqEventTag);

  UserSendCommon::send();
}

void UserSendSmsEvent::count()
{
  int len = 160 - strlen(mleSend->toPlainText().toUtf8().data());
  nfoCount->setText((len >= 0) ? len : 0);
}
