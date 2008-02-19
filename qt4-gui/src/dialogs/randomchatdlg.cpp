// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#include "randomchatdlg.h"

#include "config.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq_events.h>
#include <licq_icqd.h>

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::RandomChatDlg */
/* TRANSLATOR LicqQtGui::SetRandomChatGroupDlg */

RandomChatDlg::RandomChatDlg(QWidget* p)
  : QDialog(p),
    tag(0)
{
  Support::setWidgetProps(this, "RandomChatDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Random Chat Search"));

  QVBoxLayout* top_lay = new QVBoxLayout(this);
  lstGroups = new QListWidget(this);
  top_lay->addWidget(lstGroups);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  top_lay->addWidget(buttons);
  btnOk = buttons->addButton(QDialogButtonBox::Ok);
  btnOk->setText(tr("&Search"));
  btnCancel = buttons->addButton(QDialogButtonBox::Cancel);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));

  // Fill in the combo box
  lstGroups->addItem(tr("General"));
  lstGroups->addItem(tr("Romance"));
  lstGroups->addItem(tr("Games"));
  lstGroups->addItem(tr("Students"));
  lstGroups->addItem(tr("20 Something"));
  lstGroups->addItem(tr("30 Something"));
  lstGroups->addItem(tr("40 Something"));
  lstGroups->addItem(tr("50 Plus"));
  lstGroups->addItem(tr("Seeking Women"));
  lstGroups->addItem(tr("Seeking Men"));

  lstGroups->setCurrentRow(0);

  show();
}


RandomChatDlg::~RandomChatDlg()
{
  if (tag != 0)
    gLicqDaemon->CancelEvent(tag);
}


void RandomChatDlg::slot_ok()
{
  btnOk->setEnabled(false);
  connect(LicqGui::instance()->signalManager(),
      SIGNAL(doneUserFcn(ICQEvent*)), SLOT(slot_doneUserFcn(ICQEvent*)));
  unsigned long nGroup = ICQ_RANDOMxCHATxGROUP_NONE;
  switch(lstGroups->currentRow())
  {
    case 0: nGroup = ICQ_RANDOMxCHATxGROUP_GENERAL; break;
    case 1: nGroup = ICQ_RANDOMxCHATxGROUP_ROMANCE; break;
    case 2: nGroup = ICQ_RANDOMxCHATxGROUP_GAMES; break;
    case 3: nGroup = ICQ_RANDOMxCHATxGROUP_STUDENTS; break;
    case 4: nGroup = ICQ_RANDOMxCHATxGROUP_20SOME; break;
    case 5: nGroup = ICQ_RANDOMxCHATxGROUP_30SOME; break;
    case 6: nGroup = ICQ_RANDOMxCHATxGROUP_40SOME; break;
    case 7: nGroup = ICQ_RANDOMxCHATxGROUP_50PLUS; break;
    case 8: nGroup = ICQ_RANDOMxCHATxGROUP_MxSEEKxF; break;
    case 9: nGroup = ICQ_RANDOMxCHATxGROUP_FxSEEKxM; break;
  }
  tag = gLicqDaemon->icqRandomChatSearch(nGroup);
  setWindowTitle(tr("Searching for Random Chat Partner..."));
}


void RandomChatDlg::slot_doneUserFcn(ICQEvent* e)
{
  if (!e->Equals(tag)) return;

  btnOk->setEnabled(true);
  tag = 0;

  switch (e->Result())
  {
  case EVENT_FAILED:
    WarnUser(this, tr("No random chat user found in that group."));
    break;
  case EVENT_TIMEDOUT:
    WarnUser(this, tr("Random chat search timed out."));
    break;
  case EVENT_ERROR:
    WarnUser(this, tr("Random chat search had an error."));
    break;
  default:
    //TODO when CSearchAck changes
    LicqGui::instance()->showEventDialog(mnuUserSendChat, e->SearchAck()->Id(),
                          e->SearchAck()->PPID() );
    close();
    return;
  }

}

//=====SetRandomChatGroupDlg================================================

SetRandomChatGroupDlg::SetRandomChatGroupDlg(QWidget* p)
  : QDialog(p),
    tag(0)
{
  Support::setWidgetProps(this, "SetRandomChatGroupDlg");
  setWindowTitle(tr("Set Random Chat Group"));

  QVBoxLayout* top_lay = new QVBoxLayout(this);
  lstGroups = new QListWidget(this);
  top_lay->addWidget(lstGroups);

  QHBoxLayout* lay = new QHBoxLayout();

  lay->addStretch(2);
  btnOk = new QPushButton(tr("&Set"), this);
  lay->addWidget(btnOk);

  lay->addSpacing(10);
  btnCancel = new QPushButton(tr("&Close"), this);
  lay->addWidget(btnCancel);
  lay->addStretch(2);

  top_lay->addLayout(lay);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));

  // Fill in the combo box
  lstGroups->addItem(tr("(none)"));
  lstGroups->addItem(tr("General"));
  lstGroups->addItem(tr("Romance"));
  lstGroups->addItem(tr("Games"));
  lstGroups->addItem(tr("Students"));
  lstGroups->addItem(tr("20 Something"));
  lstGroups->addItem(tr("30 Something"));
  lstGroups->addItem(tr("40 Something"));
  lstGroups->addItem(tr("50 Plus"));
  lstGroups->addItem(tr("Seeking Women"));
  lstGroups->addItem(tr("Seeking Men"));

  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == 0)
  {
    close();
    return;
  }
  switch(o->RandomChatGroup())
  {
    case ICQ_RANDOMxCHATxGROUP_GENERAL: lstGroups->setCurrentRow(1); break;
    case ICQ_RANDOMxCHATxGROUP_ROMANCE: lstGroups->setCurrentRow(2); break;
    case ICQ_RANDOMxCHATxGROUP_GAMES: lstGroups->setCurrentRow(3); break;
    case ICQ_RANDOMxCHATxGROUP_STUDENTS: lstGroups->setCurrentRow(4); break;
    case ICQ_RANDOMxCHATxGROUP_20SOME: lstGroups->setCurrentRow(5); break;
    case ICQ_RANDOMxCHATxGROUP_30SOME: lstGroups->setCurrentRow(6); break;
    case ICQ_RANDOMxCHATxGROUP_40SOME: lstGroups->setCurrentRow(7); break;
    case ICQ_RANDOMxCHATxGROUP_50PLUS: lstGroups->setCurrentRow(8); break;
    case ICQ_RANDOMxCHATxGROUP_MxSEEKxF: lstGroups->setCurrentRow(9); break;
    case ICQ_RANDOMxCHATxGROUP_FxSEEKxM: lstGroups->setCurrentRow(10); break;
    case ICQ_RANDOMxCHATxGROUP_NONE:
    default:
      lstGroups->setCurrentRow(0); break;
  }
  gUserManager.DropOwner();

  show();
}


SetRandomChatGroupDlg::~SetRandomChatGroupDlg()
{
  if (tag != 0)
    gLicqDaemon->CancelEvent(tag);
}


void SetRandomChatGroupDlg::slot_ok()
{
  btnOk->setEnabled(false);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  connect(LicqGui::instance()->signalManager(),
      SIGNAL(doneUserFcn(ICQEvent*)), SLOT(slot_doneUserFcn(ICQEvent*)));
  unsigned long nGroup = ICQ_RANDOMxCHATxGROUP_NONE;
  switch(lstGroups->currentRow())
  {
    case 0: nGroup = ICQ_RANDOMxCHATxGROUP_NONE; break;
    case 1: nGroup = ICQ_RANDOMxCHATxGROUP_GENERAL; break;
    case 2: nGroup = ICQ_RANDOMxCHATxGROUP_ROMANCE; break;
    case 3: nGroup = ICQ_RANDOMxCHATxGROUP_GAMES; break;
    case 4: nGroup = ICQ_RANDOMxCHATxGROUP_STUDENTS; break;
    case 5: nGroup = ICQ_RANDOMxCHATxGROUP_20SOME; break;
    case 6: nGroup = ICQ_RANDOMxCHATxGROUP_30SOME; break;
    case 7: nGroup = ICQ_RANDOMxCHATxGROUP_40SOME; break;
    case 8: nGroup = ICQ_RANDOMxCHATxGROUP_50PLUS; break;
    case 9: nGroup = ICQ_RANDOMxCHATxGROUP_MxSEEKxF; break;
    case 10: nGroup = ICQ_RANDOMxCHATxGROUP_FxSEEKxM; break;
  }
  tag = gLicqDaemon->icqSetRandomChatGroup(nGroup);
  setWindowTitle(tr("Setting Random Chat Group..."));
}


void SetRandomChatGroupDlg::slot_doneUserFcn(ICQEvent* e)
{
  if (!e->Equals(tag)) return;

  btnOk->setEnabled(true);
  btnCancel = new QPushButton(tr("&Close"), this);
  tag = 0;

  switch (e->Result())
  {
  case EVENT_FAILED:
    setWindowTitle(windowTitle() + tr("failed"));
    break;
  case EVENT_TIMEDOUT:
    setWindowTitle(windowTitle() + tr("timed out"));
    break;
  case EVENT_ERROR:
    setWindowTitle(windowTitle() + tr("error"));
    break;
  default:
    setWindowTitle(windowTitle() + tr("done"));
    break;
  }

}
