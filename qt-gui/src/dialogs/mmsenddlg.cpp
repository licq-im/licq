/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2013 Licq developers <licq-dev@googlegroups.com>
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
#include <QVBoxLayout>

#include <licq/contactlist/user.h>
#include <licq/event.h>
#include <licq/icq/icq.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/protocolmanager.h>
#include <licq/protocolsignal.h>
#include <licq/translator.h>
#include <licq/userevents.h>

#include "core/signalmanager.h"

#include "helpers/support.h"

#include "views/mmuserview.h"

using Licq::StringList;
using Licq::gProtocolManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::MMSendDlg */

MMSendDlg::MMSendDlg(MMUserView* _mmv, QWidget* p)
  : QDialog(p),
    mmv(_mmv),
    icqEventTag(0)
{
  Support::setWidgetProps(this, "MMSendDialog");
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
  connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
      SLOT(slot_done(const Licq::Event*)));

  barSend->setMaximum(mmv->contacts().size());
  barSend->setValue(0);

  setMinimumWidth(300);
}

int MMSendDlg::go_message(const QString& msg)
{
  myEventType = Licq::UserEvent::TypeMessage;
  s1 = msg;

  setWindowTitle(tr("Multiple Recipient Message"));

  // Start
  SendNext();
  exec();
  return 0;
}

int MMSendDlg::go_url(const QString& url, const QString& desc)
{
  myEventType = Licq::UserEvent::TypeUrl;
  s1 = desc;
  s2 = url;

  setWindowTitle(tr("Multiple Recipient URL"));

  // Start
  SendNext();
  return exec();
}

int MMSendDlg::go_contact(StringList& users)
{
  myEventType = Licq::UserEvent::TypeContactList;
  myUsers = &users;

  setWindowTitle(tr("Multiple Recipient Contact List"));

  // Start
  SendNext();
  return exec();
}

void MMSendDlg::slot_done(const Licq::Event* e)
{
  if ( !e->Equals(icqEventTag) )
    return;

  bool isOk = (e != NULL ? (e->Result() == Licq::Event::ResultAcked) : (icqEventTag == 0));

  icqEventTag = 0;

  if (!isOk)
  {
    grpSending->setTitle(grpSending->title() + tr("failed"));
    btnCancel->setText(tr("&Close"));
    return;
  }

  if (e != NULL)
  {
    // Let other dialogs know a message was sent
    emit eventSent(e);
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

  Licq::UserId userId = *mmv->contacts().begin();

  if (!userId.isValid())
    return;

  switch (myEventType)
  {
    case Licq::UserEvent::TypeMessage:
    {
      {
        Licq::UserReadGuard u(userId);
        if (!u.isLocked())
          return;
        grpSending->setTitle(tr("Sending mass message to %1...")
            .arg(QString::fromUtf8(u->getAlias().c_str())));
      }

      // create initial strings (implicit copying, no allocation impact :)
      QByteArray wholeMessageRaw(Licq::gTranslator.returnToDos(s1.toUtf8().data()).c_str());
      int wholeMessagePos = 0;

      bool needsSplitting = false;
      // If we send through server (= have message limit), and we've crossed the limit
      if ((wholeMessageRaw.length() - wholeMessagePos) > Licq::IcqProtocol::MaxMessageSize)
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
          messageRaw = Licq::gTranslator.returnToUnix(wholeMessageRaw.mid(wholeMessagePos,
              Licq::IcqProtocol::MaxMessageSize).data()).c_str();
          message = QString::fromUtf8(messageRaw);

          if ((wholeMessageRaw.length() - wholeMessagePos) > Licq::IcqProtocol::MaxMessageSize)
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
              messageRaw = message.toUtf8();
            }
          }
        }
        else
        {
          messageRaw = s1.toUtf8();
        }

        icqEventTag = gProtocolManager.sendMessage(userId, messageRaw.data(),
            Licq::ProtocolSignal::SendToMultiple);

        wholeMessagePos += Licq::gTranslator.returnToDos(messageRaw.constData()).size();
      }

      break;
    }
    case Licq::UserEvent::TypeUrl:
    {
      {
        Licq::UserReadGuard u(userId);
        if (!u.isLocked())
          return;
        grpSending->setTitle(tr("Sending mass URL to %1...")
            .arg(QString::fromUtf8(u->getAlias().c_str())));
      }

      icqEventTag = gProtocolManager.sendUrl(userId, s2.toUtf8().constData(),
          s1.toUtf8().constData(), Licq::ProtocolSignal::SendToMultiple);
      break;
    }
    case Licq::UserEvent::TypeContactList:
    {
      Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
          Licq::gPluginManager.getProtocolInstance(userId.ownerId()));
      if (!icq)
        return;

      {
        Licq::UserReadGuard u(userId);
        if (!u.isLocked())
          return;
        grpSending->setTitle(tr("Sending mass list to %1...")
            .arg(QString::fromUtf8(u->getAlias().c_str())));
      }

      icqEventTag = icq->icqSendContactList(userId, *myUsers);
      break;
    }
  }

  if (icqEventTag == 0) slot_done(NULL);
}

MMSendDlg::~MMSendDlg()
{
  if (icqEventTag != 0)
  {
    gProtocolManager.cancelEvent(*mmv->contacts().begin(), icqEventTag);
    icqEventTag = 0;
  }
}

void MMSendDlg::slot_cancel()
{
  if (icqEventTag != 0)
  {
    gProtocolManager.cancelEvent(*mmv->contacts().begin(), icqEventTag);
    icqEventTag = 0;
  }
  //disconnect(sigman, SIGNAL(doneUserFcn(const Licq::Event*)), SLOT(slot_done(const Licq::Event*)));

  reject();
}
