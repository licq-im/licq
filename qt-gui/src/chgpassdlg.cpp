/*
 *  chgpassdlg -- change licq's stored password dialog
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>
#include <qtoolbutton.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "chgpassdlg.h"
#include "ewidgets.h"

#include "licq_user.h"

ChangePassDlg::ChangePassDlg( QWidget *parent)
	: QDialog(parent, "Licq.ChangePasswordDialog", WDestructiveClose)
{
	setCaption(tr("Licq - Set UIN/Password"));

	unsigned long nUin = gUserManager.OwnerUin();
	QString strUin;

	if (nUin)
		strUin.setNum(nUin);

	QVBoxLayout *lay = new QVBoxLayout(this, 8);
	QGroupBox *box = new QGroupBox(3, Vertical, tr("Login Information"),
		this);

	assert(lay);
	assert(box);
	lay->addWidget(box);

	
	lblUin = new QLabel(tr("&UIN:"), box);
	lblPassword = new QLabel(tr("&Password:"), box);
	lblVerify = new QLabel(tr("&Verify:"), box);
	edtUin = new QLineEdit(box);
	edtFirst = new QLineEdit(box);
	edtSecond = new QLineEdit(box);
	assert( lblUin );
	assert( lblPassword );
	assert( lblVerify );
	assert( edtFirst );
	assert( edtSecond );

	edtUin->setValidator(new QIntValidator(10000, 2147483647, edtUin));
	if (nUin) edtUin->setText(strUin);
	QWhatsThis::add(edtUin, tr("Enter your UIN here."));
	edtFirst->setEchoMode(QLineEdit::Password);
	QWhatsThis::add(edtFirst, tr("Enter your ICQ password here."));
	edtSecond->setEchoMode(QLineEdit::Password);
	QWhatsThis::add(edtSecond, tr("Verify your ICQ password here."));
	lblUin->setBuddy(edtUin);
	lblPassword->setBuddy(edtFirst);
	lblVerify->setBuddy(edtSecond);

	lay->addStretch(1);

	btnUpdate = new QPushButton(tr("&Ok"), this);
	assert(btnUpdate);
	btnUpdate->setDefault(true);
	btnUpdate->setMinimumWidth(75);
	btnCancel = new QPushButton(tr("&Cancel"), this);
	assert(btnUpdate);
	btnCancel->setMinimumWidth(75);
	QHBoxLayout *hlay = new QHBoxLayout;
	assert(hlay);
	hlay->addWidget(QWhatsThis::whatsThisButton(this), 0, AlignLeft);
	hlay->addSpacing(20);
	hlay->addStretch(1);
	hlay->addWidget(btnUpdate, 0, AlignRight);
	hlay->addSpacing(20);
	hlay->addWidget(btnCancel, 0, AlignLeft);
	lay->addLayout(hlay);

	connect (btnUpdate, SIGNAL(clicked()), SLOT(accept()) );
	connect (btnCancel, SIGNAL(clicked()), SLOT(reject()) );

	show();
}


void ChangePassDlg::accept()
{
	if (edtFirst->text().isEmpty() || edtFirst->text().length() > 8)
		InformUser (this,
		         tr("Invalid password, must be 8 characters or less."));
	else if (edtFirst->text() != edtSecond->text())
		InformUser (this, tr("Passwords do not match, try again."));
	else
	{
		gUserManager.SetOwnerUin(edtUin->text().toULong());
		ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
		o->SetPassword(edtFirst->text().latin1());
		gUserManager.DropOwner();
		close(); 
	}
}

#include "chgpassdlg.moc"

