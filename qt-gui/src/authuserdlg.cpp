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
   QWidget *parent, const char *name)
  : QDialog(parent, name, false, WDestructiveClose)
{
  server = s;
  m_nUin = nUin;
  m_bGrant = bGrant;

  setCaption(tr("Licq - Authorisation %1").arg(bGrant ? tr("Grant"):tr("Refuse")));
  QBoxLayout* toplay = new QVBoxLayout(this, 8, 8);
  QBoxLayout* lay = new QHBoxLayout(toplay);

  lblUin = new QLabel(this);
  lay->addWidget(lblUin);

  if (m_nUin == 0)
  {
    lblUin->setText(tr("Authorize which user (UIN):"));
    edtUin = new QLineEdit(this);
    edtUin->setMinimumWidth(90);
    edtUin->setValidator(new QIntValidator(10000, 2147483647, edtUin));
    lay->addWidget(edtUin);
  }
  else
  {
    ICQUser *u = gUserManager.FetchUser(m_nUin, LOCK_R);
    lblUin->setText(tr("%1 authorization to %2")
       .arg(bGrant ? tr("Grant"):tr("Refuse"))
       .arg(u->GetAlias()) );
    gUserManager.DropUser(u);
  }

  toplay->addSpacing(6);

  grpResponse = new QVGroupBox(tr("Response"), this);
  toplay->addWidget(grpResponse);
  toplay->setStretchFactor(grpResponse, 2);

  mleResponse = new MLEditWrap(true, grpResponse);
  //mleResponse->setText(tr("Authorisation granted"));

  lay = new QHBoxLayout(toplay);
  lay->addStretch(1);
  btnOk = new QPushButton(tr("&Ok"), this);
  btnOk->setMinimumWidth(75);
  lay->addWidget(btnOk);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  btnCancel->setMinimumWidth(75);
  lay->addWidget(btnCancel);
  connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
  connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
  connect (btnCancel, SIGNAL(clicked()), SLOT(close()) );

  /*if (UIN) {
    mleResponse->selectAll();
    mleResponse->setFocus();
  }
  else*/
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
