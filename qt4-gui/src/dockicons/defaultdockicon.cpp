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

#include "defaultdockicon.h"

#include "config.h"

#include <QBitmap>
#include <QPainter>

#include <licq/contactlist/user.h>

#include "config/general.h"
#include "dockiconwidget.h"

#include "xpm/dock/away.xpm"
#include "xpm/dock/back48.xpm"
#include "xpm/dock/back64.xpm"
#include "xpm/dock/digits.h"
#include "xpm/dock/dnd.xpm"
#include "xpm/dock/ffc.xpm"
#include "xpm/dock/invisible.xpm"
#include "xpm/dock/mask48.xpm"
#include "xpm/dock/mask64.xpm"
#include "xpm/dock/na.xpm"
#include "xpm/dock/occupied.xpm"
#include "xpm/dock/offline.xpm"
#include "xpm/dock/online.xpm"

using Licq::User;
using namespace LicqQtGui;

DefaultDockIcon::DefaultDockIcon(QMenu* menu)
  : DockIcon()
{
  myIcon = new DockIconWidget(menu);
  relayDockIconSignals();
  updateConfig();
}

void DefaultDockIcon::updateConfig()
{
  myFortyEight = Config::General::instance()->defaultIconFortyEight();

  QPixmap* pic = new QPixmap(myFortyEight ? back48_xpm : back64_xpm);
  QBitmap bmp = QBitmap(myFortyEight ? mask48_xpm : mask64_xpm);
  pic->setMask(bmp);
  myIcon->setFace(pic);
  delete pic;

  updateStatusIcon();
  updateIconMessages(myNewMsg, mySysMsg);
}

void DefaultDockIcon::updateStatusIcon()
{
  DockIcon::updateStatusIcon();

  if (!myFortyEight && myNewMsg == 0 && mySysMsg == 0)
    drawIcon64(myStatusIcon);

  QPixmap m;
  switch (User::singleStatus(myStatus))
  {
    case User::OfflineStatus:
      m = QPixmap(offline_xpm);
      break;
    case User::InvisibleStatus:
      m = QPixmap(invisible_xpm);
      break;
    case User::NotAvailableStatus:
      m = QPixmap(na_xpm);
      break;
    case User::AwayStatus:
      m = QPixmap(away_xpm);
      break;
    case User::DoNotDisturbStatus:
      m = QPixmap(dnd_xpm);
      break;
    case User::OccupiedStatus:
      m = QPixmap(occupied_xpm);
      break;
    case User::FreeForChatStatus:
      m = QPixmap(ffc_xpm);
      break;
    case User::OnlineStatus:
    default:
      m = QPixmap(online_xpm);
      break;
  }

  QPixmap* face = myIcon->face();
  QPainter painter(face);
  painter.drawPixmap(0, myFortyEight ? 27 : 44, m);
  painter.end();

  myIcon->setFace(face);
  delete face;
}

void DefaultDockIcon::updateIconMessages(int newMsg, int sysMsg)
{
  DockIcon::updateIconMessages(newMsg, sysMsg);

  int low, high;

#define SPLIT(num) \
  (num) = qMin(num, 99); \
  low = (num) % 10; \
  high = (num) / 10;

  QPixmap* face = myIcon->face();
  QPainter p(face);

  SPLIT(newMsg);
  p.drawPixmap(44, myFortyEight ? 8 : 26, digits[high]);
  p.drawPixmap(50, myFortyEight ? 8 : 26, digits[low]);

  SPLIT(sysMsg);
  p.drawPixmap(44, myFortyEight ? 20 : 38, digits[high]);
  p.drawPixmap(50, myFortyEight ? 20 : 38, digits[low]);

  p.end();

  myIcon->setFace(face);
  delete face;
}

void DefaultDockIcon::updateEventIcon()
{
  DockIcon::updateEventIcon();

  if (myFortyEight)
    return;

  drawIcon64((mySysMsg > 0 || myNewMsg > 0) ? myEventIcon : myStatusIcon);
}

void DefaultDockIcon::drawIcon64(QPixmap* icon)
{
  if (icon == NULL || icon->isNull())
    return;

  QPixmap* face = myIcon->face();
  QPainter p(face);

  // Clear the icon area
  p.fillRect(31, 6, 27, 16, Qt::black);

  // Calculate drawing coordinates
  int w = qMin(icon->width(), 27);
  int h = qMin(icon->height(), 16);
  int x = 45 - (w / 2);
  int y = 14 - (h / 2);

  p.drawPixmap(x, y, *icon, 0, 0, w, h);
  p.end();

  myIcon->setFace(face);
  delete face;
}
