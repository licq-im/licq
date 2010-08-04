// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2010 Licq developers
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

#include "dockicon.h"

#include "config.h"

#include <boost/foreach.hpp>

#include <licq/contactlist/owner.h>
#include <licq/contactlist/usermanager.h>

#include "config/general.h"
#include "config/iconmanager.h"

#include "dockiconwidget.h"

using Licq::User;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::DockIcon */

DockIcon::DockIcon()
  : QObject(),
    myIcon(NULL),
    myNewMsg(0),
    mySysMsg(0),
    myStatus(User::OfflineStatus)
{
  // Get icon set updates
  connect(IconManager::instance(), SIGNAL(statusIconsChanged()), SLOT(updateStatusIcon()));
  connect(IconManager::instance(), SIGNAL(generalIconsChanged()), SLOT(updateEventIcon()));

  // Get configuration updates
  connect(Config::General::instance(), SIGNAL(dockChanged()), SLOT(updateConfig()));

  unsigned short sysMsg = 0;

  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
    {
      Licq::OwnerReadGuard pOwner(owner);
      sysMsg += pOwner->NewMessages();
    }
  }

  unsigned short newMsg = Licq::User::getNumUserEvents() - sysMsg;

  updateIconMessages(newMsg, sysMsg);
  updateIconStatus();
}

DockIcon::~DockIcon()
{
  delete myIcon;
}

void DockIcon::updateIconStatus()
{
  // Default if there is no owner, just show status as offline
  myUserId = Licq::UserId();;
  myStatus = User::OfflineStatus;

  {
    Licq::OwnerListGuard ownerList;
    BOOST_FOREACH(const Licq::Owner* owner, **ownerList)
    {
      Licq::OwnerReadGuard pOwner(owner);

      // Any account is better than no account
      //   and try and get account with "best" status
      unsigned status = pOwner->status();
      if (!myUserId.isValid() || (status != User::OfflineStatus && status < myStatus))
      {
        myUserId = pOwner->id();
        myStatus = status;
      }
    }
  }

  updateToolTip();
  updateStatusIcon();
}

void DockIcon::updateStatusIcon()
{
  myStatusIcon = const_cast<QPixmap*>
    (&IconManager::instance()->iconForStatus(myStatus, myUserId));
}

void DockIcon::updateIconMessages(int newMsg, int sysMsg)
{
  myNewMsg = newMsg;
  mySysMsg = sysMsg;

  updateToolTip();
  updateEventIcon();
}

void DockIcon::updateEventIcon()
{
  if (mySysMsg > 0)
    myEventIcon = const_cast<QPixmap*>
      (&IconManager::instance()->getIcon(IconManager::ReqAuthorizeMessageIcon));
  else if (myNewMsg > 0)
    myEventIcon = const_cast<QPixmap*>
      (&IconManager::instance()->getIcon(IconManager::StandardMessageIcon));
  else
    myEventIcon = NULL;
}

void DockIcon::updateToolTip()
{
  QString s = QString("<nobr>%1</nobr>")
      .arg(User::statusToString(myStatus).c_str());

  if (mySysMsg)
    s += "<br><b>" + tr("%1 system messages").arg(mySysMsg) + "</b>";

  if (myNewMsg > 1)
    s += "<br>" + tr("%1 msgs").arg(myNewMsg);
  else if (myNewMsg)
    s += "<br>" + tr("1 msg");

  s += tr("<br>Left click - Show main window"
          "<br>Middle click - Show next message"
          "<br>Right click - System menu");

  if (myIcon != NULL)
    myIcon->setToolTip(s);
  else
    emit newToolTip(s);
}

void DockIcon::relayDockIconSignals()
{
  if (myIcon == NULL)
    return;

  connect(myIcon, SIGNAL(clicked()), SIGNAL(clicked()));
  connect(myIcon, SIGNAL(middleClicked()), SIGNAL(middleClicked()));
}
