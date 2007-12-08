/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2006 Licq developers
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

#ifndef LICQGUI_H
#define LICQGUI_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_KDE
#include <kapp.h>
#else
#include <qapplication.h>
#endif
#include <qstringlist.h>

class CICQDaemon;
class CMainWindow;
class CSignalManager;
class CQtLogWindow;

#ifdef USE_KDE
class CLicqGui : public KApplication
#else
class CLicqGui : public QApplication
#endif
{
public:
  CLicqGui(int, char **);
  ~CLicqGui(void);
  int Run(CICQDaemon *);
  virtual bool x11EventFilter(XEvent *);

  virtual void commitData(QSessionManager& sm);
  virtual void saveState(QSessionManager& sm);

  bool grabKey(QString key);

protected:
  char *m_szSkin, *m_szIcons, *m_szExtendedIcons;
  bool m_bStartHidden;
  bool m_bDisableDockIcon;
  CMainWindow *licqMainWindow;
  CSignalManager *licqSignalManager;
  CQtLogWindow *licqLogWindow;
  QStringList cmdLineParams;
  int grabKeysym;

  QStyle *SetStyle(const char *_szStyle);
};

#endif
