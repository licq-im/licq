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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qvalidator.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qvgroupbox.h>

#include "authuserdlg.h"
#include "mledit.h"

#include "licq_user.h"
#include "licq_icqd.h"

AuthUserDlg::AuthUserDlg(CICQDaemon *s, unsigned long nUin, bool bGrant,
   QWidget *parent)
  : QDialog(parent, "AuthUserDialog", false, WDestructiveClose)
{
  server = s;
  m_nUin = nUin;
  m_bGrant = bGrant;

  if(bGrant)
    setCaption(tr("Licq - Grant Authorisation"));
  else
    setCaption(tr("Licq - Refuse Authorisation"));

  QBoxLayout* toplay = new QVBoxLayout(this, 8, 8);

  lblUin = new QLabel(this);
  lblUin->setAlignment(AlignCenter);
  if (m_nUin == 0)
  {
    lblUin->setText(tr("Authorize which user (UIN):"));
    edtUin = new QLineEdit(this);
    edtUin->setMinimumWidth(90);
    edtUin->setValidator(new QIntValidator(10000, 2147483647, edtUin));
    connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
    QBoxLayout* lay = new QHBoxLayout(toplay);
    lay->addWidget(lblUin);
    lay->addWidget(edtUin);
  }
  else
  {
    edtUin = NULL;
    toplay->addWidget(lblUin);
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
    QString s = (u != NULL) ? QString("%1 (%2)").arg(u->GetAlias()).arg(m_nUin) : QString::number(m_nUin);
    gUserManager.DropUser(u);

    if (bGrant)
      lblUin->setText(tr("Grant authorization to %1").arg(s));
    else
      lblUin->setText(tr("Refuse authorization to %1").arg(s));
  }

  toplay->addSpacing(6);

  grpResponse = new QVGroupBox(tr("Response"), this);
  toplay->addWidget(grpResponse);
  toplay->setStretchFactor(grpResponse, 2);

  mleResponse = new MLEditWrap(true, grpResponse);

  QBoxLayout* lay = new QHBoxLayout(toplay);
  lay->addStretch(1);
  btnOk = new QPushButton(tr("&Ok"), this);
  btnOk->setMinimumWidth(75);
  lay->addWidget(btnOk);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  btnCancel->setMinimumWidth(75);
  lay->addWidget(btnCancel);
  connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
  connect (btnCancel, SIGNAL(clicked()), SLOT(close()) );

  if (m_nUin)
    mleResponse->setFocus();
  else
    edtUin->setFocus();

  show();
}

void AuthUserDlg::ok()
{
  unsigned long nUin = m_nUin;
  if (nUin == 0) nUin = edtUin->text().toULong();

  if (nUin != 0)
  {
    if (m_bGrant)
      server->icqAuthorizeGrant(nUin, mleResponse->text().local8Bit().data());
    else
      server->icqAuthorizeRefuse(nUin, mleResponse->text().local8Bit().data());
    accept();
    close();
  }
}

#include "authuserdlg.moc"
