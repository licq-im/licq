// -*- c-basic-offset: 2 -*-
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
#include <ctype.h>

#include <qpainter.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qkeycode.h>
#include <qscrollbar.h>
#include <qdragobject.h>
#include <qstylesheet.h>

#include "skin.h"
#include "mainwin.h"
#include "userbox.h"
#include "gui-defines.h"
#include "mainwin.h"
#include "usereventdlg.h"

#include "licq_user.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xpm/itemCollapsed.xpm"
#include "xpm/itemExpanded.xpm"
#include "xpm/iconCustomAR.xpm"
#include "xpm/iconBirthday.xpm"

#undef Status

#define FLASH_TIME 800

QColor  *CUserViewItem::s_cOnline = NULL,
        *CUserViewItem::s_cAway = NULL,
        *CUserViewItem::s_cOffline = NULL,
        *CUserViewItem::s_cNew = NULL,
        *CUserViewItem::s_cBack = NULL,
        *CUserViewItem::s_cGridLines = NULL;

//-----CUserViewItem::constructor-----------------------------------------------
CUserViewItem::CUserViewItem(ICQUser *_cUser, QListView *parent)
  : QListViewItem(parent)
{
  if(listView()->parent() == NULL)
    listView()->setCaption(CUserView::tr("%1 Floaty (%2)")
                           .arg(_cUser->GetAlias()).arg(_cUser->Uin()));

  m_nUin = _cUser->Uin();
  m_bUrgent = false;
  m_nOnlCount = 0;
  setGraphics(_cUser);
}


CUserViewItem::CUserViewItem (ICQUser *_cUser, CUserViewItem* item)
  : QListViewItem(item),
    m_sGroupName()
{
  m_nGroupId = (unsigned short)(-1);
  m_nUin = _cUser->Uin();
  m_bUrgent = false;
  m_nOnlCount = 0;
  m_nStatus = ICQ_STATUS_OFFLINE;
  setGraphics(_cUser);
}

CUserViewItem::CUserViewItem(unsigned short Id, const char* name, QListView* lv)
  : QListViewItem(lv),
    m_nGroupId(Id),
    m_sGroupName(name)
{
  m_nUin = 0;
  m_pIcon = NULL;
  m_cBack = s_cBack;
  m_cFore = s_cGridLines;
  m_bItalic = m_bStrike = false;
  m_nWeight = QFont::Bold;
  m_bUrgent = false;
  m_nOnlCount = 0;
  // Other users group is sorted at the end
  m_sSortKey = m_nGroupId ? QString::number((int)m_nGroupId) : QString("9999999999");
  m_sPrefix = "1";
  setPixmap(0, *listView()->pixCollapsed);
  setText(1, QString::fromLocal8Bit(name));

}

CUserViewItem::CUserViewItem(BarType barType, QListView *parent)
  : QListViewItem(parent),
    m_sGroupName()
{
  m_nGroupId = (unsigned short)(-1);
  m_nUin = 0;
  m_nOnlCount = 0;
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

CUserViewItem::~CUserViewItem()
{
  CUserView *v = (CUserView *)listView();

  if (v == NULL || m_nUin == 0) return;

  if (m_nStatus == ICQ_STATUS_OFFLINE)
    v->numOffline--;
  else
  {
    if (parent())
    {
      CUserViewItem *i = static_cast<CUserViewItem*>(parent());
      i->m_nOnlCount--;
      if (i->m_nOnlCount)
        i->setText(1, QString::fromLocal8Bit(i->m_sGroupName) + QString(" (") + QString::number(i->m_nOnlCount)
                   + QString(")"));
      else
        i->setText(1,  QString::fromLocal8Bit(i->m_sGroupName));
    }
    v->numOnline--;
  }

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

//-----CUserViewItem::setGraphics-----------------------------------------------
void CUserViewItem::setGraphics(ICQUser *u)
{
   static char sTemp[128];
   CUserView *v = (CUserView *)listView();

   if(parent()) {
     CUserViewItem* i = static_cast<CUserViewItem*>(parent());
     if(u->StatusOffline() && m_nStatus != ICQ_STATUS_OFFLINE)
       i->m_nOnlCount--;
     if(m_nStatus == ICQ_STATUS_OFFLINE && !u->StatusOffline())
       i->m_nOnlCount++;

     if(i->m_nOnlCount)
       i->setText(1, QString::fromLocal8Bit(i->m_sGroupName) + QString(" (") + QString::number(i->m_nOnlCount)
                  + QString(")"));
     else
       i->setText(1,  QString::fromLocal8Bit(i->m_sGroupName));
     }

   m_nStatus = u->Status();
   m_nStatusFull = u->StatusFull();
   m_bStatusInvisible = u->StatusInvisible();

   // Create any necessary bars
   if (u->StatusOffline())
   {
     v->numOffline++;
     if (v->barOffline == NULL && v->parent()
         && gMainWindow->m_bShowDividers
         && !(gMainWindow->m_bThreadView && gMainWindow->m_nGroupType == GROUPS_USER
              && gMainWindow->m_nCurrentGroup == 0))
       v->barOffline = new CUserViewItem(BAR_OFFLINE, listView());
   }
   else
   {
     v->numOnline++;
     if (v->barOnline == NULL && v->parent()
         && gMainWindow->m_bShowDividers &&
         !(gMainWindow->m_bThreadView && gMainWindow->m_nGroupType == GROUPS_USER
           && gMainWindow->m_nCurrentGroup == 0))
       v->barOnline = new CUserViewItem(BAR_ONLINE, listView());
   }

   m_sPrefix = "1";
   m_pIcon = &CMainWindow::iconForStatus(u->StatusFull());

   switch (m_nStatus)
   {
   case ICQ_STATUS_AWAY:
   case ICQ_STATUS_OCCUPIED:
   case ICQ_STATUS_DND:
   case ICQ_STATUS_NA:
     m_cFore = s_cAway;
     break;
   case ICQ_STATUS_OFFLINE:
     m_cFore = s_cOffline;
     m_sPrefix = "3";
     break;
   case ICQ_STATUS_ONLINE:
   case ICQ_STATUS_FREEFORCHAT:
   default:
     m_cFore = s_cOnline;
   }

   if (u->StatusInvisible())
      m_cFore = s_cAway;

   m_pIconStatus = m_pIcon;

   if (u->NewMessages() > 0)
   {
     m_pIcon = NULL;
     unsigned short SubCommand = 0;

     for (unsigned short i = 0; i < u->NewMessages(); i++)
     {
       switch(u->EventPeek(i)->SubCommand())
       {
         case ICQ_CMDxSUB_FILE:
           SubCommand = ICQ_CMDxSUB_FILE;
           break;
         case ICQ_CMDxSUB_CHAT:
           if (SubCommand != ICQ_CMDxSUB_FILE) SubCommand = ICQ_CMDxSUB_CHAT;
           break;
         case ICQ_CMDxSUB_URL:
           if (SubCommand != ICQ_CMDxSUB_FILE && SubCommand != ICQ_CMDxSUB_CHAT)
             SubCommand = ICQ_CMDxSUB_URL;
           break;
         case ICQ_CMDxSUB_CONTACTxLIST:
           if(SubCommand != ICQ_CMDxSUB_FILE && SubCommand != ICQ_CMDxSUB_CHAT
              && SubCommand != ICQ_CMDxSUB_URL)
             SubCommand = ICQ_CMDxSUB_CONTACTxLIST;
         case ICQ_CMDxSUB_MSG:
         default:
           if (SubCommand == 0) SubCommand = ICQ_CMDxSUB_MSG;
           break;
       }
       if (u->EventPeek(i)->IsUrgent()) m_bUrgent = true;
     }
     if(SubCommand)
       m_pIcon = &CMainWindow::iconForEvent(SubCommand);
   }

   if (v->msgTimerId == 0 &&
       ((u->NewMessages() > 0 && gMainWindow->m_nFlash == FLASH_ALL) ||
       (m_bUrgent && gMainWindow->m_nFlash == FLASH_URGENT)))
     v->msgTimerId = v->startTimer(FLASH_TIME);

   if (u->NewUser() &&
       !(gMainWindow->m_nGroupType == GROUPS_SYSTEM && gMainWindow->m_nCurrentGroup == GROUP_NEW_USERS) )
      m_cFore = s_cNew;

   m_cBack = s_cBack;
   if (m_pIcon != NULL) setPixmap(0, *m_pIcon);

   m_bItalic = m_bStrike = false;
   m_nWeight = QFont::Normal;
   if (gMainWindow->m_bFontStyles)
   {
     if (u->OnlineNotify()) m_nWeight = QFont::DemiBold;
     if (u->InvisibleList()) m_bStrike = true;
     if (u->VisibleList()) m_bItalic = true;
   }
   if (u->NewMessages() > 0) m_nWeight = QFont::Bold;

   for (unsigned short i = 0; i < gMainWindow->colInfo.size(); i++)
   {
     u->usprintf(sTemp, gMainWindow->colInfo[i]->m_szFormat);
     setText(i + 1, QString::fromLocal8Bit(sTemp));
   }

   // Set the user tag
   if (gMainWindow->m_bSortByStatus)
     // sort STATUS_FFF like STATUS_ONLINE.
     m_sSortKey.sprintf("%04x%016lx",
       (m_nStatus == ICQ_STATUS_FREEFORCHAT) ? ICQ_STATUS_ONLINE : m_nStatus,
                        ULONG_MAX - u->Touched());
   else
     m_sSortKey.sprintf("%016lx", ULONG_MAX - u->Touched());
}


// ---------------------------------------------------------------------------

void CUserViewItem::paintCell( QPainter * p, const QColorGroup & cgdefault, int column, int width, int align )
{
  QFont newFont(p->font());
  newFont.setWeight(m_nWeight);
  if (gMainWindow->m_bFontStyles)
  {
    newFont.setItalic(m_bItalic);
    newFont.setStrikeOut(m_bStrike);
  }
  p->setFont(newFont);

  bool onlBlink = (listView()->onlTimerId && listView()->onlUin &&
                   listView()->onlUin == m_nUin && listView()->onlCounter & 1);

  QColorGroup cg(cgdefault.foreground(), cgdefault.background(),
    cgdefault.light(), cgdefault.dark(), cgdefault.mid(),
    onlBlink ? *s_cOffline : *m_cFore, *m_cBack);

  const QPixmap *pix = NULL;

  if ((listView()->contentsHeight() < listView()->viewport()->height() ||
       listView()->vScrollBarMode() == QListView::AlwaysOff) &&
      listView()->parent() && gMainWindow->skin->frame.transparent )
    pix = listView()->parentWidget()->backgroundPixmap();

  if (pix != NULL)
  {
    QPoint pd(p->xForm(QPoint(0,0)).x(), p->xForm(QPoint(0,0)).y());
    QPoint pp(listView()->mapToParent(pd));
    p->drawPixmap(0, 0, *pix, pp.x(), pp.y(), width, height());
  }
  else
    p->fillRect( 0, 0, width, height(), cg.base());

  if (m_nUin != 0 || isGroupItem())
  {
    cg.setBrush(QColorGroup::Base, QBrush(NoBrush));
    // If this is a floaty then don't draw the highlight box
    if (listView()->parent() == NULL)
    {
      cg.setBrush(QColorGroup::Highlight, QBrush(NoBrush));
      cg.setColor(QColorGroup::HighlightedText, cg.text());
    }

    if (isGroupItem())
    {
      QFont f(p->font());
      f.setPointSize(f.pointSize() - 2);
      p->setFont(f);
    }

    QListViewItem::paintCell(p, cg, column, width, align);

    if (isGroupItem())
    {
      if (column == 1)
      {
        listView()->style().drawSeparator(p,
           p->fontMetrics().width(text(column)) + 4, height() >> 1,
           width - 1, height() >> 1, cg);
      }
      else if (column > 1)
      {
        listView()->style().drawSeparator(p, 0, height() >> 1, width - 1,
           height() >> 1, cg);
      }
    }
    else if (column == 1) {
      int w = p->fontMetrics().width(text(1)) + 3;

      if(width - w > 8 && (m_nStatusFull & ICQ_STATUS_FxBIRTHDAY)) {
        p->drawPixmap(w, 0, *listView()->pixBirthday);
        w += 11;
      }
    }
  }
  else
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

      if (pix)
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
  if (listView()->parent() && gMainWindow->m_bGridLines && m_nUin != 0)
  {
    p->setPen(*s_cGridLines);
    p->drawLine(0, height() - 1, width - 1, height() - 1);
    p->drawLine(width - 1, 0, width - 1, height() - 1);
  }

  if(listView()->carTimerId > 0 && listView()->carUin == m_nUin)
    drawCAROverlay(p);
}


void CUserView::paintEmptyArea( QPainter *p, const QRect &r )
{
  const QPixmap *pix = NULL;
  if ((contentsHeight() < viewport()->height() || vScrollBarMode() == AlwaysOff)
      && parent() && gMainWindow->skin->frame.transparent)
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

void CUserViewItem::drawCAROverlay(QPainter* p)
{
  QRect r(listView()->itemRect(this));
  if(!r.isValid())
    // is not on screen
    return;

  p->setBackgroundMode(OpaqueMode);
  p->setBackgroundColor((listView()->carCounter & 1) ? black : white);
  p->setPen(QPen((listView()->carCounter & 1) ? white : black, 1, SolidLine));
  p->drawRect(r);
}

void CUserView::timerEvent(QTimerEvent* e)
{
  bool doGroupView = gMainWindow->m_bThreadView &&
    gMainWindow->m_nGroupType == GROUPS_USER &&
    gMainWindow->m_nCurrentGroup == 0;

  if(e->timerId() == carTimerId)
  {
    QListViewItemIterator it(this);

    if(carCounter > 0 && carUin > 0) {
      QPainter p(viewport());
      for(; it.current(); ++it)
      {
        CUserViewItem* item = static_cast<CUserViewItem*>(it.current());

        if(item->ItemUin() == carUin)
        {
          if(carCounter == 1)
            item->repaint();
          else
            item->drawCAROverlay(&p);
          if(!doGroupView)  break;
        }
      }
    }

    if(--carCounter == 0) {
      carUin = 0;
      killTimer(carTimerId);
      carTimerId = 0;
    }
  }
  else if(e->timerId() == onlTimerId)
  {
    QListViewItemIterator it(this);
    bool found = false;
    if(onlUin > 0)
    {
      for(; it.current(); ++it)
      {
        CUserViewItem* item = static_cast<CUserViewItem*>(it.current());
        if(item->ItemUin() == onlUin)
        {
          found = true;
          item->repaint();
          if(!doGroupView)  break;
        }
      }
    }

    if(!found || (--onlCounter == 0)) {
      onlUin = 0;
      killTimer(onlTimerId);
      onlTimerId = 0;
    }
  }
  else
  {
    if(m_nFlashCounter++ & 1) // hide event icon
    {
      QListViewItemIterator it(this);
      for(; it.current(); ++it)
      {
        CUserViewItem* item = static_cast<CUserViewItem*>(it.current());
        if(item->ItemUin())  item->setPixmap(0, *item->m_pIconStatus);
      }
    }
    else {
      // show
      bool foundIcon = false;
      QListViewItemIterator it(this);

      for(; it.current(); ++it)
      {
        CUserViewItem* item = static_cast<CUserViewItem*>(it.current());
        if(item->ItemUin() && item->m_pIcon != NULL &&
           item->m_pIcon != item->m_pIconStatus)
        {
          foundIcon = true;
          item->setPixmap(0, *item->m_pIcon);
        }
      }
      // no pending messages any more, kill timer
      if(!foundIcon) {
        killTimer(msgTimerId);
        msgTimerId = 0;
      }
    }
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

UserFloatyList* CUserView::floaties = 0;

//-----UserList::constructor-----------------------------------------------------------------------
CUserView::CUserView(QPopupMenu *m, QWidget *parent, const char *name)
  : QListView(parent, name)
{
  m_nFlashCounter = carCounter = onlCounter = 0;
  msgTimerId = carTimerId = onlTimerId = 0;
  mnuUser = m;
  barOnline = barOffline = NULL;
  numOnline = numOffline = 0;

  addColumn(tr("S"), 20);

  for (unsigned short i = 0; i < gMainWindow->colInfo.size(); i++)
  {
    addColumn(gMainWindow->colInfo[i]->m_sTitle, gMainWindow->colInfo[i]->m_nWidth);
    setColumnAlignment(i + 1, 1 << gMainWindow->colInfo[i]->m_nAlign);
  }

  m_tips = new CUserViewTips(this);

  viewport()->setAcceptDrops(true);

#if QT_VERSION >= 210
  setShowSortIndicator(true);
#endif
//  setRootIsDecorated(gMainWindow->m_bThreadView);
  setAllColumnsShowFocus(true);
  setTreeStepSize(0);
  setSorting(0);
  setVScrollBarMode(gMainWindow->m_bScrollBar ? Auto : AlwaysOff);

  pixCollapsed = new QPixmap(itemCollapsed_xpm);
  pixExpanded = new QPixmap(itemExpanded_xpm);
  pixBirthday = new QPixmap(iconBirthday_xpm);
  pixCustomAR = new QPixmap(iconCustomAR_xpm);

  if (parent != NULL)
  {
    setShowHeader(gMainWindow->m_bShowHeader);
    setFrameStyle(gMainWindow->skin->frame.frameStyle);
    connect(this, SIGNAL(expanded(QListViewItem*)), this, SLOT(itemExpanded(QListViewItem*)));
    connect(this, SIGNAL(collapsed(QListViewItem*)), this, SLOT(itemCollapsed(QListViewItem*)));
  }
  else
  {
    /*XSetWindowAttributes wsa;
    wsa.override_redirect = TRUE;
    XChangeWindowAttributes( x11Display(), winId(), CWOverrideRedirect, &wsa );*/
    setWFlags(getWFlags() | WDestructiveClose);
    setShowHeader(false);
    setFrameStyle(33);
    XClassHint classhint;
    classhint.res_name = "licq";
    classhint.res_class = "Floaty";
    XSetClassHint(x11Display(), winId(), &classhint);
    XWMHints *hints = XGetWMHints(x11Display(), winId());
    hints->window_group = winId();
    hints->flags = WindowGroupHint;
    XSetWMHints(x11Display(), winId(), hints);
    XFree( hints );
    floaties->resize(floaties->size()+1);
    floaties->insert(floaties->size()-1, this);
  }
}

// -----------------------------------------------------------------------------
// CUserView destructor

CUserView::~CUserView()
{
  barOnline = barOffline = NULL;
  delete m_tips;
  if (parent() == NULL)
  {
    unsigned int i = 0;
    for (; i<floaties->size(); i++)
    {
      if (floaties->at(i) == this) {
        floaties->take(i);
        break;
      }
    }
    while(i+1 < floaties->size()) {
        floaties->insert(i, floaties->at(i+1));
        i++;
    }
    if(floaties->size())
        floaties->resize(floaties->size()-1);
  }
}


CUserView *CUserView::FindFloaty(unsigned long nUin)
{
  unsigned int i = 0;
  for (; i<floaties->size(); i++)
  {
    if (static_cast<CUserViewItem*>(floaties->at(i)->firstChild())->ItemUin()== nUin)
        break;
  }
  if(i<floaties->size()) return floaties->at(i);

  return NULL;
}


void CUserView::clear()
{
  barOnline = barOffline = NULL;
  QListView::clear();
  numOffline = numOnline = 0;
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
   pal.setColor(QColorGroup::Base, *CUserViewItem::s_cBack);
   setPalette(pal);
}


void CUserView::setShowHeader(bool isHeader)
{
  isHeader ? header()->show() : header()->hide();
}


unsigned long CUserView::MainWindowSelectedItemUin()
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
    CUserViewItem *clickedItem = static_cast<CUserViewItem*>(itemAt(e->pos()));
    if (clickedItem != NULL && e->pos().x() < header()->sectionSize(0) && clickedItem->isGroupItem())
      clickedItem->setOpen(!clickedItem->isOpen());
  }
  else if (e->button() == MidButton)
  {
    QPoint clickPoint(e->x(), e->y());
    QListViewItem *clickedItem = itemAt(clickPoint);
    if (clickedItem != NULL)
    {
      setSelected(clickedItem, true);
      setCurrentItem(clickedItem);
      emit doubleClicked(clickedItem);
    }
  }
  else if (e->button() == RightButton)
  {
    CUserViewItem *clickedItem = (CUserViewItem *)itemAt(e->pos());
    if (clickedItem != NULL)
    {
      setSelected(clickedItem, true);
      setCurrentItem(clickedItem);
      if (clickedItem->ItemUin())
      {
        gMainWindow->SetUserMenuUin(clickedItem->ItemUin());
        mnuUser->popup(mapToGlobal(e->pos()) + QPoint(4,-5), 1);
      }
    }
  }
}

void CUserView::viewportDragEnterEvent(QDragEnterEvent* e)
{
  e->accept(QTextDrag::canDecode(e) || QUriDrag::canDecode(e));
}


void CUserView::viewportDropEvent(QDropEvent* e)
{
  CUserViewItem* it = static_cast<CUserViewItem*>(itemAt(e->pos()));

  if(it)
  {
    if(it->ItemUin())
    {
      QString text;
      QStrList lst;
      if(QUriDrag::decode(e, lst))
      {
        if(!(text = QUriDrag::uriToLocalFile(lst.first())).isEmpty()) {
          UserSendFileEvent* e = static_cast<UserSendFileEvent*>
            (gMainWindow->callFunction(mnuUserSendFile, it->ItemUin()));
          e->setFile(text.mid(5), QString::null);
          e->show();
        }
        else {
          UserSendUrlEvent* e = static_cast<UserSendUrlEvent*>
            (gMainWindow->callFunction(mnuUserSendUrl, it->ItemUin()));
          e->setUrl(text, QString::null);
          e->show();
        }
      }
      else if(QTextDrag::decode(e, text)) {
        unsigned long Uin = text.toULong();

        if(Uin >= 10000) {
          if(Uin == it->ItemUin()) return;
          UserSendContactEvent* e = static_cast<UserSendContactEvent*>
            (gMainWindow->callFunction(mnuUserSendContact, it->ItemUin()));
          ICQUser* u = gUserManager.FetchUser(Uin, LOCK_R);
          QString alias = u ? u->GetAlias() : "";
          gUserManager.DropUser(u);

          e->setContact(Uin, alias);
          e->show();
        }
        else {
          UserSendMsgEvent* e = static_cast<UserSendMsgEvent*>
            (gMainWindow->callFunction(mnuUserSendMsg, it->ItemUin()));
          e->setText(text);
          e->show();
        }
      }
    }
    else if(it->isGroupItem())
    {
      QString text;
      if(QTextDrag::decode(e, text)) {
        unsigned long Uin = text.toULong();
        if(Uin >= 10000) {
          gUserManager.AddUserToGroup(Uin, it->GroupId());
          gMainWindow->updateUserWin();
        }
      }
    }
  }
}


void CUserView::keyPressEvent(QKeyEvent *e)
{
  if (e->state() & ControlButton || e->state() & AltButton)
  {
    e->ignore();
    QListView::keyPressEvent(e);
    return;
  }

  switch (e->key())
  {
    case Key_Return:
    case Key_Enter:
    case Key_Space:
    {
      CUserViewItem* item = static_cast<CUserViewItem*>(currentItem());
      if(item == NULL) return;

      if(item->isGroupItem()) {
        setOpen(item, !item->isOpen());
        return;
      }

      // user divider
      if(item->ItemUin() == 0)  return;
      gMainWindow->SetUserMenuUin(item->ItemUin());
      mnuUser->popup(mapToGlobal(QPoint(40, itemPos(item))));
      return;
    }

    case Key_Home:
    {

      QListViewItemIterator it(this);

      while(it.current() != NULL &&
            ((CUserViewItem*)(it.current()))->ItemUin() == 0)  ++it;
      setSelected(it.current(), true);
      ensureItemVisible(it.current());
      return;
    }

    case Key_End:
    {

      QListViewItemIterator it(this);
      QListViewItem* lastitem = 0;
      while(it.current() != NULL) {
        lastitem = it.current();
        ++it;
      }
      it = lastitem;
      while(it.current() && ((CUserViewItem*)(it.current()))->ItemUin() == 0)  --it;
      setSelected(it.current(), true);
      ensureItemVisible(it.current());
      return;
    }

    default:
    {
      char ascii = tolower(e->ascii());
      if (!isalnum(ascii))
      {
        QListView::keyPressEvent(e);
        return;
      }

      QListViewItemIterator it(currentItem() != NULL ? currentItem() : firstChild());
      if(currentItem() != NULL)  ++it;

      while (it.current() != NULL)
      {
        CUserViewItem* item = static_cast<CUserViewItem*>(it.current());
        if (item->text(1).at(0).lower().latin1() == ascii)
        {
          setSelected(item, true);
          ensureItemVisible(item);
          return;
        }
        it++;
      }

      // Check the first elements if we didn't find anything yet
      if (currentItem() != NULL)
      {
        it = firstChild();
        while (it.current() != NULL && it.current() != currentItem())
        {
          CUserViewItem* item = static_cast<CUserViewItem*>(it.current());
          if (item->text(1).at(0).lower().latin1() == ascii)
          {
            setSelected(item, true);
            ensureItemVisible(item);
            return;
          }
          ++it;
        }
      }

      // If we are here we didn't find any names
      QListView::keyPressEvent(e);
    }
  }
}


void CUserView::resizeEvent(QResizeEvent *e)
{
  QListView::resizeEvent(e);

  unsigned short totalWidth = 0;
  unsigned short nNumCols = header()->count();
  for (unsigned short i = 0; i < nNumCols - 1; i++)
    totalWidth += columnWidth(i);
  //QScrollBar *s = verticalScrollBar();
  //if (s != NULL) totalWidth += s->width();
  int newWidth = width() - totalWidth - 2;
  if (newWidth <= 0)
  {
    setHScrollBarMode(Auto);
    setColumnWidth(nNumCols - 1, gMainWindow->colInfo[nNumCols - 2]->m_nWidth);
  }
  else
  {
    setHScrollBarMode(AlwaysOff);
    setColumnWidth(nNumCols - 1, newWidth);
  }
}

void CUserView::AnimationAutoResponseCheck(unsigned long uin)
{
  if(carTimerId == 0) {
    // no animation yet running, so start the timer
    carTimerId = startTimer(FLASH_TIME);
    carCounter = 5*1000/FLASH_TIME; // run about 5 seconds
    carUin = uin;
  }
  // well, maybe we should move the animation to the other user
}


void CUserView::AnimationOnline(unsigned long uin)
{
  if(onlTimerId == 0) {
    onlTimerId = startTimer(FLASH_TIME);
    // run about 5 seconds, make sure that that the
    // actual "flashing" starts with a delay, because of the
    // logon case.
    onlCounter = ((5*1000/FLASH_TIME)+1)&(-2);
    onlUin = uin;
  }
  else if((onlCounter & 1) == 0 && onlUin != uin)
  {
    // whoops, another user went online
    // we just block here the blinking for the
    // rest of the time
    onlUin = 0;
    // no need for a redraw, as the user is already shown
    // correctly.
  }
}


// -----------------------------------------------------------------------------

void  CUserView::viewportMouseReleaseEvent(QMouseEvent* me)
{
  mousePressPos.setX(0);
  mousePressPos.setY(0);
}


void CUserView::UpdateFloaties()
{
  for (unsigned int i = 0; i<floaties->size(); i++)
  {
    CUserViewItem* item = static_cast<CUserViewItem*>(floaties->at(i)->firstChild());
    ICQUser *u = gUserManager.FetchUser(item->ItemUin(), LOCK_R);
    if (u == NULL) return;
    item->setGraphics(u);
    gUserManager.DropUser(u);
    floaties->at(i)->triggerUpdate();
  }
}


// -----------------------------------------------------------------------------

void CUserView::viewportMouseMoveEvent(QMouseEvent * me)
{
  CUserViewItem *i;
  QListView::viewportMouseMoveEvent(me);
  if (parent() && (me->state() & LeftButton) && (i = (CUserViewItem *)currentItem())
      && !mousePressPos.isNull() && i->ItemUin() &&
      (QPoint(me->pos() - mousePressPos).manhattanLength() > 8))
  {
    QTextDrag *d = new QTextDrag(QString::number(i->ItemUin()), this);
    d->dragCopy();
  }
  else if(!parent() && me->state() & LeftButton) {
    move(me->globalX()-mousePressPos.x(), me->globalY()-mousePressPos.y());
  }
}


// -----------------------------------------------------------------------------

void CUserView::itemExpanded(QListViewItem* i)
{
  if(i == NULL) return;
  CUserViewItem* it = static_cast<CUserViewItem*>(i);

  gMainWindow->m_nGroupStates |= 1<<it->GroupId();

  if(pixExpanded != NULL)  i->setPixmap(0, *pixExpanded);
}

void CUserView::itemCollapsed(QListViewItem* i)
{
  if(i == NULL) return;
  CUserViewItem* it = static_cast<CUserViewItem*>(i);

  gMainWindow->m_nGroupStates &= ~(1<<it->GroupId());

  if(pixCollapsed != NULL)  i->setPixmap(0, *pixCollapsed);
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

  CUserViewItem* item = static_cast<CUserViewItem*>(w->itemAt(p));
  if(item && item->m_nUin)
  {
    QRect r(w->itemRect(item));
    if(w->header()->isVisible())
      r.moveBy(0, w->header()->height());
    QString s = ICQUser::StatusToStatusStr(item->m_nStatus, item->m_bStatusInvisible);
    if (item->m_nStatusFull & ICQ_STATUS_FxBIRTHDAY) {
      s = QStyleSheet::convertFromPlainText(s);
      s += CUserView::tr("\n<b>Birthday Today!</b>");
    }
    tip(r, s);
  }
}

#include "userbox.moc"
