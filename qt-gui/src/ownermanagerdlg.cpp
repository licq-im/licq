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

#include "ownermanagerdlg.h"
#include "ewidgets.h"
#include "licq_icqd.h"
#include "licq_user.h"

//----OwnerEditDlg-------------------------------------------------------------

OwnerEditDlg::OwnerEditDlg(CICQDaemon *s, const char *szId,
  unsigned long nPPID, QWidget *parent)
    : LicqDialog(parent, "OwnerEdit", false, WDestructiveClose)
{
  server = s;
  setCaption("Edit Owner");
  
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
      gUserManager.DropOwner((*it)->PPID());
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
      gUserManager.DropOwner(nPPID);
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
      InformUser(this, "Currently one one owner per protocol is supported.");
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
  if (edtId->text().isEmpty() || edtPassword->text().isEmpty())
    return;
    
  const char *szUser = edtId->text().latin1();
  const char *szPassword = edtPassword->text().latin1();
  const char *szProtocol = cmbProtocol->currentText().latin1();
  unsigned long nPPID = 0;
  
  ProtoPluginsList pl;
  ProtoPluginsListIter it;
  server->ProtoPluginList(pl);
  for (it = pl.begin(); it != pl.end(); it++)
  {
    if (strcmp(szProtocol, (*it)->Name()) == 0)
    {
      nPPID = (*it)->PPID();
      break;
    }
  }
  
  // Invalid protocol
  if (nPPID == 0)
    return;
  
  ICQOwner *o = gUserManager.FetchOwner(nPPID, LOCK_W);
  if (o)
  {
    o->SetPassword(szPassword);
    o->SetId(szUser);
  }
  else
  {
    gUserManager.AddOwner(szUser, nPPID);
    ICQOwner *o = gUserManager.FetchOwner(nPPID, LOCK_W);
    o->SetPassword(szPassword);
  }
  
  gUserManager.DropOwner(nPPID);
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

OwnerManagerDlg::OwnerManagerDlg(CICQDaemon *s)
  : LicqDialog(NULL, "OwnerDialog", false, WDestructiveClose)
{
  server = s;
  setCaption(tr("Licq - Owner Manager"));
  
  QBoxLayout *toplay = new QVBoxLayout(this, 8, 8); 
  
  // Add the list box
  ownerView = new OwnerView(this);
  toplay->addWidget(ownerView);
      
  // Add the buttons now
  QBoxLayout *lay = new QHBoxLayout(toplay, 10);
  btnAdd = new QPushButton(tr("&Add"), this);
  lay->addWidget(btnAdd);
  btnModify = new QPushButton(tr("&Modify"), this);
  btnModify->setEnabled(false);
  lay->addWidget(btnModify);
  btnDelete = new QPushButton(tr("D&elete"), this);
  btnDelete->setEnabled(false);
  lay->addWidget(btnDelete);
  btnDone = new QPushButton(tr("&Done"), this);
  lay->addWidget(btnDone);
  
  // Connect all the signals
  connect(ownerView, SIGNAL(clicked(QListViewItem *, const QPoint &, int)),
    this, SLOT(slot_listClicked(QListViewItem *, const QPoint &, int)));
  connect(btnAdd, SIGNAL(clicked()), this, SLOT(slot_addClicked()));
  connect(btnModify, SIGNAL(clicked()), this, SLOT(slot_modifyClicked()));
  connect(btnDelete, SIGNAL(clicked()), this, SLOT(slot_deleteClicked()));
  connect(btnDone, SIGNAL(clicked()), this, SLOT(close()));
    
  // Add the owners to the list now
  updateOwners();
}

void OwnerManagerDlg::updateOwners()
{
  ownerView->clear();
  
  FOR_EACH_OWNER_START(LOCK_R)
  {
    (void) new OwnerItem(server, pOwner->IdString(), pOwner->PPID(), ownerView);
  }
  FOR_EACH_OWNER_END
}

void OwnerManagerDlg::slot_listClicked(QListViewItem *it, const QPoint &, int)
{
  btnModify->setEnabled(it);
  btnDelete->setEnabled(it);
}

void OwnerManagerDlg::slot_addClicked()
{
  OwnerEditDlg *d = new OwnerEditDlg(server, 0, 0, this);
  d->show();
  connect(d, SIGNAL(destroyed()), this, SLOT(slot_update()));
}

void OwnerManagerDlg::slot_modifyClicked()
{
  OwnerItem *i = dynamic_cast<OwnerItem *>(ownerView->selectedItem());
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
