/*
 * This file is part of Licq, an instant messaging client for UNIX.
 * Copyright (C) 1999-2011 Licq developers <licq-dev@googlegroups.com>
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

#ifndef MESSAGELIST_H
#define MESSAGELIST_H

#include <QTreeWidget>


namespace Licq
{
class UserEvent;
}

namespace LicqQtGui
{
class MessageListItem : public QTreeWidgetItem
{
public:
  MessageListItem(const Licq::UserEvent* theMsg, QTreeWidget* parent);
  ~MessageListItem(void);
  void MarkRead();
  void SetEventLine();

  Licq::UserEvent* msg() { return myMsg; }
  bool isUnread() const { return myUnread; }

private:
  bool myUnread;
  Licq::UserEvent* myMsg;
};

class MessageList : public QTreeWidget
{
  Q_OBJECT
public:
  MessageList (QWidget* parent = 0);
  Licq::UserEvent* currentMsg();
  QSize sizeHint() const;
  int getNumUnread() const;
  MessageListItem* getNextUnread();

signals:
  void sizeChange(int, int, int);

private:
  virtual void resizeEvent(QResizeEvent* e);
  virtual bool event(QEvent* event);
  void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  void SetEventLines();
};

} // namespace LicqQtGui

#endif
