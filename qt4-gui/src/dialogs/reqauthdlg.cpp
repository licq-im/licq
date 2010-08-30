// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2002-2010 Licq developers
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

using Licq::gProtocolManager;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ReqAuthDlg */

ReqAuthDlg::ReqAuthDlg(const QString& id, unsigned long ppid, QWidget* parent)
  : QDialog(parent),
    myPpid(ppid == 0 ? LICQ_PPID : ppid)
{
  Support::setWidgetProps(this, "RequestAuthDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Licq - Request Authorization"));

  QVBoxLayout* toplay = new QVBoxLayout(this);

  lblUin = new QLabel(this);
  lblUin->setAlignment(Qt::AlignCenter);
  lblUin->setText(tr("Request authorization from (UIN):"));
  edtUin = new QLineEdit(this);
  edtUin->setMinimumWidth(90);
  connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
  QHBoxLayout* lay = new QHBoxLayout();
  lay->addWidget(lblUin);
  lay->addWidget(edtUin);

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

  if (!id.isEmpty())
  {
    edtUin->setText(id);
    mleRequest->setFocus();
  }
  else
    edtUin->setFocus();

  show();
}

void ReqAuthDlg::ok()
{
  QString id = edtUin->text();
  Licq::UserId userId(id.toLatin1().data(), myPpid);

  if (!id.isEmpty())
  {
    //TODO add a drop down list for protocol
    const QTextCodec* codec = UserCodec::codecForUserId(userId);
    gProtocolManager.requestAuthorization(userId, codec->fromUnicode(mleRequest->toPlainText()).data());
    close();
  }
}
