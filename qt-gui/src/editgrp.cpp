#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <qwhatsthis.h>

#include "editgrp.h"
#include "user.h"

EditGrpDlg::EditGrpDlg(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  grpGroups = new QGroupBox(tr("Groups"), this);
  lstGroups = new QListBox(grpGroups);
  btnAdd = new QPushButton(tr("Add"), grpGroups);
  btnRemove = new QPushButton(tr("Remove"), grpGroups);
  btnUp = new QPushButton(tr("Shift Up"), grpGroups);
  btnDown = new QPushButton(tr("Shift Down"), grpGroups);
  btnEdit = new QPushButton(tr("Edit Name"), grpGroups);
  QWhatsThis::add(btnEdit, tr("Edit group name (hit enter to save)."));
  btnDefault = new QPushButton(tr("Default"), grpGroups);
  QWhatsThis::add(btnDefault, tr("The default group to start up in."));
  btnNewUser = new QPushButton(tr("New Users"), grpGroups);
  QWhatsThis::add(btnNewUser, tr("The group to which new users will be automatically added."));
  btnDone = new QPushButton(tr("Done"), this);
  nfoDefault = new CInfoField(0, 0, 5, 5, 5, "Default:", true, grpGroups);
  nfoNewUser = new CInfoField(0, 0, 5, 5, 5, "New User:", true, grpGroups);
  edtName = new QLineEdit(grpGroups);
  edtName->setEnabled(false);

  btnWhat = new QPushButton(tr("What's This?"), this);
  connect(btnWhat, SIGNAL(clicked()), this, SLOT(slot_whatsthis()));

  resize(320, 360);
  RefreshList();

  connect(btnAdd, SIGNAL(clicked()), this, SLOT(slot_add()));
  connect(btnRemove, SIGNAL(clicked()), this, SLOT(slot_remove()));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(slot_up()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(slot_down()));
  connect(btnDefault, SIGNAL(clicked()), this, SLOT(slot_default()));
  connect(btnNewUser, SIGNAL(clicked()), this, SLOT(slot_newuser()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnDone, SIGNAL(clicked()), this, SLOT(hide()));
  connect(edtName, SIGNAL(returnPressed()), this, SLOT(slot_editok()));
}


void EditGrpDlg::RefreshList()
{
  lstGroups->clear();
  lstGroups->insertItem(tr("All Users"));
  if (gUserManager.DefaultGroup() == 0)
    nfoDefault->setData(tr("All Users"));
  if (gUserManager.NewUserGroup() == 0)
    nfoNewUser->setData(tr("All Users"));
  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  for (unsigned short i = 0; i < g->size(); i++)
  {
    lstGroups->insertItem((*g)[i]);
    if (i + 1 == gUserManager.DefaultGroup())
      nfoDefault->setData((*g)[i]);
    if (i + 1== gUserManager.NewUserGroup())
      nfoNewUser->setData((*g)[i]);
  }
  gUserManager.UnlockGroupList();
}

void EditGrpDlg::slot_add()
{
  gUserManager.AddGroup(strdup(tr("noname")));
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_remove()
{
  int n = lstGroups->currentItem() - 1;
  if (n < 0) return;
  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  QString warning(tr("Are you sure you want to remove\nthe group '") +
                  QString::fromLocal8Bit((*g)[n]) + "'?");
  gUserManager.UnlockGroupList();
  if(!QueryUser(this, warning, tr("Ok"), tr("Cancel")));
    return;
  gUserManager.RemoveGroup(n + 1);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_up()
{
  int n = lstGroups->currentItem() - 1;
  if (n <= 0) return;
  gUserManager.SwapGroups(n + 1, n);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_down()
{
  int n = lstGroups->currentItem() - 1;
  if (n < 0 /* || n == max */) return;
  gUserManager.SwapGroups(n + 1, n + 2);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_default()
{
  if (QueryUser(this, tr("Clear or Set?"), tr("Clear"), tr("Set")))
  {
    gUserManager.SetDefaultGroup(0);
  }
  else
  {
    int n = lstGroups->currentItem();
    if (n == -1) return;
    gUserManager.SetDefaultGroup(n);
  }
  RefreshList();
  //emit (signal_updateGroups());
}


void EditGrpDlg::slot_newuser()
{
  if (QueryUser(this, tr("Clear or Set?"), tr("Clear"), tr("Set")))
  {
    gUserManager.SetNewUserGroup(0);
  }
  else
  {
    int n = lstGroups->currentItem();
    if (n == -1 ) return;
    gUserManager.SetNewUserGroup(n);
  }
  RefreshList();
  //emit (signal_updateGroups());
}


void EditGrpDlg::slot_edit()
{
  int n = lstGroups->currentItem() - 1;
  if (n < 0) return;
  edtName->setEnabled(true);
  GroupList *g = gUserManager.LockGroupList(LOCK_R);
  edtName->setText((*g)[n]);
  gUserManager.UnlockGroupList();
  m_nEditGrp = n + 1;
  btnEdit->setText(tr("Cancel"));
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editcancel()));
  lstGroups->setEnabled(false);
}


void EditGrpDlg::slot_editok()
{
  gUserManager.RenameGroup(m_nEditGrp, edtName->text());
  RefreshList();
  emit (signal_updateGroups());

  lstGroups->setEnabled(true);
  btnEdit->setText(tr("Edit"));
  edtName->clear();
  edtName->setEnabled(false);
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editok()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
}


void EditGrpDlg::slot_editcancel()
{
  lstGroups->setEnabled(true);
  btnEdit->setText(tr("Edit"));
  edtName->clear();
  edtName->setEnabled(false);
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editcancel()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
}

void EditGrpDlg::hide()
{
   QWidget::hide();
   delete this;
}


void EditGrpDlg::slot_whatsthis()
{
  QWhatsThis::enterWhatsThisMode();
}


void EditGrpDlg::resizeEvent (QResizeEvent *)
{
  grpGroups->setGeometry(10, 10, width() - 20, height() - 60);
  lstGroups->setGeometry(10, 20, grpGroups->width() - 120, grpGroups->height() - 115);
  btnAdd->setGeometry(grpGroups->width() - 100, 20, 80, 30);
  btnRemove->setGeometry(btnAdd->x(), btnAdd->y() + 40, 80, 30);
  btnUp->setGeometry(btnAdd->x(), btnRemove->y() + 40, 80, 30);
  btnDown->setGeometry(btnAdd->x(), btnUp->y() + 40, 80, 30);
  btnEdit->setGeometry(btnAdd->x(), btnDown->y() + 40, 80, 30);
  btnDefault->setGeometry(btnAdd->x(), btnEdit->y() + 40, 80, 30);
  btnNewUser->setGeometry(btnAdd->x(), btnDefault->y() + 40, 80, 30);
  edtName->setGeometry(10, grpGroups->height() - 80, grpGroups->width() - 120, 20);
  nfoDefault->setGeometry(10, grpGroups->height() - 55, 70, 5, grpGroups->width() - 195);
  nfoNewUser->setGeometry(10, grpGroups->height() - 30, 70, 5, grpGroups->width() - 195);

  btnWhat->setGeometry(10, height() - 38, 90, 26);
  btnDone->setGeometry((width() - 90) / 2 + 40, height() - 40, 100, 30);
}

#include "moc/moc_editgrp.h"
