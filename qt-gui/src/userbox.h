#ifndef USERBOX_H
#define USERBOX_H

#include <vector.h>

#include <qlistview.h>
#include <qtooltip.h>
#include <qvector.h>

class ICQUser;
class CUserView;

enum BarType
{
  BAR_ONLINE,
  BAR_OFFLINE
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

typedef vector<CColumnInfo *> ColumnInfos;


//=====CUserViewTips===============================================================================

class CUserViewTips : public QToolTip
{
public:
  CUserViewTips(CUserView* parent);
  virtual ~CUserViewTips() {};

protected:
  virtual void maybeTip(const QPoint&);
};


//=====UserViewItem================================================================================
class CUserViewItem : public QListViewItem
{
public:
  CUserViewItem (ICQUser *, QListView *);
  CUserViewItem (ICQUser *, CUserViewItem* item);
  CUserViewItem (BarType, QListView *);
  CUserViewItem (unsigned short Id, const char* name, QListView *);

  virtual ~CUserViewItem();
  virtual QString key(int column, bool ascending) const;
  unsigned long ItemUin() const { return m_nUin; }
  unsigned short GroupId() const { return m_nGroupId; }
  bool isGroupItem() const { return (m_nUin == 0 && m_nGroupId != (unsigned short)(-1)); }
  QCString  GroupName() const { return m_sGroupName; }
  void setGraphics(ICQUser *);
  unsigned short Status() const { return m_nStatus; };
  CUserViewItem* firstChild() const { return static_cast<CUserViewItem*>(QListViewItem::firstChild()); }
  CUserViewItem* nextSibling() const { return static_cast<CUserViewItem*>(QListViewItem::nextSibling()); }
  CUserView* listView() const { return (CUserView*) QListViewItem::listView(); }

protected:
  virtual void paintCell ( QPainter *, const QColorGroup &, int column, int width, int align);
  virtual void paintFocus ( QPainter *, const QColorGroup & cg, const QRect & r ) { };

  void drawCAROverlay(QPainter*);
  void SetThreadViewGroupTitle();

  QColor *m_cFore, *m_cBack;
  QPixmap *m_pIcon, *m_pIconStatus;

  unsigned long m_nUin;
  unsigned short m_nStatus;
  unsigned long m_nStatusFull;
  unsigned short m_nGroupId;
  int m_nOnlCount, m_nEvents;
  QCString m_sGroupName;
  bool m_bStatusInvisible, m_bCustomAR, m_bSecure;
  QFont::Weight m_nWeight;
  bool m_bItalic, m_bStrike, m_bUrgent;
  QString m_sPrefix, m_sSortKey;

  static QColor  *s_cOnline,
                 *s_cAway,
                 *s_cOffline,
                 *s_cNew,
                 *s_cBack,
                 *s_cGridLines;

  friend class CUserView;
  friend class CUserViewTips;
};

class CUserView;
typedef QVector<CUserView> UserFloatyList;


//=====UserView===============================================================
class CUserView : public QListView
{
  Q_OBJECT
public:
  CUserView (QPopupMenu *m, QWidget *parent = 0, const char *name = 0);
  virtual ~CUserView();

  virtual void clear();

  CUserViewItem *firstChild() const { return (CUserViewItem *)QListView::firstChild(); }

  void setColors(char *_sOnline, char *_sAway, char *_sOffline,
                 char *_sNew, char *_sBack, char *_sGridLines);
  void setShowHeader(bool);
  void AnimationAutoResponseCheck(unsigned long uin);
  void AnimationOnline(unsigned long uin);

  unsigned long MainWindowSelectedItemUin();

  static UserFloatyList* floaties;
  static CUserView *FindFloaty(unsigned long);
  static void UpdateFloaties();

protected:
  int m_nFlashCounter;
  int msgTimerId;
  int onlTimerId, onlCounter;
  int carTimerId, carCounter;
  unsigned long carUin;
  unsigned long onlUin;

  QPopupMenu *mnuUser;
  CUserViewTips *m_tips;
  CUserViewItem *barOnline, *barOffline;
  QPoint mousePressPos;
  QPixmap *pixExpanded, *pixCollapsed, *pixCustomAR, *pixBirthday, *pixInvisible;
  int numOnline, numOffline;
  virtual void timerEvent(QTimerEvent*);
  virtual void viewportMousePressEvent(QMouseEvent *e);
  virtual void viewportMouseReleaseEvent(QMouseEvent *e);
  virtual void viewportMouseMoveEvent(QMouseEvent * me);
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void paintEmptyArea( QPainter *, const QRect & );

  virtual void viewportDragEnterEvent(QDragEnterEvent*);
  virtual void viewportDropEvent(QDropEvent*);

  virtual void resizeEvent(QResizeEvent *);

  friend class CUserViewItem;
protected slots:
  void itemExpanded(QListViewItem*);
  void itemCollapsed(QListViewItem*);
};

#endif
