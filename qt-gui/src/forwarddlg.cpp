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

// written by Graham Roff <graham@licq.org>
// Contributions by Dirk A. Mueller <dirk@licq.org>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qlayout.h>
#include <qdragobject.h>

#include "forwarddlg.h"
#include "mainwin.h"
#include "gui-defines.h"
#include "ewidgets.h"
#include "licq_user.h"
#include "usereventdlg.h"
#include "ewidgets.h"
#include "licq_icqd.h"


CForwardDlg::CForwardDlg(CSignalManager *sigMan, CUserEvent *e, QWidget *p)
  : LicqDialog(p, "UserForwardDialog", false, WDestructiveClose | WType_TopLevel)
{
  sigman = sigMan;

  m_nEventType = e->SubCommand();
  m_szId = 0;
  m_nPPID = 0;

  QString t;
  switch (e->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      t = tr("Message");
      s1 = QString::fromLocal8Bit(((CEventMsg *)e)->Message());
      break;
    case ICQ_CMDxSUB_URL:
      t = tr("URL");
      s1 = QString::fromLocal8Bit(((CEventUrl *)e)->Url());
      s2 = QString::fromLocal8Bit(((CEventUrl *)e)->Description());
      break;
    default:
      WarnUser(this, tr("Unable to forward this message type (%d).").arg(e->SubCommand()));
      return;
  }

  setCaption(tr("Forward %1 To User").arg(t));
  setAcceptDrops(true);

  QGridLayout *lay = new QGridLayout(this, 3, 5, 10, 5);
  QLabel *lbl = new QLabel(tr("Drag the user to forward to here:"), this);
  lay->addMultiCellWidget(lbl, 0, 0, 0, 4);
  edtUser = new CInfoField(this, true);
  edtUser->setAcceptDrops(false);
  lay->addMultiCellWidget(edtUser, 1, 1, 0, 4);

  lay->setColStretch(0, 2);
  btnOk = new QPushButton(tr("&Forward"), this);
  lay->addWidget(btnOk, 2, 1);

  lay->addColSpacing(2, 10);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  lay->addWidget(btnCancel, 2, 3);
  lay->setColStretch(4, 2);

  int bw = 75;
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));
}


CForwardDlg::~CForwardDlg()
{
  if (m_szId) free(m_szId);
}


void CForwardDlg::slot_ok()
{
  if (m_szId == 0) return;

  switch(m_nEventType)
  {
    case ICQ_CMDxSUB_MSG:
    {
      s1.prepend(tr("Forwarded message:\n"));
      UserSendMsgEvent *e = new UserSendMsgEvent(gLicqDaemon, sigman, gMainWindow, m_szId,
        m_nPPID);
      e->setText(s1);
      e->show();
      break;
    }
    case ICQ_CMDxSUB_URL:
    {
      s1.prepend(tr("Forwarded URL:\n"));
      UserSendUrlEvent *e = new UserSendUrlEvent(gLicqDaemon, sigman, gMainWindow, m_szId,
        m_nPPID);
      e->setUrl(s2, s1);
      e->show();
      break;
    }
  }

  close();
}



void CForwardDlg::dragEnterEvent(QDragEnterEvent * dee)
{
  dee->accept(QTextDrag::canDecode(dee));
}


void CForwardDlg::dropEvent(QDropEvent * de)
{
  QString text;

  // extract the text from the event
  if (!QTextDrag::decode(de, text))
    return;

  if (text.length() == 0) return;
  m_szId = strdup(text.latin1());
  m_nPPID = LICQ_PPID; //TODO dropevent needs the ppid

  ICQUser *u = gUserManager.FetchUser(m_szId, m_nPPID, LOCK_R);

  edtUser->setText(QString::fromUtf8(u->GetAlias()) + " (" + text + ")");
  gUserManager.DropUser(u);
}


#include "forwarddlg.moc"
