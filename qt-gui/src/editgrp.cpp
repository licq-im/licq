#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "editgrp.h"
#include "user.h"
#include "licq-locale.h"

EditGrpDlg::EditGrpDlg(QWidget *parent = 0, const char *name = 0 ) : QWidget(parent, name)
{
  grpGroups = new QGroupBox(_("Groups"), this);
  lstGroups = new QListBox(grpGroups);
  btnAdd = new QPushButton(_("Add"), grpGroups);
  btnRemove = new QPushButton(_("Remove"), grpGroups);
  btnUp = new QPushButton(_("Shift Up"), grpGroups);
  btnDown = new QPushButton(_("Shift Down"), grpGroups);
  btnEdit = new QPushButton(_("Edit Name"), grpGroups);
  btnDefault = new QPushButton(_("Default"), grpGroups);
  btnDone = new QPushButton(_("Done"), this);
  nfoDefault = new CInfoField(0, 0, 5, 5, 5, "Default:", true, grpGroups);
  edtName = new QLineEdit(grpGroups);
  edtName->setEnabled(false);

  resize(300, 320);
  RefreshList();

  connect(btnAdd, SIGNAL(clicked()), this, SLOT(slot_add()));
  connect(btnRemove, SIGNAL(clicked()), this, SLOT(slot_remove()));
  connect(btnUp, SIGNAL(clicked()), this, SLOT(slot_up()));
  connect(btnDown, SIGNAL(clicked()), this, SLOT(slot_down()));
  connect(btnDefault, SIGNAL(clicked()), this, SLOT(slot_default()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
  connect(btnDone, SIGNAL(clicked()), this, SLOT(hide()));
  connect(edtName, SIGNAL(returnPressed()), this, SLOT(slot_editok()));
}


void EditGrpDlg::RefreshList()
{
  lstGroups->clear();
  CUserGroup *g;
  for (unsigned short i = 0; i < gUserManager.NumGroups(); i++)
  {
    g = gUserManager.FetchGroup(i, LOCK_R);
    lstGroups->insertItem(g->Name());
    gUserManager.DropGroup(g);
  }
  g = gUserManager.FetchGroup(gUserManager.DefaultGroup(), LOCK_R);
  nfoDefault->setData(g->Name());
  gUserManager.DropGroup(g);
}

void EditGrpDlg::slot_add()
{
  gUserManager.AddGroup(new CUserGroup(_("noname"), false));
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_remove()
{
  int n = lstGroups->currentItem();
  if (n < 2) return;
  CUserGroup *g = gUserManager.FetchGroup(n, LOCK_R);
  char warning[256];
  sprintf(warning, _("Are you sure you want to remove\nthe group \"%s\"?"), g->Name());
  gUserManager.DropGroup(g);
  if(!QueryUser(this, warning, _("Ok"), _("Cancel"))) return;
  gUserManager.RemoveGroup(n);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_up()
{
  int n = lstGroups->currentItem();
  if (n == -1 ) return;
  gUserManager.SwapGroups(n, n - 1);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_down()
{
  int n = lstGroups->currentItem();
  if (n == -1 ) return;
  gUserManager.SwapGroups(n, n + 1);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_default()
{
  int n = lstGroups->currentItem();
  if (n == -1 ) return;
  gUserManager.SetDefaultGroup(n);
  RefreshList();
  emit (signal_updateGroups());
}


void EditGrpDlg::slot_edit()
{
  int n = lstGroups->currentItem();
  if (n == -1 ) return;
  edtName->setEnabled(true);
  CUserGroup *g = gUserManager.FetchGroup(n, LOCK_R);
  if (g == NULL) return;
  edtName->setText(g->Name());
  gUserManager.DropGroup(g);
  m_nEditGrp = n;
  btnEdit->setText(_("Cancel"));
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
  btnEdit->setText(_("Edit"));
  edtName->clear();
  edtName->setEnabled(false);
  disconnect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_editok()));
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slot_edit()));
}


void EditGrpDlg::slot_editcancel()
{
  lstGroups->setEnabled(true);
  btnEdit->setText(_("Edit"));
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


void EditGrpDlg::resizeEvent (QResizeEvent *)
{
  grpGroups->setGeometry(10, 10, width() - 20, height() - 60);
  lstGroups->setGeometry(10, 20, grpGroups->width() - 120, grpGroups->height() - 85);
  btnAdd->setGeometry(grpGroups->width() - 100, 20, 80, 30);
  btnRemove->setGeometry(btnAdd->x(), btnAdd->y() + 40, 80, 30);
  btnUp->setGeometry(btnAdd->x(), btnRemove->y() + 40, 80, 30);
  btnDown->setGeometry(btnAdd->x(), btnUp->y() + 40, 80, 30);
  btnEdit->setGeometry(btnAdd->x(), btnDown->y() + 40, 80, 30);
  btnDefault->setGeometry(btnAdd->x(), btnEdit->y() + 40, 80, 30);
  edtName->setGeometry(10, grpGroups->height() - 55, grpGroups->width() - 120, 20);
  nfoDefault->setGeometry(10, grpGroups->height() - 30, 50, 5, grpGroups->width() - 175);

  btnDone->setGeometry(width() / 2 - 50, height() - 40, 100, 30);
}

#include "moc/moc_editgrp.h"
