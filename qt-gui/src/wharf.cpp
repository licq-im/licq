/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

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
#endif

#include "wharf.h"
#include "mainwin.h"
#include "licq_user.h"
#include "ewidgets.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#undef Status
#undef Bool
#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut


/*
  Constructs a WharfIcon widget.
*/
IconManager::IconManager(QWidget *parent )
  : KPanelApplet(parent, "LicqWharf")
{
  setCaption("LicqWharf");
  m_nNewMsg = m_nSysMsg = 0;
  wharfIcon = NULL;
}


void IconManager::X11Init()
{
  // set the hints
  Display *dsp = x11Display();  // get the display
  WId win = winId();     // get the window
  XWMHints *hints;  // hints
  XClassHint classhint;  // class hints
  classhint.res_name = "licq";  // res_name
  classhint.res_class = "Wharf";  // res_class
  XSetClassHint(dsp, win, &classhint); // set the class hints
  hints = XGetWMHints(dsp, win);  // init hints
  hints->initial_state = WithdrawnState;
  hints->icon_x = 0;
  hints->icon_y = 0;
  hints->icon_window = wharfIcon->winId();
  hints->window_group = win;  // set the window hint
  hints->flags = WindowGroupHint | IconWindowHint | IconPositionHint | StateHint; // set the window group hint
  XSetWMHints(dsp, win, hints);  // set the window hints for WM to use.
  XFree( hints );

#ifdef USE_KDE
  //int argc = 0;
  //init(argc, 0);
  dock("LicqWharf");
  setMinimumWidth(wharfIcon->width());
  setMinimumHeight(wharfIcon->height());
#endif
  resize (wharfIcon->width(), wharfIcon->height());
  setMask(*wharfIcon->vis->mask());
  show();
}



IconManager::~IconManager()
{
  delete wharfIcon;
  // clean up behind us the prevent segfaults
  gMainWindow->licqIcon = NULL;
}

void IconManager::closeEvent( QCloseEvent* e)
{
  gMainWindow->slot_shutdown();
  e->accept();
}

#ifdef USE_KDE
int IconManager::widthForHeight(int)
{
  return wharfIcon->width();
}

int IconManager::heightForWidth(int)
{
  return wharfIcon->height();
}

void IconManager::removedFromPanel()
{
  InformUser(this, tr("The applet can be removed by\ndisabling it in the options dialog"));
}
#endif

void IconManager::mouseReleaseEvent( QMouseEvent *e )
{
#ifdef DEBUG_WHARF
  printf("icon release\n");
#endif
  wharfIcon->mouseReleaseEvent(e);
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
  : IconManager(parent)
{
  m_bFortyEight = _bFortyEight;
  if (m_bFortyEight)
  {
    pix = new QPixmap((const char **)iconBack_48_xpm);
    QBitmap b;
    b = QPixmap((const char **)iconMask_48_xpm);
    pix->setMask(b);
    wharfIcon = new WharfIcon(_mainwin, _menu, pix, this);
  }
  else
  {
    pix = new QPixmap((const char **)iconBack_64_xpm);
    QBitmap b;
    b = QPixmap((const char **)iconMask_64_xpm);
    pix->setMask(b);
    wharfIcon = new WharfIcon(_mainwin, _menu, pix, this);
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
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
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
  gUserManager.DropOwner();
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
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned long s = o->Status();
  gUserManager.DropOwner();
  switch (s)
  {
    case ICQ_STATUS_ONLINE: return &wharfIcon->mainwin->pmOnline;
    case ICQ_STATUS_AWAY: return &wharfIcon->mainwin->pmAway;
    case ICQ_STATUS_NA: return &wharfIcon->mainwin->pmNa;
    case ICQ_STATUS_OCCUPIED: return &wharfIcon->mainwin->pmOccupied;
    case ICQ_STATUS_DND: return &wharfIcon->mainwin->pmDnd;
    case ICQ_STATUS_FREEFORCHAT: return &wharfIcon->mainwin->pmChat;
    case ICQ_STATUS_OFFLINE: return &wharfIcon->mainwin->pmOffline;
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
    if (nSysMsg > 0 || nNewMsg > 0)
      m = &wharfIcon->mainwin->pmMessage;
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
  : IconManager(parent)
{
  pixNoMessages = pixBothMessages = pixRegularMessages = pixSystemMessages = NULL;
  pixOnline = pixOffline = pixAway = pixNA = pixOccupied = pixDND = pixInvisible = pixFFC = NULL;

  SetTheme(theme);

  wharfIcon = new WharfIcon(_mainwin, _menu, pixNoMessages, this);
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
  sprintf(filename, "%s%s.dock", QFile::encodeName(baseDockDir).data(), theme);
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
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
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
  gUserManager.DropOwner();
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

//=====WharfIcon=============================================================

WharfIcon::WharfIcon(CMainWindow *_mainwin, QPopupMenu *_menu, QPixmap *p,
                     QWidget *parent)
  : QWidget(parent, "WharfIcon")
{
  mainwin = _mainwin;
  menu = _menu;
  vis = NULL;
  Set(p);
  QToolTip::add(this, tr("Left click - Show main window\n"
                         "Middle click - Show next message\n"
                         "Right click - System menu"));
  show();
}


void WharfIcon::Set(QPixmap *pix)
{
  delete vis;
  vis = new QPixmap(*pix);
  resize(vis->width(), vis->height());
  setMask(*vis->mask());
}

WharfIcon::~WharfIcon()
{
  delete vis;
}

void WharfIcon::mouseReleaseEvent( QMouseEvent *e )
{
#ifdef DEBUG_WHARF
  printf("wharf release\n");
#endif
  switch(e->button())
  {
    case LeftButton:
      /* I don't like this, and it causes Licq to disappear forever with certain
         (probably buggy) window managers such as enlightenment.
      if(mainwin->isVisible())
        mainwin->hide();
      else*/
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
}



void WharfIcon::paintEvent( QPaintEvent * )
{
#ifdef DEBUG_WHARF
  printf("wharf paint\n");
#endif
  QPainter painter(this);
  painter.drawPixmap(0, 0, *vis);
  painter.end();
}


