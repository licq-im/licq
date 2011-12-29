/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
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
// Contributions by Dirk A. Mueller <dirk@licq.org>

#include "forwarddlg.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <QGridLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLabel>
#include <QPushButton>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/user.h>
#include <licq/contactlist/usermanager.h>
#include <licq/userevents.h>

#include "core/gui-defines.h"
#include "core/messagebox.h"

#include "helpers/support.h"

#include "userevents/usersendevent.h"

#include "widgets/infofield.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ForwardDlg */

ForwardDlg::ForwardDlg(Licq::UserEvent* e, QWidget* p)
  : QDialog(p, Qt::Window)
{
  Support::setWidgetProps(this, "UserForwardDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  myEventType = e->eventType();

  QString t;
  switch (e->eventType())
  {
    case Licq::UserEvent::TypeMessage:
      t = tr("Message");
      s1 = QString::fromUtf8(dynamic_cast<Licq::EventMsg*>(e)->message().c_str());
      break;
    case Licq::UserEvent::TypeUrl:
      t = tr("URL");
      s1 = QString::fromUtf8(dynamic_cast<Licq::EventUrl*>(e)->url().c_str());
      s2 = QString::fromUtf8(dynamic_cast<Licq::EventUrl*>(e)->description().c_str());
      break;
    default:
      WarnUser(this, tr("Unable to forward this message type (%s).")
          .arg(e->eventName().c_str()));
      return;
  }

  setWindowTitle(tr("Forward %1 To User").arg(t));
  setAcceptDrops(true);

  QGridLayout* lay = new QGridLayout(this);
  QLabel* lbl = new QLabel(tr("Drag the user to forward to here:"));
  lay->addWidget(lbl, 0, 0, 1, 5);
  edtUser = new InfoField(true);
  edtUser->setAcceptDrops(false);
  lay->addWidget(edtUser, 1, 0, 1, 5);

  lay->setColumnStretch(0, 2);
  btnOk = new QPushButton(tr("&Forward"));
  lay->addWidget(btnOk, 2, 1);

  lay->setColumnMinimumWidth(2, 10);
  btnCancel = new QPushButton(tr("&Cancel"));
  lay->addWidget(btnCancel, 2, 3);
  lay->setColumnStretch(4, 2);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));
}


ForwardDlg::~ForwardDlg()
{
}


void ForwardDlg::slot_ok()
{
  if (!myUserId.isValid())
    return;

  switch (myEventType)
  {
    case Licq::UserEvent::TypeMessage:
    {
      s1.prepend(tr("Forwarded message:\n"));
      UserSendEvent* e = new UserSendEvent(MessageEvent, myUserId);
      e->setText(s1);
      e->show();
      break;
    }
    case Licq::UserEvent::TypeUrl:
    {
      s1.prepend(tr("Forwarded URL:\n"));
      UserSendEvent* e = new UserSendEvent(UrlEvent, myUserId);
      e->setUrl(s2, s1);
      e->show();
      break;
    }
  }

  close();
}



void ForwardDlg::dragEnterEvent(QDragEnterEvent* dee)
{
  if (dee->mimeData()->hasText())
    dee->accept();
}


void ForwardDlg::dropEvent(QDropEvent* de)
{
  QString text = de->mimeData()->text();
  if (text.isEmpty())
    return;

  unsigned long nPPID = 0;

  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(Licq::Owner* owner, **ownerList)
    {
      unsigned long ppid = owner->protocolId();
      if (text.startsWith(Licq::protocolId_toString(ppid).c_str()))
      {
        nPPID = ppid;
        break;
      }
    }
  }

  if (nPPID == 0 || text.length() <= 4)
    return;

  myUserId = Licq::UserId(text.toLatin1().constData(), nPPID);

  Licq::UserReadGuard u(myUserId);
  if (!u.isLocked())
    return;

  edtUser->setText(QString::fromUtf8(u->getAlias().c_str()) +
      " (" + u->accountId().c_str() + ")");
}
