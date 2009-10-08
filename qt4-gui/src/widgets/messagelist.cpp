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

#include "messagelist.h"

#include "config.h"

#include <QDateTime>
#include <QHeaderView>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QTextCodec>

#include <licq_message.h>
#include <licq_icq.h>

#include "helpers/eventdesc.h"

using namespace LicqQtGui;
/* TRANSLATOR LicqQtGui::MessageList */
/* TRANSLATOR LicqQtGui::MessageListItem */

MessageListItem::MessageListItem(const CUserEvent* theMsg, const QTextCodec* codec, QTreeWidget* parent)
  : QTreeWidgetItem(parent)
{
  // Keep a copy of the event
  myMsg = theMsg->Copy();

  myCodec = codec;

  myUnread = (myMsg->Direction() == D_RECEIVER);

  setText(0, myMsg->Direction() == D_SENDER ? "S" : "*R");
  setTextAlignment(0, Qt::AlignHCenter);
  SetEventLine();
  QString t =  "-----";

  if (myMsg->IsDirect())    t[0] = 'D';
  if (myMsg->IsUrgent())    t[1] = 'U';
  if (myMsg->IsMultiRec())  t[2] = 'M';
  //if (myMsg->IsCancelled()) t[3] = 'C';
  if (myMsg->IsLicq())      t[3] = 'L';
  if (myMsg->IsEncrypted()) t[4] = 'E';

  setText(2, t);
  setTextAlignment(2, Qt::AlignHCenter);

  QDateTime d;
  d.setTime_t(myMsg->Time());
  QString sd = d.toString();
  sd.truncate(sd.length() - 5);
  setText(3, sd);

  QColor foreColor;
  if (myMsg->Direction() == D_SENDER)
    foreColor = QColor("blue");
  else
    foreColor = QColor("red");
  setForeground(0, foreColor);
  setForeground(1, foreColor);
  setForeground(2, foreColor);
  setForeground(3, foreColor);

  QFont f(font(0));
  f.setBold(myUnread);
  f.setItalic(myMsg->IsUrgent());
  setFont(0, f);
  setFont(1, f);
  setFont(2, f);
  setFont(3, f);

  // Add ourselves first in the list instead of last
  int index = parent->indexOfTopLevelItem(this);
  if (index > -1)
    parent->takeTopLevelItem(index);
  parent->insertTopLevelItem(0, this);
}

MessageListItem::~MessageListItem(void)
{
  delete myMsg;
}

void MessageListItem::SetEventLine()
{
  QString s = EventDescription(myMsg);
  QString text;

  switch(myMsg->SubCommand())
  {
    case ICQ_CMDxSUB_MSG:
      text = myCodec->toUnicode(myMsg->Text());
      break;

    case ICQ_CMDxSUB_URL:
      text = myCodec->toUnicode(dynamic_cast<CEventUrl*>(myMsg)->Url());
      break;

    case ICQ_CMDxSUB_CHAT:
      text = myCodec->toUnicode(dynamic_cast<CEventChat*>(myMsg)->Reason());
      break;

    case ICQ_CMDxSUB_FILE:
      text = myCodec->toUnicode(dynamic_cast<CEventFile*>(myMsg)->Filename());
      break;

    case ICQ_CMDxSUB_EMAILxALERT:
      text = myCodec->toUnicode(dynamic_cast<CEventEmailAlert*>(myMsg)->From());
      break;

    default:
      break;
  }

  if (!text.trimmed().isEmpty())
    s += " [" + text.trimmed().replace('\n', "   ") + "]";
  setText(1, s);
}

void MessageListItem::MarkRead()
{
  myUnread = false;
  QFont f(font(0));
  f.setBold(false);
  f.setItalic(myMsg->IsUrgent());
  setFont(0, f);
  setFont(1, f);
  setFont(2, f);
  setFont(3, f);

  setText(0, myMsg->Direction() == D_SENDER ? "S" : "R");
  SetEventLine();
}

//-----MessageList::constructor------------------------------------------------------------------------
MessageList::MessageList(QWidget* parent)
  : QTreeWidget(parent)
{
  setColumnCount(4);
  QStringList headers;
  headers << tr("D") << tr("Event Type") << tr("Options") << tr("Time");
  setHeaderLabels(headers);
  setAllColumnsShowFocus(true);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSortingEnabled(false);
  setIndentation(0);
  header()->hide();

  QPalette pal(palette());
  QColor c = pal.color(QPalette::Active, QPalette::Window);
  pal.setColor(QPalette::Active, QPalette::Base, c);
  pal.setColor(QPalette::Inactive, QPalette::Base, c);
  pal.setColor(QPalette::Highlight, pal.color(QPalette::Mid));
  setPalette(pal);

  setFrameStyle(Panel | Sunken);
  setMinimumHeight(40);
}

CUserEvent* MessageList::currentMsg()
{
  if (currentItem() == NULL)
    return NULL;
  return (dynamic_cast<MessageListItem*>(currentItem())->msg());
}

QSize MessageList::sizeHint() const
{
  QSize s = QTreeWidget::sizeHint();
  s.setHeight(minimumHeight());

  return s;
}

int MessageList::getNumUnread() const
{
  int num = 0;

  for (int i = 0; i < topLevelItemCount(); ++i)
  {
    MessageListItem* item = dynamic_cast<MessageListItem*>(topLevelItem(i));
    if (item->isUnread())
      num++;
  }
  return num;
}

MessageListItem* MessageList::getNextUnread()
{
  MessageListItem* next = NULL;
  for (int i = 0; i < topLevelItemCount(); ++i)
  {
    MessageListItem* item = dynamic_cast<MessageListItem*>(topLevelItem(i));
    if (item->isUnread())
      next = item;
  }
  return next;
}

void MessageList::resizeEvent(QResizeEvent* e)
{
  QScrollBar* s = verticalScrollBar();
  int ow = header()->sectionSize(1);
  int nw = width() - 200 - s->width();
  QTreeWidget::resizeEvent(e);
  if (ow != nw)
  {
    header()->resizeSection(1, nw);
    emit sizeChange(1, ow, nw);
  }
  SetEventLines();
}

void MessageList::SetEventLines()
{
  for (int i = 0; i < topLevelItemCount(); ++i)
  {
    MessageListItem* item = dynamic_cast<MessageListItem*>(topLevelItem(i));
    item->SetEventLine();
  }
}

bool MessageList::event(QEvent* event)
{
  if (event->type() == QEvent::ToolTip)
  {
    QHelpEvent* helpEvent = dynamic_cast<QHelpEvent*>(event);
    MessageListItem* item = dynamic_cast<MessageListItem*>(itemAt(helpEvent->pos()));

    if (item != NULL)
    {
      QString s(item->msg()->IsDirect() ? tr("Direct") : tr("Server"));
      if (item->msg()->IsUrgent())
        s += QString(" / ") + tr("Urgent");
      if (item->msg()->IsMultiRec())
        s += QString(" / ") + tr("Multiple Recipients");
      if (item->msg()->IsCancelled())
        s += QString(" / ") + tr("Cancelled Event");
      if (item->msg()->IsLicq())
        s += QString(" / Licq ") + QString::fromLocal8Bit(item->msg()->LicqVersionStr());

      setToolTip(s);
    }
  }

  return QTreeWidget::event(event);
}

void MessageList::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem option2(option);
  // Make highlighted row have same text color as when it is not highlighted
  option2.palette.setBrush(QPalette::HighlightedText, itemFromIndex(index)->foreground(0));

  QTreeWidget::drawRow(painter, option2, index);
  QRect r = visualRect(index);

  painter->save();
  painter->setPen(QPen(option2.palette.dark(), 1));
  int right = 0;
  for (int i = 0; i < columnCount(); ++i)
  {
    right += columnWidth(i);
    painter->drawLine(right, r.top(), right, r.bottom());
  }
  painter->drawLine(r.left(), r.bottom(), right, r.bottom());
  painter->restore();
}
