#ifndef WHARF_H
#define WHARF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpopupmenu.h>

class WharfIcon : public QWidget
{
  Q_OBJECT
public:
  WharfIcon(class CMainWindow *, QPopupMenu *, QWidget *parent = NULL, const char *name = NULL);
  virtual ~WharfIcon(void);
  void DrawIcon(void);
protected:
  virtual void mouseReleaseEvent (QMouseEvent *);
  virtual void paintEvent (QPaintEvent *);
  QPixmap vis;
  QBitmap mask;
  class CMainWindow *mainwin;
  QPopupMenu *menu;
friend class IconManager;
};


class IconManager : public QWidget
{
  Q_OBJECT
public:
  IconManager(class CMainWindow *, QPopupMenu *, QWidget *parent = NULL, const char *name = NULL);
  virtual ~IconManager(void);
  void setDockIconStatus(void);
  void setDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg);
  void DrawIcon(void);
protected:
  QPixmap *GetDockIconStatusIcon(void);
  virtual void mouseReleaseEvent( QMouseEvent *e );
  virtual void closeEvent (QCloseEvent *);
  virtual void paintEvent (QPaintEvent *);
  WharfIcon wharfIcon;
  int m_nNewMsg, m_nSysMsg;
};


#endif
