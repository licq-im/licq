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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include "refusedlg.h"
#include "mledit.h"

#include "licq_user.h"


CRefuseDlg::CRefuseDlg(const char *szId, unsigned long nPPID, QString t,
   QWidget* parent)
   : LicqDialog(parent, "RefuseDialog", true)
{
  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  QLabel *lbl = new QLabel(tr("Refusal message for %1 with ").arg(t) + QString::fromUtf8(u->GetAlias()) + ":", this);
  gUserManager.DropUser(u);

  mleRefuseMsg = new MLEditWrap(true, this);

  QPushButton *btnRefuse = new QPushButton(tr("Refuse"), this );
  connect( btnRefuse, SIGNAL(clicked()), SLOT(accept()) );
  QPushButton *btnCancel = new QPushButton(tr("Cancel"), this );
  connect( btnCancel, SIGNAL(clicked()), SLOT(reject()) );
  int bw = 75;
  bw = QMAX(bw, btnRefuse->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnRefuse->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  QGridLayout *lay = new QGridLayout(this, 3, 5, 15, 10);
  lay->addMultiCellWidget(lbl, 0, 0, 0, 4);
  lay->addMultiCellWidget(mleRefuseMsg, 1, 1, 0, 4);
  lay->addWidget(btnRefuse, 2, 1);
  lay->addWidget(btnCancel, 2, 3);
  lay->setColStretch(0, 2);
  lay->setColStretch(4, 2);
  lay->addColSpacing(2, 10);

  setCaption(tr("Licq %1 Refusal").arg(t));
}

QString CRefuseDlg::RefuseMessage()
{
  return mleRefuseMsg->text();
}

#include "refusedlg.moc"
