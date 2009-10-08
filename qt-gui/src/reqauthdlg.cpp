// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2002-2009 Licq developers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qvalidator.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qtextcodec.h>

#include "reqauthdlg.h"
#include "mledit.h"

#include "licq_icqd.h"
#include "usercodec.h"


ReqAuthDlg::ReqAuthDlg(CICQDaemon *s, const char *szId, unsigned long /* nPPID */, QWidget *parent)
  : LicqDialog(parent, "RequestAuthDialog", false, WDestructiveClose)
{
  server = s;

  setCaption(tr("Licq - Request Authorization"));

  QBoxLayout* toplay = new QVBoxLayout(this, 8, 8);

  lblUin = new QLabel(this);
  lblUin->setAlignment(AlignCenter);
  lblUin->setText(tr("Request authorization from (UIN):"));
  edtUin = new QLineEdit(this);
  edtUin->setMinimumWidth(90);
  connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
  QBoxLayout* lay = new QHBoxLayout(toplay);
  lay->addWidget(lblUin);
  lay->addWidget(edtUin);

  toplay->addSpacing(6);

  grpRequest = new QVGroupBox(tr("Request"), this);
  toplay->addWidget(grpRequest);
  toplay->setStretchFactor(grpRequest, 2);

  mleRequest = new MLEditWrap(true, grpRequest);

  QBoxLayout *lay2 = new QHBoxLayout(toplay);

  lay2->addStretch(1);
  btnOk = new QPushButton(tr("&Ok"), this);
  btnOk->setMinimumWidth(75);
  lay2->addWidget(btnOk);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  btnCancel->setMinimumWidth(75);
  lay2->addWidget(btnCancel);
  connect (mleRequest, SIGNAL(signal_CtrlEnterPressed()), this, SLOT(ok()));
  connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
  connect (btnCancel, SIGNAL(clicked()), SLOT(close()) );

  if (szId)
  {
    edtUin->setText(szId);
    mleRequest->setFocus();
  }
  else
    edtUin->setFocus();

  show();
}

void ReqAuthDlg::ok()
{
  char *szId = (char *)edtUin->text().ascii();
  UserId userId = LicqUser::makeUserId(szId, LICQ_PPID);

  if (szId)
  {
    //TODO add a drop down list for protocol
    const QTextCodec* codec = UserCodec::codecForUserId(userId);
    server->icqRequestAuth(szId, codec->fromUnicode(mleRequest->text()));
    close(true);
  }
}

#include "reqauthdlg.moc"
