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
#include <stdio.h>

#include <qpainter.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qkeycode.h>
#include <qscrollbar.h>
#include <qdragobject.h>
#include <qstylesheet.h>
#include <qdatetime.h>
#include <qtextcodec.h>
#include <qdrawutil.h>
#include <qstyle.h>

#include "userbox.moc"
#include "skin.h"
#include "mainwin.h"
#include "userbox.h"
#include "gui-defines.h"
#include "mainwin.h"
#include "usereventdlg.h"
#include "usercodec.h"

#include "licq_user.h"
#include "licq_socket.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xpm/itemCollapsed.xpm"
#include "xpm/itemExpanded.xpm"
#include "xpm/pixCustomAR.xpm"
#include "xpm/pixPhone.xpm"
#include "xpm/pixCellular.xpm"
#include "xpm/pixBirthday.xpm"
#include "xpm/pixInvisible.xpm"


#undef Status

#define FLASH_TIME 500

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
  m_bSecure = false;
  m_bBirthday = false;
  m_bPhone = false;
  m_bCellular = false;
  m_nOnlCount = 0;
  m_nEvents = 0;
  setGraphics(_cUser);
}


CUserViewItem::CUserViewItem (ICQUser *_cUser, CUserViewItem* item)
  : QListViewItem(item),
    m_sGroupName()
{
  m_nGroupId = (unsigned short)(-1);
  m_nUin = _cUser->Uin();
  m_bUrgent = false;
  m_bSecure = false;
  m_bBirthday = false;
  m_bPhone = false;
  m_bCellular = false;
  m_nOnlCount = 0;
  m_nEvents = 0;
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
  m_bSecure = false;
  m_bBirthday = false;
  m_bPhone = false;
  m_bCellular = false;
  m_nOnlCount = 0;
  m_nEvents = 0;
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
  m_nEvents = 0;
  m_pIcon = NULL;
  m_cBack = s_cBack;
  m_cFore = s_cOnline;
  m_bItalic = m_bStrike = false;
  m_nWeight = QFont::Normal;
  m_bUrgent = false;
  m_bSecure = false;
  setSelectable(false);
  setHeight(10);
  m_sSortKey = "";
  if (barType == BAR_ONLINE)
    m_sPrefix = "0";
  else
    m_sPrefix = "2";
}

void CUserViewItem::SetThreadViewGroupTitle()
{
    QString t;
    /*if (m_nEvents > 0)
      t = QString("* ");*/
    t += QString::fromLocal8Bit(m_sGroupName);
    if (m_nOnlCount > 0)
      t += QString(" (") + QString::number(m_nOnlCount) + QString(")");
    setText(1, t);
}


CUserViewItem::~CUserViewItem()
{
  CUserView *v = (CUserView *)listView();

  if (v == NULL || m_nUin == 0) return;

  if (m_nStatus == ICQ_STATUS_OFFLINE)
    v->numOffline--;
  else
    v->numOnline--;

  if (parent())
  {
    CUserViewItem *i = static_cast<CUserViewItem*>(parent());
    if (m_nStatus != ICQ_STATUS_OFFLINE) i->m_nOnlCount--;
    i->m_nEvents -= m_nEvents;
    i->SetThreadViewGroupTitle();
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

   if (parent())
   {
     CUserViewItem *i = static_cast<CUserViewItem*>(parent());
     if (u->StatusOffline() && m_nStatus != ICQ_STATUS_OFFLINE)
       i->m_nOnlCount--;
     if (m_nStatus == ICQ_STATUS_OFFLINE && !u->StatusOffline())
       i->m_nOnlCount++;

     i->m_nEvents -= m_nEvents;
     i->m_nEvents += u->NewMessages();

     i->SetThreadViewGroupTitle();
   }

   m_nStatus = u->Status();
   m_nStatusFull = u->StatusFull();
   m_bStatusInvisible = u->StatusInvisible();
   m_bCustomAR = u->CustomAutoResponse()[0] != '\0';
   m_nEvents = u->NewMessages();
   m_bSecure = u->Secure();
   m_bUrgent = false;
   m_bBirthday =  (u->Birthday() == 0);
   m_bPhone  = u->GetPhoneNumber()[0] != '\0';
   m_bCellular = u->GetCellularNumber()[0] !='\0';

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

   //if (u->StatusInvisible())
   //   m_cFore = s_cAway;

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

   m_bFlash = ((u->NewMessages() > 0 && gMainWindow->m_nFlash == FLASH_ALL) ||
       (m_bUrgent && gMainWindow->m_nFlash == FLASH_URGENT));

   if (v->msgTimerId == 0 && m_bFlash)
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
     QTextCodec * codec = UserCodec::codecForICQUser(u);
     u->usprintf(sTemp, gMainWindow->colInfo[i]->m_szFormat);
     setText(i + 1, codec->toUnicode(sTemp));
   }

   // Set the user tag
   if (gMainWindow->m_bSortByStatus)
   {
     int sort = 9;
     switch (m_nStatus)
     {
     case ICQ_STATUS_FREEFORCHAT:
     case ICQ_STATUS_ONLINE:
       sort = 0;
       break;
     case ICQ_STATUS_OCCUPIED:
       sort = 1;
       break;
     case ICQ_STATUS_DND:
       sort = 2;
       break;
     case ICQ_STATUS_AWAY:
       sort = 3;
       break;
     case ICQ_STATUS_NA:
       sort = 4;
       break;
     case ICQ_STATUS_OFFLINE:
       sort = 5;
       break;
     }
     m_sSortKey.sprintf("%1x%016lx",sort,ULONG_MAX - u->Touched());
   }
   else
     m_sSortKey.sprintf("%016lx", ULONG_MAX - u->Touched());
}


// ---------------------------------------------------------------------------

void CUserViewItem::paintCell( QPainter *p, const QColorGroup & cgdefault, int column, int width, int align )
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

#if QT_VERSION >= 220
  if (listView()->parent() && gMainWindow->skin->frame.transparent )
    pix = listView()->QListView::parentWidget()->backgroundPixmap();

  listView()->setStaticBackground(pix && listView()->contentsHeight() >= listView()->viewport()->height());
#else
  if ((listView()->contentsHeight() < listView()->viewport()->height() ||
       listView()->vScrollBarMode() == QListView::AlwaysOff) &&
      listView()->parent() && gMainWindow->skin->frame.transparent )
    pix = listView()->QListView::parentWidget()->backgroundPixmap();
#endif

  if (pix != NULL)
  {
    QRect r(listView()->itemRect(this));
    QPoint pd(r.topLeft()+QPoint(listView()->header()->sectionPos(column), 0));
    listView()->viewport()->mapToParent(pd);
    QPoint pp(listView()->mapToParent(pd));
    //p->drawPixmap(0, 0, *pix, pp.x(), pp.y(), width, height());
    p->drawTiledPixmap(0, 0, width, height(), *pix, pp.x(), pp.y());
  }
  else
    p->fillRect( 0, 0, width, height(), cg.base());

  if (m_nUin != 0 || isGroupItem())
  {
    cg.setBrush(QColorGroup::Base, QBrush(NoBrush));
    // If this is a floaty then don't draw the highlight box
    if (listView()->parent() == NULL || isGroupItem())
    {
      cg.setBrush(QColorGroup::Highlight, QBrush(NoBrush));
      cg.setColor(QColorGroup::HighlightedText, cg.text());
    }

    if (isGroupItem())
    {
      QFont f(p->font());
      if(f.pointSize() > 2)
        f.setPointSize(f.pointSize() - 2);
      p->setFont(f);
    }

    QListViewItem::paintCell(p, cg, column, width, align);

    if (isGroupItem())
    {
      if (column == 1)
      {
        int w = p->fontMetrics().width(text(1)) + 4;

        if (m_nEvents > 0 && !isOpen())
        {
          p->drawPixmap(w, 0, gMainWindow->pmMessage);
          w += gMainWindow->pmMessage.width() + 4;
        }

#if QT_VERSION >= 300
        qDrawShadeLine( p, 0, height() >> 1, width - 5, (height() >> 1) + 2, cg, true, 1, 0);
#else
        listView()->style().drawSeparator(p,
                                          w, height() >> 1, width - (listView()->header()->count() == 1 ? 5 : 1),
                                          height() >> 1, cg);
#endif
      }
      else if (column == listView()->header()->count() - 1)
      {
#if QT_VERSION >= 300
        qDrawShadeLine( p, 0, height() >> 1, width - 5, (height() >> 1) + 2, cg, true, 1, 0);
#else
        listView()->style().drawSeparator(p, 0, height() >> 1, width - 5,
           height() >> 1, cg);
#endif
      }
      else if (column > 1)
      {
#if QT_VERSION >= 300
        qDrawShadeLine( p, 0, height() >> 1, width - 5, (height() >> 1) + 2, cg, true, 1, 0);
#else
        listView()->style().drawSeparator(p, 0, height() >> 1, width - 1,
           height() >> 1, cg);
#endif
      }
    }
    // If this is the first column then add some extra icons after the text
    else if (column == 1 && gMainWindow->m_bShowExtendedIcons)
    {
      int w = p->fontMetrics().width(text(1)) + 6;

      if (width - w > 8 && (m_bPhone))
      {
        p->drawPixmap(w, 0, *listView()->pixPhone);
        w += listView()->pixPhone->width() + 2;
      }
      if (width - w > 8 && (m_bCellular))
      {
        p->drawPixmap(w, 0, *listView()->pixCellular);
        w += listView()->pixCellular->width() + 2;
      }
      if (width - w > 8 && (m_bBirthday))
      {
        p->drawPixmap(w, 0, *listView()->pixBirthday);
        w += listView()->pixBirthday->width() + 2;
      }
      if (width - w > 8 && m_bStatusInvisible)
      {
        if (gMainWindow->pmPrivate.isNull())
        {
          p->drawPixmap(w, 0, *listView()->pixInvisible);
          w += listView()->pixInvisible->width() + 2;
        }
        else
        {
          p->drawPixmap(w, 0, gMainWindow->pmPrivate);
          w += gMainWindow->pmPrivate.width() + 2;
        }
      }
      if (width - w > 8 && m_bSecure)
      {
        p->drawPixmap(w, 0, gMainWindow->pmSecureOn);
        w += gMainWindow->pmSecureOn.width() + 2;
      }
      if (width - w > 8 && m_bCustomAR)
      {
        p->drawPixmap(w, 0, *listView()->pixCustomAR);
        w += listView()->pixCustomAR->width() + 2;
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
        QRect r(listView()->itemRect(this));
        QPoint pd(r.topLeft()+QPoint(listView()->header()->sectionPos(column)+5, 0));
        listView()->viewport()->mapToParent(pd);
        QPoint pp(listView()->mapToParent(pd));
        //p->drawPixmap(5, 0, *pix, pp.x(), pp.y(), p->fontMetrics().width(sz) + 6, height());
        p->drawTiledPixmap(5, 0, p->fontMetrics().width(sz) + 6, height(), *pix, pp.x(), pp.y());
      }
      else
      {
        p->fillRect(5, 0, p->fontMetrics().width(sz) + 6, height(), *m_cBack);
      }
      QFont f(p->font());
      if (f.pointSize() > 2)
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
#if QT_VERSION >= 220
  if( parent() && gMainWindow->skin->frame.transparent)
    pix = QListView::parentWidget()->backgroundPixmap();
#else
  if ((contentsHeight() < viewport()->height() || vScrollBarMode() == AlwaysOff)
      && parent() && gMainWindow->skin->frame.transparent)
    pix = QListView::parentWidget()->backgroundPixmap();
#endif

  if (pix != NULL)
  {
    QPoint pp(mapToParent(r.topLeft()));
    //p->drawPixmap(r.x(), r.y(), *pix, pp.x(), pp.y(), r.width(), r.height());
    p->drawTiledPixmap(r.x(), r.y(), r.width(), r.height(), *pix, pp.x(), pp.y());
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
    if (m_nFlashCounter++ & 1) // hide event icon
    {
      QListViewItemIterator it(this);
      for(; it.current(); ++it)
      {
        CUserViewItem *item = static_cast<CUserViewItem*>(it.current());
        if (item->ItemUin() && item->m_bFlash && item->m_pIconStatus != NULL)
        {
          item->setPixmap(0, *item->m_pIconStatus);
        }
      }
    }
    else
    {
      // show
      bool foundIcon = false;
      QListViewItemIterator it(this);

      for(; it.current(); ++it)
      {
        CUserViewItem* item = static_cast<CUserViewItem*>(it.current());
        if(item->ItemUin() && item->m_bFlash && item->m_pIcon != NULL)
        {
          foundIcon = true;
          item->setPixmap(0, *item->m_pIcon);
        }
      }
      // no pending messages any more, kill timer
      if(!foundIcon)
      {
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
#if QT_VERSION >= 220
  : QListView(parent, name, parent == NULL ? WStyle_Customize | WStyle_NoBorder | WRepaintNoErase /*| WStyle_StaysOnTop*/
              : WRepaintNoErase),
#else
  : QListView(parent, name),
#endif
    QToolTip(viewport())
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

  viewport()->setAcceptDrops(true);
  #if QT_VERSION < 300
  viewport()->setBackgroundMode(NoBackground);
  setBackgroundMode(NoBackground);
  #endif

#if QT_VERSION >= 210
  setShowSortIndicator(true);
#endif
  setAllColumnsShowFocus(true);
  setTreeStepSize(0);
  setSorting(0);
  setVScrollBarMode(gMainWindow->m_bScrollBar ? Auto : AlwaysOff);

  pixCollapsed = new QPixmap(itemCollapsed_xpm);
  pixExpanded  = new QPixmap(itemExpanded_xpm);
  pixBirthday  = new QPixmap(pixBirthday_xpm);
  pixCustomAR  = new QPixmap(pixCustomAR_xpm);
  pixPhone     = new QPixmap(pixPhone_xpm);
  pixCellular  = new QPixmap(pixCellular_xpm);
  pixInvisible = new QPixmap(pixInvisible_xpm);

  if (parent != NULL)
  {
    setShowHeader(gMainWindow->m_bShowHeader);
    setFrameStyle(gMainWindow->skin->frame.frameStyle);
    connect(this, SIGNAL(expanded(QListViewItem*)), this, SLOT(itemExpanded(QListViewItem*)));
    connect(this, SIGNAL(collapsed(QListViewItem*)), this, SLOT(itemCollapsed(QListViewItem*)));
  }
  else
  {
    char szClass[16];
    sprintf(szClass, "Floaty%d", floaties->size() + 1);
    setWFlags(getWFlags() | WDestructiveClose);
    setShowHeader(false);
    setFrameStyle(33);
    XClassHint classhint;
    classhint.res_name = "licq";
    classhint.res_class = szClass;
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
   CUserViewItem::s_cGridLines->setNamedColor(_sGridLines);
   if (gMainWindow->m_bSystemBackground)
   {
     *CUserViewItem::s_cBack =
      QListView::palette().color(QPalette::Normal, QColorGroup::Base);
   }
   else
   {
     if (_sBack != NULL)
       CUserViewItem::s_cBack->setNamedColor(_sBack);
     else
       CUserViewItem::s_cBack->setNamedColor("grey76");

     QPalette pal(QListView::palette());
     pal.setColor(QColorGroup::Base, *CUserViewItem::s_cBack);
     QListView::setPalette(pal);
   }
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
#if QT_VERSION < 300
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
        mnuUser->popup(viewport()->mapToGlobal(e->pos()), 1);
      }
    }
  }
#endif
}

#if QT_VERSION >= 300
void CUserView::contentsContextMenuEvent ( QContextMenuEvent* e )
{
  CUserViewItem *clickedItem = (CUserViewItem *)itemAt(contentsToViewport( e->pos()) );
  if (clickedItem != NULL)
  {
    setSelected(clickedItem, true);
    setCurrentItem(clickedItem);
    if (clickedItem->ItemUin())
    {
      gMainWindow->SetUserMenuUin(clickedItem->ItemUin());
      mnuUser->popup(viewport()->mapToGlobal(e->pos()), 1);
    }
  }
}
#endif

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
          e->setFile(text, QString::null);
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
      mnuUser->popup(viewport()->mapToGlobal(QPoint(40, itemPos(item))));
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
  QListView::viewportMouseReleaseEvent( me );

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

// -----------------------------------------------------------------------------

void CUserView::maybeTip(const QPoint& c)
{
  CUserViewItem* item = static_cast<CUserViewItem*>(itemAt(c));
  if(item && item->m_nUin)
  {
    QRect r(itemRect(item));
    QString s = QString("<nobr>") + QString(ICQUser::StatusToStatusStr(item->m_nStatus, item->m_bStatusInvisible))
      + QString("</nobr>");

    if (item->m_nStatusFull & ICQ_STATUS_FxBIRTHDAY)
      s += tr("<br><b>Birthday&nbsp;Today!</b>");

    if (item->m_bSecure)
      s += tr("<br>Secure&nbsp;connection");

    if (item->m_bCustomAR)
      s += tr("<br>Custom&nbsp;Auto&nbsp;Response");

    ICQUser* u = gUserManager.FetchUser(item->m_nUin, LOCK_R);
    QTextCodec * codec = UserCodec::codecForICQUser(u);
    if (u != NULL)
    {
      if (u->SecureChannelSupport() == SECURE_CHANNEL_SUPPORTED)
        s += tr("<br>Licq&nbsp;%1/SSL").arg(CUserEvent::LicqVersionToString(u->LicqVersion()));
      else if (u->SecureChannelSupport() == SECURE_CHANNEL_NOTSUPPORTED)
        s += tr("<br>Licq&nbsp;%1").arg(CUserEvent::LicqVersionToString(u->LicqVersion()));

      if (u->AutoResponse() && *u->AutoResponse() &&
          item->m_nStatus != ICQ_STATUS_OFFLINE &&
          item->m_nStatus != ICQ_STATUS_ONLINE)
        s += tr("<br><u>Auto Response:</u>") + QStyleSheet::convertFromPlainText(codec->toUnicode(u->AutoResponse()));

      gUserManager.DropUser(u);
    }

    if (*u->GetEmailPrimary() && gMainWindow->m_bPopEmail)
      s += tr("<br><nobr>E: ") + QString::fromLocal8Bit(u->GetEmailPrimary()) + tr("</nobr>");

    if (item->m_bPhone && gMainWindow->m_bPopPhone)
      s += tr("<br><nobr>P: ") + QString::fromLocal8Bit(u->GetPhoneNumber()) + tr("</nobr>");

    if (item->m_bCellular && gMainWindow->m_bPopCellular)
      s += tr("<br><nobr>C: ") + QString::fromLocal8Bit(u->GetCellularNumber()) + tr("</nobr>");

    if ((u->GetFaxNumber()[0]!='\0') && gMainWindow->m_bPopEmail)
      s += tr("<br><nobr>F: ") + QString::fromLocal8Bit(u->GetFaxNumber()) + tr("</nobr>");

    if ((u->Ip() || u->RealIp()) && gMainWindow->m_bPopIP) {
      char buf1[32];
      char buf[32];
      ip_ntoa(u->Ip(),buf1);
      ip_ntoa(u->RealIp(),buf);
      if (u->Ip() != u->RealIp())
        s += tr("<br><nobr>Ip: ") + buf1 +"/"+buf+ tr("</nobr>");
      else
        s += tr("<br><nobr>Ip: ") + buf1 + tr("</nobr>");
    }

    if ((u->LastOnline()>0) && gMainWindow->m_bPopLastOnline) {
      QDateTime t;
      t.setTime_t(u->LastOnline());
      QString ds = t.toString();
      s += tr("<br><nobr>O: ") +  ds + tr("</nobr>");
    }

    tip(r, s);
  }
}
