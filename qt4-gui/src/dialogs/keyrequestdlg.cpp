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

// written by Graham Roff <graham@licq.org>
// -----------------------------------------------------------------------------

#include "keyrequestdlg.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <licq/contactlist/user.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/protocolmanager.h>
#include <licq/userevents.h>

#include "core/signalmanager.h"

#include "helpers/support.h"

using Licq::gProtocolManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::KeyRequestDlg */

KeyRequestDlg::KeyRequestDlg(const Licq::UserId& userId, QWidget* parent)
  : QDialog(parent),
    myUserId(userId),
    myIcqEventTag(0)
{
  Support::setWidgetProps(this, "KeyRequestDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  Licq::UserReadGuard u(myUserId);
  setWindowTitle(tr("Licq - Secure Channel with %1")
      .arg(QString::fromUtf8(u->GetAlias())));

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  QString t1 = tr("Secure channel is established using SSL\n"
                  "with Diffie-Hellman key exchange and\n"
                  "the TLS version 1 protocol.\n\n");
  QString t2;
  switch (u->SecureChannelSupport())
  {
    case Licq::SECURE_CHANNEL_SUPPORTED:
      t2 = tr("The remote uses Licq %1/SSL.")
        .arg(Licq::UserEvent::licqVersionToString(u->LicqVersion()).c_str());
      if (Licq::gDaemon.haveCryptoSupport())
        QTimer::singleShot(0, this, SLOT(startSend()));
      break;

    case Licq::SECURE_CHANNEL_NOTSUPPORTED:
      t2 = tr("The remote uses Licq %1, however it\n"
              "has no secure channel support compiled in.\n"
              "This probably won't work.")
        .arg(Licq::UserEvent::licqVersionToString(u->LicqVersion()).c_str());
      break;

    default:
      t2 = tr("This only works with other Licq clients >= v0.85\n"
              "The remote doesn't seem to use such a client.\n"
              "This might not work.");
      break;
  }

  QLabel* lbl = new QLabel(t1 + t2);
  top_lay->addWidget(lbl);

  lblStatus = new QLabel();
  lblStatus->setFrameStyle(QLabel::Box | QLabel::Sunken);
  lblStatus->setAlignment(Qt::AlignHCenter);
  top_lay->addWidget(lblStatus);

  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close);

  btnSend = buttons->addButton(tr("&Send"), QDialogButtonBox::ActionRole);

  connect(btnSend, SIGNAL(clicked()), SLOT(startSend()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  top_lay->addWidget(buttons);

  if (Licq::gDaemon.haveCryptoSupport())
  {
    myOpen = !u->Secure();
    if (u->Secure())
      lblStatus->setText(tr("Ready to close channel"));
    else
      lblStatus->setText(tr("Ready to request channel"));
  }
  else
  {
    lblStatus->setText(tr("Client does not support OpenSSL.\n"
                          "Rebuild Licq with OpenSSL support."));
    btnSend->setEnabled(false);
  }

  show();
}

KeyRequestDlg::~KeyRequestDlg()
{
  if (myIcqEventTag != 0)
  {
    Licq::gDaemon.cancelEvent(myIcqEventTag);
    myIcqEventTag = 0;
  }
}

void KeyRequestDlg::startSend()
{
  connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
      SLOT(doneEvent(const Licq::Event*)));
  btnSend->setEnabled(false);

  if (myOpen)
  {
    lblStatus->setText(tr("Requesting secure channel..."));
    QTimer::singleShot(100, this, SLOT(openConnection()));
  }
  else
  {
    lblStatus->setText(tr("Closing secure channel..."));
    QTimer::singleShot(100, this, SLOT(closeConnection()));
  }
}

void KeyRequestDlg::openConnection()
{
  myIcqEventTag = gProtocolManager.secureChannelOpen(myUserId);
}

void KeyRequestDlg::closeConnection()
{
  myIcqEventTag = gProtocolManager.secureChannelClose(myUserId);
}

void KeyRequestDlg::doneEvent(const Licq::Event* e)
{
  if (!e->Equals(myIcqEventTag))
    return;

  QString result = "<center><font color=\"|\">#</font></center>";
  QString color, text;
  if (e == NULL)
  {
    color = "yellow";
    if (myOpen)
      text = tr("Secure channel already established.");
    else
      text = tr("Secure channel not established.");

    btnSend->setEnabled(false);
  }
  else
  {
    color = "red";
    switch (e->Result())
    {
      case Licq::Event::ResultFailed:
        text = tr("Remote client does not support OpenSSL.");
        break;
      case Licq::Event::ResultError: // could not connect to remote host (or out of memory)
        text = tr("Could not connect to remote client.");
        break;
      case Licq::Event::ResultSuccess:
        if (myOpen)
        {
          color = "ForestGreen";
          text = tr("Secure channel established.");
        }
        else
        {
          color = "blue";
          text = tr("Secure channel closed.");
        }
        break;
      default:
        text = tr("Unknown state.");
        break;
    }
    if (e->Result() == Licq::Event::ResultSuccess)
    {
      btnSend->setEnabled(false);
      QTimer::singleShot(500, this, SLOT(close()));
    }
    else
      btnSend->setEnabled(true);
  }

  result.replace('|', color);
  result.replace('#', text);
  lblStatus->setText(result);

  myIcqEventTag = 0;
}
