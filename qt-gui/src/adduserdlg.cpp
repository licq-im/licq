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
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>

#include "adduserdlg.h"

#include "licq_icqd.h"

AddUserDlg::AddUserDlg(CICQDaemon *s, QWidget *parent)
   : LicqDialog(parent, "AddUserDialog")
{
	server = s;
	
	QBoxLayout *lay = new QBoxLayout(this, QBoxLayout::Down, 8);
	QFrame *frmUin = new QFrame(this);
	chkAlert = new QCheckBox(tr("&Alert User"), this);
	QFrame *frmBtn = new QFrame(this);
	lay->addWidget(frmUin);
	lay->addWidget(chkAlert);
	lay->addSpacing(5);
	lay->addStretch();
	lay->addWidget(frmBtn);
	
	QBoxLayout *layUin = new QBoxLayout(frmUin, QBoxLayout::LeftToRight);
	lblUin = new QLabel(tr("New User UIN:"), frmUin);
	edtUin = new QLineEdit(frmUin);
	edtUin->setValidator(new QIntValidator(10000, 2147483647, edtUin));
	layUin->addWidget(lblUin);
	layUin->addWidget(edtUin);
	
	QBoxLayout *layBtn = new QBoxLayout(frmBtn, QBoxLayout::LeftToRight);
	btnOk = new QPushButton(tr("&Ok"), frmBtn);
	btnCancel = new QPushButton(tr("&Cancel"), frmBtn);
	layBtn->addStretch();
	layBtn->addWidget(btnOk);
	layBtn->addSpacing(20);
	layBtn->addWidget(btnCancel);
	
	setCaption(tr("Licq - Add User"));
	connect (btnOk, SIGNAL(clicked()), SLOT(ok()) );
	connect (edtUin, SIGNAL(returnPressed()), SLOT(ok()) );
	connect (btnCancel, SIGNAL(clicked()), SLOT(reject()) );

	// Set Tab Order
	setTabOrder(edtUin, chkAlert);
	setTabOrder(chkAlert, btnOk);
	setTabOrder(btnOk, btnCancel);
}


void AddUserDlg::show()
{
   edtUin->setText("");
   edtUin->setFocus();
   chkAlert->setChecked(true);
   QDialog::show();
}

void AddUserDlg::ok()
{
   unsigned long nUin = edtUin->text().toULong();
   if (nUin != 0)
   {
     server->AddUserToList(nUin);
     if (chkAlert->isChecked()) // alert the user they were added
       server->icqAlertUser(nUin);
   }
   close(true);
}

#include "adduserdlg.moc"
