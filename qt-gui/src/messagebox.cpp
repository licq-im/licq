// -*- c-basic-offset: 2 -*-
/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2009 Licq developers
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

#include <qpainter.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qheader.h>
#include <qtextcodec.h>

#include "licq_message.h"
#include "licq_icq.h"
#include "messagebox.h"
#include "eventdesc.h"
#include "gui-defines.h"

MsgViewItem::MsgViewItem(const CUserEvent* theMsg, QTextCodec *codec, QListView *parent) : QListViewItem(parent)
{
  msg = theMsg->Copy();

  m_codec = codec;

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
  QString text;

  switch(msg->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      text = m_codec->toUnicode(msg->Text());
      break;

    case ICQ_CMDxSUB_URL:
      text = m_codec->toUnicode(((CEventUrl *)msg)->Url());
      break;

    case ICQ_CMDxSUB_CHAT:
      text = m_codec->toUnicode(((CEventChat *)msg)->Reason());
      break;

    case ICQ_CMDxSUB_FILE:
      text = m_codec->toUnicode(((CEventFile *)msg)->Filename());
      break;

    case ICQ_CMDxSUB_EMAILxALERT:
      text = m_codec->toUnicode(((CEventEmailAlert *)msg)->From());
      break;

    default:
      break;
  }

  if (!text.isNull())
  {
    int width = listView()->columnWidth(1);
    QFont f = listView()->font();
    if (m_nEventId != -1) f.setBold(true);
    QFontMetrics fm(f);
    width -= fm.width(s) + fm.width(" [...]") + listView()->itemMargin() * 2;

    s += " [";
    
    // We're going to take the event's message and display as much of it
    // as fits in the widget. If not everything fits, we'll append "..."
    uint length = text.length();
    const QChar *c = text.unicode();
    while (length--) {
      if (*c == '\n') break; // we only print the first line
      width -= fm.width(*c);
      if (width <= 0) {
         s += "...";
         break;
      }
      s += *c;
      c++;
    }
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
