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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qheader.h>

#include "licq_message.h"
#include "licq_icq.h"
#include "messagebox.h"
#include "eventdesc.h"
#include "gui-defines.h"

MsgViewItem::MsgViewItem(CUserEvent *theMsg, QListView *parent) : QListViewItem(parent)
{
  if (theMsg->Direction() == D_SENDER)
    msg = theMsg;
  else
    msg = theMsg->Copy();

  m_nEventId = msg->Direction() == D_SENDER ? -1 : theMsg->Id();
  QDateTime d;
  d.setTime_t(msg->Time());
  QString sd = d.toString();
  sd.truncate(sd.length() - 5);

  setText(0, msg->Direction() == D_SENDER ? "S" : "*R");
  SetEventLine();
  QString t =  "-----";

  if (msg->IsDirect())    t[0] = 'D';
  if (msg->IsUrgent())    t[1] = 'U';
  if (msg->IsMultiRec())  t[2] = 'M';
  //if (msg->IsCancelled()) t[3] = 'C';
  if (msg->IsLicq())      t[3] = 'L';
  if (msg->IsEncrypted()) t[4] = 'E';

  setText(2, t);
  setText(3, sd);
}


MsgViewItem::~MsgViewItem(void)
{
  delete msg;
}


void MsgViewItem::SetEventLine()
{
  QString s = EventDescription(msg);
  const char *sz = NULL;

  switch(msg->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      sz = msg->Text();
      break;

    case ICQ_CMDxSUB_URL:
      sz = ((CEventUrl *)msg)->Url();
      break;

    case ICQ_CMDxSUB_CHAT:
      sz = ((CEventChat *)msg)->Reason();
      break;

    case ICQ_CMDxSUB_FILE:
      sz = ((CEventFile *)msg)->Filename();
      break;

    default:
      break;
  }

  if (sz != NULL && sz[0] != '\0')
  {
    int width = listView()->columnWidth(1);
    QFont f = listView()->font();
    if (m_nEventId != -1) f.setBold(true);
    QFontMetrics fm(f);
    width -= fm.width(s) + fm.width(" [...]") + listView()->itemMargin() * 2;
    unsigned short n = 0;

    s += " [";
    while (sz[n] != '\n' && sz[n] != '\0')
    {
      width -= fm.width(sz[n]);
      if (width <= 0) break;
      s += sz[n++];
    }
    if (sz[n] != '\0')
      s += "...]";
    else
      s += "]";
  }

  setText(1, s);
}


void MsgViewItem::MarkRead()
{
  m_nEventId = -1;
  setText(0, msg->Direction() == D_SENDER ? "S" : "R");
  SetEventLine();
}


void MsgViewItem::paintCell( QPainter * p, const QColorGroup &cgdefault,
   int column, int width, int align )
{
  QColorGroup cg(cgdefault);
  if (msg->Direction() == D_SENDER)
  {
    cg.setColor(QColorGroup::Text, COLOR_SENT);
    cg.setColor(QColorGroup::HighlightedText, COLOR_SENT);
  }
  else
  {
    cg.setColor(QColorGroup::Text, COLOR_RECEIVED);
    cg.setColor(QColorGroup::HighlightedText, COLOR_RECEIVED);
  }
  QFont f(p->font());
  f.setBold(m_nEventId != -1 && msg->Direction() == D_RECEIVER);
  f.setItalic(msg->IsUrgent());
  p->setFont(f);

  cg.setColor(QColorGroup::Highlight, cg.color(QColorGroup::Mid));

  QListViewItem::paintCell(p, cg, column, width, align);

  // add line to bottom and right side
  p->setPen(cg.dark());
  p->drawLine(0, height() - 1, width - 1, height() - 1);
  p->drawLine(width - 1, 0, width - 1, height() - 1);
}


//-----MsgView::constructor------------------------------------------------------------------------
MsgView::MsgView (QWidget *parent)
  : QListView(parent, "MessageView"), QToolTip(viewport())
{
  addColumn(tr("D"), 20);
  addColumn(tr("Event Type"), 100);
  addColumn(tr("Options"), 50);
  addColumn(tr("Time"), 130);
  setAllColumnsShowFocus (true);
  setColumnAlignment(0, AlignHCenter);
  setVScrollBarMode(AlwaysOn);
  setHScrollBarMode(AlwaysOff);
  setSorting(-1);
  connect(this, SIGNAL(sizeChange( int, int, int )), this, SLOT(handleSizeChange( int, int, int )) );
  header()->hide();

  QPalette pal(QListView::palette());
  QColor c = pal.color(QPalette::Active, QColorGroup::Background);
  pal.setColor(QPalette::Active, QColorGroup::Base, c);
  pal.setColor(QPalette::Inactive, QColorGroup::Base, c);
  QListView::setPalette(pal);

  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setMinimumHeight(40);
}

CUserEvent *MsgView::currentMsg(void)
{
   if (currentItem() == NULL) return (NULL);
   return (((MsgViewItem *)currentItem())->msg);
}


QSize MsgView::sizeHint() const
{
  QSize s = QListView::sizeHint();
  s.setHeight(minimumHeight());

  return s;
}


void MsgView::resizeEvent(QResizeEvent *e)
{
  QScrollBar *s = verticalScrollBar();
  int ow = header()->sectionSize(1);
  int nw = width() - 200 - s->width();
  QListView::resizeEvent(e);
  if (ow != nw)
  {
    header()->resizeSection(1, nw);
    emit sizeChange(1, ow, nw);
  }
  SetEventLines();
}


void MsgView::SetEventLines()
{
  QListViewItemIterator it(this);
  while(it.current())
  {
    MsgViewItem *item = static_cast<MsgViewItem*>(it.current());
    item->SetEventLine();
    it++;
  }
}

void MsgView::maybeTip(const QPoint& c)
{
  MsgViewItem *item = (MsgViewItem*) itemAt(c);

  if (item == NULL) return;

  QRect r(itemRect(item));

  QString s(item->msg->IsDirect() ? tr("Direct") : tr("Server"));
  if (item->msg->IsUrgent())
    s += QString(" / ") + tr("Urgent");
  if (item->msg->IsMultiRec())
    s += QString(" / ") + tr("Multiple Recipients");
  if (item->msg->IsCancelled())
    s += QString(" / ") + tr("Cancelled Event");
  if (item->msg->IsLicq())
    s += QString(" / Licq ") + QString::fromLocal8Bit(item->msg->LicqVersionStr());

  tip(r, s);
}


// -----------------------------------------------------------------------------

#include "messagebox.moc"
