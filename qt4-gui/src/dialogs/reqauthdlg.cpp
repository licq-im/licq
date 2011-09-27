/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2002-2011 Licq developers
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

#include "reqauthdlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextCodec>
#include <QVBoxLayout>

#include <licq/userid.h>
#include <licq/protocolmanager.h>

#include "helpers/support.h"
#include "helpers/usercodec.h"

#include "widgets/mledit.h"
#include "widgets/protocombobox.h"

using Licq::gProtocolManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ReqAuthDlg */

ReqAuthDlg::ReqAuthDlg(const Licq::UserId& userId, QWidget* parent)
  : QDialog(parent),
    myUserId(userId)
{
  Support::setWidgetProps(this, "RequestAuthDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Licq - Request Authorization"));

  QVBoxLayout* toplay = new QVBoxLayout(this);

  QLabel* lblProtocol = new QLabel(this);
  lblProtocol->setAlignment(Qt::AlignRight);
  lblProtocol->setText(tr("&Protocol:"));
  myProtocolCombo = new ProtoComboBox(ProtoComboBox::FilterOwnersOnly);
  lblProtocol->setBuddy(myProtocolCombo);

  QLabel* lblUin = new QLabel(this);
  lblUin->setAlignment(Qt::AlignRight);
  lblUin->setText(tr("&User ID:"));
  myIdEdit = new QLineEdit(this);
  myIdEdit->setMinimumWidth(90);
  lblUin->setBuddy(myIdEdit);
  connect(myIdEdit, SIGNAL(returnPressed()), SLOT(ok()) );

  QHBoxLayout* lay = new QHBoxLayout();
  lay->addWidget(lblProtocol);
  lay->addWidget(myProtocolCombo);
  lay->addWidget(lblUin);
  lay->addWidget(myIdEdit);

  toplay->addLayout(lay);
  toplay->addSpacing(6);

  grpRequest = new QGroupBox(tr("Request"), this);
  toplay->addWidget(grpRequest);
  toplay->setStretchFactor(grpRequest, 2);

  QVBoxLayout* layRequest = new QVBoxLayout(grpRequest);
  mleRequest = new MLEdit(true);
  mleRequest->setSizeHintLines(5);
  layRequest->addWidget(mleRequest);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  btnOk = buttons->addButton(QDialogButtonBox::Ok);
  btnCancel = buttons->addButton(QDialogButtonBox::Cancel);

  connect (mleRequest, SIGNAL(ctrlEnterPressed()), this, SLOT(ok()));
  connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
  connect (btnCancel, SIGNAL(clicked()), SLOT(close()) );

  toplay->addWidget(buttons);

  if (userId.isValid())
  {
    myIdEdit->setText(userId.accountId().c_str());
    myIdEdit->setEnabled(false);
    myProtocolCombo->setCurrentPpid(userId.protocolId());
    myProtocolCombo->setEnabled(false);
    mleRequest->setFocus();
  }
  else
  {
    myProtocolCombo->setFocus();
  }

  show();
}

void ReqAuthDlg::ok()
{
  Licq::UserId userId = myUserId;
  if (!userId.isValid())
    userId = Licq::UserId(myIdEdit->text().toLatin1().constData(), myProtocolCombo->currentPpid());

  if (userId.isValid())
  {
    const QTextCodec* codec = UserCodec::codecForUserId(userId);
    gProtocolManager.requestAuthorization(userId, codec->fromUnicode(mleRequest->toPlainText()).data());
    close();
  }
}
