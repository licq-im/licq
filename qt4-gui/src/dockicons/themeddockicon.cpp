// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2007-2009 Licq developers
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

#include <licq_user.h>

#include "config/general.h"
#include "core/messagebox.h"
#include "core/gui-defines.h"

#include "dockiconwidget.h"

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
  char temp[MAX_FILENAME_LEN];
  QString baseDockDir;

  if (myTheme[0] == '/')
  {
    baseDockDir = myTheme;
    if (baseDockDir[baseDockDir.length() - 1] != '/')
      baseDockDir += "/";
  }
  else
    baseDockDir = QString::fromLocal8Bit(SHARE_DIR) + QTGUI_DIR +
      DOCK_DIR + myTheme + "/";

  QByteArray filename = QFile::encodeName(baseDockDir);
  filename.append(myTheme);
  filename.append(".dock");

  CIniFile dockFile(INI_FxWARN);

  if (!dockFile.LoadFile(filename))
  {
    WarnUser(NULL, tr("Unable to load dock theme file:\n(%1)\n%2")
        .arg(filename.data())
        .arg(strerror(dockFile.Error())));
    myIcon->hide();
    return;
  }

  myIcon->show();

  // Message icons
  if (dockFile.SetSection("background"))
  {
#define READDOCK(parm, var) \
    dockFile.ReadStr((parm), temp, "none"); \
    if (strcmp(temp, "none") == 0) \
      WarnUser(NULL, tr("Dock theme unspecified image: %1").arg((parm))); \
    else \
    { \
      (var) = new QPixmap(baseDockDir + temp); \
      if ((var)->isNull()) \
      { \
        WarnUser(NULL, tr("Unable to load dock theme image: %1").arg((parm))); \
        delete (var); \
        (var) = NULL; \
      } \
      else \
      { \
        dockFile.ReadStr(parm "Mask", temp, "none"); \
        if (strcmp(temp, "none") != 0) \
          (var)->setMask(QBitmap(baseDockDir + temp)); \
      } \
    }

    READDOCK("NoMessages", pixNoMessages);
    READDOCK("RegularMessages", pixRegularMessages);
    READDOCK("SystemMessages", pixSystemMessages);
    READDOCK("BothMessages", pixBothMessages);
#undef READDOCK
  }

  // Status icons
  if (dockFile.SetSection("status"))
  {
    QBitmap mask;
    dockFile.ReadStr("Mask", temp, "none");
    if (strcmp(temp, "none") != 0)
      mask = QBitmap(baseDockDir + temp);
#define READDOCK(parm, var) \
    dockFile.ReadStr((parm), temp, "none"); \
    if (strcmp(temp, "none") != 0) \
    { \
      (var) = new QPixmap(baseDockDir + temp); \
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

  dockFile.CloseFile();

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

  if (myInvisible)
    p = pixInvisible;
  else
    switch (myStatus)
    {
      case ICQ_STATUS_ONLINE:
        p = pixOnline;
        break;
      case ICQ_STATUS_AWAY:
        p = pixAway;
        break;
      case ICQ_STATUS_NA:
        p = pixNA;
        break;
      case ICQ_STATUS_OCCUPIED:
        p = pixOccupied;
        break;
      case ICQ_STATUS_DND:
        p = pixDND;
        break;
      case ICQ_STATUS_FREEFORCHAT:
        p = pixFFC;
        break;
      case ICQ_STATUS_OFFLINE:
        p = pixOffline;
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
