// -*- c-basic-offset: 2 -*-
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

// written by Graham Roff <graham@licq.org>
// enhanced by Dirk A. Mueller <dmuell@gmx.net>
// -----------------------------------------------------------------------------

#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qtimer.h>

#include "awaymsgdlg.h"
#include "licq_log.h"
#include "mainwin.h"
#include "mledit.h"
#include "optionsdlg.h"
#include "licq_sar.h"
#include "licq_user.h"
#include "licq_icqd.h"


// -----------------------------------------------------------------------------

QPoint AwayMsgDlg::snPos = QPoint();

// -----------------------------------------------------------------------------

AwayMsgDlg::AwayMsgDlg(QWidget *parent)
  : QDialog(parent, "AwayMessageDialog", false, WDestructiveClose)
{
  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleAwayMsg = new MLEditWrap(true, this);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  top_lay->addWidget(mleAwayMsg);

  mnuSelect = new QPopupMenu(this);
  connect(mnuSelect, SIGNAL(activated(int)), this, SLOT(slot_selectMessage(int)));

  QBoxLayout* l = new QHBoxLayout(top_lay, 10);
  int bw = 75;
  btnSelect = new QPushButton(tr("&Select"), this);
  btnSelect->setPopup(mnuSelect);
  btnOk = new QPushButton(tr("&Ok"), this );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), SLOT(ok()) );
  btnCancel = new QPushButton(tr("&Cancel"), this );
  connect( btnCancel, SIGNAL(clicked()), SLOT(close()) );
  bw = QMAX(bw, btnSelect->sizeHint().width());
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnSelect->setFixedWidth(bw);
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  l->addWidget(btnSelect);
  l->addStretch(1);
  l->addSpacing(30);
  l->addWidget(btnOk);
  l->addWidget(btnCancel);
}


// -----------------------------------------------------------------------------

void AwayMsgDlg::SelectAutoResponse(unsigned short _status)
{
  if((_status & 0xFF) == ICQ_STATUS_ONLINE || _status == ICQ_STATUS_OFFLINE)
    _status = (_status & 0xFF00) | ICQ_STATUS_AWAY;

  m_nStatus = _status;

  // Fill in the select menu
  mnuSelect->clear();
  switch (m_nStatus)
  {
    case ICQ_STATUS_NA: m_nSAR = SAR_NA; break;
    case ICQ_STATUS_OCCUPIED: m_nSAR = SAR_OCCUPIED; break;
    case ICQ_STATUS_DND: m_nSAR = SAR_DND; break;
    case ICQ_STATUS_FREEFORCHAT: m_nSAR = SAR_FFC; break;
    case ICQ_STATUS_AWAY:
    default:
      m_nSAR = SAR_AWAY;
  }

  if (m_nSAR >= 0) {
    SARList &sar = gSARManager.Fetch(m_nSAR);
    for (unsigned i = 0; i < sar.size(); i++)
      mnuSelect->insertItem(sar[i]->Name(), i);
    gSARManager.Drop();
  }

  mnuSelect->insertSeparator();
  mnuSelect->insertItem(tr("&Edit Items"), 999);

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  setCaption(QString(tr("Set %1 Response for %2"))
             .arg(ICQUser::StatusToStatusStr(m_nStatus, false)).arg(QString::fromLocal8Bit(o->GetAlias())));
  if (*o->AutoResponse())
    mleAwayMsg->setText(QString::fromLocal8Bit(o->AutoResponse()));
  else
    mleAwayMsg->setText(tr("I'm currently %1, %a.\n"
                           "You can leave me a message.\n"
                           "(%m messages pending from you).")
                        .arg(ICQUser::StatusToStatusStr(m_nStatus, false)));
  gUserManager.DropOwner();

  mleAwayMsg->setFocus();
  QTimer::singleShot(0, mleAwayMsg, SLOT(selectAll()));

  if (!isVisible())
  {
    if (!snPos.isNull()) move(snPos);
    show();
  }
}

AwayMsgDlg::~AwayMsgDlg()
{
  emit done();
}

// -----------------------------------------------------------------------------

void AwayMsgDlg::ok()
{
  QString s = mleAwayMsg->text();
  while (s[s.length()-1].isSpace())
    s.truncate(s.length()-1);

  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetAutoResponse(s.local8Bit());
  gUserManager.DropOwner();
  accept();
  close();
}

void AwayMsgDlg::reject()
{
  QTimer::singleShot(0, this, SLOT(close()));
}

// -----------------------------------------------------------------------------

void AwayMsgDlg::slot_selectMessage(int result)
{
  if (result == 999)
    emit popupOptions(OptionsDlg::ODlgStatus);
  else
  {
    SARList &sar = gSARManager.Fetch(m_nSAR);
    if ((unsigned) result < sar.size())
      mleAwayMsg->setText(sar[result]->AutoResponse());

    gSARManager.Drop();
  }
}


// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

CustomAwayMsgDlg::CustomAwayMsgDlg(unsigned long nUin, QWidget *parent)
    : QDialog(parent, "CustomAwayMessageDialog", false, WDestructiveClose)
{
  m_nUin = nUin;

  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleAwayMsg = new MLEditWrap(true, this);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(slot_ok()));
  top_lay->addWidget(mleAwayMsg);

  QBoxLayout* l = new QHBoxLayout(top_lay, 10);

  int bw = 75;
  QPushButton *btnOk = new QPushButton(tr("&Ok"), this );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), SLOT(slot_ok()) );
  QPushButton *btnClear = new QPushButton(tr("&Clear"), this );
  connect( btnClear, SIGNAL(clicked()), SLOT(slot_clear()) );
  QPushButton *btnCancel = new QPushButton(tr("&Cancel"), this );
  connect( btnCancel, SIGNAL(clicked()), SLOT(close()) );
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnClear->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnOk->setFixedWidth(bw);
  btnClear->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  l->addStretch(1);
  l->addSpacing(30);
  l->addWidget(btnOk);
  l->addWidget(btnClear);
  l->addWidget(btnCancel);

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
  setCaption(QString(tr("Set Custom Auto Response for %1"))
             .arg(QString::fromLocal8Bit(u->GetAlias())));
  if (*u->CustomAutoResponse())
    mleAwayMsg->setText(QString::fromLocal8Bit(u->CustomAutoResponse()));
  else if (u->StatusToUser() != ICQ_STATUS_OFFLINE)
    mleAwayMsg->setText(tr("I am currently %1.\nYou can leave me a message.")
                        .arg(ICQUser::StatusToStatusStr(u->StatusToUser(), false)));

  gUserManager.DropUser(u);

  mleAwayMsg->setFocus();
  QTimer::singleShot(0, mleAwayMsg, SLOT(selectAll()));

  show();
}


// -----------------------------------------------------------------------------

void CustomAwayMsgDlg::slot_ok()
{
  QString s = mleAwayMsg->text();
  while(s[s.length()-1].isSpace())
    s.truncate(s.length()-1);

  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  u->SetCustomAutoResponse(s.local8Bit());
  gUserManager.DropUser(u);
  CICQSignal sig(SIGNAL_UPDATExUSER, USER_BASIC, m_nUin);
  gMainWindow->slot_updatedUser(&sig);
  accept();
  close();
}


void CustomAwayMsgDlg::slot_clear()
{
  ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_W);
  u->ClearCustomAutoResponse();
  gUserManager.DropUser(u);
  CICQSignal sig(SIGNAL_UPDATExUSER, USER_BASIC, m_nUin);
  gMainWindow->slot_updatedUser(&sig);
  accept();
  close();
}


#include "awaymsgdlg.moc"
