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

#include "awaymsgdlg.h"
#include "log.h"
#include "mledit.h"
#include "sar.h"
#include "user.h"

int AwayMsgDlg::s_nX = 100;
int AwayMsgDlg::s_nY = 100;

// -----------------------------------------------------------------------------

AwayMsgDlg::AwayMsgDlg(QWidget *parent, const char *name)
    : QDialog(parent, name)
{
  QBoxLayout* top_lay = new QVBoxLayout(this, 10);

  mleAwayMsg = new MLEditWrap(true, this);
  // ICQ99b allows 37 chars per line, so we do the same
  mleAwayMsg->setWordWrap(QMultiLineEditNew::FixedColumnWidth);
  mleAwayMsg->setWrapColumnOrWidth(37);
  connect(mleAwayMsg, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  top_lay->addWidget(mleAwayMsg);

  QBoxLayout* l = new QHBoxLayout(top_lay, 10);

  int bw = 75;
  btnSelect = new QPushButton(tr("&Select"), this);
//  btnSelect->setIsMenuButton(true);
  connect(btnSelect, SIGNAL(clicked()), SLOT(slot_selectMessage()));
  btnOk = new QPushButton(tr("&Ok"), this );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), SLOT(ok()) );
  btnCancel = new QPushButton(tr("&Cancel"), this );
  connect( btnCancel, SIGNAL(clicked()), SLOT(reject()) );
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
  m_nStatus = _status;

  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  setCaption(QString(tr("Set %1 Response for %2"))
             .arg(ICQUser::StatusToStatusStr(m_nStatus, false)).arg(QString::fromLocal8Bit(o->GetAlias())));
  if (*o->AutoResponse())
    mleAwayMsg->setText(QString::fromLocal8Bit(o->AutoResponse()));
  else
    mleAwayMsg->setText(tr("I am currently %1.\nYou can leave me a message.")
                        .arg(ICQUser::StatusToStatusStr(m_nStatus, false)));
  gUserManager.DropOwner();

  move(s_nX, s_nY);
  mleAwayMsg->setFocus();
  mleAwayMsg->selectAll();

  QDialog::show();
}

// -----------------------------------------------------------------------------

void AwayMsgDlg::show()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned short m_nStatus = o->Status();
  gUserManager.DropOwner();

  SelectAutoResponse(m_nStatus);
}

// -----------------------------------------------------------------------------

void AwayMsgDlg::hide()
{
  s_nX = x();
  s_nY = y();
  QDialog::hide();
}

// -----------------------------------------------------------------------------

void AwayMsgDlg::ok()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetAutoResponse(mleAwayMsg->text().local8Bit());
  gUserManager.DropOwner();
  accept();
}

// -----------------------------------------------------------------------------

void AwayMsgDlg::slot_selectMessage()
{
  QPopupMenu* menu = new QPopupMenu(this);

  int result = 0;

  // Fill in the menu bar
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
      menu->insertItem(sar[i]->Name(), i);
    gSARManager.Drop();
  }

  menu->insertSeparator();
  // as this is not yet implemented, give user feedback
  menu->setItemEnabled(menu->insertItem(tr("&Edit Items"), -2), false);

  result = menu->exec(btnSelect->mapToGlobal(QPoint(0,btnSelect->height())));

  if(result == -2) {
    // todo: open options menu
  }
  else {
    SARList &sar = gSARManager.Fetch(m_nSAR);
    if ((unsigned) result < sar.size())
      mleAwayMsg->setText(sar[result]->AutoResponse());

    gSARManager.Drop();
  }

  delete menu;
}

// -----------------------------------------------------------------------------

#include "awaymsgdlg.moc"
