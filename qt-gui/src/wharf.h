#ifndef WHARF_H
#define WHARF_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpixmap.h>
#include <qbitmap.h>
#include <qpopupmenu.h>

#ifndef USE_KDE
#define KPanelApplet QWidget
#else
#include <kpanelapplet.h>
#endif


class WharfIcon : public QWidget
{
public:
  WharfIcon(class CMainWindow *, QPopupMenu *, QPixmap *, QWidget *parent= 0);
  virtual ~WharfIcon();
  void Set(QPixmap *);
protected:
  virtual void mouseReleaseEvent (QMouseEvent *);
  virtual void paintEvent (QPaintEvent *);
  QPixmap *vis;
  class CMainWindow *mainwin;
  QPopupMenu *menu;
friend class IconManager;
friend class IconManager_Default;
friend class IconManager_Themed;
};


class IconManager : public KPanelApplet
{
public:
  IconManager(QWidget *parent = 0);
  virtual ~IconManager();
  virtual void SetDockIconStatus() = 0;
  virtual void SetDockIconMsg(unsigned short nNewMsg, unsigned short nSysMsg) = 0;
protected:
  void X11Init();
  virtual void mouseReleaseEvent(QMouseEvent *);
  virtual void closeEvent (QCloseEvent *);
  virtual void paintEvent (QPaintEvent *);
#ifdef USE_KDE
  virtual int widthForHeight(int height);
  virtual int heightForWidth(int width);
  virtual void removedFromPanel();
#endif
  WharfIcon *wharfIcon;
  int m_nNewMsg, m_nSysMsg;
  int mouseX, mouseY;
friend class WharfIcon;
};


class IconManager_Default : public IconManager
{
public:
  IconManager_Default(class CMainWindow *, QPopupMenu *, bool, QWidget *parent = 0);
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
public:
  IconManager_Themed(class CMainWindow *, QPopupMenu *, const char *, QWidget *parent = 0);
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


#endif
