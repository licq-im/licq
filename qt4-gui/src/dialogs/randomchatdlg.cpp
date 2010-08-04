// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2010 Licq developers
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

#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>
#include <licq/daemon.h>
#include <licq/event.h>
#include <licq/icq.h>
#include <licq/icqdefines.h>

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::RandomChatDlg */
/* TRANSLATOR LicqQtGui::SetRandomChatGroupDlg */

RandomChatDlg::RandomChatDlg(QWidget* parent)
  : QDialog(parent),
    myTag(0)
{
  Support::setWidgetProps(this, "RandomChatDialog");
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("Licq - Random Chat Search"));

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  myGroupsList = new QListWidget(this);
  topLayout->addWidget(myGroupsList);

  QDialogButtonBox* buttons = new QDialogButtonBox();
  topLayout->addWidget(buttons);
  myOkButton = buttons->addButton(QDialogButtonBox::Ok);
  myOkButton->setText(tr("&Search"));
  myCancelButton = buttons->addButton(QDialogButtonBox::Cancel);

  connect(myOkButton, SIGNAL(clicked()), SLOT(okPressed()));
  connect(myCancelButton, SIGNAL(clicked()), SLOT(close()));

  // Fill in the combo box
  myGroupsList->addItem(tr("General"));
  myGroupsList->addItem(tr("Romance"));
  myGroupsList->addItem(tr("Games"));
  myGroupsList->addItem(tr("Students"));
  myGroupsList->addItem(tr("20 Something"));
  myGroupsList->addItem(tr("30 Something"));
  myGroupsList->addItem(tr("40 Something"));
  myGroupsList->addItem(tr("50 Plus"));
  myGroupsList->addItem(tr("Seeking Women"));
  myGroupsList->addItem(tr("Seeking Men"));

  myGroupsList->setCurrentRow(0);

  show();
}

RandomChatDlg::~RandomChatDlg()
{
  if (myTag != 0)
    Licq::gDaemon.cancelEvent(myTag);
}

void RandomChatDlg::okPressed()
{
  myOkButton->setEnabled(false);
  connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
      SLOT(userEventDone(const Licq::Event*)));
  unsigned long nGroup = ICQ_RANDOMxCHATxGROUP_NONE;
  switch(myGroupsList->currentRow())
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
  myTag = gLicqDaemon->icqRandomChatSearch(nGroup);
  setWindowTitle(tr("Searching for Random Chat Partner..."));
}

void RandomChatDlg::userEventDone(const Licq::Event* event)
{
  if (!event->Equals(myTag))
    return;

  myOkButton->setEnabled(true);
  myTag = 0;

  switch (event->Result())
  {
    case Licq::Event::ResultFailed:
      WarnUser(this, tr("No random chat user found in that group."));
      break;
    case Licq::Event::ResultTimedout:
      WarnUser(this, tr("Random chat search timed out."));
      break;
    case Licq::Event::ResultError:
      WarnUser(this, tr("Random chat search had an error."));
      break;
    default:
      //TODO when CSearchAck changes
      Licq::UserId userId = event->SearchAck()->userId();
      Licq::gUserManager.addUser(userId, false);
      gLicqGui->showEventDialog(ChatEvent, userId);
      close();
      return;
  }

}

//=====SetRandomChatGroupDlg================================================

SetRandomChatGroupDlg::SetRandomChatGroupDlg(QWidget* parent)
  : QDialog(parent),
    myTag(0)
{
  Support::setWidgetProps(this, "SetRandomChatGroupDlg");
  setWindowTitle(tr("Set Random Chat Group"));

  QVBoxLayout* topLayout = new QVBoxLayout(this);
  myGroupsList = new QListWidget(this);
  topLayout->addWidget(myGroupsList);

  QHBoxLayout* lay = new QHBoxLayout();

  lay->addStretch(2);
  myOkButton = new QPushButton(tr("&Set"), this);
  lay->addWidget(myOkButton);

  lay->addSpacing(10);
  myCancelButton = new QPushButton(tr("&Close"), this);
  lay->addWidget(myCancelButton);
  lay->addStretch(2);

  topLayout->addLayout(lay);

  connect(myOkButton, SIGNAL(clicked()), SLOT(okPressed()));
  connect(myCancelButton, SIGNAL(clicked()), SLOT(close()));

  // Fill in the combo box
  myGroupsList->addItem(tr("(none)"));
  myGroupsList->addItem(tr("General"));
  myGroupsList->addItem(tr("Romance"));
  myGroupsList->addItem(tr("Games"));
  myGroupsList->addItem(tr("Students"));
  myGroupsList->addItem(tr("20 Something"));
  myGroupsList->addItem(tr("30 Something"));
  myGroupsList->addItem(tr("40 Something"));
  myGroupsList->addItem(tr("50 Plus"));
  myGroupsList->addItem(tr("Seeking Women"));
  myGroupsList->addItem(tr("Seeking Men"));

  Licq::OwnerReadGuard o(LICQ_PPID);
  if (!o.isLocked())
  {
    close();
    return;
  }
  switch(o->RandomChatGroup())
  {
    case ICQ_RANDOMxCHATxGROUP_GENERAL: myGroupsList->setCurrentRow(1); break;
    case ICQ_RANDOMxCHATxGROUP_ROMANCE: myGroupsList->setCurrentRow(2); break;
    case ICQ_RANDOMxCHATxGROUP_GAMES: myGroupsList->setCurrentRow(3); break;
    case ICQ_RANDOMxCHATxGROUP_STUDENTS: myGroupsList->setCurrentRow(4); break;
    case ICQ_RANDOMxCHATxGROUP_20SOME: myGroupsList->setCurrentRow(5); break;
    case ICQ_RANDOMxCHATxGROUP_30SOME: myGroupsList->setCurrentRow(6); break;
    case ICQ_RANDOMxCHATxGROUP_40SOME: myGroupsList->setCurrentRow(7); break;
    case ICQ_RANDOMxCHATxGROUP_50PLUS: myGroupsList->setCurrentRow(8); break;
    case ICQ_RANDOMxCHATxGROUP_MxSEEKxF: myGroupsList->setCurrentRow(9); break;
    case ICQ_RANDOMxCHATxGROUP_FxSEEKxM: myGroupsList->setCurrentRow(10); break;
    case ICQ_RANDOMxCHATxGROUP_NONE:
    default:
      myGroupsList->setCurrentRow(0); break;
  }

  show();
}

SetRandomChatGroupDlg::~SetRandomChatGroupDlg()
{
  if (myTag != 0)
    Licq::gDaemon.cancelEvent(myTag);
}

void SetRandomChatGroupDlg::okPressed()
{
  myOkButton->setEnabled(false);
  myCancelButton = new QPushButton(tr("&Cancel"), this);
  connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
      SLOT(userEventDone(const Licq::Event*)));
  unsigned long nGroup = ICQ_RANDOMxCHATxGROUP_NONE;
  switch(myGroupsList->currentRow())
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
  myTag = gLicqDaemon->icqSetRandomChatGroup(nGroup);
  setWindowTitle(tr("Setting Random Chat Group..."));
}

void SetRandomChatGroupDlg::userEventDone(const Licq::Event* event)
{
  if (!event->Equals(myTag))
    return;

  myOkButton->setEnabled(true);
  myCancelButton = new QPushButton(tr("&Close"), this);
  myTag = 0;

  switch (event->Result())
  {
    case Licq::Event::ResultFailed:
      setWindowTitle(windowTitle() + tr("failed"));
      break;
    case Licq::Event::ResultTimedout:
      setWindowTitle(windowTitle() + tr("timed out"));
      break;
    case Licq::Event::ResultError:
      setWindowTitle(windowTitle() + tr("error"));
      break;
    default:
      setWindowTitle(windowTitle() + tr("done"));
      break;
  }
}
