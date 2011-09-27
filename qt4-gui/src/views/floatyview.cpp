/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2011 Licq developers
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

#include "floatyview.h"

#include <QHeaderView>
#include <QMouseEvent>

#include <licq/contactlist/user.h>

#include "config/contactlist.h"
#include "contactlist/contactlist.h"
#include "contactlist/singlecontactproxy.h"
#include "core/usermenu.h"
#include "helpers/support.h"

#include "contactdelegate.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::FloatyView */

UserFloatyList FloatyView::floaties;

FloatyView::FloatyView(ContactListModel* contactList, const Licq::UserId& userId,  QWidget* parent)
  : UserViewBase(contactList, true, parent),
  myUserId(userId)
{
  setWindowFlags(Qt::FramelessWindowHint);
  Support::ghostWindow(winId());
  setAttribute(Qt::WA_DeleteOnClose, true);

  QString name;
  name.sprintf("Floaty%d", floaties.size() + 1);
  Support::setWidgetProps(this, name);

  {
    Licq::UserReadGuard u(myUserId);

    setWindowTitle(tr("%1 Floaty (%2)")
        .arg(QString::fromUtf8(u->getAlias().c_str()))
        .arg(u->accountId().c_str()));
  }

  setFrameStyle(QFrame::Raised | QFrame::Box);
  setSelectionMode(NoSelection);
  header()->hide();

  // Use a proxy model to get a single user from the contact list
  myListProxy = new SingleContactProxy(myContactList, myUserId, this);
  setModel(myListProxy);

  connect(Config::ContactList::instance(), SIGNAL(listLookChanged()), SLOT(configUpdated()));
  configUpdated();

  floaties.append(this);
}

FloatyView::~FloatyView()
{
  int pos = floaties.indexOf(this);
  if (pos != -1)
    floaties.remove(pos);
}

FloatyView* FloatyView::findFloaty(const Licq::UserId& userId)
{
  for (int i = 0; i < floaties.size(); i++)
  {
    FloatyView* p = floaties.at(i);
    if (p->myUserId == userId)
      return p;
  }

  return NULL;
}

void FloatyView::mouseMoveEvent(QMouseEvent* event)
{
  UserViewBase::mouseMoveEvent(event);

  // Move the floaty the same distance as the mouse has moved since button was pressed
  if (event->buttons() & Qt::LeftButton)
    move(event->globalPos() - myMousePressPos);
}

void FloatyView::configUpdated()
{
  // Set column widths
  for (int i = 0; i < Config::ContactList::instance()->columnCount(); i++)
    setColumnWidth(i, Config::ContactList::instance()->columnWidth(i));
}
