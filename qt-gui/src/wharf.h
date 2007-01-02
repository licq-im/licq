// -*- c-basic-offset: 2 -*-
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

// written by Graham Roff <graham@licq.org>
// KDE support by Dirk Mueller <dirk@licq.org>

#ifndef WHARF_H
#define WHARF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpopupmenu.h>

class CMainWindow;

class WharfIcon : public QWidget
{
  Q_OBJECT

public:
  WharfIcon(QPixmap *, QWidget *parent);
  virtual ~WharfIcon();
  void Set(QPixmap *);
protected:
  virtual void mousePressEvent (QMouseEvent *);
  virtual void paintEvent (QPaintEvent *);
  QPixmap *vis;
friend class IconManager;
friend class IconManager_Default;
friend class IconManager_Themed;
friend class IconManager_KDEStyle;
};


class IconManager : public QWidget
{
  Q_OBJECT

public:
  IconManager(CMainWindow *, QPopupMenu *, QWidget *parent = 0);
  virtual ~IconManager();
  // reimplement these two
  virtual void SetDockIconStatus() {};
  virtual void SetDockIconMsg(unsigned short, unsigned short) {};

protected:
  CMainWindow *mainwin;
  QPopupMenu *menu;
  void X11Init();
  virtual void mousePressEvent(QMouseEvent *);
  virtual void closeEvent (QCloseEvent *);
  virtual void paintEvent (QPaintEvent *);
  WharfIcon *wharfIcon;
  int m_nNewMsg, m_nSysMsg;
  int mouseX, mouseY;
friend class WharfIcon;
};


class IconManager_Default : public IconManager
{
  Q_OBJECT

public:
  IconManager_Default(CMainWindow *, QPopupMenu *, bool, QWidget *parent = 0);
  virtual ~IconManager_Default();
  virtual void SetDockIconStatus();
  virtual void SetDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg);

  bool FortyEight() {  return m_bFortyEight; }
protected:
  QPixmap *GetDockIconStatusIcon();
  QPixmap *pix;
  bool m_bFortyEight;
};


class IconManager_Themed : public IconManager
{
  Q_OBJECT

public:
  IconManager_Themed(CMainWindow *, QPopupMenu *, const char *, QWidget *parent = 0);
  virtual ~IconManager_Themed();
  virtual void SetDockIconStatus();
  virtual void SetDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg);

  QString Theme() { return m_szTheme; }
  void SetTheme(const char *);
protected:
  QString m_szTheme;
  QPixmap *pixNoMessages, *pixRegularMessages, *pixSystemMessages, *pixBothMessages,
          *pixOnline, *pixOffline, *pixAway, *pixNA, *pixOccupied, *pixDND,
          *pixInvisible, *pixFFC;
};

class IconManager_KDEStyle : public IconManager
{
  Q_OBJECT

public:
  IconManager_KDEStyle(CMainWindow*, QPopupMenu*, QWidget* parent=0);
  virtual ~IconManager_KDEStyle();

  virtual void SetDockIconStatus();
  virtual void SetDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg);

protected:
  virtual void mousePressEvent (QMouseEvent *);
  virtual void timerEvent(QTimerEvent*);
  virtual void paintEvent (QPaintEvent *);
  virtual bool x11Event(XEvent *);

  void updateTooltip();

  QPixmap m_statusIcon;
  QPixmap m_eventIcon;

  unsigned short m_ownerStatus;
  unsigned short m_NewMsgs;
  unsigned short m_SysMsgs;

  bool m_bStatusInvisible;
  bool m_timerToggle;
};

#endif
