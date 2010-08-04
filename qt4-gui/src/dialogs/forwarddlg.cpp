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
#include <licq/icqdefines.h>
#include <licq/userevents.h>

#include "core/gui-defines.h"
#include "core/messagebox.h"

#include "helpers/support.h"

#include "userevents/usersendmsgevent.h"
#include "userevents/usersendurlevent.h"

#include "widgets/infofield.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ForwardDlg */

ForwardDlg::ForwardDlg(Licq::UserEvent* e, QWidget* p)
  : QDialog(p, Qt::Window)
{
  Support::setWidgetProps(this, "UserForwardDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  m_nEventType = e->SubCommand();

  QString t;
  switch (e->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      t = tr("Message");
      s1 = QString::fromLocal8Bit(dynamic_cast<Licq::EventMsg*>(e)->message().c_str());
      break;
    case ICQ_CMDxSUB_URL:
      t = tr("URL");
      s1 = QString::fromLocal8Bit(dynamic_cast<Licq::EventUrl*>(e)->url().c_str());
      s2 = QString::fromLocal8Bit(dynamic_cast<Licq::EventUrl*>(e)->description().c_str());
      break;
    default:
      WarnUser(this, tr("Unable to forward this message type (%d).")
          .arg(e->SubCommand()));
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

  switch(m_nEventType)
  {
    case ICQ_CMDxSUB_MSG:
    {
      s1.prepend(tr("Forwarded message:\n"));
      UserSendMsgEvent* e = new UserSendMsgEvent(myUserId);
      e->setText(s1);
      e->show();
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      s1.prepend(tr("Forwarded URL:\n"));
      UserSendUrlEvent* e = new UserSendUrlEvent(myUserId);
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
      unsigned long ppid = owner->ppid();
      char ppidStr[5];
      Licq::protocolId_toStr(ppidStr, ppid);
      if (text.startsWith(ppidStr))
      {
        nPPID = ppid;
        break;
      }
    }
  }

  if (nPPID == 0 || text.length() <= 4)
    return;

  myUserId = Licq::UserId(text.toLatin1().data(), nPPID);

  Licq::UserReadGuard u(myUserId);
  if (!u.isLocked())
    return;

  edtUser->setText(QString::fromUtf8(u->GetAlias()) + " (" + u->accountId().c_str() + ")");
}
