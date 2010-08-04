// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2010 Licq developers
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

#include "customautorespdlg.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/pluginsignal.h>

#include "helpers/support.h"

#include "widgets/mledit.h"

#include "awaymsgdlg.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::CustomAutoRespDlg */

CustomAutoRespDlg::CustomAutoRespDlg(const Licq::UserId& userId, QWidget* parent)
  : QDialog(parent),
    myUserId(userId)
{
  Support::setWidgetProps(this, "CustomAutoResponseDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  QVBoxLayout* lay = new QVBoxLayout(this);

  myMessage = new MLEdit(true);
  myMessage->setSizeHintLines(5);
  connect(myMessage, SIGNAL(ctrlEnterPressed()), SLOT(ok()));
  lay->addWidget(myMessage);

  QDialogButtonBox* buttons = new QDialogButtonBox(
      QDialogButtonBox::Ok |
      QDialogButtonBox::Cancel);
  connect(buttons, SIGNAL(accepted()), SLOT(ok()));
  connect(buttons, SIGNAL(rejected()), SLOT(close()));

  QPushButton* btn;

  btn = buttons->addButton(QDialogButtonBox::Discard);
  btn->setText(tr("Clear"));
  connect(btn, SIGNAL(clicked()), SLOT(clear()));

  btn = buttons->addButton(QDialogButtonBox::Help);
  btn->setText(tr("Hints"));
  connect(btn, SIGNAL(clicked()), SLOT(hints()));

  lay->addWidget(buttons);

  Licq::UserReadGuard u(myUserId);
  if (!u.isLocked())
    return;

  setWindowTitle(tr("Set Custom Auto Response for %1").arg(QString::fromUtf8(u->GetAlias())));
  if (!u->customAutoResponse().empty())
    myMessage->setText(QString::fromLocal8Bit(u->customAutoResponse().c_str()));
  else
  {
    unsigned status = u->statusToUser();
    if (status != Licq::User::OfflineStatus)
      myMessage->setText(tr("I am currently %1.\nYou can leave me a message.")
          .arg(Licq::User::statusToString(status, true, false).c_str()));
  }

  myMessage->setFocus();
  QTimer::singleShot(0, myMessage, SLOT(selectAll()));

  show();
}

void CustomAutoRespDlg::ok()
{
  QString s = myMessage->toPlainText().trimmed();

  {
    Licq::UserWriteGuard u(myUserId);
    if (u.isLocked())
      u->setCustomAutoResponse(s.toLocal8Bit().data());
  }

  // Notify all plugins (including ourselves)
  Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserSettings);

  close();
}

void CustomAutoRespDlg::clear()
{
  {
    Licq::UserWriteGuard u(myUserId);
    if (u.isLocked())
      u->clearCustomAutoResponse();
  }

  // Notify all plugins (including ourselves)
  Licq::gUserManager.notifyUserUpdated(myUserId, Licq::PluginSignal::UserSettings);

  close();
}

void CustomAutoRespDlg::hints()
{
  AwayMsgDlg::showAutoResponseHints(this);
}
