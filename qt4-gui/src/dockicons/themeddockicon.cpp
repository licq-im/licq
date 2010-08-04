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

#include "themeddockicon.h"

#include "config.h"

#include <QBitmap>
#include <QFile>
#include <QPainter>

#include <licq/contactlist/user.h>
#include <licq/daemon.h>
#include <licq/inifile.h>

#include "config/general.h"
#include "core/messagebox.h"
#include "core/gui-defines.h"

#include "dockiconwidget.h"

using Licq::User;
using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::ThemedDockIcon */

ThemedDockIcon::ThemedDockIcon(QMenu* menu)
  : DockIcon()
{
  myIcon = new DockIconWidget(menu);
  relayDockIconSignals();
  cleanup(true);
  updateConfig();
}

void ThemedDockIcon::updateConfig()
{
  myTheme = Config::General::instance()->themedIconTheme();

  cleanup();

  // Open the config file and read it
  std::string temp;
  QString baseDockDir;

  if (myTheme[0] == '/')
  {
    baseDockDir = myTheme;
    if (baseDockDir[baseDockDir.length() - 1] != '/')
      baseDockDir += "/";
  }
  else
    baseDockDir = QString::fromLocal8Bit(Licq::gDaemon.shareDir().c_str()) + QTGUI_DIR +
      DOCK_DIR + myTheme + "/";

  QByteArray filename = QFile::encodeName(baseDockDir);
  filename.append(myTheme);
  filename.append(".dock");

  Licq::IniFile dockFile(filename.data());

  if (!dockFile.loadFile())
  {
    WarnUser(NULL, tr("Unable to load dock theme file:\n(%1)")
        .arg(filename.data()));
    myIcon->hide();
    return;
  }

  myIcon->show();

  // Message icons
  if (dockFile.setSection("background", false))
  {
#define READDOCK(parm, var) \
    dockFile.get((parm), temp, "none"); \
    if (temp == "none") \
      WarnUser(NULL, tr("Dock theme unspecified image: %1").arg((parm))); \
    else \
    { \
      (var) = new QPixmap(baseDockDir + QString::fromLocal8Bit(temp.c_str())); \
      if ((var)->isNull()) \
      { \
        WarnUser(NULL, tr("Unable to load dock theme image: %1").arg((parm))); \
        delete (var); \
        (var) = NULL; \
      } \
      else \
      { \
        dockFile.get(parm "Mask", temp, "none"); \
        if (temp != "none") \
          (var)->setMask(QBitmap(baseDockDir + QString::fromLocal8Bit(temp.c_str()))); \
      } \
    }

    READDOCK("NoMessages", pixNoMessages);
    READDOCK("RegularMessages", pixRegularMessages);
    READDOCK("SystemMessages", pixSystemMessages);
    READDOCK("BothMessages", pixBothMessages);
#undef READDOCK
  }

  // Status icons
  if (dockFile.setSection("status", false))
  {
    QBitmap mask;
    dockFile.get("Mask", temp, "none");
    if (temp != "none")
      mask = QBitmap(baseDockDir + QString::fromLocal8Bit(temp.c_str()));
#define READDOCK(parm, var) \
    dockFile.get((parm), temp, "none"); \
    if (temp != "none") \
    { \
      (var) = new QPixmap(baseDockDir + QString::fromLocal8Bit(temp.c_str())); \
      if ((var)->isNull()) \
      { \
        WarnUser(NULL, tr("Unable to load dock theme image: %1").arg((parm))); \
        delete (var); \
        (var) = NULL; \
      } \
      else \
        if (!mask.isNull()) \
          (var)->setMask(mask); \
    }

    READDOCK("Online", pixOnline);
    READDOCK("Offline", pixOffline);
    READDOCK("Away", pixAway);
    READDOCK("NA", pixNA);
    READDOCK("Occupied", pixOccupied);
    READDOCK("DND", pixDND);
    READDOCK("Invisible", pixInvisible);
    READDOCK("FFC", pixFFC);
#undef READDOCK
  }

  updateStatusIcon();
  updateIconMessages(myNewMsg, mySysMsg);
}

ThemedDockIcon::~ThemedDockIcon()
{
  cleanup();
}

void ThemedDockIcon::updateIconStatus()
{
  DockIcon::updateIconStatus();

  QPixmap* p = NULL;

  switch (User::singleStatus(myStatus))
  {
    case User::InvisibleStatus:
      p = pixInvisible;
      break;
    case User::AwayStatus:
      p = pixAway;
      break;
    case User::NotAvailableStatus:
      p = pixNA;
      break;
    case User::OccupiedStatus:
      p = pixOccupied;
      break;
    case User::DoNotDisturbStatus:
      p = pixDND;
      break;
    case User::FreeForChatStatus:
      p = pixFFC;
      break;
    case User::OfflineStatus:
      p = pixOffline;
      break;
    case User::OnlineStatus:
    default:
      p = pixOnline;
      break;
  }

  QPixmap* face = myIcon->face();
  if (face != NULL && p != NULL)
  {
    QPainter pt(face);
    pt.drawPixmap(0, 0, *p);
  }
  myIcon->setFace(face, false);
  delete face;
}

void ThemedDockIcon::updateIconMessages(int newMsg, int sysMsg)
{
  DockIcon::updateIconMessages(newMsg, sysMsg);

  QPixmap* p = NULL;

  if (newMsg > 0 && sysMsg > 0)
    p = pixBothMessages;
  else if (newMsg > 0)
    p = pixRegularMessages;
  else if (sysMsg > 0)
    p = pixSystemMessages;
  else
    p = pixNoMessages;

  myIcon->setFace(p);
  updateIconStatus();
}

void ThemedDockIcon::cleanup(bool initial)
{
  QList<QPixmap**> pics;
  pics << &pixNoMessages << &pixRegularMessages << &pixSystemMessages
    << &pixBothMessages << &pixOnline << &pixOffline << &pixAway << &pixNA
    << &pixOccupied << &pixDND << &pixInvisible << &pixFFC;
  QPixmap** it;

  if (!initial) 
  {
    foreach(it, pics)
      delete *it;
  }

  foreach(it, pics)
    *it = NULL;
}
