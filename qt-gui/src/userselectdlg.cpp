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

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcombobox.h>

#include "userselectdlg.h"
#include "licq_icqd.h"
#include "licq_user.h"

UserSelectDlg::UserSelectDlg(CICQDaemon *s, QWidget *parent)
  : LicqDialog(parent, "UserSelectDialog", false, WStyle_ContextHelp | WDestructiveClose)
{
  server = s;

  QGridLayout *lay = new QGridLayout(this, 4, 2, 10, 5);

  lblUser = new QLabel(tr("User:"), this);
  lay->addWidget(lblUser, 0, 0);
  cmbUser = new QComboBox(this);
  lay->addWidget(cmbUser, 0, 1);

  lblPassword = new QLabel(tr("Password:"), this);
  lay->addWidget(lblPassword, 1, 0);
  edtPassword = new QLineEdit(this);
  edtPassword->setEchoMode(QLineEdit::Password);
  lay->addWidget(edtPassword, 1, 1);

  chkSavePassword = new QCheckBox(tr("&Save Password"), this);
  lay->addMultiCellWidget(chkSavePassword, 2, 2, 0, 1);

  QBoxLayout *hbox = new QHBoxLayout(NULL);
  btnOk = new QPushButton(tr("&Ok"), this);
  hbox->addWidget(btnOk);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  hbox->addWidget(btnCancel);
  lay->addMultiCellLayout(hbox, 3, 3, 0, 1);

  int bw = 75;
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  connect(cmbUser, SIGNAL(activated(const QString &)), SLOT(slot_cmbSelectUser(const QString &)));
  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));

  setCaption(tr("Licq User Select"));

  // Populate the combo box
 
  // For now, just have one owner
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  cmbUser->insertItem(tr("%1 (%2)").arg(o->GetAlias()).arg(o->Uin()));
  edtPassword->setText(o->Password());
  gUserManager.DropOwner();
    
  show();
}

UserSelectDlg::~UserSelectDlg()
{
}

void UserSelectDlg::slot_ok()
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
  o->SetSavePassword(chkSavePassword->isChecked());
  o->SetPassword(edtPassword->text().latin1());
  gUserManager.DropOwner();

  close();
}

void UserSelectDlg::slot_cmbSelectUser(const QString &str)
{
} 


#include "userselectdlg.moc"

