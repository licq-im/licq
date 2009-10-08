// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xpm/iconOnline.xpm"
#include "xpm/iconOffline.xpm"
#include "xpm/iconAway.xpm"
#include "xpm/iconFFC.xpm"
#include "xpm/iconNA.xpm"
#include "xpm/iconDND.xpm"
#include "xpm/iconInvisible.xpm"
#include "xpm/iconOccupied.xpm"
#include "xpm/iconBack-64.xpm"
#include "xpm/iconMask-64.xpm"
#include "xpm/iconBack-48.xpm"
#include "xpm/iconMask-48.xpm"
#include "xpm/iconDigits.h"

#include <stdio.h>
#include <qpainter.h>
#include <qfile.h>
#ifdef USE_KDE
#include <kwin.h>
#include <kpopupmenu.h>
#include <kdeversion.h>
#endif

#include "wharf.h"
#include "mainwin.h"
#include "licq_user.h"
#include "ewidgets.h"
#include "gui-strings.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#undef Status
#undef Bool
#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut

//#define DEBUG_WHARF
#define SYSTEM_TRAY_REQUEST_DOCK    0

/*
  Constructs a WharfIcon widget.
*/
IconManager::IconManager(CMainWindow *_mainwin, QPopupMenu *_menu, QWidget* /* parent */)
  : QWidget(NULL, "LicqWharf", WType_TopLevel)
{
  setCaption("LicqWharf");
  m_nNewMsg = m_nSysMsg = 0;
  wharfIcon = NULL;
  mainwin = _mainwin;
  menu = _menu;

  setBackgroundMode(X11ParentRelative);
}


void IconManager::X11Init()
{
  // set the hints
#ifndef USE_KDE
  Display *dsp = x11Display();  // get the display
  WId win = winId();     // get the window
  XWMHints *hints;  // hints
  XClassHint classhint;  // class hints
  XSizeHints sizeHints;
  classhint.res_name = const_cast<char*>("licq");  // res_name
  classhint.res_class = const_cast<char*>("Wharf");  // res_class
  XSetClassHint(dsp, win, &classhint); // set the class hints
  sizeHints.flags= USPosition;
  sizeHints.x = 0;
  sizeHints.y = 0;
  XSetWMNormalHints(dsp, win, &sizeHints);
  hints = XGetWMHints(dsp, win);  // init hints
  hints->initial_state = WithdrawnState;
  hints->icon_x = 0;
  hints->icon_y = 0;
  hints->icon_window = wharfIcon->winId();
  hints->window_group = win;  // set the window hint
  hints->flags = WindowGroupHint | IconWindowHint | IconPositionHint | StateHint; // set the window group hint
  XSetWMHints(dsp, win, hints);  // set the window hints for WM to use.
  XFree( hints );

  XMapWindow(dsp, wharfIcon->winId());
  XMapWindow(dsp, win);
  XSync(dsp, False);

#endif

  resize (wharfIcon->width(), wharfIcon->height());
  setMask(*wharfIcon->vis->mask());
  move(-100,-100);
  show();
#ifndef USE_KDE
  XWithdrawWindow(dsp, win, XScreenNumberOfScreen(DefaultScreenOfDisplay(dsp)));
#endif
}



IconManager::~IconManager()
{
  // Ensure that our main window is visible when the dock gets disabled:
  if (mainwin != NULL)
    mainwin->show();

  delete wharfIcon;
  // clean up behind us the prevent segfaults
  gMainWindow->licqIcon = NULL;
}

void IconManager::closeEvent( QCloseEvent* e)
{
  e->ignore();
}

void IconManager::mousePressEvent( QMouseEvent *e )
{
#ifdef DEBUG_WHARF
  printf("icon press\n");
#endif
  switch(e->button())
  {
    case LeftButton:
      if (mainwin->isVisible())
        mainwin->hide();
      else
      {
        mainwin->show();
        mainwin->raise();
      }
      break;
    case MidButton:
      mainwin->callMsgFunction();
      break;
    case RightButton:
      menu->popup(e->globalPos());
      break;
    default:
      break;
  }
  //wharfIcon->mousePressEvent(e);
}

void IconManager::paintEvent( QPaintEvent * )
{
#ifdef DEBUG_WHARF
  printf("icon paint\n");
#endif
  QPainter painter(this);
  painter.drawPixmap(0, 0, *wharfIcon->vis);
  painter.end();
}



//=====IconManager_Default===================================================

IconManager_Default::IconManager_Default(CMainWindow *_mainwin, QPopupMenu *_menu, bool _bFortyEight, QWidget *parent )
  : IconManager(_mainwin, _menu, parent)
{
  m_bFortyEight = _bFortyEight;
  if (m_bFortyEight)
  {
    pix = new QPixmap((const char **)iconBack_48_xpm);
    QBitmap b;
    b = QPixmap((const char **)iconMask_48_xpm);
    pix->setMask(b);
    wharfIcon = new WharfIcon(pix, this);
  }
  else
  {
    pix = new QPixmap((const char **)iconBack_64_xpm);
    QBitmap b;
    b = QPixmap((const char **)iconMask_64_xpm);
    pix->setMask(b);
    wharfIcon = new WharfIcon(pix, this);
  }
  X11Init();
}

IconManager_Default::~IconManager_Default()
{
  delete pix;
}


//-----CMainWindow::setDockIconStatus-------------------------------------------
void IconManager_Default::SetDockIconStatus()
{
  QPixmap m;
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == 0) return;
  switch (o->Status())
  {
    case ICQ_STATUS_ONLINE: m = QPixmap((const char **)iconOnline_xpm); break;
    case ICQ_STATUS_AWAY: m = QPixmap((const char **)iconAway_xpm); break;
    case ICQ_STATUS_NA: m = QPixmap((const char **)iconNA_xpm); break;
    case ICQ_STATUS_OCCUPIED: m = QPixmap((const char **)iconOccupied_xpm); break;
    case ICQ_STATUS_DND: m = QPixmap((const char **)iconDND_xpm); break;
    case ICQ_STATUS_FREEFORCHAT: m = QPixmap((const char **)iconFFC_xpm); break;
    case ICQ_STATUS_OFFLINE: m = QPixmap((const char **)iconOffline_xpm); break;
  }
  if (o->StatusInvisible()) m = QPixmap((const char **)iconInvisible_xpm);
  gUserManager.DropOwner(o);
  QPainter painter(wharfIcon->vis);
  if (!m_bFortyEight)
  {
    painter.drawPixmap(0, 44, m);
    // Draw the little icon if no new messages
    if (m_nNewMsg == 0 && m_nSysMsg == 0)
    {
      QPixmap *p = GetDockIconStatusIcon();
      painter.fillRect(31, 6, 27, 16, QColor(0,0,0));
      if (p != NULL)
      {
        int w = p->width() > 27 ? 27 : p->width();
        int h = p->height() > 16 ? 16 : p->height();
        int x = 45 - (w / 2);
        int y = 14 - (h / 2);
        painter.drawPixmap(x, y, *p, 0, 0, w, h);
      }
    }
  }
  else
    painter.drawPixmap(0, 27, m);
  painter.end();

  wharfIcon->repaint(false);
  repaint(false);
}


//-----IconManager_Default::GetDockIconStatusIcon----------------------------
QPixmap *IconManager_Default::GetDockIconStatusIcon()
{
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == 0) return NULL;

  unsigned long s = o->Status();
  gUserManager.DropOwner(o);
  switch (s)
  {
    case ICQ_STATUS_ONLINE: return &mainwin->pmOnline;
    case ICQ_STATUS_AWAY: return &mainwin->pmAway;
    case ICQ_STATUS_NA: return &mainwin->pmNa;
    case ICQ_STATUS_OCCUPIED: return &mainwin->pmOccupied;
    case ICQ_STATUS_DND: return &mainwin->pmDnd;
    case ICQ_STATUS_FREEFORCHAT: return &mainwin->pmChat;
    case ICQ_STATUS_OFFLINE: return &mainwin->pmOffline;
  }
  return NULL;
}

//-----IconManager_Default::setDockIconMsg-----------------------------------
void IconManager_Default::SetDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg)
{
  QPainter p(wharfIcon->vis);
  unsigned short d1, d10;
  m_nNewMsg = nNewMsg;
  m_nSysMsg = nSysMsg;

  // set the new msg digits
  if (nNewMsg > 99)
  {
    d1 = 9;
    d10 = 9;
  }
  else
  {
    d1 = nNewMsg % 10;
    d10 = (nNewMsg - d1) / 10;
  }
  if (!m_bFortyEight)
  {
    p.drawPixmap(44, 26, iconDigits[d10]);
    p.drawPixmap(50, 26, iconDigits[d1]);
  }
  else
  {
    p.drawPixmap(44, 9, iconDigits[d10]);
    p.drawPixmap(50, 9, iconDigits[d1]);
  }

  // set the system msg digits
  if (nSysMsg > 99)
  {
    d1 = 9;
    d10 = 9;
  }
  else
  {
    d1 = nSysMsg % 10;
    d10 = (nSysMsg - d1) / 10;
  }
  if (!m_bFortyEight)
  {
    p.drawPixmap(44, 38, iconDigits[d10]);
    p.drawPixmap(50, 38, iconDigits[d1]);
  }
  else
  {
    p.drawPixmap(44, 21, iconDigits[d10]);
    p.drawPixmap(50, 21, iconDigits[d1]);
  }

  if (!m_bFortyEight)
  {
    // Draw the little icon now
    QPixmap *m = NULL;
    if (nSysMsg > 0)
      m = &mainwin->pmReqAuthorize;
    else if (nNewMsg > 0)
      m = &mainwin->pmMessage;
    else
      m = GetDockIconStatusIcon();
    p.fillRect(31, 6, 27, 16, QColor(0,0,0));
    if (m != NULL)
    {
      int w = m->width() > 27 ? 27 : m->width();
      int h = m->height() > 16 ? 16 : m->height();
      int x = 45 - (w / 2);
      int y = 14 - (h / 2);
      p.drawPixmap(x, y, *m, 0, 0, w, h);
    }
  }
  p.end();

  wharfIcon->repaint(false);
  repaint(false);
}

//=====IconManager_Themed===================================================

IconManager_Themed::IconManager_Themed(CMainWindow *_mainwin, QPopupMenu *_menu, const char *theme, QWidget *parent)
  : IconManager(_mainwin, _menu, parent)
{
  pixNoMessages = pixBothMessages = pixRegularMessages = pixSystemMessages = NULL;
  pixOnline = pixOffline = pixAway = pixNA = pixOccupied = pixDND = pixInvisible = pixFFC = NULL;

  SetTheme(theme);

  wharfIcon = new WharfIcon(pixNoMessages, this);
  X11Init();
}


void IconManager_Themed::SetTheme(const char *theme)
{
  m_szTheme = theme;

  // Delete old pixmaps
  delete pixNoMessages;
  delete pixBothMessages;
  delete pixRegularMessages;
  delete pixSystemMessages;
  delete pixOnline;
  delete pixOffline;
  delete pixAway;
  delete pixNA;
  delete pixOccupied;
  delete pixDND;
  delete pixFFC;
  delete pixInvisible;

  // Open the config file and read it
  char temp[MAX_FILENAME_LEN];
  QString baseDockDir;
  if (theme[0] == '/')
  {
    baseDockDir = theme;
    if (baseDockDir[baseDockDir.length() - 1] != '/') baseDockDir += QString("/");
  }
  else
    baseDockDir.sprintf("%s%sdock.%s/", SHARE_DIR, QTGUI_DIR, theme);
  char filename[MAX_FILENAME_LEN];
  snprintf(filename, MAX_FILENAME_LEN, "%s%s.dock", QFile::encodeName(baseDockDir).data(), theme);
  filename[MAX_FILENAME_LEN - 1] = '\0';
  CIniFile dockFile(INI_FxWARN);
  if (!dockFile.LoadFile(filename))
  {
    WarnUser(NULL, tr("Unable to load dock theme file\n(%1)\n:%2")
                      .arg(filename).arg(strerror(dockFile.Error())));
    pixBothMessages = pixRegularMessages = pixSystemMessages = NULL;
    pixOnline = pixOffline = pixAway = pixNA = pixOccupied = pixDND = pixInvisible = pixFFC = NULL;
    pixNoMessages = new QPixmap(48, 48);
    return;
  }
  dockFile.SetSection("background");
  // No messages
  dockFile.ReadStr("NoMessages", temp);
  pixNoMessages = new QPixmap(baseDockDir + QString(temp));
  if (pixNoMessages->isNull())
    WarnUser(NULL, tr("Unable to load dock theme image\n%1").arg(baseDockDir));
  dockFile.ReadStr("NoMessagesMask", temp, "none");
  if (strcmp(temp, "none") != 0)
    pixNoMessages->setMask(QBitmap(baseDockDir + QString(temp)));
  // Regular messages
  dockFile.ReadStr("RegularMessages", temp);
  pixRegularMessages = new QPixmap(baseDockDir + QString(temp));
  if (pixRegularMessages->isNull())
    WarnUser(NULL, tr("Unable to load dock theme image\n%1").arg(baseDockDir));
  dockFile.ReadStr("RegularMessagesMask", temp, "none");
  if (strcmp(temp, "none") != 0)
    pixRegularMessages->setMask(QBitmap(baseDockDir + QString(temp)));
  // System messages
  dockFile.ReadStr("SystemMessages", temp);
  pixSystemMessages = new QPixmap(baseDockDir + QString(temp));
  if (pixSystemMessages->isNull())
    WarnUser(NULL, tr("Unable to load dock theme image\n%1").arg(baseDockDir));
  dockFile.ReadStr("SystemMessagesMask", temp, "none");
  if (strcmp(temp, "none") != 0)
    pixSystemMessages->setMask(QBitmap(baseDockDir + QString(temp)));
  // Both messages
  dockFile.ReadStr("BothMessages", temp);
  pixBothMessages = new QPixmap(baseDockDir + QString(temp));
  if (pixBothMessages->isNull())
    WarnUser(NULL, tr("Unable to load dock theme image\n%1").arg(baseDockDir));
  dockFile.ReadStr("BothMessagesMask", temp, "none");
  if (strcmp(temp, "none") != 0)
    pixBothMessages->setMask(QBitmap(baseDockDir + QString(temp)));

  // Status icons
  pixOnline = pixOffline = pixAway = pixNA = pixOccupied = pixDND = pixInvisible = pixFFC = NULL;
  dockFile.SetFlags(0);
  if (dockFile.SetSection("status"))
  {
    dockFile.SetFlags(INI_FxWARN);
    // Online
    dockFile.ReadStr("Online", temp, "none");
    if (strcmp(temp, "none") != 0)
      pixOnline = new QPixmap(baseDockDir + QString(temp));
    // Offline
    dockFile.ReadStr("Offline", temp, "none");
    if (strcmp(temp, "none") != 0)
      pixOffline = new QPixmap(baseDockDir + QString(temp));
    // Away
    dockFile.ReadStr("Away", temp, "none");
    if (strcmp(temp, "none") != 0)
      pixAway = new QPixmap(baseDockDir + QString(temp));
    // NA
    dockFile.ReadStr("NA", temp, "none");
    if (strcmp(temp, "none") != 0)
      pixNA = new QPixmap(baseDockDir + QString(temp));
    // Occupied
    dockFile.ReadStr("Occupied", temp, "none");
    if (strcmp(temp, "none") != 0)
      pixOccupied = new QPixmap(baseDockDir + QString(temp));
    // DND
    dockFile.ReadStr("DND", temp, "none");
    if (strcmp(temp, "none") != 0)
      pixDND = new QPixmap(baseDockDir + QString(temp));
    // Invisible
    dockFile.ReadStr("Invisible", temp, "none");
    if (strcmp(temp, "none") != 0)
      pixInvisible = new QPixmap(baseDockDir + QString(temp));
    // FFC
    dockFile.ReadStr("FFC", temp, "none");
    if (strcmp(temp, "none") != 0)
      pixFFC = new QPixmap(baseDockDir + QString(temp));

    dockFile.ReadStr("Mask", temp, "none");
    if (strcmp(temp, "none") != 0)
    {
      QBitmap mask(baseDockDir + QString(temp));
      if (pixOnline != NULL) pixOnline->setMask(mask);
      if (pixOffline != NULL) pixOffline->setMask(mask);
      if (pixAway != NULL) pixAway->setMask(mask);
      if (pixNA != NULL) pixNA->setMask(mask);
      if (pixOccupied != NULL) pixOccupied->setMask(mask);
      if (pixDND != NULL) pixDND->setMask(mask);
      if (pixInvisible != NULL) pixInvisible->setMask(mask);
      if (pixFFC != NULL) pixFFC->setMask(mask);
    }
  }
  dockFile.CloseFile();

  if (wharfIcon != NULL)
  {
    QPixmap *p = NULL;
    if (m_nNewMsg > 0 && m_nSysMsg > 0)
      p = pixBothMessages;
    else if (m_nNewMsg > 0)
      p = pixRegularMessages;
    else if (m_nSysMsg > 0)
      p = pixSystemMessages;
    else
      p = pixNoMessages;

    wharfIcon->Set(p);
    SetDockIconStatus();
    setMask(*wharfIcon->vis->mask());
    wharfIcon->repaint(false);
    repaint(false);
  }
}


IconManager_Themed::~IconManager_Themed()
{
  delete pixNoMessages;
  delete pixBothMessages;
  delete pixRegularMessages;
  delete pixSystemMessages;
  delete pixOnline;
  delete pixOffline;
  delete pixAway;
  delete pixNA;
  delete pixOccupied;
  delete pixDND;
  delete pixFFC;
  delete pixInvisible;
}



//-----CMainWindow::setDockIconStatus-------------------------------------------
void IconManager_Themed::SetDockIconStatus()
{
  QPixmap *p = NULL;
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == 0) return;
  switch (o->Status())
  {
    case ICQ_STATUS_ONLINE: p = pixOnline; break;
    case ICQ_STATUS_AWAY: p = pixAway; break;
    case ICQ_STATUS_NA: p = pixNA; break;
    case ICQ_STATUS_OCCUPIED: p = pixOccupied; break;
    case ICQ_STATUS_DND: p = pixDND; break;
    case ICQ_STATUS_FREEFORCHAT: p = pixFFC; break;
    case ICQ_STATUS_OFFLINE: p = pixOffline; break;
  }
  if (o->StatusInvisible()) p = pixInvisible;
  gUserManager.DropOwner(o);
  if (p != NULL)
  {
    QPainter painter(wharfIcon->vis);
    painter.drawPixmap(0, 0, *p);
    painter.end();
  }

  wharfIcon->repaint(false);
  repaint(false);
}


//-----IconManager_Themed::setDockIconMsg-----------------------------------
void IconManager_Themed::SetDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg)
{
  QPixmap *p = NULL;
  if (nNewMsg > 0 && nSysMsg > 0)
  {
    if (!(m_nNewMsg > 0 && m_nSysMsg > 0))
      p = pixBothMessages;
  }
  else if (nNewMsg > 0)
  {
    if (!(m_nNewMsg > 0) || m_nSysMsg > 0)
      p = pixRegularMessages;
  }
  else if (nSysMsg > 0)
  {
    if (!(m_nSysMsg > 0) || m_nNewMsg > 0)
      p = pixSystemMessages;
  }
  else
  {
    if (!(m_nNewMsg == 0 && m_nSysMsg == 0))
      p = pixNoMessages;
  }

  if (p != NULL)
  {
    wharfIcon->Set(p);
    SetDockIconStatus();
    setMask(*wharfIcon->vis->mask());
    wharfIcon->repaint(false);
    repaint(false);
  }

  m_nNewMsg = nNewMsg;
  m_nSysMsg = nSysMsg;

}

//=============================================================================

IconManager_KDEStyle::IconManager_KDEStyle(CMainWindow *_mainwin, QPopupMenu *_menu, QWidget *parent )
  : IconManager(_mainwin, _menu, parent)
{
  m_timerToggle = false;

  m_ownerStatus = 0;
  m_nSysMsg = 0;
  m_nNewMsg = 0;
  m_bStatusInvisible = false;
  resize(22, 22);
  setMinimumSize(22, 22);
  setBackgroundMode(X11ParentRelative);

#ifdef USE_KDE
  KWin::setSystemTrayWindowFor( winId(), _mainwin ? _mainwin->topLevelWidget()->winId() : qt_xrootwin() );
  setBackgroundMode(X11ParentRelative);
#else
  Display *dsp = x11Display();  // get the display
  char buffer[128];
  XEvent ev;

  snprintf(buffer, sizeof(buffer), "_NET_SYSTEM_TRAY_S%d", x11Screen());
  Atom a = XInternAtom(dsp, buffer, False);
  Window systray = XGetSelectionOwner(dsp, a);


  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = systray;
  ev.xclient.message_type = XInternAtom(dsp, "_NET_SYSTEM_TRAY_OPCODE", False);
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = SYSTEM_TRAY_REQUEST_DOCK;
  ev.xclient.data.l[2] = winId();
  ev.xclient.data.l[3] = 0;
  ev.xclient.data.l[4] = 0;

  XSendEvent(dsp, systray, False, NoEventMask, &ev);
  XSync(dsp, False);

#endif
  show();
}


IconManager_KDEStyle::~IconManager_KDEStyle()
{
}

void IconManager_KDEStyle::SetDockIconStatus()
{
  //TODO make general for all plugins
  ICQOwner* o = gUserManager.FetchOwner(LICQ_PPID, LOCK_R);
  if (o == 0) return;
  m_statusIcon = CMainWindow::iconForStatus(o->StatusFull());
  m_ownerStatus = o->Status();
  m_bStatusInvisible = o->StatusInvisible();
  gUserManager.DropOwner(o);

  updateTooltip();

  repaint();
}

void IconManager_KDEStyle::SetDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg)
{
  m_nNewMsg = nNewMsg;
  m_nSysMsg = nSysMsg;

  killTimers();
  if (m_nSysMsg > 0)
  {
    m_eventIcon = mainwin->pmReqAuthorize;
    startTimer(500);
  }
  else if (m_nNewMsg > 0)
  {
    m_eventIcon = mainwin->pmMessage;
    startTimer(500);
  }
  else
    m_eventIcon = QPixmap();

  updateTooltip();

  repaint();
}

void IconManager_KDEStyle::timerEvent(QTimerEvent* /* e */)
{
  m_timerToggle = !m_timerToggle;
  repaint();
}

void IconManager_KDEStyle::paintEvent( QPaintEvent* /* e */)
{
  QPainter p(this);

  if(m_timerToggle && !m_eventIcon.isNull())
    p.drawPixmap((width()-m_eventIcon.width())/2, (height()-m_eventIcon.height())/2, m_eventIcon);
  else
    p.drawPixmap((width()-m_statusIcon.width())/2, (height()-m_statusIcon.height())/2, m_statusIcon);
}

bool IconManager_KDEStyle::x11Event(XEvent *ev)
{
  if ((ev->type == ReparentNotify))
  {
    XWindowAttributes a;
    XGetWindowAttributes(qt_xdisplay(), ev->xreparent.parent, &a);
  //  move((a.width - width()) / 2, (a.height - height()) / 2);
  }
  return QWidget::x11Event(ev);
}

void IconManager_KDEStyle::mousePressEvent( QMouseEvent *e )
{
  switch(e->button())
  {
  case LeftButton:
    if(mainwin->isVisible() && !mainwin->isMinimized())
      mainwin->hide();
    else
    {
      mainwin->show();
#ifdef USE_KDE
      KWin::setOnDesktop(mainwin->winId(), KWin::currentDesktop());
#endif
      if (mainwin->isMaximized())
      	mainwin->showMaximized();
      else
      	mainwin->showNormal();
      mainwin->raise();
    }
    break;
   default:
     IconManager::mousePressEvent(e);
     break;
  }
}

void IconManager_KDEStyle::updateTooltip()
{
  QToolTip::remove(this);
  QString s = QString("<nobr>%1</nobr>")
      .arg(Strings::getStatus(m_ownerStatus, m_bStatusInvisible));

  if(m_nSysMsg)
    s += tr("<br><b>%1 system messages</b>").arg(m_nSysMsg);

  if(m_nNewMsg > 1)
    s += tr("<br>%1 msgs").arg(m_nNewMsg);
  else if(m_nNewMsg)
    s += tr("<br>1 msg");

  s += tr("<br>Left click - Show main window"
          "<br>Middle click - Show next message"
          "<br>Right click - System menu");

  QToolTip::add(this, s);
}

//=====WharfIcon=============================================================

WharfIcon::WharfIcon(QPixmap *p, QWidget *parent)
  : QWidget(parent, "WharfIcon")
{
  vis = NULL;
  Set(p);
  QToolTip::add(this, tr("Left click - Show main window\n"
                         "Middle click - Show next message\n"
                         "Right click - System menu"));
  //show();
}


void WharfIcon::Set(QPixmap *pix)
{
  delete vis;
  vis = new QPixmap(*pix);
  resize(vis->width(), vis->height());
  setMask(*vis->mask());
  repaint();
}

WharfIcon::~WharfIcon()
{
  delete vis;
}

void WharfIcon::mousePressEvent( QMouseEvent *e )
{
#ifdef DEBUG_WHARF
  printf("wharf press\n");
#endif
  ((IconManager *)parent())->mousePressEvent(e);
}



void WharfIcon::paintEvent( QPaintEvent * )
{
#ifdef DEBUG_WHARF
  printf("wharf paint\n");
#endif
  if (!vis) return;
  QPainter painter(this);
  painter.drawPixmap(0, 0, *vis);
  painter.end();
}

#include "wharf.moc"
