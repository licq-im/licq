#ifndef LISTBOXITEM_H
#define LISTBOXITEM_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qlistview.h>
#include <qpopmenu.h>
#include <vector.h>
#include <qpixmap.h>
#include <qfont.h>

#include "user.h"
/*
#ifdef USE_KDE
#include <drag.h>
#else
class KDNDDropZone {};
#endif
*/
#define COL_ALIAS 1
#define COL_UIN 2
#define COL_NAME 3
#define COL_EMAIL 4

class ColInfo
{
public:
   ColInfo(unsigned theInfo = 0, unsigned short theWidth = 0, unsigned short theAlign = 0)
      { info = theInfo; width = theWidth; align = (unsigned short)theAlign; };
   unsigned short info, width, align;
};


//=====UserViewItem================================================================================
class CUserViewItem : public QListViewItem
{
public:
   CUserViewItem (ICQUser *, short, QListView *);
   virtual QString key(int column, bool ascending) const;
   unsigned long ItemUin(void)  { return m_nUin; };
protected:
  void setGraphics(ICQUser *);
  virtual void paintCell ( QPainter *, const QColorGroup &, int column, int width, int align);
  virtual void paintFocus ( QPainter *, const QColorGroup & cg, const QRect & r );
  QColor *m_cFore, *m_cBack;
  QPixmap *m_pIcon;

  unsigned long m_nUin;
  short m_nIndex;
  QFont::Weight m_nWeight;
  bool m_bItalic, m_bStrike;

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
};


//=====UserView====================================================================================
class CUserView : public QListView
{
  Q_OBJECT
public:
   CUserView (QPopupMenu *m, QPopupMenu *mg, vector <ColInfo> colInfo,
              bool isHeader, bool _bGridLines, bool _bFontStyles,
              QWidget *parent = 0, const char *name = 0);
   void maxLastColumn(void);

   void setPixmaps(QPixmap *_pOnline, QPixmap *_pOffline, QPixmap *_pAway,
                   QPixmap *_pNa, QPixmap *_pOccupied, QPixmap *_pDnd,
                   QPixmap *_pPrivate, QPixmap *_pFFC, QPixmap *_pMessage,
                   QPixmap *_pUrl, QPixmap *_pChat, QPixmap *_pFile);
   void setColors(char *_sOnline, char *_sAway, char *_sOffline,
                  char *_sNew, char *_sBack, char *_sGridLines);

   void setShowHeader(bool);
   unsigned long SelectedItemUin(void);

   bool getGridLines(void)  { return CUserViewItem::s_bGridLines; };
   void setGridLines(bool _b)  { CUserViewItem::s_bGridLines = _b; };
   void setFontStyles(bool _b)  { CUserViewItem::s_bFontStyles = _b; };

   vector <ColInfo> colInfo;
protected:
   QPopupMenu *mnuUser, *mnuGroup;
   virtual void viewportMouseReleaseEvent(QMouseEvent *e);
   virtual void keyPressEvent(QKeyEvent *e);
/*
public slots:
  void slot_dropAction(KDNDDropZone *);
  void slot_dropEnter(KDNDDropZone *);
  void slot_dropLeave(KDNDDropZone *);
signals:
  void signal_dropedFile(const char *);
  void signal_dropedURL(const char *);
*/
};


#endif
