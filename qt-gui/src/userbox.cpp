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

#include <qpainter.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qkeycode.h>
#include <qscrollbar.h>

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "userbox.h"
#include "gui-defines.h"

#include "user.h"

bool    CUserViewItem::s_bGridLines,
        CUserViewItem::s_bFontStyles;
QPixmap *CUserViewItem::s_pOnline = NULL,
        *CUserViewItem::s_pOffline = NULL,
        *CUserViewItem::s_pAway = NULL,
        *CUserViewItem::s_pNa = NULL,
        *CUserViewItem::s_pOccupied = NULL,
        *CUserViewItem::s_pDnd = NULL,
        *CUserViewItem::s_pPrivate = NULL,
        *CUserViewItem::s_pMessage = NULL,
        *CUserViewItem::s_pUrl = NULL,
        *CUserViewItem::s_pChat = NULL,
        *CUserViewItem::s_pFile = NULL,
        *CUserViewItem::s_pFFC = NULL;
QColor  *CUserViewItem::s_cOnline = NULL,
        *CUserViewItem::s_cAway = NULL,
        *CUserViewItem::s_cOffline = NULL,
        *CUserViewItem::s_cNew = NULL,
        *CUserViewItem::s_cBack = NULL,
        *CUserViewItem::s_cGridLines = NULL;


//-----CUserViewItem::constructor-----------------------------------------------
CUserViewItem::CUserViewItem(ICQUser *_cUser, short _nIndex, QListView *parent)
   : QListViewItem(parent)
{
  m_nIndex = _nIndex;
  if (_cUser == NULL)
  {
    m_nUin = 0;
    m_pIcon = NULL;
    m_cBack = s_cBack;
    m_cFore = s_cOnline;
    m_bItalic = m_bStrike = false;
    m_nWeight = QFont::Normal;
    setSelectable(false);
    setHeight(10);
    if (m_nIndex == -1) m_sPrefix = "0";
    else m_sPrefix = "2";
  }
  else
  {
    m_nUin = _cUser->Uin();
    m_status = _cUser->Status();
    setGraphics(_cUser);
  }

}


//-----CUserViewItem::setGraphics-----------------------------------------------
void CUserViewItem::setGraphics(ICQUser *u)
{
   static char sTemp[128];

   m_sPrefix = "1";
   switch (u->Status())
   {
   case ICQ_STATUS_FREEFORCHAT:
      m_pIcon = s_pFFC;
      m_cFore = s_cOnline;
      break;
   case ICQ_STATUS_ONLINE:
      m_pIcon = s_pOnline;
      m_cFore = s_cOnline;
      break;
   case ICQ_STATUS_AWAY:
      m_pIcon = s_pAway;
      m_cFore = s_cAway;
      break;
   case ICQ_STATUS_OCCUPIED:
      m_pIcon = s_pOccupied;
      m_cFore = s_cAway;
      break;
   case ICQ_STATUS_DND:
      m_pIcon = s_pDnd;
      m_cFore = s_cAway;
      break;
   case ICQ_STATUS_NA:
      m_pIcon = s_pNa;
      m_cFore = s_cAway;
      break;
   case ICQ_STATUS_OFFLINE:
      m_pIcon = s_pOffline;
      m_cFore = s_cOffline;
      m_sPrefix = "3";
      break;
   default:
      m_pIcon = s_pOnline;
      m_cFore = s_cOnline;
      break;
   }

   if (u->StatusInvisible())
   {
      m_pIcon = s_pPrivate;
      m_cFore = s_cAway;
   }

   if (u->NewMessages() > 0)
   {
     m_pIcon = NULL;
     for (unsigned short i = 0; i < u->NewMessages(); i++)
     {
       switch(u->GetEvent(i)->SubCommand())
       {
       case ICQ_CMDxSUB_MSG:
         if (m_pIcon == NULL)
           m_pIcon = s_pMessage;
         break;
       case ICQ_CMDxSUB_URL:
         if (m_pIcon == NULL || m_pIcon == s_pMessage)
           m_pIcon = s_pUrl;
         break;
       case ICQ_CMDxSUB_CHAT:
         if (m_pIcon == NULL || m_pIcon == s_pMessage || m_pIcon == s_pUrl)
           m_pIcon = s_pChat;
         break;
       case ICQ_CMDxSUB_FILE:
         m_pIcon = s_pFile;
         break;
       default:
         m_pIcon = s_pMessage;
         break;
       }
     }
   }

   if (u->NewUser())
      m_cFore = s_cNew;

   m_cBack = s_cBack;
   if (m_pIcon != NULL) setPixmap(0, *m_pIcon);

   m_bItalic = m_bStrike = false;
   m_nWeight = QFont::Normal;
   if (s_bFontStyles)
   {
     if (u->OnlineNotify()) m_nWeight = QFont::DemiBold;
     if (u->InvisibleList()) m_bStrike = true;
     if (u->VisibleList()) m_bItalic = true;
   }
   if (u->NewMessages() > 0) m_nWeight = QFont::Bold;

   CUserView *v = (CUserView *)listView();
   for (unsigned short i = 0; i < v->colInfo.size(); i++)
   {
     u->usprintf(sTemp, v->colInfo[i]->m_szFormat);
     setText(i + 1, QString::fromLocal8Bit(sTemp));
   }
}


void CUserViewItem::paintFocus ( QPainter *p, const QColorGroup & cg, const QRect & r )
{
  if (m_nUin != 0) QListViewItem::paintFocus(p, cg, r);
}


//-----CUserViewItem::paintCell-------------------------------------------------
void CUserViewItem::paintCell( QPainter * p, const QColorGroup & cgdefault, int column, int width, int align )
{
  QFont newFont(p->font());
  newFont.setWeight(m_nWeight);
  if (s_bFontStyles)
  {
    newFont.setItalic(m_bItalic);
    newFont.setStrikeOut(m_bStrike);
  }
  p->setFont(newFont);

  QColorGroup cg(cgdefault.foreground(), cgdefault.background(),
                 cgdefault.light(), cgdefault.dark(), cgdefault.mid(),
                 *m_cFore, *m_cBack);
  const QPixmap *pix = NULL;
  if (!listView()->verticalScrollBar()->isVisible() &&
      ((CUserView *)listView())->m_bTransparent)
    pix = ((QWidget *)listView()->parent())->backgroundPixmap();

  if (pix != NULL)
  {
    QPoint pd(p->xForm(QPoint(0,0)).x(), p->xForm(QPoint(0,0)).y());
    QPoint pp(listView()->mapToParent(pd));
    p->drawPixmap(0, 0, *pix, pp.x(), pp.y(), width, height());
  }
  else
  {
    p->fillRect( 0, 0, width, height(), cg.base());
  }

  //QListViewItem::paintCell(p, cg, column, width, align);
  //-----QListViewItem::paintCell------

          if ( !p )
              return;

          QListView *lv = listView();
          int r = lv ? lv->itemMargin() : 1;
          const QPixmap * icon = pixmap( column );

          //p->fillRect( 0, 0, width, height(), cg.base() );

          int marg = lv ? lv->itemMargin() : 1;

          if ( isSelected() &&
               (column==0 || listView()->allColumnsShowFocus()) ) {
                  p->fillRect( r - marg, 0, width - r + marg, height(),
                           cg.brush( QColorGroup::Highlight ) );
                  p->setPen( cg.highlightedText() );
          } else {
              p->setPen( cg.text() );
          }

          if ( icon ) {
              p->drawPixmap( r, (height()-icon->height())/2, *icon );
              r += icon->width() + listView()->itemMargin();
          }

          QString t = text( column );
          if ( !t.isEmpty() ) {
              // should do the ellipsis thing in drawText()
              p->drawText( r, 0, width-marg-r, height(),
                           align | AlignVCenter, t );
          }

  //-----------------------------------


  // Make the dividing line between online and offline users
  if (m_nUin == 0)
  {
    QFont newFont(p->font());
    newFont.setBold(false);
    newFont.setItalic(false);
    newFont.setStrikeOut(false);
    p->setFont(newFont);
    int x1 = 0, x2 = width;
    if (column == 0)
      x1 = 5;
    if (column == listView()->header()->count() - 1)
      x2 = width - 5;
    p->setPen(QPen(QColor(128, 128, 128), 1));
    p->drawLine(x1, height() >> 1, x2, height() >> 1);
    p->setPen(QPen(QColor(255, 255, 255), 1));
    p->drawLine(x1, (height() >> 1) + 1, x2, (height() >> 1) + 1);
    if (column == 1)
    {
      char sz[12] = "Offline";
      if (m_nIndex == -1) strcpy(sz, "Online");
      if (pix != NULL)
      {
        QPoint pd(p->xForm(QPoint(5,0)).x(), p->xForm(QPoint(5,0)).y());
        QPoint pp(listView()->mapToParent(pd));
        p->drawPixmap(5, 0, *pix, pp.x(), pp.y(), p->fontMetrics().width(sz) + 6, height());
      }
      else
      {
        p->fillRect(5, 0, p->fontMetrics().width(sz) + 6, height(), *m_cBack);
      }
      QFont f(p->font());
      f.setPointSize(f.pointSize() - 2);
      p->setFont(f);
      p->setPen(QPen(*s_cGridLines));
      p->drawText(8, 0, width - 8, height(), AlignVCenter, sz);
    }
  }

  // add line to bottom and right side
  if (s_bGridLines && m_nUin != 0)
  {
    p->setPen(*s_cGridLines);
    p->drawLine(0, height() - 1, width - 1, height() - 1);
    p->drawLine(width - 1, 0, width - 1, height() - 1);
  }
}


void CUserView::paintEmptyArea( QPainter *p, const QRect &r )
{
  const QPixmap *pix = NULL;
  if (!verticalScrollBar()->isVisible() && m_bTransparent)
    pix = ((QWidget *)parent())->backgroundPixmap();

  if (pix != NULL)
  {
    QPoint pp(mapToParent(r.topLeft()));
    p->drawPixmap(r.x(), r.y(), *pix, pp.x(), pp.y(), r.width(), r.height());
  }
  else
  {
    QListView::paintEmptyArea(p, r);
  }
}


//-----CUserViewItem::key-------------------------------------------------------
QString CUserViewItem::key (int column, bool ascending) const
{
  if (column == 0)
  {
    if (m_nIndex < 0)  // bar
      return(m_sPrefix);
    char s[8];
    sprintf(s, "%04d", m_nIndex);
    return (m_sPrefix + s);
  }
  else
    return(m_sPrefix + QListViewItem::key(column, ascending));
}



//-----UserList::constructor-----------------------------------------------------------------------
CUserView::CUserView (QPopupMenu *m, QPopupMenu *mg, ColumnInfos _colInfo,
                    bool isHeader, bool _bGridLines, bool _bFontStyles,
                    bool bTransparent,
                    QWidget *parent, const char *name)
   : QListView(parent, name)
{
   mnuUser = m;
   mnuGroup = mg;
   colInfo = _colInfo;
   m_bTransparent = bTransparent;

   addColumn(tr("S"), 20);
   for (unsigned short i = 0; i < colInfo.size(); i++)
   {
     addColumn(colInfo[i]->m_sTitle, colInfo[i]->m_nWidth);
     setColumnAlignment(i + 1, pow(2, colInfo[i]->m_nAlign));
   }

   m_tips = new CUserViewTips(this);

   setAllColumnsShowFocus (true);
   setShowHeader(isHeader);
   setGridLines(_bGridLines);
   setFontStyles(_bFontStyles);
}

// -----------------------------------------------------------------------------
// CUserView destructor

CUserView::~CUserView()
{
  delete m_tips;
}


//-----CUserView::setPixmaps----------------------------------------------------
void CUserView::setPixmaps(QPixmap *_pOnline, QPixmap *_pOffline, QPixmap *_pAway,
                   QPixmap *_pNa, QPixmap *_pOccupied, QPixmap *_pDnd,
                   QPixmap *_pPrivate, QPixmap *_pFFC, QPixmap *_pMessage,
                   QPixmap *_pUrl, QPixmap *_pChat, QPixmap *_pFile)
{
   CUserViewItem::s_pOnline = _pOnline;
   CUserViewItem::s_pOffline = _pOffline;
   CUserViewItem::s_pAway = _pAway;
   CUserViewItem::s_pNa = _pNa;
   CUserViewItem::s_pOccupied = _pOccupied;
   CUserViewItem::s_pDnd = _pDnd;
   CUserViewItem::s_pFFC = _pFFC;
   CUserViewItem::s_pPrivate = _pPrivate;
   CUserViewItem::s_pMessage = _pMessage;
   CUserViewItem::s_pUrl = _pUrl;
   CUserViewItem::s_pChat = _pChat;
   CUserViewItem::s_pFile = _pFile;
}


//-----CUserView::setColors-----------------------------------------------------
void CUserView::setColors(char *_sOnline, char *_sAway, char *_sOffline,
                          char *_sNew, char *_sBack, char *_sGridLines)
{
   if (CUserViewItem::s_cOnline == NULL) CUserViewItem::s_cOnline = new QColor;
   if (CUserViewItem::s_cAway == NULL) CUserViewItem::s_cAway = new QColor;
   if (CUserViewItem::s_cOffline == NULL) CUserViewItem::s_cOffline = new QColor;
   if (CUserViewItem::s_cNew == NULL) CUserViewItem::s_cNew = new QColor;
   if (CUserViewItem::s_cBack == NULL) CUserViewItem::s_cBack = new QColor;
   if (CUserViewItem::s_cGridLines == NULL) CUserViewItem::s_cGridLines = new QColor;

   CUserViewItem::s_cOnline->setNamedColor(_sOnline);
   CUserViewItem::s_cAway->setNamedColor(_sAway);
   CUserViewItem::s_cOffline->setNamedColor(_sOffline);
   CUserViewItem::s_cNew->setNamedColor(_sNew);
   CUserViewItem::s_cBack->setNamedColor(_sBack);
   CUserViewItem::s_cGridLines->setNamedColor(_sGridLines);

   QPalette pal(palette());
   QColorGroup normal(pal.normal());
   QColorGroup newNormal(normal.foreground(), normal.background(), normal.light(), normal.dark(),
                         normal.mid(), normal.text(), *CUserViewItem::s_cBack); //QColor(194, 194, 194));
   setPalette(QPalette(newNormal, pal.disabled(), newNormal));

}


void CUserView::setShowHeader(bool isHeader)
{
   QHeader *h = header();
   if (!isHeader) h->hide();
   else h->show();
}


unsigned long CUserView::SelectedItemUin()
{
   CUserViewItem *i = (CUserViewItem *)currentItem();
   if (i == NULL) return (0);
   return i->ItemUin();
}


//-----CUserList::mousePressEvent---------------------------------------------
void CUserView::viewportMousePressEvent(QMouseEvent *e)
{
   QListView::viewportMousePressEvent(e);
   if (e->button() == MidButton)
   {
      QPoint clickPoint(e->x(), e->y());
      QListViewItem *clickedItem = itemAt(clickPoint);
      if (clickedItem != NULL)
      {
         setSelected(clickedItem, true);
         setCurrentItem(clickedItem);
         doubleClicked(clickedItem);
      }
   }
   else if (e->button() == RightButton)
   {
      QPoint clickPoint(e->x(), e->y());
      QListViewItem *clickedItem = itemAt(clickPoint);
      if (clickedItem != NULL)
      {
         setSelected(clickedItem, true);
         setCurrentItem(clickedItem);
         if (SelectedItemUin() != 0)
         {
           ICQUser *u = gUserManager.FetchUser(SelectedItemUin(), LOCK_R);
           if (u == NULL) return;
           mnuUser->setItemChecked(mnuUser->idAt(MNUxITEM_ONLINExNOTIFY),
                                   u->OnlineNotify());
           mnuUser->setItemChecked(mnuUser->idAt(MNUxITEM_INVISIBLExLIST),
                                   u->InvisibleList());
           mnuUser->setItemChecked(mnuUser->idAt(MNUxITEM_VISIBLExLIST),
                                   u->VisibleList());
           mnuUser->setItemChecked(mnuUser->idAt(MNUxITEM_IGNORExLIST),
                                   u->IgnoreList());
           for (unsigned short i = 0; i < mnuGroup->count(); i++)
              mnuGroup->setItemEnabled(mnuGroup->idAt(i), !u->GetInGroup(GROUPS_USER, i+1));
           gUserManager.DropUser(u);
           mnuUser->popup(mapToGlobal(clickPoint));
         }
      }
   }
}



void CUserView::keyPressEvent(QKeyEvent *e)
{
  QListViewItem *item = NULL;
  if (e->state() & ControlButton || e->state() & AltButton)
  {
    e->ignore();
    QListView::keyPressEvent(e);
    return;
  }

  switch (e->key())
  {
    case Key_Space:
    {
      // Get the menu width (512 when not initialized; used sane value)
      if (currentItem() == NULL) return;
      int nMenuWidth = mnuUser->width();
      if ( 512 == nMenuWidth )
         nMenuWidth = 120;
      // Calculate where to position the menu
      const QListViewItem *pcItem = currentItem();
      QPoint cRelPos( (width() - nMenuWidth)/2,
                     itemPos(pcItem) + pcItem->height() );
      QPoint cPos( mapToGlobal( cRelPos ) );
      mnuUser->popup( cPos );
      return;
    }

    case Key_Home:
      item = firstChild();
      if (item == NULL) return;
      if (((CUserViewItem *)item)->ItemUin() == 0)
        item = item->nextSibling();
      setCurrentItem(item);
      setSelected(item, true);
      ensureItemVisible(item);
      return;

    case Key_End:
      item = firstChild();
      if (item == NULL) return;
      while (item->nextSibling()) item = item->nextSibling();
      setCurrentItem(item);
      setSelected(item, true);
      ensureItemVisible(item);
      return;

    default:
    {
      char ascii = tolower(e->ascii());
      if (!isalnum(ascii))
      {
        QListView::keyPressEvent(e);
        return;
      }

      QListViewItem *item = currentItem() != NULL ? currentItem()->nextSibling() : firstChild();

      while (item != NULL)
      {
        if (item->text(1).at(0).lower().latin1() == ascii)
        {
          setSelected(item, true);
          setCurrentItem(item);
          ensureItemVisible(item);
          return;
        }
        item = item->nextSibling();
      }

      // Check the first elements if we didn't find anything yet
      if (currentItem() != NULL)
      {
        item = firstChild();
        while (item != NULL && item != currentItem())
        {
          if (item->text(1).at(0).lower().latin1() == ascii)
          {
            setSelected(item, true);
            setCurrentItem(item);
            ensureItemVisible(item);
            return;
          }
          item = item->nextSibling();
        }
      }

      // If we are here we didn't find any names
      QListView::keyPressEvent(e);
    }
  }

}

void CUserView::maxLastColumn()
{
  unsigned short totalWidth = 0;
  unsigned short nNumCols = header()->count();
  for (unsigned short i = 0; i < nNumCols - 1; i++)
    totalWidth += columnWidth(i);
//  QScrollBar *s = verticalScrollBar();
//  if (s != NULL) totalWidth += s->width();
  int newWidth = width() - totalWidth - 2;
  if (newWidth < colInfo[nNumCols - 2]->m_nWidth)
  {
    setHScrollBarMode(Auto);
    setColumnWidth(nNumCols - 1, colInfo[nNumCols - 2]->m_nWidth);
  }
  else
  {
    setHScrollBarMode(AlwaysOff);
    setColumnWidth(nNumCols - 1, newWidth);
  }
}

#if 0
void CUserView::dragEnterEvent(QDragEnterEvent*)
{
  debug("received dragEnterEvent");
}

void CUserView::dragMoveEvent(QDragMoveEvent*)
{
  debug("received dragMoveEvent");
}

void CUserView::dropEvent(QDropEvent*)
{
  debug("received dropEvent");
}
#endif

/*
#ifdef USE_KDE
void CUserView::slot_dropAction(KDNDDropZone *zone)
{
  QListViewItem *item;
  QPoint pos;
  QStrList urls;
  KURL url;

  pos = viewport()->mapFromGlobal(QPoint(zone->getMouseX(), zone->getMouseY()));
  if ((item=itemAt(pos))!=NULL)
  {
    setSelected(item, true);
    setCurrentItem(item);
    doubleClicked(item);
    urls = zone->getURLList();
    url = urls.getFirst();
    if (url.isLocalFile())
      emit signal_dropedFile(url.path());
    else
      emit signal_dropedURL(url.url());
  }
}


void CUserView::slot_dropEnter(KDNDDropZone *zone)
{
  QListViewItem *item = itemAt(viewport()->mapFromGlobal(QPoint(zone->getMouseX(), zone->getMouseY())));
  if (item != NULL)
    setCurrentItem(item);
  else
    setCurrentItem(NULL);
}


void CUserView::slot_dropLeave(KDNDDropZone *)
{
  setCurrentItem(NULL);
}

#else

void CUserView::slot_dropAction(KDNDDropZone *) {}
void CUserView::slot_dropEnter(KDNDDropZone *) {}
void CUserView::slot_dropLeave(KDNDDropZone *) {}

#endif
*/

//=====CUserViewTips===============================================================================

CUserViewTips::CUserViewTips(CUserView* parent)
  : QToolTip(parent)
{
  // nothing to do
}

void CUserViewTips::maybeTip(const QPoint& c)
{
  QListView* w = (QListView*) parentWidget();
  CUserViewItem* item = (CUserViewItem*) w->itemAt(c);

  if(item && item->m_nUin) {
    char s[32];

    ICQUser::StatusToStatusStr(item->m_status, false, s);
    tip(w->itemRect(item), QString(s));
  }
}


#include "moc/moc_userbox.h"
