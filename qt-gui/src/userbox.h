#ifndef USERBOX_H
#define USERBOX_H

#include <vector.h>
#include <list.h>

#include <qlistview.h>
#include <qtooltip.h>

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
class CUserViewItem : public QObject, public QListViewItem
{
  Q_OBJECT
public:
  CUserViewItem (ICQUser *, QListView *);
  CUserViewItem (BarType, QListView *);
  virtual ~CUserViewItem();
  virtual QString key(int column, bool ascending) const;
  unsigned long ItemUin()  { return m_nUin; }
  void setGraphics(ICQUser *);
  unsigned short Status() const { return m_nStatus; };
protected:
  virtual void paintCell ( QPainter *, const QColorGroup &, int column, int width, int align);
  virtual void paintFocus ( QPainter *, const QColorGroup & cg, const QRect & r ) { };

  QColor *m_cFore, *m_cBack;
  QPixmap *m_pIcon, *m_pIconStatus;

  static QTimer *s_tFlash;
  static int s_nFlashCounter;
  static FlashType s_nFlash;

  unsigned long m_nUin;
  unsigned short m_nStatus;
  QFont::Weight m_nWeight;
  bool m_bItalic, m_bStrike, m_bUrgent;
  QString m_sPrefix, m_sSortKey;

  static bool    s_bGridLines, s_bFontStyles, s_bSortByStatus, s_bFlashUrgent;
  static QPixmap *s_pOnline,
                 *s_pOffline,
                 *s_pAway,
                 *s_pNa,
                 *s_pOccupied,
                 *s_pDnd,
                 *s_pPrivate,
                 *s_pMessage,
                 *s_pUrl,
                 *s_pChat,
                 *s_pFile,
                 *s_pFFC,
                 *s_pNone;
  static QColor  *s_cOnline,
                 *s_cAway,
                 *s_cOffline,
                 *s_cNew,
                 *s_cBack,
                 *s_cGridLines;

  friend class CUserView;
  friend class CUserViewTips;

protected slots:
  void slot_flash();

};

class CUserView;
typedef list<CUserView *> UserFloatyList;


//=====UserView===============================================================
class CUserView : public QListView
{
  Q_OBJECT
public:
  CUserView (QPopupMenu *m, QPopupMenu *mg, QPopupMenu *ma, ColumnInfos _colInfo,
             bool isHeader, bool _bGridLines, bool _bFontStyles,
             bool bTransparent, bool bShowBars, bool bSortByStatus,
             FlashType nFlash,
             QWidget *parent = 0, const char *name = 0);
  virtual ~CUserView();

  virtual void clear();
  //void maxLastColumn();

  virtual CUserViewItem *firstChild() { return (CUserViewItem *)QListView::firstChild(); }

  void setPixmaps(QPixmap *_pOnline, QPixmap *_pOffline, QPixmap *_pAway,
                  QPixmap *_pNa, QPixmap *_pOccupied, QPixmap *_pDnd,
                  QPixmap *_pPrivate, QPixmap *_pFFC, QPixmap *_pMessage,
                  QPixmap *_pUrl, QPixmap *_pChat, QPixmap *_pFile);
  void setColors(char *_sOnline, char *_sAway, char *_sOffline,
                 char *_sNew, char *_sBack, char *_sGridLines);

  void setSortByStatus(bool);
  void setShowHeader(bool);
  void setShowBars(bool);

  static unsigned long SelectedItemUin();
  static bool SelectedItemFloaty();
  static CUserViewItem *SelectedItem();

  bool ShowBars(void)  { return m_bShowBars; }

  bool getGridLines()  { return CUserViewItem::s_bGridLines; };
  void setGridLines(bool _b)  { CUserViewItem::s_bGridLines = _b; };
  void setFontStyles(bool _b)  { CUserViewItem::s_bFontStyles = _b; };

  static UserFloatyList floaties;
  static CUserView *FindFloaty(unsigned long);
  static void UpdateFloaties();

protected:
  QPopupMenu *mnuUser, *mnuGroup, *mnuAwayModes;
  bool m_bTransparent, m_bShowBars;
  CUserViewTips *m_tips;
  ColumnInfos colInfo;
  CUserViewItem *barOnline, *barOffline;
  QPoint mousePressPos;
  int numOnline, numOffline;
  virtual void viewportMousePressEvent(QMouseEvent *e);
  virtual void viewportMouseReleaseEvent(QMouseEvent *e);
  virtual void viewportMouseMoveEvent(QMouseEvent * me);
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void paintEmptyArea( QPainter *, const QRect & );

  virtual void hideEvent(QHideEvent *);
  virtual void resizeEvent(QResizeEvent *);

  static unsigned long s_nUin;
  static bool s_bFloaty;
  static CUserViewItem *s_pItem;

  friend class CUserViewItem;

protected slots:
  void slot_flash();
};

#endif
