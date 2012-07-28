/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2012 Licq developers <licq-dev@googlegroups.com>
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
#include <QVBoxLayout>

#include <licq/contactlist/user.h>
#include <licq/event.h>
#include <licq/icq/icq.h>
#include <licq/protocolmanager.h>

#include "core/signalmanager.h"

#include "helpers/support.h"

#include "widgets/mledit.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ShowAwayMsgDlg */

ShowAwayMsgDlg::ShowAwayMsgDlg(const Licq::UserId& userId, bool fetch, QWidget* parent)
  : QDialog(parent),
    myUserId(userId),
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

  {
    Licq::UserReadGuard u(myUserId);
    chkShowAgain->setChecked(u->ShowAwayMsg());

    setWindowTitle(QString(tr("%1 Response for %2"))
        .arg(u->statusString(true, false).c_str())
        .arg(QString::fromUtf8(u->getAlias().c_str())));

    if (!fetch)
      mleAwayMsg->setText(QString::fromUtf8(u->autoResponse().c_str()));
  }

  if (fetch)
  {
    mleAwayMsg->setEnabled(false);
    connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
        SLOT(doneEvent(const Licq::Event*)));
    icqEventTag = gLicqDaemon->icqFetchAutoResponse(myUserId);
  }

  show();
}

ShowAwayMsgDlg::~ShowAwayMsgDlg()
{
  {
    Licq::UserWriteGuard u(myUserId);
    u->SetShowAwayMsg(chkShowAgain->isChecked());
  }

  if (icqEventTag != 0)
    Licq::gProtocolManager.cancelEvent(myUserId, icqEventTag);
}

void ShowAwayMsgDlg::doneEvent(const Licq::Event* e)
{
  if (!e->Equals(icqEventTag))
    return;

  bool isOk = (e->Result() == Licq::Event::ResultAcked ||
      e->Result() == Licq::Event::ResultSuccess);

  QString title, result;

  if (e->ExtendedAck() && !e->ExtendedAck()->accepted())
    result = tr("refused");
  else
  {
    switch (e->Result())
    {
      case Licq::Event::ResultFailed:
      case Licq::Event::ResultUnsupported:
      result = tr("failed");
        break;
      case Licq::Event::ResultTimedout:
      result = tr("timed out");
        break;
      case Licq::Event::ResultError:
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

  if (isOk)
  {
    Licq::UserReadGuard u(myUserId);
    QString strResponse = QString::fromUtf8(
      (e->ExtendedAck() && !e->ExtendedAck()->accepted()) ?
       e->ExtendedAck()->response().c_str() :
       u->autoResponse().c_str());

    if (u->protocolId() == LICQ_PPID && QString(u->accountId().c_str())[0].isLetter())
    {
      // Strip HTML
      QRegExp regExp("<.*>");
      regExp.setMinimal(true);
      strResponse.replace(regExp, "");
    }
    mleAwayMsg->setText(strResponse);

    mleAwayMsg->setEnabled(true);
  }
}
