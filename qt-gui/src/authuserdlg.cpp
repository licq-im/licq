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

#include "authuserdlg.h"

#include "icqd.h"

AuthUserDlg::AuthUserDlg(CICQDaemon *s, QWidget *parent, const char *name)
  : QDialog(parent, name)
{
   server = s;
   setCaption(tr("Licq"));
   resize(340, 100);
   lblUin = new QLabel(tr("Authorize which user (UIN):"), this);
   lblUin->setGeometry(10, 15, 160, 20);
   edtUin = new QLineEdit(this);
   edtUin->setGeometry(180, 15, 120, 20);
   edtUin->setValidator(new QIntValidator(0, 2147483647, edtUin));
   btnOk = new QPushButton(tr("&Ok"), this);
   btnOk->setGeometry(80, 60, 80, 30);
   btnCancel = new QPushButton(tr("&Cancel"), this);
   btnCancel->setGeometry(180, 60, 80, 30);
   connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
   connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
   connect (btnCancel, SIGNAL(clicked()), SLOT(reject()) );
}


void AuthUserDlg::show()
{
  edtUin->setText("");
  edtUin->setFocus();
  QDialog::show();
}


void AuthUserDlg::hide()
{
   QDialog::hide();
   delete this;
}


void AuthUserDlg::ok()
{
  unsigned long nUin = atol((const char *)edtUin->text());
  if (nUin != 0) server->icqAuthorize(nUin);
  accept();
}

#include "moc/moc_authuserdlg.h"
