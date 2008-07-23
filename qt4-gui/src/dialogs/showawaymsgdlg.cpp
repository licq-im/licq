// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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
// enhanced by Dirk A. Mueller <dmuell@gmx.net>
// -----------------------------------------------------------------------------

#include "showawaymsgdlg.h"

#include <ctype.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRegExp>
#include <QTextCodec>
#include <QVBoxLayout>

#include <licq_icqd.h>
#include <licq_user.h>

#include "core/licqgui.h"
#include "core/signalmanager.h"

#include "helpers/licqstrings.h"
#include "helpers/support.h"
#include "helpers/usercodec.h"

#include "widgets/mledit.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ShowAwayMsgDlg */

ShowAwayMsgDlg::ShowAwayMsgDlg(QString id, unsigned long ppid,
    bool fetch, QWidget* parent)
  : QDialog(parent),
    myId(id),
    myPpid(ppid),
    icqEventTag(0)
{
  Support::setWidgetProps(this, "ShowAwayMessageDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);

  QVBoxLayout* top_lay = new QVBoxLayout(this);

  mleAwayMsg = new MLEdit(true, this);
  mleAwayMsg->setReadOnly(true);
  mleAwayMsg->setSizeHintLines(5);
  connect(mleAwayMsg, SIGNAL(ctrlEnterPressed()), SLOT(close()));
  top_lay->addWidget(mleAwayMsg);

  QHBoxLayout* lay = new QHBoxLayout();
  top_lay->addLayout(lay);

  chkShowAgain = new QCheckBox(tr("&Show Again"), this);
  lay->addWidget(chkShowAgain);

  lay->addStretch(1);

  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
  buttons->button(QDialogButtonBox::Close)->setDefault(true);
  connect(buttons, SIGNAL(rejected()), SLOT(close()));
  lay->addWidget(buttons);

  const ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_R);
  QTextCodec* codec = UserCodec::codecForICQUser(u);
  chkShowAgain->setChecked(u->ShowAwayMsg());

  setWindowTitle(QString(tr("%1 Response for %2"))
      .arg(LicqStrings::getStatus(u, false))
      .arg(QString::fromUtf8(u->GetAlias())));

  if (fetch)
  {
    bool bSendServer =
      (u->SocketDesc(ICQ_CHNxNONE) <= 0 &&
       u->Version() > 6);
    gUserManager.DropUser(u);
    mleAwayMsg->setEnabled(false);
    connect(LicqGui::instance()->signalManager(),
        SIGNAL(doneUserFcn(ICQEvent*)), SLOT(doneEvent(ICQEvent*)));
    icqEventTag = gLicqDaemon->icqFetchAutoResponse(
        myId.toLatin1(), myPpid, bSendServer);
  }
  else
  {
    mleAwayMsg->setText(codec->toUnicode(u->AutoResponse()));
    gUserManager.DropUser(u);
  }

  show();
}

ShowAwayMsgDlg::~ShowAwayMsgDlg()
{
  ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_W);
  u->SetShowAwayMsg(chkShowAgain->isChecked());
  gUserManager.DropUser(u);

  if (icqEventTag != 0)
    gLicqDaemon->CancelEvent(icqEventTag);
}

void ShowAwayMsgDlg::doneEvent(ICQEvent* e)
{
  if (!e->Equals(icqEventTag))
    return;

  bool isOk =
    (e->Result() == EVENT_ACKED ||
     e->Result() == EVENT_SUCCESS);

  QString title, result;

  if (e->ExtendedAck() &&
      !e->ExtendedAck()->Accepted())
    result = tr("refused");
  else
  {
    switch (e->Result())
    {
    case EVENT_FAILED:
      result = tr("failed");
      break;
    case EVENT_TIMEDOUT:
      result = tr("timed out");
      break;
    case EVENT_ERROR:
      result = tr("error");
      break;
    default:
      break;
    }
  }

  if (!result.isEmpty())
  {
    title = " [" + result + "]";
    setWindowTitle(windowTitle() + title);
  }

  icqEventTag = 0;

  if (isOk &&
      (e->Command() == ICQ_CMDxTCP_START ||
       e->SNAC() == MAKESNAC(ICQ_SNACxFAM_MESSAGE, ICQ_SNACxMSG_SENDxSERVER) ||
       e->SNAC() == MAKESNAC(ICQ_SNACxFAM_LOCATION, ICQ_SNACxLOC_INFOxREQ)))
  {
    const ICQUser* u = gUserManager.FetchUser(myId.toLatin1(), myPpid, LOCK_R);
    QTextCodec* codec = UserCodec::codecForICQUser(u);
    const char* szAutoResp =
      (e->ExtendedAck() && !e->ExtendedAck()->Accepted()) ?
       e->ExtendedAck()->Response() :
       u->AutoResponse();

    if (myPpid == LICQ_PPID && myId[0].isLetter())
    {
      // Strip HTML
      QString strResponse(codec->toUnicode(szAutoResp));
      QRegExp regExp("<.*>");
      regExp.setMinimal(true);
      strResponse.replace(regExp, "");
      mleAwayMsg->setText(strResponse);
    }
    else
      mleAwayMsg->setText(codec->toUnicode(szAutoResp));

    gUserManager.DropUser(u);
    mleAwayMsg->setEnabled(true);
  }
}
