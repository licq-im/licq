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

#include "mmsenddlg.h"

#include "config.h"

#include <stdio.h>
#include <unistd.h>

#include <QByteArray>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QProgressBar>
#include <QPushButton>
#include <QRegExp>
#include <QTextCodec>
#include <QVBoxLayout>

#include <licq_events.h>
#include <licq_icqd.h>
#include <licq_translate.h>
#include <licq_user.h>

#include "core/licqgui.h"
#include "core/signalmanager.h"

#include "helpers/support.h"
#include "helpers/usercodec.h"

#include "views/mmuserview.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::MMSendDlg */

MMSendDlg::MMSendDlg(MMUserView* _mmv, QWidget* p)
  : QDialog(p),
    mmv(_mmv),
    m_nPPID(0),
    icqEventTag(0)
{
  Support::setWidgetProps(this, "MMSendDialog");
  setModal(true);
  setAttribute(Qt::WA_DeleteOnClose, true);

  QVBoxLayout* v = new QVBoxLayout(this);

  grpSending = new QGroupBox();
  QVBoxLayout* laySending = new QVBoxLayout(grpSending);
  barSend = new QProgressBar();
  laySending->addWidget(barSend);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  btnCancel = buttons->addButton(QDialogButtonBox::Cancel);

  v->addWidget(grpSending);
  v->addWidget(buttons);

  connect(btnCancel, SIGNAL(clicked()), SLOT(slot_cancel()));
  connect(LicqGui::instance()->signalManager(),
      SIGNAL(doneUserFcn(ICQEvent*)), SLOT(slot_done(ICQEvent*)));

  barSend->setMaximum(mmv->contacts().count());
  barSend->setValue(0);

  setMinimumWidth(300);
}

int MMSendDlg::go_message(QString msg)
{
  m_nEventType = ICQ_CMDxSUB_MSG;
  s1 = msg;

  setWindowTitle(tr("Multiple Recipient Message"));

  // Start
  SendNext();
  show();
  return 0;
}

int MMSendDlg::go_url(QString url, QString desc)
{
  m_nEventType = ICQ_CMDxSUB_URL;
  s1 = desc;
  s2 = url;

  setWindowTitle(tr("Multiple Recipient URL"));

  // Start
  SendNext();
  show();
  return result();
}

int MMSendDlg::go_contact(StringList& users)
{
  m_nEventType = ICQ_CMDxSUB_CONTACTxLIST;
  myUsers = &users;

  setWindowTitle(tr("Multiple Recipient Contact List"));

  // Start
  SendNext();
  show();
  return result();
}

void MMSendDlg::slot_done(ICQEvent* e)
{
  if ( !e->Equals(icqEventTag) )
    return;

  bool isOk = (e != NULL && e->Result() == EVENT_ACKED);

  icqEventTag = 0;

  if (!isOk)
  {
    grpSending->setTitle(grpSending->title() + tr("failed"));
    btnCancel->setText(tr("&Close"));
    return;
  }

  // Send next message
  barSend->setValue(barSend->value() + 1);
  mmv->removeFirst();

  SendNext();
}

void MMSendDlg::SendNext()
{
  if (mmv->contacts().empty())
  {
    accept();
    return;
  }

  QPair<QString, unsigned long> contact = *mmv->contacts().begin();

  myId = contact.first;
  m_nPPID = contact.second;

  if (myId.isEmpty()) return;

  switch (m_nEventType)
  {
    case ICQ_CMDxSUB_MSG:
    {
      const ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), m_nPPID, LOCK_R);
      if (u == NULL) return;
      QTextCodec* codec = UserCodec::codecForICQUser(u);
      grpSending->setTitle(tr("Sending mass message to %1...").arg(QString::fromUtf8(u->GetAlias())));
      gUserManager.DropUser(u);

      // create initial strings (implicit copying, no allocation impact :)
      char* tmp = gTranslator.NToRN(codec->fromUnicode(s1));
      QByteArray wholeMessageRaw(tmp);
      delete [] tmp;
      int wholeMessagePos = 0;

      bool needsSplitting = false;
      // If we send through server (= have message limit), and we've crossed the limit
      if ((wholeMessageRaw.length() - wholeMessagePos) > MAX_MESSAGE_SIZE)
      {
        needsSplitting = true;
      }

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
          messageRaw = wholeMessageRaw.mid(wholeMessagePos, MAX_MESSAGE_SIZE);
          tmp = gTranslator.RNToN(messageRaw);
          messageRaw = tmp;
          delete [] tmp;
          message = codec->toUnicode(messageRaw);

          if ((wholeMessageRaw.length() - wholeMessagePos) > MAX_MESSAGE_SIZE)
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
              message.truncate(foundIndex);
              messageRaw = codec->fromUnicode(message);
            }
          }
        }
        else
        {
          messageRaw = codec->fromUnicode(s1);
        }

        icqEventTag = gLicqDaemon->ProtoSendMessage(
            myId.toLatin1(), m_nPPID, messageRaw.data(), false, ICQ_TCPxMSG_NORMAL, true);

        tmp = gTranslator.NToRN(messageRaw);
        wholeMessagePos += strlen(tmp);
        delete [] tmp;
      }

      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      const ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), m_nPPID, LOCK_R);
      if (u == NULL) return;
      grpSending->setTitle(tr("Sending mass URL to %1...").arg(QString::fromUtf8(u->GetAlias())));
      QTextCodec* codec = UserCodec::codecForICQUser(u);
      gUserManager.DropUser(u);

      icqEventTag = gLicqDaemon->ProtoSendUrl(
          myId.toLatin1(), m_nPPID, s2.toLatin1(),
          codec->fromUnicode(s1), false, ICQ_TCPxMSG_NORMAL, true);
      break;
    }
    case ICQ_CMDxSUB_CONTACTxLIST:
    {
      const ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), m_nPPID, LOCK_R);
      if (u == NULL) return;
      grpSending->setTitle(tr("Sending mass list to %1...").arg(QString::fromUtf8(u->GetAlias())));
      gUserManager.DropUser(u);

      icqEventTag = gLicqDaemon->icqSendContactList(
          myId.toLatin1(), *myUsers, false, ICQ_TCPxMSG_NORMAL);
      break;
    }
  }

  if (icqEventTag == 0) slot_done(NULL);
}

MMSendDlg::~MMSendDlg()
{
  if (icqEventTag != 0)
  {
    gLicqDaemon->CancelEvent(icqEventTag);
    icqEventTag = 0;
  }
}

void MMSendDlg::slot_cancel()
{
  if (icqEventTag != 0)
  {
    gLicqDaemon->CancelEvent(icqEventTag);
    icqEventTag = 0;
  }
  //disconnect(sigman, SIGNAL(doneUserFcn(ICQEvent*)), SLOT(slot_done(ICQEvent*)));

  reject();
}
