/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2013 Licq developers <licq-dev@googlegroups.com>
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
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include <licq/contactlist/usermanager.h>
#include <licq/event.h>
#include <licq/icq/icq.h>
#include <licq/icq/user.h>
#include <licq/plugin/pluginmanager.h>
#include <licq/protocolmanager.h>

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
    ADD_RCG(Licq::IcqRandomChatGroupNone,       tr("(none)"))
  }
  ADD_RCG(Licq::IcqRandomChatGroupGeneral,      tr("General"));
  ADD_RCG(Licq::IcqRandomChatGroupRomance,      tr("Romance"));
  ADD_RCG(Licq::IcqRandomChatGroupGames,        tr("Games"));
  ADD_RCG(Licq::IcqRandomChatGroupStudents,     tr("Students"));
  ADD_RCG(Licq::IcqRandomChatGroup20Some,       tr("20 Something"));
  ADD_RCG(Licq::IcqRandomChatGroup30Some,       tr("30 Something"));
  ADD_RCG(Licq::IcqRandomChatGroup40Some,       tr("40 Something"));
  ADD_RCG(Licq::IcqRandomChatGroup50Plus,       tr("50 Plus"));
  ADD_RCG(Licq::IcqRandomChatGroupSeekF,        tr("Seeking Women"));
  ADD_RCG(Licq::IcqRandomChatGroupSeekM,        tr("Seeking Men"));

#undef ADD_RCG
}

RandomChatDlg::RandomChatDlg(const Licq::UserId& ownerId, QWidget* parent)
  : QDialog(parent),
    myOwnerId(ownerId),
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
    Licq::gProtocolManager.cancelEvent(Licq::UserId(myOwnerId, "0000"), myTag);
}

void RandomChatDlg::okPressed()
{
  Licq::IcqProtocol::Ptr icq = plugin_internal_cast<Licq::IcqProtocol>(
      Licq::gPluginManager.getProtocolInstance(myOwnerId));
  if (!icq)
    return;

  myOkButton->setEnabled(false);
  connect(gGuiSignalManager, SIGNAL(doneUserFcn(const Licq::Event*)),
      SLOT(userEventDone(const Licq::Event*)));
  unsigned chatGroup = myGroupsList->currentItem()->data(Qt::UserRole).toInt();
  myTag = icq->randomChatSearch(myOwnerId, chatGroup);
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
    case Licq::Event::ResultUnsupported:
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
