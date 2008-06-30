// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 2000-2006 Licq developers
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>

#include <qpainter.h>
#include <qpopupmenu.h>
#include <qheader.h>
#include <qkeycode.h>
#include <qscrollbar.h>
#include <qdragobject.h>
#include <qtextcodec.h>

#include "mmlistview.h"
#include "gui-defines.h"
#include "ewidgets.h"
#include "mainwin.h"
#include "licq_user.h"
#include "usercodec.h"


//-----CUserViewItem::constructor-----------------------------------------------
CMMUserViewItem::CMMUserViewItem(ICQUser *u, QListView *parent)
   : QListViewItem(parent)
{
  char *sTemp;

  m_szId = u->IdString() ? strdup(u->IdString()) : 0;
  m_nPPID = u->PPID();

  CMMUserView *v = (CMMUserView *)listView();

  for (unsigned short i = 0; i < v->colInfo.size(); i++)
  {
    sTemp = u->usprintf(v->colInfo[i]->m_szFormat);
    setText(i, QString::fromUtf8(sTemp));
    free(sTemp);
  }
}


CMMUserViewItem::~CMMUserViewItem()
{
}



// ---------------------------------------------------------------------------
/*
void CMMUserViewItem::paintCell( QPainter * p, const QColorGroup & cgdefault, int column, int width, int align )
{
  QListViewItem::paintCell(p, cgdefault, column, width, align);
}
*/

//-----CUserViewItem::key-------------------------------------------------------
/*QString CUserViewItem::key (int column, bool ascending) const
{
  return(m_sPrefix + QListViewItem::key(column, ascending));
}*/

enum mnuMM_ids
{
  mnuMM_remove,
  mnuMM_crop,
  mnuMM_clear,
  mnuMM_addgroup,
  mnuMM_addall
};

//-----UserList::constructor-----------------------------------------------------------------------
CMMUserView::CMMUserView (ColumnInfos &_colInfo, bool bHeader,
    const char* szId, unsigned long nPPID, CMainWindow *pMainwin,
   QWidget *parent)
   : QListView(parent, "MMUserView")
{
  mnuMM = new QPopupMenu(NULL);
  mnuMM->insertItem(tr("Remove"), mnuMM_remove);
  mnuMM->insertItem(tr("Crop"), mnuMM_crop);
  mnuMM->insertItem(tr("Clear"), mnuMM_clear);
  mnuMM->insertSeparator();
  mnuMM->insertItem(tr("Add Group"), mnuMM_addgroup);
  mnuMM->insertItem(tr("Add All"), mnuMM_addall);
  connect(mnuMM, SIGNAL(activated(int)), SLOT(slot_menu(int)));

  colInfo = _colInfo;
  m_szId = szId ? strdup(szId) : 0;
  m_nPPID = nPPID;
  mainwin = pMainwin;

  for (unsigned short i = 0; i < colInfo.size(); i++)
  {
    addColumn(colInfo[i]->m_sTitle, colInfo[i]->m_nWidth);
    setColumnAlignment(i, 1<<colInfo[i]->m_nAlign);
  }

  setAllColumnsShowFocus (true);
  setSelectionMode(Extended);
  setSorting(0);
  bHeader ? header()->show() : header()->hide();

  setAcceptDrops(true);
}

CMMUserView::~CMMUserView()
{
  if (m_szId) free(m_szId);
}


void CMMUserView::slot_menu(int id)
{
  setUpdatesEnabled(false);

  switch (id)
  {
    case mnuMM_remove:
    {
      CMMUserViewItem *i = (CMMUserViewItem *)firstChild();
      CMMUserViewItem *j = NULL;
      while (i != NULL)
      {
        j = (CMMUserViewItem *)i->nextSibling();
        if (i->isSelected()) delete i;
        i = j;
      }
      break;
    }

    case mnuMM_crop:
    {
      CMMUserViewItem *i = (CMMUserViewItem *)firstChild();
      CMMUserViewItem *j = NULL;
      while (i != NULL)
      {
        j = (CMMUserViewItem *)i->nextSibling();
        if (!i->isSelected()) delete i;
        i = j;
      }
      break;
    }

    case mnuMM_clear:
      clear();
      break;

    case mnuMM_addgroup:
    {
      CUserViewItem *i = (CUserViewItem *)mainwin->UserView()->firstChild();
      while (i != NULL)
      {
        AddUser(i->ItemId(), i->ItemPPID());
        i = (CUserViewItem *)i->nextSibling();
      }
      break;
    }

    case mnuMM_addall:
    {
      clear();
      FOR_EACH_USER_START(LOCK_R)
      {
        if (pUser->PPID() != m_nPPID || strcmp(pUser->IdString(), m_szId))
          (void) new CMMUserViewItem(pUser, this);
      }
      FOR_EACH_USER_END
      break;
    }
  } // switch

  setUpdatesEnabled(true);
  triggerUpdate();
}


void CMMUserView::dragEnterEvent(QDragEnterEvent * dee)
{
  dee->accept(QTextDrag::canDecode(dee));
}


void CMMUserView::dropEvent(QDropEvent * de)
{
  QString text;

  // extract the text from the event
  if (QTextDrag::decode(de, text) == false)
  {
    WarnUser(NULL, tr("Drag'n'Drop didn't work"));
    return;
  }

  //TODO get protocol id from text
  AddUser(text.mid(4, text.length() - 4).latin1(), LICQ_PPID);
}

void CMMUserView::AddUser(const char *szId, unsigned long nPPID)
{
  if (szId == 0 || (nPPID == m_nPPID && strcmp(szId, m_szId) == 0))
    return;

  CMMUserViewItem *i = (CMMUserViewItem *)firstChild();
  while (i != NULL && (i->PPID() != nPPID || strcmp(i->Id(), szId )))
    i = (CMMUserViewItem *)i->nextSibling();
  if (i != NULL) return;

  ICQUser *u = gUserManager.FetchUser(szId, nPPID, LOCK_R);
  if (u == NULL) return;
  (void) new CMMUserViewItem(u, this);
  gUserManager.DropUser(u);
}

//-----CUserList::mousePressEvent---------------------------------------------
void CMMUserView::viewportMousePressEvent(QMouseEvent *e)
{
  QListView::viewportMousePressEvent(e);
  if (e->button() == LeftButton)
  {
    //mousePressPos = e->pos();
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
    CMMUserViewItem *clickedItem = (CMMUserViewItem *)itemAt(e->pos());
    if (clickedItem != NULL)
    {
      setSelected(clickedItem, true);
      setCurrentItem(clickedItem);
    }
    mnuMM->popup(mapToGlobal(e->pos()) + QPoint(4,-5), 1);
  }
}



void CMMUserView::keyPressEvent(QKeyEvent *e)
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
      int nMenuWidth = mnuMM->width();
      if ( 512 == nMenuWidth )
         nMenuWidth = 120;
      // Calculate where to position the menu
      const QListViewItem *pcItem = currentItem();
      QPoint cRelPos( (width() - nMenuWidth)/2,
                     itemPos(pcItem) + pcItem->height() );
      QPoint cPos( mapToGlobal( cRelPos ) );
      mnuMM->popup( cPos );
      return;
    }

    case Key_Home:
      item = firstChild();
      if (item == NULL) return;
      if (((CMMUserViewItem *)item)->Id() == NULL)
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


void CMMUserView::resizeEvent(QResizeEvent *e)
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
    setColumnWidth(nNumCols - 1, colInfo[nNumCols - 2]->m_nWidth);
  }
  else
  {
    setHScrollBarMode(AlwaysOff);
    setColumnWidth(nNumCols - 1, newWidth);
  }
}


// -----------------------------------------------------------------------------
/*
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
*/



#include "mmlistview.moc"
