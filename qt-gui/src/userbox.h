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

#ifndef USERBOX_H
#define USERBOX_H

#include <vector>

#include <qlistview.h>
#include <qtooltip.h>
#include <qptrvector.h>

class QTimer;
class ICQUser;
class CUserView;

enum BarType
{
  BAR_ONLINE,
  BAR_OFFLINE,
  BAR_NOTINLIST
};

enum FlashType
{
  FLASH_NONE,
  FLASH_ALL,
  FLASH_URGENT
};

class CColumnInfo
{
public:
  CColumnInfo(QString _sTitle, const char *_szFormat,
              unsigned short _nWidth, unsigned short _nAlign)
    { m_sTitle = _sTitle;
      m_szFormat = _szFormat;
      m_nWidth = _nWidth;
      m_nAlign = _nAlign; }
  ~CColumnInfo() {};

  QString m_sTitle;
  QCString m_szFormat;
  unsigned short m_nWidth, m_nAlign;
};

typedef std::vector<CColumnInfo *> ColumnInfos;


//=====UserViewItem================================================================================
class CUserViewItem : public QListViewItem
{
public:
  CUserViewItem (ICQUser *, QListView *);
  CUserViewItem (ICQUser *, CUserViewItem* item);
  CUserViewItem (BarType, QListView *);
  CUserViewItem(unsigned short Id, const char* name, unsigned short sortKey, QListView* parent);

  virtual ~CUserViewItem();
  virtual QString key(int column, bool ascending) const;
  char *ItemId() { return m_szId; }
  unsigned long ItemPPID() { return m_nPPID; }
  unsigned short GroupId() const { return m_nGroupId; }
  bool isGroupItem() const { return (!m_szId && !m_nPPID && m_nGroupId != (unsigned short)(-1)); }
  QCString  GroupName() const { return m_sGroupName; }
  void setGraphics(ICQUser *);
  unsigned short Status() const { return m_nStatus; };
  CUserViewItem* firstChild() const { return static_cast<CUserViewItem*>(QListViewItem::firstChild()); }
  CUserViewItem* nextSibling() const { return static_cast<CUserViewItem*>(QListViewItem::nextSibling()); }
  CUserView* listView() const { return (CUserView*) QListViewItem::listView(); }

protected:
  virtual void paintCell ( QPainter *, const QColorGroup &, int column, int width, int align);
  virtual void paintFocus(QPainter *, const QColorGroup &, const QRect &) {}

  void drawCAROverlay(QPainter*);
  void SetThreadViewGroupTitle();

  QColor *m_cFore, *m_cBack;
  QPixmap *m_pIcon, *m_pIconStatus, *m_pUserIcon;

  char *m_szId;
  QString m_szAlias;
  unsigned long m_nPPID;
  unsigned short m_nStatus;
  unsigned long m_nStatusFull;
  unsigned short m_nGroupId;
  int m_nOnlCount, m_nEvents;
  QCString m_sGroupName;
  bool m_bStatusInvisible, m_bStatusTyping, m_bCustomAR, m_bSecure, m_bFlash, m_bBirthday, m_bPhone, m_bCellular, m_bGPGKey, m_bGPGKeyEnabled, m_bNotInList;
  unsigned long m_nPhoneFollowMeStatus, m_nICQphoneStatus, m_nSharedFilesStatus;
  QFont::Weight m_nWeight;
  bool m_bItalic, m_bStrike, m_bUrgent;
  QString m_sPrefix, m_sSortKey;

  static QColor  *s_cOnline,
                 *s_cAway,
                 *s_cOffline,
                 *s_cNew,
                 *s_cBack,
                 *s_cGridLines,
                 *s_cGroupBack;

private:
  enum alignment { LEFT = 1, RIGHT = 2, CENTER = 4 };

  friend class CUserView;
};

class CUserView;
  typedef QPtrVector<CUserView> UserFloatyList;


//=====UserView===============================================================
class CUserView : public QListView, public QToolTip
{
  Q_OBJECT
public:
  CUserView (QPopupMenu *m, QWidget *parent = 0, const char *name = 0);
  virtual ~CUserView();

  virtual void clear();

  CUserViewItem *firstChild() const { return (CUserViewItem *)QListView::firstChild(); }

  void setColors(char *_sOnline, char *_sAway, char *_sOffline,
                 char *_sNew, char *_sBack, char *_sGridLines, char *_sGroupBack);
  void setShowHeader(bool);
  void AnimationAutoResponseCheck(const char *, unsigned long);
  void AnimationOnline(const char *, unsigned long);
  bool MainWindowSelectedItemUser(char *&, unsigned long &);

  static UserFloatyList* floaties;
  static CUserView *FindFloaty(const char *, unsigned long);
  static void UpdateFloaties();
  virtual void setSorting( int column, bool ascending = true);

public slots:
  void updateItems();

protected:
  int m_nFlashCounter;
  int msgTimerId;
  int onlTimerId, onlCounter;
  int carTimerId, carCounter;
  char *carId;
  char *onlId;
  unsigned long carPPID;
  unsigned long onlPPID;
  QTimer *tmrRefresh;

  QString m_typeAhead;	    /*! type-ahead buffer  */
  unsigned int m_typePos;   /*! hmmm */

  QPopupMenu *mnuUser;
  CUserViewItem *barOnline, *barOffline, *barNotInList;
  QPoint mousePressPos;
  int numOnline, numOffline, numNotInList;
  virtual void timerEvent(QTimerEvent*);

  virtual void viewportMousePressEvent(QMouseEvent *e);
  virtual void viewportMouseReleaseEvent(QMouseEvent *e);
  virtual void viewportMouseMoveEvent(QMouseEvent * me);
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void paintEmptyArea( QPainter *, const QRect & );

  virtual void viewportDragEnterEvent(QDragEnterEvent*);
  virtual void viewportDropEvent(QDropEvent*);
  virtual void contentsContextMenuEvent ( QContextMenuEvent * );

  virtual void resizeEvent(QResizeEvent *);
  virtual void maybeTip(const QPoint&);

  friend class CUserViewItem;
protected slots:
  void itemExpanded(QListViewItem*);
  void itemCollapsed(QListViewItem*);
};

#endif
