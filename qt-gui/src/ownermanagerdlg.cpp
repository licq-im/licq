/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2004-2006 Licq developers
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

#include <qcheckbox.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qvbuttongroup.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qlabel.h>
#include <qdialog.h>

#include "ownermanagerdlg.h"
#include "ewidgets.h"
#include "mainwin.h"
#include "licq_icqd.h"
#include "licq_log.h"
#include "licq_user.h"

//----OwnerEditDlg-------------------------------------------------------------

OwnerEditDlg::OwnerEditDlg(CICQDaemon *s, const char *szId,
  unsigned long nPPID, QWidget *parent)
    : LicqDialog(parent, "OwnerEdit", false, WDestructiveClose)
{
  server = s;
  setCaption(tr("Edit Account"));
  
  QGridLayout *lay = new QGridLayout(this, 1, 3, 8, 4);
  lay->setColStretch(2, 2);
  lay->addColSpacing(1, 8);
  
  // User ID
  lay->addWidget(new QLabel(tr("User ID:"), this), 0, 0);
  edtId = new QLineEdit(this);
  edtId->setMinimumWidth(edtId->sizeHint().width()*2);
  lay->addWidget(edtId, 0, 2);
  
  // Password
  lay->addWidget(new QLabel(tr("Password:"), this), 1, 0); 
  edtPassword = new QLineEdit(this);
  edtPassword->setEchoMode(QLineEdit::Password);
  lay->addWidget(edtPassword, 1, 2);
  
  // Protocol
  lay->addWidget(new QLabel(tr("Protocol:"), this), 2, 0);
  cmbProtocol = new QComboBox(this);
  lay->addWidget(cmbProtocol, 2, 2);
  
  // Fill the combo list now
  ProtoPluginsList pl;
  ProtoPluginsListIter it;
  server->ProtoPluginList(pl);
  int n = 0;
  for (it = pl.begin(); it != pl.end(); it++)
  {
    if (nPPID) // Modifying a user
      cmbProtocol->insertItem((*it)->Name(), n++);
    else // Adding a user
    {
      ICQOwner *o = gUserManager.FetchOwner((*it)->PPID(), LOCK_R);
      if (o == NULL)
        cmbProtocol->insertItem((*it)->Name(), n++);
      else
        gUserManager.DropOwner(o);
    }
  }

  // Set the fields
  if (szId && nPPID)
  {
    edtId->setText(szId);
    ICQOwner *o = gUserManager.FetchOwner(nPPID, LOCK_R);
    if (o)
    {
      edtPassword->setText(o->Password());
      gUserManager.DropOwner(o);
    }

    n = 0;
    for (it = pl.begin(); it != pl.end(); it++, n++)
    {
      if ((*it)->PPID() == nPPID)
      {
        cmbProtocol->setCurrentItem(n);  
        break;
      }
    }
    cmbProtocol->setEnabled(false);
  }
  else
  {
    if (cmbProtocol->count() == 0)
    {
      InformUser(this, tr("Currently only one account per protocol is supported."));
      close();
      return;
    }
  }
        
  QBoxLayout *hlay = new QHBoxLayout();
  lay->addMultiCell(hlay, 5, 5, 0, 2);
  hlay->addStretch(1);
  btnOk = new QPushButton(tr("&Ok"), this);
  btnOk->setMinimumWidth(75);
  hlay->addWidget(btnOk, 0, AlignRight);
  hlay->addSpacing(20);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  btnCancel->setMinimumWidth(75);
  hlay->addWidget(btnCancel, 0, AlignLeft);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(edtId, SIGNAL(returnPressed()), SLOT(slot_ok()));
  connect(edtPassword, SIGNAL(returnPressed()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));

  // Set Tab Order
  setTabOrder(edtId, edtPassword);
  setTabOrder(edtPassword, cmbProtocol);
  setTabOrder(cmbProtocol, btnOk);
  setTabOrder(btnOk, btnCancel);
}

void OwnerEditDlg::slot_ok()
{
  const char *szUser = edtId->text().latin1();
  const char *szPassword = 0;
  if (!edtPassword->text().isEmpty())
    szPassword = edtPassword->text().latin1();
  QString szProtocol = cmbProtocol->currentText();
  unsigned long nPPID = 0;
  
  ProtoPluginsList pl;
  ProtoPluginsListIter it;
  server->ProtoPluginList(pl);
  for (it = pl.begin(); it != pl.end(); it++)
  {
    if (szProtocol == QString((*it)->Name()))
    {
      nPPID = (*it)->PPID();
      break;
    }
  }
  
  // Invalid protocol
  if (nPPID == 0)
  {
    gLog.Error("%sInvalid protocol '%s'\n", L_ERRORxSTR, szProtocol.latin1());
    return;
  }
  
  ICQOwner *o = gUserManager.FetchOwner(nPPID, LOCK_W);
  if (o)
  {
    if (szPassword)
      o->SetPassword(szPassword);
    o->SetId(szUser);
  }
  else
  {
    gUserManager.AddOwner(szUser, nPPID);
    o = gUserManager.FetchOwner(nPPID, LOCK_W);
    if (szPassword)
      o->SetPassword(szPassword);
  }

  gUserManager.DropOwner(o);
  server->SaveConf();

  close();
}

//----OwnerView----------------------------------------------------------------

OwnerView::OwnerView(QWidget *parent)
  : QListView(parent, "OwnerManagerDialog")
{
  addColumn(tr("User ID"), 150);
  addColumn(tr("Protocol"), 70);
  
  setAllColumnsShowFocus(false);
  setMinimumHeight(30);
  setMinimumWidth(230);
  setResizeMode(QListView::LastColumn);
}

//----OwnerItem----------------------------------------------------------------

OwnerItem::OwnerItem(CICQDaemon *s, const char *szId, unsigned long nPPID,
  QListView *parent) : QListViewItem(parent)
{
  m_szId = szId ? strdup(szId) : strdup(OwnerView::tr("(Invalid ID)"));
  m_nPPID = nPPID;
  char *szPluginName = 0;
  
  ProtoPluginsList pl;
  ProtoPluginsListIter it;
  s->ProtoPluginList(pl);
  for (it = pl.begin(); it != pl.end(); it++)
  {
    if ((*it)->PPID() == nPPID)
      szPluginName = strdup((*it)->Name());
  }
  
  if (szPluginName == 0)
    szPluginName = strdup(OwnerView::tr("Invalid Protocol"));
    
  setText(0, m_szId);
  setText(1, szPluginName); 

  free (szPluginName);
}

//----OwnerManagerDlg----------------------------------------------------------

OwnerManagerDlg::OwnerManagerDlg(CMainWindow *m, CICQDaemon *s, CSignalManager *sm)
  : LicqDialog(NULL, "AccountDialog", false, WDestructiveClose)
{
  mainwin = m;
  server = s;
  sigman = sm;
  registerUserDlg = 0;
  setCaption(tr("Licq - Account Manager"));
  
  QBoxLayout *toplay = new QVBoxLayout(this, 8, 8); 
  
  // Add the list box
  ownerView = new OwnerView(this);
  toplay->addWidget(ownerView);
      
  // Add the buttons now
  QBoxLayout *lay = new QHBoxLayout(toplay, 10);
  btnAdd = new QPushButton(tr("&Add"), this);
  lay->addWidget(btnAdd);
  btnRegister = new QPushButton(tr("&Register"), this);
  lay->addWidget(btnRegister);
  btnModify = new QPushButton(tr("&Modify"), this);
  lay->addWidget(btnModify);
  btnDelete = new QPushButton(tr("D&elete"), this);
  lay->addWidget(btnDelete);
  btnDone = new QPushButton(tr("&Done"), this);
  lay->addWidget(btnDone);
  
  // Connect all the signals
  connect(ownerView, SIGNAL(clicked(QListViewItem *, const QPoint &, int)),
          this, SLOT(slot_listClicked(QListViewItem *, const QPoint &, int)));
  connect(ownerView, SIGNAL(currentChanged(QListViewItem *)),
          this, SLOT(slot_listClicked(QListViewItem *)));
  connect(ownerView, SIGNAL(spacePressed(QListViewItem *)),
          this, SLOT(slot_listClicked(QListViewItem *)));
  connect(ownerView, SIGNAL(doubleClicked(QListViewItem*)),
      this, SLOT(modifyOwner(QListViewItem*)));
  connect(btnAdd, SIGNAL(clicked()), this, SLOT(slot_addClicked()));
  connect(btnRegister, SIGNAL(clicked()), this, SLOT(slot_registerClicked()));
  connect(btnModify, SIGNAL(clicked()), this, SLOT(slot_modifyClicked()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(slot_deleteClicked()));
  connect(btnDone, SIGNAL(clicked()), this, SLOT(close()));

  // Add the owners to the list now
  updateOwners();

  // Show information to the user
  if (gUserManager.NumOwners() == 0)
  {
    InformUser(this, tr("From the Account Manager dialog you are able to add and register "
                        "your accounts.\n"
                        "Currently, only one account per protocol is supported, but this "
                        "will be changed in future versions."));
  }
}

OwnerManagerDlg::~OwnerManagerDlg()
{
  emit signal_done();
}

void OwnerManagerDlg::updateOwners()
{
  ownerView->clear();
  
  FOR_EACH_OWNER_START(LOCK_R)
  {
    (void) new OwnerItem(server, pOwner->IdString(), pOwner->PPID(), ownerView);
  }
  FOR_EACH_OWNER_END
  
  btnModify->setEnabled(false);
  btnDelete->setEnabled(false);
}

void OwnerManagerDlg::slot_listClicked(QListViewItem *it)
{
  btnModify->setEnabled(it);
  btnDelete->setEnabled(it);
}

void OwnerManagerDlg::slot_listClicked(QListViewItem *it, const QPoint &, int)
{
  slot_listClicked(it);
}

void OwnerManagerDlg::slot_addClicked()
{
  OwnerEditDlg *d = new OwnerEditDlg(server, 0, 0, this);
  d->show();
  connect(d, SIGNAL(destroyed()), this, SLOT(slot_update()));
}

void OwnerManagerDlg::slot_registerClicked()
{
  if (!gUserManager.OwnerId(LICQ_PPID).empty())
  {
    QString buf = tr("You are currently registered as\n"
                    "UIN (User ID): %1\n"
                    "Base Directory: %2\n"
                    "Rerun licq with the -b option to select a new\n"
                    "base directory and then register a new user.")
                    .arg(gUserManager.OwnerId(LICQ_PPID).c_str()).arg(BASE_DIR);
    InformUser(this, buf);
    return;
  }

  if (registerUserDlg != 0)
    registerUserDlg->raise();
  else
  {
    registerUserDlg = new RegisterUserDlg(server, sigman, this);
    connect(registerUserDlg, SIGNAL(signal_done(bool, char *, unsigned long)),
      this, SLOT(slot_doneregister(bool, char *, unsigned long)));
  }
}

void OwnerManagerDlg::slot_doneregister(bool bSuccess, char *szNewId, unsigned long nPPID)
{
  registerUserDlg = 0;
  
  if (bSuccess)
  {
    updateOwners();
    mainwin->callInfoTab(mnuUserGeneral, szNewId, nPPID);
  }
}

void OwnerManagerDlg::slot_doneRegisterUser(ICQEvent *e)
{
  // Deprecated
  delete registerUserDlg;
  registerUserDlg = NULL;
  
  if (e->Result() == EVENT_SUCCESS)
  {
    updateOwners();

    //TODO which owner
    QString id = gUserManager.OwnerId(LICQ_PPID);
    InformUser(this, tr("Successfully registered, your user identification\n"
                        "number (UIN) is %1.\n"
                        "Now set your personal information.").arg(id));
    mainwin->callInfoTab(mnuUserGeneral, id, LICQ_PPID);
  }
  else
  {
    InformUser(this, tr("Registration failed.  See network window for details."));
  }
}

void OwnerManagerDlg::slot_modifyClicked()
{
  modifyOwner(ownerView->selectedItem());
}

void OwnerManagerDlg::modifyOwner(QListViewItem* item)
{
  OwnerItem *i = dynamic_cast<OwnerItem *>(item);
  if (i == 0) return;

  OwnerEditDlg *d = new OwnerEditDlg(server, i->Id(), i->PPID(), this);
  d->show();
  connect(d, SIGNAL(destroyed()), this, SLOT(slot_update()));
}

void OwnerManagerDlg::slot_deleteClicked()
{
  OwnerItem *i = dynamic_cast<OwnerItem *>(ownerView->selectedItem());
  if (i == NULL) return;
  gUserManager.RemoveOwner(i->PPID());
  server->SaveConf();
  updateOwners();
}

void OwnerManagerDlg::slot_update()
{
  updateOwners();
}

#include "ownermanagerdlg.moc"
