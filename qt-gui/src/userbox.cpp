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
#include <qdragobject.h>

#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <math.h>
#include "userbox.h"
#include "gui-defines.h"

#include "licq_user.h"

#define FLASH_TIME 800

bool    CUserViewItem::s_bGridLines = false,
        CUserViewItem::s_bFontStyles = true,
        CUserViewItem::s_bSortByStatus = true;
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
        *CUserViewItem::s_pFFC = NULL,
        *CUserViewItem::s_pNone = NULL;
QColor  *CUserViewItem::s_cOnline = NULL,
        *CUserViewItem::s_cAway = NULL,
        *CUserViewItem::s_cOffline = NULL,
        *CUserViewItem::s_cNew = NULL,
        *CUserViewItem::s_cBack = NULL,
        *CUserViewItem::s_cGridLines = NULL;
QTimer *CUserViewItem::s_tFlash = NULL;
FlashType CUserViewItem::s_nFlash = FLASH_NONE;
int CUserViewItem::s_nFlashCounter = 0;


//-----CUserViewItem::constructor-----------------------------------------------
CUserViewItem::CUserViewItem(ICQUser *_cUser, QListView *parent)
   : QListViewItem(parent)
{
  m_nUin = _cUser->Uin();
  m_bUrgent = false;
  setSelectable(m_nUin != 0);
  setGraphics(_cUser);
}


CUserViewItem::~CUserViewItem()
{
  CUserView *v = (CUserView *)listView();

  if (v == NULL || m_nUin == 0) return;

  if (m_nStatus == ICQ_STATUS_OFFLINE)
    v->numOffline--;
  else
    v->numOnline--;

  if (v->numOffline == 0 && v->barOffline != NULL)
  {
    delete v->barOffline;
    v->barOffline = NULL;
  }
  if (v->numOnline == 0 && v->barOnline != NULL)
  {
    delete v->barOnline;
    v->barOnline = NULL;
  }
}


CUserViewItem::CUserViewItem(BarType barType, QListView *parent)
   : QListViewItem(parent)
{
  m_nUin = 0;
  m_pIcon = NULL;
  m_cBack = s_cBack;
  m_cFore = s_cOnline;
  m_bItalic = m_bStrike = false;
  m_nWeight = QFont::Normal;
  m_bUrgent = false;
  setSelectable(false);
  setHeight(10);
  m_sSortKey = "";
  if (barType == BAR_ONLINE)
    m_sPrefix = "0";
  else
    m_sPrefix = "2";
}


//-----CUserViewItem::setGraphics-----------------------------------------------
void CUserViewItem::setGraphics(ICQUser *u)
{
   static char sTemp[128];
   CUserView *v = (CUserView *)listView();
   m_nStatus = u->Status();

   // Create any necessary bars
   if (u->StatusOffline())
   {
     v->numOffline++;
     if (v->barOffline == NULL && v->ShowBars() )
       v->barOffline = new CUserViewItem(BAR_OFFLINE, listView());
   }
   else
   {
     v->numOnline++;
     if (v->barOnline == NULL && v->ShowBars() )
       v->barOnline = new CUserViewItem(BAR_ONLINE, listView());
   }

   m_sPrefix = "1";
   switch (m_nStatus)
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
   m_pIconStatus = m_pIcon;

   if (u->NewMessages() > 0)
   {
     m_pIcon = NULL;
     for (unsigned short i = 0; i < u->NewMessages(); i++)
     {
       switch(u->EventPeek(i)->SubCommand())
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
       if (u->EventPeek(i)->IsUrgent()) m_bUrgent = true;
     }
   }

   if (u->NewMessages() > 0 && s_nFlash == FLASH_ALL || m_bUrgent && s_nFlash == FLASH_URGENT)
   {
     connect(s_tFlash, SIGNAL(timeout()), SLOT(slot_flash()));
     //m_tFlash->start(FLASH_TIME);
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

   for (unsigned short i = 0; i < v->colInfo.size(); i++)
   {
     u->usprintf(sTemp, v->colInfo[i]->m_szFormat);
     setText(i + 1, QString::fromLocal8Bit(sTemp));
   }

   // Set the user tag
   if (s_bSortByStatus)
     // sort STATUS_FFF like STATUS_ONLINE.
     m_sSortKey.sprintf("%04x%016lx",
       (m_nStatus == ICQ_STATUS_FREEFORCHAT) ? ICQ_STATUS_ONLINE : m_nStatus,
                        ULONG_MAX - u->Touched());
   else
     m_sSortKey.sprintf("%016lx", ULONG_MAX - u->Touched());
}


void CUserViewItem::slot_flash()
{
  if (s_nFlashCounter & 0x0001) // hide
  {
    setPixmap(0, *m_pIconStatus);
  }
  else  // show
  {
    if (m_pIcon != NULL) setPixmap(0, *m_pIcon);
  }
}

void CUserView::slot_flash()
{
  CUserViewItem::s_nFlashCounter++;
}


// ---------------------------------------------------------------------------

void CUserViewItem::paintCell( QPainter * p, const QColorGroup & cgdefault, int column, int width, int align )
{
  //QListViewItem::paintCell(p, cgdefault, column, width, align);

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

  /*if (!listView()->verticalScrollBar()->isVisible() &&
      ((CUserView *)listView())->m_bTransparent)*/
  if (listView()->contentsHeight() < listView()->viewport()->height() &&
      ((CUserView *)listView())->m_bTransparent)
    pix = listView()->parentWidget()->backgroundPixmap();

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
      QString sz = CUserView::tr("Offline");
      if (m_sPrefix == "0")
        sz = CUserView::tr("Online");

      if (pix) {
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
  if (contentsHeight() < viewport()->height() && m_bTransparent)
    pix = parentWidget()->backgroundPixmap();

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
    return (m_sPrefix + m_sSortKey);
  else
    return(m_sPrefix + QListViewItem::key(column, ascending));
}



//-----UserList::constructor-----------------------------------------------------------------------
CUserView::CUserView (QPopupMenu *m, QPopupMenu *mg, QPopupMenu *ma, ColumnInfos _colInfo,
                    bool isHeader, bool _bGridLines, bool _bFontStyles,
                    bool bTransparent, bool bShowBars, bool bSortByStatus,
                    FlashType nFlash,
                    QWidget *parent, const char *name)
   : QListView(parent, name)
{
   mnuUser = m;
   mnuGroup = mg;
   mnuAwayModes = ma;
   colInfo = _colInfo;
   m_bTransparent = bTransparent;
   m_bShowBars = bShowBars;
   barOnline = barOffline = NULL;
   numOnline = numOffline = 0;

   addColumn(tr("S"), 20);
   for (unsigned short i = 0; i < colInfo.size(); i++)
   {
     addColumn(colInfo[i]->m_sTitle, colInfo[i]->m_nWidth);
     setColumnAlignment(i + 1, pow(2, colInfo[i]->m_nAlign));
   }

   m_tips = new CUserViewTips(this);

   setAllColumnsShowFocus (true);
   setSorting(0);
   setShowHeader(isHeader);
   setGridLines(_bGridLines);
   setFontStyles(_bFontStyles);
   setSortByStatus(bSortByStatus);
   //setFlashUrgent(bFlashUrgent);

   // Flash stuff
   CUserViewItem::s_tFlash = new QTimer(this);
   CUserViewItem::s_nFlash = nFlash;
   connect(CUserViewItem::s_tFlash, SIGNAL(timeout()), SLOT(slot_flash()));
   CUserViewItem::s_tFlash->start(FLASH_TIME);

   //CUserViewItem::s_pNone = new QPixmap;

   //setAutoMask(true);
}

// -----------------------------------------------------------------------------
// CUserView destructor

CUserView::~CUserView()
{
  barOnline = barOffline = NULL;
  delete m_tips;
}

void CUserView::clear()
{
  barOnline = barOffline = NULL;
  QListView::clear();
  numOffline = numOnline = 0;
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
   CUserViewItem::s_pUrl = _pUrl->isNull() ? _pMessage : _pUrl;
   CUserViewItem::s_pChat = _pChat->isNull() ? _pMessage :_pChat;
   CUserViewItem::s_pFile = _pFile->isNull() ? _pMessage : _pFile;;
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
  isHeader ? header()->show() : header()->hide();
}

void CUserView::setShowBars(bool s)
{
}

void CUserView::setSortByStatus(bool s)
{
  CUserViewItem::s_bSortByStatus = s;
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
  if (e->button() == LeftButton)
  {
    mousePressPos = e->pos();
  }
  else if (e->button() == MidButton)
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
    QListViewItem *clickedItem = itemAt(e->pos());
    if (clickedItem != NULL)
    {
      setSelected(clickedItem, true);
      setCurrentItem(clickedItem);
      if (SelectedItemUin() != 0)
      {
        ICQUser *u = gUserManager.FetchUser(SelectedItemUin(), LOCK_R);
        if (u == NULL) return;
        mnuUser->setItemChecked(mnuUserOnlineNotify, u->OnlineNotify());
        mnuUser->setItemChecked(mnuUserInvisibleList, u->InvisibleList());
        mnuUser->setItemChecked(mnuUserVisibleList, u->VisibleList());
        mnuUser->setItemChecked(mnuUserIgnoreList, u->IgnoreList());
        // AcceptIn[Away] mode checked/unchecked stuff -- Andypoo (andypoo@ihug.com.au)
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(0), u->AcceptInAway());
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(1), u->AcceptInNA());
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(2), u->AcceptInOccupied());
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(3), u->AcceptInDND());
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(5), u->StatusToUser() == ICQ_STATUS_ONLINE);
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(6), u->StatusToUser() == ICQ_STATUS_AWAY);
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(7), u->StatusToUser() == ICQ_STATUS_NA);
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(8), u->StatusToUser() == ICQ_STATUS_OCCUPIED);
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(9), u->StatusToUser() == ICQ_STATUS_DND);
        mnuAwayModes->setItemChecked(mnuAwayModes->idAt(11), u->CustomAutoResponse()[0] != '\0');

        for (unsigned short i = 0; i < mnuGroup->count(); i++)
          mnuGroup->setItemEnabled(mnuGroup->idAt(i), !u->GetInGroup(GROUPS_USER, i+1));
        gUserManager.DropUser(u);
        mnuUser->popup(mapToGlobal(e->pos()) + QPoint(4,-5), 1);
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


// -----------------------------------------------------------------------------

void  CUserView::viewportMouseReleaseEvent(QMouseEvent* me)
{
  mousePressPos.setX(0);
  mousePressPos.setY(0);
}


// -----------------------------------------------------------------------------

void CUserView::viewportMouseMoveEvent(QMouseEvent * me)
{
  CUserViewItem *i;
  QListView::viewportMouseMoveEvent(me);

  if (me->state() == LeftButton && (i = (CUserViewItem *)currentItem())
      && !mousePressPos.isNull() && i->ItemUin() &&
      (QPoint(me->pos() - mousePressPos).manhattanLength() > 8))
  {
    QTextDrag *d = new QTextDrag(QString::number(i->ItemUin()), this);
    d->dragCopy();
  }
}


//=====CUserViewTips===============================================================================

CUserViewTips::CUserViewTips(CUserView* parent)
  : QToolTip(parent)
{
  // nothing to do
}

void CUserViewTips::maybeTip(const QPoint& c)
{
  QPoint p(c);
  QListView* w = (QListView*) parentWidget();
  if(w->header()->isVisible())
    p.setY(p.y()-w->header()->height());

  CUserViewItem* item = (CUserViewItem*) w->itemAt(p);

  if(item && item->m_nUin)
  {
    QRect r(w->itemRect(item));
    if(w->header()->isVisible())
      r.moveBy(0, w->header()->height());
    tip(r, QString(ICQUser::StatusToStatusStr(item->m_nStatus, false)));
  }
}

#include "userbox.moc"
