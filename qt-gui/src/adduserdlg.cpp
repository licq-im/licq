/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcombobox.h>

#include "adduserdlg.h"

#include "licq_icqd.h"
#include <licq_user.h>

//TODO Add a drop down list of the avaialable protocols
//     that a user may be added for
AddUserDlg::AddUserDlg(CICQDaemon *s, const char* szId, unsigned long PPID,
                       QWidget *parent)
   : LicqDialog(parent, "AddUserDialog")
{
	server = s;

        QBoxLayout *lay = new QBoxLayout(this, QBoxLayout::Down, 8);
        QFrame *frmProtocol = new QFrame(this);
        QFrame *frmUin = new QFrame(this);
	QFrame *frmBtn = new QFrame(this);
        lay->addWidget(frmProtocol);
        lay->addWidget(frmUin);
	lay->addSpacing(5);
	lay->addStretch();
	lay->addWidget(frmBtn);

        QBoxLayout *layProtocol = new QBoxLayout(frmProtocol, QBoxLayout::LeftToRight);
        lblProtocol = new QLabel(tr("Protocol:"), frmProtocol);
        cmbProtocol = new QComboBox(frmProtocol);
        layProtocol->addWidget(lblProtocol);
        layProtocol->addWidget(cmbProtocol);
        
        // Fill the combo list now
        ProtoPluginsList pl;
        ProtoPluginsListIter it;
        server->ProtoPluginList(pl);
        uint index = 0;
        uint ppidIndex = 0;
        for (it = pl.begin(); it != pl.end(); it++, ++index)
        {
          cmbProtocol->insertItem((*it)->Name());
          if ((*it)->PPID() == PPID) ppidIndex = index;
        }
        cmbProtocol->setCurrentItem(ppidIndex);
        
        QBoxLayout *layUin = new QBoxLayout(frmUin, QBoxLayout::LeftToRight);
	lblUin = new QLabel(tr("New User ID:"), frmUin);
	edtUin = new QLineEdit(frmUin);
	layUin->addWidget(lblUin);
	layUin->addWidget(edtUin);

    if (szId != 0) edtUin->setText(szId);
    
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
  connect (btnCancel, SIGNAL(clicked()), SLOT(close()) );

	// Set Tab Order
	setTabOrder(edtUin, btnOk);
	setTabOrder(btnOk, btnCancel);
}


void AddUserDlg::show()
{
   edtUin->setFocus();
   QDialog::show();
}

void AddUserDlg::ok()
{
  QString strUser = edtUin->text().latin1();
  if (!strUser.isEmpty())
  {
    ProtoPluginsList pl;
    ProtoPluginsListIter it;
    server->ProtoPluginList(pl);
    for (it = pl.begin(); it != pl.end(); it++)
      if (strcmp((*it)->Name(), cmbProtocol->currentText().latin1()) == 0)
      {
        UserId userId = LicqUser::makeUserId(strUser, (*it)->PPID());
        gUserManager.addUser(userId);
      }
  }

  close(true);
}

#include "adduserdlg.moc"
