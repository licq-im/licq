#ifndef WHARF_H
#define WHARF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpopupmenu.h>

#ifndef USE_KDE
#define KApplet QWidget
#else
#include <kapplet.h>
#endif


class WharfIcon : public QWidget
{
public:
  WharfIcon(class CMainWindow *, QPopupMenu *, bool, QWidget *parent = NULL, const char *name = NULL);
  virtual ~WharfIcon();
protected:
  virtual void mouseReleaseEvent (QMouseEvent *);
  virtual void paintEvent (QPaintEvent *);
  QPixmap *vis;
  QBitmap mask;
  class CMainWindow *mainwin;
  QPopupMenu *menu;
friend class IconManager;
};


class IconManager : public KApplet
{
public:
  IconManager(class CMainWindow *, QPopupMenu *, bool, QWidget *parent = NULL, const char *name = NULL);
  virtual ~IconManager();
  void setDockIconStatus();
  void setDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg);
protected:
  QPixmap *GetDockIconStatusIcon();
  virtual void mouseReleaseEvent( QMouseEvent *e );
  virtual void closeEvent (QCloseEvent *);
  virtual void paintEvent (QPaintEvent *);
#ifdef USE_KDE
  void setupGeometry(Orientation orientation, int width, int height);
#endif
  WharfIcon wharfIcon;
  int m_nNewMsg, m_nSysMsg;
  bool m_bFortyEight;
};

#endif
