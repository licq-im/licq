#ifndef USERBOX_H
#define USERBOX_H

#include <vector.h>

#include <qlistview.h>
#include <qtooltip.h>

class ICQUser;
class CUserView;

enum BarType
{
  BAR_ONLINE,
  BAR_OFFLINE
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
  CUserViewItem (BarType, QListView *);
  virtual QString key(int column, bool ascending) const;
  unsigned long ItemUin()  { return m_nUin; }
  void setGraphics(ICQUser *);
protected:
  virtual void paintCell ( QPainter *, const QColorGroup &, int column, int width, int align);
  virtual void paintFocus ( QPainter *, const QColorGroup & cg, const QRect & r );
  QColor *m_cFore, *m_cBack;
  QPixmap *m_pIcon;

  unsigned long m_nUin;
  unsigned short m_nStatus;
  QFont::Weight m_nWeight;
  bool m_bItalic, m_bStrike;
  QString m_sPrefix, m_sSortKey;

  static bool    s_bGridLines, s_bFontStyles;
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
                 *s_pFFC;
  static QColor  *s_cOnline,
                 *s_cAway,
                 *s_cOffline,
                 *s_cNew,
                 *s_cBack,
                 *s_cGridLines;

  friend class CUserView;
  friend class CUserViewTips;
public: 
  unsigned short Status() const { return m_nStatus; };

};

//=====UserView====================================================================================
class CUserView : public QListView
{
  Q_OBJECT
public:
  CUserView (QPopupMenu *m, QPopupMenu *mg, ColumnInfos _colInfo,
             bool isHeader, bool _bGridLines, bool _bFontStyles,
             bool bTransparent, bool bShowBars,
             QWidget *parent = 0, const char *name = 0);
  ~CUserView();

  virtual void clear();
  void maxLastColumn();

  void setPixmaps(QPixmap *_pOnline, QPixmap *_pOffline, QPixmap *_pAway,
                  QPixmap *_pNa, QPixmap *_pOccupied, QPixmap *_pDnd,
                  QPixmap *_pPrivate, QPixmap *_pFFC, QPixmap *_pMessage,
                  QPixmap *_pUrl, QPixmap *_pChat, QPixmap *_pFile);
  void setColors(char *_sOnline, char *_sAway, char *_sOffline,
                 char *_sNew, char *_sBack, char *_sGridLines);

  void setShowHeader(bool);
  void setShowBars(bool);
  unsigned long SelectedItemUin();

  bool ShowBars(void)  { return m_bShowBars; }

  bool getGridLines()  { return CUserViewItem::s_bGridLines; };
  void setGridLines(bool _b)  { CUserViewItem::s_bGridLines = _b; };
  void setFontStyles(bool _b)  { CUserViewItem::s_bFontStyles = _b; };

protected:
  QPopupMenu *mnuUser, *mnuGroup;
  bool m_bTransparent, m_bShowBars;
  CUserViewTips* m_tips;
  ColumnInfos colInfo;
  CUserViewItem *barOnline, *barOffline;
  virtual void viewportMousePressEvent(QMouseEvent *e);
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void paintEmptyArea( QPainter *, const QRect & );

friend class CUserViewItem;
};

#endif
