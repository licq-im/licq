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

#include "authuserdlg.h"

#include "licq_icqd.h"

AuthUserDlg::AuthUserDlg(CICQDaemon *s, QWidget *parent, const char *name)
  : QDialog(parent, name)
{
  server = s;
  setCaption(tr("Licq - Authorisation"));
  QBoxLayout* toplay = new QVBoxLayout(this, 8, 8);
  QBoxLayout* lay = new QHBoxLayout(toplay);

  lblUin = new QLabel(tr("Authorize which user (UIN):"), this);
  lay->addWidget(lblUin);
  edtUin = new QLineEdit(this);
  edtUin->setMinimumWidth(90);
  lay->addWidget(edtUin);
  edtUin->setValidator(new QIntValidator(100000, 2147483647, edtUin));
  toplay->addSpacing(20);
  toplay->addStretch(2);
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
  connect (btnCancel, SIGNAL(clicked()), SLOT(reject()) );

  edtUin->setFocus();
  show();
}


void AuthUserDlg::hideEvent(QHideEvent*)
{
  close(true);
}


void AuthUserDlg::ok()
{
  unsigned long nUin = edtUin->text().toULong();
  if (nUin != 0) server->icqAuthorizeGrant(nUin);
  accept();
}

#include "authuserdlg.moc"
