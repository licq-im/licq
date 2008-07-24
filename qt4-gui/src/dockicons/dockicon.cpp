// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007 Licq developers
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

#include <licq_user.h>

#include "config/general.h"
#include "config/iconmanager.h"

#include "helpers/licqstrings.h"

#include "dockiconwidget.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::DockIcon */

DockIcon::DockIcon()
  : QObject(),
    myIcon(NULL),
    myNewMsg(0),
    mySysMsg(0),
    myStatus(0),
    myInvisible(false)
{
  // Get icon set updates
  connect(IconManager::instance(), SIGNAL(statusIconsChanged()), SLOT(updateStatusIcon()));
  connect(IconManager::instance(), SIGNAL(generalIconsChanged()), SLOT(updateEventIcon()));

  // Get configuration updates
  connect(Config::General::instance(), SIGNAL(dockChanged()), SLOT(updateConfig()));

  unsigned short sysMsg = 0;

  FOR_EACH_OWNER_START(LOCK_R)
  {
    sysMsg += pOwner->NewMessages();
  }
  FOR_EACH_OWNER_END

  unsigned short newMsg = ICQUser::getNumUserEvents() - sysMsg;

  updateIconMessages(newMsg, sysMsg);
  updateIconStatus();
}

DockIcon::~DockIcon()
{
  delete myIcon;
}

void DockIcon::updateIconStatus()
{
  // First check for ICQ/AIM owner presence
  const ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);

  // Take any existent one otherwise
  if (o == NULL && gUserManager.NumOwners() != 0)
  {
    const OwnerList* ol = gUserManager.LockOwnerList(LOCK_R);
    o = ol->front();
    o->Lock(LOCK_R);
    gUserManager.UnlockOwnerList();
  }

  if (o != NULL)
  {
    myId = o->IdString();
    myPpid = o->PPID();
    myFullStatus = o->StatusFull();
    myStatus = o->Status();
    myInvisible = o->StatusInvisible();
    gUserManager.DropOwner(o);
  }
  else
  {
    // We got no owner, this could be normal if this is first run,
    // just show status as offline until user has created the first account
    myId = "0";
    myPpid = LICQ_PPID;
    myFullStatus = ICQ_STATUS_OFFLINE;
    myStatus = ICQ_STATUS_OFFLINE;
    myInvisible = false;
  }

  updateToolTip();
  updateStatusIcon();
}

void DockIcon::updateStatusIcon()
{
  myStatusIcon = const_cast<QPixmap*>
    (&IconManager::instance()->iconForStatus(myFullStatus, myId, myPpid));
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
      .arg(LicqStrings::getStatus(myStatus, myInvisible));

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
