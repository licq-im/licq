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

#include "wharf.h"
#include "mainwin.h"
#include "user.h"
#include <qpainter.h>
#ifdef USE_KDE
#include <kwin.h>
#endif
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#undef Status


/*
  Constructs a WharfIcon widget.
*/


IconManager::IconManager(CMainWindow *_mainwin, QPopupMenu *_menu, bool _bFortyEight, QWidget *parent, const char *name )
  : KApplet(parent, name), wharfIcon(_mainwin, _menu, _bFortyEight, this)
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
  hints->icon_window = wharfIcon.winId();
  hints->window_group = win;  // set the window hint
  hints->flags = WindowGroupHint | IconWindowHint | StateHint; // set the window group hint
  XSetWMHints(dsp, win, hints);  // set the window hints for WM to use.
  XFree( hints );

  setCaption("LicqWharf");
  m_nNewMsg = m_nSysMsg = 0;
  m_bFortyEight = _bFortyEight;
#ifdef USE_KDE
  int argc = 0;
  init(argc, 0);
  setMinimumWidth(wharfIcon.width());
  setMinimumHeight(wharfIcon.height());
#else
  resize (wharfIcon.width(), wharfIcon.height());
  iconify();
#endif
  setMask(wharfIcon.mask);
  DrawIcon();
  show();
}

IconManager::~IconManager()
{
}

void IconManager::closeEvent( QCloseEvent*) {}


//-----CMainWindow::setDockIconStatus-------------------------------------------
void IconManager::setDockIconStatus(void)
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
  QPainter painter(wharfIcon.vis);
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

  wharfIcon.DrawIcon();
  DrawIcon();
}


//-----IconManager::GetDockIconStatusIcon--------------------------------------
QPixmap *IconManager::GetDockIconStatusIcon(void)
{
  ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
  unsigned long s = o->Status();
  gUserManager.DropOwner();
  switch (s)
  {
  case ICQ_STATUS_ONLINE: return wharfIcon.mainwin->pmOnline;
  case ICQ_STATUS_AWAY: return wharfIcon.mainwin->pmAway;
  case ICQ_STATUS_NA: return wharfIcon.mainwin->pmNa;
  case ICQ_STATUS_OCCUPIED: return (wharfIcon.mainwin->pmOccupied);
  case ICQ_STATUS_DND: return wharfIcon.mainwin->pmDnd;
  case ICQ_STATUS_FREEFORCHAT: return wharfIcon.mainwin->pmChat;
  case ICQ_STATUS_OFFLINE: return wharfIcon.mainwin->pmOffline;
  }
  return NULL;
}

//-----CMainWindow::setDockIconMsg----------------------------------------------
void IconManager::setDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg)
{
  QPainter p(wharfIcon.vis);
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
      m = wharfIcon.mainwin->pmMessage;
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

  wharfIcon.DrawIcon();
  DrawIcon();
}

void IconManager::mouseReleaseEvent( QMouseEvent *e )
{
  wharfIcon.mouseReleaseEvent(e);
}

WharfIcon::WharfIcon(CMainWindow *_mainwin, QPopupMenu *_menu, bool _bFortyEight,
                     QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  mainwin = _mainwin;
  menu = _menu;
  if (_bFortyEight)
  {
    vis = new QPixmap((const char **)iconBack_48_xpm);
    QPixmap p((const char **)iconMask_48_xpm);
    mask = p;
  }
  else
  {
    vis = new QPixmap((const char **)iconBack_64_xpm);
    QPixmap p((const char **)iconMask_64_xpm);
    mask = p;
  }
  resize(vis->width(), vis->height());
  setMask(mask);

  DrawIcon();
  show();
}

WharfIcon::~WharfIcon()
{
  delete vis;
}

void WharfIcon::DrawIcon(void)
{
  QPainter painter(this);
  painter.drawPixmap(0, 0, *vis);
  painter.end();
}


void IconManager::DrawIcon(void)
{
  QPainter painter(this);
  painter.drawPixmap(0, 0, *wharfIcon.vis);
  painter.end();
}


void WharfIcon::mouseReleaseEvent( QMouseEvent *e )
{
  switch(e->button())
  {
  case LeftButton:
    mainwin->show();
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
  DrawIcon();
}

void IconManager::paintEvent( QPaintEvent * )
{
  DrawIcon();
}

#include "wharf.moc"
