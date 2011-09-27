/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2011 Licq developers
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
#include <licq/icq/icq.h>

#include "core/gui-defines.h"
#include "core/licqgui.h"
#include "core/messagebox.h"
#include "core/signalmanager.h"

#include "helpers/support.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::RandomChatDlg */
/* TRANSLATOR LicqQtGui::SetRandomChatGroupDlg */

void RandomChatDlg::fillGroupsList(QListWidget* list, bool addNone, unsigned def)
{
  QListWidgetItem* item;

#define ADD_RCG(id, text) \
  item = new QListWidgetItem(text, list); \
  item->setData(Qt::UserRole, id); \
  if (def == id) \
    list->setCurrentItem(item);

  if (addNone)
  {
    ADD_RCG(CICQDaemon::RandomChatGroupNone,    tr("(none)"))
  }
  ADD_RCG(CICQDaemon::RandomChatGroupGeneral,   tr("General"));
  ADD_RCG(CICQDaemon::RandomChatGroupRomance,   tr("Romance"));
  ADD_RCG(CICQDaemon::RandomChatGroupGames,     tr("Games"));
  ADD_RCG(CICQDaemon::RandomChatGroupStudents,  tr("Students"));
  ADD_RCG(CICQDaemon::RandomChatGroup20Some,    tr("20 Something"));
  ADD_RCG(CICQDaemon::RandomChatGroup30Some,    tr("30 Something"));
  ADD_RCG(CICQDaemon::RandomChatGroup40Some,    tr("40 Something"));
  ADD_RCG(CICQDaemon::RandomChatGroup50Plus,    tr("50 Plus"));
  ADD_RCG(CICQDaemon::RandomChatGroupSeekF,     tr("Seeking Women"));
  ADD_RCG(CICQDaemon::RandomChatGroupSeekM,     tr("Seeking Men"));

#undef ADD_RCG
}

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
  fillGroupsList(myGroupsList, false, 0);

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
  unsigned chatGroup = myGroupsList->currentItem()->data(Qt::UserRole).toInt();
  myTag = gLicqDaemon->randomChatSearch(chatGroup);
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

  Licq::OwnerReadGuard o(LICQ_PPID);
  if (!o.isLocked())
  {
    close();
    return;
  }

  // Fill in the combo box
  RandomChatDlg::fillGroupsList(myGroupsList, true, o->randomChatGroup());

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
  unsigned chatGroup = myGroupsList->currentItem()->data(Qt::UserRole).toInt();
  myTag = gLicqDaemon->setRandomChatGroup(chatGroup);
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
