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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlistbox.h>

#include "randomchatdlg.h"
#include "sigman.h"
#include "mainwin.h"
#include "licq_icqd.h"
#include "licq_events.h"
#include "gui-defines.h"
#include "ewidgets.h"

//=====CRandomChatDlg========================================================

CRandomChatDlg::CRandomChatDlg(CMainWindow *_mainwin, CICQDaemon *s,
                               CSignalManager *_sigman, QWidget* p)
  : LicqDialog(p, "RandomChatDialog", false, WDestructiveClose)
{
  mainwin = _mainwin;
  server = s;
  sigman = _sigman;
  tag = 0;

  setCaption(tr("Random Chat Search"));

  QGridLayout *lay = new QGridLayout(this, 2, 5, 10, 5);
  lstGroups = new QListBox(this);
  lay->addMultiCellWidget(lstGroups, 0, 0, 0, 4);

  lay->setColStretch(0, 2);
  btnOk = new QPushButton(tr("&Search"), this);
  lay->addWidget(btnOk, 1, 1);

  lay->addColSpacing(2, 10);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  lay->addWidget(btnCancel, 1, 3);
  lay->setColStretch(4, 2);

  int bw = 75;
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));

  // Fill in the combo box
  lstGroups->insertItem(tr("General"));
  lstGroups->insertItem(tr("Romance"));
  lstGroups->insertItem(tr("Games"));
  lstGroups->insertItem(tr("Students"));
  lstGroups->insertItem(tr("20 Something"));
  lstGroups->insertItem(tr("30 Something"));
  lstGroups->insertItem(tr("40 Something"));
  lstGroups->insertItem(tr("50 Plus"));
  lstGroups->insertItem(tr("Seeking Women"));
  lstGroups->insertItem(tr("Seeking Men"));

  show();
}


CRandomChatDlg::~CRandomChatDlg()
{
  if (tag != 0)
    server->CancelEvent(tag);
}


void CRandomChatDlg::slot_ok()
{
  if (lstGroups->currentItem() == -1) return;

  btnOk->setEnabled(false);
  QObject::connect(sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)),
                   this, SLOT(slot_doneUserFcn(ICQEvent *)));
  unsigned long nGroup = ICQ_RANDOMxCHATxGROUP_NONE;
  switch(lstGroups->currentItem())
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
  tag = server->icqRandomChatSearch(nGroup);
  setCaption(tr("Searching for Random Chat Partner..."));
}


void CRandomChatDlg::slot_doneUserFcn(ICQEvent *e)
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
    mainwin->callFunction(mnuUserSendChat, e->SearchAck()->Id(),
                          e->SearchAck()->PPID() );
    close();
    return;
  }

}

//=====CSetRandomChatGroupDlg================================================

CSetRandomChatGroupDlg::CSetRandomChatGroupDlg(CICQDaemon *s,
                               CSignalManager *_sigman, QWidget* p)
  : LicqDialog(p, "SetRandomChatGroupDlg")
{
  server = s;
  sigman = _sigman;
  tag = 0;

  setCaption(tr("Set Random Chat Group"));

  QGridLayout *lay = new QGridLayout(this, 2, 5, 10, 5);
  lstGroups = new QListBox(this);
  lay->addMultiCellWidget(lstGroups, 0, 0, 0, 4);

  lay->setColStretch(0, 2);
  btnOk = new QPushButton(tr("&Set"), this);
  lay->addWidget(btnOk, 1, 1);

  lay->addColSpacing(2, 10);
  btnCancel = new QPushButton(tr("&Close"), this);
  lay->addWidget(btnCancel, 1, 3);
  lay->setColStretch(4, 2);

  int bw = 75;
  bw = QMAX(bw, btnOk->sizeHint().width());
  bw = QMAX(bw, btnCancel->sizeHint().width());
  btnOk->setFixedWidth(bw);
  btnCancel->setFixedWidth(bw);

  connect(btnOk, SIGNAL(clicked()), SLOT(slot_ok()));
  connect(btnCancel, SIGNAL(clicked()), SLOT(close()));

  // Fill in the combo box
  lstGroups->insertItem(tr("(none)"));
  lstGroups->insertItem(tr("General"));
  lstGroups->insertItem(tr("Romance"));
  lstGroups->insertItem(tr("Games"));
  lstGroups->insertItem(tr("Students"));
  lstGroups->insertItem(tr("20 Something"));
  lstGroups->insertItem(tr("30 Something"));
  lstGroups->insertItem(tr("40 Something"));
  lstGroups->insertItem(tr("50 Plus"));
  lstGroups->insertItem(tr("Seeking Women"));
  lstGroups->insertItem(tr("Seeking Men"));

  ICQOwner *o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == 0)
  {
    close();
    return;
  }
  switch(o->RandomChatGroup())
  {
    case ICQ_RANDOMxCHATxGROUP_GENERAL: lstGroups->setCurrentItem(1); break;
    case ICQ_RANDOMxCHATxGROUP_ROMANCE: lstGroups->setCurrentItem(2); break;
    case ICQ_RANDOMxCHATxGROUP_GAMES: lstGroups->setCurrentItem(3); break;
    case ICQ_RANDOMxCHATxGROUP_STUDENTS: lstGroups->setCurrentItem(4); break;
    case ICQ_RANDOMxCHATxGROUP_20SOME: lstGroups->setCurrentItem(5); break;
    case ICQ_RANDOMxCHATxGROUP_30SOME: lstGroups->setCurrentItem(6); break;
    case ICQ_RANDOMxCHATxGROUP_40SOME: lstGroups->setCurrentItem(7); break;
    case ICQ_RANDOMxCHATxGROUP_50PLUS: lstGroups->setCurrentItem(8); break;
    case ICQ_RANDOMxCHATxGROUP_MxSEEKxF: lstGroups->setCurrentItem(9); break;
    case ICQ_RANDOMxCHATxGROUP_FxSEEKxM: lstGroups->setCurrentItem(10); break;
    case ICQ_RANDOMxCHATxGROUP_NONE:
    default:
      lstGroups->setCurrentItem(0); break;
  }
  gUserManager.DropOwner(o);

  show();
}


CSetRandomChatGroupDlg::~CSetRandomChatGroupDlg()
{
  if (tag != 0)
    server->CancelEvent(tag);
}


void CSetRandomChatGroupDlg::slot_ok()
{
  if (lstGroups->currentItem() == -1) return;

  btnOk->setEnabled(false);
  btnCancel = new QPushButton(tr("&Cancel"), this);
  QObject::connect(sigman, SIGNAL(signal_doneUserFcn(ICQEvent *)),
                   this, SLOT(slot_doneUserFcn(ICQEvent *)));
  unsigned long nGroup = ICQ_RANDOMxCHATxGROUP_NONE;
  switch(lstGroups->currentItem())
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
  tag = server->icqSetRandomChatGroup(nGroup);
  setCaption(tr("Setting Random Chat Group..."));
}


void CSetRandomChatGroupDlg::slot_doneUserFcn(ICQEvent *e)
{
  if (!e->Equals(tag)) return;

  btnOk->setEnabled(true);
  btnCancel = new QPushButton(tr("&Close"), this);
  tag = 0;

  switch (e->Result())
  {
  case EVENT_FAILED:
    setCaption(caption() + tr("failed"));
    break;
  case EVENT_TIMEDOUT:
    setCaption(caption() + tr("timed out"));
    break;
  case EVENT_ERROR:
    setCaption(caption() + tr("error"));
    break;
  default:
    setCaption(caption() + tr("done"));
    break;
  }

}

#include "randomchatdlg.moc"
